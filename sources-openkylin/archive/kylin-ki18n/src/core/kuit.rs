// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::cell::RefCell;
use std::collections::HashMap;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::sync::{Mutex, OnceLock};

use once_cell::sync::Lazy;
use quick_xml::events::Event;
use quick_xml::Reader;
use regex::Regex;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum VisualFormat {
    UndefinedFormat = 0,
    PlainText = 10,
    RichText = 20,
    TermText = 30,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum TagClass {
    PhraseTag = 0,
    StructTag = 1,
}

/// 与上游 KUIT 一致：按属性名排序后生成组合键，如 `[]` 或 `[a href]`。
fn attribute_set_key_from_names(names: &[String]) -> String {
    if names.is_empty() {
        return "[]".to_string();
    }
    let mut v: Vec<&str> = names.iter().map(|s| s.as_str()).collect();
    v.sort();
    format!("[{}]", v.join(" "))
}

fn attribute_set_key_from_attrs(attrs: &HashMap<String, String>) -> String {
    let mut keys: Vec<String> = attrs.keys().cloned().collect();
    keys.sort();
    attribute_set_key_from_names(&keys)
}

/// 上游 `countWrappingNewlines`：首尾连续换行数。
fn count_wrapping_newlines(text: &str) -> (usize, usize) {
    let len = text.chars().count();
    let chars: Vec<char> = text.chars().collect();
    let mut numle = 0usize;
    while numle < len && chars.get(numle) == Some(&'\n') {
        numle += 1;
    }
    let mut numtr = 0usize;
    while numtr < len && len > numtr && chars.get(len - numtr - 1) == Some(&'\n') {
        numtr += 1;
    }
    (numle, numtr)
}

fn apply_leading_newlines(preceding: &str, formatted: &str, required: i32) -> String {
    if required <= 0 {
        return formatted.to_string();
    }
    let (_pnumle, pnumtr) = count_wrapping_newlines(preceding);
    let (fnumle, _fnumtr) = count_wrapping_newlines(formatted);
    let numle = pnumtr + fnumle;
    let req = required as usize;
    let extra = req.saturating_sub(numle);
    let prefix = "\n".repeat(extra);
    format!("{}{}", prefix, formatted)
}

#[derive(Clone, Debug)]
struct PatternEntry {
    /// `%2`、`%3`… 对应的属性名顺序（与上游 `attributeOrders` 一致）
    attrib_order: Vec<String>,
    pattern: Option<String>,
    formatter_handle: u64,
}

#[derive(Clone, Debug, Default)]
struct KuitConfig {
    /// tag -> attrib_key -> VisualFormat -> entry
    tag_patterns: HashMap<String, HashMap<String, HashMap<VisualFormat, PatternEntry>>>,
    marker_formats: HashMap<String, VisualFormat>,
    tag_classes: HashMap<String, TagClass>,
    /// 标签名 -> 用作 tooltip 的属性名（小写）；富文本 `<a>` 上输出为 `title="..."`。
    tooltip_attribute_by_tag: HashMap<String, String>,
    /// 上游每个 `KuitTag` 仅保留一个值，最后一次 `setTagPattern` 覆盖
    tag_leading_newlines: HashMap<String, i32>,
}

/// 与 `Kuit::setupForDomain` 对齐：每个翻译 domain 独立一份 KUIT 配置；空串表示默认/全局。
static KUIT_REGISTRY: OnceLock<Mutex<HashMap<String, KuitConfig>>> = OnceLock::new();

fn normalize_domain_key(domain: &str) -> String {
    domain.trim().to_string()
}

fn kuit_registry() -> &'static Mutex<HashMap<String, KuitConfig>> {
    KUIT_REGISTRY.get_or_init(|| Mutex::new(HashMap::new()))
}

fn get_config_clone(domain: &str) -> KuitConfig {
    let key = normalize_domain_key(domain);
    let map = kuit_registry().lock().unwrap();
    map.get(&key).cloned().unwrap_or_default()
}

fn with_config_mut<R>(domain: &str, f: impl FnOnce(&mut KuitConfig) -> R) -> R {
    let key = normalize_domain_key(domain);
    let mut map = kuit_registry().lock().unwrap();
    let cfg = map.entry(key).or_insert_with(KuitConfig::default);
    f(cfg)
}

#[cfg(test)]
pub fn reset_kuit_domain(domain: &str) {
    let key = normalize_domain_key(domain);
    let mut map = kuit_registry().lock().unwrap();
    map.remove(&key);
}

type KuitFormatterInvoker = unsafe extern "C" fn(
    handle: u64,
    langs: *const *const c_char,
    lang_count: usize,
    tag: *const c_char,
    attr_keys: *const *const c_char,
    attr_vals: *const *const c_char,
    attr_count: usize,
    text: *const c_char,
    tag_path: *const *const c_char,
    tag_path_len: usize,
    visual_format: c_int,
    out: *mut *mut c_char,
) -> c_int;

static KUIT_FORMATTER_INVOKER: OnceLock<Mutex<Option<KuitFormatterInvoker>>> = OnceLock::new();

fn formatter_invoker() -> &'static Mutex<Option<KuitFormatterInvoker>> {
    KUIT_FORMATTER_INVOKER.get_or_init(|| Mutex::new(None))
}

/// C++ 在库加载时注册，用于调用 `Kuit::TagFormatter`。
pub fn register_formatter_invoker(f: Option<KuitFormatterInvoker>) {
    let mut g = formatter_invoker().lock().unwrap();
    *g = f;
}

type KuitWarningHandler = unsafe extern "C" fn(msg: *const c_char);

static KUIT_WARNING_HANDLER: OnceLock<Mutex<Option<KuitWarningHandler>>> = OnceLock::new();

fn kuit_warning_handler() -> &'static Mutex<Option<KuitWarningHandler>> {
    KUIT_WARNING_HANDLER.get_or_init(|| Mutex::new(None))
}

/// C++ 注册 `qWarning` 桥接；`msg` 为 UTF-8、NUL 结尾。
pub fn register_kuit_warning_handler(f: Option<KuitWarningHandler>) {
    let mut g = kuit_warning_handler().lock().unwrap();
    *g = f;
}

fn invoke_kuit_warning(msg: &str) {
    let Ok(cs) = CString::new(msg) else {
        return;
    };
    let g = kuit_warning_handler().lock().unwrap();
    if let Some(f) = g.as_ref() {
        unsafe {
            f(cs.as_ptr());
        }
    }
}

thread_local! {
    static KUIT_WARN_FULL_MESSAGE: RefCell<Option<String>> = const { RefCell::new(None) };
}

/// 与上游 `KuitFormatterPrivate::parseOpenEl` 警告一致：整段待格式化消息（未 `trim`）。
pub fn kuit_set_markup_warning_full_message(msg: Option<&str>) {
    KUIT_WARN_FULL_MESSAGE.with(|c| {
        *c.borrow_mut() = msg.map(|s| s.to_string());
    });
}

fn kuit_markup_warning_full_message() -> Option<String> {
    KUIT_WARN_FULL_MESSAGE.with(|c| c.borrow().clone())
}

