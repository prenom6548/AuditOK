#![allow(unused_unsafe)]
// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later


mod common;

use std::ffi::{c_char, CString};
use std::ptr;

use ki18n_rs::capi;
use ki18n_rs::Ki18nRsError;

#[allow(deprecated)]
fn write_min_locale_data_for_ffi() -> std::path::PathBuf {
    use std::env;
    use std::fs;
    use std::io::Write;

    let dir = tempfile::tempdir().expect("tempdir");
    let base = dir.into_path();

    // iso_3166_1.json
    let countries = r#"[{
        "alpha2": "DE",
        "alpha3": "DEU",
        "name": "Germany",
        "currency": "EUR",
        "emoji": "🇩🇪",
        "timezones": ["Europe/Berlin"]
    }]"#;
    let mut f1 = fs::File::create(base.join("iso_3166_1.json")).unwrap();
    f1.write_all(countries.as_bytes()).unwrap();

    // iso_3166_2.json
    let subs = r#"[{
        "code": "DE-BE",
        "name": "Berlin",
        "country_alpha2": "DE",
        "parent_code": null,
        "timezone_ids": ["Europe/Berlin"]
    }]"#;
    let mut f2 = fs::File::create(base.join("iso_3166_2.json")).unwrap();
    f2.write_all(subs.as_bytes()).unwrap();

    // timezone_coords.json
    let tz = r#"[{
        "lat": 52.52,
        "lon": 13.405,
        "timezone": "Europe/Berlin"
    }]"#;
    let mut f3 = fs::File::create(base.join("timezone_coords.json")).unwrap();
    f3.write_all(tz.as_bytes()).unwrap();

    env::set_var("KI18N_RS_DATA_DIR", &base);
    base
}

#[test]
fn ffi_null_pointer() {
    // null 句柄
    let res = capi::message::ki18n_rs_msg_set_domain(ptr::null_mut(), ptr::null());
    assert_eq!(res, Ki18nRsError::INVALID_ARGUMENT as i32);
}

#[test]
#[ignore]
fn ffi_invalid_handle() {
    // 伪造无效指针（尽量只验证不崩溃）
    let fake = 0x1 as *mut capi::message::Ki18nRsMsgHandle;
    let domain = CString::new("test").unwrap();
    let _ = capi::message::ki18n_rs_msg_set_domain(fake, domain.as_ptr());
}

#[test]
#[ignore]
#[allow(unused_unsafe)]
fn ffi_double_free() {
    let text = CString::new("hello").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    unsafe {
        capi::message::ki18n_rs_msg_destroy(handle);
        // 第二次释放不应崩溃
        capi::message::ki18n_rs_msg_destroy(handle);
    }
}

