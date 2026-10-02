//! HTTP 请求构造：Method、Range（仅 GET 且无 body）、body、与 GNU wget 一致的默认 Content-Type。

use crate::cli::Cli;
use crate::exit::ExitCode;
use crate::resume::ResumePlan;
use reqwest::Method;
use reqwest::header::{CONTENT_TYPE, HeaderValue, RANGE};
use url::Url;

/// 单次请求的方法与 body（`--post-*` 在 `prepare_request` 内已规范为 POST + body）。
pub struct RequestSpec {
    pub method: Method,
    pub body: Option<Vec<u8>>,
}

/// 从 CLI 解析出方法与 body（读取 `body-file` / `post-file`）。
pub fn prepare_request(cli: &Cli) -> Result<RequestSpec, ExitCode> {
    if cli.post_data.is_some() || cli.post_file.is_some() {
        let body = if let Some(s) = &cli.post_data {
            s.as_bytes().to_vec()
        } else if let Some(p) = &cli.post_file {
            std::fs::read(p).map_err(|_| ExitCode::IoFail)?
        } else {
            unreachable!()
        };
        return Ok(RequestSpec {
            method: Method::POST,
            body: Some(body),
        });
    }

    if let Some(m) = &cli.method {
        let ms = m.trim();
        let method = Method::from_bytes(ms.as_bytes())
            .or_else(|_| Method::from_bytes(ms.to_uppercase().as_bytes()))
            .map_err(|_| ExitCode::ParseError)?;
        let body = if let Some(s) = &cli.body_data {
            Some(s.as_bytes().to_vec())
        } else if let Some(p) = &cli.body_file {
            Some(std::fs::read(p).map_err(|_| ExitCode::IoFail)?)
        } else {
            None
        };
        return Ok(RequestSpec { method, body });
    }

    Ok(RequestSpec {
        method: Method::GET,
        body: None,
    })
}

pub fn build_request(
    client: &reqwest::Client,
    url: Url,
    resume: &ResumePlan,
    spec: &RequestSpec,
    basic: Option<(&str, &str)>,
) -> Result<reqwest::RequestBuilder, ExitCode> {
    let mut rb = client.request(spec.method.clone(), url);

    if let Some((user, pass)) = basic {
        rb = rb.basic_auth(user, Some(pass));
    }

    let allow_range = spec.method == Method::GET && spec.body.is_none();
    if allow_range && resume.want_range {
        rb = rb.header(RANGE, format!("bytes={}-", resume.start_from));
    }

    if let Some(b) = &spec.body {
        // 对齐 src/http.c：带 body 的 --method 请求默认 Content-Type
        let ct = HeaderValue::from_static("application/x-www-form-urlencoded");
        rb = rb.header(CONTENT_TYPE, ct);
        rb = rb.body(b.clone());
    }

    Ok(rb)
}

/// `--save-headers`：合成「状态行 + 头 + 空行」，风格接近 wget 落盘（非原始字节级一致）。
pub fn format_saved_headers(resp: &reqwest::Response) -> Result<String, ExitCode> {
    use reqwest::Version;
    let ver = match resp.version() {
        Version::HTTP_09 => "HTTP/0.9",
        Version::HTTP_10 => "HTTP/1.0",
        Version::HTTP_11 => "HTTP/1.1",
        Version::HTTP_2 => "HTTP/2.0",
        Version::HTTP_3 => "HTTP/3.0",
        _ => "HTTP/1.1",
    };
    let st = resp.status();
    let reason = st.canonical_reason().unwrap_or("");
    let mut out = format!("{} {} {}\r\n", ver, st.as_u16(), reason);
    for (k, v) in resp.headers() {
        let Ok(vs) = v.to_str() else {
            continue;
        };
        out.push_str(&format!("{}: {}\r\n", k.as_str(), vs));
    }
    out.push_str("\r\n");
    Ok(out)
}
