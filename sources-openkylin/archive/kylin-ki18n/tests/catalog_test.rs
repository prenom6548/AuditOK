// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::path::Path;

use ki18n_rs::catalog::Catalog;

#[test]
fn test_mo_load_basic_and_lookup() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("catalog_basic.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    assert_eq!(catalog.get_translated_message("hello", 0), Some("Hello"));
}

#[test]
fn test_mo_plural_lookup() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "1 file\0%1 files")];
    let path = common::write_test_mo("catalog_plural.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let idx1 = catalog.get_plural_form(1);
    let idx2 = catalog.get_plural_form(2);
    assert_eq!(
        catalog.get_translated_message("files", idx1),
        Some("1 file")
    );
    assert_eq!(
        catalog.get_translated_message("files", idx2),
        Some("%1 files")
    );
}

#[test]
fn test_po_load_basic_and_lookup() {
    use std::fs;
    use std::io::Write;

    let dir = common::ensure_data_dir();
    let path = dir.join("test.po");
    if !path.exists() {
        let mut f = fs::File::create(&path).unwrap();
        writeln!(
            f,
            r#"msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "hello"
msgstr "Hello from PO"
"#
        )
        .unwrap();
    }
    let cat = Catalog::load_po(&path, Some("en")).expect("load po");
    assert_eq!(cat.get_translated_message("hello", 0), Some("Hello from PO"));
}

// ---------------------------------------------------------------------------
// 追加 Catalog 测试：更多 mo/po 边界与复数形式
// ---------------------------------------------------------------------------

#[test]
fn test_mo_load_with_context_like_key() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("ctx\u{4}hello", "Hello CTX")];
    let path = common::write_test_mo("catalog_ctx.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    assert_eq!(
        catalog.get_translated_message("ctx\u{4}hello", 0),
        Some("Hello CTX")
    );
}

#[test]
fn test_mo_missing_translation_falls_back_to_none() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("known", "Known")];
    let path = common::write_test_mo("catalog_missing.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    assert_eq!(catalog.get_translated_message("unknown", 0), None);
}

#[test]
fn test_catalog_plural_form_arabic_like_rule() {
    let header = "nplurals=6; plural=(n==0?0:n==1?1:n==2?2:n%100>=3 && n%100<=10?3:n%100>=11 && n%100<=99?4:5);";
    let entries = &[("files", "z\0a\0b\0c\0d\0e")];
    let path = common::write_test_mo("catalog_ar.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("ar")).expect("load mo");

    assert_eq!(catalog.get_plural_form(0), 0);
    assert_eq!(catalog.get_plural_form(1), 1);
    assert_eq!(catalog.get_plural_form(2), 2);
    assert_eq!(catalog.get_plural_form(3), 3);
    assert_eq!(catalog.get_plural_form(11), 4);
    assert_eq!(catalog.get_plural_form(100), 5);
}

// coverage-task-5-plural-catalog: Catalog 扩展 -------------------------------

#[test]
fn test_catalog_multiple_translations() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[
        ("hello", "Hello"),
        ("world", "World"),
        ("ctx\u{4}key", "WithCtx"),
    ];
    let path = common::write_test_mo("catalog_multi.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    assert_eq!(catalog.get_translated_message("hello", 0), Some("Hello"));
    assert_eq!(catalog.get_translated_message("world", 0), Some("World"));
    assert_eq!(catalog.get_translated_message("ctx\u{4}key", 0), Some("WithCtx"));
}

#[test]
fn test_catalog_fallback_language_header_missing() {
    use std::fs;
    use std::io::Write;

    let dir = common::ensure_data_dir();
    let path = dir.join("fallback.po");
    let mut f = fs::File::create(&path).unwrap();
    // 不含 Plural-Forms，要求解析时使用默认规则
    writeln!(
        f,
        r#"msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"

msgid "hello"
msgstr "Hello Fallback"
"#
    )
    .unwrap();

    let cat = Catalog::load_po(&path, Some("en")).expect("load po");
    assert_eq!(
        cat.get_translated_message("hello", 0),
        Some("Hello Fallback")
    );
}

#[test]
fn test_catalog_empty_catalog_handled() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries: &[(&str, &str)] = &[];
    let path = common::write_test_mo("catalog_empty.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    assert_eq!(catalog.get_translated_message("anything", 0), None);
}

#[test]
fn test_catalog_context_lookups_distinct() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[
        ("ctx1\u{4}key", "V1"),
        ("ctx2\u{4}key", "V2"),
        ("key", "V0"),
    ];
    let path = common::write_test_mo("catalog_ctx_multi.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    assert_eq!(
        catalog.get_translated_message("ctx1\u{4}key", 0),
        Some("V1")
    );
    assert_eq!(
        catalog.get_translated_message("ctx2\u{4}key", 0),
        Some("V2")
    );
    assert_eq!(catalog.get_translated_message("key", 0), Some("V0"));
}

#[test]
fn test_catalog_plural_forms_parsing_from_mo_header() {
    let header = "nplurals=3; plural=(n==1?0:(n==2?1:2));";
    let entries = &[("files", "one\0two\0many")];
    let path = common::write_test_mo("catalog_pf.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("xx")).expect("load mo");

    let i1 = catalog.get_plural_form(1);
    let i2 = catalog.get_plural_form(2);
    let i3 = catalog.get_plural_form(5);

    assert_eq!(catalog.get_translated_message("files", i1), Some("one"));
    assert_eq!(catalog.get_translated_message("files", i2), Some("two"));
    assert_eq!(catalog.get_translated_message("files", i3), Some("many"));
}

