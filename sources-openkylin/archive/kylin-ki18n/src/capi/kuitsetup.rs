// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

use crate::capi::kuit::apply_kuit_set_format;
use crate::core::kuit::VisualFormat;

/// Opaque handle for KuitSetup configuration（备用 FFI；主路径为 C++ `kuitmarkup.cpp` 直接调 `ki18n_rs_kuit_*`）。
/// 实际指向堆上的 `String`（domain）；请勿在 C 侧解引用。
pub enum KuitSetupHandle {}

fn c_str_opt(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() {
        return None;
    }
    unsafe { CStr::from_ptr(ptr).to_str().ok().map(|s| s.to_string()) }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_kuitsetup_create(domain: *const c_char) -> *mut KuitSetupHandle {
    let domain = c_str_opt(domain).unwrap_or_default();
    Box::into_raw(Box::new(domain)) as *mut KuitSetupHandle
}

#[no_mangle]
pub extern "C" fn ki18n_rs_kuitsetup_destroy(handle: *mut KuitSetupHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle as *mut String);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_kuitsetup_set_tag_class(
    handle: *mut KuitSetupHandle,
    tag_name: *const c_char,
    class_id: c_int,
) {
    if handle.is_null() {
        return;
    }
    let Some(tag) = c_str_opt(tag_name).filter(|s| !s.is_empty()) else {
        return;
    };
    let domain = unsafe { &*(handle as *const String) };
    // 对齐 C++：`setTagClass` → `ki18n_rs_kuit_set_format(class, tagName, UndefinedFormat, …)`
    apply_kuit_set_format(domain, class_id, &tag, VisualFormat::UndefinedFormat as c_int);
}

#[no_mangle]
pub extern "C" fn ki18n_rs_kuitsetup_set_format(
    handle: *mut KuitSetupHandle,
    marker: *const c_char,
    format: c_int,
) {
    if handle.is_null() {
        return;
    }
    let Some(m) = c_str_opt(marker).filter(|s| !s.is_empty()) else {
        return;
    };
    let domain = unsafe { &*(handle as *const String) };
    // 对齐 C++：`setFormatForMarker` 传入 `Kuit::PhraseTag` 作为首参
    apply_kuit_set_format(
        domain,
        crate::core::kuit::TagClass::PhraseTag as c_int,
        &m,
        format,
    );
}
