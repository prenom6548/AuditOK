//! GNU gettext 风格国际化：源码里写英文 msgid，译文维护在 `po/*.po`。

include!(concat!(env!("OUT_DIR"), "/i18n_catalogs.rs"));

use crate::locale;
use std::sync::OnceLock;

static DOMAIN: OnceLock<&'static str> = OnceLock::new();

/// 返回当前语言的译文；无对应 po 或无译文时回退 msgid（英文）。
#[inline]
pub fn gettext(msgid: &'static str) -> &'static str {
    let lang = locale::catalog_name();
    if lang == "C" {
        return msgid;
    }
    if let Some(t) = lookup_catalog(lang, msgid) {
        return t;
    }
    if lang.starts_with("zh") {
        if let Some(t) = lookup_catalog("zh_CN", msgid) {
            return t;
        }
    }
    msgid
}

/// 带 `{name}` 占位符的译文（msgid 与 po 中须使用相同占位符名）。
pub fn tr_named(msgid: &'static str, replacements: &[(&str, &str)]) -> String {
    let mut s = gettext(msgid).to_string();
    for (key, value) in replacements {
        s = s.replace(&format!("{{{key}}}"), value);
    }
    s
}

/// 设置 gettext domain（与 GNU `TEXTDOMAIN` 类似，默认 `kylin-wget`）。
pub fn textdomain(domain: &'static str) -> &'static str {
    *DOMAIN.get_or_init(|| domain)
}

fn lookup_catalog(lang: &str, msgid: &str) -> Option<&'static str> {
    let domain = DOMAIN.get().copied().unwrap_or("kylin-wget");
    let _ = domain;
    for cat in CATALOGS {
        if cat.lang != lang {
            continue;
        }
        for (id, tr) in cat.pairs {
            if *id == msgid {
                return Some(tr);
            }
        }
    }
    None
}

/// 宏：`_i18n!(msgid)`，用法接近 GNU `_()`。
#[macro_export]
macro_rules! _i18n {
    ($msgid:expr) => {
        $crate::i18n::gettext($msgid)
    };
}
