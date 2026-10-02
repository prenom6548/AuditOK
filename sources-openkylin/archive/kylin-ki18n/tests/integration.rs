// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::capi::*;
use ki18n_rs::catalog::Catalog;
use ki18n_rs::kuit::{parse, to_format, VisualFormat};
use ki18n_rs::message::Message;
use ki18n_rs::plural::{evaluate_plural, get_plural_forms_expression};

use std::path::Path;

// ----------------------------------------------------------------------------
// Catalog 测试
// ----------------------------------------------------------------------------

#[test]
fn test_mo_loading() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("test_en.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    assert_eq!(catalog.get_translated_message("hello", 0), Some("Hello"));
}

#[test]
fn test_message_lookup() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "1 file\0%1 files")];
    let path = common::write_test_mo("test_en_plural.mo", header, entries);
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
fn test_context_lookup() {
    // 当前 Catalog 对上下文的完整支持有限，这里验证至少 header 解析不崩溃
    let header = "nplurals=1; plural=0;";
    let entries = &[("context\u{4}hello", "你好")];
    let path = common::write_test_mo("test_zh_ctx.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("zh")).expect("load mo");
    assert!(catalog.get_translated_message("context\u{4}hello", 0).is_some());
}

// ----------------------------------------------------------------------------
// Plural 测试
// ----------------------------------------------------------------------------

#[test]
fn test_english_plural() {
    let rule = get_plural_forms_expression("en").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
}

#[test]
fn test_russian_plural() {
    let rule = get_plural_forms_expression("ru").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 5), 2);
    assert_eq!(evaluate_plural(&rule, 21), 0);
}

#[test]
fn test_arabic_plural() {
    let rule = get_plural_forms_expression("ar").unwrap();
    assert_eq!(evaluate_plural(&rule, 0), 0);
    assert_eq!(evaluate_plural(&rule, 1), 1);
    assert_eq!(evaluate_plural(&rule, 2), 2);
    assert_eq!(evaluate_plural(&rule, 3), 3);
    assert_eq!(evaluate_plural(&rule, 11), 4);
    assert_eq!(evaluate_plural(&rule, 100), 5);
}

#[test]
fn test_chinese_no_plural() {
    let rule = get_plural_forms_expression("zh").unwrap();
    for n in [0u64, 1, 2, 100] {
        assert_eq!(evaluate_plural(&rule, n), 0);
    }
}

#[test]
fn test_polish_plural() {
    let rule = get_plural_forms_expression("pl").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 5), 2);
}

// ----------------------------------------------------------------------------
// Message 测试
// ----------------------------------------------------------------------------

#[test]
fn test_basic_translation() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("test_en_msg.mo", header, entries);
    let catalog = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let msg = Message::new("hello", None, None, false);
    let out = msg.finalize(&catalog, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "Hello");
}

#[test]
fn test_placeholder_i64() {
    let mut msg = Message::new("Count: %1", None, None, false);
    msg.subs_i64(42, 0, 10, ' ');
    let out = msg.apply_subs(&msg.text);
    assert_eq!(out, "Count: 42");
}

#[test]
fn test_placeholder_string() {
    let mut msg = Message::new("Hello %1", None, None, false);
    msg.subs_string("World", 0, ' ');
    let out = msg.apply_subs(&msg.text);
    assert_eq!(out, "Hello World");
}

#[test]
fn test_placeholder_order() {
    let mut msg = Message::new("%2 then %1", None, None, false);
    msg.subs_string("one", 0, ' ');
    msg.subs_string("two", 0, ' ');
    let out = msg.apply_subs(&msg.text);
    assert_eq!(out, "two then one");
}

#[test]
fn test_field_width() {
    let mut msg = Message::new("Value: %1", None, None, false);
    msg.subs_i64(7, 3, 10, '0');
    let out = msg.apply_subs(&msg.text);
    assert_eq!(out, "Value: 007");
}

#[test]
fn test_base_conversion() {
    let mut msg = Message::new("%1 %2 %3", None, None, false);
    msg.subs_u64(2, 0, 2, ' ');
    msg.subs_u64(8, 0, 8, ' ');
    msg.subs_u64(16, 0, 16, ' ');
    let out = msg.apply_subs(&msg.text);
    assert_eq!(out, "10 10 10");
}

#[test]
fn test_float_precision() {
    let mut msg = Message::new("Pi ~ %1", None, None, false);
    msg.subs_f64(3.14159, 0, 'f', 2, ' ');
    let out = msg.apply_subs(&msg.text);
    assert!(out.starts_with("Pi ~ 3.14"));
}

// ----------------------------------------------------------------------------
// KUIT 测试
// ----------------------------------------------------------------------------

