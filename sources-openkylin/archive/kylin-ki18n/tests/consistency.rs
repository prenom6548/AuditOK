// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::core::{catalog::Catalog, message::Message};
use std::path::Path;

#[test]
fn consistency_basic_i18n() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("cons_en.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let msg = Message::new("hello", None, None, false);
    let rust_out = msg.finalize(&catalog, &vec!["en".to_string()]).unwrap();

    // 这里无法在测试中直接调用 C++ 版本，只验证 Rust 输出稳定且与期望英文翻译一致
    assert_eq!(rust_out, "Hello");
}

#[test]
fn consistency_plural_english() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "1 file\0%1 files")];
    let path = common::write_test_mo("cons_en_plural.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut msg = Message::new("files", Some("%1 files"), None, false);
    msg.subs_i64(1, 0, 10, ' ');
    let out1 = msg.finalize(&catalog, &vec!["en".to_string()]).unwrap();

    let mut msg2 = Message::new("files", Some("%1 files"), None, false);
    msg2.subs_i64(2, 0, 10, ' ');
    let out2 = msg2.finalize(&catalog, &vec!["en".to_string()]).unwrap();

    assert!(out1.contains("1 file"));
    assert!(out2.contains("2"));
}

