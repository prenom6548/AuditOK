//! `-F` / `--force-html`：从本地 HTML 按 GNU `html-url.c` 思路抽取可下载 URL（文档顺序、`<base href>`、无 base 时相对链报警告）。

use std::path::Path;

use scraper::{Html, Selector};
use url::Url;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LinkKind {
    /// 普通“跟随”链接（如 <a href>）
    Follow,
    /// 页面渲染所需资源（img/script/css 等）
    Inline,
}

#[derive(Clone, Debug)]
pub struct RecLink {
    pub resolved: String,
    pub kind: LinkKind,
    pub raw: String,
    pub link_relative_p: bool,
}

fn trim_attr(s: &str) -> &str {
    s.trim_matches(|c: char| matches!(c, ' ' | '\t' | '\r' | '\n' | '\x0c'))
}

fn skip_non_fetchable(raw: &str) -> bool {
    let t = trim_attr(raw);
    if t.is_empty() || t.starts_with('#') {
        return true;
    }
    let low = t.to_ascii_lowercase();
    low.starts_with("javascript:")
        || low.starts_with("mailto:")
        || low.starts_with("tel:")
        || low.starts_with("data:")
}

/// 是否像绝对 URL（含 `://` 或以 `//` 开头）。
fn looks_absolute(s: &str) -> bool {
    let t = trim_attr(s);
    t.contains("://") || t.starts_with("//")
}

fn merge_with_base(raw: &str, base: Option<&Url>, doc_file: &Path) -> Option<String> {
    let raw = trim_attr(raw);
    if skip_non_fetchable(raw) {
        return None;
    }

    if looks_absolute(raw) {
        if raw.starts_with("//") {
            let scheme = base
                .map(|b| b.scheme().to_string())
                .unwrap_or_else(|| "http".to_string());
            return Url::parse(&format!("{scheme}:{raw}"))
                .ok()
                .map(|u| u.to_string());
        }
        return Url::parse(raw).ok().map(|u| u.to_string());
    }

    let Some(b) = base else {
        eprintln!(
            "{}",
            crate::i18n::tr_named(
                "wget: {file}: Cannot resolve incomplete link {link}.",
                &[
                    ("file", &doc_file.display().to_string()),
                    ("link", raw),
                ],
            )
        );
        return None;
    };

    b.join(raw).ok().map(|u| u.to_string())
}

#[derive(Clone, Debug)]
pub struct HtmlLink {
    /// attribute value as it appears in the HTML (trimmed), e.g. `/a.html` or `sub/b.html`
    pub raw: String,
    /// resolved absolute URL string (after applying base/join)
    pub resolved: String,
}

fn push_link(out: &mut Vec<HtmlLink>, raw_attr: &str, base: Option<&Url>, doc_file: &Path) {
    let raw = trim_attr(raw_attr);
    if skip_non_fetchable(raw) {
        return;
    }
    if let Some(res) = merge_with_base(raw, base, doc_file) {
        out.push(HtmlLink {
            raw: raw.to_string(),
            resolved: res,
        });
    }
}

/// 与 GNU `tag_handle_base` 一致：`parent` 为 `-B`；无 parent 时 `href` 须自洽为可解析 URL。
fn apply_base_href(parent: Option<&Url>, href: &str) -> Option<Url> {
    let href = trim_attr(href);
    if href.is_empty() {
        return None;
    }
    if let Some(p) = parent {
        p.join(href).ok()
    } else {
        Url::parse(href).ok()
    }
}

/// `<meta http-equiv="refresh" content="0; url=...">`（对齐 GNU 解析：先分号再找 `URL=`）。
fn try_meta_refresh(content: &str) -> Option<&str> {
    let c = trim_attr(content);
    let semi = c.find(';')?;
    let n = c[..semi].trim().parse::<i64>().ok()?;
    if n < 0 {
        return None;
    }
    let p = c[semi + 1..].trim_start();
    let b = p.as_bytes();
    let key = b"url=";
    let mut i = 0usize;
    while i + key.len() <= b.len() {
        if b[i..i + key.len()].eq_ignore_ascii_case(key) {
            let u = p[i + key.len()..].trim();
            return Some(u.trim_matches(|q| q == '"' || q == '\''));
        }
        i += 1;
    }
    None
}

fn push_if_some(out: &mut Vec<String>, m: Option<String>) {
    if let Some(s) = m {
        if !s.is_empty() {
            out.push(s);
        }
    }
}

