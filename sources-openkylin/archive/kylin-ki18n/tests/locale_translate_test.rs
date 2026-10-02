// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::ffi::CString;
use std::fs;
use std::ptr;

use ki18n_rs::capi;
use ki18n_rs::reset_locale_languages_cache_for_tests;
use ki18n_rs::Ki18nRsError;

#[test]
fn translate_falls_through_when_first_catalog_has_no_entry() {
    let _guard = common::global_state_test_lock();
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=1; plural=0;";

    let tw_src = common::write_test_mo("tw-empty.mo", header, &[]);
    fs::copy(&tw_src, dir.path().join("zh_TW.mo")).expect("copy zh_TW mo");

    let cn_src = common::write_test_mo("tw-cn.mo", header, &[("Hello", "你好")]);
    fs::copy(&cn_src, dir.path().join("zh_CN.mo")).expect("copy zh_CN mo");

    let dom = CString::new("tw-dom").unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();

    capi::global::ki18n_rs_clear_languages();
    reset_locale_languages_cache_for_tests();
    capi::global::ki18n_rs_set_application_domain(dom.as_ptr());

    let zh_tw = CString::new("zh_TW").unwrap();
    let zh_cn = CString::new("zh_CN").unwrap();
    let langs = [zh_tw.as_ptr(), zh_cn.as_ptr()];
    capi::global::ki18n_rs_set_languages(langs.as_ptr(), langs.len());
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let text = CString::new("Hello").unwrap();
    let handle = capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let mut out: *mut std::os::raw::c_char = ptr::null_mut();
    let res = capi::message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res, Ki18nRsError::OK as i32);
    let s = unsafe { CString::from_raw(out) }.to_string_lossy().into_owned();
    assert_eq!(s, "你好");
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn capi_translate_uses_lc_messages_zh_tw() {
    let _guard = common::global_state_test_lock();
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=1; plural=0;";

    let mo_src = common::write_test_mo("lc-dom.mo", header, &[("Hi", "繁體")]);
    fs::copy(&mo_src, dir.path().join("zh_TW.mo")).expect("copy zh_TW mo");

    let dom = CString::new("lc-dom").unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();

    capi::global::ki18n_rs_clear_languages();
    reset_locale_languages_cache_for_tests();
    std::env::set_var("LANGUAGE", "");
    std::env::set_var("LC_ALL", "");
    std::env::set_var("LC_MESSAGES", "zh_TW.UTF-8");
    std::env::set_var("LANG", "en_US.UTF-8");
    capi::global::ki18n_rs_set_application_domain(dom.as_ptr());
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let text = CString::new("Hi").unwrap();
    let handle = capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let mut out: *mut std::os::raw::c_char = ptr::null_mut();
    let res = capi::message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res, Ki18nRsError::OK as i32);
    let s = unsafe { CString::from_raw(out) }.to_string_lossy().into_owned();
    assert_eq!(s, "繁體");

    capi::message::ki18n_rs_msg_destroy(handle);
    std::env::remove_var("LANGUAGE");
    std::env::remove_var("LC_ALL");
    std::env::remove_var("LC_MESSAGES");
    std::env::remove_var("LANG");
    reset_locale_languages_cache_for_tests();
}
