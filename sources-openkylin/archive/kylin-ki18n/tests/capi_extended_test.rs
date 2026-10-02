// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::ffi::{CStr, CString};
use std::fs;
use std::os::raw::{c_char, c_int};
use std::path::Path;
use std::ptr;
use std::time::{SystemTime, UNIX_EPOCH};

use ki18n_rs::capi;
use ki18n_rs::catalog::Catalog;
use ki18n_rs::message::Message;
use ki18n_rs::Ki18nRsError;

#[test]
fn test_finalize_script_fence_ordinary_and_scripted() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo(
        "script_fence.mo",
        header,
        &[("greet", "Ignored|/|$[upper hello]")],
    );
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("greet", None, None, false);
    let out = msg.finalize(&cat, &["en".to_string()]).unwrap();
    assert_eq!(out, "HELLO");
}

#[test]
fn test_finalize_script_fence_at_start_falls_back_to_msgid() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo(
        "script_fence_start.mo",
        header,
        &[("greet", "|/|$[upper x]")],
    );
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("greet", None, None, false);
    let out = msg.finalize(&cat, &["en".to_string()]).unwrap();
    assert_eq!(out, "greet");
}

#[test]
fn test_ffi_msg_create_markup_clone_and_untranslated() {
    let _state_guard = common::global_state_test_lock();

    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("@b{Bold}", "<b>Bold</b>")];
    let path = common::write_test_mo("ffi_markup.mo", header, entries);
    let dir = path.parent().unwrap();

    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let domain = format!("ffi-markup-{nanos}");
    let dom = CString::new(domain.as_str()).unwrap();
    let dir_c = CString::new(dir.to_string_lossy().as_bytes()).unwrap();
    capi::global::ki18n_rs_set_application_domain(dom.as_ptr());
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    capi::global::ki18n_rs_set_languages(langs.as_ptr(), langs.len());

    let text = CString::new("@b{Bold}").unwrap();
    let handle = capi::message::ki18n_rs_msg_create_markup(
        ptr::null(),
        text.as_ptr(),
        ptr::null(),
    );
    assert!(!handle.is_null());
    capi::message::ki18n_rs_msg_set_domain(handle, dom.as_ptr());

    let mut untranslated: *mut c_char = ptr::null_mut();
    assert_eq!(
        capi::message::ki18n_rs_msg_get_untranslated_text(handle, &mut untranslated),
        Ki18nRsError::OK as i32
    );
    assert!(!untranslated.is_null());
    let raw = unsafe { CStr::from_ptr(untranslated).to_string_lossy().into_owned() };
    assert_eq!(raw, "@b{Bold}");
    capi::global::ki18n_rs_string_free(untranslated);

    let cloned = capi::message::ki18n_rs_msg_clone(handle);
    assert!(!cloned.is_null());

    let mut out: *mut c_char = ptr::null_mut();
    assert_eq!(
        capi::message::ki18n_rs_msg_to_string(cloned, &mut out),
        Ki18nRsError::OK as i32
    );
    assert!(!out.is_null());
    let rendered = unsafe { CStr::from_ptr(out).to_string_lossy().into_owned() };
    assert!(rendered.contains("Bold"));
    capi::global::ki18n_rs_string_free(out);

    capi::message::ki18n_rs_msg_destroy(cloned);
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_ffi_kuit_set_format_and_tag_pattern() {
    let tag = CString::new("em").unwrap();
    capi::kuit::ki18n_rs_kuit_set_format(ptr::null(), 0, tag.as_ptr(), 20, ptr::null());

    let attr = CString::new("title").unwrap();
    let attrs = [attr.as_ptr()];
    unsafe {
        capi::kuit::ki18n_rs_kuit_set_tag_pattern(
            ptr::null(),
            tag.as_ptr(),
            attrs.as_ptr(),
            attrs.len(),
            20,
            ptr::null(),
            0,
            0,
        );
    }

    capi::kuit::ki18n_rs_kuit_register_formatter_invoker(None);
    capi::kuit::ki18n_rs_kuit_register_warning_handler(None);

    let tooltip_attr = CString::new("tooltip").unwrap();
    capi::kuit::ki18n_rs_kuit_set_tooltip_attribute(ptr::null(), tag.as_ptr(), tooltip_attr.as_ptr());
}

