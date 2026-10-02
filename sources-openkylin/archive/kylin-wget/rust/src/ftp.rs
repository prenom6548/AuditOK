use crate::cli::Cli;
use crate::exit::ExitCode;
use crate::log;
use crate::progress::{DownloadProgress, label_from_output_path};
use percent_encoding::percent_decode_str;
use std::net::SocketAddr;
use std::path::Path;
use std::time::Instant;
use tokio::io::{
    AsyncBufReadExt, AsyncReadExt, AsyncWrite, AsyncWriteExt, BufReader, ReadHalf, WriteHalf, split,
};
use tokio::fs::File;
use tokio::net::TcpStream;
use url::Url;

/// GNU `ftp.c` 在发 RETR/LIST/CWD 前对 URL path 做 `%XX` 解码，以便 FTP 服务器收到 UTF-8 路径。
fn decode_ftp_path(path: &str) -> String {
    if path.is_empty() {
        return "/".to_string();
    }
    String::from_utf8_lossy(&percent_decode_str(path).collect::<Vec<u8>>()).into_owned()
}

fn ftp_basename(path: &str) -> String {
    let p = path.trim_start_matches('/');
    Path::new(p)
        .file_name()
        .and_then(|s| s.to_str())
        .unwrap_or(p)
        .to_string()
}

/// 从 FTP 应答行提取说明（`550 File unavailable` → `File unavailable`）。
fn ftp_reply_text(code: u16, lines: &[String]) -> String {
    if let Some(line) = lines.last() {
        let t = line.trim();
        if t.len() > 4 {
            let rest = t[4..].trim();
            if !rest.is_empty() {
                return rest.to_string();
            }
        }
    }
    match code {
        421 => "Service not available, closing control connection.",
        425 => "Can't open data connection.",
        426 => "Connection closed; transfer aborted.",
        530 => "Not logged in.",
        550 => "File unavailable.",
        _ => "FTP command failed.",
    }
    .to_string()
}

enum FtpFailContext {
    Generic,
    RetrFile { name: String },
}

fn ftp_fail(
    cli: &Cli,
    raw_url: &str,
    code: u16,
    lines: &[String],
    ctx: FtpFailContext,
) -> ExitCode {
    if log::ftp_verbose_log(cli) {
        if let FtpFailContext::RetrFile { name } = &ctx {
            if code == 550 || code == 553 {
                log::log_ftp_no_such_file(cli, name);
                return ExitCode::ServerError;
            }
        }
    }
    if !log::ftp_verbose_log(cli) {
        log::log_http_error(cli, Some(raw_url), code, &ftp_reply_text(code, lines));
    } else if !matches!(ctx, FtpFailContext::RetrFile { .. }) {
        let reason = ftp_reply_text(code, lines);
        log::log_http_error(cli, None, code, &reason);
    }
    match code {
        530 | 531 | 532 | 533 => ExitCode::ServerAuthFail,
        421 | 425 | 426 => ExitCode::NetworkFail,
        _ => ExitCode::ServerError,
    }
}

fn default_ftp_pass_for_user(user: &str) -> String {
    if user.eq_ignore_ascii_case("anonymous") {
        return "-wget@".to_string();
    }
    "".to_string()
}

async fn read_reply_line(
    r: &mut BufReader<ReadHalf<TcpStream>>,
) -> Result<(u16, String), ExitCode> {
    let mut line = String::new();
    let n = r
        .read_line(&mut line)
        .await
        .map_err(|_| ExitCode::NetworkFail)?;
    if n == 0 {
        return Err(ExitCode::NetworkFail);
    }
    let t = line.trim_end_matches(['\r', '\n']);
    if t.len() < 3 {
        return Err(ExitCode::ProtocolError);
    }
    let code: u16 = t[0..3].parse().map_err(|_| ExitCode::ProtocolError)?;
    Ok((code, t.to_string()))
}

async fn read_reply(
    r: &mut BufReader<ReadHalf<TcpStream>>,
) -> Result<(u16, Vec<String>), ExitCode> {
    let (code, first) = read_reply_line(r).await?;
    let mut lines = vec![first.clone()];
    if first.as_bytes().get(3) == Some(&b'-') {
        loop {
            let (c2, l2) = read_reply_line(r).await?;
            lines.push(l2.clone());
            if c2 == code && l2.as_bytes().get(3) == Some(&b' ') {
                break;
            }
        }
    }
    Ok((code, lines))
}