fn shorten_for_kuit_warning_message(s: &str) -> String {
    const MAX: usize = 80;
    let count = s.chars().count();
    if count <= MAX {
        s.to_string()
    } else {
        let head: String = s.chars().take(MAX).collect();
        format!("{}...", head)
    }
}

/// 包装解析失败时携带 `quick-xml` 错误与最后遇到的开始标签名（对齐 `lastElementName`）。
#[derive(Debug)]
pub struct KuitWrappedXmlError {
    pub error: quick_xml::Error,
    pub last_open_tag: String,
}

/// 上游 `kuitmarkup.cpp`：`qCWarning` 中 “Markup error in message …” 一行。
pub fn emit_kuit_markup_xml_parse_warning(full_message: &str, err: &KuitWrappedXmlError) {
    let line = format!(
        "Markup error in message {{{}}}: {}. Last tag parsed: {}. Complete message follows:\n{}",
        shorten_for_kuit_warning_message(full_message),
        err.error,
        err.last_open_tag,
        full_message
    );
    invoke_kuit_warning(&line);
}

pub fn set_tag_pattern_full(
    tag: &str,
    attrib_names: &[&str],
    format: VisualFormat,
    pattern: Option<&str>,
    leading_newlines: i32,
    formatter_handle: u64,
) {
    set_tag_pattern_full_for_domain("", tag, attrib_names, format, pattern, leading_newlines, formatter_handle);
}

pub fn set_tag_pattern_full_for_domain(
    domain: &str,
    tag: &str,
    attrib_names: &[&str],
    format: VisualFormat,
    pattern: Option<&str>,
    leading_newlines: i32,
    formatter_handle: u64,
) {
    let tag = tag.to_ascii_lowercase();
    if tag.is_empty() {
        return;
    }
    let names: Vec<String> = attrib_names
        .iter()
        .map(|s| (*s).to_string())
        .filter(|s| !s.is_empty())
        .collect();
    let attrib_key = attribute_set_key_from_names(&names);
    let pattern = pattern.map(|s| s.to_string()).filter(|s| !s.is_empty());

    with_config_mut(domain, |cfg| {
        cfg.tag_leading_newlines.insert(tag.clone(), leading_newlines);
        let by_tag = cfg.tag_patterns.entry(tag).or_default();
        let by_attrib = by_tag.entry(attrib_key).or_default();
        by_attrib.insert(
            format,
            PatternEntry {
                attrib_order: names,
                pattern,
                formatter_handle,
            },
        );
    });
}

/// 测试与旧代码兼容：无属性、可选格式与模式串。
pub fn set_tag_pattern(tag: &str, format: Option<VisualFormat>, replacement: Option<String>) {
    let fmt = format.unwrap_or(VisualFormat::PlainText);
    set_tag_pattern_full(
        tag,
        &[],
        fmt,
        replacement.as_deref(),
        0,
        0,
    );
}

pub fn set_tag_class(tag: &str, class: TagClass) {
    set_tag_class_for_domain("", tag, class);
}

pub fn set_tag_class_for_domain(domain: &str, tag: &str, class: TagClass) {
    let tag = tag.to_ascii_lowercase();
    if tag.is_empty() {
        return;
    }
    with_config_mut(domain, |cfg| {
        cfg.tag_classes.insert(tag, class);
    });
}

pub fn set_marker_format(marker: &str, format: VisualFormat) {
    set_marker_format_for_domain("", marker, format);
}

pub fn set_marker_format_for_domain(domain: &str, marker: &str, format: VisualFormat) {
    let marker = marker.to_ascii_lowercase();
    if marker.is_empty() {
        return;
    }
    with_config_mut(domain, |cfg| {
        cfg.marker_formats.insert(marker, format);
    });
}

/// 与 `ki18n_rs_kuit_set_tooltip_attribute` 一致：为 XML/KUIT 标签指定 tooltip 来源属性（如 `link` + `title`）。
pub fn set_tooltip_attribute(tag: &str, attribute: &str) {
    set_tooltip_attribute_for_domain("", tag, attribute);
}

pub fn set_tooltip_attribute_for_domain(domain: &str, tag: &str, attribute: &str) {
    let tag = tag.to_ascii_lowercase();
    let attribute = attribute.to_ascii_lowercase();
    if tag.is_empty() || attribute.is_empty() {
        return;
    }
    with_config_mut(domain, |cfg| {
        cfg.tooltip_attribute_by_tag.insert(tag, attribute);
    });
}

fn apply_pattern_placeholders(pat: &str, subs: &[String], format: VisualFormat) -> String {
    let mut s = pat.to_string();
    for i in (0..subs.len()).rev() {
        let ph = format!("%{}", i + 1);
        s = s.replace(&ph, &subs[i]);
    }
    if format != VisualFormat::RichText {
        s = s.replace("&lt;", "<").replace("&gt;", ">").replace("&amp;", "&");
    }
    s
}

/// 与 `Kuit::escape` 一致：供 RichText 下 `KLocalizedString::subs` + `ignoreMarkup().toString` 路径等价替换。
fn kuit_escape_pattern_substitution(s: &str) -> String {
    let mut n = String::with_capacity(s.len());
    for c in s.chars() {
        match c {
            '&' => n.push_str("&amp;"),
            '<' => n.push_str("&lt;"),
            '>' => n.push_str("&gt;"),
            '\'' => n.push_str("&apos;"),
            '"' => n.push_str("&quot;"),
            _ => n.push(c),
        }
    }
    n
}

fn invoke_cpp_formatter(
    handle: u64,
    langs: &[String],
    tag: &str,
    attrs: &HashMap<String, String>,
    text: &str,
    tag_path: &[String],
    format: VisualFormat,
) -> Option<String> {
    if handle == 0 {
        return None;
    }
    let inv = *formatter_invoker().lock().ok()?.as_ref()?;

    let mut lang_cstrs: Vec<CString> = Vec::new();
    let mut lang_ptrs: Vec<*const c_char> = Vec::new();
    for l in langs {
        if let Ok(c) = CString::new(l.as_str()) {
            lang_ptrs.push(c.as_ptr());
            lang_cstrs.push(c);
        }
    }

    let mut keys: Vec<String> = attrs.keys().cloned().collect();
    keys.sort();
    let mut key_cstrs: Vec<CString> = Vec::new();
    let mut val_cstrs: Vec<CString> = Vec::new();
    let mut key_ptrs: Vec<*const c_char> = Vec::new();
    let mut val_ptrs: Vec<*const c_char> = Vec::new();
    for k in &keys {
        let v = attrs.get(k).map(|s| s.as_str()).unwrap_or("");
        let ck = CString::new(k.as_str()).ok()?;
        let cv = CString::new(v).ok()?;
        key_ptrs.push(ck.as_ptr());
        val_ptrs.push(cv.as_ptr());
        key_cstrs.push(ck);
        val_cstrs.push(cv);
    }

    let tag_c = CString::new(tag).ok()?;
    let text_c = CString::new(text).ok()?;

    let mut path_cstrs: Vec<CString> = Vec::new();
    let mut path_ptrs: Vec<*const c_char> = Vec::new();
    for p in tag_path {
        let c = CString::new(p.as_str()).ok()?;
        path_ptrs.push(c.as_ptr());
        path_cstrs.push(c);
    }

    let mut out: *mut c_char = std::ptr::null_mut();
    let rc = unsafe {
        inv(
            handle,
            lang_ptrs.as_ptr(),
            lang_ptrs.len(),
            tag_c.as_ptr(),
            key_ptrs.as_ptr(),
            val_ptrs.as_ptr(),
            key_ptrs.len(),
            text_c.as_ptr(),
            path_ptrs.as_ptr(),
            path_ptrs.len(),
            format as c_int,
            &mut out,
        )
    };
    if rc != 0 || out.is_null() {
        if !out.is_null() {
            unsafe {
                let _ = CString::from_raw(out);
            }
        }
        return None;
    }
    let s = unsafe {
        let slice = CStr::from_ptr(out);
        let owned = slice.to_string_lossy().into_owned();
        let _ = CString::from_raw(out);
        owned
    };
    Some(s)
}

