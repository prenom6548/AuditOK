//! 从 CLI 与 `-i` 输入文件展开待下载 URL 列表（对齐 GNU wget 顺序：先做命令行 URL，再做输入文件）。

use std::ffi::OsStr;
use std::fs;
use std::io::{self, Read};
use std::path::Path;

use crate::cli::Cli;
use crate::exit::ExitCode;
use crate::html_urls;
use percent_encoding::percent_decode_str;
use url::Url;

/// 命令行上的 URL 也做「无 scheme 则补 `http://`」以贴近 GNU `maybe_prepend_scheme`。
fn resolve_cli_url(raw: &str) -> Result<String, String> {
    let t = raw.trim();
    if t.is_empty() {
        return Err(crate::i18n::gettext("empty URL").to_string());
    }
    if Url::parse(t)
        .ok()
        .is_some_and(|u| u.host_str().is_some() || u.scheme() == "file")
    {
        return Ok(t.to_string());
    }
    if t.contains("://") {
        return Url::parse(t)
            .map(|u| u.to_string())
            .map_err(|e| format!("Invalid URL '{t}': {e}"));
    }
    let with = format!("http://{t}");
    Url::parse(&with)
        .map(|_| with)
        .map_err(|e| format!("Invalid URL '{t}': {e}"))
}

/// 输入文件中的一行：trim；可相对；可与 `-B` 合并；无 scheme 且无 base 时补 `http://`。
fn resolve_file_line(line: &str, base: Option<&Url>) -> Result<Option<String>, String> {
    let t = line.trim();
    if t.is_empty() {
        return Ok(None);
    }
    let url: Url = match Url::parse(t) {
        Ok(u) if u.host_str().is_some() || u.scheme() == "file" => u,
        Ok(_) | Err(_) => {
            if let Some(b) = base {
                b.join(t).map_err(|e| format!("{e} (with --base): '{t}'"))?
            } else if t.contains("://") {
                Url::parse(t).map_err(|e| format!("Invalid URL '{t}': {e}"))?
            } else {
                Url::parse(&format!("http://{t}")).map_err(|e| format!("Invalid URL '{t}': {e}"))?
            }
        }
    };
    Ok(Some(url.to_string()))
}

fn read_input_as_string(path: &Path) -> Result<String, io::Error> {
    if path.as_os_str() == OsStr::new("-") {
        let mut s = String::new();
        io::stdin().read_to_string(&mut s)?;
        Ok(s)
    } else {
        fs::read_to_string(path)
    }
}

/// 展开顺序：**先** `cli.urls`，**再** `-i` 文件中各行（与 GNU `main.c` 一致）。
pub fn expand_url_list(cli: &Cli) -> Result<Vec<String>, String> {
    let base = if let Some(ref b) = cli.base_url {
        Some(Url::parse(b.trim()).map_err(|e| format!("Invalid --base URL '{b}': {e}"))?)
    } else {
        None
    };

    let mut out: Vec<String> = Vec::new();
    for u in &cli.urls {
        out.push(resolve_cli_url(u)?);
    }

    if let Some(ref path) = cli.input_file {
        let text = read_input_as_string(path).map_err(|e| format!("{}: {}", path.display(), e))?;

        if cli.force_html {
            let extracted = html_urls::extract_urls_from_html(&text, base.as_ref(), path.as_path());
            if extracted.is_empty() && cli.urls.is_empty() {
                return Err(crate::i18n::tr_named(
                    "No URLs found in {path}",
                    &[("path", &path.display().to_string())],
                ));
            }
            out.extend(extracted);
        } else {
            let mut any_line = false;
            for line in text.lines() {
                if let Some(u) = resolve_file_line(line, base.as_ref())? {
                    any_line = true;
                    out.push(u);
                }
            }
            if !any_line && cli.urls.is_empty() {
                return Err(crate::i18n::tr_named(
                    "No URLs found in {path}",
                    &[("path", &path.display().to_string())],
                ));
            }
        }
    }

    if out.is_empty() {
        crate::cli::print_missing_url_usage();
        return Err(String::new());
    }
    Ok(out)
}

/// GNU wget：命令行出现多个 `-O` 时，**最后一个** `-O` 作为全部 URL 的输出文件
///（与 `output_document.last()` / 手册 “concatenate to one file” 语义一致）。
pub fn output_documents_for_all_urls(cli: &Cli, url_count: usize) -> Vec<Option<String>> {
    let doc = cli
        .effective_output_document()
        .map(|s| s.to_string());
    vec![doc; url_count]
}

