use crate::cli::Cli;
use crate::version::WGET_COMPAT_VERSION;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue, REFERER, USER_AGENT};

pub fn build_headers(cli: &Cli) -> Result<HeaderMap, String> {
    let mut headers = HeaderMap::new();
    // GNU wget 默认会带一个 `Wget/...` 的 User-Agent。
    // 许多站点（如 gitee）会对非 wget-like 客户端返回 HTML（而不是目标文件）。
    // 因此：当用户没显式指定 `-U/--user-agent` 时，这里注入一个 wget 默认 UA 以保持外部行为一致。
    let default_ua = format!("Wget/{WGET_COMPAT_VERSION} (linux-gnu)");
    let ua = cli.user_agent.as_deref().unwrap_or(default_ua.as_str());
    let v = HeaderValue::from_str(ua)
        .map_err(|_| crate::i18n::gettext("invalid --user-agent").to_string())?;
    headers.insert(USER_AGENT, v);
    if let Some(r) = &cli.referer {
        let v = HeaderValue::from_str(r)
            .map_err(|_| crate::i18n::gettext("invalid --referer").to_string())?;
        headers.insert(REFERER, v);
    }
    for h in &cli.header {
        let (name, value) = h
            .split_once(':')
            .ok_or_else(|| {
                crate::i18n::tr_named("invalid --header value: {header}", &[("header", h)])
            })?;
        let name = name.trim();
        let value = value.trim();
        let hn = HeaderName::from_bytes(name.as_bytes()).map_err(|_| {
            crate::i18n::tr_named(
                "invalid header name in --header: {name}",
                &[("name", name)],
            )
        })?;
        let hv = HeaderValue::from_str(value).map_err(|_| {
            crate::i18n::tr_named(
                "invalid header value in --header: {name}",
                &[("name", name)],
            )
        })?;
        headers.insert(hn, hv);
    }
    Ok(headers)
}