fn apply_registered_pattern(
    entry: &PatternEntry,
    content: &str,
    attrs: &HashMap<String, String>,
    langs: &[String],
    tag: &str,
    tag_path: &[String],
    format: VisualFormat,
) -> String {
    let mod_text = invoke_cpp_formatter(
        entry.formatter_handle,
        langs,
        tag,
        attrs,
        content,
        tag_path,
        format,
    )
    .unwrap_or_else(|| content.to_string());

    if let Some(pat) = &entry.pattern {
        let mut subs = vec![mod_text];
        for k in &entry.attrib_order {
            let v = attrs.get(k).cloned().unwrap_or_default();
            subs.push(if format == VisualFormat::RichText {
                kuit_escape_pattern_substitution(&v)
            } else {
                v
            });
        }
        apply_pattern_placeholders(pat, &subs, format)
    } else {
        mod_text
    }
}

fn resolve_builtin(
    tag_lower: &str,
    attrs: &HashMap<String, String>,
    content: &str,
    format: VisualFormat,
    cfg: &KuitConfig,
) -> String {
    match format {
        VisualFormat::PlainText | VisualFormat::UndefinedFormat => {
            resolve_plain(tag_lower, attrs, content)
        }
        VisualFormat::RichText => resolve_rich(tag_lower, attrs, content, cfg),
        VisualFormat::TermText => resolve_term(tag_lower, attrs, content),
    }
}

/// 与 `KuitSetupPrivate::setTagClass` + `setTagPattern` 默认一致：
/// - `setTagPattern` 首次插入的标签在 C++ 中为 `PhraseTag`，由 `tag_classes` 覆盖；
/// - `setDefaultMarkup` 里显式 `StructTag` 的：`title`、`subtitle`、`para`、`list`、`item`。
fn effective_tag_class(tag: &str, cfg: &KuitConfig) -> TagClass {
    let t = tag.trim().to_ascii_lowercase();
    if let Some(c) = cfg.tag_classes.get(&t) {
        return *c;
    }
    match t.as_str() {
        "root" => TagClass::StructTag,
        "title" | "subtitle" | "para" | "list" | "item" => TagClass::StructTag,
        _ => TagClass::PhraseTag,
    }
}

/// `parseOpenEl`：Struct 不能嵌在 Phrase 下 → `Dropout`，等价于仅拼接子树格式化结果（无外层标签模式）。
fn phrase_struct_dropout(parent_tag: &str, child_tag: &str, cfg: &KuitConfig) -> bool {
    if parent_tag.eq_ignore_ascii_case("root") {
        return false;
    }
    let p = parent_tag.to_ascii_lowercase();
    let c = child_tag.to_ascii_lowercase();
    effective_tag_class(&c, cfg) == TagClass::StructTag
        && effective_tag_class(&p, cfg) == TagClass::PhraseTag
}

/// 与 `setup.d->knownTags` / `parseOpenEl` 一致：不在集合内的标签走 `OpenEl::Ignored`，`formatSubText` 原样输出标签。
fn is_kuit_known_tag(tag_lc: &str, cfg: &KuitConfig) -> bool {
    if cfg.tag_patterns.contains_key(tag_lc)
        || cfg.tag_classes.contains_key(tag_lc)
        || cfg.tooltip_attribute_by_tag.contains_key(tag_lc)
    {
        return true;
    }
    matches!(
        tag_lc,
        "root"
            | "title"
            | "subtitle"
            | "para"
            | "list"
            | "item"
            | "note"
            | "warning"
            | "link"
            | "filename"
            | "application"
            | "command"
            | "resource"
            | "icode"
            | "bcode"
            | "shortcut"
            | "interface"
            | "emphasis"
            | "placeholder"
            | "email"
            | "envar"
            | "message"
            | "nl"
            | "b"
            | "strong"
            | "em"
            | "i"
            | "u"
            | "code"
            | "tt"
            | "p"
            | "li"
            | "ul"
            | "ol"
            | "h1"
            | "h2"
            | "h3"
            | "h4"
            | "h5"
            | "h6"
            | "a"
            | "br"
            | "img"
            | "label"
            | "button"
            | "action"
            | "error"
            | "info"
            | "table"
            | "tr"
            | "td"
            | "th"
            | "dl"
            | "dt"
            | "dd"
            | "hr"
            | "span"
            | "div"
            | "pre"
    )
}

/// `oel.tagPath`：自内向外的 proper 祖先链（当前元素的直接 proper 父为 path[0]）。
fn tag_path_for_resolution(proper_parent: &str, proper_stack: &[String]) -> Vec<String> {
    if proper_parent.eq_ignore_ascii_case("root") {
        proper_stack.to_vec()
    } else {
        let mut p = vec![proper_parent.to_string()];
        p.extend_from_slice(proper_stack);
        p
    }
}

/// `formatSubText` 中 `OpenEl::Ignored` 分支：`<name attrs>inner</name>`。
fn format_ignored_xml_tag(name_lc: &str, attrs: &HashMap<String, String>, inner: &str) -> String {
    let mut keys: Vec<&String> = attrs.keys().collect();
    keys.sort();
    let mut out = String::new();
    out.push('<');
    out.push_str(name_lc);
    for k in keys {
        let v = &attrs[k];
        let quote = if v.contains('\'') { '"' } else { '\'' };
        out.push(' ');
        out.push_str(&k.to_ascii_lowercase());
        out.push('=');
        out.push(quote);
        out.push_str(v);
        out.push(quote);
    }
    out.push('>');
    out.push_str(inner);
    out.push_str("</");
    out.push_str(name_lc);
    out.push('>');
    out
}

fn resolve_tag_extended(
    tag: &str,
    attrs: &HashMap<String, String>,
    content: &str,
    format: VisualFormat,
    preceding_text: &str,
    tag_path: &[String],
    langs: &[String],
    cfg: &KuitConfig,
) -> String {
    let tag_lower = tag.to_ascii_lowercase();

    let marker_fmt = cfg.marker_formats.get(&tag_lower).copied();
    let registered = cfg
        .tag_patterns
        .get(&tag_lower)
        .and_then(|m| {
            let ak = attribute_set_key_from_attrs(attrs);
            m.get(&ak).and_then(|fm| fm.get(&format).cloned())
        });
    let leading_nl = *cfg.tag_leading_newlines.get(&tag_lower).unwrap_or(&0);

    if let Some(fmt) = marker_fmt {
        if fmt != format {
            return resolve_tag_extended(
                &tag_lower,
                attrs,
                content,
                fmt,
                preceding_text,
                tag_path,
                langs,
                cfg,
            );
        }
    }

    let mut out = if let Some(entry) = registered {
        apply_registered_pattern(&entry, content, attrs, langs, &tag_lower, tag_path, format)
    } else {
        resolve_builtin(&tag_lower, attrs, content, format, cfg)
    };

    out = apply_leading_newlines(preceding_text, &out, leading_nl);

    if effective_tag_class(&tag_lower, cfg) == TagClass::StructTag {
        if !out.is_empty() {
            out = format!("\n{}\n", out.trim_end_matches('\n'));
        }
    }

    out
}

