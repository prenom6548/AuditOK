// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::{HashMap, HashSet};
use std::path::{Path, PathBuf};

use once_cell::sync::Lazy;
use parking_lot::RwLock;

use crate::core::catalog::Catalog;

static GLOBAL_STATE: Lazy<RwLock<GlobalState>> = Lazy::new(|| RwLock::new(GlobalState::new()));

#[derive(Debug)]
pub struct GlobalState {
    application_domain: Option<String>,
    /// 显式覆盖语言列表；为空时使用 `locale_languages`。
    languages: Vec<String>,
    /// 启动时从环境变量收集的语言链（与 ki18n `localeLanguages` 一致）。
    locale_languages: Vec<String>,
    locale_languages_initialized: bool,
    /// 与上游 `KLocalizedStringPrivateStatics::codeLanguage` 一致：到达此语言时不再查 catalog。
    code_language: String,
    /// `translateQt` 使用的 domain 栈（前部优先）；每项为 (domain, 引用计数)。
    qt_domains: Vec<(String, u32)>,
    /// domain -> set of search paths
    domain_paths: HashMap<String, HashSet<PathBuf>>,
    /// (domain, lang) -> Catalog
    catalogs: HashMap<(String, String), Catalog>,
    /// `available_languages(domain)` 扫描结果缓存；`add_domain_locale_dir` 时按 domain 失效。
    available_languages_cache: HashMap<String, Vec<String>>,
}

impl GlobalState {
    pub fn new() -> Self {
        GlobalState {
            application_domain: None,
            languages: Vec::new(),
            locale_languages: Vec::new(),
            locale_languages_initialized: false,
            code_language: "en_US".to_string(),
            qt_domains: Vec::new(),
            domain_paths: HashMap::new(),
            catalogs: HashMap::new(),
            available_languages_cache: HashMap::new(),
        }
    }
}

/// 源码/回退语言标签（默认 `en_US`，与 KDE `KLocalizedStringPrivateStatics` 一致）。
pub fn code_language() -> String {
    GLOBAL_STATE.read().code_language.clone()
}

pub fn insert_qt_domain(domain: &str) {
    if domain.is_empty() {
        return;
    }
    let mut state = GLOBAL_STATE.write();
    if let Some((_, c)) = state.qt_domains.iter_mut().find(|(d, _)| d == domain) {
        *c += 1;
    } else {
        state.qt_domains.insert(0, (domain.to_string(), 1));
    }
}

pub fn remove_qt_domain(domain: &str) {
    if domain.is_empty() {
        return;
    }
    let mut state = GLOBAL_STATE.write();
    if let Some(pos) = state.qt_domains.iter().position(|(d, _)| d == domain) {
        if state.qt_domains[pos].1 <= 1 {
            state.qt_domains.remove(pos);
        } else {
            state.qt_domains[pos].1 -= 1;
        }
    }
}

fn catalog_lookup_key(msgctxt: Option<&str>, msgid: &str) -> String {
    match msgctxt {
        // gettext 上下文键：msgctxt + '\x04' + msgid
        Some(c) if !c.is_empty() => format!("{}\u{4}{}", c, msgid),
        _ => msgid.to_string(),
    }
}

/// 与 `KLocalizedStringPrivate::translateRaw` 对齐的查表语义（非 plural）。
pub fn translate_raw(
    domain: &str,
    languages: &[String],
    msgctxt: Option<&str>,
    msgid: &str,
) -> (String, String) {
    let code = code_language();
    let mut msgstr = msgid.to_string();
    let mut language = code.clone();

    if msgid.is_empty() {
        language.clear();
        msgstr.clear();
        return (language, msgstr);
    }
    if domain.is_empty() {
        return (language, msgstr);
    }

    for test_lang in languages {
        if test_lang == &code {
            continue;
        }
        if let Some(cat) = get_or_load_catalog(domain, test_lang) {
            let key = catalog_lookup_key(msgctxt, msgid);
            if let Some(t) = cat.get_translated_message(&key, 0) {
                if !t.is_empty() {
                    return (test_lang.clone(), t.to_string());
                }
            }
        }
    }

    (language, msgstr)
}

