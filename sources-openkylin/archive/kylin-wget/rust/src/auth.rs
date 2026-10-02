//! HTTP 服务端鉴权（Basic）：对齐 GNU「默认等 401，可 `--auth-no-challenge` 抢先发 Basic」。

use reqwest::Response;
use reqwest::header::WWW_AUTHENTICATE;
use url::Url;

use crate::cli::Cli;

/// 解析顺序对齐 GNU `http.c`（URL 内嵌优先，再 `--http-*`，再 `--user`/`--password`）。
pub fn http_credentials_for_url(cli: &Cli, url: &Url) -> Option<(String, String)> {
    // GNU：user/password 可能一边缺失；缺失时才会去查 .netrc（若开启）。
    let mut user: Option<String> = None;
    let mut pass: Option<String> = None;

    // user priority
    if !url.username().is_empty() {
        user = Some(url.username().to_string());
    } else if let Some(u) = &cli.http_user {
        user = Some(u.clone());
    } else if let Some(u) = &cli.user {
        user = Some(u.clone());
    }

    // password priority
    if url.password().is_some() {
        pass = url.password().map(|s| s.to_string());
    } else if let Some(p) = &cli.http_password {
        pass = Some(p.clone());
    } else if let Some(p) = &cli.password {
        pass = Some(p.clone());
    }

    if cli.effective_netrc() && (user.is_none() || pass.is_none()) {
        if let Some(host) = url.host_str() {
            crate::netrc::search_and_apply(host, &mut user, &mut pass, false);
        }
    }

    match (user, pass) {
        (Some(u), Some(p)) => Some((u, p)),
        _ => None,
    }
}

pub fn www_authenticate_is_basic(resp: &Response) -> bool {
    for v in resp.headers().get_all(WWW_AUTHENTICATE) {
        if let Ok(s) = v.to_str() {
            let low = s.trim().to_ascii_lowercase();
            if low.starts_with("basic") {
                return true;
            }
        }
    }
    false
}

/// 首个 `WWW-Authenticate` 头（用于 GNU “Selected authentication scheme” 行）。
pub fn first_www_authenticate(resp: &Response) -> Option<String> {
    resp.headers()
        .get(WWW_AUTHENTICATE)
        .and_then(|v| v.to_str().ok())
        .map(|s| s.to_string())
}
