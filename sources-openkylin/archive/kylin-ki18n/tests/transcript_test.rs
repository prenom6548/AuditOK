// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use std::collections::HashMap;
use std::path::Path;

use ki18n_rs::catalog::Catalog;
use ki18n_rs::transcript::{apply_transcript, apply_transcript_with_fallback};

fn dummy_catalog() -> Catalog {
    let _state_guard = common::global_state_test_lock();
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("transcript_en.mo", header, &[("x", "x")]);
    Catalog::load_mo(Path::new(&path), Some("en")).expect("load mo")
}

#[test]
fn test_dynctxt_basic() {
    let mut ctx = HashMap::new();
    ctx.insert("name".to_string(), "Alice".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    let out = apply_transcript("Hello $[dynctxt name]", &ctx, &args, &cat, "en");
    assert_eq!(out, "Hello Alice");
}

#[test]
fn test_unknown_call_triggers_fallback_empty() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // 任意插值触发 fallback 时，scriptedTranslation 直接变为空串。
    let out = apply_transcript("$[missingKey]", &ctx, &args, &cat, "en");
    assert_eq!(out, "");
}

#[test]
fn test_malformed_unclosed_interpolation_requests_fallback() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // Missing closing ']' should trigger full scripted fallback.
    let (out, fb) = apply_transcript_with_fallback(
        "$[upper foo",
        &ctx,
        &args,
        &cat,
        "en",
    );
    assert_eq!(out, "");
    assert!(fb);
}

#[test]
fn test_free_segment_backslash_escape_space_and_bracket() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // `\ ` inside a free segment should drop the backslash and keep the space.
    assert_eq!(
        apply_transcript("$[upper foo\\ bar]", &ctx, &args, &cat, "en"),
        "FOO BAR"
    );

    // `\]` should keep ']' as part of the argument, while the trailing ']' still closes the interpolation.
    assert_eq!(
        apply_transcript("$[upper foo\\]]", &ctx, &args, &cat, "en"),
        "FOO]"
    );
}

#[test]
fn test_substring_boundary_cases() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[substring 'abcdef' 0 2]", &ctx, &args, &cat, "en"),
        "ab"
    );
    assert_eq!(
        apply_transcript("$[substring 'abcdef' 1 2]", &ctx, &args, &cat, "en"),
        "bc"
    );
    assert_eq!(
        apply_transcript("$[substring 'abcdef' 2 0]", &ctx, &args, &cat, "en"),
        ""
    );
    // start beyond end => empty
    assert_eq!(
        apply_transcript("$[substring 'abcdef' 10]", &ctx, &args, &cat, "en"),
        ""
    );
    // missing length => till end
    assert_eq!(
        apply_transcript("$[substring 'abcdef' 3]", &ctx, &args, &cat, "en"),
        "def"
    );
}

#[test]
fn test_arg_range_boundary_cases() {
    let ctx = HashMap::new();
    let args = vec![
        "A".to_string(),
        "B".to_string(),
        "C".to_string(),
        "D".to_string(),
    ];
    let cat = dummy_catalog();

    // default sep = " "
    assert_eq!(
        apply_transcript("$[argRange 1 3]", &ctx, &args, &cat, "en"),
        "A B C"
    );

    // single element range
    assert_eq!(
        apply_transcript("$[argRange 2 2]", &ctx, &args, &cat, "en"),
        "B"
    );

    // start > end => empty
    assert_eq!(
        apply_transcript("$[argRange 3 2]", &ctx, &args, &cat, "en"),
        ""
    );

    // third arg is quoted empty => builtin treats as " ,"
    assert_eq!(
        apply_transcript("$[argRange 1 2 '']", &ctx, &args, &cat, "en"),
        "A ,B"
    );
}

#[test]
fn test_remove_accelerator_shady_and_edge_cases() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // From upstream KLocalizedString::removeAcceleratorMarker tests.
    assert_eq!(
        apply_transcript("$[removeAccelerator '']", &ctx, &args, &cat, "en"),
        ""
    );
    assert_eq!(
        apply_transcript("$[removeAccelerator '&']", &ctx, &args, &cat, "en"),
        "&"
    );
    assert_eq!(
        apply_transcript("$[removeAccelerator 'Foo bar &']", &ctx, &args, &cat, "en"),
        "Foo bar &"
    );
    assert_eq!(
        apply_transcript("$[removeAccelerator 'Foo & Bar']", &ctx, &args, &cat, "en"),
        "Foo & Bar"
    );
}

#[test]
fn test_upper_first_lower_first_boundary_punctuation_and_unicode() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[upperFirst '...123 foo']", &ctx, &args, &cat, "en"),
        "...123 Foo"
    );
    assert_eq!(
        apply_transcript("$[upperFirst '...123 фу']", &ctx, &args, &cat, "en"),
        "...123 Фу"
    );
    assert_eq!(
        apply_transcript("$[lowerFirst '...123 FOO']", &ctx, &args, &cat, "en"),
        "...123 fOO"
    );

    // empty input => empty output
    assert_eq!(
        apply_transcript("$[upperFirst '']", &ctx, &args, &cat, "en"),
        ""
    );
}

