//! 构建 `reqwest::Client`：重定向、默认头、代理（对齐 GNU wget）、超时。

use crate::cert_check;
use crate::cli::Cli;
use crate::cookies;
use crate::dns_family::{AddrFamilyPolicy, FamilyFilterResolver};
use crate::exit::ExitCode;
use crate::headers::build_headers;
use crate::tls_ca;
use futures_util::StreamExt;
use reqwest::redirect::Policy;
use std::time::Duration;
use url::Url;

/// GNU wget：默认 **900 秒空闲读超时**；`-T`/`--read-timeout` 覆盖；`0` 关闭。
pub fn effective_read_idle_duration(cli: &Cli) -> Option<Duration> {
    if let Some(secs) = cli.read_timeout_seconds.or(cli.timeout_seconds) {
        if secs <= 0.0 || !secs.is_finite() {
            return None;
        }
        return Some(Duration::from_secs_f64(secs));
    }
    Some(Duration::from_secs(900))
}

/// GNU wget：`-T`/`--connect-timeout` 仅限制 TCP 建连阶段（无默认上限）。
pub fn effective_connect_timeout_duration(cli: &Cli) -> Option<Duration> {
    let secs = cli.connect_timeout_seconds.or(cli.timeout_seconds)?;
    if secs <= 0.0 || !secs.is_finite() {
        return None;
    }
    Some(Duration::from_secs_f64(secs))
}

/// HTTP 发送失败：传输错误或 GNU 风格空闲读超时。
#[derive(Debug)]
pub enum SendError {
    Transport(reqwest::Error),
    ReadIdle,
}

/// 在「空闲读超时」内等待 HTTP 响应首包（对齐 GNU read timeout，非整次下载总时长）。
pub async fn send_with_read_idle(
    cli: &Cli,
    req: reqwest::RequestBuilder,
) -> Result<reqwest::Response, SendError> {
    let idle = effective_read_idle_duration(cli);
    match idle {
        None => req.send().await.map_err(SendError::Transport),
        Some(d) => match tokio::time::timeout(d, req.send()).await {
            Ok(Ok(r)) => Ok(r),
            Ok(Err(e)) => Err(SendError::Transport(e)),
            Err(_) => Err(SendError::ReadIdle),
        },
    }
}

/// 流式 body 下一 chunk：两次收到数据间隔超过空闲读超时则失败（GNU read timeout）。
pub async fn next_body_chunk_with_read_idle<S>(
    cli: &Cli,
    stream: &mut S,
) -> Result<Option<bytes::Bytes>, ExitCode>
where
    S: futures_util::Stream<Item = Result<bytes::Bytes, reqwest::Error>> + Unpin,
{
    let idle = effective_read_idle_duration(cli);
    let next = match idle {
        None => stream.next().await,
        Some(d) => match tokio::time::timeout(d, stream.next()).await {
            Ok(v) => v,
            Err(_) => return Err(ExitCode::NetworkFail),
        },
    };
    match next {
        Some(Ok(b)) => Ok(Some(b)),
        Some(Err(e)) => Err(classify_reqwest_error(&e)),
        None => Ok(None),
    }
}

fn first_env_nonempty(keys: &[&str]) -> Option<String> {
    for k in keys {
        if let Ok(v) = std::env::var(k) {
            let t = v.trim();
            if !t.is_empty() {
                return Some(t.to_string());
            }
        }
    }
    None
}

/// 对齐 GNU wget：默认读 `no_proxy`；若未设置再读 `NO_PROXY`（常见生态兼容）。
fn wget_style_no_proxy() -> Option<reqwest::NoProxy> {
    let raw = std::env::var("no_proxy")
        .or_else(|_| std::env::var("NO_PROXY"))
        .ok()?;
    let t = raw.trim();
    if t.is_empty() {
        return None;
    }
    reqwest::NoProxy::from_string(t)
}

