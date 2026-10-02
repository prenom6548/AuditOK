// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashSet;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use crate::capi::Ki18nRsError;
use crate::core::global as core_global;
use crate::core::message::Message;

#[repr(C)]
pub struct TranslatorHandle {
    domain: Option<String>,
    contexts: HashSet<String>,
}

fn c_str_opt(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() {
        return None;
    }
    unsafe { CStr::from_ptr(ptr).to_str().ok().map(|s| s.to_string()) }
}

fn c_str_required(ptr: *const c_char) -> Result<String, Ki18nRsError> {
    if ptr.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    unsafe {
        CStr::from_ptr(ptr)
            .to_str()
            .map(|s| s.to_string())
            .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_translator_create() -> *mut TranslatorHandle {
    Box::into_raw(Box::new(TranslatorHandle {
        domain: None,
        contexts: HashSet::new(),
    }))
}

#[no_mangle]
pub extern "C" fn ki18n_rs_translator_destroy(handle: *mut TranslatorHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_translator_set_domain(
    handle: *mut TranslatorHandle,
    domain: *const c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let domain_s = match c_str_required(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        (*handle).domain = Some(domain_s);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_translator_add_context(
    handle: *mut TranslatorHandle,
    context: *const c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ctx = match c_str_required(context) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        (*handle).contexts.insert(ctx);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_translator_remove_context(
    handle: *mut TranslatorHandle,
    context: *const c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ctx = match c_str_required(context) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        (*handle).contexts.remove(&ctx);
    }
    Ki18nRsError::OK.into()
}

/// 对齐 `KLocalizedTranslator::translate`：在已监控 context 且已设置 domain 时，按语言回退链加载 catalog、
/// 用 `Message::finalize` 完成翻译与 transcript；未监控或缺 domain 时返回 NULL。
#[no_mangle]
pub extern "C" fn ki18n_rs_translator_translate(
    handle: *mut TranslatorHandle,
    context: *const c_char,
    msgid: *const c_char,
    _msgid_plural: *const c_char,
    _n: i64,
) -> *mut c_char {
    if handle.is_null() {
        return std::ptr::null_mut();
    }
    let ctx = match c_str_opt(context) {
        Some(s) => s,
        None => return std::ptr::null_mut(),
    };
    let msgid_s = match c_str_required(msgid) {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };
    unsafe {
        let h = &*handle;
        if h.domain.is_none() || !h.contexts.contains(&ctx) {
            return std::ptr::null_mut();
        }
    }

    // In upstream KLocalizedTranslator:
    // - msgid_plural is used as disambiguation/context (msgctxt) and translation depends on monitored "context".
    // - n is ignored by KLocalizedTranslator's translate() implementation.
    let disambiguation = c_str_opt(_msgid_plural).map(|s| s);
    let msgctxt_opt = disambiguation.as_ref().filter(|s| !s.is_empty()).cloned();

    let h = unsafe { &*handle };
    let domain = h.domain.as_ref().unwrap();

    // languages: highest-to-lowest priority. If unset, behave reasonably.
    let mut languages = core_global::effective_languages();
    if languages.is_empty() {
        languages.push("en".to_string());
    }

    let mut first_catalog: Option<(String, crate::core::catalog::Catalog)> = None;
    let mut selected_catalog: Option<(String, crate::core::catalog::Catalog)> = None;

    for lang in &languages {
        if let Some(cat) = core_global::get_or_load_catalog(domain, lang) {
            if first_catalog.is_none() {
                first_catalog = Some((lang.clone(), cat.clone()));
            }

            let lookup_key = match msgctxt_opt.as_deref() {
                Some(c) if !c.is_empty() => format!("{}\u{4}{}", c, msgid_s),
                _ => msgid_s.clone(),
            };

            // Only consider it "found" if the catalog has a non-empty msgstr.
            let found = cat
                .get_translated_message(&lookup_key, 0)
                .is_some_and(|s| !s.is_empty());

            if found {
                selected_catalog = Some((lang.clone(), cat));
                break;
            }
        }
    }

    let (selected_lang, selected_cat) = match selected_catalog.or(first_catalog) {
        Some(v) => v,
        None => {
            // No catalog available; fall back to original text.
            return CString::new(msgid_s)
                .ok()
                .map_or(std::ptr::null_mut(), |s| s.into_raw());
        }
    };

    let msg = Message::new(
        &msgid_s,
        None, // KLocalizedTranslator translate() doesn't do plural selection.
        msgctxt_opt.as_deref(),
        false,
    );

    let out = msg
        .finalize(&selected_cat, &[selected_lang.clone()])
        .unwrap_or_else(|_| msgid_s.clone());

    CString::new(out).ok().map_or(std::ptr::null_mut(), |s| s.into_raw())
}

