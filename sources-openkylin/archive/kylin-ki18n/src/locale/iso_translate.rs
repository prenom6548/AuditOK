// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

//! gettext 风格的 ISO 3166-1 / 3166-2 名称翻译（与上游 `i18nd("iso_3166-*", ...)` 对齐）。

use parking_lot::RwLock;
use std::collections::HashMap;

use once_cell::sync::Lazy;

use crate::core::catalog::Catalog;
use crate::core::global as core_global;

static MO_CACHE: Lazy<RwLock<HashMap<(String, String), Option<Catalog>>>> =
    Lazy::new(|| RwLock::new(HashMap::new()));

fn effective_lang_chain() -> Vec<String> {
    let gl = core_global::get_languages();
    if !gl.is_empty() {
        return gl;
    }
    if let Ok(l) = std::env::var("LANGUAGE") {
        let v: Vec<String> = l
            .split(':')
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty())
            .collect();
        if !v.is_empty() {
            return v;
        }
    }
    if let Ok(l) = std::env::var("LANG") {
        let base = l
            .split('.')
            .next()
            .unwrap_or("")
            .split('@')
            .next()
            .unwrap_or("")
            .to_string();
        if !base.is_empty() {
            return vec![base];
        }
    }
    Vec::new()
}

fn expand_lang_candidates(lang: &str) -> Vec<String> {
    let mut v = vec![lang.to_string()];
    if let Some((a, _)) = lang.split_once('_') {
        if a != lang {
            v.push(a.to_string());
        }
    }
    v
}

fn try_load_mo(domain: &str, lang: &str) -> Option<Catalog> {
    if let Some(c) = core_global::get_or_load_catalog(domain, lang) {
        return Some(c);
    }
    for root in ["/usr/share/locale", "/usr/local/share/locale"] {
        let path = std::path::Path::new(root)
            .join(lang)
            .join("LC_MESSAGES")
            .join(format!("{}.mo", domain));
        if path.is_file() {
            return Catalog::load_mo(&path, Some(lang)).ok();
        }
    }
    None
}

fn load_mo_cached(domain: &str, lang: &str) -> Option<Catalog> {
    let key = (domain.to_string(), lang.to_string());
    {
        let g = MO_CACHE.read();
        if let Some(e) = g.get(&key) {
            return e.clone();
        }
    }
    let cat = try_load_mo(domain, lang);
    MO_CACHE.write().insert(key, cat.clone());
    cat
}

/// 在指定翻译域中解析 `msgid`；无可用语言或找不到翻译时返回 `msgid`。
pub fn translate_iso_message(domain: &str, msgid: &str) -> String {
    if msgid.is_empty() {
        return String::new();
    }
    let chain = effective_lang_chain();
    if chain.is_empty() {
        return msgid.to_string();
    }
    for lang in chain {
        for cand in expand_lang_candidates(&lang) {
            if let Some(cat) = load_mo_cached(domain, &cand) {
                if let Some(entry) = cat.get_message(msgid) {
                    if let Some(t) = entry.msgstr.get(0) {
                        if !t.is_empty() {
                            return t.clone();
                        }
                    }
                }
            }
        }
    }
    msgid.to_string()
}
