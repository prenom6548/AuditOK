// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use crate::capi::message::translate_with_domain_and_langs;
use crate::capi::Ki18nRsError;
use crate::core::message::Message;

/// Opaque handle for a translation context (domain + optional language list).
#[repr(C)]
pub struct ContextHandle {
    domain: Option<String>,
}

fn c_str_opt(ptr: *const c_char) -> Result<Option<String>, Ki18nRsError> {
    if ptr.is_null() {
        return Ok(None);
    }
    unsafe {
        CStr::from_ptr(ptr)
            .to_str()
            .map(|s| Some(s.to_string()))
            .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)
    }
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

unsafe fn alloc_c_string(s: &str, out: *mut *mut c_char) -> Result<(), Ki18nRsError> {
    if out.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let cstr = CString::new(s).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
    *out = cstr.into_raw();
    Ok(())
}

#[no_mangle]
pub extern "C" fn ki18n_rs_context_create() -> *mut ContextHandle {
    Box::into_raw(Box::new(ContextHandle { domain: None }))
}

#[no_mangle]
pub extern "C" fn ki18n_rs_context_destroy(handle: *mut ContextHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_context_set_domain(
    handle: *mut ContextHandle,
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

/// 对齐 `KLocalizedContext::i18n` / `i18nc` / `i18np`：`ctx` 为 msgctxt；若 `plural` 非空则按 `n` 选复数形式。
#[no_mangle]
pub extern "C" fn ki18n_rs_context_translate(
    handle: *mut ContextHandle,
    msg: *const c_char,
    ctx: *const c_char,
    plural: *const c_char,
    n: i64,
    langs: *const *const c_char,
    langs_count: usize,
    out_utf8: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = std::ptr::null_mut();
    }

    let msg_s = match c_str_required(msg) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let ctx_s = match c_str_opt(ctx) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let plural_s = match c_str_opt(plural) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };

    let mut message = if let Some(p) = plural_s.filter(|s| !s.is_empty()) {
        let mut m = Message::new(&msg_s, Some(&p), ctx_s.as_deref(), false);
        m.subs_i64(n, 0, 10, ' ');
        m
    } else {
        Message::new(&msg_s, None, ctx_s.as_deref(), false)
    };

    let domain = unsafe { (*handle).domain.clone() };
    message.domain = domain;

    let lang_override: Option<Vec<String>> = if langs_count == 0 {
        None
    } else if langs.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    } else {
        let mut v = Vec::with_capacity(langs_count);
        for i in 0..langs_count {
            let p = unsafe { *langs.add(i) };
            match c_str_required(p) {
                Ok(s) => v.push(s),
                Err(e) => return e.into(),
            }
        }
        Some(v)
    };

    let out = match translate_with_domain_and_langs(
        &message,
        None,
        lang_override.as_deref(),
    ) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };

    unsafe {
        match alloc_c_string(&out, out_utf8) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

