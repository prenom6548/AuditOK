//! 根据 `LANG` / `LC_ALL` / `LC_MESSAGES` 选择 gettext catalog；默认英文。

use std::sync::OnceLock;

static CATALOG: OnceLock<&'static str> = OnceLock::new();

/// gettext catalog 名（对应 `po/<name>.po` 文件名；无对应 po 时由 `i18n` 回退 msgid）。
pub fn catalog_name() -> &'static str {
    *CATALOG.get_or_init(|| {
        let s = detect_catalog();
        Box::leak(s.into_boxed_str())
    })
}

/// 当前是否为中文环境（`zh*`）。
#[inline]
pub fn is_zh() -> bool {
    let c = catalog_name();
    c == "zh_CN" || c.starts_with("zh_") || c == "zh"
}

fn detect_catalog() -> String {
    for key in ["LC_ALL", "LC_MESSAGES", "LANG"] {
        if let Ok(v) = std::env::var(key) {
            if let Some(c) = normalize_locale(&v) {
                return c;
            }
        }
    }
    "C".to_string()
}

/// `zh_CN.UTF-8` → `zh_CN`；`C`/`POSIX`/空 → `C`；`en_US` → `C`（英文不查表）。
fn normalize_locale(s: &str) -> Option<String> {
    let base = s.split('.').next().unwrap_or(s).trim();
    if base.is_empty() || base == "C" || base.eq_ignore_ascii_case("POSIX") {
        return Some("C".to_string());
    }
    let canon = base.replace('-', "_");
    let low = canon.to_ascii_lowercase();
    if low == "en" || low.starts_with("en_") {
        return Some("C".to_string());
    }
    if low == "zh" || low.starts_with("zh_") {
        return Some(if low == "zh" {
            "zh_CN".to_string()
        } else {
            canon
        });
    }
    Some(canon)
}
