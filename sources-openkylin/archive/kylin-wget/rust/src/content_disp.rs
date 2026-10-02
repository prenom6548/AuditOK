//! 从 `Content-Disposition` 解析建议本地文件名（`--content-disposition`；对齐 GNU `http.c:parse_content_disposition`）。

use reqwest::header::{CONTENT_DISPOSITION, HeaderMap};
use std::collections::BTreeMap;

use encoding_rs::Encoding;

/// 解析 `Content-Disposition` 并返回建议文件名（不含路径）。
///
/// 对齐点（GNU）：
/// - `filename*`（RFC 6266/RFC 5987）优先于 `filename`
/// - `filename*0=...; filename*1=...`（RFC 2231 continuation）会拼接
/// - 若同时存在 `filename="A"` 与 `filename*="B"`，选择 `B`
pub fn preferred_filename(headers: &HeaderMap) -> Option<String> {
    let raw = headers.get(CONTENT_DISPOSITION)?.to_str().ok()?;
    parse_content_disposition(raw)
}

fn parse_content_disposition(hdr: &str) -> Option<String> {
    let params = split_params(hdr);

    // unencoded: filename=...
    let mut unencoded: Option<String> = None;
    // encoded single: filename*=...
    let mut encoded_single: Option<String> = None;
    // encoded continuation: filename*0=... filename*1=...
    let mut encoded_cont: BTreeMap<usize, String> = BTreeMap::new();
    // unencoded continuation (rare, but GNU allows append_value_to_filename regardless)
    let mut unencoded_cont: BTreeMap<usize, String> = BTreeMap::new();

    for (k_raw, v_raw) in params {
        let key = k_raw.to_ascii_lowercase();
        if !key.starts_with("filename") {
            continue;
        }
        let val = strip_quotes(v_raw.trim());

        if key == "filename" {
            if !val.is_empty() {
                unencoded = Some(val.to_string());
            }
            continue;
        }

        if key == "filename*" {
            if let Some(s) = decode_rfc5987_value(val) {
                encoded_single = Some(s);
            } else if !val.is_empty() {
                // fallback: treat as raw, unescape %HH
                encoded_single = Some(percent_decode_token(val));
            }
            continue;
        }

        // filename*0 / filename*1 / filename*0* (encoded segment) ...
        if let Some(idx) = key.strip_prefix("filename*") {
            let mut num_part = idx;
            let mut is_encoded_segment = false;
            if let Some(rest) = idx.strip_suffix('*') {
                is_encoded_segment = true;
                num_part = rest;
            }
            if let Ok(n) = num_part.parse::<usize>() {
                if is_encoded_segment {
                    // per RFC2231, encoded segments are percent-encoded octets
                    encoded_cont.insert(n, percent_decode_token(val));
                } else {
                    unencoded_cont.insert(n, val.to_string());
                }
            }
            continue;
        }
    }

    let chosen = if let Some(s) = encoded_single {
        Some(s)
    } else if !encoded_cont.is_empty() {
        Some(join_continuations(encoded_cont))
    } else if !unencoded_cont.is_empty() {
        Some(join_continuations(unencoded_cont))
    } else {
        unencoded
    }?;

    let safe = sanitize_local_name(&chosen);
    if safe.is_empty() { None } else { Some(safe) }
}

fn join_continuations(parts: BTreeMap<usize, String>) -> String {
    let mut out = String::new();
    for (_i, s) in parts {
        out.push_str(&s);
    }
    out
}

fn split_params(hdr: &str) -> Vec<(String, String)> {
    // Split by ';' but honor quoted strings.
    let mut out: Vec<(String, String)> = Vec::new();
    let mut cur = String::new();
    let mut in_quote = false;
    let mut quote_ch = '\0';
    let mut prev_bs = false;

    for ch in hdr.chars() {
        if prev_bs {
            cur.push(ch);
            prev_bs = false;
            continue;
        }
        if ch == '\\' {
            // keep escapes inside quotes similar to GNU tokenizer behavior
            prev_bs = true;
            continue;
        }
        if in_quote {
            if ch == quote_ch {
                in_quote = false;
                cur.push(ch);
            } else {
                cur.push(ch);
            }
            continue;
        }
        if ch == '"' || ch == '\'' {
            in_quote = true;
            quote_ch = ch;
            cur.push(ch);
            continue;
        }
        if ch == ';' {
            push_param(&mut out, &cur);
            cur.clear();
            continue;
        }
        cur.push(ch);
    }
    push_param(&mut out, &cur);
    out
}

fn push_param(out: &mut Vec<(String, String)>, raw: &str) {
    let t = raw.trim();
    if t.is_empty() {
        return;
    }
    let Some(eq) = t.find('=') else {
        return;
    };
    let k = t[..eq].trim();
    let v = t[eq + 1..].trim();
    if k.is_empty() {
        return;
    }
    out.push((k.to_string(), v.to_string()));
}

fn strip_quotes(s: &str) -> &str {
    let t = s.trim();
    if t.len() >= 2 {
        let b = t.as_bytes();
        if (b[0] == b'"' && b[t.len() - 1] == b'"') || (b[0] == b'\'' && b[t.len() - 1] == b'\'') {
            return &t[1..t.len() - 1];
        }
    }
    t
}

fn decode_rfc5987_value(v: &str) -> Option<String> {
    // charset'lang'%XX...
    let (cs, rest) = v.split_once('\'')?;
    let (_lang, enc) = rest.split_once('\'')?;
    let bytes = percent_decode_bytes(enc);
    let enc = Encoding::for_label(cs.trim().as_bytes()).unwrap_or(encoding_rs::UTF_8);
    let (cow, _used, _had_err) = enc.decode(&bytes);
    Some(cow.into_owned())
}

fn hex_val(c: u8) -> Option<u8> {
    match c {
        b'0'..=b'9' => Some(c - b'0'),
        b'a'..=b'f' => Some(c - b'a' + 10),
        b'A'..=b'F' => Some(c - b'A' + 10),
        _ => None,
    }
}

fn percent_decode_bytes(s: &str) -> Vec<u8> {
    let b = s.as_bytes();
    let mut out = Vec::with_capacity(s.len());
    let mut i = 0usize;
    while i < b.len() {
        if b[i] == b'%' && i + 2 < b.len() {
            if let (Some(a), Some(bb)) = (hex_val(b[i + 1]), hex_val(b[i + 2])) {
                out.push(a * 16 + bb);
                i += 3;
                continue;
            }
        }
        out.push(b[i]);
        i += 1;
    }
    out
}

fn percent_decode_token(s: &str) -> String {
    String::from_utf8_lossy(&percent_decode_bytes(s)).into_owned()
}

/// 仅保留最后路径段，丢弃 `..` / 空。
pub fn sanitize_local_name(name: &str) -> String {
    let t = name.trim().trim_matches(|c| c == '"' || c == '\'');
    if t.is_empty() {
        return String::new();
    }
    let base = std::path::Path::new(t)
        .file_name()
        .and_then(|x| x.to_str())
        .unwrap_or(t);
    if base.is_empty() || base == "." || base == ".." {
        return String::new();
    }
    base.to_string()
}
