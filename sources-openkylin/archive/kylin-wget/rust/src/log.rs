//! 与 GNU wget 接近的 stderr 日志策略：`-q`、默认 verbose、`-nv`、`--no-verbose`、`--show-progress`、`-S`。
use crate::i18n;

use crate::cli::Cli;
use crate::exit::ExitCode;
use chrono::Local;
use std::fs::File;
use std::io::BufWriter;
use std::io::Write;
use std::sync::{Mutex, OnceLock};
use std::time::Duration;
use url::Url;

static OUTPUT_FILE: OnceLock<Mutex<Option<BufWriter<File>>>> = OnceLock::new();

/// 是否与 GNU `log_init()` 一致：指定 `-o/-a` 时常规消息写入日志文件而非 stderr。
#[inline]
pub(crate) fn has_output_log(cli: &Cli) -> bool {
    cli.log_file.is_some() || cli.append_log_file.is_some()
}

/// 用户可见状态行：无 `-o/-a` 时写 stderr；有则写日志文件（对齐 GNU `logfp = filelogfp`）。
pub fn log_status_line(cli: &Cli, msg: &str) {
    if cli.quiet {
        return;
    }
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = writeln!(w, "{msg}");
        });
    } else {
        eprintln!("{msg}");
    }
}

/// 初始化 `-o/--output-file` 或 `-a/--append-output` 的日志落盘。
/// 与下载内容文件输出无关，仅用于模拟 GNU wget 的非下载日志落盘语义。
pub fn init_output_file(cli: &Cli) -> Result<(), ExitCode> {
    let lock = OUTPUT_FILE.get_or_init(|| Mutex::new(None));
    let mut guard = lock.lock().map_err(|_| ExitCode::IoFail)?;

    // 若未指定任何落盘文件，则保持 None
    if cli.log_file.is_none() && cli.append_log_file.is_none() {
        *guard = None;
        return Ok(());
    }

    // 顺序约束：由 cli.validate 保证不会同时设置 -o 与 -a
    if let Some(ref p) = cli.log_file {
        let f = File::create(p).map_err(|_| ExitCode::IoFail)?;
        *guard = Some(BufWriter::new(f));
        return Ok(());
    }
    if let Some(ref p) = cli.append_log_file {
        let f = std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(p)
            .map_err(|_| ExitCode::IoFail)?;
        *guard = Some(BufWriter::new(f));
        return Ok(());
    }
    *guard = None;
    Ok(())
}

fn with_output_writer<F: FnOnce(&mut dyn Write)>(f: F) {
    let lock = OUTPUT_FILE.get_or_init(|| Mutex::new(None));
    if let Ok(mut guard) = lock.lock() {
        if let Some(w) = guard.as_mut() {
            f(w);
            let _ = w.flush();
        }
    }
}

/// FTP verbose 日志（GNU `ftp.c` 风格，与 HTTP 分流）。
#[inline]
pub fn ftp_verbose_log(cli: &Cli) -> bool {
    cli.effective_verbose() && !cli.quiet
}

fn log_partial(cli: &Cli, msg: &str) {
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = write!(w, "{msg}");
        });
    } else {
        eprint!("{msg}");
        let _ = std::io::stderr().flush();
    }
}

/// GNU：`           => '/path/out'`
pub fn log_ftp_output_target(cli: &Cli, out: &str) {
    if !ftp_verbose_log(cli) || out == "-" {
        return;
    }
    log_status_line(cli, &format!("           => '{out}'"));
}

/// GNU：`Logging in as USER ... Logged in!`（同一行）。
pub fn log_ftp_login_begin(cli: &Cli, user: &str) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_partial(
        cli,
        &i18n::tr_named("Logging in as {user} ...", &[("user", user)]),
    );
}

pub fn log_ftp_login_ok(cli: &Cli) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_partial(cli, i18n::gettext(" Logged in!"));
    log_status_line(cli, "");
}

pub fn log_ftp_login_fail(cli: &Cli, detail: &str) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named("Login failed: {detail}", &[("detail", detail)]),
    );
}

/// GNU verbose：合并 `==> SYST ... done.    ==> PWD ... done.` 等同屏输出。
pub struct FtpCmdLog {
    pending: Option<String>,
}

impl FtpCmdLog {
    pub fn new() -> Self {
        Self { pending: None }
    }

    fn piece_cmd(cmd: &str, suffix: &str) -> String {
        let suf = if suffix == "done" { "done." } else { suffix };
        format!("==> {cmd} ... {suf}")
    }