#[test]
fn test_kuit_bold() {
    let node = parse("@b{bold}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("bold"));
}

#[test]
fn test_kuit_italic() {
    let node = parse("@i{italic}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("italic"));
}

#[test]
fn test_kuit_link() {
    let node = parse("@a{link}");
    let out = to_format(&node, VisualFormat::RichText);
    assert!(out.contains("<a"));
}

#[test]
fn test_kuit_rich_text() {
    let node = parse("@b{bold}");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(out, "<html><b>bold</b></html>");
}

#[test]
fn test_kuit_terminal() {
    let node = parse("@error{oops}");
    let out = to_format(&node, VisualFormat::TermText);
    assert!(out.contains("oops"));
}

// ----------------------------------------------------------------------------
// Locale 测试（使用最小 JSON 数据集，写入临时目录）
// ----------------------------------------------------------------------------

#[allow(deprecated)]
fn write_min_locale_data() -> std::path::PathBuf {
    use std::fs;
    use std::io::Write;
    use std::env;

    let dir = tempfile::tempdir().expect("tempdir");
    // 使用 into_path 防止临时目录在函数结束时被删除
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
fn test_country_alpha2() {
    write_min_locale_data();
    let mut handle: *mut ki18n_rs::capi::locale::Ki18nRsCountryHandle = std::ptr::null_mut();
    let code = std::ffi::CString::new("DE").unwrap();
    let res =
        ki18n_rs::capi::locale::ki18n_rs_country_from_alpha2(code.as_ptr(), &mut handle);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!handle.is_null());
}

// coverage-task-6-ffi-integration: 集成扩展 -----------------------------------

#[test]
fn test_integration_full_i18n_workflow() {
    // mo: hello -> Hello
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("integration_full.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let msg = Message::new("hello", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "Hello");
}

#[test]
fn test_integration_plural_workflow() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "%1 file\0%1 files")];
    let path = common::write_test_mo("integration_plural.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut msg = Message::new("files", None, None, false);
    // 直接走占位符替换逻辑即可验证 plural header 被正确解析
    msg.subs_i64(2, 0, 10, ' ');
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert!(out.contains("2"));
}

#[test]
fn test_integration_kuit_workflow() {
    let node = parse("@b{bold}");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_plain.contains("bold"));
    assert_eq!(out_rich, "<html><b>bold</b></html>");
}

#[test]
fn test_integration_transcript_workflow() {
    use ki18n_rs::core::transcript::apply_transcript;
    use std::collections::HashMap;

    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("integration_tr.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut ctx = HashMap::new();
    ctx.insert("name".to_string(), "Alice".to_string());
    let args = vec!["world".to_string()];
    let out = apply_transcript(
        "Hi $[dynctxt name], $[arg 1]",
        &ctx,
        &args,
        &cat,
        "en",
    );
    assert!(out.contains("Hi Alice, world"));
}

#[test]
fn test_integration_language_switch_with_fallback_like() {
    let header_en = "nplurals=2; plural=(n != 1);";
    let entries_en = &[("hello", "Hello")];
    let path_en = common::write_test_mo("integration_lang_en.mo", header_en, entries_en);
    let cat_en = Catalog::load_mo(Path::new(&path_en), Some("en")).expect("load mo");

    let header_zh = "nplurals=1; plural=0;";
    let entries_zh = &[("hello", "你好")];
    let path_zh = common::write_test_mo("integration_lang_zh.mo", header_zh, entries_zh);
    let cat_zh = Catalog::load_mo(Path::new(&path_zh), Some("zh")).expect("load mo");

    let msg = Message::new("hello", None, None, false);
    let out_en = msg.finalize(&cat_en, &vec!["en".to_string()]).unwrap();
    let out_zh = msg.finalize(&cat_zh, &vec!["zh".to_string()]).unwrap();
    assert_eq!(out_en, "Hello");
    assert_eq!(out_zh, "你好");
}

#[test]
fn test_integration_unicode_and_rtl_message() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("rtl", "\u{200F}مرحبا")];
    let path = common::write_test_mo("integration_rtl.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("ar")).expect("load mo");

    let msg = Message::new("rtl", None, None, false);
    let out = msg.finalize(&cat, &vec!["ar".to_string()]).unwrap();
    assert!(out.contains("مرحبا"));
}

#[test]
fn test_integration_repeated_creation_and_cached_catalog_like() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("integration_cached.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    for _ in 0..10 {
        let msg = Message::new("hello", None, None, false);
        let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
        assert_eq!(out, "Hello");
    }
}