async fn cmd(
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    s: &str,
) -> Result<(u16, Vec<String>), ExitCode> {
    w.write_all(s.as_bytes())
        .await
        .map_err(|_| ExitCode::NetworkFail)?;
    w.write_all(b"\r\n")
        .await
        .map_err(|_| ExitCode::NetworkFail)?;
    w.flush().await.map_err(|_| ExitCode::NetworkFail)?;
    read_reply(r).await
}

fn parse_pasv(line: &str) -> Result<SocketAddr, ExitCode> {
    let start = line.find('(').ok_or(ExitCode::ProtocolError)? + 1;
    let end = line[start..].find(')').ok_or(ExitCode::ProtocolError)? + start;
    let nums: Vec<u16> = line[start..end]
        .split(',')
        .map(|x| x.trim().parse::<u16>().map_err(|_| ExitCode::ProtocolError))
        .collect::<Result<Vec<_>, _>>()?;
    if nums.len() != 6 {
        return Err(ExitCode::ProtocolError);
    }
    let ip = std::net::IpAddr::V4(std::net::Ipv4Addr::new(
        nums[0] as u8,
        nums[1] as u8,
        nums[2] as u8,
        nums[3] as u8,
    ));
    let port: u16 = nums[4] * 256 + nums[5];
    Ok(SocketAddr::new(ip, port))
}

async fn open_pasv_data(
    raw_url: &str,
    cli: &Cli,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
) -> Result<TcpStream, ExitCode> {
    let (c, lines) = cmd(w, r, "PASV").await?;
    if c != 227 {
        return Err(ftp_fail(cli, raw_url, c, &lines, FtpFailContext::Generic));
    }
    let addr = parse_pasv(lines.last().unwrap_or(&"".to_string()))?;
    TcpStream::connect(addr)
        .await
        .map_err(|_| ExitCode::NetworkFail)
}

async fn login(
    cli: &Cli,
    host: &str,
    url: &Url,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
) -> Result<(), ExitCode> {
    let raw_url = url.as_str();
    let (c, lines) = read_reply(r).await?;
    if c != 220 {
        return Err(ftp_fail(cli, raw_url, c, &lines, FtpFailContext::Generic));
    }

    let mut user: Option<String> = None;
    let mut pass: Option<String> = None;

    if !url.username().is_empty() {
        user = Some(url.username().to_string());
    } else if let Some(u) = &cli.ftp_user {
        user = Some(u.clone());
    } else if let Some(u) = &cli.user {
        user = Some(u.clone());
    }

    if url.password().is_some() {
        pass = url.password().map(|s| s.to_string());
    } else if let Some(p) = &cli.ftp_password {
        pass = Some(p.clone());
    } else if let Some(p) = &cli.password {
        pass = Some(p.clone());
    }

    if cli.effective_netrc() && (user.is_none() || pass.is_none()) {
        crate::netrc::search_and_apply(host, &mut user, &mut pass, true);
    }

    let user = user.unwrap_or_else(|| "anonymous".to_string());
    let pass = pass.unwrap_or_else(|| default_ftp_pass_for_user(&user));

    log::log_ftp_login_begin(cli, &user);
    let (c1, lines1) = cmd(w, r, &format!("USER {user}")).await?;
    match c1 {
        230 => {
            log::log_ftp_login_ok(cli);
            return Ok(());
        }
        331 => {}
        _ => {
            if log::ftp_verbose_log(cli) {
                log::log_ftp_login_fail(cli, &ftp_reply_text(c1, &lines1));
            }
            return Err(ftp_fail(cli, raw_url, c1, &lines1, FtpFailContext::Generic));
        }
    }
    let (c2, lines2) = cmd(w, r, &format!("PASS {pass}")).await?;
    if c2 == 230 {
        log::log_ftp_login_ok(cli);
        Ok(())
    } else {
        if log::ftp_verbose_log(cli) {
            log::log_ftp_login_fail(cli, &ftp_reply_text(c2, &lines2));
        }
        Err(ftp_fail(cli, raw_url, c2, &lines2, FtpFailContext::Generic))
    }
}