#[test]
fn test_arg_and_upper_literal() {
    let ctx = HashMap::new();
    let args = vec!["hello".to_string()];
    let cat = dummy_catalog();

    let out = apply_transcript("$[upper $[arg 1]]", &ctx, &args, &cat, "en");
    assert_eq!(out, "HELLO");
}

#[test]
fn test_capitalize_sort_reverse_replace_substring() {
    let mut ctx = HashMap::new();
    ctx.insert("phrase".to_string(), "hello world".to_string());
    ctx.insert("list".to_string(), "b a c".to_string());
    ctx.insert("text".to_string(), "abcabc".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[capitalize $[dynctxt phrase]]", &ctx, &args, &cat, "en"),
        "Hello World"
    );
    assert_eq!(
        apply_transcript("$[sort $[dynctxt list]]", &ctx, &args, &cat, "en"),
        "a b c"
    );
    assert_eq!(
        apply_transcript("$[reverse $[dynctxt phrase]]", &ctx, &args, &cat, "en"),
        "dlrow olleh"
    );
    assert_eq!(
        apply_transcript("$[replace $[dynctxt text] ab xy]", &ctx, &args, &cat, "en"),
        "xycxyc"
    );
    assert_eq!(
        apply_transcript("$[substring $[dynctxt phrase] 6 5]", &ctx, &args, &cat, "en"),
        "world"
    );
}

#[test]
fn test_nested_contains_with_dynctxt() {
    let mut ctx = HashMap::new();
    ctx.insert("text".to_string(), "Hello 世界".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    let out = apply_transcript("$[contains $[dynctxt text] 世界]", &ctx, &args, &cat, "en");
    assert_eq!(out, "true");
}

#[test]
fn test_remove_accelerator() {
    let mut ctx = HashMap::new();
    ctx.insert("label".to_string(), "&File".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    let out = apply_transcript(
        "$[removeAccelerator $[dynctxt label]]",
        &ctx,
        &args,
        &cat,
        "en",
    );
    assert_eq!(out, "File");
}

#[test]
fn test_language_country_script() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[language]", &ctx, &args, &cat, "en_US"),
        "en_US"
    );
    assert_eq!(
        apply_transcript("$[country]", &ctx, &args, &cat, "en_US"),
        "US"
    );
    assert_eq!(
        apply_transcript("$[script]", &ctx, &args, &cat, "zh_TW"),
        "Hant"
    );
}

#[test]
fn test_plural_and_gender() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(apply_transcript("$[plural 1]", &ctx, &args, &cat, "en"), "0");
    assert_eq!(apply_transcript("$[plural 2]", &ctx, &args, &cat, "en"), "1");

    assert_eq!(
        apply_transcript("$[gender 1 he she they]", &ctx, &args, &cat, "en"),
        "he"
    );
    assert_eq!(
        apply_transcript("$[gender 2 he she they]", &ctx, &args, &cat, "en"),
        "she"
    );
}

#[test]
fn test_subs_and_nsubs() {
    let ctx = HashMap::new();
    let args = vec!["10".to_string(), "qwyx".to_string()];
    let cat = dummy_catalog();

    assert_eq!(apply_transcript("$[nsubs]", &ctx, &args, &cat, "en"), "2");
    assert_eq!(apply_transcript("$[subs 1]", &ctx, &args, &cat, "en"), "10");
    assert_eq!(apply_transcript("$[subs 2]", &ctx, &args, &cat, "en"), "qwyx");
    assert_eq!(apply_transcript("$[subs 3]", &ctx, &args, &cat, "en"), "");
}

#[test]
fn test_msgid_and_msgkey() {
    let mut ctx = HashMap::new();
    ctx.insert("msgid".to_string(), "source-text".to_string());
    ctx.insert("msgctxt".to_string(), "ctx".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[msgid]", &ctx, &args, &cat, "en"),
        "source-text"
    );
    assert_eq!(
        apply_transcript("$[msgkey]", &ctx, &args, &cat, "en"),
        "ctx|source-text"
    );
}

#[test]
fn test_value_reference_pow_1() {
    let ctx = HashMap::new();
    let args = vec!["Hello".to_string()];
    let cat = dummy_catalog();

    let out = apply_transcript("$[contains ^1 Hello]", &ctx, &args, &cat, "en");
    assert_eq!(out, "true");
}

#[test]
fn test_apply_transcript_executes_js_ts_call() {
    let mut ctx = HashMap::new();
    ctx.insert("msgid".to_string(), "source-text".to_string());
    ctx.insert("msgctxt".to_string(), "a-context".to_string());
    ctx.insert("msgstrf".to_string(), "translated-text".to_string());
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    let out = apply_transcript("$[test_basic foo]", &ctx, &args, &cat, "en");
    assert_eq!(out, "foo bar");
}

