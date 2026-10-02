// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use crate::capi::Ki18nRsError;
use crate::core::global as core_global;

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

fn c_str_opt(ptr: *const c_char) -> Result<Option<String>, Ki18nRsError> {
    if ptr.is_null() {
        return Ok(None);
    }
    unsafe {
        let s = CStr::from_ptr(ptr)
            .to_str()
            .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
        Ok(Some(s.to_string()))
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

/// Opaque handle for a catalog instance (domain + language).
#[repr(C)]
pub struct KCatalogHandle {
    domain: String,
    language: String,
}

/// Translate a raw message from a catalog, without placeholder substitution.
///
/// - `domain`: translation domain
/// - `msgid`: singular message id
/// - `msgid_plural`: optional plural message id (NULL if none)
/// - `n`: number for plural selection (ignored if `msgid_plural` is NULL)
/// - `context`: optional context (NULL if none)
///
/// Languages are taken from the current global language list.
///
/// On success, writes a newly allocated UTF‑8 string to `*out_utf8`
/// (caller must free with `ki18n_rs_string_free`) and returns `KI18N_RS_OK`.
/// On failure, `*out_utf8` is set to NULL and a non‑zero Ki18nRsError code is returned.
#[no_mangle]
pub extern "C" fn ki18n_rs_catalog_translate(
    domain: *const c_char,
    msgid: *const c_char,
    msgid_plural: *const c_char,
    n: i64,
    context: *const c_char,
    out_utf8: *mut *mut c_char,
) -> c_int {
    // Prepare output pointer early
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = std::ptr::null_mut();
    }

    let domain_s = match c_str_required(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let msgid_s = match c_str_required(msgid) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let plural_s = match c_str_opt(msgid_plural) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let context_s = match c_str_opt(context) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };

    // Build composite key when context is present: "ctx\0msgid"
    let key = if let Some(ctx) = context_s.as_ref() {
        let mut k = String::with_capacity(ctx.len() + 1 + msgid_s.len());
        k.push_str(ctx);
        k.push('\u{0}');
        k.push_str(&msgid_s);
        k
    } else {
        msgid_s.clone()
    };

    // Determine plural index if plural form is requested
    let mut want_plural = false;
    let mut plural_index: usize = 0;
    if plural_s.is_some() && n >= 0 {
        want_plural = true;
    }

    // Try languages in order of global preference
    let mut langs = core_global::get_languages();
    if langs.is_empty() {
        // Fall back to source language if nothing is configured
        langs.push("en_US".to_string());
    }

    for lang in &langs {
        if let Some(catalog) = core_global::get_or_load_catalog(&domain_s, lang) {
            if want_plural {
                plural_index = catalog.get_plural_form(n as u64);
            }
            if let Some(translated) = catalog.get_translated_message(&key, plural_index) {
                unsafe {
                    match alloc_c_string(translated, out_utf8) {
                        Ok(()) => return Ki18nRsError::OK.into(),
                        Err(e) => return e.into(),
                    }
                }
            }
        }
    }

    // Not found in any catalog
    Ki18nRsError::NOT_FOUND.into()
}

// -------------------------------------------------------------------------
// Instance-based catalog API (for ABI completeness; thin wrapper around the
// global catalog cache).
// -------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_catalog_create(
    domain: *const c_char,
    language: *const c_char,
) -> *mut KCatalogHandle {
    let domain_s = match c_str_required(domain) {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };
    let lang_s = match c_str_required(language) {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };
    Box::into_raw(Box::new(KCatalogHandle {
        domain: domain_s,
        language: lang_s,
    }))
}

#[no_mangle]
pub extern "C" fn ki18n_rs_catalog_destroy(handle: *mut KCatalogHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_catalog_translate_instance(
    handle: *mut KCatalogHandle,
    msgid: *const c_char,
    msgid_plural: *const c_char,
    n: i64,
    context: *const c_char,
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

    let msgid_s = match c_str_required(msgid) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let plural_s = match c_str_opt(msgid_plural) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let context_s = match c_str_opt(context) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };

    let catalog = unsafe {
        let h = &*handle;
        match core_global::get_or_load_catalog(&h.domain, &h.language) {
            Some(c) => c,
            None => return Ki18nRsError::NOT_FOUND.into(),
        }
    };

    // Build composite key if context is present
    let key = if let Some(ctx) = context_s.as_ref() {
        let mut k = String::with_capacity(ctx.len() + 1 + msgid_s.len());
        k.push_str(ctx);
        k.push('\u{0}');
        k.push_str(&msgid_s);
        k
    } else {
        msgid_s.clone()
    };

    let mut plural_index: usize = 0;
    if plural_s.is_some() && n >= 0 {
        plural_index = catalog.get_plural_form(n as u64);
    }

    if let Some(translated) = catalog.get_translated_message(&key, plural_index) {
        unsafe {
            match alloc_c_string(translated, out_utf8) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            }
        }
    } else {
        Ki18nRsError::NOT_FOUND.into()
    }
}