/// `KLocalizedString::translateQt`：仅在自 `code_language` 以外的语言命中 catalog 时返回译文，否则空串。
pub fn translate_qt(context: Option<&str>, source: &str, comment: Option<&str>) -> String {
    if source.is_empty() {
        return String::new();
    }
    let code = code_language();
    let langs = effective_languages();
    let domains: Vec<String> = GLOBAL_STATE
        .read()
        .qt_domains
        .iter()
        .map(|(d, _)| d.clone())
        .collect();

    for domain in domains {
        let (lang_out, tr) = if let Some(c) = comment.filter(|x| !x.is_empty()) {
            translate_raw(&domain, &langs, Some(c), source)
        } else {
            // 先尝试带上下文的查找
            if let Some(ctx) = context.filter(|x| !x.is_empty()) {
                let (l, t) = translate_raw(&domain, &langs, Some(ctx), source);
                if !l.is_empty() && l != code && !t.is_empty() {
                    return t;
                }
            }
            
            // 如果带上下文的查找失败，尝试不带上下文的查找
            let (language, translation) = translate_raw(&domain, &langs, None, source);
            
            (language, translation)
        };
        if !lang_out.is_empty() && lang_out != code {
            return tr;
        }
    }
    String::new()
}

pub fn set_application_domain(domain: &str) {
    let mut state = GLOBAL_STATE.write();
    state.application_domain = Some(domain.to_string());
}

pub fn get_application_domain() -> Option<String> {
    GLOBAL_STATE.read().application_domain.clone()
}

pub fn set_languages(langs: &[String]) {
    let mut state = GLOBAL_STATE.write();
    state.languages = langs.to_vec();
}

pub fn get_languages() -> Vec<String> {
    let state = GLOBAL_STATE.read();
    if !state.languages.is_empty() {
        return state.languages.clone();
    }
    drop(state);
    locale_languages()
}

fn push_unique(out: &mut Vec<String>, tag: &str) {
    if !tag.is_empty() && !out.iter().any(|x| x == tag) {
        out.push(tag.to_string());
    }
}

/// 分解 locale 为 `[language[_territory][.codeset][@modifier]]`（与 ki18n `splitLocale` 一致）。
fn split_locale(value: &str) -> (String, String, String) {
    let mut locale = value.trim();
    if let Some(i) = locale.find(':') {
        locale = &locale[..i];
    }
    let mut modifier = String::new();
    if let Some(i) = locale.find('@') {
        modifier = locale[i + 1..].to_string();
        locale = &locale[..i];
    }
    if let Some(i) = locale.find('.') {
        locale = &locale[..i];
    }
    let (language, country) = match locale.find('_') {
        Some(i) => (locale[..i].to_string(), locale[i + 1..].to_string()),
        None => (locale.to_string(), String::new()),
    };
    (language, country, modifier)
}

/// 将 locale 展开为候选语言标签（与 ki18n `appendLocaleString` 一致）。
fn append_locale_string(out: &mut Vec<String>, value: &str) {
    let (language, country, modifier) = split_locale(value);
    if language.is_empty() {
        return;
    }
    if !country.is_empty() && !modifier.is_empty() {
        push_unique(out, &format!("{language}_{country}@{modifier}"));
    }
    if !modifier.is_empty() {
        push_unique(out, &format!("{language}@{modifier}"));
    }
    if !country.is_empty() {
        push_unique(out, &format!("{language}_{country}"));
    }
    push_unique(out, &language);
}

fn locale_languages_from_env() -> Vec<String> {
    let mut out = Vec::new();
    if let Ok(v) = std::env::var("LANGUAGE") {
        for part in v.split(':') {
            if !part.is_empty() {
                append_locale_string(&mut out, part);
            }
        }
    }
    for key in ["LC_ALL", "LC_MESSAGES", "LANG"] {
        if let Ok(v) = std::env::var(key) {
            if !v.is_empty() {
                append_locale_string(&mut out, &v);
            }
        }
    }
    if out.is_empty() {
        if let Some(loc) = sys_locale::get_locale() {
            append_locale_string(&mut out, &loc);
        }
    }
    out
}

