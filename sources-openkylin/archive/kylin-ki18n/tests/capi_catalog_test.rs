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

fn setup_domain_mo(domain: &str, lang: &str, entries: &[(&str, &str)]) -> tempfile::TempDir {
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=2; plural=(n != 1);";
    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let src = common::write_test_mo(
        &format!("catalog_ffi_{nanos}.mo"),
        header,
        entries,
    );
    fs::copy(&src, dir.path().join(format!("{lang}.mo"))).expect("copy mo");

    let dom = CString::new(domain).unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();
    let _ = capi::global::ki18n_rs_clear_languages();
    let lang_c = CString::new(lang).unwrap();
    let langs = [lang_c.as_ptr()];
    assert_eq!(
        capi::global::ki18n_rs_set_languages(langs.as_ptr(), langs.len()),
        Ki18nRsError::OK as i32
    );
    assert_eq!(
        capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr()),
        Ki18nRsError::OK as i32
    );
    dir
}

#[test]
fn test_ffi_catalog_translate_singular_and_context() {
    let _state_guard = common::global_state_test_lock();
    let domain = "ffi-catalog-dom";
    let ctx_key = format!("Menu\u{0}Open");
    let _dir = setup_domain_mo(domain, "en", &[(ctx_key.as_str(), "Open Menu")]);

    let dom = CString::new(domain).unwrap();
    let msgid = CString::new("Open").unwrap();
    let ctx = CString::new("Menu").unwrap();
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::catalog::ki18n_rs_catalog_translate(
        dom.as_ptr(),
        msgid.as_ptr(),
        ptr::null(),
        0,
        ctx.as_ptr(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!out.is_null());
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert_eq!(s, "Open Menu");
    capi::global::ki18n_rs_string_free(out);
}

#[test]
fn test_ffi_catalog_translate_plural() {
    let _state_guard = common::global_state_test_lock();
    let domain = "ffi-catalog-plural";
    let _dir = setup_domain_mo(domain, "en", &[("files", "1 file\0%1 files")]);

    let dom = CString::new(domain).unwrap();
    let msgid = CString::new("files").unwrap();
    let plural = CString::new("%1 files").unwrap();
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::catalog::ki18n_rs_catalog_translate(
        dom.as_ptr(),
        msgid.as_ptr(),
        plural.as_ptr(),
        2,
        ptr::null(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!out.is_null());
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert_eq!(s, "%1 files");
    capi::global::ki18n_rs_string_free(out);
}

#[test]
fn test_ffi_catalog_translate_not_found() {
    let _state_guard = common::global_state_test_lock();
    let domain = "ffi-catalog-missing";
    let _dir = setup_domain_mo(domain, "en", &[("known", "Known")]);

    let dom = CString::new(domain).unwrap();
    let msgid = CString::new("unknown-key").unwrap();
    let mut out: *mut c_char = ptr::null_mut();

    let res = capi::catalog::ki18n_rs_catalog_translate(
        dom.as_ptr(),
        msgid.as_ptr(),
        ptr::null(),
        0,
        ptr::null(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::NOT_FOUND as i32);
    assert!(out.is_null());
}

#[test]
fn test_ffi_catalog_translate_invalid_args() {
    let mut out: *mut c_char = ptr::null_mut();
    let res = capi::catalog::ki18n_rs_catalog_translate(
        ptr::null(),
        ptr::null(),
        ptr::null(),
        0,
        ptr::null(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::INVALID_ARGUMENT as i32);
}

#[test]
fn test_ffi_catalog_instance_translate_and_destroy() {
    let _state_guard = common::global_state_test_lock();
    let domain = "ffi-catalog-instance";
    let _dir = setup_domain_mo(domain, "en", &[("hello", "Hello FFI")]);

    let dom = CString::new(domain).unwrap();
    let lang = CString::new("en").unwrap();
    let handle = capi::catalog::ki18n_rs_catalog_create(dom.as_ptr(), lang.as_ptr());
    assert!(!handle.is_null());

    let msgid = CString::new("hello").unwrap();
    let mut out: *mut c_char = ptr::null_mut();
    let res = capi::catalog::ki18n_rs_catalog_translate_instance(
        handle,
        msgid.as_ptr(),
        ptr::null(),
        0,
        ptr::null(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!out.is_null());
    let s = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert_eq!(s, "Hello FFI");
    capi::global::ki18n_rs_string_free(out);
    capi::catalog::ki18n_rs_catalog_destroy(handle);
}

#[test]
fn test_ffi_catalog_create_invalid_returns_null() {
    let handle = capi::catalog::ki18n_rs_catalog_create(ptr::null(), ptr::null());
    assert!(handle.is_null());
}