/// GNU verbose：RETR/LIST 前 `SYST` / `PWD`（合并一行）。
async fn ftp_pre_retr_setup(
    cli: &Cli,
    raw_url: &str,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    cmdlog: &mut log::FtpCmdLog,
) -> Result<(), ExitCode> {
    let (c, lines) = cmd(w, r, "SYST").await?;
    if c == 215 || c == 200 {
        cmdlog.cmd(cli, "SYST", "done", true);
    } else if c >= 400 {
        return Err(ftp_fail(cli, raw_url, c, &lines, FtpFailContext::Generic));
    }
    let (c2, lines2) = cmd(w, r, "PWD").await?;
    if c2 == 257 || c2 == 200 {
        cmdlog.cmd(cli, "PWD", "done", true);
    } else if c2 >= 400 {
        return Err(ftp_fail(cli, raw_url, c2, &lines2, FtpFailContext::Generic));
    }
    Ok(())
}

async fn query_file_size_silent(
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    path: &str,
) -> Option<u64> {
    let (c, lines) = cmd(w, r, &format!("SIZE {path}")).await.ok()?;
    if c != 213 {
        return None;
    }
    let line = lines.last()?;
    let size_str = line[3..].trim();
    size_str.parse::<u64>().ok()
}

async fn query_file_size(
    cli: &Cli,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    path: &str,
    cmdlog: &mut log::FtpCmdLog,
) -> Option<u64> {
    let name = ftp_basename(path);
    let (c, lines) = cmd(w, r, &format!("SIZE {path}")).await.ok()?;
    if log::ftp_verbose_log(cli) {
        if c == 213 {
            if let Some(line) = lines.last() {
                let size_str = line[3..].trim();
                if let Ok(size) = size_str.parse::<u64>() {
                    cmdlog.cmd(cli, &format!("SIZE {name}"), &size.to_string(), false);
                    return Some(size);
                }
            }
            cmdlog.cmd(cli, &format!("SIZE {name}"), "done", false);
        } else {
            cmdlog.cmd(cli, &format!("SIZE {name}"), "done", false);
        }
        return None;
    }
    if c != 213 {
        return None;
    }
    let line = lines.last()?;
    let size_str = line[3..].trim();
    size_str.parse::<u64>().ok()
}

async fn copy_data_stream(
    cli: &Cli,
    data: &mut TcpStream,
    writer: &mut (impl AsyncWrite + Unpin),
    total: Option<u64>,
    out_path: &str,
) -> Result<u64, ExitCode> {
    let mut buf = [0u8; 64 * 1024];
    let label = label_from_output_path(out_path);
    let mut prog = DownloadProgress::new(cli, total, label.as_deref());
    let start = Instant::now();
    prog.begin_transfer(cli);
    let mut received = 0u64;
    loop {
        let n = data
            .read(&mut buf)
            .await
            .map_err(|_| ExitCode::NetworkFail)?;
        if n == 0 {
            break;
        }
        writer
            .write_all(&buf[..n])
            .await
            .map_err(|_| ExitCode::IoFail)?;
        received = received.saturating_add(n as u64);
        prog.update(cli, n as u64, start.elapsed().as_secs_f64());
    }
    prog.finish(cli, start.elapsed().as_secs_f64());
    Ok(received)
}

async fn copy_data_stream_sync(
    cli: &Cli,
    data: &mut TcpStream,
    writer: &mut impl std::io::Write,
    total: Option<u64>,
    out_path: &str,
) -> Result<u64, ExitCode> {
    let mut buf = [0u8; 64 * 1024];
    let label = label_from_output_path(out_path);
    let mut prog = DownloadProgress::new(cli, total, label.as_deref());
    let start = Instant::now();
    prog.begin_transfer(cli);
    let mut received = 0u64;
    loop {
        let n = data
            .read(&mut buf)
            .await
            .map_err(|_| ExitCode::NetworkFail)?;
        if n == 0 {
            break;
        }
        writer
            .write_all(&buf[..n])
            .map_err(|_| ExitCode::IoFail)?;
        received = received.saturating_add(n as u64);
        prog.update(cli, n as u64, start.elapsed().as_secs_f64());
    }
    prog.finish(cli, start.elapsed().as_secs_f64());
    Ok(received)
}