fn ensure_locale_languages_initialized(state: &mut GlobalState) {
    if state.locale_languages_initialized {
        return;
    }
    state.locale_languages = locale_languages_from_env();
    state.locale_languages_initialized = true;
}

/// 返回缓存的环境语言链（与 ki18n `localeLanguages` 一致）。
pub fn locale_languages() -> Vec<String> {
    let mut state = GLOBAL_STATE.write();
    ensure_locale_languages_initialized(&mut state);
    state.locale_languages.clone()
}

fn expand_explicit_languages(inputs: &[String]) -> Vec<String> {
    let mut out = Vec::new();
    for l in inputs {
        append_locale_string(&mut out, l);
    }
    out
}

/// 返回实际用于查表的语言优先级链。
///
/// 优先级：
/// 1) 显式 `set_languages`（展开后与 ki18n 相同）
/// 2) 缓存的 `locale_languages`（来自 `LANGUAGE`、`LC_ALL`、`LC_MESSAGES`、`LANG`）
pub fn effective_languages() -> Vec<String> {
    let state = GLOBAL_STATE.read();
    if !state.languages.is_empty() {
        return expand_explicit_languages(&state.languages);
    }
    drop(state);
    locale_languages()
}

pub fn clear_languages() {
    let mut state = GLOBAL_STATE.write();
    state.languages.clear();
}

/// 测试专用：使 `locale_languages` 在修改环境变量后重新从环境收集。
#[doc(hidden)]
pub fn reset_locale_languages_cache_for_tests() {
    let mut state = GLOBAL_STATE.write();
    state.locale_languages_initialized = false;
    state.locale_languages.clear();
}

pub fn add_domain_locale_dir(domain: &str, path: &Path) {
    let mut state = GLOBAL_STATE.write();
    let entry = state
        .domain_paths
        .entry(domain.to_string())
        .or_insert_with(HashSet::new);
    entry.insert(path.to_path_buf());
    state.available_languages_cache.remove(domain);
}

/// 在 `locale_root`（gettext 的 `.../locale`）下列出存在 `{domain}.mo` 的语言。
fn scan_locale_root_for_domain(locale_root: &Path, domain: &str, langs: &mut HashSet<String>) {
    if !locale_root.is_dir() {
        return;
    }
    let Ok(rd) = std::fs::read_dir(locale_root) else {
        return;
    };
    for entry in rd.flatten() {
        let mo = entry
            .path()
            .join("LC_MESSAGES")
            .join(format!("{domain}.mo"));
        if mo.is_file() {
            langs.insert(entry.file_name().to_string_lossy().to_string());
        }
    }
}

/// 从扁平 `.mo` 文件中提取语言代码。
fn try_add_flat_mo_lang(path: &Path, langs: &mut HashSet<String>) {
    if !path.is_file() {
        return;
    }
    if !path.extension().is_some_and(|e| e == "mo") {
        return;
    }
    if let Some(file_stem) = path.file_stem().and_then(|s| s.to_str()) {
        langs.insert(file_stem.to_string());
    }
}

/// 从 `<lang>/LC_MESSAGES/{domain}.mo` 目录结构中提取语言代码。
fn try_add_lc_messages_lang(dir_path: &Path, domain: &str, entry_name: &str, langs: &mut HashSet<String>) {
    if !dir_path.is_dir() {
        return;
    }
    let mo = dir_path.join("LC_MESSAGES").join(format!("{domain}.mo"));
    if mo.is_file() {
        langs.insert(entry_name.to_string());
    }
}

/// 扫描 `addDomainLocaleDir` 注册路径（扁平 `.mo`、`<lang>/LC_MESSAGES/`、`locale/<lang>/`）。
fn scan_registered_paths_for_domain(domain: &str, langs: &mut HashSet<String>) {
    let paths: Vec<PathBuf> = GLOBAL_STATE
        .read()
        .domain_paths
        .get(domain)
        .map(|set| set.iter().cloned().collect())
        .unwrap_or_default();

    for dir in paths {
        let Ok(rd) = std::fs::read_dir(&dir) else {
            continue;
        };
        for entry in rd.flatten() {
            let name = entry.file_name().to_string_lossy().to_string();
            try_add_flat_mo_lang(&entry.path(), langs);
            try_add_lc_messages_lang(&entry.path(), domain, &name, langs);
        }
        scan_locale_root_for_domain(&dir.join("locale"), domain, langs);
    }
}

