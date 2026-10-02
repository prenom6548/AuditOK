use url::Url;

fn skip_non_fetchable(raw: &str) -> bool {
    let t = raw.trim();
    if t.is_empty() || t.starts_with('#') {
        return true;
    }
    let low = t.to_ascii_lowercase();
    low.starts_with("data:")
        || low.starts_with("javascript:")
        || low.starts_with("mailto:")
        || low.starts_with("tel:")
}

fn merge_with_base(raw: &str, base: &Url) -> Option<String> {
    let t = raw.trim().trim_matches(|c| c == '"' || c == '\'');
    if skip_non_fetchable(t) {
        return None;
    }
    if t.contains("://") || t.starts_with("//") {
        if t.starts_with("//") {
            let scheme = base.scheme();
            return Url::parse(&format!("{scheme}:{t}"))
                .ok()
                .map(|u| u.to_string());
        }
        return Url::parse(t).ok().map(|u| u.to_string());
    }
    base.join(t).ok().map(|u| u.to_string())
}

/// 从 CSS 文本抽取 URL（覆盖 `url(...)` 与 `@import` 的常用写法）。
pub fn extract_urls_from_css(css: &str, base_url: &Url) -> Vec<String> {
    let mut out: Vec<String> = Vec::new();
    let bytes = css.as_bytes();
    let mut i = 0usize;
    while i < bytes.len() {
        // naive scan for "url(" or "@import"
        if bytes[i..].starts_with(b"url(") || bytes[i..].starts_with(b"URL(") {
            i += 4;
            let mut j = i;
            while j < bytes.len() && bytes[j].is_ascii_whitespace() {
                j += 1;
            }
            let mut k = j;
            let mut depth = 1i32;
            while k < bytes.len() {
                let c = bytes[k] as char;
                if c == '(' {
                    depth += 1;
                } else if c == ')' {
                    depth -= 1;
                    if depth == 0 {
                        break;
                    }
                }
                k += 1;
            }
            if k < bytes.len() {
                let raw = &css[j..k];
                if let Some(u) = merge_with_base(raw, base_url) {
                    out.push(u);
                }
                i = k + 1;
                continue;
            }
        }
        if bytes[i..].starts_with(b"@import") || bytes[i..].starts_with(b"@IMPORT") {
            i += 7;
            while i < bytes.len() && bytes[i].is_ascii_whitespace() {
                i += 1;
            }
            // allow url(...) or quoted string
            if i < bytes.len()
                && (bytes[i..].starts_with(b"url(") || bytes[i..].starts_with(b"URL("))
            {
                continue;
            }
            let start = i;
            while i < bytes.len() {
                let c = bytes[i] as char;
                if c == ';' || c == '\n' || c == '\r' {
                    break;
                }
                i += 1;
            }
            let raw = css[start..i].trim();
            if let Some(u) = merge_with_base(raw, base_url) {
                out.push(u);
            }
            continue;
        }
        i += 1;
    }
    out
}