    fn emit(&mut self, cli: &Cli, piece: String, pair_next: bool) {
        if !ftp_verbose_log(cli) {
            return;
        }
        if pair_next {
            match self.pending.take() {
                Some(prev) => log_status_line(cli, &format!("{prev}    {piece}")),
                None => self.pending = Some(piece),
            }
        } else if let Some(prev) = self.pending.take() {
            log_status_line(cli, &format!("{prev}    {piece}"));
        } else {
            log_status_line(cli, &piece);
        }
    }

    pub fn cmd(&mut self, cli: &Cli, cmd: &str, suffix: &str, pair_next: bool) {
        self.emit(cli, Self::piece_cmd(cmd, suffix), pair_next);
    }

    /// GNU：`==> CWD not needed.`（常与 `TYPE I` 同行）
    pub fn cwd_not_needed(&mut self, cli: &Cli, pair_next: bool) {
        self.emit(cli, i18n::gettext("==> CWD not needed.").to_string(), pair_next);
    }

    /// GNU 递归子文件：`==> CWD not required.`
    pub fn cwd_not_required(&mut self, cli: &Cli) {
        if !ftp_verbose_log(cli) {
            return;
        }
        self.flush(cli);
        log_status_line(cli, i18n::gettext("==> CWD not required."));
    }

    pub fn flush(&mut self, cli: &Cli) {
        if !ftp_verbose_log(cli) {
            return;
        }
        if let Some(p) = self.pending.take() {
            log_status_line(cli, &p);
        }
    }
}

/// 兼容旧调用点（单行、不合并）。
pub fn log_ftp_cmd_done(cli: &Cli, cmd_display: &str, suffix: &str) {
    let mut log = FtpCmdLog::new();
    log.cmd(cli, cmd_display, suffix, false);
    log.flush(cli);
}

pub fn log_ftp_cwd_not_needed(cli: &Cli) {
    let mut log = FtpCmdLog::new();
    log.cwd_not_needed(cli, false);
    log.flush(cli);
}

/// GNU：`Length: 3 (unauthoritative)`
pub fn log_ftp_length_unauth(cli: &Cli, size: u64) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named(
            "Length: {size} (unauthoritative)",
            &[("size", &size.to_string())],
        ),
    );
    log_status_line(cli, "");
}

/// GNU FTP 收尾：`timestamp (rate) - 'file' saved [N]`
pub fn log_ftp_download_complete(
    cli: &Cli,
    out: &str,
    received: u64,
    elapsed: Duration,
) {
    if !ftp_verbose_log(cli) || out == "-" {
        return;
    }
    let secs = elapsed.as_secs_f64().max(0.0001);
    let rate = fmt_rate_mb_per_sec(received as f64 / secs);
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    log_status_line(
        cli,
        &i18n::tr_named(
            "{timestamp} ({rate}) - '{out}' saved [{size}]",
            &[
                ("timestamp", &ts.to_string()),
                ("rate", &rate),
                ("out", out),
                ("size", &received.to_string()),
            ],
        ),
    );
}

/// GNU：`No such file 'name'.`
pub fn log_ftp_no_such_file(cli: &Cli, name: &str) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_status_line(cli, &i18n::tr_named("No such file '{name}'.", &[("name", name)]));
}

/// GNU：`Removed '.listing'.`
pub fn log_ftp_listing_removed(cli: &Cli) {
    if !ftp_verbose_log(cli) {
        return;
    }
    log_status_line(cli, i18n::gettext("Removed '.listing'."));
}

/// GNU verbose：`Wrote HTML-ized index to 'path' [778].`
pub fn log_ftp_index_written(cli: &Cli, path: &str, bytes: Option<u64>) {
    if !ftp_verbose_log(cli) {
        return;
    }
    let msg = match bytes {
        Some(n) => i18n::tr_named(
            "Wrote HTML-ized index to '{path}' [{bytes}].",
            &[("path", path), ("bytes", &n.to_string())],
        ),
        None => i18n::tr_named("Wrote HTML-ized index to '{path}'.", &[("path", path)]),
    };
    log_status_line(cli, &msg);
}