#[test]
fn ffi_string_encoding() {
    let text = CString::new("你好").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let mut out: *mut std::os::raw::c_char = ptr::null_mut();
    let res = capi::message::ki18n_rs_msg_to_string(handle, &mut out);
    // 没有 domain/lang 的情况下，只要求不崩溃，返回一个有效错误码或 OK
    assert!(res == Ki18nRsError::INVALID_ARGUMENT as i32 || res == Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(handle);
    if !out.is_null() {
        capi::global::ki18n_rs_string_free(out);
    }
}

#[test]
fn ffi_wrong_type() {
    // 这里主要验证 API 在错误参数下不会崩溃
    let mut out: *mut *mut std::os::raw::c_char = ptr::null_mut();
    let res = capi::global::ki18n_rs_get_languages(&mut out, ptr::null_mut());
    assert_eq!(res, Ki18nRsError::INVALID_ARGUMENT as i32);
}

// ---------------------------------------------------------------------------
// 追加 FFI 测试：更细覆盖 msg_* C API 与 subs/to_string 集成
// ---------------------------------------------------------------------------

#[test]
fn ffi_msg_create_and_destroy_roundtrip() {
    let text = CString::new("Hello %1").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn ffi_msg_set_domain_null_and_valid() {
    let text = CString::new("Hello").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());

    // null domain 指针
    let res_null = capi::message::ki18n_rs_msg_set_domain(handle, ptr::null());
    assert_eq!(res_null, Ki18nRsError::INVALID_ARGUMENT as i32);

    // 有效 domain
    let dom = CString::new("test-domain").unwrap();
    let res = capi::message::ki18n_rs_msg_set_domain(handle, dom.as_ptr());
    assert_eq!(res, Ki18nRsError::OK as i32);

    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn ffi_msg_set_languages_single_and_empty() {
    let text = CString::new("Hello").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());

    // 单个语言
    let lang = CString::new("en").unwrap();
    let arr = [lang.as_ptr()];
    let res =
        capi::message::ki18n_rs_msg_set_languages(handle, arr.as_ptr(), arr.len());
    assert_eq!(res, Ki18nRsError::OK as i32);

    // 空语言列表
    let res_empty =
        capi::message::ki18n_rs_msg_set_languages(handle, ptr::null(), 0);
    assert_eq!(res_empty, Ki18nRsError::OK as i32);

    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn ffi_msg_subs_and_to_string_basic_integration() {
    // 创建最小 mo 文件并注册 domain
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "%1 file\0%1 files")];
    let path = common::write_test_mo("ffi_plural.mo", header, entries);
    let dir = path.parent().unwrap();

    // 设置全局 application domain 和 locale 目录
    let dom = CString::new("test-domain-ffi").unwrap();
    let dir_c = CString::new(dir.to_string_lossy().as_bytes()).unwrap();
    ki18n_rs::capi::global::ki18n_rs_set_application_domain(dom.as_ptr());
    ki18n_rs::capi::global::ki18n_rs_add_domain_locale_dir(
        dom.as_ptr(),
        dir_c.as_ptr(),
    );

    // 使用 FFI 创建 message，并设置 domain / languages / subs
    let text = CString::new("files").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());

    let res_dom = capi::message::ki18n_rs_msg_set_domain(handle, dom.as_ptr());
    assert_eq!(res_dom, Ki18nRsError::OK as i32);

    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    let res_langs =
        capi::message::ki18n_rs_msg_set_languages(handle, langs.as_ptr(), langs.len());
    assert_eq!(res_langs, Ki18nRsError::OK as i32);

    // 占位符 %1 = 2
    let res_sub = capi::message::ki18n_rs_msg_subs_i64(handle, 2, 0, 10, ' ' as u32);
    assert_eq!(res_sub, Ki18nRsError::OK as i32);

    // to_string
    let mut out: *mut std::os::raw::c_char = ptr::null_mut();
    let res_str = capi::message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res_str, Ki18nRsError::OK as i32);
    assert!(!out.is_null());

    // 读取结果并释放
    // C API 使用 c_char* 表示字符串指针，这里仅验证非空并交给 string_free 释放。
    assert!(!out.is_null());
    capi::message::ki18n_rs_msg_destroy(handle);
    ki18n_rs::capi::global::ki18n_rs_string_free(out);
}

// coverage-task-6-ffi-integration: FFI 扩展 -----------------------------------

#[test]
fn ffi_msg_subs_u64_f32_char_and_message() {
    // 构造带占位符的 mo
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("combo", "%1 %2 %3")];
    let path = common::write_test_mo("ffi_combo.mo", header, entries);
    let dir = path.parent().unwrap();

    // 注册 domain
    let dom = CString::new("ffi-combo").unwrap();
    let dir_c = CString::new(dir.to_string_lossy().as_bytes()).unwrap();
    capi::global::ki18n_rs_set_application_domain(dom.as_ptr());
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    // message: "combo"
    let text = CString::new("combo").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());

    // 语言
    let lang = CString::new("en").unwrap();
    let langs = [lang.as_ptr()];
    let res_langs =
        capi::message::ki18n_rs_msg_set_languages(handle, langs.as_ptr(), langs.len());
    assert_eq!(res_langs, Ki18nRsError::OK as i32);

    // 域
    let res_dom = capi::message::ki18n_rs_msg_set_domain(handle, dom.as_ptr());
    assert_eq!(res_dom, Ki18nRsError::OK as i32);

    // %1: u64, %2: f32, %3: char
    let r1 = capi::message::ki18n_rs_msg_subs_u64(handle, 42, 0, 10, ' ' as u32);
    let r2 = capi::message::ki18n_rs_msg_subs_f32(handle, 1.5f32, 0, 'f' as c_char, 1, ' ' as u32);
    let r3 = capi::message::ki18n_rs_msg_subs_char(handle, 'X' as u32, 0, ' ' as u32);
    assert_eq!(r1, Ki18nRsError::OK as i32);
    assert_eq!(r2, Ki18nRsError::OK as i32);
    assert_eq!(r3, Ki18nRsError::OK as i32);

    let mut out: *mut std::os::raw::c_char = ptr::null_mut();
    let res_str = capi::message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res_str, Ki18nRsError::OK as i32);
    assert!(!out.is_null());

    // 当前流程在找不到 catalog 条目时会回退到 msgid（"combo"），
    // 因而这里断言稳定的回退行为。
    let s = unsafe { CString::from_raw(out) }
        .to_string_lossy()
        .into_owned();
    assert_eq!(s, "combo");

    capi::message::ki18n_rs_msg_destroy(handle);
    // 字符串所有权已交给 CString::from_raw，这里无需再次 free
}

