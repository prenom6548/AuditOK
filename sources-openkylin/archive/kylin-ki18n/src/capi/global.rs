// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};

use crate::capi::Ki18nRsError;
use crate::core::global as core_global;

fn c_str_to_string(ptr: *const c_char) -> Result<String, Ki18nRsError> {
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

/// `NULL` → `None`；非 `NULL` 但非法 UTF‑8 → `Err`。
fn c_str_optional(ptr: *const c_char) -> Result<Option<String>, Ki18nRsError> {
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

unsafe fn alloc_c_string(s: &str, out: *mut *mut c_char) -> Result<(), Ki18nRsError> {
    if out.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let cstr = CString::new(s).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
    let ptr = cstr.into_raw();
    *out = ptr;
    Ok(())
}

/// 将 UTF-8 字节复制为 Rust 分配的 C 字符串（与 `ki18n_rs_string_free` 配对释放）。
/// 供 C++ 侧（如 KUIT `TagFormatter` 回调）写回格式化结果。
#[no_mangle]
pub unsafe extern "C" fn ki18n_rs_dup_utf8(
    ptr: *const u8,
    len: usize,
    out: *mut *mut c_char,
) -> c_int {
    if out.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    if len > 0 && ptr.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let slice = std::slice::from_raw_parts(ptr, len);
    let cstr = match CString::new(slice.to_vec()) {
        Ok(c) => c,
        Err(_) => return Ki18nRsError::INVALID_ARGUMENT.into(),
    };
    *out = cstr.into_raw();
    Ki18nRsError::OK.into()
}

unsafe fn alloc_c_string_array(
    list: &[String],
    out_arr: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> Result<(), Ki18nRsError> {
    if out_arr.is_null() || out_count.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let count = list.len();
    let mut vec_ptrs: Vec<*mut c_char> = Vec::with_capacity(count);
    for s in list {
        let cstr = CString::new(s.as_str()).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
        vec_ptrs.push(cstr.into_raw());
    }
    let arr = libc::calloc(count, std::mem::size_of::<*mut c_char>()) as *mut *mut c_char;
    if arr.is_null() {
        // 回收已经分配的字符串
        for p in vec_ptrs {
            if !p.is_null() {
                let _ = CString::from_raw(p);
            }
        }
        return Err(Ki18nRsError::OUT_OF_MEMORY);
    }
    for (i, p) in vec_ptrs.into_iter().enumerate() {
        *arr.add(i) = p;
    }
    *out_arr = arr;
    *out_count = count;
    Ok(())
}

#[no_mangle]
pub extern "C" fn ki18n_rs_set_application_domain(domain: *const c_char) -> c_int {
    match c_str_to_string(domain) {
        Ok(s) => {
            core_global::set_application_domain(&s);
            Ki18nRsError::OK.into()
        }
        Err(e) => e.into(),
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_get_application_domain(out_domain: *mut *mut c_char) -> c_int {
    unsafe {
        match core_global::get_application_domain() {
            Some(domain) => match alloc_c_string(&domain, out_domain) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            None => {
                if !out_domain.is_null() {
                    *out_domain = std::ptr::null_mut();
                }
                Ki18nRsError::NOT_FOUND.into()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_set_languages(
    langs: *const *const c_char,
    count: usize,
) -> c_int {
    if langs.is_null() && count > 0 {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let mut list = Vec::with_capacity(count);
    for i in 0..count {
        let ptr = unsafe { *langs.add(i) };
        match c_str_to_string(ptr) {
            Ok(s) => list.push(s),
            Err(e) => return e.into(),
        }
    }
    core_global::set_languages(&list);
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_get_languages(
    out_langs: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> c_int {
    unsafe {
        let langs = core_global::get_languages();
        match alloc_c_string_array(&langs, out_langs, out_count) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_clear_languages() -> c_int {
    core_global::clear_languages();
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_add_domain_locale_dir(
    domain: *const c_char,
    path: *const c_char,
) -> c_int {
    let domain_s = match c_str_to_string(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let path_s = match c_str_to_string(path) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    core_global::add_domain_locale_dir(&domain_s, std::path::Path::new(&path_s));
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_available_languages(
    domain: *const c_char,
    out_langs: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> c_int {
    unsafe {
        let domain_s = match c_str_to_string(domain) {
            Ok(s) => s,
            Err(e) => return e.into(),
        };
        let langs = core_global::available_languages(&domain_s);
        match alloc_c_string_array(&langs, out_langs, out_count) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

/// 返回 1 表示该 domain 存在 `language` 的 `.mo`（或 `language` 为源码语言）；0 表示否。
#[no_mangle]
pub extern "C" fn ki18n_rs_is_language_available(
    domain: *const c_char,
    language: *const c_char,
) -> c_int {
    let domain_s = match c_str_to_string(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let lang_s = match c_str_to_string(language) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    if core_global::is_language_available(&domain_s, &lang_s) {
        1
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_qt_domain_insert(domain: *const c_char) -> c_int {
    match c_str_to_string(domain) {
        Ok(s) => {
            core_global::insert_qt_domain(&s);
            Ki18nRsError::OK.into()
        }
        Err(e) => e.into(),
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_qt_domain_remove(domain: *const c_char) -> c_int {
    match c_str_to_string(domain) {
        Ok(s) => {
            core_global::remove_qt_domain(&s);
            Ki18nRsError::OK.into()
        }
        Err(e) => e.into(),
    }
}

/// 与 `KLocalizedString::translateQt` 一致；`n` 为保留参数（与上游相同，未使用）。
#[no_mangle]
pub extern "C" fn ki18n_rs_translate_qt(
    context: *const c_char,
    source: *const c_char,
    comment: *const c_char,
    _n: c_int,
    out_utf8: *mut *mut c_char,
) -> c_int {
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = std::ptr::null_mut();
    }
    let source_s = match c_str_to_string(source) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    if source_s.is_empty() {
        unsafe {
            return match alloc_c_string("", out_utf8) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            };
        }
    }
    let ctx = match c_str_optional(context) {
        Ok(o) => o,
        Err(e) => return e.into(),
    };
    let com = match c_str_optional(comment) {
        Ok(o) => o,
        Err(e) => return e.into(),
    };
    let out = core_global::translate_qt(ctx.as_deref(), &source_s, com.as_deref());
    unsafe {
        match alloc_c_string(&out, out_utf8) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_catalog_locale_dir(
    domain: *const c_char,
    lang: *const c_char,
    out_path: *mut *mut c_char,
) -> c_int {
    unsafe {
        let domain_s = match c_str_to_string(domain) {
            Ok(s) => s,
            Err(e) => return e.into(),
        };
        let lang_s = match c_str_to_string(lang) {
            Ok(s) => s,
            Err(e) => return e.into(),
        };
        match core_global::catalog_locale_dir(&domain_s, &lang_s) {
            Some(path) => {
                let s = path.to_string_lossy().to_string();
                match alloc_c_string(&s, out_path) {
                    Ok(()) => Ki18nRsError::OK.into(),
                    Err(e) => e.into(),
                }
            }
            None => {
                if !out_path.is_null() {
                    *out_path = std::ptr::null_mut();
                }
                Ki18nRsError::NOT_FOUND.into()
            }
        }
    }
}

/// 释放由 Rust 分配的字符串
#[no_mangle]
pub extern "C" fn ki18n_rs_string_free(ptr: *mut c_char) {
    if ptr.is_null() {
        return;
    }
    unsafe {
        let _ = CString::from_raw(ptr);
    }
}

/// 释放由 Rust 分配的字符串数组
#[no_mangle]
pub extern "C" fn ki18n_rs_string_array_free(arr: *mut *mut c_char, count: usize) {
    if arr.is_null() {
        return;
    }
    unsafe {
        for i in 0..count {
            let elem = *arr.add(i);
            if !elem.is_null() {
                let _ = CString::from_raw(elem);
            }
        }
        libc::free(arr as *mut libc::c_void);
    }
}