/// GNU `-nv` FTP：`URL: ftp://... [3] -> "out" [1]`（单数字括号）。
pub fn log_nv_transfer_summary_ftp(
    cli: &Cli,
    raw_url: &str,
    out: &str,
    received: u64,
    attempt: i32,
) {
    if cli.quiet || cli.effective_verbose() || out == "-" {
        return;
    }
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    let size = received.to_string();
    log_status_line(
        cli,
        &format!("{ts} URL:{raw_url} [{size}] -> \"{out}\" [{attempt}]"),
    );
}

/// GNU verbose 重试：`Retrying.`（`-nv` 下不输出）。
pub fn log_retry(cli: &Cli) {
    if cli.quiet || !cli.effective_verbose() {
        return;
    }
    log_status_line(cli, i18n::gettext("Retrying."));
}

/// 详细模式：GNU 风格 `--timestamp--  URL`；重试时为 `--timestamp--  (try: N)  URL`。
pub fn log_url(cli: &Cli, raw_url: &str, try_no: Option<i32>) {
    if cli.quiet || !cli.effective_verbose() {
        return;
    }
    let shown = Url::parse(raw_url)
        .map(|u| crate::urls::url_for_user_display(cli, &u))
        .unwrap_or_else(|_| raw_url.to_string());
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    let line = match try_no {
        None => i18n::tr_named(
            "--{timestamp}--  {url}",
            &[("timestamp", &ts.to_string()), ("url", &shown)],
        ),
        Some(n) => i18n::tr_named(
            "--{timestamp}--  (try: {try})  {url}",
            &[
                ("timestamp", &ts.to_string()),
                ("try", &n.to_string()),
                ("url", &shown),
            ],
        ),
    };
    log_status_line(cli, &line);
}

/// GNU verbose：`Connecting to HOST:PORT... connected.`（单行）。
pub fn log_connect_ok(cli: &Cli, host: &str, port: u16) {
    if cli.quiet || !cli.effective_verbose() {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named(
            "Connecting to {host}:{port}... connected.",
            &[("host", host), ("port", &port.to_string())],
        ),
    );
}

/// GNU verbose：`Connecting to HOST:PORT... failed: REASON.`
pub fn log_connect_failed_verbose(cli: &Cli, host: &str, port: u16, reason: &str) {
    if cli.quiet || !cli.effective_verbose() {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named(
            "Connecting to {host}:{port}... failed: {reason}",
            &[("host", host), ("port", &port.to_string()), ("reason", reason)],
        ),
    );
}

/// GNU `-N`：HEAD 判定远端未更新且长度一致时跳过下载。
pub fn log_timestamping_omit_download(cli: &Cli, out: &str) {
    if cli.quiet {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named(
            "File '{out}' not modified on server. Omitting download.",
            &[("out", out)],
        ),
    );
}

/// GNU：HTTPS 证书校验失败（退出码 5）；默认/verbose 均输出，不打印 `wget: failed`。
pub fn log_certificate_verify_failed(cli: &Cli, host: &str, issuer: Option<&str>) {
    if cli.quiet {
        return;
    }
    let issuer = issuer.unwrap_or("unknown");
    log_status_line(
        cli,
        &i18n::tr_named(
            "ERROR: cannot verify {host}'s certificate, issued by '{issuer}':",
            &[("host", host), ("issuer", issuer)],
        ),
    );
    log_status_line(
        cli,
        i18n::gettext("  Unable to locally verify the issuer's authority."),
    );
    log_status_line(
        cli,
        &i18n::tr_named(
            "To connect to {host} insecurely, use `--no-check-certificate'.",
            &[("host", host)],
        ),
    );
}

/// GNU `-nv`：`failed: REASON.`
pub fn log_connect_failed_nv(cli: &Cli, reason: &str) {
    if cli.quiet || cli.effective_verbose() {
        return;
    }
    log_status_line(cli, &i18n::tr_named("failed: {reason}", &[("reason", reason)]));
}

/// GNU verbose：多次重试耗尽后 `Giving up.`
pub fn log_giving_up(cli: &Cli) {
    if cli.quiet || !cli.effective_verbose() {
        return;
    }
    log_status_line(cli, i18n::gettext("Giving up."));
}

/// `-nv` 失败前：`http://host/path:`
pub fn log_nv_url_colon(cli: &Cli, raw_url: &str) {
    if cli.quiet || cli.effective_verbose() {
        return;
    }
    log_status_line(cli, &format!("{raw_url}:"));
}

fn format_nv_transfer_size(received: u64, total: Option<u64>) -> String {
    match total {
        Some(t) => format!("{received}/{t}"),
        None => received.to_string(),
    }
}