/// 系统级 locale 根目录（XDG + /usr/share + /usr/local），去重后返回。
fn system_locale_roots() -> Vec<PathBuf> {
    let mut roots = Vec::new();
    let mut seen = HashSet::new();
    let mut push = |p: PathBuf| {
        if seen.insert(p.clone()) && p.is_dir() {
            roots.push(p);
        }
    };
    if let Ok(xdg) = std::env::var("XDG_DATA_DIRS") {
        for part in xdg.split(':') {
            let t = part.trim();
            if !t.is_empty() {
                push(Path::new(t).join("locale"));
            }
        }
    }
    push(PathBuf::from("/usr/share/locale"));
    push(PathBuf::from("/usr/local/share/locale"));
    roots
}

fn available_languages_uncached(domain: &str) -> Vec<String> {
    let mut langs = HashSet::new();
    scan_registered_paths_for_domain(domain, &mut langs);
    for locale_root in system_locale_roots() {
        scan_locale_root_for_domain(&locale_root, domain, &mut langs);
    }
    let mut list: Vec<String> = langs.into_iter().collect();
    list.sort();
    list
}

/// 枚举给定 domain 已安装翻译的语言列表（结果按 domain 缓存）。
pub fn available_languages(domain: &str) -> Vec<String> {
    {
        let state = GLOBAL_STATE.read();
        if let Some(cached) = state.available_languages_cache.get(domain) {
            return cached.clone();
        }
    }
    let list = available_languages_uncached(domain);
    let mut state = GLOBAL_STATE.write();
    state
        .available_languages_cache
        .insert(domain.to_string(), list.clone());
    list
}

/// 判断 `domain` 是否提供 `language` 的翻译（与上游 `isApplicationTranslatedInto` 语义一致）。
pub fn is_language_available(domain: &str, language: &str) -> bool {
    if domain.is_empty() {
        return false;
    }
    if language == code_language() {
        return true;
    }
    resolve_domain_mo_path(domain, language).is_some()
}

/// 返回给定 domain 已注册的 locale 根目录列表。
///
/// 这些路径由 `ki18n_rs_add_domain_locale_dir()` 注入，Rust 内部会把它们当作
/// “包含 `locale/<lang>/...` 目录结构的根目录”来尝试定位脚本/属性/翻译文件。
pub fn domain_locale_dirs(domain: &str) -> Vec<PathBuf> {
    let state = GLOBAL_STATE.read();
    state
        .domain_paths
        .get(domain)
        .map(|set| set.iter().cloned().collect())
        .unwrap_or_default()
}

/// 解析 `domain` + `lang` 对应的 `.mo` 路径（与 KCatalog / XDG_DATA_DIRS 布局一致）。
pub fn resolve_domain_mo_path(domain: &str, lang: &str) -> Option<PathBuf> {
    let state = GLOBAL_STATE.read();
    let mut roots: Vec<PathBuf> = state
        .domain_paths
        .get(domain)
        .map(|set| set.iter().cloned().collect())
        .unwrap_or_default();
    drop(state);

    if let Ok(xdg) = std::env::var("XDG_DATA_DIRS") {
        for part in xdg.split(':') {
            let t = part.trim();
            if !t.is_empty() {
                roots.push(PathBuf::from(t));
            }
        }
    }
    roots.push(PathBuf::from("/usr/share"));
    roots.push(PathBuf::from("/usr/local/share"));

    for base in roots {
        let flat = base.join(format!("{lang}.mo"));
        if flat.is_file() {
            return Some(flat);
        }
        let nested = base
            .join("locale")
            .join(lang)
            .join("LC_MESSAGES")
            .join(format!("{domain}.mo"));
        if nested.is_file() {
            return Some(nested);
        }
        let nested2 = base
            .join(lang)
            .join("LC_MESSAGES")
            .join(format!("{domain}.mo"));
        if nested2.is_file() {
            return Some(nested2);
        }
    }
    None
}