fn apply_scheme_proxy(
    cli: &Cli,
    url: &Url,
    client: reqwest::ClientBuilder,
) -> Result<reqwest::ClientBuilder, ExitCode> {
    let scheme = url.scheme();
    if !scheme.eq_ignore_ascii_case("http") && !scheme.eq_ignore_ascii_case("https") {
        return Ok(client);
    }

    let proxy_url = if let Some(p) = &cli.proxy_url {
        Some(p.clone())
    } else if scheme.eq_ignore_ascii_case("https") {
        first_env_nonempty(&["https_proxy", "HTTPS_PROXY"])
    } else {
        first_env_nonempty(&["http_proxy", "HTTP_PROXY"])
    };

    let Some(mut pu) = proxy_url else {
        return Ok(client);
    };

    // GNU wget 允许 `--proxy=HOST:PORT`；这里做一个最小兼容：补 http://
    if !pu.contains("://") {
        pu = format!("http://{pu}");
    }

    let no_proxy = wget_style_no_proxy();

    let mut p = if scheme.eq_ignore_ascii_case("https") {
        reqwest::Proxy::https(&pu).map_err(|_| ExitCode::ParseError)?
    } else {
        reqwest::Proxy::http(&pu).map_err(|_| ExitCode::ParseError)?
    };

    if let Some(ref n) = no_proxy {
        p = p.no_proxy(Some(n.clone()));
    }

    if let Some(u) = &cli.proxy_user {
        let pw = cli.proxy_password.as_deref().unwrap_or("");
        p = p.basic_auth(u, pw);
    }

    Ok(client.proxy(p))
}

/// 关闭 reqwest 隐式系统代理后，按需附加 wget 风格代理与环境超时。
pub fn build_client(cli: &Cli, url: &Url, max_redir: usize) -> Result<reqwest::Client, ExitCode> {
    let redirect_policy = if cli.https_only && cli.effective_recursive() {
        // 对齐 GNU recur.c：递归 + --https-only 时不跟随到非 HTTPS（含重定向链）。
        Policy::custom(move |attempt| {
            if attempt.url().scheme().eq_ignore_ascii_case("https") {
                attempt.follow()
            } else {
                attempt.stop()
            }
        })
    } else if max_redir == 0 {
        // GNU --max-redirect=0：不跟随；由 http 层处理 3xx 并报告 “redirections exceeded”。
        Policy::none()
    } else {
        Policy::limited(max_redir)
    };

    let mut client = reqwest::Client::builder()
        .redirect(redirect_policy)
        .default_headers(build_headers(cli).map_err(|_| ExitCode::ParseError)?)
        .no_proxy();

    if let Some(store) = cookies::maybe_cookie_provider(cli) {
        client = client.cookie_provider(store);
    }

    if cli.inet4_only {
        client = client.dns_resolver(FamilyFilterResolver::arc(AddrFamilyPolicy::Inet4Only));
    } else if cli.inet6_only {
        client = client.dns_resolver(FamilyFilterResolver::arc(AddrFamilyPolicy::Inet6Only));
    }

    // GNU wget：`--no-proxy` 主要用于忽略环境代理；若用户显式给了 `--proxy=...`，
    // 则仍应走该显式代理（即 `--no-proxy` 不应覆盖 `--proxy`）。
    if !cli.disable_proxy || cli.proxy_url.is_some() {
        client = apply_scheme_proxy(cli, url, client)?;
    }

    // 勿用 reqwest 的 request 总超时：GNU `-T`/`--read-timeout` 是「空闲读」超时，
    // 持续有数据时大文件可下载任意久；见 http.rs 流式读与 send_with_read_idle。
    if let Some(d) = effective_connect_timeout_duration(cli) {
        client = client.connect_timeout(d);
    }

    if url.scheme().eq_ignore_ascii_case("https") {
        let verify = cert_check::verify_server_certificate(cli);
        if verify {
            client = tls_ca::apply_custom_ca_roots(client, cli)?;
        }
        if !verify {
            client = client.danger_accept_invalid_certs(true);
        }
    }

    client.build().map_err(|e| classify_reqwest_error(&e))
}