#[derive(Debug, Clone)]
pub enum KuitNode {
    Text(String),
    Tag {
        name: String,
        attrs: HashMap<String, String>,
        children: Vec<KuitNode>,
    },
}

/// KUIT 解析器，负责将带有 @tag 的字符串解析为语法树。
pub struct KuitParser;

impl KuitParser {
    pub fn new() -> Self {
        KuitParser
    }

    pub fn parse(&self, text: &str) -> KuitNode {
        parse_internal(text)
    }
}

pub fn parse(text: &str) -> KuitNode {
    let trimmed = text.trim();

    // 顶层 UI marker：@role:content/format
    if let Some(node) = parse_ui_marker_root(trimmed) {
        return node;
    }

    // 其次尝试 XML（与上游一致：整段包在内部 `<root>` 下解析）
    if trimmed.starts_with('<') {
        if let Ok(node) = parse_wrapped_xml_markup(trimmed) {
            return node;
        }
    }

    // 其它情况使用 KUIT 顺序解析器（支持多段 @tag{...}）
    parse_kuit_sequence(text)
}

/// 与上游一致：按子节点顺序拼接，以便 `leadingNewlines` 能观察前文。
/// `proper_parent` / `proper_stack` 对齐 `toVisualText` 里为 `parseOpenEl` 选取的 **首个 `Proper` 祖先**
///（中间的 Dropout / Ignored 不进入 containment 与 `tagPath`）。
fn format_children_sequence(
    children: &[KuitNode],
    proper_parent: &str,
    proper_stack: &[String],
    langs: &[String],
    format: VisualFormat,
    cfg: &KuitConfig,
) -> String {
    let mut acc = String::new();
    for child in children {
        match child {
            KuitNode::Text(t) => {
                // 与 `QXmlStreamReader` 行为类似：合成 `<root>` 下仅空白文本不输出（避免多余换行）
                if proper_parent.eq_ignore_ascii_case("root") && t.trim().is_empty() {
                    continue;
                }
                // RichText 格式下需要对文本内容进行 HTML 转义
                // PlainText 格式下直接输出解码后的内容
                if format == VisualFormat::RichText {
                    acc.push_str(&html_escape_attr(t));
                } else {
                    acc.push_str(t);
                }
            }
            KuitNode::Tag {
                name,
                attrs,
                children: sub,
            } => {
                let name_lc = name.to_ascii_lowercase();
                let known = is_kuit_known_tag(&name_lc, cfg);
                let ignored = !known;
                let dropout = known && phrase_struct_dropout(proper_parent, &name_lc, cfg);
                if dropout {
                    if let Some(src) = kuit_markup_warning_full_message() {
                        let short = shorten_for_kuit_warning_message(&src);
                        let phrase = proper_parent.to_ascii_lowercase();
                        invoke_kuit_warning(&format!(
                            "Structuring tag ('{}') cannot be subtag of phrase tag ('{}') in message {{{}}}.",
                            name_lc, phrase, short
                        ));
                    }
                }

                let next_stack: Vec<String> = if ignored || dropout {
                    proper_stack.to_vec()
                } else if proper_parent.eq_ignore_ascii_case("root") {
                    vec![]
                } else {
                    let mut s = vec![proper_parent.to_string()];
                    s.extend_from_slice(proper_stack);
                    s
                };

                let next_proper = if ignored || dropout {
                    proper_parent
                } else {
                    name_lc.as_str()
                };

                let inner =
                    format_children_sequence(sub, next_proper, &next_stack, langs, format, cfg);

                let tag_path = tag_path_for_resolution(proper_parent, proper_stack);
                let resolved = if ignored {
                    format_ignored_xml_tag(&name_lc, attrs, &inner)
                } else if dropout {
                    inner
                } else {
                    resolve_tag_extended(
                        &name_lc,
                        attrs,
                        &inner,
                        format,
                        &acc,
                        &tag_path,
                        langs,
                        cfg,
                    )
                };
                acc.push_str(&resolved);
            }
        }
    }
    acc
}

/// 与上游 `KuitFormatterPrivate::formatFromUiMarker` + `setDefaultFormats` 对齐。
pub fn resolve_visual_format_from_context(
    context: Option<&str>,
    explicit: VisualFormat,
    domain: &str,
) -> VisualFormat {
    if explicit != VisualFormat::UndefinedFormat {
        return explicit;
    }
    let Some(ctx) = context.map(|s| s.trim()).filter(|s| !s.is_empty()) else {
        return VisualFormat::PlainText;
    };
    let cfg = get_config_clone(domain);
    if !ctx.starts_with('@') {
        return VisualFormat::PlainText;
    }
    let after_at = &ctx[1..];
    let end = after_at
        .find(char::is_whitespace)
        .unwrap_or(after_at.len());
    let seg = after_at[..end].trim();
    if seg.is_empty() {
        return VisualFormat::PlainText;
    }
    let marker_lookup = format!("@{}", seg.to_ascii_lowercase());
    if let Some(&f) = cfg.marker_formats.get(&marker_lookup) {
        return f;
    }
    let (main_part, slash_fmt) = seg
        .split_once('/')
        .map(|(a, b)| (a, Some(b.trim())))
        .unwrap_or((seg, None));
    if let Some(fname) = slash_fmt {
        match fname {
            "plain" => return VisualFormat::PlainText,
            "rich" => return VisualFormat::RichText,
            "term" => return VisualFormat::TermText,
            _ => {}
        }
    }
    let (role, cue) = main_part
        .split_once(':')
        .map(|(a, b)| (a.trim(), b.trim()))
        .unwrap_or((main_part, ""));
    match role {
        "info" => match cue {
            "status" | "progress" | "credit" => VisualFormat::PlainText,
            "shell" => VisualFormat::TermText,
            _ => VisualFormat::RichText,
        },
        "action" | "title" | "label" | "option" | "item" => VisualFormat::PlainText,
        _ => VisualFormat::PlainText,
    }
}

pub fn to_format_with_langs(node: &KuitNode, format: VisualFormat, langs: &[String]) -> String {
    to_format_with_langs_for_domain(node, format, langs, "")
}

/// `domain` 与 `Kuit::setupForDomain` 一致；用于在格式化时选用对应 KUIT 配置。
/// 与上游 `finalizeVisualText` 中 RichText 的 `<html>` 包裹一致。
pub fn finalize_visual_for_format(out: &str, format: VisualFormat) -> String {
    if format == VisualFormat::RichText {
        let trim = out.trim_start();
        let lower = trim.to_ascii_lowercase();
        if lower.starts_with("<html>") {
            out.to_string()
        } else {
            format!("<html>{out}</html>")
        }
    } else {
        out.to_string()
    }
}

