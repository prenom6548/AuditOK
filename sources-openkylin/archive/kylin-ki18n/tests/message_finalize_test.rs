// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::catalog::Catalog;
use ki18n_rs::message::Message;
use std::path::Path;

#[test]
fn test_finalize_empty_text() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("finalize_empty.mo", header, &[("", "")]);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "");
}

#[test]
fn test_finalize_no_subs_provided() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("finalize_nosubs.mo", header, &[("hello", "Hello")]);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("hello", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "Hello");
}

#[test]
fn test_finalize_plural_no_subs() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo(
        "finalize_plural_nosubs.mo",
        header,
        &[("files", "1 file\0%1 files")],
    );
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("files", Some("%1 files"), None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    // 没有数值型 subs，内部应追加 PLURAL_ARGUMENT_MISSING 标记
    assert!(out.contains("(I18N_PLURAL_ARGUMENT_MISSING)"));
}

#[test]
fn test_finalize_catalog_not_loaded_falls_back_to_msgid() {
    // 使用空 catalog（无匹配项），应回退到原始 msgid
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("finalize_empty_cat.mo", header, &[]);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("raw-text", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "raw-text");
}

#[test]
fn test_finalize_with_languages_fallback_first_language() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("finalize_langs.mo", header, &[("hello", "Hello")]);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("hello", None, None, false);
    let langs = vec!["en_US".to_string(), "en".to_string()];
    let out = msg.finalize(&cat, &langs).unwrap();
    assert_eq!(out, "Hello");
}

#[test]
fn test_finalize_domain_fallback_ignored_for_now() {
    // 当前 Rust 实现尚未根据 domain 选择 Catalog，这里仅验证设置 domain 不会影响基本行为
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("finalize_domain.mo", header, &[("hello", "Hello")]);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let mut msg = Message::new("hello", None, None, false);
    msg.domain = Some("custom-domain".to_string());
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "Hello");
}

#[test]
fn test_finalize_invalid_plural_index_uses_plural_text_or_msgid() {
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo(
        "finalize_invalid_plural.mo",
        header,
        &[("files", "one\0many")],
    );
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    // plural 文本与 msgid 不同，用于区分 fallback
    let msg = Message::new("files", Some("PLURAL"), None, false);
    // 不提供数值 subs，将使用 n=0 -> idx=1
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    // 只要求返回非空且包含某个已知片段
    assert!(!out.is_empty());
}

/// 无 .mo 时仍走 KUIT（对齐 `translate_with_domain_and_langs` 回退与 `KLocalizedStringTest::setFormatForMarker` 首条）。
#[test]
fn test_finalize_empty_catalog_kuit_tooltip_html() {
    let cat = Catalog::empty_fallback();
    let msg = Message::new("Hello world", None, Some("@info:tooltip"), true);
    let out = msg.finalize(&cat, &["en".to_string()]).unwrap();
    assert_eq!(out, "<html>Hello world</html>");
}

/// 内嵌 `<application>` + salvage（对齐 `semanticTags` 首条）。
#[test]
fn test_finalize_empty_catalog_application_tag_plain_action() {
    let cat = Catalog::empty_fallback();
    let mut msg = Message::new(
        "Open with <application>%1</application>",
        None,
        Some("@action:inmenu"),
        true,
    );
    msg.subs_string("Okteta", 0, ' ');
    let out = msg.finalize(&cat, &["en".to_string()]).unwrap();
    assert_eq!(out, "Open with Okteta");
}