/// GNU `-nv` 成功：`timestamp URL:url [n/n] -> "out" [try]`
pub fn log_nv_transfer_summary(
    cli: &Cli,
    raw_url: &str,
    out: &str,
    received: u64,
    total: Option<u64>,
    attempt: i32,
) {
    if cli.quiet || cli.effective_verbose() || out == "-" {
        return;
    }
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    let size = format_nv_transfer_size(received, total);
    log_status_line(
        cli,
        &format!("{ts} URL:{raw_url} [{size}] -> \"{out}\" [{attempt}]"),
    );
}

/// 非 verbose 成功摘要；`-o/-a` 时同步写入日志文件。
pub fn log_transfer_summary(
    cli: &Cli,
    raw_url: &str,
    out: &str,
    received: u64,
    total: Option<u64>,
    attempt: i32,
    is_ftp: bool,
) {
    if cli.quiet || out == "-" {
        return;
    }
    if !cli.effective_verbose() {
        if is_ftp {
            log_nv_transfer_summary_ftp(cli, raw_url, out, received, attempt);
        } else {
            log_nv_transfer_summary(cli, raw_url, out, received, total, attempt);
        }
    }
    if has_output_log(cli) {
        log_output_saved(cli, raw_url, out, received, total, attempt);
    }
}

fn http_version_string(resp: &reqwest::Response) -> &'static str {
    use reqwest::Version;
    match resp.version() {
        Version::HTTP_09 => "HTTP/0.9",
        Version::HTTP_10 => "HTTP/1.0",
        Version::HTTP_11 => "HTTP/1.1",
        Version::HTTP_2 => "HTTP/2.0",
        Version::HTTP_3 => "HTTP/3.0",
        _ => "HTTP/1.1",
    }
}

/// 收到响应后：verbose 打一行状态；`-S` 再打完整响应头（对齐 GNU）。
pub fn log_http_transfer_response(cli: &Cli, resp: &reqwest::Response) -> Result<(), ExitCode> {
    if cli.quiet {
        return Ok(());
    }
    let ver = http_version_string(resp);
    let st = resp.status();
    let code = st.as_u16();
    let reason = st.canonical_reason().unwrap_or("");

    if cli.server_response {
        log_status_line(cli, i18n::gettext("HTTP request sent, awaiting response..."));
        log_status_line(cli, &format!("  {ver} {code} {reason}"));
        for (k, v) in resp.headers() {
            if let Ok(vs) = v.to_str() {
                log_status_line(cli, &format!("  {}: {}", k.as_str(), vs));
            }
        }
        log_status_line(cli, "");
    } else if cli.effective_verbose() {
        log_status_line(
            cli,
            &i18n::tr_named(
                "HTTP request sent, awaiting response... {code} {reason}",
                &[
                    ("code", &code.to_string()),
                    ("reason", reason),
                ],
            ),
        );
    }
    Ok(())
}

/// GNU：`HTTP request sent, awaiting response... 302 Found` + `Location: ... [following]`（各一行）。
pub fn log_http_redirect_not_followed(
    cli: &Cli,
    status_u16: u16,
    reason: &str,
    location: Option<&str>,
) {
    if cli.quiet {
        return;
    }
    let reason = if reason.is_empty() { "Found" } else { reason };
    log_status_line(
        cli,
        &i18n::tr_named(
            "HTTP request sent, awaiting response... {code} {reason}",
            &[("code", &status_u16.to_string()), ("reason", reason)],
        ),
    );
    if let Some(loc) = location {
        log_status_line(
            cli,
            &i18n::tr_named("Location: {location} [following]", &[("location", loc)]),
        );
    }
}

/// GNU：`0 redirections exceeded.`（`--max-redirect=N` 中的 N）。
pub fn log_redirects_exceeded(cli: &Cli, limit: u32) {
    if cli.quiet {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named("{limit} redirections exceeded.", &[("limit", &limit.to_string())]),
    );
}

/// GNU：`Selected authentication scheme: Basic realm="..."`（401 挑战后、重试前）。
pub fn log_http_auth_scheme_selected(cli: &Cli, scheme: &str) {
    if cli.quiet {
        return;
    }
    log_status_line(
        cli,
        &i18n::tr_named(
            "Selected authentication scheme: {scheme}",
            &[("scheme", scheme)],
        ),
    );
}