pub fn to_format_with_langs_for_domain(
    node: &KuitNode,
    format: VisualFormat,
    langs: &[String],
    domain: &str,
) -> String {
    let cfg = get_config_clone(domain);
    let raw = match node {
        KuitNode::Text(t) => t.clone(),
        KuitNode::Tag { name, attrs, children } => {
            if name.eq_ignore_ascii_case("root") {
                format_children_sequence(children, "root", &[], langs, format, &cfg)
            } else {
                let name_lc = name.to_ascii_lowercase();
                let inner = format_children_sequence(children, &name_lc, &[], langs, format, &cfg);
                resolve_tag_extended(&name_lc, attrs, &inner, format, "", &[], langs, &cfg)
            }
        }
    };
    let out = normalize_ellipsis(&raw);
    finalize_visual_for_format(&out, format)
}

pub fn to_format(node: &KuitNode, format: VisualFormat) -> String {
    to_format_with_langs(node, format, &[])
}

/// 无上下文解析（无 leadingNewlines / formatter 语言列表）。
pub fn resolve_tag(
    tag: &str,
    attrs: &HashMap<String, String>,
    content: &str,
    format: VisualFormat,
) -> String {
    let cfg = get_config_clone("");
    resolve_tag_extended(tag, attrs, content, format, "", &[], &[], &cfg)
}

fn resolve_plain(tag: &str, attrs: &HashMap<String, String>, content: &str) -> String {
    match tag {
        "application" => content.to_string(),
        "b" | "strong" => format!("*{}*", content),
        // `@em` / `<em>` / `@emphasis` / `<emphasis>` 用星号；`@i` / `<i>` 用斜杠
        "em" | "emphasis" => format!("*{}*", content),
        "i" => format!("/{}/", content),
        "u" => format!("_{}_", content),
        "code" | "tt" => format!("`{}`", content),
        "br" => "\n".to_string(),
        "p" => format!("\n{}\n", content),
        "li" => format!("- {}", content),
        "ul" | "ol" => format!("\n{}\n", content),
        "h1" | "h2" | "h3" | "h4" | "h5" | "h6" => format!("{}\n{}", content, "-".repeat(content.len())),
        "a" => content.to_string(),
        "link" => content.to_string(),
        "email" => {
            if let Some(addr) = attr_value_ci(attrs, "address") {
                format!("{} <{}>", content, addr)
            } else {
                format!("<{}>", content)
            }
        }
        "img" => attrs_alt_or(content),
        // 表格与定义列表等可选标签
        "table" => format!("\n[Table]\n{}\n[/Table]\n", content),
        "tr" => format!("| {}\n", content.replace('\n', "\n| ")),
        "td" => content.to_string(),
        "th" => format!("**{}**", content),
        "dl" => format!("\n{}\n", content),
        "dt" => format!("{}: ", content),
        "dd" => format!("{}\n", content),
        "hr" => "\n---\n".to_string(),
        "span" => content.to_string(),
        "div" => format!("\n{}\n", content),
        "pre" => format!("\n```\n{}\n```\n", content),
        "bcode" => content.to_string(),
        "title" | "subtitle" | "label" | "button" | "action" | "warning" | "error" | "info" => {
            format!("{}: {}", tag.to_ascii_uppercase(), content)
        }
        _ => content.to_string(),
    }
}