async fn retr_file_to_path(
    cli: &Cli,
    raw_url: &str,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    path: &str,
    out_path: &str,
    quick_retr: bool,
) -> Result<Option<u64>, ExitCode> {
    let file_name = ftp_basename(path);
    let retr_ctx = FtpFailContext::RetrFile {
        name: file_name.clone(),
    };
    let mut cmdlog = log::FtpCmdLog::new();

    let total = if quick_retr {
        cmdlog.cwd_not_required(cli);
        let (c_type, lines_type) = cmd(w, r, "TYPE I").await?;
        if c_type >= 400 {
            return Err(ftp_fail(cli, raw_url, c_type, &lines_type, FtpFailContext::Generic));
        }
        query_file_size_silent(w, r, path).await
    } else {
        ftp_pre_retr_setup(cli, raw_url, w, r, &mut cmdlog).await?;
        let (c_type, lines_type) = cmd(w, r, "TYPE I").await?;
        if c_type >= 400 {
            return Err(ftp_fail(cli, raw_url, c_type, &lines_type, FtpFailContext::Generic));
        }
        cmdlog.cmd(cli, "TYPE I", "done", true);
        cmdlog.cwd_not_needed(cli, true);
        query_file_size(cli, w, r, path, &mut cmdlog).await
    };

    let mut data = open_pasv_data(raw_url, cli, w, r).await?;
    cmdlog.cmd(cli, "PASV", "done", true);
    let (c, lines) = cmd(w, r, &format!("RETR {path}")).await?;
    if c != 150 && c != 125 {
        return Err(ftp_fail(cli, raw_url, c, &lines, retr_ctx));
    }
    cmdlog.cmd(cli, &format!("RETR {file_name}"), "done", true);
    cmdlog.flush(cli);

    if log::ftp_verbose_log(cli) && out_path != "-" {
        if let Some(size) = total {
            log::log_ftp_length_unauth(cli, size);
        }
    }

    if out_path == "-" {
        let mut out = std::io::stdout();
        copy_data_stream_sync(cli, &mut data, &mut out, total, out_path).await?;
    } else {
        let mut file = File::create(out_path)
            .await
            .map_err(|_| ExitCode::IoFail)?;
        copy_data_stream(cli, &mut data, &mut file, total, out_path).await?;
    }
    let _ = data.shutdown().await;
    let (c2, lines2) = read_reply(r).await?;
    if c2 != 226 && c2 != 250 {
        return Err(ftp_fail(cli, raw_url, c2, &lines2, retr_ctx));
    }
    Ok(total)
}

/// 解析 Unix `LIST` 行，供 HTML 索引与 `parse_unix_listing` 复用元数据。
struct ListingLineMeta {
    is_dir: bool,
    name: String,
    size: Option<u64>,
    date: String,
}

fn parse_listing_line_meta(line: &str) -> Option<ListingLineMeta> {
    let t = line.trim_matches(['\r', '\n']).trim();
    if t.is_empty() {
        return None;
    }
    let b0 = t.as_bytes().first().copied()?;
    let is_dir = b0 == b'd';
    if !(b0 == b'-' || b0 == b'd' || b0 == b'l') {
        return None;
    }
    let parts: Vec<&str> = t.split_whitespace().collect();
    if parts.len() < 9 {
        return None;
    }
    let size = parts[4].parse::<u64>().ok();
    let date = format!("{} {} {}", parts[5], parts[6], parts[7]);
    let mut name = t.split_whitespace().skip(8).collect::<Vec<_>>().join(" ");
    if let Some((left, _)) = name.split_once(" -> ") {
        name = left.trim().to_string();
    }
    if name.is_empty() || name == "." || name == ".." {
        return None;
    }
    Some(ListingLineMeta {
        is_dir,
        name,
        size,
        date,
    })
}

fn ftp_entry_href(url: &Url, entry_name: &str, is_dir: bool) -> String {
    let base_path = url.path();
    let joined = if base_path.is_empty() || base_path.ends_with('/') {
        format!("{base_path}{entry_name}")
    } else {
        format!("{base_path}/{entry_name}")
    };
    let joined = if is_dir && !joined.ends_with('/') {
        format!("{joined}/")
    } else {
        joined
    };
    let mut u = url.clone();
    u.set_path(&joined);
    u.to_string()
}