/// GNU：`Username/password authentication failed.`（退出码 6，仍创建空 `-O` 文件）。
pub fn log_http_auth_failed(cli: &Cli) {
    if cli.quiet {
        return;
    }
    log_status_line(
        cli,
        i18n::gettext("Username/password authentication failed."),
    );
}

/// GNU：`2026-05-22 15:44:53 ERROR 404: Not Found.`；`-nv` 前先打 `URL:` 行。
pub fn log_http_error(cli: &Cli, raw_url: Option<&str>, status_u16: u16, reason: &str) {
    if cli.quiet {
        return;
    }
    if let Some(url) = raw_url {
        log_nv_url_colon(cli, url);
    }
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    let detail = if reason.is_empty() {
        i18n::tr_named("ERROR {code}:.", &[("code", &status_u16.to_string())])
    } else {
        i18n::tr_named(
            "ERROR {code}: {reason}.",
            &[("code", &status_u16.to_string()), ("reason", reason)],
        )
    };
    log_status_line(cli, &format!("{ts} {detail}"));
}

/// GNU verbose 收尾：`timestamp (rate) - 'file' saved [received/total]`
pub fn log_download_complete(
    cli: &Cli,
    out: &str,
    received: u64,
    total: Option<u64>,
    elapsed: Duration,
) {
    if cli.quiet || !cli.effective_verbose() || out == "-" {
        return;
    }
    let secs = elapsed.as_secs_f64().max(0.0001);
    let rate = fmt_rate_mb_per_sec(received as f64 / secs);
    let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
    let total_s = total
        .unwrap_or(received)
        .to_string();
    log_status_line(
        cli,
        &i18n::tr_named(
            "{timestamp} ({rate}) - '{out}' saved [{received}/{total}]",
            &[
                ("timestamp", &ts.to_string()),
                ("rate", &rate),
                ("out", out),
                ("received", &received.to_string()),
                ("total", &total_s),
            ],
        ),
    );
}

/// `-o/--output-file`（或 `-a`）：GNU wget 的非下载落盘行。
pub fn log_output_saved(
    cli: &Cli,
    raw_url: &str,
    out: &str,
    received: u64,
    total: Option<u64>,
    attempt: i32,
) {
    if cli.quiet {
        return;
    }
    with_output_writer(|w| {
        let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
        let size = format_nv_transfer_size(received, total);
        let _ = writeln!(
            w,
            "{ts} URL:{raw_url} [{size}] -> \"{out}\" [{attempt}]"
        );
    });
}

/// `-o/--output-file`（或 `-a`）：GNU wget 错误落盘。
pub fn log_output_error(cli: &Cli, raw_url: &str, status_u16: u16, reason: &str) {
    if cli.quiet {
        return;
    }
    with_output_writer(|w| {
        let ts = Local::now().format("%Y-%m-%d %H:%M:%S");
        let _ = writeln!(w, "{}:", raw_url);
        let _ = writeln!(
            w,
            "{} ERROR {}: {}.",
            ts,
            status_u16,
            if reason.is_empty() { "" } else { reason }
        );
    });
}

/// dot 进度条：无换行片段（对齐 GNU `logputs`）。
pub fn progress_write_partial(cli: &Cli, s: &str) {
    if cli.quiet || !cli.effective_show_progress() {
        return;
    }
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = write!(w, "{s}");
        });
    } else {
        eprint!("{s}");
        let _ = std::io::stderr().flush();
    }
}

/// dot 进度条：带换行（用于收尾空行等）。
pub fn progress_write_line(cli: &Cli, s: &str) {
    if cli.quiet || !cli.effective_show_progress() {
        return;
    }
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = writeln!(w, "{s}");
        });
    } else {
        eprintln!("{s}");
    }
}

/// `--show-progress` / 默认 verbose：在已知 Content-Length 时输出 `\r N%`（约每 5% 刷新）。
pub fn progress_render(cli: &Cli, received: u64, total: Option<u64>) {
    if !cli.effective_show_progress() || cli.quiet {
        return;
    }
    let Some(t) = total else {
        return;
    };
    if t == 0 {
        return;
    }
    let p = (received.saturating_mul(100) / t).min(100);
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = write!(w, "\r {:>3}%", p);
        });
    } else {
        eprint!("\r {:>3}%", p);
        let _ = std::io::stderr().flush();
    }
}

/// 结束进度行（换行，避免与后续 `wget:` 糊在同一行）。
pub fn progress_finish(cli: &Cli) {
    if !cli.effective_show_progress() || cli.quiet {
        return;
    }
    log_status_line(cli, "");
}

