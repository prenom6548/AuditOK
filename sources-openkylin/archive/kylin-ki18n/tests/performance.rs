// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::catalog::Catalog;
use ki18n_rs::message::Message;
use std::path::Path;

#[test]
fn perf_message_throughput() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("perf_en.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let ((), elapsed) = common::measure("message throughput", || {
        for _ in 0..10_000 {
            let msg = Message::new("hello", None, None, false);
            let _ = msg.finalize(&catalog, &vec!["en".to_string()]).unwrap();
        }
    });
    // 只是基本性能 sanity check，不做严格断言
    assert!(elapsed > 0);
}

#[test]
fn perf_catalog_load() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("perf_load.mo", header, entries);
    let (cat, elapsed) =
        common::measure("catalog load", || Catalog::load_mo(Path::new(&path), Some("en")).unwrap());
    assert!(elapsed > 0);
    assert!(cat.get_translated_message("hello", 0).is_some());
}

#[test]
fn perf_country_lookup() {
    let base = common::write_test_mo("dummy.mo", "nplurals=2; plural=(n != 1);", &[("x", "x")]);
    std::env::set_var("KI18N_RS_DATA_DIR", base.parent().unwrap());
    let mut handle: *mut ki18n_rs::capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let code = std::ffi::CString::new("DE").unwrap();
    let (_, elapsed) = common::measure("country lookup", || {
        ki18n_rs::capi::locale::ki18n_rs_country_from_alpha2(code.as_ptr(), &mut handle)
    });
    assert!(elapsed > 0);
}

