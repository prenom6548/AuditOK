// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

use crate::core::kuit::{self, TagClass, VisualFormat};

fn c_str_opt(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() {
        return None;
    }
    unsafe {
        CStr::from_ptr(ptr)
            .to_str()
            .ok()
            .map(|s| s.to_string())
    }
}

fn domain_from_ptr(domain: *const c_char) -> String {
    c_str_opt(domain).unwrap_or_default()
}

/// 与 C++ `KuitSetup::setTagClass` / `setFormatForMarker` 及 `ki18n_rs_kuit_set_format` 一致；供 `kuitsetup` 桥接复用。
pub(crate) fn apply_kuit_set_format(domain: &str, tag_class: c_int, tag: &str, format: c_int) {
    if tag.is_empty() {
        return;
    }

    if let Some(fmt) = visual_format_from_i32(format) {
        if fmt != VisualFormat::UndefinedFormat {
            kuit::set_marker_format_for_domain(domain, tag, fmt);
            return;
        }
    }

    if let Some(class) = tag_class_from_i32(tag_class) {
        kuit::set_tag_class_for_domain(domain, tag, class);
    }
}

/// C++ `KuitSetup::setTagClass` / `setFormatForMarker` 使用；`replacement` 参数保留以兼容旧声明，已忽略。
/// `domain` 可为 NULL 或空串，表示默认 domain（与 `Kuit::setupForDomain("")` 一致）。
#[no_mangle]
pub extern "C" fn ki18n_rs_kuit_set_format(
    domain: *const c_char,
    tag_class: c_int,
    tag: *const c_char,
    format: c_int,
    _replacement: *const c_char,
) {
    let dom = domain_from_ptr(domain);
    let tag = match c_str_opt(tag) {
        Some(v) if !v.is_empty() => v,
        _ => return,
    };
    apply_kuit_set_format(&dom, tag_class, &tag, format);
}

/// 完整 `KuitSetup::setTagPattern`：`attrib_names` 为 UTF-8 字符串指针数组（可空当 `attrib_count==0`）。
/// `pattern` 可为空指针表示仅 formatter；`formatter_handle` 由 C++ 注册表生成，0 表示无回调。
#[no_mangle]
pub unsafe extern "C" fn ki18n_rs_kuit_set_tag_pattern(
    domain: *const c_char,
    tag: *const c_char,
    attrib_names: *const *const c_char,
    attrib_count: usize,
    format: c_int,
    pattern: *const c_char,
    leading_newlines: c_int,
    formatter_handle: u64,
) {
    let dom = domain_from_ptr(domain);
    let tag_s = match c_str_opt(tag) {
        Some(v) if !v.is_empty() => v,
        _ => return,
    };
    let Some(fmt) = visual_format_from_i32(format) else {
        return;
    };

    let mut owned_names: Vec<String> = Vec::new();
    if !attrib_names.is_null() && attrib_count > 0 {
        let slice = std::slice::from_raw_parts(attrib_names, attrib_count);
        for &p in slice {
            if p.is_null() {
                continue;
            }
            if let Ok(cs) = CStr::from_ptr(p).to_str() {
                if !cs.is_empty() {
                    owned_names.push(cs.to_string());
                }
            }
        }
    }
    let refs: Vec<&str> = owned_names.iter().map(|s| s.as_str()).collect();

    let pattern_opt = c_str_opt(pattern).filter(|s| !s.is_empty());

    kuit::set_tag_pattern_full_for_domain(
        &dom,
        &tag_s,
        &refs,
        fmt,
        pattern_opt.as_deref(),
        leading_newlines,
        formatter_handle,
    );
}

pub type Ki18nKuitFormatterInvoker = unsafe extern "C" fn(
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

#[no_mangle]
pub extern "C" fn ki18n_rs_kuit_register_formatter_invoker(f: Option<Ki18nKuitFormatterInvoker>) {
    kuit::register_formatter_invoker(f);
}

pub type Ki18nKuitWarningHandler = unsafe extern "C" fn(msg: *const c_char);

#[no_mangle]
pub extern "C" fn ki18n_rs_kuit_register_warning_handler(f: Option<Ki18nKuitWarningHandler>) {
    kuit::register_kuit_warning_handler(f);
}

#[no_mangle]
pub extern "C" fn ki18n_rs_kuit_set_tooltip_attribute(
    domain: *const c_char,
    tag: *const c_char,
    attribute: *const c_char,
) {
    let dom = domain_from_ptr(domain);
    let tag = match c_str_opt(tag) {
        Some(v) if !v.is_empty() => v,
        _ => return,
    };
    let attr = match c_str_opt(attribute) {
        Some(v) if !v.is_empty() => v,
        _ => return,
    };
    kuit::set_tooltip_attribute_for_domain(&dom, &tag, &attr);
}

fn visual_format_from_i32(v: c_int) -> Option<VisualFormat> {
    match v {
        0 => Some(VisualFormat::UndefinedFormat),
        10 => Some(VisualFormat::PlainText),
        20 => Some(VisualFormat::RichText),
        30 => Some(VisualFormat::TermText),
        _ => None,
    }
}

fn tag_class_from_i32(v: c_int) -> Option<TagClass> {
    match v {
        0 => Some(TagClass::PhraseTag),
        1 => Some(TagClass::StructTag),
        _ => None,
    }
}