fn fmt_bytes_iec(bytes: u64) -> String {
    const KB: f64 = 1024.0;
    const MB: f64 = KB * 1024.0;
    const GB: f64 = MB * 1024.0;
    let b = bytes as f64;
    if b < KB {
        format!("{}B", bytes)
    } else if b < MB {
        format!("{:.2}K", b / KB)
    } else if b < GB {
        format!("{:.2}M", b / MB)
    } else {
        format!("{:.2}G", b / GB)
    }
}

fn fmt_rate_mb_per_sec(bytes_per_sec: f64) -> String {
    // 尽量包含 `MB/s` 字符串，便于用户和回归观感/断言。
    const MB: f64 = 1024.0 * 1024.0;
    const KB: f64 = 1024.0;
    if bytes_per_sec >= MB {
        format!("{:.2} MB/s", bytes_per_sec / MB)
    } else if bytes_per_sec >= KB {
        format!("{:.2} KB/s", bytes_per_sec / KB)
    } else {
        format!("{:.0} B/s", bytes_per_sec)
    }
}

/// GNU `Length` 行仅显示主类型（去掉 `; charset=utf-8` 等参数）。
pub fn content_type_for_length(raw: &str) -> &str {
    raw.split(';').next().unwrap_or(raw).trim()
}

/// GNU wget：`Length: 11 [text/plain]`；大文件可带 `(2.9G)` 辅助可读。
pub fn log_length_known(cli: &Cli, out: &str, total: u64, content_type: &str) {
    let content_type = content_type_for_length(content_type);
    if cli.quiet || !cli.effective_show_progress() || out == "-" {
        return;
    }
    const MIB: u64 = 1024 * 1024;
    let length_line = if total >= MIB {
        let human = fmt_bytes_iec(total);
        i18n::tr_named(
            "Length: {size} ({human}) [{content_type}]",
            &[
                ("size", &total.to_string()),
                ("human", &human),
                ("content_type", content_type),
            ],
        )
    } else {
        i18n::tr_named(
            "Length: {size} [{content_type}]",
            &[
                ("size", &total.to_string()),
                ("content_type", content_type),
            ],
        )
    };
    log_status_line(cli, &length_line);
    log_status_line(cli, &i18n::tr_named("Saving to: '{out}'", &[("out", out)]));
    log_status_line(cli, "");
}

/// GNU wget 风格：当没有 `Content-Length` 时，打印 `Length: unspecified ...` + `Saving to: ...`
pub fn log_length_unspecified(cli: &Cli, out: &str, content_type: &str) {
    if cli.quiet || !cli.effective_show_progress() || out == "-" {
        return;
    }
    let content_type = content_type_for_length(content_type);
    // 与 GNU 输出的关键子串对齐：`Length: unspecified`（TC70 等对照测试依赖）
    log_status_line(
        cli,
        &i18n::tr_named(
            "Length: unspecified [{content_type}]",
            &[("content_type", content_type)],
        ),
    );
    log_status_line(cli, &i18n::tr_named("Saving to: '{out}'", &[("out", out)]));
    log_status_line(cli, "");
}

/// `Content-Length` 未知时：用一行动态文本模拟进度（包含 MB/s 便于观感）。
pub fn progress_render_unknown(cli: &Cli, received: u64, speed_bps: f64) {
    if cli.quiet || !cli.effective_show_progress() {
        return;
    }
    let r = fmt_bytes_iec(received);
    let rate = fmt_rate_mb_per_sec(speed_bps);
    if has_output_log(cli) {
        with_output_writer(|w| {
            let _ = write!(w, "\r <=> {} ({})", r, rate);
        });
    } else {
        let _ = std::io::stderr().flush();
        eprint!("\r <=> {} ({})", r, rate);
        let _ = std::io::stderr().flush();
    }
}

pub fn progress_finish_unknown(cli: &Cli, received: u64, elapsed: Duration) {
    if cli.quiet || !cli.effective_show_progress() {
        return;
    }
    let secs = elapsed.as_secs_f64().max(0.0001);
    let speed = received as f64 / secs;
    let r = fmt_bytes_iec(received);
    let rate = fmt_rate_mb_per_sec(speed);
    log_status_line(
        cli,
        &i18n::tr_named(
            "\n{received} downloaded in {secs}s ({rate})",
            &[
                ("received", &r),
                ("secs", &format!("{secs:.1}")),
                ("rate", &rate),
            ],
        ),
    );
}