/// `parent_base`：CLI `-B`（对齐 `get_urls_html(..., url=NULL)` 时仅 `opt.base_href`）。
/// `source_path`：用于日志（`-` 表示 stdin）。
pub fn extract_urls_from_html(
    html: &str,
    parent_base: Option<&Url>,
    source_path: &Path,
) -> Vec<String> {
    let document = Html::parse_document(html);
    let sel = match Selector::parse("*") {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };

    let mut out: Vec<String> = Vec::new();
    // 文档内 `<base href>`（HTML5：仅第一个生效）
    let mut document_base: Option<Url> = None;
    let mut base_element_seen = false;

    for el in document.select(&sel) {
        let tag = el.value().name().to_ascii_lowercase();

        if tag == "base" {
            if base_element_seen {
                continue;
            }
            if let Some(href) = el.value().attr("href") {
                if let Some(u) = apply_base_href(parent_base, href) {
                    document_base = Some(u);
                    base_element_seen = true;
                }
            }
            continue;
        }

        let effective_base = document_base.as_ref().or(parent_base);

        match tag.as_str() {
            "a" | "area" => {
                if let Some(h) = el.value().attr("href") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "frame" | "iframe" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "img" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
                if let Some(h) = el.value().attr("href") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
                if let Some(h) = el.value().attr("lowsrc") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "embed" => {
                for attr in ["src", "href"] {
                    if let Some(h) = el.value().attr(attr) {
                        push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                    }
                }
            }
            "object" => {
                if let Some(h) = el.value().attr("data") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "script" | "bgsound" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "body" | "table" | "td" | "th" => {
                if let Some(h) = el.value().attr("background") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "input" | "video" | "audio" | "source" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
                if tag == "video" || tag == "audio" {
                    if let Some(h) = el.value().attr("poster") {
                        push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                    }
                }
            }
            "link" => {
                if let Some(h) = el.value().attr("href") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "form" => {
                // GNU：`form action` 进列表但 `ignore_when_downloading`，此处不入队。
            }
            "meta" => {
                let http_equiv = el
                    .value()
                    .attr("http-equiv")
                    .map(|s| s.to_ascii_lowercase());
                let Some(he) = http_equiv else { continue };
                if he != "refresh" {
                    continue;
                }
                let Some(content) = el.value().attr("content") else {
                    continue;
                };
                if let Some(u) = try_meta_refresh(content) {
                    push_if_some(&mut out, merge_with_base(u, effective_base, source_path));
                }
            }
            _ => {}
        }
    }

    out
}

/// 递归“跟随页面链接”子集：只抽取会导致“跳转到另一个页面”的链接（不含图片/脚本/css 等资源）。
pub fn extract_follow_urls_from_html(
    html: &str,
    parent_base: Option<&Url>,
    source_path: &Path,
) -> Vec<String> {
    let document = Html::parse_document(html);
    let sel = match Selector::parse("*") {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };

    let mut out: Vec<String> = Vec::new();
    let mut document_base: Option<Url> = None;
    let mut base_element_seen = false;

    for el in document.select(&sel) {
        let tag = el.value().name().to_ascii_lowercase();

        if tag == "base" {
            if base_element_seen {
                continue;
            }
            if let Some(href) = el.value().attr("href") {
                if let Some(u) = apply_base_href(parent_base, href) {
                    document_base = Some(u);
                    base_element_seen = true;
                }
            }
            continue;
        }

        let effective_base = document_base.as_ref().or(parent_base);
        match tag.as_str() {
            "a" | "area" => {
                if let Some(h) = el.value().attr("href") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "frame" | "iframe" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "meta" => {
                let http_equiv = el
                    .value()
                    .attr("http-equiv")
                    .map(|s| s.to_ascii_lowercase());
                let Some(he) = http_equiv else { continue };
                if he != "refresh" {
                    continue;
                }
                let Some(content) = el.value().attr("content") else {
                    continue;
                };
                if let Some(u) = try_meta_refresh(content) {
                    push_if_some(&mut out, merge_with_base(u, effective_base, source_path));
                }
            }
            _ => {}
        }
    }
    out
}

/// `-p/--page-requisites`：抽取用于渲染页面的资源 URL（不含 `<a href>` 跳转）。
pub fn extract_requisite_urls_from_html(
    html: &str,
    parent_base: Option<&Url>,
    source_path: &Path,
) -> Vec<String> {
    let document = Html::parse_document(html);
    let sel = match Selector::parse("*") {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };

    let mut out: Vec<String> = Vec::new();
    let mut document_base: Option<Url> = None;
    let mut base_element_seen = false;

    for el in document.select(&sel) {
        let tag = el.value().name().to_ascii_lowercase();

        if tag == "base" {
            if base_element_seen {
                continue;
            }
            if let Some(href) = el.value().attr("href") {
                if let Some(u) = apply_base_href(parent_base, href) {
                    document_base = Some(u);
                    base_element_seen = true;
                }
            }
            continue;
        }

        let effective_base = document_base.as_ref().or(parent_base);
        match tag.as_str() {
            "img" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
                if let Some(h) = el.value().attr("lowsrc") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "script" | "bgsound" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "link" => {
                if let Some(h) = el.value().attr("href") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "embed" => {
                for attr in ["src", "href"] {
                    if let Some(h) = el.value().attr(attr) {
                        push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                    }
                }
            }
            "object" => {
                if let Some(h) = el.value().attr("data") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "body" | "table" | "td" | "th" => {
                if let Some(h) = el.value().attr("background") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
            }
            "input" | "video" | "audio" | "source" => {
                if let Some(h) = el.value().attr("src") {
                    push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                }
                if tag == "video" || tag == "audio" {
                    if let Some(h) = el.value().attr("poster") {
                        push_if_some(&mut out, merge_with_base(h, effective_base, source_path));
                    }
                }
            }
            _ => {}
        }
    }
    out
}

/// 递归下载用：一次解析返回 Follow/Inline 两类链接（对齐 GNU recur.c 的 link_inline_p 思路）。
pub fn extract_recursive_links_from_html(
    html: &str,
    parent_base: Option<&Url>,
    source_path: &Path,
) -> Vec<RecLink> {
    let document = Html::parse_document(html);
    let sel = match Selector::parse("*") {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };

    let mut out: Vec<RecLink> = Vec::new();
    let mut document_base: Option<Url> = None;
    let mut base_element_seen = false;

    let mut push = |raw_attr: &str, kind: LinkKind, base: Option<&Url>| {
        let raw = trim_attr(raw_attr);
        if skip_non_fetchable(raw) {
            return;
        }
        let link_relative_p = !looks_absolute(raw);
        if let Some(u) = merge_with_base(raw, base, source_path) {
            out.push(RecLink {
                resolved: u,
                kind,
                raw: raw.to_string(),
                link_relative_p,
            });
        }
    };

    for el in document.select(&sel) {
        let tag = el.value().name().to_ascii_lowercase();

        if tag == "base" {
            if base_element_seen {
                continue;
            }
            if let Some(href) = el.value().attr("href") {
                if let Some(u) = apply_base_href(parent_base, href) {
                    document_base = Some(u);
                    base_element_seen = true;
                }
            }
            continue;
        }

        let effective_base = document_base.as_ref().or(parent_base);
        match tag.as_str() {
            // follow
            "a" | "area" => {
                if let Some(h) = el.value().attr("href") {
                    push(h, LinkKind::Follow, effective_base);
                }
            }
            "meta" => {
                let http_equiv = el
                    .value()
                    .attr("http-equiv")
                    .map(|s| s.to_ascii_lowercase());
                let Some(he) = http_equiv else { continue };
                if he != "refresh" {
                    continue;
                }
                let Some(content) = el.value().attr("content") else {
                    continue;
                };
                if let Some(u) = try_meta_refresh(content) {
                    push(u, LinkKind::Follow, effective_base);
                }
            }

            // inline resources (including frames; GNU treats them as inline for -p depth allowance)
            "frame" | "iframe" => {
                if let Some(h) = el.value().attr("src") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "img" => {
                if let Some(h) = el.value().attr("src") {
                    push(h, LinkKind::Inline, effective_base);
                }
                if let Some(h) = el.value().attr("lowsrc") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "script" | "bgsound" => {
                if let Some(h) = el.value().attr("src") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "link" => {
                if let Some(h) = el.value().attr("href") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "embed" => {
                for attr in ["src", "href"] {
                    if let Some(h) = el.value().attr(attr) {
                        push(h, LinkKind::Inline, effective_base);
                    }
                }
            }
            "object" => {
                if let Some(h) = el.value().attr("data") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "body" | "table" | "td" | "th" => {
                if let Some(h) = el.value().attr("background") {
                    push(h, LinkKind::Inline, effective_base);
                }
            }
            "input" | "video" | "audio" | "source" => {
                if let Some(h) = el.value().attr("src") {
                    push(h, LinkKind::Inline, effective_base);
                }
                if tag == "video" || tag == "audio" {
                    if let Some(h) = el.value().attr("poster") {
                        push(h, LinkKind::Inline, effective_base);
                    }
                }
            }
            _ => {}
        }
    }
    out
}

pub fn meta_robots_nofollow(html: &str) -> bool {
    // 对齐 GNU html-url.c：<meta name="robots" content="none|...,nofollow,...">
    let document = Html::parse_document(html);
    let sel = Selector::parse("meta").ok();
    let Some(sel) = sel else { return false };
    for el in document.select(&sel) {
        let name = el.value().attr("name").unwrap_or("").to_ascii_lowercase();
        if name != "robots" {
            continue;
        }
        let content = el
            .value()
            .attr("content")
            .unwrap_or("")
            .to_ascii_lowercase();
        if content == "none" {
            return true;
        }
        // split on commas and whitespace
        for tok in content.split(|c: char| c == ',' || c.is_whitespace()) {
            if tok.trim() == "nofollow" {
                return true;
            }
        }
    }
    false
}

/// 与 `extract_urls_from_html` 相同的抽取子集，但保留「原 attribute 值」用于 `-k/--convert-links` 改写。
pub fn extract_links_from_html(
    html: &str,
    parent_base: Option<&Url>,
    source_path: &Path,
) -> Vec<HtmlLink> {
    let document = Html::parse_document(html);
    let sel = match Selector::parse("*") {
        Ok(s) => s,
        Err(_) => return Vec::new(),
    };

    let mut out: Vec<HtmlLink> = Vec::new();
    let mut document_base: Option<Url> = None;
    let mut base_element_seen = false;

    for el in document.select(&sel) {
        let tag = el.value().name().to_ascii_lowercase();

        if tag == "base" {
            if base_element_seen {
                continue;
            }
            if let Some(href) = el.value().attr("href") {
                if let Some(u) = apply_base_href(parent_base, href) {
                    document_base = Some(u);
                    base_element_seen = true;
                }
            }
            continue;
        }

        let effective_base = document_base.as_ref().or(parent_base);

        match tag.as_str() {
            "a" | "area" => {
                if let Some(h) = el.value().attr("href") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "frame" | "iframe" => {
                if let Some(h) = el.value().attr("src") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "img" => {
                if let Some(h) = el.value().attr("src") {
                    push_link(&mut out, h, effective_base, source_path);
                }
                if let Some(h) = el.value().attr("href") {
                    push_link(&mut out, h, effective_base, source_path);
                }
                if let Some(h) = el.value().attr("lowsrc") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "embed" => {
                for attr in ["src", "href"] {
                    if let Some(h) = el.value().attr(attr) {
                        push_link(&mut out, h, effective_base, source_path);
                    }
                }
            }
            "object" => {
                if let Some(h) = el.value().attr("data") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "script" | "bgsound" => {
                if let Some(h) = el.value().attr("src") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "body" | "table" | "td" | "th" => {
                if let Some(h) = el.value().attr("background") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "input" | "video" | "audio" | "source" => {
                if let Some(h) = el.value().attr("src") {
                    push_link(&mut out, h, effective_base, source_path);
                }
                if tag == "video" || tag == "audio" {
                    if let Some(h) = el.value().attr("poster") {
                        push_link(&mut out, h, effective_base, source_path);
                    }
                }
            }
            "link" => {
                if let Some(h) = el.value().attr("href") {
                    push_link(&mut out, h, effective_base, source_path);
                }
            }
            "meta" => {
                let http_equiv = el
                    .value()
                    .attr("http-equiv")
                    .map(|s| s.to_ascii_lowercase());
                let Some(he) = http_equiv else { continue };
                if he != "refresh" {
                    continue;
                }
                let Some(content) = el.value().attr("content") else {
                    continue;
                };
                if let Some(u) = try_meta_refresh(content) {
                    push_link(&mut out, u, effective_base, source_path);
                }
            }
            _ => {}
        }
    }

    out
}
