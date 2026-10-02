// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::fs;
use std::path::Path;

use ki18n_rs::catalog::{Catalog, CatalogError};

#[test]
fn test_mo_big_endian_load_and_lookup() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello BE")];
    let bytes = common::build_test_mo_bytes(header, entries, true, 0);

    let dir = common::ensure_data_dir();
    let path = dir.join("catalog_be.mo");
    fs::write(&path, &bytes).expect("write be mo");

    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load be mo");
    assert_eq!(catalog.get_translated_message("hello", 0), Some("Hello BE"));
}

#[test]
fn test_mo_invalid_magic() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let mut bytes = common::build_test_mo_bytes(header, entries, false, 0);
    bytes[0..4].copy_from_slice(&0xBAD0_BAD0u32.to_le_bytes());

    let dir = common::ensure_data_dir();
    let path = dir.join("catalog_bad_magic.mo");
    fs::write(&path, &bytes).expect("write bad magic mo");

    let err = Catalog::load_mo(Path::new(&path), Some("en")).unwrap_err();
    assert!(matches!(err, CatalogError::InvalidMagic));
}

#[test]
fn test_mo_unsupported_revision() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let bytes = common::build_test_mo_bytes(header, entries, false, 1);

    let dir = common::ensure_data_dir();
    let path = dir.join("catalog_rev1.mo");
    fs::write(&path, &bytes).expect("write rev1 mo");

    let err = Catalog::load_mo(Path::new(&path), Some("en")).unwrap_err();
    assert!(matches!(err, CatalogError::UnsupportedFormat));
}

#[test]
fn test_catalog_get_message_nul_eot_compat() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("ctx\u{4}key", "WithEOT")];
    let path = common::write_test_mo("catalog_eot_compat.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    // NUL separator lookup should find EOT-indexed entry.
    let nul_key = "ctx\u{0}key";
    assert_eq!(
        catalog.get_translated_message(nul_key, 0),
        Some("WithEOT")
    );
    assert_eq!(
        catalog.get_translated_message("ctx\u{4}key", 0),
        Some("WithEOT")
    );
}

#[test]
fn test_po_msgid_plural() {
    use std::io::Write;

    let dir = common::ensure_data_dir();
    let path = dir.join("catalog_po_plural.po");
    let mut f = fs::File::create(&path).unwrap();
    writeln!(
        f,
        r#"msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "file"
msgid_plural "files"
msgstr[0] "1 file"
msgstr[1] "%1 files"
"#
    )
    .unwrap();

    let cat = Catalog::load_po(&path, Some("en")).expect("load po");
    let idx1 = cat.get_plural_form(1);
    let idx2 = cat.get_plural_form(2);
    assert_eq!(cat.get_translated_message("file", idx1), Some("1 file"));
    assert_eq!(cat.get_translated_message("file", idx2), Some("%1 files"));
}