fn resolve_rich(
    tag: &str,
    attrs: &HashMap<String, String>,
    content: &str,
    cfg: &KuitConfig,
) -> String {
    match tag {
        "application" => content.to_string(),
        "b" | "strong" => format!("<b>{}</b>", content),
        "i" | "em" | "emphasis" => format!("<i>{}</i>", content),
        "u" => format!("<u>{}</u>", content),
        "code" | "tt" => format!("<code>{}</code>", content),
        "p" => format!("<p>{}</p>", content),
        "li" => format!("<li>{}</li>", content),
        "ul" => format!("<ul>{}</ul>", content),
        "ol" => format!("<ol>{}</ol>", content),
        "h1" => format!("<h1>{}</h1>", content),
        "h2" => format!("<h2>{}</h2>", content),
        "h3" => format!("<h3>{}</h3>", content),
        "h4" => format!("<h4>{}</h4>", content),
        "h5" => format!("<h5>{}</h5>", content),
        "h6" => format!("<h6>{}</h6>", content),
        "a" => {
            if let Some(href) = attr_value_ci(attrs, "href") {
                rich_anchor_href("a", href, content, attrs, cfg)
            } else {
                // 对于来自 KUIT 的 @a{...}，至少生成 <a> 包裹，方便富文本渲染
                format!("<a>{}</a>", content)
            }
        }
        // KUIT XML `<link url="…">…</link>`（与上游标签名一致）
        "link" => {
            let href = attr_value_ci(attrs, "url")
                .or_else(|| attr_value_ci(attrs, "href"))
                .unwrap_or("");
            if href.is_empty() {
                format!("<a>{}</a>", content)
            } else {
                rich_anchor_href("link", href, content, attrs, cfg)
            }
        }
        "email" => {
            if let Some(addr) = attr_value_ci(attrs, "address") {
                let e1 = html_escape_attr(content);
                let e2 = html_escape_attr(addr);
                format!(r#"<a href="mailto:{e2}">{e1}</a>"#)
            } else {
                let e = html_escape_attr(content);
                let inner = format!(r#"<a href="mailto:{e}">{e}</a>"#);
                format!("<{inner}>")
            }
        }
        "br" => "<br/>".to_string(),
        "pre" => format!("<pre>{}</pre>", content),
        "bcode" => format!("<pre>{}</pre>", content),
        "img" => {
            let src = attrs.get("src").map(|s| s.as_str()).unwrap_or("");
            let alt = attrs.get("alt").cloned().unwrap_or_else(|| content.to_string());
            format!(r#"<img src="{}" alt="{}"/>"#, src, alt)
        }
        "title" => format!("<h1>{}</h1>", content),
        "subtitle" => format!("<h2>{}</h2>", content),
        "label" => format!(r#"<span class="label">{}</span>"#, content),
        "button" => {
            let mut classes = vec!["button".to_string()];
            if let Some(cue) = attrs.get("cue") {
                classes.push(cue.to_string());
            }
            format!(r#"<button class="{}">{}</button>"#, classes.join(" "), content)
        }
        "action" => {
            let mut classes = vec!["action".to_string()];
            if let Some(cue) = attrs.get("cue") {
                classes.push(cue.to_string());
            }
            format!(r#"<span class="{}">{}</span>"#, classes.join(" "), content)
        }
        "warning" => format!(r#"<span class="warning">{}</span>"#, content),
        "error" => format!(r#"<span class="error">{}</span>"#, content),
        "info" => format!(r#"<span class="info">{}</span>"#, content),
        // 可选 struct/phrase 标签
        "table" => format!("<table>{}</table>", content),
        "tr" => format!("<tr>{}</tr>", content),
        "td" => format!("<td>{}</td>", content),
        "th" => format!("<th>{}</th>", content),
        "dl" => format!("<dl>{}</dl>", content),
        "dt" => format!("<dt>{}</dt>", content),
        "dd" => format!("<dd>{}</dd>", content),
        "hr" => "<hr/>".to_string(),
        "span" => {
            if let Some(class) = attrs.get("class") {
                format!(r#"<span class="{}">{}</span>"#, class, content)
            } else {
                format!("<span>{}</span>", content)
            }
        }
        "div" => {
            if let Some(class) = attrs.get("class") {
                format!(r#"<div class="{}">{}</div>"#, class, content)
            } else {
                format!("<div>{}</div>", content)
            }
        }
        _ => content.to_string(),
    }
}

fn resolve_term(tag: &str, attrs: &HashMap<String, String>, content: &str) -> String {
    // ANSI 转义序列，尽量与 C++ 终端输出风格接近
    match tag {
        "b" | "strong" => format!("\x1b[1m{}\x1b[0m", content),
        "i" | "em" | "emphasis" => format!("\x1b[3m{}\x1b[0m", content),
        "u" => format!("\x1b[4m{}\x1b[0m", content),
        "code" | "tt" => format!("\x1b[2m{}\x1b[0m", content),
        "br" => "\n".to_string(),
        "warning" => format!("\x1b[33m{}\x1b[0m", content),
        "error" => format!("\x1b[31m{}\x1b[0m", content),
        "info" => format!("\x1b[36m{}\x1b[0m", content),
        "title" | "subtitle" => format!("\x1b[1m{}\x1b[0m", content),
        "label" => format!("\x1b[1m{}\x1b[0m", content),
        "button" | "action" => {
            let cue = attrs.get("cue").map(|s| s.as_str()).unwrap_or("");
            match cue {
                "inmenubar" | "inmenu" => format!("\x1b[1m[{}]\x1b[0m", content),
                "intoolbar" => format!("\x1b[1m{{{}}}\x1b[0m", content),
                _ => format!("\x1b[1m{}\x1b[0m", content),
            }
        }
        "table" => format!("\n┌────────┐\n{}\n└────────┘\n", content),
        "tr" => format!("| {}\n", content),
        "td" | "th" => content.to_string(),
        "hr" => "\n─────────────────────\n".to_string(),
        "span" | "div" => content.to_string(),
        "pre" => format!("\n```\n{}\n```\n", content),
        "email" => {
            if let Some(addr) = attr_value_ci(attrs, "address") {
                format!("{} <{}>", content, addr)
            } else {
                format!("<{}>", content)
            }
        }
        _ => content.to_string(),
    }
}

fn attrs_alt_or(content: &str) -> String {
    if content.is_empty() {
        "[image]".to_string()
    } else {
        content.to_string()
    }
}

fn attr_value_ci<'a>(attrs: &'a HashMap<String, String>, key_lc: &str) -> Option<&'a str> {
    attrs
        .iter()
        .find(|(k, _)| k.eq_ignore_ascii_case(key_lc))
        .map(|(_, v)| v.as_str())
}

fn html_escape_attr(s: &str) -> String {
    let mut o = String::with_capacity(s.len());
    for ch in s.chars() {
        match ch {
            '&' => o.push_str("&amp;"),
            '"' => o.push_str("&quot;"),
            '<' => o.push_str("&lt;"),
            '>' => o.push_str("&gt;"),
            _ => o.push(ch),
        }
    }
    o
}

fn tooltip_title_for_tag(
    tag: &str,
    attrs: &HashMap<String, String>,
    cfg: &KuitConfig,
) -> Option<String> {
    let attr_name = cfg.tooltip_attribute_by_tag.get(tag)?.clone();
    let raw = attr_value_ci(attrs, &attr_name)?;
    if raw.is_empty() {
        return None;
    }
    Some(html_escape_attr(raw))
}

fn rich_anchor_href(
    tag_for_tooltip: &str,
    href: &str,
    content: &str,
    attrs: &HashMap<String, String>,
    cfg: &KuitConfig,
) -> String {
    let esc_href = html_escape_attr(href);
    if let Some(title) = tooltip_title_for_tag(tag_for_tooltip, attrs, cfg) {
        format!(r#"<a href="{}" title="{}">{}</a>"#, esc_href, title, content)
    } else {
        format!(r#"<a href="{}">{}</a>"#, esc_href, content)
    }
}

fn parse_internal(text: &str) -> KuitNode {
    let mut chars = text.chars().peekable();
    let mut children = Vec::new();
    let mut buf = String::new();

    while let Some(&ch) = chars.peek() {
        if ch == '@' {
            // 先提交缓冲区中的文本
            if !buf.is_empty() {
                children.push(KuitNode::Text(decode_entities(&buf)));
                buf.clear();
            }
            // 解析标签
            chars.next(); // consume '@'
            let tag_name: String = chars
                .by_ref()
                .take_while(|c| c.is_alphanumeric())
                .collect();

            // UI marker 形式 @title: ...
            if let Some(':') = chars.peek().copied() {
                // UI marker: @role:cue/format 或 @role:content/format
                chars.next();
                let rest: String = chars.by_ref().collect();
                let (content, ui_attrs) = parse_ui_marker(rest.trim(), &tag_name);
                children.push(KuitNode::Tag {
                    name: tag_name,
                    attrs: ui_attrs,
                    children: vec![KuitNode::Text(content)],
                });
                break;
            } else if let Some('{') = chars.peek().copied() {
                // 结构化标签：@b{...}
                chars.next(); // consume '{'
                let inner = parse_braced(&mut chars);
                let attrs = HashMap::new();
                children.push(KuitNode::Tag {
                    name: tag_name,
                    attrs,
                    children: vec![inner],
                });
            } else {
                // 非法 / 不支持的形式，当作普通文本处理
                buf.push('@');
                buf.push_str(&tag_name);
            }
        } else if ch == '<' {
            // XML 块由 quick-xml 顶层解析处理；此处按普通文本收集
            buf.push(ch);
            chars.next();
        } else {
            buf.push(ch);
            chars.next();
        }
    }

    if !buf.is_empty() {
        children.push(KuitNode::Text(decode_entities(&buf)));
    }

    KuitNode::Tag {
        name: "root".to_string(),
        attrs: HashMap::new(),
        children,
    }
}

fn parse_braced<I>(chars: &mut I) -> KuitNode
where
    I: Iterator<Item = char>,
{
    let mut buf = String::new();
    let mut depth = 1;

    while let Some(ch) = chars.next() {
        match ch {
            '{' => {
                depth += 1;
                buf.push(ch);
            }
            '}' => {
                depth -= 1;
                if depth == 0 {
                    break;
                } else {
                    buf.push(ch);
                }
            }
            _ => buf.push(ch),
        }
    }

    // 递归解析内部的 KUIT 标签
    parse_internal(&buf)
}

fn decode_entities(input: &str) -> String {
    let mut result = String::new();
    let mut chars = input.chars().peekable();

    while let Some(ch) = chars.next() {
        if ch == '&' {
            let mut entity = String::new();
            while let Some(&c) = chars.peek() {
                chars.next();
                if c == ';' {
                    break;
                }
                entity.push(c);
            }
            let decoded = match entity.as_str() {
                "amp" => Some('&'),
                "lt" => Some('<'),
                "gt" => Some('>'),
                "quot" => Some('"'),
                "apos" => Some('\''),
                _ if entity.starts_with("#x") || entity.starts_with("#X") => {
                    i32::from_str_radix(&entity[2..], 16)
                        .ok()
                        .and_then(|v| char::from_u32(v as u32))
                }
                _ if entity.starts_with('#') => entity[1..]
                    .parse::<u32>()
                    .ok()
                    .and_then(char::from_u32),
                _ => None,
            };
            if let Some(c) = decoded {
                result.push(c);
            } else {
                result.push('&');
                result.push_str(&entity);
                result.push(';');
            }
        } else {
            result.push(ch);
        }
    }

    result
}

fn normalize_ellipsis(input: &str) -> String {
    input.to_string()
}

// 与 `KuitFormatterPrivate::salvageMarkup` 同结构的简化实现（best-effort）。
// 注意：`regex` crate 不支持反向引用，故拆成「开标签 + 按名查找闭标签」。
static SALVAGE_OPEN: Lazy<Regex> =
    Lazy::new(|| Regex::new(r#"(?si)<\s*(\w+)\b[^>]*>"#).expect("salvage open"));
static SALVAGE_VOID: Lazy<Regex> =
    Lazy::new(|| Regex::new(r#"(?si)<\s*(\w+)\b[^>]*/\s*>"#).expect("salvage void"));

fn salvage_markup_inner(
    text: &str,
    format: VisualFormat,
    langs: &[String],
    domain: &str,
    cfg: &KuitConfig,
) -> String {
    let text = salvage_wrapped_pass_inner(text, format, langs, domain, cfg);
    salvage_void_pass_inner(&text, format, langs, domain, cfg)
}

fn salvage_wrapped_pass_inner(
    text: &str,
    format: VisualFormat,
    langs: &[String],
    domain: &str,
    cfg: &KuitConfig,
) -> String {
    let mut result = String::new();
    let mut last = 0usize;
    let mut pos = 0usize;
    while pos < text.len() {
        let slice = &text[pos..];
        let Some(cap) = SALVAGE_OPEN.captures(slice) else {
            break;
        };
        let m = cap.get(0).unwrap();
        let abs_start = pos + m.start();
        let abs_open_end = pos + m.end();
        let open_s = m.as_str();
        if open_s.trim_end().ends_with("/>") {
            pos = abs_open_end;
            continue;
        }
        let tagname = cap.get(1).unwrap().as_str();
        let close_pat = format!(r"(?si)</\s*{}\s*>", regex::escape(tagname));
        let Ok(close_re) = Regex::new(&close_pat) else {
            pos = abs_start + 1;
            continue;
        };
        let rest = &text[abs_open_end..];
        if let Some(cm) = close_re.find(rest) {
            let inner_raw = &rest[..cm.start()];
            let full_end = abs_open_end + cm.end();
            result.push_str(&text[last..abs_start]);
            let inner_done = salvage_markup_inner(inner_raw, format, langs, domain, cfg);
            let formatted = resolve_tag_extended(
                &tagname.to_ascii_lowercase(),
                &HashMap::new(),
                &inner_done,
                format,
                "",
                &[],
                langs,
                cfg,
            );
            result.push_str(&formatted);
            last = full_end;
            pos = full_end;
        } else {
            pos = abs_start + 1;
        }
    }
    result.push_str(&text[last..]);
    result
}

fn salvage_void_pass_inner(
    text: &str,
    format: VisualFormat,
    langs: &[String],
    _domain: &str,
    cfg: &KuitConfig,
) -> String {
    let mut result = String::new();
    let mut last = 0usize;
    for cap in SALVAGE_VOID.captures_iter(text) {
        let full = cap.get(0).unwrap();
        result.push_str(&text[last..full.start()]);
        let tagname = cap.get(1).unwrap().as_str().to_ascii_lowercase();
        let formatted =
            resolve_tag_extended(&tagname, &HashMap::new(), "", format, "", &[], langs, cfg);
        if formatted.is_empty() {
            result.push_str(full.as_str());
        } else {
            result.push_str(&formatted);
        }
        last = full.end();
    }
    result.push_str(&text[last..]);
    result
}

/// 畸形 XML 时的回退格式化，与上游 `salvageMarkup` 类似；输出已含 `finalizeVisualText` 的 RichText 包裹。
pub fn salvage_markup(
    text: &str,
    format: VisualFormat,
    langs: &[String],
    domain: &str,
) -> String {
    let cfg = get_config_clone(domain);
    let raw = salvage_markup_inner(text, format, langs, domain, &cfg);
    finalize_visual_for_format(&normalize_ellipsis(&raw), format)
}

/// 若能用 `quick-xml` 完整解析为闭合树则返回根节点，否则 `None`（用于触发 salvage）。
pub fn try_parse_xml_markup(text: &str) -> Option<KuitNode> {
    let t = text.trim_start();
    if !t.contains('<') {
        return None;
    }
    parse_wrapped_xml_markup(t).ok()
}

/// 顺序解析一串包含 0 个或多个 `@tag{...}` 片段的文本。
fn parse_kuit_sequence(text: &str) -> KuitNode {
    let mut chars = text.chars().peekable();
    let mut children = Vec::new();
    let mut buf = String::new();

    while let Some(&ch) = chars.peek() {
        if ch == '@' {
            // 先提交缓冲区中的纯文本
            if !buf.is_empty() {
                children.push(KuitNode::Text(decode_entities(&buf)));
                buf.clear();
            }

            // 读取标签名
            chars.next(); // consume '@'
            let mut tag_name = String::new();
            while let Some(&c) = chars.peek() {
                if c.is_alphanumeric() {
                    tag_name.push(c);
                    chars.next();
                } else {
                    break;
                }
            }

            if tag_name.is_empty() {
                buf.push('@');
                continue;
            }

            // 结构化标签：@tag{...}
            if let Some('{') = chars.peek().copied() {
                chars.next(); // consume '{'
                let inner = parse_braced(&mut chars);
                let attrs = HashMap::new();
                children.push(KuitNode::Tag {
                    name: tag_name,
                    attrs,
                    children: vec![inner],
                });
            } else {
                // 非法 / 不支持的形式，当作普通文本处理
                buf.push('@');
                buf.push_str(&tag_name);
            }
        } else {
            buf.push(ch);
            chars.next();
        }
    }

    if !buf.is_empty() {
        children.push(KuitNode::Text(decode_entities(&buf)));
    }

    KuitNode::Tag {
        name: "root".to_string(),
        attrs: HashMap::new(),
        children,
    }
}

/// 顶层 UI marker 解析：`@title:Main Window/rich` 等。
fn parse_ui_marker_root(text: &str) -> Option<KuitNode> {
    let chars: Vec<char> = text.chars().collect();
    if chars.first().copied()? != '@' {
        return None;
    }
    let body: String = chars[1..].iter().collect();
    let (role, rest) = body.split_once(':')?;
    if role.is_empty() {
        return None;
    }
    let (content, attrs) = parse_ui_marker(rest.trim(), role);
    Some(KuitNode::Tag {
        name: role.to_string(),
        attrs,
        children: vec![KuitNode::Text(content)],
    })
}

fn parse_xml_document(text: &str) -> Result<KuitNode, KuitWrappedXmlError> {
    let mut reader = Reader::from_str(text);
    let mut buf = Vec::new();
    let mut last_open_tag = String::new();

    let mut stack: Vec<KuitNode> = Vec::new();
    stack.push(KuitNode::Tag {
        name: "root".to_string(),
        attrs: HashMap::new(),
        children: Vec::new(),
    });

    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(e)) => {
                last_open_tag = String::from_utf8_lossy(e.name().as_ref()).to_string();
                let name = last_open_tag.clone();
                let mut attrs = HashMap::new();
                for a in e.attributes().flatten() {
                    let key = String::from_utf8_lossy(a.key.as_ref()).to_string();
                    let value = a
                        .unescape_value()
                        .map(|v| v.to_string())
                        .unwrap_or_default();
                    attrs.insert(key, decode_entities(&value));
                }
                stack.push(KuitNode::Tag {
                    name,
                    attrs,
                    children: Vec::new(),
                });
            }
            Ok(Event::Empty(e)) => {
                let name = String::from_utf8_lossy(e.name().as_ref()).to_string();
                last_open_tag.clone_from(&name);
                let mut attrs = HashMap::new();
                for a in e.attributes().flatten() {
                    let key = String::from_utf8_lossy(a.key.as_ref()).to_string();
                    let value = a
                        .unescape_value()
                        .map(|v| v.to_string())
                        .unwrap_or_default();
                    attrs.insert(key, decode_entities(&value));
                }
                if let Some(KuitNode::Tag { children, .. }) = stack.last_mut() {
                    children.push(KuitNode::Tag {
                        name,
                        attrs,
                        children: Vec::new(),
                    });
                }
            }
            Ok(Event::Text(e)) => {
                if let Some(KuitNode::Tag { children, .. }) = stack.last_mut() {
                    let txt = e.unescape().map(|v| v.to_string()).unwrap_or_default();
                    let decoded = decode_entities(&txt);
                    if !decoded.is_empty() {
                        // 文本节点中可能再次包含 KUIT 标记（例如 @b{...}）
                        if decoded.contains('@') {
                            let parsed = parse_kuit_sequence(&decoded);
                            if let KuitNode::Tag { children: inner, .. } = parsed {
                                children.extend(inner);
                            } else {
                                children.push(parsed);
                            }
                        } else {
                            children.push(KuitNode::Text(decoded));
                        }
                    }
                }
            }
            Ok(Event::End(_)) => {
                if stack.len() > 1 {
                    let node = stack.pop().unwrap();
                    if let Some(KuitNode::Tag { children, .. }) = stack.last_mut() {
                        children.push(node);
                    }
                }
            }
            Ok(Event::Eof) => break,
            Err(e) => {
                return Err(KuitWrappedXmlError {
                    error: e,
                    last_open_tag,
                });
            }
            _ => {}
        }
        buf.clear();
    }

    if stack.len() != 1 {
        return Err(KuitWrappedXmlError {
            error: quick_xml::Error::UnexpectedToken("unclosed element".into()),
            last_open_tag,
        });
    }
    stack.pop().ok_or_else(|| KuitWrappedXmlError {
        error: quick_xml::Error::UnexpectedToken("empty stack".into()),
        last_open_tag,
    })
}

/// 去掉「栈底虚拟 root + 包装用 XML `<root>`」叠成的双 root，只保留一层逻辑根。
fn flatten_kuit_xml_wrapper(node: KuitNode) -> KuitNode {
    let KuitNode::Tag { name, attrs, mut children } = node else {
        return node;
    };
    if !name.eq_ignore_ascii_case("root") || !attrs.is_empty() || children.len() != 1 {
        return KuitNode::Tag { name, attrs, children };
    }
    let inner = children.remove(0);
    let KuitNode::Tag {
        name: n2,
        attrs: a2,
        children: c2,
    } = inner
    else {
        return KuitNode::Tag {
            name,
            attrs,
            children: vec![inner],
        };
    };
    if n2.eq_ignore_ascii_case("root") && a2.is_empty() {
        KuitNode::Tag {
            name: "root".to_string(),
            attrs: HashMap::new(),
            children: c2,
        }
    } else {
        KuitNode::Tag {
            name: "root".to_string(),
            attrs: HashMap::new(),
            children: vec![KuitNode::Tag {
                name: n2,
                attrs: a2,
                children: c2,
            }],
        }
    }
}

/// 与上游 `toVisualText` 一致：将用户片段包在 `<root>…</root>` 内再解析。
pub fn parse_wrapped_xml_markup(user_fragment: &str) -> Result<KuitNode, KuitWrappedXmlError> {
    let wrapped = format!("<root>{}</root>", user_fragment);
    let node = parse_xml_document(&wrapped)?;
    Ok(flatten_kuit_xml_wrapper(node))
}

fn parse_ui_marker(rest: &str, role: &str) -> (String, HashMap<String, String>) {
    let mut attrs = HashMap::new();
    attrs.insert("role".to_string(), role.to_string());

    let mut format: Option<String> = None;
    let content_str = if let Some((left, right)) = rest.split_once('/') {
        let content = left.trim();
        let fmt = right.trim();
        if matches!(fmt, "plain" | "rich" | "term") {
            format = Some(fmt.to_string());
        } else if !fmt.is_empty() {
            attrs.insert("cue".to_string(), fmt.to_string());
        }
        content
    } else {
        rest.trim()
    };

    if let Some(f) = format {
        attrs.insert("format".to_string(), f);
    }

    (content_str.to_string(), attrs)
}

#[cfg(test)]
mod tooltip_attribute_tests {
    use super::*;

    #[test]
    fn xml_link_rich_matches_anchor() {
        let node = parse(r#"<link url="http://example.com/p">Go</link>"#);
        let out = to_format(&node, VisualFormat::RichText);
        assert_eq!(
            out,
            r#"<html><a href="http://example.com/p">Go</a></html>"#
        );
    }

    #[test]
    fn tooltip_title_on_link_after_set_tooltip_attribute() {
        let dom = "kuit_ut_tooltip_link";
        set_tooltip_attribute_for_domain(dom, "link", "tip");
        let node = parse(r#"<link url="http://z" tip="Hi &quot;there&quot;">Txt</link>"#);
        let out = to_format_with_langs_for_domain(&node, VisualFormat::RichText, &[], dom);
        assert!(out.contains("title="), "got {out:?}");
        assert!(out.contains("Hi"), "got {out:?}");
        assert!(out.contains("http://z"), "got {out:?}");
        super::reset_kuit_domain(dom);
    }

    #[test]
    fn tooltip_title_on_anchor_after_set_tooltip_attribute() {
        let dom = "kuit_ut_tooltip_a";
        set_tooltip_attribute_for_domain(dom, "a", "title");
        let node = parse(r#"<a href="u" title="Tool">x</a>"#);
        let out = to_format_with_langs_for_domain(&node, VisualFormat::RichText, &[], dom);
        assert!(out.contains("title="), "got {out:?}");
        assert!(out.contains("Tool"), "got {out:?}");
        super::reset_kuit_domain(dom);
    }
}