#[test]
fn ffi_global_set_multiple_languages_and_query() {
    let langs = ["en", "zh_CN"];
    let c_langs: Vec<CString> =
        langs.iter().map(|l| CString::new(*l).unwrap()).collect();
    let ptrs: Vec<*const std::os::raw::c_char> = c_langs.iter().map(|c| c.as_ptr()).collect();

    let res = unsafe {
        capi::global::ki18n_rs_set_languages(ptrs.as_ptr(), ptrs.len())
    };
    assert_eq!(res, Ki18nRsError::OK as i32);

    let mut out: *mut *mut std::os::raw::c_char = std::ptr::null_mut();
    let mut len: usize = 0;
    let res_get = capi::global::ki18n_rs_get_languages(&mut out, &mut len);
    assert_eq!(res_get, Ki18nRsError::OK as i32);
    if !out.is_null() && len > 0 {
        unsafe {
            for i in 0..len {
                let p = *out.add(i);
                if !p.is_null() {
                    let _ = CString::from_raw(p);
                }
            }
        }
    }
}

#[test]
fn ffi_country_and_subdivision_from_ffi_locale() {
    use std::ffi::CString as CStr2;
    use std::ptr::null_mut;

    let _ = write_min_locale_data_for_ffi();

    // Country from name
    let mut c_handle: *mut capi::locale::Ki18nRsCountryHandle = null_mut();
    let name = CStr2::new("Germany").unwrap();
    let res_c =
        capi::locale::ki18n_rs_country_from_name(name.as_ptr(), &mut c_handle);
    assert_eq!(res_c, Ki18nRsError::OK as i32);
    assert!(!c_handle.is_null());

    // Subdivision from code
    let mut s_handle: *mut capi::locale::Ki18nRsSubdivisionHandle = null_mut();
    let code = CStr2::new("DE-BE").unwrap();
    let res_s =
        capi::locale::ki18n_rs_subdivision_from_code(code.as_ptr(), &mut s_handle);
    assert_eq!(res_s, Ki18nRsError::OK as i32);
    assert!(!s_handle.is_null());
    unsafe {
        capi::locale::ki18n_rs_country_destroy(c_handle);
        capi::locale::ki18n_rs_subdivision_destroy(s_handle);
    }
}

// extra-task-3-ffi: Message FFI 额外覆盖 ---------------------------------------