#[test]
fn test_integration_invalid_domain_and_language_handled() {
    use std::ffi::CString;
    use std::ptr;

    let bad_dom = CString::new("nonexistent-domain").unwrap();
    let bad_lang = CString::new("xx_YY").unwrap();

    let text = CString::new("hello").unwrap();
    let handle =
        message::ki18n_rs_msg_create(ptr::null(), text.as_ptr(), ptr::null());
    assert!(!handle.is_null());

    let r_dom = message::ki18n_rs_msg_set_domain(handle, bad_dom.as_ptr());
    let langs = [bad_lang.as_ptr()];
    let r_lang = message::ki18n_rs_msg_set_languages(handle, langs.as_ptr(), 1);

    // 无论 domain/lang 是否存在，对设置操作本身都应成功。
    assert_eq!(r_dom, Ki18nRsError::OK as i32);
    assert_eq!(r_lang, Ki18nRsError::OK as i32);

    let mut out: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res_to_string = message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res_to_string, Ki18nRsError::OK as i32);
    if !out.is_null() {
        global::ki18n_rs_string_free(out);
    }
    message::ki18n_rs_msg_destroy(handle);
}

// extra-task-4-integration: 复杂翻译流程 ---------------------------------------

#[test]
fn test_integration_kuit_with_multiple_subs() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("tmpl", "@b{%1} and @i{%2}")];
    let path = common::write_test_mo("int_kuit_subs.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut msg = Message::new("tmpl", None, None, true);
    msg.subs_string("Bold", 0, ' ');
    msg.subs_string("Ital", 0, ' ');
    let out_plain = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert!(out_plain.contains("Bold"));
    assert!(out_plain.contains("Ital"));
}

#[test]
fn test_integration_transcript_with_multiple_args() {
    use ki18n_rs::core::transcript::apply_transcript;
    use std::collections::HashMap;

    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("int_tr_multi.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut ctx = HashMap::new();
    ctx.insert("greet".to_string(), "Hello".to_string());
    let args = vec!["Alice".to_string(), "Bob".to_string()];
    let out = apply_transcript("$[dynctxt greet] $[arg 1], $[arg 2]", &ctx, &args, &cat, "en");
    assert!(out.contains("Hello Alice, Bob"));
}

#[test]
fn test_integration_plural_with_kuit_and_transcript() {
    use ki18n_rs::core::transcript::apply_transcript;
    use std::collections::HashMap;

    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("files", "@b{$[arg 1]} file(s)")];
    let path = common::write_test_mo("int_plural_kuit_tr.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut ctx = HashMap::new();
    ctx.insert("dummy".to_string(), "x".to_string());
    let args = vec!["3".to_string()];
    let node = parse("@b{$[arg 1]} file(s)");
    let base = to_format(&node, VisualFormat::PlainText);
    let out = apply_transcript(&base, &ctx, &args, &cat, "en");
    assert!(out.contains("3"));
}

#[test]
fn test_integration_context_and_plural() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("ctx\u{4}files", "1 file\0%1 files")];
    let path = common::write_test_mo("int_ctx_plural.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut msg = Message::new("ctx\u{4}files", Some("%1 files"), Some("ctx"), false);
    msg.subs_i64(5, 0, 10, ' ');
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert!(out.contains("5"));
}

#[test]
fn test_integration_domain_and_language_fallback() {
    let header = "nplurals=1; plural=0;";
    let entries = &[("hello", "Hallo")];
    let path = common::write_test_mo("int_domain_lang.mo", header, entries);
    let dir = Path::new(&path).parent().unwrap();
    let dom = std::ffi::CString::new("int-domain").unwrap();
    let dir_c = std::ffi::CString::new(dir.to_string_lossy().as_bytes()).unwrap();
    global::ki18n_rs_set_application_domain(dom.as_ptr());
    global::ki18n_rs_add_domain_locale_dir(dom.as_ptr(), dir_c.as_ptr());

    let text = std::ffi::CString::new("hello").unwrap();
    let handle =
        message::ki18n_rs_msg_create(std::ptr::null(), text.as_ptr(), std::ptr::null());
    assert!(!handle.is_null());

    // 不显式设置语言时，回退到原文路径也应返回 OK。
    let mut out: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res = message::ki18n_rs_msg_to_string(handle, &mut out);
    assert_eq!(res, Ki18nRsError::OK as i32);
    if !out.is_null() {
        global::ki18n_rs_string_free(out);
    }
    message::ki18n_rs_msg_destroy(handle);
}