/// 由 `.listing` 生成 GNU 风格 HTML 目录索引。
pub fn write_htmlized_index(url: &Url, listing: &[u8], out_path: &str) -> Result<u64, ExitCode> {
    let host = url.host_str().unwrap_or("localhost");
    let port = url.port_or_known_default().unwrap_or(21);
    let title_path = url.path();
    let title = format!("Index of {title_path} on {host}:{port}");

    let mut rows = String::new();
    let text = String::from_utf8_lossy(listing);
    for line in text.lines() {
        let Some(meta) = parse_listing_line_meta(line) else {
            continue;
        };
        let href = ftp_entry_href(url, &meta.name, meta.is_dir);
        let kind = if meta.is_dir { "Directory" } else { "File" };
        let size_suffix = if meta.is_dir {
            String::new()
        } else if let Some(sz) = meta.size {
            format!("  ({sz} bytes)")
        } else {
            String::new()
        };
        rows.push_str(&format!(
            "  {}  {kind:<10}  <a href=\"{href}\">{}</a>{size_suffix}\n",
            meta.date, meta.name
        ));
    }

    let html = format!(
        "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n\
<html>\n<head>\n<title>{title}</title>\n</head>\n<body>\n\
<h1>{title}</h1>\n<hr>\n<pre>\n{rows}</pre>\n</body>\n</html>\n"
    );
    std::fs::write(out_path, html.as_bytes()).map_err(|_| ExitCode::IoFail)?;
    Ok(html.len() as u64)
}

async fn list_dir(
    cli: &Cli,
    raw_url: &str,
    w: &mut WriteHalf<TcpStream>,
    r: &mut BufReader<ReadHalf<TcpStream>>,
    path: &str,
) -> Result<Vec<u8>, ExitCode> {
    let mut cmdlog = log::FtpCmdLog::new();
    ftp_pre_retr_setup(cli, raw_url, w, r, &mut cmdlog).await?;

    let (c_type, lines_type) = cmd(w, r, "TYPE I").await?;
    if c_type >= 400 {
        return Err(ftp_fail(cli, raw_url, c_type, &lines_type, FtpFailContext::Generic));
    }
    cmdlog.cmd(cli, "TYPE I", "done", true);
    cmdlog.cwd_not_needed(cli, true);

    let mut data = open_pasv_data(raw_url, cli, w, r).await?;
    cmdlog.cmd(cli, "PASV", "done", true);
    let list_arg = if path == "/" || path.is_empty() {
        "LIST".to_string()
    } else {
        format!("LIST {path}")
    };
    let (c, lines) = cmd(w, r, &list_arg).await?;
    if c != 150 && c != 125 {
        return Err(ftp_fail(cli, raw_url, c, &lines, FtpFailContext::Generic));
    }
    cmdlog.cmd(cli, "LIST", "done", true);
    cmdlog.flush(cli);

    let mut buf = Vec::new();
    let mut sock_buf = [0u8; 64 * 1024];
    let mut prog = DownloadProgress::new(cli, None, None);
    let start = Instant::now();
    prog.begin_transfer(cli);
    loop {
        let n = data
            .read(&mut sock_buf)
            .await
            .map_err(|_| ExitCode::NetworkFail)?;
        if n == 0 {
            break;
        }
        buf.extend_from_slice(&sock_buf[..n]);
        prog.update(cli, n as u64, start.elapsed().as_secs_f64());
    }
    prog.finish(cli, start.elapsed().as_secs_f64());
    let _ = data.shutdown().await;
    let (c2, lines2) = read_reply(r).await?;
    if c2 != 226 && c2 != 250 {
        return Err(ftp_fail(cli, raw_url, c2, &lines2, FtpFailContext::Generic));
    }
    Ok(buf)
}

fn io_connect_failure_reason(err: &std::io::Error) -> &'static str {
    match err.kind() {
        std::io::ErrorKind::ConnectionRefused => "Connection refused.",
        std::io::ErrorKind::TimedOut => "Connection timed out.",
        std::io::ErrorKind::NotFound => "Name or service not known.",
        _ => "Connection failed.",
    }
}

/// 由 listing 路径推导同目录下的 `.listing` 路径（与 `http.rs` / 递归共用）。
pub fn ftp_listing_path_for_index(index_path: &str) -> String {
    let p = std::path::Path::new(index_path);
    let parent = p.parent().unwrap_or(std::path::Path::new(""));
    if parent.as_os_str().is_empty() {
        ".listing".to_string()
    } else {
        parent.join(".listing").to_string_lossy().to_string()
    }
}