pub fn catalog_locale_dir(domain: &str, lang: &str) -> Option<PathBuf> {
    resolve_domain_mo_path(domain, lang)
}

pub fn get_or_load_catalog(domain: &str, lang: &str) -> Option<Catalog> {
    {
        let state = GLOBAL_STATE.read();
        if let Some(cat) = state.catalogs.get(&(domain.to_string(), lang.to_string())) {
            return Some(cat.clone());
        }
    }

    let path = resolve_domain_mo_path(domain, lang)?;
    let catalog = Catalog::load_mo(&path, Some(lang)).ok()?;

    let mut state = GLOBAL_STATE.write();
    state
        .catalogs
        .insert((domain.to_string(), lang.to_string()), catalog);
    state
        .catalogs
        .get(&(domain.to_string(), lang.to_string()))
        .cloned()
}

#[cfg(test)]
mod tests {
    use super::*;
    use once_cell::sync::Lazy;
    use std::fs;
    use std::sync::Mutex;
    use tempfile::tempdir;

    static TEST_LOCK: Lazy<Mutex<()>> = Lazy::new(|| Mutex::new(()));

    fn build_minimal_mo(plural_header: &str, entries: &[(&str, &str)]) -> Vec<u8> {
        let mut orig_strings: Vec<Vec<u8>> = Vec::new();
        let mut trans_strings: Vec<Vec<u8>> = Vec::new();
        let mut header = Vec::new();
        header.extend_from_slice(b"Content-Type: text/plain; charset=UTF-8\n");
        header.extend_from_slice(b"Plural-Forms: ");
        header.extend_from_slice(plural_header.as_bytes());
        header.extend_from_slice(b"\n");
        orig_strings.push(Vec::new());
        trans_strings.push(header);
        for (msgid, msgstr) in entries {
            orig_strings.push(msgid.as_bytes().to_vec());
            trans_strings.push(msgstr.as_bytes().to_vec());
        }
        let nstrings = orig_strings.len() as u32;
        let header_size: u32 = 28;
        let table_size: u32 = nstrings * 8 * 2;
        let orig_offset: u32 = header_size + table_size;
        let trans_offset: u32 =
            orig_offset + orig_strings.iter().map(|s| s.len() as u32 + 1).sum::<u32>();
        let mut file = Vec::new();
        file.extend_from_slice(&0x9504_12deu32.to_le_bytes());
        file.extend_from_slice(&0u32.to_le_bytes());
        file.extend_from_slice(&nstrings.to_le_bytes());
        file.extend_from_slice(&(header_size as u32).to_le_bytes());
        file.extend_from_slice(&((header_size + (nstrings * 8)) as u32).to_le_bytes());
        file.extend_from_slice(&0u32.to_le_bytes());
        file.extend_from_slice(&0u32.to_le_bytes());
        let mut cur_offset = orig_offset;
        for s in &orig_strings {
            let len = s.len() as u32;
            file.extend_from_slice(&len.to_le_bytes());
            file.extend_from_slice(&cur_offset.to_le_bytes());
            cur_offset += len + 1;
        }
        cur_offset = trans_offset;
        for s in &trans_strings {
            let len = s.len() as u32;
            file.extend_from_slice(&len.to_le_bytes());
            file.extend_from_slice(&cur_offset.to_le_bytes());
            cur_offset += len + 1;
        }
        for s in &orig_strings {
            file.extend_from_slice(s);
            file.push(0);
        }
        for s in &trans_strings {
            file.extend_from_slice(s);
            file.push(0);
        }
        file
    }

    #[test]
    fn translate_qt_empty_domains_returns_empty() {
        let _guard = TEST_LOCK.lock().expect("lock");
        clear_languages();
        let mut s = GLOBAL_STATE.write();
        s.qt_domains.clear();
        drop(s);
        assert_eq!(translate_qt(Some("Ctx"), "Hello", None), "");
    }

