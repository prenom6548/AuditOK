// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::ffi::CString;
use std::fs;

use ki18n_rs::capi;
use ki18n_rs::locale::iso_translate::translate_iso_message;

#[test]
fn test_translate_iso_message_from_temp_mo() {
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=1; plural=0;";
    let src = common::write_test_mo("iso_de.mo", header, &[("Germany", "Deutschland")]);
    fs::copy(&src, dir.path().join("de.mo")).expect("copy mo");

    let dom = CString::new("iso_3166-1").unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();
    capi::global::ki18n_rs_clear_languages();
    let lang = CString::new("de").unwrap();
    let langs = [lang.as_ptr()];
    capi::global::ki18n_rs_set_languages(langs.as_ptr(), langs.len());
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let tr = translate_iso_message("iso_3166-1", "Germany");
    assert_eq!(tr, "Deutschland");
}

#[test]
fn test_translate_iso_message_empty_and_fallback() {
    assert_eq!(translate_iso_message("iso_3166-1", ""), "");
    capi::global::ki18n_rs_clear_languages();
    assert_eq!(translate_iso_message("iso_3166-1", "UnknownLand"), "UnknownLand");
}

#[test]
fn test_effective_lang_chain_from_language_env() {
    let dir = tempfile::tempdir().expect("tempdir");
    let header = "nplurals=1; plural=0;";
    let src = common::write_test_mo("iso_lang_env.mo", header, &[("X", "Y")]);
    fs::copy(&src, dir.path().join("de.mo")).expect("copy mo");

    let dom = CString::new("iso_3166-2").unwrap();
    let dir_c = CString::new(dir.path().to_string_lossy().as_bytes()).unwrap();
    capi::global::ki18n_rs_clear_languages();
    ki18n_rs::reset_locale_languages_cache_for_tests();
    capi::global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    std::env::set_var("LANGUAGE", "de:en");
    let tr = translate_iso_message("iso_3166-2", "X");
    std::env::remove_var("LANGUAGE");
    ki18n_rs::reset_locale_languages_cache_for_tests();
    assert_eq!(tr, "Y");
}
