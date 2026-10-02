use crate::auth;
use crate::backups;
use crate::cli::Cli;
use crate::client::{self, classify_reqwest_error};
use crate::content_disp;
use crate::exit::ExitCode;
use crate::log;
use crate::noclobber;
use crate::output::{OutputTarget, open_output};
use crate::request;
use crate::resume::{ResumePlan, plan_resume};
use crate::{dirstruct, pathgen};
use reqwest::StatusCode;
use reqwest::header::CONTENT_LENGTH;
use reqwest::header::CONTENT_TYPE;
use reqwest::header::IF_MODIFIED_SINCE;
use reqwest::header::LAST_MODIFIED;
use reqwest::header::LOCATION;
use reqwest::Method;
use std::io::Write;
use std::time::Duration;
use std::time::SystemTime;
use url::Url;

fn next_available_numbered_name(base: &str) -> String {
    // 对齐 GNU 默认：X 已存在 => X.1 / X.2 / ...
    for i in 1u32.. {
        let cand = format!("{base}.{i}");
        if !std::path::Path::new(&cand).exists() {
            return cand;
        }
    }
    unreachable!("u32 loop should always find an available name");
}

fn effective_overwrite_allowed_by_clobber(cli: &Cli) -> bool {
    // GNU 默认：文件存在时不覆盖，而是使用 .1/.2/...（即 overwrite=false）。
    // `--clobber[=WHEN]` 允许覆盖；`--clobber=off` 明确禁止覆盖；与 `--no-clobber` 通过 argv 顺序 last-wins。
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    for a in &args {
        if a == "--no-clobber" {
            last = Some(false);
            continue;
        }
        if a == "--clobber" {
            last = Some(true);
            continue;
        }
        if let Some(v) = a.strip_prefix("--clobber=") {
            let v = v.trim();
            let allow =
                !(v.eq_ignore_ascii_case("off") || v == "0" || v.eq_ignore_ascii_case("false"));
            last = Some(allow);
        }
    }
    if let Some(v) = last {
        return v;
    }
    // 无显式参数时：保持 “不覆盖” 的默认。
    if let Some(v) = &cli.clobber {
        let v = v.trim();
        return !(v.eq_ignore_ascii_case("off") || v == "0" || v.eq_ignore_ascii_case("false"));
    }
    false
}

fn default_output_name(cli: &Cli, url: &Url) -> String {
    let path = url.path();
    if path.is_empty() || path.ends_with('/') {
        return cli.effective_default_page().to_string();
    }
    let last = path.rsplit('/').find(|s| !s.is_empty()).unwrap_or("");
    if last.is_empty() {
        cli.effective_default_page().to_string()
    } else {
        last.to_string()
    }
}

/// GNU：下载失败（含 TLS/连接错误、4xx/5xx 等）且指定了本地输出路径时，仍创建空文件（`-` 除外）。
#[inline]
fn http_fail<T>(out: &str, ec: ExitCode) -> Result<T, ExitCode> {
    if out != "-" {
        let _ = touch_empty_output(out);
    }
    Err(ec)
}

/// GNU：重定向被拒绝时仍创建 `-O` 指定的空文件。
fn touch_empty_output(path: &str) -> Result<(), ExitCode> {
    if path == "-" {
        return Ok(());
    }
    ensure_parent_dir_for_path(path)?;
    std::fs::File::create(path).map_err(|_| ExitCode::IoFail)?;
    Ok(())
}

fn ensure_parent_dir_for_path(path: &str) -> Result<(), ExitCode> {
    if path == "-" {
        return Ok(());
    }
    let p = std::path::Path::new(path);
    let Some(parent) = p.parent() else {
        return Ok(());
    };
    if parent.as_os_str().is_empty() {
        return Ok(());
    }
    std::fs::create_dir_all(parent).map_err(|_| ExitCode::IoFail)?;
    Ok(())
}

/// GNU `--no-clobber` 跳过已存在文件时的单次退出码：
/// - 显式 `-O`（含 `-O FILE -i` 多 URL 共用一个输出）→ 1
/// - URL 默认本地名 → 0
fn noclobber_skip_exit_code(explicit_output: bool) -> i32 {
    if explicit_output {
        ExitCode::GenericError as i32
    } else {
        ExitCode::Success as i32
    }
}

fn is_html_content_type(headers: &reqwest::header::HeaderMap) -> bool {
    let Some(v) = headers.get(CONTENT_TYPE) else {
        return false;
    };
    let Ok(s) = v.to_str() else {
        return false;
    };
    let low = s.to_ascii_lowercase();
    low.starts_with("text/html") || low.starts_with("application/xhtml+xml")
}

fn is_css_content_type(headers: &reqwest::header::HeaderMap) -> bool {
    let Some(v) = headers.get(CONTENT_TYPE) else {
        return false;
    };
    let Ok(s) = v.to_str() else {
        return false;
    };
    s.to_ascii_lowercase().starts_with("text/css")
}

fn ends_with_html_ext(name: &str) -> bool {
    let low = name.to_ascii_lowercase();
    low.ends_with(".html") || low.ends_with(".htm")
}