/// GNU 风格连接失败说明（`failed: ...` / `Connecting... failed: ...`）。
pub(crate) fn reqwest_error_summary(e: &reqwest::Error) -> String {
    if tls_or_cert_likely(e) || connect_failure_likely_tls(e) {
        return tls_error_summary(e);
    }
    if e.is_timeout() {
        return "Connection timed out.".to_string();
    }
    let mut cur: &(dyn std::error::Error + 'static) = e;
    loop {
        if let Some(ioe) = cur.downcast_ref::<std::io::Error>() {
            return match ioe.kind() {
                std::io::ErrorKind::ConnectionRefused => "Connection refused.".to_string(),
                std::io::ErrorKind::TimedOut => "Connection timed out.".to_string(),
                std::io::ErrorKind::NotFound => "Name or service not known.".to_string(),
                _ => "Connection failed.".to_string(),
            };
        }
        let s = cur.to_string().to_ascii_lowercase();
        if s.contains("connection refused") {
            return "Connection refused.".to_string();
        }
        if s.contains("timed out") || s.contains("timeout") {
            return "Connection timed out.".to_string();
        }
        if s.contains("failed to lookup")
            || s.contains("failed to resolve")
            || s.contains("name or service not known")
            || s.contains("nodename nor servname")
            || s.contains("dns error")
        {
            return "Name or service not known.".to_string();
        }
        match cur.source() {
            Some(next) => cur = next,
            None => break,
        }
    }
    let top = e.to_string();
    if top.is_empty() {
        "Connection failed.".to_string()
    } else if top.ends_with('.') {
        top
    } else {
        format!("{top}.")
    }
}

/// 重定向次数超限（reqwest `Policy::limited` 失败）。
#[inline]
pub(crate) fn is_redirect_limit_error(e: &reqwest::Error) -> bool {
    e.is_redirect()
}

/// 将 `reqwest::Error` 粗分为：TLS/证书相关 → `SslAuthFail(5)`，其余连接/传输 → `NetworkFail(4)`。
pub(crate) fn classify_reqwest_error(e: &reqwest::Error) -> ExitCode {
    if is_redirect_limit_error(e) {
        return ExitCode::ServerError;
    }
    if tls_or_cert_likely(e) {
        return ExitCode::SslAuthFail;
    }
    // rustls/webpki 常见：顶层文案不含 “certificate”，但连接失败实为证书校验未通过。
    if connect_failure_likely_tls(e) {
        return ExitCode::SslAuthFail;
    }
    ExitCode::NetworkFail
}

fn connect_failure_likely_tls(e: &reqwest::Error) -> bool {
    let mut s = e.to_string().to_lowercase();
    let mut cur: &(dyn std::error::Error + 'static) = e;
    while let Some(next) = cur.source() {
        s.push(' ');
        s.push_str(&next.to_string().to_lowercase());
        cur = next;
    }
    s.contains("rustls")
        || s.contains("webpki")
        || s.contains("invalid peer")
        || s.contains("handshake")
        || (s.contains("tls") && s.contains("error"))
        || s.contains("unable to get local issuer")
        || s.contains("self signed")
}

fn tls_or_cert_likely(e: &reqwest::Error) -> bool {
    let mut s = e.to_string().to_lowercase();
    let mut cur: &(dyn std::error::Error + 'static) = e;
    while let Some(next) = cur.source() {
        s.push(' ');
        s.push_str(&next.to_string().to_lowercase());
        cur = next;
    }
    if s.contains("certificate")
        || s.contains("webpki")
        || s.contains("rustls")
        || s.contains("invalid peer")
    {
        return true;
    }
    if s.contains("tls")
        && (s.contains("handshake") || s.contains("invalid") || s.contains("alert"))
    {
        return true;
    }
    s.contains("unknown issuer") || s.contains("cert verify")
}

fn tls_error_summary(e: &reqwest::Error) -> String {
    let mut s = e.to_string().to_lowercase();
    let mut cur: &(dyn std::error::Error + 'static) = e;
    while let Some(next) = cur.source() {
        s.push(' ');
        s.push_str(&next.to_string().to_lowercase());
        cur = next;
    }
    if s.contains("self signed") || s.contains("self-signed") {
        return "The certificate is NOT trusted (self-signed).".to_string();
    }
    if s.contains("unknown issuer") || s.contains("unable to get local issuer") {
        return "The certificate is NOT trusted (unknown issuer).".to_string();
    }
    if s.contains("certificate") || s.contains("webpki") || s.contains("rustls") {
        return "The certificate is NOT trusted.".to_string();
    }
    "TLS handshake failed.".to_string()
}
