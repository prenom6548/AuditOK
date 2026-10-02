// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::ffi::{CStr, CString};
use std::fs;
use std::os::raw::c_char;
use std::ptr;
use std::time::{SystemTime, UNIX_EPOCH};

use ki18n_rs::capi;
use ki18n_rs::Ki18nRsError;

fn setup_context_catalog(domain: &str) -> tempfile::TempDir {
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=2; plural=(n != 1);";
    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let ctx_key = format!("Ctx\u{4}item");
    let src = common::write_test_mo(
        &format!("ctx_ffi_{nanos}.mo"),
        header,
        &[
            ("hello", "Hello"),
            (ctx_key.as_str(), "Hello Ctx"),
            ("files", "1 file\0%1 files"),
        ],
    );
    fs::copy(&src, dir.path().join("en.mo")).expect("copy mo");

    let dom = CString::new(domain).unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();
    let _ = capi::global::ki18n_rs_clear_languages();
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());
    dir
}

#[test]
fn test_ffi_context_translate_singular() {
    let domain = "ffi-ctx-singular";
    let _dir = setup_context_catalog(domain);

    let handle = capi::context::ki18n_rs_context_create();
    assert!(!handle.is_null());

    let dom = CString::new(domain).unwrap();
    assert_eq!(
        capi::context::ki18n_rs_context_set_domain(handle, dom.as_ptr()),
        Ki18nRsError::OK as i32
    );

    let msg = CString::new("hello").unwrap();
    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::context::ki18n_rs_context_translate(
        handle,
        msg.as_ptr(),
        ptr::null(),
        ptr::null(),
        0,
        langs.as_ptr(),
        langs.len(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!out.is_null());
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert_eq!(s, "Hello");
    capi::global::ki18n_rs_string_free(out);
    capi::context::ki18n_rs_context_destroy(handle);
}

#[test]
fn test_ffi_context_translate_with_msgctxt() {
    let domain = "ffi-ctx-disamb";
    let _dir = setup_context_catalog(domain);

    let handle = capi::context::ki18n_rs_context_create();
    let dom = CString::new(domain).unwrap();
    capi::context::ki18n_rs_context_set_domain(handle, dom.as_ptr());

    let msg = CString::new("item").unwrap();
    let ctx = CString::new("Ctx").unwrap();
    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::context::ki18n_rs_context_translate(
        handle,
        msg.as_ptr(),
        ctx.as_ptr(),
        ptr::null(),
        0,
        langs.as_ptr(),
        langs.len(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert_eq!(s, "Hello Ctx");
    capi::global::ki18n_rs_string_free(out);
    capi::context::ki18n_rs_context_destroy(handle);
}

#[test]
fn test_ffi_context_translate_plural() {
    let domain = "ffi-ctx-plural";
    let _dir = setup_context_catalog(domain);

    let handle = capi::context::ki18n_rs_context_create();
    let dom = CString::new(domain).unwrap();
    capi::context::ki18n_rs_context_set_domain(handle, dom.as_ptr());

    let msg = CString::new("files").unwrap();
    let plural = CString::new("%1 files").unwrap();
    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::context::ki18n_rs_context_translate(
        handle,
        msg.as_ptr(),
        ptr::null(),
        plural.as_ptr(),
        2,
        langs.as_ptr(),
        langs.len(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert!(s.contains("files"));
    capi::global::ki18n_rs_string_free(out);
    capi::context::ki18n_rs_context_destroy(handle);
}

#[test]
fn test_ffi_context_invalid_handle_and_null_out() {
    let msg = CString::new("x").unwrap();
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::context::ki18n_rs_context_translate(
        ptr::null_mut(),
        msg.as_ptr(),
        ptr::null(),
        ptr::null(),
        0,
        ptr::null(),
        0,
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::INVALID_ARGUMENT as i32);

    let handle = capi::context::ki18n_rs_context_create();
    let res2 = capi::context::ki18n_rs_context_translate(
        handle,
        msg.as_ptr(),
        ptr::null(),
        ptr::null(),
        0,
        ptr::null(),
        0,
        ptr::null_mut(),
    );
    assert_eq!(res2, Ki18nRsError::INVALID_ARGUMENT as i32);
    capi::context::ki18n_rs_context_destroy(handle);
}

#[test]
fn test_ffi_context_destroy_null_safe() {
    capi::context::ki18n_rs_context_destroy(ptr::null_mut());
}