fn ends_with_css_ext(name: &str) -> bool {
    name.to_ascii_lowercase().ends_with(".css")
}

fn maybe_adjust_extension(cli: &Cli, out: &mut String, headers: &reqwest::header::HeaderMap) {
    if !cli.adjust_extension {
        return;
    }
    if out == "-" {
        return;
    }
    if is_html_content_type(headers) {
        if !ends_with_html_ext(out) {
            out.push_str(".html");
        }
    } else if is_css_content_type(headers) {
        if !ends_with_css_ext(out) {
            out.push_str(".css");
        }
    }
}

fn http_date_from_system_time(t: SystemTime) -> Option<String> {
    let dt: chrono::DateTime<chrono::Utc> = t.into();
    // IMF-fixdate: Sun, 06 Nov 1994 08:49:37 GMT
    Some(dt.format("%a, %d %b %Y %H:%M:%S GMT").to_string())
}

fn remote_content_length(headers: &reqwest::header::HeaderMap) -> Option<u64> {
    headers
        .get(CONTENT_LENGTH)
        .and_then(|v| v.to_str().ok())
        .and_then(|s| s.parse().ok())
}

/// GNU：长度不一致则始终重新下载（与 Last-Modified 无关）。
fn timestamping_lengths_match(local_size: u64, headers: &reqwest::header::HeaderMap) -> bool {
    match remote_content_length(headers) {
        Some(remote) => local_size == remote,
        None => true,
    }
}

/// GNU HTTP `-N`：HEAD 后若远端不新于本地且 Content-Length 一致则跳过 GET。
fn timestamping_should_omit_after_head(
    local_mtime: SystemTime,
    local_size: u64,
    headers: &reqwest::header::HeaderMap,
) -> bool {
    let Some(remote_lm) = maybe_last_modified(headers) else {
        return false;
    };
    if remote_lm > local_mtime {
        return false;
    }
    timestamping_lengths_match(local_size, headers)
}

fn wants_timestamping_head(cli: &Cli, out_path: &str, spec: &request::RequestSpec) -> bool {
    if !cli.effective_timestamping() || cli.spider || out_path == "-" {
        return false;
    }
    if spec.method != Method::GET || spec.body.is_some() {
        return false;
    }
    if cli.resume || cli.start_pos.is_some() {
        return false;
    }
    std::path::Path::new(out_path).is_file()
}

fn maybe_if_modified_since(
    cli: &Cli,
    out_path: &str,
    spec: &request::RequestSpec,
) -> Option<String> {
    if !cli.effective_timestamping() {
        return None;
    }
    if cli.spider {
        return None;
    }
    if out_path == "-" {
        return None;
    }
    // Timestamping 子集：只对 GET 且无 body 生效。
    if spec.method != reqwest::Method::GET || spec.body.is_some() {
        return None;
    }
    // 与 Range/续传/起始偏移互斥（GNU 里也会让 timestamping 失去意义或被覆盖）。
    if cli.resume || cli.start_pos.is_some() {
        return None;
    }
    let meta = std::fs::metadata(out_path).ok()?;
    let modified = meta.modified().ok()?;
    http_date_from_system_time(modified)
}

fn parse_http_date(s: &str) -> Option<SystemTime> {
    let dt = chrono::DateTime::parse_from_rfc2822(s).ok()?;
    let utc = dt.with_timezone(&chrono::Utc);
    let secs = utc.timestamp();
    if secs < 0 {
        return None;
    }
    Some(SystemTime::UNIX_EPOCH + Duration::from_secs(secs as u64))
}

fn maybe_last_modified(headers: &reqwest::header::HeaderMap) -> Option<SystemTime> {
    let v = headers.get(LAST_MODIFIED)?.to_str().ok()?;
    parse_http_date(v)
}

fn retry_on_http_error_includes(spec: &str, status_code: u16) -> bool {
    // GNU: `--retry-on-http-error=ERRORS` accepts comma-separated numeric HTTP status codes.
    for tok in spec.split(',') {
        let t = tok.trim();
        if t.is_empty() {
            continue;
        }
        if let Ok(n) = t.parse::<u16>() {
            if n == status_code {
                return true;
            }
        }
    }
    false
}