pub fn parse_url_with_iri_policy(cli: &Cli, raw_url: &str) -> Result<Url, ExitCode> {
    // GNU `--iri/--no-iri` 的差异主要体现在 IRI 编码/显示与 IDN 处理策略上；
    // 对于 UTF-8 path 本身，GNU 在 `--no-iri` 下也允许（至少在我们对齐的场景中）。
    let _ = cli.effective_enable_iri();
    match Url::parse(raw_url) {
        Ok(u) => Ok(u),
        Err(_) => {
            // GNU url.c: Decode %HH sequences in host name (before potential IDNA encode).
            // Rust `url` crate may reject percent-encoded bytes in host; try a narrow rewrite:
            // percent-decode only the host portion and re-parse.
            let rewritten = rewrite_percent_encoded_host(raw_url)?;
            Url::parse(&rewritten).map_err(|_| ExitCode::ParseError)
        }
    }
}

fn rewrite_percent_encoded_host(raw_url: &str) -> Result<String, ExitCode> {
    let Some(pos) = raw_url.find("://") else {
        return Err(ExitCode::ParseError);
    };
    let scheme_part = &raw_url[..pos + 3];
    let rest = &raw_url[pos + 3..];
    let end_auth = rest.find(&['/', '?', '#'][..]).unwrap_or(rest.len());
    let auth = &rest[..end_auth];
    if !auth.contains('%') {
        return Err(ExitCode::ParseError);
    }

    let after_auth = &rest[end_auth..];
    let (userinfo, hostport) = match auth.rsplit_once('@') {
        Some((u, hp)) => (Some(u), hp),
        None => (None, auth),
    };

    // IPv6 literal in brackets: don't percent-decode.
    if hostport.starts_with('[') {
        return Err(ExitCode::ParseError);
    }

    // Split optional port; use last ':' only (hostnames don't contain ':' unless IPv6).
    let (host, port) = match hostport.rsplit_once(':') {
        Some((h, p)) if !p.is_empty() && p.bytes().all(|b| b.is_ascii_digit()) => (h, Some(p)),
        _ => (hostport, None),
    };
    if !host.contains('%') {
        return Err(ExitCode::ParseError);
    }
    let decoded_host: String =
        String::from_utf8_lossy(&percent_decode_str(host).collect::<Vec<u8>>()).to_string();

    let mut new_auth = String::new();
    if let Some(u) = userinfo {
        new_auth.push_str(u);
        new_auth.push('@');
    }
    new_auth.push_str(&decoded_host);
    if let Some(p) = port {
        new_auth.push(':');
        new_auth.push_str(p);
    }

    Ok(format!("{scheme_part}{new_auth}{after_auth}"))
}

/// 日志/终端展示用 URL：IRI 开启时将 path/query 中的 `%XX` 解码为 UTF-8（中文等）。
pub fn url_for_user_display(cli: &Cli, u: &Url) -> String {
    if !cli.effective_enable_iri() {
        return u.as_str().to_string();
    }

    let mut s = String::new();
    s.push_str(u.scheme());
    s.push_str("://");

    if !u.username().is_empty() {
        s.push_str(u.username());
        if let Some(pass) = u.password() {
            s.push(':');
            s.push_str(pass);
        }
        s.push('@');
    }

    if let Some(host) = u.host_str() {
        s.push_str(&crate::idn::host_display(cli, host));
    }

    if let Some(port) = u.port() {
        let default = match u.scheme() {
            "ftp" => 21,
            "http" => 80,
            "https" => 443,
            _ => 0,
        };
        if default == 0 || port != default {
            s.push(':');
            s.push_str(&port.to_string());
        }
    }

    s.push_str(&percent_decode_iri_component(u.path()));

    if let Some(q) = u.query() {
        s.push('?');
        s.push_str(&percent_decode_iri_component(q));
    }
    if let Some(f) = u.fragment() {
        s.push('#');
        s.push_str(&percent_decode_iri_component(f));
    }
    s
}

fn percent_decode_iri_component(part: &str) -> String {
    String::from_utf8_lossy(&percent_decode_str(part).collect::<Vec<u8>>()).into_owned()
}