pub async fn fetch_ftp(
    cli: &Cli,
    url: &Url,
    listing_path: &str,
    want_listing: bool,
    index_output_path: Option<&str>,
    quick_retr: bool,
) -> Result<(i32, Option<Vec<u8>>), ExitCode> {
    log::log_url(cli, url.as_str(), None);
    log::log_ftp_output_target(cli, listing_path);

    let host = url.host_str().ok_or(ExitCode::ParseError)?;
    let port = url.port_or_known_default().unwrap_or(21);
    let addr = format!("{host}:{port}");

    let raw_url = url.as_str();
    let stream = match TcpStream::connect(addr).await {
        Ok(s) => {
            log::log_connect_ok(cli, host, port);
            s
        }
        Err(e) => {
            let reason = io_connect_failure_reason(&e);
            if !cli.quiet {
                if cli.effective_verbose() {
                    log::log_connect_failed_verbose(cli, host, port, reason);
                    log::log_giving_up(cli);
                } else {
                    log::log_connect_failed_nv(cli, reason);
                }
            }
            return Err(ExitCode::NetworkFail);
        }
    };
    let (rhalf, mut whalf) = split(stream);
    let mut r = BufReader::new(rhalf);

    login(cli, host, url, &mut whalf, &mut r).await?;

    let path = decode_ftp_path(url.path());
    let mut received = 0u64;
    let mut total = None;
    let listing_bytes = if want_listing {
        let dl_start = Instant::now();
        let data = list_dir(cli, raw_url, &mut whalf, &mut r, &path).await?;
        if listing_path != "-" {
            std::fs::write(listing_path, &data).map_err(|_| ExitCode::IoFail)?;
            received = data.len() as u64;
            log::log_ftp_download_complete(cli, listing_path, received, dl_start.elapsed());
        } else {
            use std::io::Write;
            std::io::stdout()
                .write_all(&data)
                .map_err(|_| ExitCode::IoFail)?;
            received = data.len() as u64;
        }

        if let Some(index_path) = index_output_path {
            if !cli.effective_dont_remove_listing() && listing_path != "-" {
                if std::fs::remove_file(listing_path).is_ok() {
                    log::log_ftp_listing_removed(cli);
                }
            }
            if index_path != "-" {
                let index_len = write_htmlized_index(url, &data, index_path)?;
                log::log_ftp_index_written(cli, index_path, Some(index_len));
            }
        }
        Some(data)
    } else {
        let dl_start = Instant::now();
        total = retr_file_to_path(
            cli,
            raw_url,
            &mut whalf,
            &mut r,
            &path,
            listing_path,
            quick_retr,
        )
        .await?;
        if listing_path != "-" {
            received = std::fs::metadata(listing_path)
                .ok()
                .map(|m| m.len())
                .unwrap_or(0);
            log::log_ftp_download_complete(cli, listing_path, received, dl_start.elapsed());
        }
        None
    };

    let _ = cmd(&mut whalf, &mut r, "QUIT").await;

    if want_listing {
        log::log_transfer_summary(cli, raw_url, listing_path, received, None, 1, true);
    } else {
        log::log_transfer_summary(cli, raw_url, listing_path, received, total, 1, true);
    }
    Ok((ExitCode::Success as i32, listing_bytes))
}

pub fn parse_unix_listing(listing: &[u8]) -> Vec<(String, bool)> {
    let text = String::from_utf8_lossy(listing);
    let mut out: Vec<(String, bool)> = Vec::new();
    for line in text.lines() {
        let t = line.trim_matches(['\r', '\n']).trim();
        if t.is_empty() {
            continue;
        }
        if let Some((name, is_dir)) = parse_unix_ls_line(t) {
            if name == "." || name == ".." {
                continue;
            }
            out.push((name, is_dir));
        }
    }
    out
}

fn parse_unix_ls_line(line: &str) -> Option<(String, bool)> {
    let b0 = line.as_bytes().first().copied()?;
    let is_dir = b0 == b'd';
    let is_file_like = b0 == b'-' || b0 == b'd' || b0 == b'l';
    if !is_file_like {
        return None;
    }

    let mut field = 0u32;
    let mut in_ws = true;
    let mut name_start: Option<usize> = None;
    for (i, ch) in line.char_indices() {
        let ws = ch.is_whitespace();
        if in_ws && !ws {
            field += 1;
            in_ws = false;
            if field == 9 {
                name_start = Some(i);
                break;
            }
        } else if !in_ws && ws {
            in_ws = true;
        }
    }
    let mut name = match name_start {
        Some(i) => line[i..].trim().to_string(),
        None => return None,
    };
    if let Some((left, _right)) = name.split_once(" -> ") {
        name = left.trim().to_string();
    }
    if name.is_empty() {
        return None;
    }
    Some((name, is_dir))
}