    #[test]
    fn translate_qt_french_hits_catalog() {
        let _guard = TEST_LOCK.lock().expect("lock");
        let dir = tempdir().expect("tempdir");
        let mo_path = dir.path().join("locale").join("fr").join("LC_MESSAGES");
        fs::create_dir_all(&mo_path).expect("mkdir");
        let mo_file = mo_path.join("qt-dom.mo");
        let mo_bytes = build_minimal_mo("nplurals=2; plural=(n != 1);", &[("Landscape", "Paysage")]);
        fs::write(&mo_file, &mo_bytes).expect("write mo");

        let mut s = GLOBAL_STATE.write();
        s.qt_domains.clear();
        s.domain_paths.clear();
        s.catalogs.clear();
        s.languages = vec!["fr".to_string(), "en_US".to_string()];
        drop(s);

        add_domain_locale_dir("qt-dom", dir.path());
        insert_qt_domain("qt-dom");

        let tr = translate_qt(Some("QPrintPreviewDialog"), "Landscape", None);
        assert_eq!(tr, "Paysage");

        remove_qt_domain("qt-dom");
        let mut s = GLOBAL_STATE.write();
        s.domain_paths.remove("qt-dom");
        s.catalogs.clear();
        s.available_languages_cache.clear();
        s.languages.clear();
        s.qt_domains.clear();
    }

    #[test]
    fn translate_qt_with_context_uses_eot_key() {
        let _guard = TEST_LOCK.lock().expect("lock");
        let dir = tempdir().expect("tempdir");
        let mo_path = dir.path().join("locale").join("fr").join("LC_MESSAGES");
        fs::create_dir_all(&mo_path).expect("mkdir");
        let mo_file = mo_path.join("qt-dom.mo");
        let mo_bytes = build_minimal_mo(
            "nplurals=2; plural=(n != 1);",
            &[("QPrintPreviewDialog\u{4}Landscape", "PaysageCtx")],
        );
        fs::write(&mo_file, &mo_bytes).expect("write mo");

        let mut s = GLOBAL_STATE.write();
        s.qt_domains.clear();
        s.domain_paths.clear();
        s.catalogs.clear();
        s.languages = vec!["fr".to_string(), "en_US".to_string()];
        drop(s);

        add_domain_locale_dir("qt-dom", dir.path());
        insert_qt_domain("qt-dom");

        let tr = translate_qt(Some("QPrintPreviewDialog"), "Landscape", None);
        assert_eq!(tr, "PaysageCtx");

        remove_qt_domain("qt-dom");
        let mut s = GLOBAL_STATE.write();
        s.domain_paths.remove("qt-dom");
        s.catalogs.clear();
        s.available_languages_cache.clear();
        s.languages.clear();
        s.qt_domains.clear();
    }

    #[test]
    fn available_languages_is_cached_per_domain() {
        let _guard = TEST_LOCK.lock().expect("lock");
        let dir = tempdir().expect("tempdir");
        let mo_path = dir.path().join("locale").join("fr").join("LC_MESSAGES");
        fs::create_dir_all(&mo_path).expect("mkdir");
        let mo_bytes = build_minimal_mo("nplurals=2; plural=(n != 1);", &[("x", "x")]);
        fs::write(mo_path.join("cache-dom.mo"), &mo_bytes).expect("write mo");

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.clear();
        s.available_languages_cache.clear();
        drop(s);

        add_domain_locale_dir("cache-dom", dir.path());
        let first = available_languages("cache-dom");
        assert!(first.contains(&"fr".to_string()));

        fs::remove_file(mo_path.join("cache-dom.mo")).expect("remove mo");
        let second = available_languages("cache-dom");
        assert_eq!(first, second);

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.remove("cache-dom");
        s.available_languages_cache.clear();
    }

    fn with_env<F: FnOnce()>(vars: &[(&str, &str)], f: F) {
        let _guard = TEST_LOCK.lock().expect("lock");
        reset_locale_languages_cache_for_tests();
        clear_languages();
        for (k, v) in vars {
            if v.is_empty() {
                std::env::remove_var(k);
            } else {
                std::env::set_var(k, v);
            }
        }
        f();
        for (k, _) in vars {
            std::env::remove_var(k);
        }
        reset_locale_languages_cache_for_tests();
        clear_languages();
    }