/// T9: `$[hascall name]` 检查的是 **name** 是否可调用（内置或 Ts.setcall），而非字面 `"hascall"`。
#[test]
fn test_hascall_interpolation_checks_target_name() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[hascall plural]", &ctx, &args, &cat, "en"),
        "true"
    );
    assert_eq!(
        apply_transcript("$[hascall not_a_real_builtin_xyz]", &ctx, &args, &cat, "en"),
        "false"
    );
}

// R06: normKey / getConf* builtins (pure Rust path)
#[test]
fn test_norm_key_builtin() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // Strip whitespace, accelerator (&), lowercase - matches ki18n normKeystr
    assert_eq!(
        apply_transcript("$[normKey '  foo  bar  ']", &ctx, &args, &cat, "en"),
        "foobar"
    );
    assert_eq!(
        apply_transcript("$[normKey 'Some &Thing']", &ctx, &args, &cat, "en"),
        "something"
    );
    assert_eq!(
        apply_transcript("$[normKey 'FOO']", &ctx, &args, &cat, "en"),
        "foo"
    );
}

#[test]
fn test_get_conf_builtins() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // ktranscript-test.ini has [fr] with StringKey, BoolKey, NumberKey
    assert_eq!(
        apply_transcript("$[getConfString StringKey]", &ctx, &args, &cat, "fr"),
        "StringValue"
    );
    assert_eq!(
        apply_transcript(
            "$[getConfString NoSuchKey DefaultValue]",
            &ctx,
            &args,
            &cat,
            "fr"
        ),
        "DefaultValue"
    );
    assert_eq!(
        apply_transcript("$[getConfBool BoolKey]", &ctx, &args, &cat, "fr"),
        "true"
    );
    assert_eq!(
        apply_transcript(
            "$[getConfBool NoSuchKey true]",
            &ctx,
            &args,
            &cat,
            "fr"
        ),
        "true"
    );
    assert_eq!(
        apply_transcript("$[getConfNumber NumberKey]", &ctx, &args, &cat, "fr"),
        "12345"
    );
    assert_eq!(
        apply_transcript(
            "$[getConfNumber NoSuchKey 54321]",
            &ctx,
            &args,
            &cat,
            "fr"
        ),
        "54321"
    );
}

// R02: grammatical case
#[test]
fn test_case_builtin_russian() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // 1=nom, 2=gen, 3=dat, 4=acc, 5=inst, 6=prep
    assert_eq!(
        apply_transcript("$[case 1 файл]", &ctx, &args, &cat, "ru"),
        "файл"
    );
    assert_eq!(
        apply_transcript("$[case 2 файл]", &ctx, &args, &cat, "ru"),
        "файла"
    );
    assert_eq!(
        apply_transcript("$[case 3 файл]", &ctx, &args, &cat, "ru"),
        "файлу"
    );
    assert_eq!(
        apply_transcript("$[case 2 день]", &ctx, &args, &cat, "ru"),
        "дня"
    );
    // Unsupported language: return noun as-is
    assert_eq!(
        apply_transcript("$[case 2 file]", &ctx, &args, &cat, "en"),
        "file"
    );
}

#[test]
fn test_case_builtin_polish() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    assert_eq!(
        apply_transcript("$[case 1 plik]", &ctx, &args, &cat, "pl"),
        "plik"
    );
    assert_eq!(
        apply_transcript("$[case 2 plik]", &ctx, &args, &cat, "pl"),
        "pliku"
    );
    assert_eq!(
        apply_transcript("$[case 2 Plik]", &ctx, &args, &cat, "pl"),
        "Pliku"
    );
}

// R01: loadProps / getProp / setProp
#[test]
fn test_load_props_get_prop_builtin() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // cities.pmap: Athens -> gen=Atine
    let _ = apply_transcript("$[loadProps cities]", &ctx, &args, &cat, "fr");
    assert_eq!(
        apply_transcript("$[getProp Athens gen]", &ctx, &args, &cat, "fr"),
        "Atine"
    );
}

#[test]
fn test_get_prop_with_relpath() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // 3-arg form: (relpath, phrase, prop) - loads on demand
    assert_eq!(
        apply_transcript("$[getProp cities Athens gen]", &ctx, &args, &cat, "fr"),
        "Atine"
    );
}

#[test]
fn test_set_prop_builtin() {
    let ctx = HashMap::new();
    let args: Vec<String> = vec![];
    let cat = dummy_catalog();

    // setProp then getProp
    apply_transcript("$[setProp Oslo dat Oslou]", &ctx, &args, &cat, "fr");
    assert_eq!(
        apply_transcript("$[getProp Oslo dat]", &ctx, &args, &cat, "fr"),
        "Oslou"
    );
}