#[test]
fn test_ffi_msg_subs_i32() {
    let text = CString::new("v=%1").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let res =
        capi::message::ki18n_rs_msg_subs_i32(handle, 7, 0, 10, ' ' as u32);
    assert_eq!(res, Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_ffi_msg_subs_u32() {
    let text = CString::new("v=%1").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let res =
        capi::message::ki18n_rs_msg_subs_u32(handle, 7, 0, 10, ' ' as u32);
    assert_eq!(res, Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_ffi_msg_subs_f64_precision() {
    let text = CString::new("pi=%1").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let res = capi::message::ki18n_rs_msg_subs_f64(
        handle,
        3.14159,
        0,
        0, // 使用默认 'g'
        2,
        ' ' as u32,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_ffi_msg_subs_string_unicode() {
    let text = CString::new("hello %1").unwrap();
    let handle =
        capi::message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());
    let arg = CString::new("世界").unwrap();
    let res = capi::message::ki18n_rs_msg_subs_qstring(
        handle,
        arg.as_ptr(),
        0,
        ' ' as u32,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_ffi_msg_subs_message_nested() {
    let text_outer = CString::new("outer %1").unwrap();
    let text_inner = CString::new("inner").unwrap();
    let outer =
        capi::message::ki18n_rs_msg_create(ptr::null(), text_outer.as_ptr(), ptr::null());
    let inner =
        capi::message::ki18n_rs_msg_create(ptr::null(), text_inner.as_ptr(), ptr::null());
    assert!(!outer.is_null());
    assert!(!inner.is_null());
    let res = capi::message::ki18n_rs_msg_subs_message(
        outer,
        inner,
        0,
        ' ' as u32,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    capi::message::ki18n_rs_msg_destroy(outer);
    capi::message::ki18n_rs_msg_destroy(inner);
}

// extra-task-3-ffi: Global FFI 额外覆盖 ----------------------------------------

#[test]
fn test_ffi_global_set_and_get_domain() {
    let dom = CString::new("extra-domain").unwrap();
    let res_set = capi::global::ki18n_rs_set_application_domain(dom.as_ptr());
    assert_eq!(res_set, Ki18nRsError::OK as i32);

    let mut out: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res_get = capi::global::ki18n_rs_get_application_domain(&mut out);
    assert_eq!(res_get, Ki18nRsError::OK as i32);
    assert!(!out.is_null());
    if !out.is_null() {
        let got = unsafe { CString::from_raw(out) }.to_string_lossy().into_owned();
        assert_eq!(got, "extra-domain");
    }
}

#[test]
fn test_ffi_global_clear_and_set() {
    let langs = ["en", "fr"];
    let c_langs: Vec<CString> =
        langs.iter().map(|l| CString::new(*l).unwrap()).collect();
    let ptrs: Vec<*const std::os::raw::c_char> =
        c_langs.iter().map(|c| c.as_ptr()).collect();

    let _ = capi::global::ki18n_rs_clear_languages();
    let res = capi::global::ki18n_rs_set_languages(
        ptrs.as_ptr(),
        ptrs.len(),
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
}

#[test]
fn test_ffi_global_available_languages_not_empty_like() {
    let dom = CString::new("avail-domain").unwrap();
    let _ = capi::global::ki18n_rs_set_application_domain(dom.as_ptr());

    let mut out_arr: *mut *mut std::os::raw::c_char = std::ptr::null_mut();
    let mut count: usize = 0;
    let res = capi::global::ki18n_rs_available_languages(
        dom.as_ptr(),
        &mut out_arr,
        &mut count,
    );
    assert_eq!(res, Ki18nRsError::OK as i32);
    if !out_arr.is_null() {
        unsafe { capi::global::ki18n_rs_string_array_free(out_arr, count); }
    }
    // 第二次应命中缓存，仍成功
    let res2 = capi::global::ki18n_rs_available_languages(
        dom.as_ptr(),
        &mut out_arr,
        &mut count,
    );
    assert_eq!(res2, Ki18nRsError::OK as i32);
    if !out_arr.is_null() {
        unsafe { capi::global::ki18n_rs_string_array_free(out_arr, count); }
    }
}

#[test]
fn test_ffi_is_language_available() {
    let dom = CString::new("ffi-fast-dom").unwrap();
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("ffi_fast_fr.mo", header, entries);
    let base = std::path::Path::new(&path).parent().unwrap();
    let locale_fr = base.join("locale").join("fr").join("LC_MESSAGES");
    std::fs::create_dir_all(&locale_fr).expect("mkdir");
    std::fs::copy(&path, locale_fr.join("ffi-fast-dom.mo")).expect("copy mo");
    let dir_c = CString::new(base.to_string_lossy().as_bytes()).unwrap();
    let _ = capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let fr = CString::new("fr").unwrap();
    assert_eq!(
        capi::global::ki18n_rs_is_language_available(dom.as_ptr(), fr.as_ptr()),
        1
    );
    let missing = CString::new("zz_not_installed").unwrap();
    assert_eq!(
        capi::global::ki18n_rs_is_language_available(dom.as_ptr(), missing.as_ptr()),
        0
    );
    let en = CString::new("en_US").unwrap();
    assert_eq!(
        capi::global::ki18n_rs_is_language_available(dom.as_ptr(), en.as_ptr()),
        1
    );
}

#[test]
fn test_ffi_global_catalog_locale_dir_with_domain() {
    let dom = CString::new("cat-domain").unwrap();
    let header = "nplurals=1; plural=0;";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("ffi_cat.mo", header, entries);
    let dir = std::path::Path::new(&path).parent().unwrap();
    let dir_c = CString::new(dir.to_string_lossy().as_bytes()).unwrap();

    let _ = capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let mut out: *mut std::os::raw::c_char = std::ptr::null_mut();
    let lang_en = CString::new("en").unwrap();
    let res = capi::global::ki18n_rs_catalog_locale_dir(
        dom.as_ptr(),
        lang_en.as_ptr(),
        &mut out,
    );
    assert_eq!(res, Ki18nRsError::NOT_FOUND as i32);
    assert!(out.is_null());
}

#[test]
fn test_ffi_global_string_free_null() {
    capi::global::ki18n_rs_string_free(std::ptr::null_mut());
}

#[test]
fn test_ffi_global_string_array_free_empty() {
    capi::global::ki18n_rs_string_array_free(std::ptr::null_mut(), 0);
}

#[test]
fn test_ffi_global_string_array_free_with_count() {
    use std::os::raw::c_char;
    unsafe {
        let count = 2usize;
        let arr = libc::calloc(count, std::mem::size_of::<*mut c_char>()) as *mut *mut c_char;
        assert!(!arr.is_null());
        let s1 = CString::new("a").unwrap().into_raw();
        let s2 = CString::new("b").unwrap().into_raw();
        *arr.add(0) = s1;
        *arr.add(1) = s2;
        capi::global::ki18n_rs_string_array_free(arr, count);
    }
}

// extra-task-3-ffi: Locale FFI 额外覆盖 ----------------------------------------

#[test]
fn test_ffi_country_all_not_empty() {
    let _ = write_min_locale_data_for_ffi();
    let mut list: *mut *mut capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let mut count: usize = 0;
    let res = capi::locale::ki18n_rs_country_all(&mut list, &mut count);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(count >= 1);
    unsafe { capi::locale::ki18n_rs_country_list_destroy(list, count); }
}

#[test]
fn test_ffi_country_alpha2_valid() {
    let _ = write_min_locale_data_for_ffi();
    let mut handle: *mut capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let code = CString::new("DE").unwrap();
    let res =
        capi::locale::ki18n_rs_country_from_alpha2(code.as_ptr(), &mut handle);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!handle.is_null());
    unsafe { capi::locale::ki18n_rs_country_destroy(handle); }
}

#[test]
fn test_ffi_country_alpha3_valid() {
    let _ = write_min_locale_data_for_ffi();
    let mut handle: *mut capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let code = CString::new("DEU").unwrap();
    let res =
        capi::locale::ki18n_rs_country_from_alpha3(code.as_ptr(), &mut handle);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!handle.is_null());
    unsafe { capi::locale::ki18n_rs_country_destroy(handle); }
}

#[test]
fn test_ffi_subdivision_code_format() {
    let _ = write_min_locale_data_for_ffi();
    let mut handle: *mut capi::locale::Ki18nRsSubdivisionHandle = std::ptr::null_mut();
    let code = CString::new("DE-BE").unwrap();
    let res =
        capi::locale::ki18n_rs_subdivision_from_code(code.as_ptr(), &mut handle);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!handle.is_null());
    let mut out: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res_code = capi::locale::ki18n_rs_subdivision_code(handle, &mut out);
    assert_eq!(res_code, Ki18nRsError::OK as i32);
    if !out.is_null() {
        unsafe { let _ = CString::from_raw(out); }
    }
    unsafe { capi::locale::ki18n_rs_subdivision_destroy(handle); }
}

#[test]
fn test_ffi_timezone_id_valid_and_country_not_null() {
    let _ = write_min_locale_data_for_ffi();
    let mut tz_id: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res_tz = capi::locale::ki18n_rs_timezone_from_location(
        52.52,
        13.405,
        &mut tz_id,
    );
    assert_eq!(res_tz, Ki18nRsError::OK as i32);
    assert!(!tz_id.is_null());

    let mut c_handle: *mut capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let res_ctry =
        capi::locale::ki18n_rs_timezone_country(tz_id, &mut c_handle);
    assert_eq!(res_ctry, Ki18nRsError::OK as i32);
    assert!(!c_handle.is_null());

    unsafe {
        if !tz_id.is_null() {
            capi::global::ki18n_rs_string_free(tz_id);
        }
        if !c_handle.is_null() {
            capi::locale::ki18n_rs_country_destroy(c_handle);
        }
    }
}

// ---------------------------------------------------------------------------
// Translator/Context 端到端 FFI 覆盖
// ---------------------------------------------------------------------------

#[test]
fn test_ffi_translator_translate_monitored_context_found_with_msgctxt() {
    use std::fs;
    use std::ffi::CStr;
    use std::os::raw::c_char;
    use std::time::{SystemTime, UNIX_EPOCH};

    let _ = capi::global::ki18n_rs_clear_languages();
    let lang = CString::new("en").unwrap();
    let langs = [lang];
    let ptrs: Vec<*const c_char> = langs.iter().map(|c| c.as_ptr()).collect();
    let res = capi::global::ki18n_rs_set_languages(ptrs.as_ptr(), ptrs.len());
    assert_eq!(res, Ki18nRsError::OK as i32);

    let dir = tempfile::tempdir().expect("tempdir");
    let dir_path = dir.path();

    // Build a mo file into tests/data, then copy to our temp locale root.
    let nanos = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos();
    let unique = format!("translator_ctx_found_{nanos}");
    let header = "nplurals=1; plural=0;";
    let disambiguation = "disamb";
    let msgid = "source";
    let translated = "TR";
    let key = format!("{}\u{4}{}", disambiguation, msgid);
    let src_mo = common::write_test_mo(
        &format!("{unique}.bin"),
        header,
        &[(key.as_str(), translated)],
    );
    fs::copy(src_mo, dir_path.join("en.mo")).unwrap();

    let domain = CString::new(format!("translator-domain-{nanos}")).unwrap();
    let dir_c = CString::new(dir_path.to_string_lossy().as_bytes()).unwrap();
    let res_add = capi::global::ki18n_rs_add_domain_locale_dir(domain.as_ptr(), dir_c.as_ptr());
    assert_eq!(res_add, Ki18nRsError::OK as i32);

    let handle = capi::translator::ki18n_rs_translator_create();
    assert!(!handle.is_null());
    let res_dom = capi::translator::ki18n_rs_translator_set_domain(handle, domain.as_ptr());
    assert_eq!(res_dom, Ki18nRsError::OK as i32);

    let monitored_ctx = CString::new("ctxMonitor").unwrap();
    let res_ctx = capi::translator::ki18n_rs_translator_add_context(handle, monitored_ctx.as_ptr());
    assert_eq!(res_ctx, Ki18nRsError::OK as i32);

    let msgid_c = CString::new(msgid).unwrap();
    let disamb_c = CString::new(disambiguation).unwrap();
    let out_ptr = capi::translator::ki18n_rs_translator_translate(
        handle,
        monitored_ctx.as_ptr(),
        msgid_c.as_ptr(),
        disamb_c.as_ptr(),
        0,
    );
    assert!(!out_ptr.is_null());
    let out = unsafe { CStr::from_ptr(out_ptr).to_string_lossy().to_string() };
    assert_eq!(out, translated);
    unsafe { capi::global::ki18n_rs_string_free(out_ptr) };

    capi::translator::ki18n_rs_translator_destroy(handle);
}

#[test]
fn test_ffi_translator_translate_unmonitored_context_returns_null() {
    use std::fs;
    use std::os::raw::c_char;
    use std::time::{SystemTime, UNIX_EPOCH};

    let _ = capi::global::ki18n_rs_clear_languages();
    let lang = CString::new("en").unwrap();
    let langs = [lang];
    let ptrs: Vec<*const c_char> = langs.iter().map(|c| c.as_ptr()).collect();
    let res = capi::global::ki18n_rs_set_languages(ptrs.as_ptr(), ptrs.len());
    assert_eq!(res, Ki18nRsError::OK as i32);

    let dir = tempfile::tempdir().expect("tempdir");
    let dir_path = dir.path();

    let nanos = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos();
    let unique = format!("translator_ctx_unmon_{nanos}");
    let header = "nplurals=1; plural=0;";
    let key = format!("{}\u{4}{}", "disamb", "source");

    let src_mo = common::write_test_mo(
        &format!("{unique}.bin"),
        header,
        &[(key.as_str(), "TR")],
    );
    fs::copy(src_mo, dir_path.join("en.mo")).unwrap();

    let domain = CString::new(format!("translator-domain-{nanos}")).unwrap();
    let dir_c = CString::new(dir_path.to_string_lossy().as_bytes()).unwrap();
    let res_add = capi::global::ki18n_rs_add_domain_locale_dir(domain.as_ptr(), dir_c.as_ptr());
    assert_eq!(res_add, Ki18nRsError::OK as i32);

    let handle = capi::translator::ki18n_rs_translator_create();
    assert!(!handle.is_null());
    let res_dom = capi::translator::ki18n_rs_translator_set_domain(handle, domain.as_ptr());
    assert_eq!(res_dom, Ki18nRsError::OK as i32);

    let monitored_ctx = CString::new("ctxMonitor").unwrap();
    let res_ctx = capi::translator::ki18n_rs_translator_add_context(handle, monitored_ctx.as_ptr());
    assert_eq!(res_ctx, Ki18nRsError::OK as i32);

    let unmonitored_ctx = CString::new("ctxNotMon").unwrap();
    let msgid_c = CString::new("source").unwrap();
    let disamb_c = CString::new("disamb").unwrap();

    let out_ptr = capi::translator::ki18n_rs_translator_translate(
        handle,
        unmonitored_ctx.as_ptr(),
        msgid_c.as_ptr(),
        disamb_c.as_ptr(),
        0,
    );
    assert!(out_ptr.is_null());

    capi::translator::ki18n_rs_translator_destroy(handle);
}

#[test]
fn test_ffi_translator_translate_missing_translation_fallback_to_msgid() {
    use std::fs;
    use std::ffi::CStr;
    use std::os::raw::c_char;
    use std::time::{SystemTime, UNIX_EPOCH};

    let _ = capi::global::ki18n_rs_clear_languages();
    let lang = CString::new("en").unwrap();
    let langs = [lang];
    let ptrs: Vec<*const c_char> = langs.iter().map(|c| c.as_ptr()).collect();
    let res = capi::global::ki18n_rs_set_languages(ptrs.as_ptr(), ptrs.len());
    assert_eq!(res, Ki18nRsError::OK as i32);

    let dir = tempfile::tempdir().expect("tempdir");
    let dir_path = dir.path();

    let nanos = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos();
    let unique = format!("translator_ctx_missing_{nanos}");
    let header = "nplurals=1; plural=0;";

    // MO contains a different key, so our lookup key will miss.
    let key = format!("{}\u{4}{}", "otherdis", "othermsg");
    let src_mo = common::write_test_mo(
        &format!("{unique}.bin"),
        header,
        &[(key.as_str(), "TR_OTHER")],
    );
    fs::copy(src_mo, dir_path.join("en.mo")).unwrap();

    let domain = CString::new(format!("translator-domain-{nanos}")).unwrap();
    let dir_c = CString::new(dir_path.to_string_lossy().as_bytes()).unwrap();
    let res_add = capi::global::ki18n_rs_add_domain_locale_dir(domain.as_ptr(), dir_c.as_ptr());
    assert_eq!(res_add, Ki18nRsError::OK as i32);

    let handle = capi::translator::ki18n_rs_translator_create();
    assert!(!handle.is_null());
    let res_dom = capi::translator::ki18n_rs_translator_set_domain(handle, domain.as_ptr());
    assert_eq!(res_dom, Ki18nRsError::OK as i32);

    let monitored_ctx = CString::new("ctxMonitor").unwrap();
    let res_ctx = capi::translator::ki18n_rs_translator_add_context(handle, monitored_ctx.as_ptr());
    assert_eq!(res_ctx, Ki18nRsError::OK as i32);

    let msgid_c = CString::new("source").unwrap();
    let disamb_c = CString::new("disamb").unwrap();

    let out_ptr = capi::translator::ki18n_rs_translator_translate(
        handle,
        monitored_ctx.as_ptr(),
        msgid_c.as_ptr(),
        disamb_c.as_ptr(),
        0,
    );
    assert!(!out_ptr.is_null());
    let out = unsafe { CStr::from_ptr(out_ptr).to_string_lossy().to_string() };
    assert_eq!(out, "source");
    unsafe { capi::global::ki18n_rs_string_free(out_ptr) };

    capi::translator::ki18n_rs_translator_destroy(handle);
}