fn is_connection_refused_error(err: &reqwest::Error) -> bool {
    // 优先从 error chain 中判断具体的 io::ErrorKind::ConnectionRefused。
    let mut cur: &(dyn std::error::Error + 'static) = err;
    loop {
        if let Some(ioe) = cur.downcast_ref::<std::io::Error>() {
            return ioe.kind() == std::io::ErrorKind::ConnectionRefused;
        }
        match cur.source() {
            Some(next) => cur = next,
            None => break,
        }
    }

    // 兜底：按文本匹配，保证本地测试场景可判定。
    err.to_string()
        .to_ascii_lowercase()
        .contains("connection refused")
}

fn log_reqwest_connect_failed(
    cli: &Cli,
    host: &str,
    port: u16,
    e: &reqwest::Error,
    giving_up: bool,
) {
    let msg = client::reqwest_error_summary(e);
    if cli.quiet {
        return;
    }
    if cli.effective_verbose() {
        log::log_connect_failed_verbose(cli, host, port, &msg);
        if giving_up {
            log::log_giving_up(cli);
        }
    } else {
        log::log_connect_failed_nv(cli, &msg);
    }
}

fn log_reqwest_connect_failure(
    cli: &Cli,
    host: &str,
    port: u16,
    e: &reqwest::Error,
    giving_up: bool,
    out: &str,
    issuer: Option<String>,
) -> ExitCode {
    let ec = classify_reqwest_error(e);
    if !cli.quiet {
        if ec == ExitCode::SslAuthFail {
            // GNU：仅 verbose 输出 Connecting…connected；`-nv` 只打证书错误三行（TC73b）。
            log::log_connect_ok(cli, host, port);
            log::log_certificate_verify_failed(cli, host, issuer.as_deref());
        } else {
            log_reqwest_connect_failed(cli, host, port, e, giving_up);
        }
    }
    if out != "-" {
        let _ = touch_empty_output(out);
    }
    ec
}

async fn resolve_tls_failure_issuer(host: &str, port: u16, e: &reqwest::Error) -> Option<String> {
    if classify_reqwest_error(e) != ExitCode::SslAuthFail {
        return None;
    }
    crate::tls_peer::fetch_peer_certificate_issuer(host, port).await
}

fn is_host_error(err: &reqwest::Error) -> bool {
    // DNS / host resolution failures（GNU 中会落到 HOSTERR 类型）。
    // 本项目用本地回归 URL（nonexistent.invalid）稳定触发。
    // 通过错误链逐层匹配关键词；reqwest 顶层 error 文本通常过于抽象。
    let mut cur: &(dyn std::error::Error + 'static) = err;
    loop {
        let s = cur.to_string().to_ascii_lowercase();
        if s.contains("dns error")
            || s.contains("failed to lookup address")
            || s.contains("nodename nor servname provided")
            || s.contains("name or service not known")
            || s.contains("temporary failure in name resolution")
            || s.contains("failed to resolve")
        {
            return true;
        }
        match cur.source() {
            Some(next) => cur = next,
            None => break,
        }
    }

    false
}

pub async fn fetch_one(
    cli: &Cli,
    raw_url: &str,
    url_output: Option<&str>,
    existed_at_start: &std::collections::HashSet<String>,
) -> Result<(i32, bool), ExitCode> {
    let (code, _out, _effective_url, _body, _hdrs, retrieved) =
        fetch_one_with_body(cli, raw_url, None, false, url_output, existed_at_start).await?;
    Ok((code, retrieved))
}

/// 下载一个 URL；可选覆盖输出文件名，并在需要时捕获 HTML body（用于递归抽链）。
///
/// 返回：(退出码, 实际输出路径, 最终有效 URL（含重定向后）, 捕获到的 body（仅在 HTML/CSS 且 capture_html=true 时 Some）, 响应头（同上）)
pub async fn fetch_one_with_body(
    cli: &Cli,
    raw_url: &str,
    output_override: Option<&str>,
    capture_html: bool,
    url_output: Option<&str>,
    existed_at_start: &std::collections::HashSet<String>,
) -> Result<
    (
        i32,
        String,
        Url,
        Option<Vec<u8>>,
        Option<reqwest::header::HeaderMap>,
        bool,
    ),
    ExitCode,
> {
    let parsed = crate::urls::parse_url_with_iri_policy(cli, raw_url)?;

    if parsed.scheme() == "ftp" {
        let want_listing = parsed.path().is_empty() || parsed.path().ends_with('/');
        let index_out = if let Some(p) = output_override {
            p.to_string()
        } else if let Some(o) = url_output {
            o.to_string()
        } else {
            let dirstruct_on = dirstruct::effective_dirstruct(cli, false);
            pathgen::output_path_from_url(cli, &parsed, dirstruct_on)
        };
        let listing_out = if want_listing {
            crate::ftp::ftp_listing_path_for_index(&index_out)
        } else {
            index_out.clone()
        };
        if listing_out != "-" {
            std::fs::create_dir_all(
                std::path::Path::new(&listing_out)
                    .parent()
                    .unwrap_or(std::path::Path::new("")),
            )
            .ok();
        }
        if index_out != "-" && want_listing {
            std::fs::create_dir_all(
                std::path::Path::new(&index_out)
                    .parent()
                    .unwrap_or(std::path::Path::new("")),
            )
            .ok();
        }
        let index_arg = if want_listing {
            Some(index_out.as_str())
        } else {
            None
        };
        let (code, _) = crate::ftp::fetch_ftp(
            cli,
            &parsed,
            &listing_out,
            want_listing,
            index_arg,
            false,
        )
        .await?;
        let ret_out = if want_listing {
            index_out
        } else {
            listing_out
        };
        return Ok((code, ret_out, parsed, None, None, code == ExitCode::Success as i32));
    }

    // HSTS 可能会把 http:// 重写成 https://（按 GNU hsts.c 语义）。
    let url = crate::hsts::maybe_rewrite_url(cli, &parsed);

    crate::dns_family::enforce_literal_host_policy(cli, &url)?;

    let url_default = default_output_name(cli, &url);
    let output_doc = output_override.or(url_output);
    let explicit_output = output_doc.is_some();
    let mut out = if let Some(p) = output_doc {
        p.to_string()
    } else {
        url_default.clone()
    };
    let out_from_url_default = output_doc.is_none();
    if out_from_url_default {
        // 非递归：默认不建目录树；`-x/--force-directories` 可显式开启。
        let dirstruct_on = dirstruct::effective_dirstruct(cli, false);
        out = pathgen::output_path_from_url(cli, &url, dirstruct_on);
    }

    // `-c` / `-O` 的续传键：无 `-O` 时用 URL 默认本地名（与 Content-Disposition 最终落盘名可不同）
    let resume_path = if explicit_output {
        out.clone()
    } else {
        url_default.clone()
    };

    let out_known_before_fetch = explicit_output || cli.resume || !cli.content_disposition;

    let no_clobber = noclobber::effective_noclobber(cli);

    let nc_skip_allowed = std::path::Path::new(&out).exists()
        && (existed_at_start.contains(&out) || !explicit_output);
    if no_clobber
        && !cli.resume
        && out != "-"
        && out_known_before_fetch
        && nc_skip_allowed
    {
        if !cli.quiet {
            log::log_status_line(
                cli,
                &crate::i18n::tr_named(
                    "File '{out}' already there; not retrieving.",
                    &[("out", &out)],
                ),
            );
        }
        let code = noclobber_skip_exit_code(explicit_output);
        return Ok((code, out.clone(), url.clone(), None, None, false));
    }

    let max_redir = cli.max_redirect.unwrap_or(20) as usize;

    let spec = request::prepare_request(cli)?;
    let mut rp = if spec.body.is_none() {
        plan_resume(cli, &resume_path)
    } else {
        ResumePlan {
            start_from: 0,
            want_range: false,
        }
    };

    // `--spider`：只检查资源可用性，不做下载/续传细节（避免 Range）。
    if cli.spider {
        rp.start_from = 0;
        rp.want_range = false;
    }

    let max_attempts: i32 = match cli.tries {
        None => 1,
        Some(0) => 50, // wget's "0 unlimits" - cap to avoid hanging forever
        Some(n) => n.max(1),
    };

    let host = url.host_str().unwrap_or("localhost");
    let port = url.port_or_known_default().unwrap_or(80);

    let creds = auth::http_credentials_for_url(cli, &url);
    let sent_ims = maybe_if_modified_since(cli, &out, &spec).is_some();
    let (local_size, local_mtime) = if sent_ims && out != "-" {
        let meta = std::fs::metadata(&out).ok();
        let sz = meta.as_ref().map(|m| m.len());
        let mt = meta.and_then(|m| m.modified().ok());
        (sz, mt)
    } else {
        (None, None)
    };
    for attempt in 1..=max_attempts {
        let try_no = if attempt > 1 { Some(attempt) } else { None };
        log::log_url(cli, url.as_str(), try_no);

        let client = client::build_client(cli, &url, max_redir).map_err(|ec| {
            if out != "-" {
                let _ = touch_empty_output(&out);
            }
            ec
        })?;
        let preempt = cli.auth_no_challenge && creds.is_some();
        let basic_first = if preempt {
            creds.as_ref().map(|(u, p)| (u.as_str(), p.as_str()))
        } else {
            None
        };

        if attempt == 1 && wants_timestamping_head(cli, &out, &spec) {
            let head_spec = request::RequestSpec {
                method: Method::HEAD,
                body: None,
            };
            let head_rb = request::build_request(&client, url.clone(), &rp, &head_spec, basic_first)?;
            if let Ok(head_resp) = client::send_with_read_idle(cli, head_rb).await {
                if let Ok(meta) = std::fs::metadata(&out) {
                    if let Ok(local_mt) = meta.modified() {
                        let ls = meta.len();
                        if timestamping_should_omit_after_head(local_mt, ls, head_resp.headers()) {
                            let _ = head_resp.bytes().await;
                            if !cli.quiet {
                                log::log_timestamping_omit_download(cli, &out);
                            }
                            return Ok((
                                ExitCode::Success as i32,
                                out.clone(),
                                url.clone(),
                                None,
                                None,
                                false,
                            ));
                        }
                    }
                }
                let _ = head_resp.bytes().await;
            }
        }

        let mut req = request::build_request(&client, url.clone(), &rp, &spec, basic_first)?;
        if let Some(ims) = maybe_if_modified_since(cli, &out, &spec) {
            req = req.header(IF_MODIFIED_SINCE, ims);
        }

        let mut resp = match client::send_with_read_idle(cli, req).await {
            Ok(r) => r,
            Err(client::SendError::ReadIdle) => {
                if attempt < max_attempts {
                    log::log_connect_failed_nv(cli, "Connection timed out.");
                    log::log_retry(cli);
                    continue;
                }
                log::log_connect_failed_nv(cli, "Connection timed out.");
                return http_fail(&out, ExitCode::NetworkFail);
            }
            Err(client::SendError::Transport(e)) => {
                if client::is_redirect_limit_error(&e) {
                    let limit = cli.max_redirect.unwrap_or(20);
                    log::log_connect_ok(cli, host, port);
                    log::log_redirects_exceeded(cli, limit);
                    return http_fail(&out, ExitCode::ServerError);
                }
                let conn_refused = is_connection_refused_error(&e);
                let host_error = is_host_error(&e);
                let should_retry = attempt < max_attempts
                    && (!conn_refused || cli.retry_connrefused)
                    && (!host_error || cli.retry_on_host_error);

                if should_retry {
                    log_reqwest_connect_failed(cli, host, port, &e, false);
                    log::log_retry(cli);
                    continue;
                }
                let issuer = resolve_tls_failure_issuer(host, port, &e).await;
                return http_fail(
                    &out,
                    log_reqwest_connect_failure(cli, host, port, &e, true, &out, issuer),
                );
            }
        };
        log::log_connect_ok(cli, host, port);
        let mut effective_url = resp.url().clone();

        let status_early = resp.status();

        // 304 属于 3xx，但不是 Location 重定向；必须在 is_redirection() 之前处理（对齐 GNU -N + IMS）。
        if cli.effective_timestamping() && status_early == StatusCode::NOT_MODIFIED {
            let _ = resp.bytes().await;
            if !cli.quiet {
                log::log_status_line(cli, crate::i18n::gettext("Not Modified."));
            }
            return Ok((
                ExitCode::Success as i32,
                out.clone(),
                effective_url,
                None,
                None,
                false,
            ));
        }

        if status_early.is_redirection() {
            let reason = status_early.canonical_reason().unwrap_or("");
            let loc = resp
                .headers()
                .get(LOCATION)
                .and_then(|v| v.to_str().ok());
            let limit = cli.max_redirect.unwrap_or(20);
            log::log_http_redirect_not_followed(cli, status_early.as_u16(), reason, loc);
            log::log_redirects_exceeded(cli, limit);
            let _ = resp.bytes().await.map_err(|e| classify_reqwest_error(&e))?;
            return http_fail(&out, ExitCode::ServerError);
        }

        // GNU：如果服务器忽略 If-Modified-Since（返回 200），但 Last-Modified <= 本地 mtime 且长度相同，则跳过下载。
        if cli.effective_timestamping() && sent_ims && resp.status() == StatusCode::OK && out != "-"
        {
            if let (Some(remote_lm), Some(local_mt)) =
                (maybe_last_modified(resp.headers()), local_mtime)
            {
                let contlen = resp.content_length();
                let size_ok = match (local_size, contlen) {
                    (Some(ls), Some(rl)) => ls == rl,
                    _ => contlen.is_none(),
                };
                if remote_lm <= local_mt && size_ok {
                    let _ = resp.bytes().await;
                    // 对齐 GNU：提示 server ignored IMS（不做强断言，只保证可见行为接近）
                    if !cli.quiet {
                        log::log_status_line(
                            cli,
                            &crate::i18n::tr_named(
                                "Server ignored If-Modified-Since header for file {out}.",
                                &[("out", &out)],
                            ),
                        );
                        log::log_status_line(
                            cli,
                            crate::i18n::gettext(
                                "You might want to add --no-if-modified-since option.",
                            ),
                        );
                    }
                    return Ok((
                        ExitCode::Success as i32,
                        out.clone(),
                        effective_url,
                        None,
                        None,
                        false,
                    ));
                }
            }
        }

        // 观察 HTTPS 响应的 STS 头并更新 HSTS 数据库（GNU：每次看到就刷新 created/max-age）。
        crate::hsts::observe_sts_header(cli, &url, resp.headers());

        let mut response_logged = false;

        // GNU：首包 401 → 打印验证方式 → 带 Basic 重试（非 `--auth-no-challenge`）。
        if resp.status() == StatusCode::UNAUTHORIZED
            && auth::www_authenticate_is_basic(&resp)
            && !preempt
        {
            log::log_http_transfer_response(cli, &resp)?;
            response_logged = true;
            if let Some(scheme) = auth::first_www_authenticate(&resp) {
                log::log_http_auth_scheme_selected(cli, &scheme);
            }
            if let Some((u, p)) = &creds {
                let _ = resp.bytes().await;
                log::log_connect_ok(cli, host, port);
                let req2 = request::build_request(
                    &client,
                    url.clone(),
                    &rp,
                    &spec,
                    Some((u.as_str(), p.as_str())),
                )?;
                resp = match client::send_with_read_idle(cli, req2).await {
                    Ok(r) => r,
                    Err(client::SendError::ReadIdle) => {
                        if attempt < max_attempts {
                            log::log_connect_failed_nv(cli, "Connection timed out.");
                            log::log_retry(cli);
                            continue;
                        }
                        log::log_connect_failed_nv(cli, "Connection timed out.");
                        return http_fail(&out, ExitCode::NetworkFail);
                    }
                    Err(client::SendError::Transport(e)) => {
                        if attempt < max_attempts {
                            log_reqwest_connect_failed(cli, host, port, &e, false);
                            log::log_retry(cli);
                            continue;
                        }
                        let issuer = resolve_tls_failure_issuer(host, port, &e).await;
                        return http_fail(
                            &out,
                            log_reqwest_connect_failure(cli, host, port, &e, true, &out, issuer),
                        );
                    }
                };
                effective_url = resp.url().clone();
                response_logged = false;
            } else {
                let _ = resp.bytes().await;
                log::log_http_auth_failed(cli);
                return http_fail(&out, ExitCode::ServerAuthFail);
            }
        }

        if !response_logged {
            log::log_http_transfer_response(cli, &resp)?;
        }

        let status = resp.status();
        let status_u16 = status.as_u16();
        let reason = status.canonical_reason().unwrap_or("");

        // GNU `AUTHFAILED` → 退出码 6：已带 Basic（抢先或 401 后重试）仍返回 401。
        if status == StatusCode::UNAUTHORIZED
            && auth::www_authenticate_is_basic(&resp)
            && (preempt || creds.is_some())
        {
            let _ = resp.bytes().await.map_err(|e| classify_reqwest_error(&e))?;
            if !cli.quiet {
                log::log_status_line(cli, "");
            }
            log::log_http_auth_failed(cli);
            return http_fail(&out, ExitCode::ServerAuthFail);
        }
        let content_length = resp.content_length();
        let wants_retry_on_http_error = cli
            .retry_on_http_error
            .as_ref()
            .is_some_and(|s| retry_on_http_error_includes(s, status_u16));

        let mut want_return_server_error_after_write = false;
        let mut want_exit_server_error = false;
        if status == reqwest::StatusCode::RANGE_NOT_SATISFIABLE {
            if resume_path != "-" && std::path::Path::new(&resume_path).exists() {
                return Ok((
                    ExitCode::Success as i32,
                    out.clone(),
                    url.clone(),
                    None,
                    None,
                    false,
                ));
            }
            return http_fail(&out, ExitCode::ServerError);
        }

        if !status.is_success() && status != reqwest::StatusCode::PARTIAL_CONTENT {
            if wants_retry_on_http_error && attempt < max_attempts {
                // GNU：把指定 HTTP 错误当作非致命，继续尝试。
                let _ = resp.bytes().await;
                log::log_retry(cli);
                continue;
            }
            if cli.spider {
                // GNU：--spider 会把响应 body 丢弃，但仍创建输出文件（通常为空）
                // 并保持错误退出码（如 404 -> 8）。
                want_exit_server_error = true;
            } else if cli.content_on_error {
                want_return_server_error_after_write = true;
            } else {
                let _ = resp.bytes().await;
                log::log_http_error(cli, Some(raw_url), status_u16, reason);
                log::log_output_error(cli, raw_url, status_u16, reason);
                return http_fail(&out, ExitCode::ServerError);
            }
        }

        // 对齐 GNU http.c：未启用 Content-Disposition 且未指定 -O 时，
        // `--trust-server-names` 决定使用“重定向后的最终 URL 的 last component”还是“原始 URL 的 last component”。
        if out_from_url_default && !cli.content_disposition {
            let url_for_name = if cli.trust_server_names {
                &effective_url
            } else {
                &url
            };
            let dirstruct_on = dirstruct::effective_dirstruct(cli, false);
            out = pathgen::output_path_from_url(cli, url_for_name, dirstruct_on);
        }

        if cli.content_disposition && output_override.is_none() && output_doc.is_none() && !cli.resume {
            if let Some(suggested) = content_disp::preferred_filename(resp.headers()) {
                let safe = content_disp::sanitize_local_name(&suggested);
                if !safe.is_empty() {
                    let dirstruct_on = dirstruct::effective_dirstruct(cli, false);
                    out = pathgen::output_path_with_filename(
                        cli,
                        &effective_url,
                        &safe,
                        dirstruct_on,
                    );
                }
            }
        }

        // 对齐 GNU http.c：在确定最终 local_file 后，按 Content-Type 决定是否追加 .html/.css
        maybe_adjust_extension(cli, &mut out, resp.headers());

        let nc_skip_after_resp = std::path::Path::new(&out).exists()
            && (existed_at_start.contains(&out) || !explicit_output);
        if no_clobber && !cli.resume && out != "-" && nc_skip_after_resp {
            if !cli.quiet {
                log::log_status_line(
                    cli,
                    &crate::i18n::tr_named(
                        "File '{out}' already there; not retrieving.",
                        &[("out", &out)],
                    ),
                );
            }
            let _ = resp.bytes().await.map_err(|e| classify_reqwest_error(&e))?;
            let code = noclobber_skip_exit_code(explicit_output);
            return Ok((code, out.clone(), effective_url, None, None, false));
        }

        // GNU 默认行为：若目标文件已存在且未启用 --backups，则自动选择不冲突的 “.1/.2/...” 名称。
        // 注意：与 --no-clobber 不同；--no-clobber 是“直接跳过”，这里是“改名继续下载”。
        if out != "-"
            && !cli.resume
            && !no_clobber
            && !cli.effective_timestamping()
            && cli.backups.is_none()
            && !effective_overwrite_allowed_by_clobber(cli)
            && std::path::Path::new(&out).exists()
        {
            out = next_available_numbered_name(&out);
        }

        let last_modified_after_write = maybe_last_modified(resp.headers());

        let want_capture = capture_html
            && !cli.spider
            && (is_html_content_type(resp.headers()) || is_css_content_type(resp.headers()));
        let mut captured: Vec<u8> = Vec::new();
        let captured_headers = if want_capture {
            Some(resp.headers().clone())
        } else {
            None
        };

        let save_hdr = if cli.spider || !cli.save_headers {
            None
        } else {
            Some(request::format_saved_headers(&resp)?)
        };

        // GNU：多个 URL 共用一个 `-O` 时后续响应追加写入；`-nc` 仅阻止「启动前已存在」的落盘。
        let do_append = out != "-"
            && ((rp.want_range && status == reqwest::StatusCode::PARTIAL_CONTENT)
                || (explicit_output
                    && std::path::Path::new(&out).exists()
                    && !existed_at_start.contains(&out)));
        let restarted = out != "-" && rp.want_range && !do_append && rp.start_from > 0;

        // 服务端忽略 Range 并返回 200 + 全文时：丢弃前 start_from 字节再落盘（206 时 body 已是后缀，skip=0）。
        let skip_leading: u64 =
            if rp.want_range && status == reqwest::StatusCode::OK && rp.start_from > 0 {
                rp.start_from
            } else {
                0
            };

        let target = if out == "-" {
            OutputTarget::Stdout
        } else {
            OutputTarget::File {
                path: out.clone(),
                append: do_append,
            }
        };

        if !do_append && out != "-" {
            backups::rotate_backups(cli, &out);
        }
        ensure_parent_dir_for_path(&out)?;
        let mut w = open_output(target, cli, restarted)?;

        if let Some(ref block) = save_hdr {
            w.write_all(block.as_bytes())
                .map_err(|_| ExitCode::IoFail)?;
        }

        // 下载前输出易读的长度与保存路径（FTP/HTTP 共用）。
        if cli.effective_show_progress() && !cli.spider && out != "-" {
            let ct = resp
                .headers()
                .get(CONTENT_TYPE)
                .and_then(|v| v.to_str().ok())
                .unwrap_or("application/octet-stream");
            if let Some(len) = content_length {
                log::log_length_known(cli, &out, len, ct);
            } else {
                log::log_length_unspecified(cli, &out, ct);
            }
        }

        // `--spider`：只检查，不下载文档内容；文件保留为空或仅包含 save-headers（但此处已禁止）。
        if cli.spider {
            w.flush().ok();
            if want_exit_server_error {
                log::log_http_error(cli, Some(raw_url), status_u16, reason);
                log::log_output_error(cli, raw_url, status_u16, reason);
                return http_fail(&out, ExitCode::ServerError);
            }
            log::log_transfer_summary(cli, raw_url, &out, 0, content_length, attempt, false);
            return Ok((
                ExitCode::Success as i32,
                out.clone(),
                url.clone(),
                None,
                None,
                false,
            ));
        }

        // --limit-rate：流式 + 节流；否则大块下载或 --show-progress 时用流式以便刷新进度
        let dl_start = std::time::Instant::now();
        if let Some(rate) = &cli.limit_rate {
            let bytes_per_sec = parse_rate_bytes_per_sec(rate).map_err(|_| ExitCode::ParseError)?;
            let cap = if want_capture {
                Some(&mut captured)
            } else {
                None
            };
            if let Err(ec) =
                stream_with_limit(cli, resp, &mut w, bytes_per_sec, skip_leading, cap, &out).await
            {
                if ec == ExitCode::NetworkFail && attempt < max_attempts {
                    log::log_retry(cli);
                    continue;
                }
                return Err(ec);
            }
        } else {
            let cap = if want_capture {
                Some(&mut captured)
            } else {
                None
            };
            // 始终走流式读，以便应用 GNU 空闲读超时（默认 900s；-T 为空闲间隔非总时长）。
            if let Err(ec) = stream_body_with_progress(cli, resp, &mut w, skip_leading, cap, &out).await
            {
                if ec == ExitCode::NetworkFail && attempt < max_attempts {
                    log::log_retry(cli);
                    continue;
                }
                return Err(ec);
            }
        }
        w.flush().ok();
        let received = if out == "-" {
            0
        } else {
            std::fs::metadata(&out).ok().map(|m| m.len()).unwrap_or(0)
        };
        log::log_download_complete(cli, &out, received, content_length, dl_start.elapsed());
        log::log_transfer_summary(cli, raw_url, &out, received, content_length, attempt, false);
        // 成功下载后：按 Last-Modified 回写本地 mtime（GNU 默认开启 useservertimestamps）。
        if cli.effective_timestamping() && cli.effective_use_server_timestamps() && out != "-" {
            if let Some(lm) = last_modified_after_write {
                let _ = filetime::set_file_mtime(&out, filetime::FileTime::from_system_time(lm));
            }
        }
        if want_return_server_error_after_write {
            log::log_http_error(cli, Some(raw_url), status_u16, reason);
            log::log_output_error(cli, raw_url, status_u16, reason);
            return http_fail(&out, ExitCode::ServerError);
        }
        if cli.delete_after && output_override.is_none() && out != "-" {
            if std::fs::remove_file(&out).is_ok() {
                if cli.effective_verbose() {
                    log::log_status_line(
                        cli,
                        &crate::i18n::tr_named("Removing {out}.", &[("out", &out)]),
                    );
                }
            }
        }
        return Ok((
            ExitCode::Success as i32,
            out.clone(),
            effective_url,
            if want_capture { Some(captured) } else { None },
            captured_headers,
            true,
        ));
    }

    http_fail(&out, ExitCode::NetworkFail)
}

fn parse_rate_bytes_per_sec(s: &str) -> Result<u64, ()> {
    let s = s.trim();
    if s.is_empty() {
        return Err(());
    }
    let (num_part, mul) = match s.chars().last().unwrap() {
        'k' | 'K' => (&s[..s.len() - 1], 1024u64),
        'm' | 'M' => (&s[..s.len() - 1], 1024u64 * 1024),
        _ => (s, 1u64),
    };
    let v: u64 = num_part.trim().parse().map_err(|_| ())?;
    if v == 0 {
        return Err(());
    }
    Ok(v.saturating_mul(mul))
}

async fn stream_body_with_progress(
    cli: &Cli,
    resp: reqwest::Response,
    w: &mut dyn Write,
    mut skip_leading: u64,
    mut capture: Option<&mut Vec<u8>>,
    out_path: &str,
) -> Result<(), ExitCode> {
    let total = resp.content_length();
    let mut stream = resp.bytes_stream();
    let start = std::time::Instant::now();
    let label = crate::progress::label_from_output_path(out_path);
    let mut prog = crate::progress::DownloadProgress::new(cli, total, label.as_deref());
    prog.begin_transfer(cli);
    while let Some(next) = client::next_body_chunk_with_read_idle(cli, &mut stream).await? {
        let mut chunk: bytes::Bytes = next;
        if skip_leading > 0 {
            let cl = chunk.len() as u64;
            if skip_leading >= cl {
                skip_leading -= cl;
                continue;
            }
            chunk = chunk.slice(skip_leading as usize..);
            skip_leading = 0;
        }
        w.write_all(&chunk).map_err(|_| ExitCode::IoFail)?;
        if let Some(ref mut c) = capture {
            c.extend_from_slice(&chunk);
        }
        prog.update(cli, chunk.len() as u64, start.elapsed().as_secs_f64());
    }
    prog.finish(cli, start.elapsed().as_secs_f64());
    Ok(())
}

async fn stream_with_limit(
    cli: &Cli,
    resp: reqwest::Response,
    w: &mut dyn Write,
    bytes_per_sec: u64,
    mut skip_leading: u64,
    mut capture: Option<&mut Vec<u8>>,
    out_path: &str,
) -> Result<(), ExitCode> {
    use tokio::time::{Instant, sleep};

    let total = resp.content_length();
    let mut stream = resp.bytes_stream();
    let start = Instant::now();
    let label = crate::progress::label_from_output_path(out_path);
    let mut prog = crate::progress::DownloadProgress::new(cli, total, label.as_deref());
    let mut sent_total: u64 = 0;
    prog.begin_transfer(cli);

    while let Some(next) = client::next_body_chunk_with_read_idle(cli, &mut stream).await? {
        let mut chunk: bytes::Bytes = next;
        if skip_leading > 0 {
            let cl = chunk.len() as u64;
            if skip_leading >= cl {
                skip_leading -= cl;
                continue;
            }
            chunk = chunk.slice(skip_leading as usize..);
            skip_leading = 0;
        }
        w.write_all(&chunk).map_err(|_| ExitCode::IoFail)?;
        if let Some(ref mut c) = capture {
            c.extend_from_slice(&chunk);
        }
        prog.update(cli, chunk.len() as u64, start.elapsed().as_secs_f64());
        sent_total = sent_total.saturating_add(chunk.len() as u64);

        // Token-bucket-like throttling: ensure wall time >= sent_total / bytes_per_sec.
        let should_elapsed = sent_total as f64 / bytes_per_sec as f64;
        let actual_elapsed = start.elapsed().as_secs_f64();
        if should_elapsed > actual_elapsed {
            let to_sleep = Duration::from_secs_f64(should_elapsed - actual_elapsed);
            // Avoid sleeping too tiny intervals.
            if to_sleep > Duration::from_millis(5) {
                sleep(to_sleep).await;
            }
        }
    }
    prog.finish(cli, start.elapsed().as_secs_f64());
    Ok(())
}