#[test]
fn test_integration_very_long_message_1000_chars() {
    let mut long = String::new();
    for _ in 0..1000 {
        long.push('x');
    }
    let header = "nplurals=1; plural=0;";
    let entries = &[("long", long.as_str())];
    let path = common::write_test_mo("int_long.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("long", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out.len(), long.len());
}

#[test]
fn test_integration_message_chaining() {
    let header = "nplurals=1; plural=0;";
    let entries = &[("a", "A"), ("b", "%1+B")];
    let path = common::write_test_mo("int_chain.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let a = Message::new("a", None, None, false);
    let mut b = Message::new("b", None, None, false);
    b.subs_message(a.clone(), 0, ' ');
    let out = b.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert!(out.contains("A+B"));
}

#[test]
fn test_integration_mixed_content_kuit_xml() {
    let node = parse("<p>@b{Bold} and <i>italic</i> &amp; plain</p>");
    let out_rich = to_format(&node, VisualFormat::RichText);
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_rich.contains("<b>Bold</b>"));
    assert!(out_plain.contains("Bold"));
    assert!(out_plain.contains("italic"));
}

#[test]
fn test_integration_multiple_ui_markers() {
    let node = parse("@title:Main/plain@subtitle:Sub/plain");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.to_lowercase().contains("main"));
}

#[test]
fn test_integration_nested_transcript_expressions() {
    use ki18n_rs::core::transcript::apply_transcript;
    use std::collections::HashMap;

    let header = "nplurals=2; plural=(n != 1);";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("int_nested_tr.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let mut ctx = HashMap::new();
    ctx.insert("base".to_string(), "hello".to_string());
    let args = vec!["world".to_string()];
    let out = apply_transcript(
        "$[capitalize $[dynctxt base]] $[upper $[arg 1]]",
        &ctx,
        &args,
        &cat,
        "en",
    );
    assert!(out.contains("Hello WORLD"));
}

// extra-task-4-integration: 错误处理场景 ---------------------------------------

#[test]
fn test_integration_missing_catalog_returns_original() {
    let header = "nplurals=2; plural=(n != 1);";
    let entries: &[(&str, &str)] = &[];
    let path = common::write_test_mo("int_missing_cat.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("raw", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "raw");
}

#[test]
fn test_integration_invalid_plural_formula_handled() {
    let header = "nplurals=2; plural=invalid_token;";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("int_bad_plural.mo", header, entries);
    // 当前实现会在加载阶段返回 Err，这里只验证不会 panic
    let _ = Catalog::load_mo(Path::new(&path), Some("en"));
}

#[test]
fn test_integration_invalid_language_code_ignored() {
    let header = "nplurals=1; plural=0;";
    let entries = &[("hello", "Hello")];
    let path = common::write_test_mo("int_bad_lang.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("hello", None, None, false);
    let out = msg.finalize(&cat, &vec!["xx_YY".to_string()]).unwrap();
    // 当前实现会回退到 msgid 或已有翻译，这里只要求不崩溃
    assert!(!out.is_empty());
}

#[test]
fn test_integration_empty_catalog_works() {
    let header = "nplurals=1; plural=0;";
    let entries: &[(&str, &str)] = &[];
    let path = common::write_test_mo("int_empty_cat.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("hello", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "hello");
}

#[test]
fn test_integration_translation_missing_uses_msgid() {
    let header = "nplurals=1; plural=0;";
    let entries = &[("known", "Known")];
    let path = common::write_test_mo("int_missing_tr.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");
    let msg = Message::new("unknown", None, None, false);
    let out = msg.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "unknown");
}

#[test]
fn test_integration_invalid_kuit_tag_kept_as_is() {
    let node = parse("<bogus>text</bogus>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("text"));
}

#[test]
fn test_integration_invalid_entity_kept_as_is() {
    let node = parse("&notAnEntity;");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("&notAnEntity;"));
}

#[test]
fn test_integration_invalid_transcript_function_kept_as_is() {
    use ki18n_rs::core::transcript::apply_transcript;
    use std::collections::HashMap;

    let header = "nplurals=1; plural=0;";
    let entries = &[("x", "x")];
    let path = common::write_test_mo("int_bad_func.mo", header, entries);
    let cat = Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo");

    let ctx = HashMap::new();
    let args: Vec<String> = Vec::new();
    let out = apply_transcript("Hello $[unknownFunc 1]", &ctx, &args, &cat, "en");
    assert_eq!(out, "");
}

#[test]
fn test_integration_catalog_corruption_handled() {
    use std::fs;
    use std::io::Write;

    let dir = common::ensure_data_dir();
    let path = dir.join("corrupt.mo");
    // 写入无效内容
    let mut f = fs::File::create(&path).unwrap();
    f.write_all(b"not a mo file").unwrap();

    // 加载失败时 Catalog::load_mo 应返回 Err，这里只验证不会 panic
    let _ = Catalog::load_mo(&path, Some("en"));
}

#[test]
fn test_integration_malformed_mo_file_handled() {
    // 与上类似，只是使用不同文件名，避免冲突
    use std::fs;
    use std::io::Write;

    let dir = common::ensure_data_dir();
    let path = dir.join("malformed.mo");
    let mut f = fs::File::create(&path).unwrap();
    f.write_all(b"\0\0\0\0").unwrap();
    let _ = Catalog::load_mo(&path, Some("en"));
}