    #[test]
    fn effective_languages_reads_lc_messages_before_lang() {
        with_env(
            &[
                ("LANGUAGE", ""),
                ("LC_ALL", ""),
                ("LC_MESSAGES", "zh_TW.UTF-8"),
                ("LANG", "en_US.UTF-8"),
            ],
            || {
                let langs = effective_languages();
                assert!(
                    langs.first() == Some(&"zh_TW".to_string()),
                    "expected zh_TW first, got {langs:?}"
                );
            },
        );
    }

    #[test]
    fn effective_languages_expands_zh_hant_tw_like_ki18n() {
        with_env(
            &[
                ("LANGUAGE", ""),
                ("LC_ALL", ""),
                ("LC_MESSAGES", ""),
                ("LANG", "zh_Hant_TW.UTF-8"),
            ],
            || {
                let langs = effective_languages();
                // ki18n splitLocale：仅在首个 '_' 处分割 → zh + Hant_TW
                assert!(langs.contains(&"zh_Hant_TW".to_string()));
                assert!(langs.contains(&"zh".to_string()));
                assert!(!langs.contains(&"zh_TW".to_string()));
            },
        );
    }

    #[test]
    fn effective_languages_expands_zh_cn_utf8() {
        with_env(
            &[
                ("LANGUAGE", "zh_CN"),
                ("LC_ALL", ""),
                ("LC_MESSAGES", ""),
                ("LANG", ""),
            ],
            || {
                let langs = effective_languages();
                assert!(
                    !langs.is_empty(),
                    "expected non-empty language list, got {langs:?}"
                );
                assert_eq!(langs.first(), Some(&"zh_CN".to_string()));
                assert!(langs.contains(&"zh".to_string()));
            },
        );
    }

    #[test]
    fn clear_languages_restores_locale_chain() {
        with_env(
            &[("LANGUAGE", "zh_TW"), ("LC_MESSAGES", ""), ("LANG", "")],
            || {
                let baseline = locale_languages();
                set_languages(&["fr".to_string()]);
                assert_eq!(get_languages(), vec!["fr".to_string()]);
                clear_languages();
                assert_eq!(get_languages(), baseline);
                assert_eq!(effective_languages(), baseline);
            },
        );
    }

    #[test]
    fn is_language_available_exact_locale_only_like_ki18n() {
        let _guard = TEST_LOCK.lock().expect("lock");
        let dir = tempdir().expect("tempdir");
        let mo_path = dir.path().join("locale").join("zh_TW").join("LC_MESSAGES");
        fs::create_dir_all(&mo_path).expect("mkdir");
        let mo_bytes = build_minimal_mo("nplurals=1; plural=0;", &[("x", "x")]);
        let domain = "test-exact-locale-dom";
        fs::write(mo_path.join(format!("{domain}.mo")), &mo_bytes).expect("write mo");

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.clear();
        s.available_languages_cache.clear();
        drop(s);

        add_domain_locale_dir(domain, dir.path());
        assert!(is_language_available(domain, "zh_TW"));
        assert!(!is_language_available(domain, "zh_HK"));

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.remove(domain);
        s.available_languages_cache.clear();
    }

    #[test]
    fn is_language_available_does_not_full_scan() {
        let _guard = TEST_LOCK.lock().expect("lock");
        let dir = tempdir().expect("tempdir");
        let mo_path = dir.path().join("locale").join("fr").join("LC_MESSAGES");
        fs::create_dir_all(&mo_path).expect("mkdir");
        let mo_bytes = build_minimal_mo("nplurals=2; plural=(n != 1);", &[("x", "x")]);
        fs::write(mo_path.join("fast-dom.mo"), &mo_bytes).expect("write mo");

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.clear();
        s.available_languages_cache.clear();
        drop(s);

        add_domain_locale_dir("fast-dom", dir.path());
        assert!(is_language_available("fast-dom", "en_US"));
        assert!(is_language_available("fast-dom", "fr"));
        assert!(!is_language_available("fast-dom", "zz_not_installed"));

        let mut s = GLOBAL_STATE.write();
        s.domain_paths.remove("fast-dom");
        s.available_languages_cache.clear();
    }
}