#[test]
fn test_ffi_dup_utf8_and_translate_qt() {
    let _state_guard = common::global_state_test_lock();

    let bytes = b"dup-test";
    let mut out: *mut c_char = ptr::null_mut();
    unsafe {
        let res = capi::global::ki18n_rs_dup_utf8(bytes.as_ptr(), bytes.len(), &mut out);
        assert_eq!(res, Ki18nRsError::OK as i32);
        assert!(!out.is_null());
        let s = CStr::from_ptr(out).to_string_lossy().into_owned();
        assert_eq!(s, "dup-test");
        capi::global::ki18n_rs_string_free(out);
    }

    unsafe {
        let res_null = capi::global::ki18n_rs_dup_utf8(ptr::null(), 1, &mut out);
        assert_eq!(res_null, Ki18nRsError::INVALID_ARGUMENT as i32);
    }

    let dir = tempfile::tempdir().expect("tempdir");
    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let domain = format!("qt-dom-ffi-{nanos}");
    let header = "nplurals=1; plural=0;";
    let src = common::write_test_mo(
        &format!("ffi_qt_{nanos}.mo"),
        header,
        &[("Landscape", "Paysage")],
    );
    let mo_dir = dir
        .path()
        .join("locale")
        .join("fr")
        .join("LC_MESSAGES");
    fs::create_dir_all(&mo_dir).expect("mkdir");
    fs::copy(&src, mo_dir.join(format!("{domain}.mo"))).expect("copy mo");

    capi::global::ki18n_rs_clear_languages();
    let fr = CString::new("fr").unwrap();
    let en = CString::new("en_US").unwrap();
    let langs = [fr.as_ptr(), en.as_ptr()];
    capi::global::ki18n_rs_set_languages(langs.as_ptr(), langs.len());

    let dom = CString::new(domain.as_str()).unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());
    capi::global::ki18n_rs_qt_domain_insert(dom.as_ptr());

    let ctx = CString::new("QPrintPreviewDialog").unwrap();
    let source = CString::new("Landscape").unwrap();
    let mut qt_out: *mut c_char = ptr::null_mut();
    let res = capi::global::ki18n_rs_translate_qt(
        ctx.as_ptr(),
        source.as_ptr(),
        ptr::null(),
        0,
        &mut qt_out,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!qt_out.is_null());
    let tr = unsafe { CStr::from_ptr(qt_out).to_string_lossy().into_owned() };
    assert_eq!(tr, "Paysage");
    capi::global::ki18n_rs_string_free(qt_out);
    capi::global::ki18n_rs_qt_domain_remove(dom.as_ptr());
}

#[test]
fn test_ffi_ktranscript_eval_basic_and_empty_argv() {
    use ki18n_rs::paths::ki18n_autotests_dir;

    let mut out_result: *mut c_char = ptr::null_mut();
    let mut out_error: *mut c_char = ptr::null_mut();
    let mut out_fallback: c_int = 0;

    unsafe {
        let res = capi::transcript::ki18n_rs_ktranscript_eval(
            ptr::null(),
            0,
            ptr::null(),
            ptr::null(),
            ptr::null(),
            ptr::null(),
            ptr::null(),
            0,
            ptr::null(),
            ptr::null(),
            0,
            ptr::null(),
            0,
            ptr::null(),
            ptr::null(),
            ptr::null(),
            0,
            &mut out_result,
            &mut out_error,
            &mut out_fallback,
        );
        assert_eq!(res, Ki18nRsError::OK as i32);
        assert!(!out_result.is_null());
        assert!(out_error.is_null());
        assert_eq!(out_fallback, 0);
        capi::global::ki18n_rs_string_free(out_result);
    }

    let test_js = ki18n_autotests_dir().join("test.js");
    if !test_js.is_file() {
        return;
    }
    let path_c = CString::new(test_js.to_string_lossy().as_bytes()).unwrap();
    let lang_c = CString::new("fr").unwrap();
    let argv0 = CString::new("test_basic").unwrap();
    let argv1 = CString::new("foo").unwrap();
    let argv = [argv0.as_ptr(), argv1.as_ptr()];
    let mod_paths = [path_c.as_ptr()];
    let mod_langs = [lang_c.as_ptr()];

    unsafe {
        let res = capi::transcript::ki18n_rs_ktranscript_eval(
            argv.as_ptr(),
            argv.len(),
            lang_c.as_ptr(),
            ptr::null(),
            ptr::null(),
            ptr::null(),
            ptr::null(),
            0,
            ptr::null(),
            ptr::null(),
            0,
            ptr::null(),
            0,
            ptr::null(),
            mod_paths.as_ptr(),
            mod_langs.as_ptr(),
            1,
            &mut out_result,
            &mut out_error,
            &mut out_fallback,
        );
        assert_eq!(res, Ki18nRsError::OK as i32);
        assert!(!out_result.is_null());
        let s = CStr::from_ptr(out_result).to_string_lossy().into_owned();
        assert_eq!(s, "foo bar");
        capi::global::ki18n_rs_string_free(out_result);
    }
}

#[test]
fn test_ffi_translator_remove_context() {
    let handle = capi::translator::ki18n_rs_translator_create();
    assert!(!handle.is_null());

    let ctx = CString::new("ctxA").unwrap();
    assert_eq!(
        capi::translator::ki18n_rs_translator_add_context(handle, ctx.as_ptr()),
        Ki18nRsError::OK as i32
    );
    assert_eq!(
        capi::translator::ki18n_rs_translator_remove_context(handle, ctx.as_ptr()),
        Ki18nRsError::OK as i32
    );
    capi::translator::ki18n_rs_translator_destroy(handle);
}
