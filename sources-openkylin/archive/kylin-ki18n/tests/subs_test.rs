#![allow(non_snake_case)]
// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later


mod common;

use ki18n_rs::message::Message;
use proptest::prelude::*;

#[test]
fn test_subs_i64_basic() {
    let mut msg = Message::new("Count: %1", None, None, false);
    msg.subs_i64(42, 0, 10, ' ');
    let out = msg.apply_subs("Count: %1");
    assert_eq!(out, "Count: 42");
}

#[test]
fn test_subs_i64_field_width_and_fill() {
    let mut msg = Message::new("Value: %1", None, None, false);
    msg.subs_i64(7, 3, 10, '0');
    let out = msg.apply_subs("Value: %1");
    assert_eq!(out, "Value: 007");
}

#[test]
fn test_subs_i64_base_16() {
    let mut msg = Message::new("Hex: %1", None, None, false);
    msg.subs_i64(255, 0, 16, ' ');
    let out = msg.apply_subs("Hex: %1");
    assert_eq!(out, "Hex: ff");
}

#[test]
fn test_subs_f64_precision() {
    let mut msg = Message::new("Pi: %1", None, None, false);
    msg.subs_f64(3.14159, 0, 'f', 2, ' ');
    let out = msg.apply_subs("Pi: %1");
    assert!(out.starts_with("Pi: 3.14"));
}

#[test]
fn test_percent_zero_preserved() {
    let msg = Message::new("Use %0 to exit", None, None, false);
    let out = msg.apply_subs("Use %0 to exit");
    assert_eq!(out, "Use %0 to exit");
}

#[test]
fn test_percent_L0_preserved() {
    let msg = Message::new("Use %L0 here", None, None, false);
    let out = msg.apply_subs("Use %L0 here");
    assert_eq!(out, "Use %L0 here");
}

#[test]
fn test_percent_double_percent() {
    let msg = Message::new("100%% sure", None, None, false);
    // 当前实现不识别 "%%" 为转义，占位符解析失败时按字面保留
    let out = msg.apply_subs("100%% sure");
    assert_eq!(out, "100%% sure");
}

#[test]
fn test_percent_L_without_digit() {
    let msg = Message::new("value: %L", None, None, false);
    let out = msg.apply_subs("value: %L");
    assert_eq!(out, "value: %L");
}

#[test]
fn test_percent_only_L() {
    let msg = Message::new("%L", None, None, false);
    let out = msg.apply_subs("%L");
    assert_eq!(out, "%L");
}

#[test]
fn test_missing_argument_marks() {
    let mut msg = Message::new("A %1 B %2", None, None, false);
    msg.subs_i64(1, 0, 10, ' ');
    let out = msg.apply_subs("A %1 B %2");
    assert!(out.contains("(I18N_ARGUMENT_MISSING)"));
    assert!(out.contains("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
}

// 额外整数覆盖 ---------------------------------------------------------------

#[test]
fn test_subs_i64_negative_and_zero() {
    let mut msg = Message::new("%1,%2", None, None, false);
    msg.subs_i64(-5, 0, 10, ' ');
    msg.subs_i64(0, 0, 10, ' ');
    let out = msg.apply_subs("%1,%2");
    assert_eq!(out, "-5,0");
}

#[test]
fn test_subs_i64_max_value() {
    let mut msg = Message::new("%1", None, None, false);
    msg.subs_i64(i64::MAX, 0, 10, ' ');
    let out = msg.apply_subs("%1");
    assert_eq!(out, i64::MAX.to_string());
}

#[test]
fn test_subs_i64_min_value() {
    let mut msg = Message::new("%1", None, None, false);
    msg.subs_i64(i64::MIN, 0, 10, ' ');
    let out = msg.apply_subs("%1");
    assert_eq!(out, i64::MIN.to_string());
}

#[test]
fn test_subs_i32_and_i16_i8() {
    let mut msg = Message::new("%1 %2 %3", None, None, false);
    msg.subs_i32(10, 0, 10, ' ');
    msg.subs_i16(-3, 0, 10, ' ');
    msg.subs_i8(7, 0, 10, ' ');
    let out = msg.apply_subs("%1 %2 %3");
    assert_eq!(out, "10 -3 7");
}

#[test]
fn test_subs_u64_and_smaller() {
    let mut msg = Message::new("%1 %2 %3 %4", None, None, false);
    msg.subs_u64(42, 0, 10, ' ');
    msg.subs_u32(7, 0, 10, ' ');
    msg.subs_u16(1, 0, 10, ' ');
    msg.subs_u8(255, 0, 16, ' ');
    let out = msg.apply_subs("%1 %2 %3 %4");
    assert_eq!(out, "42 7 1 ff");
}

#[test]
fn test_subs_u64_field_width_and_fill() {
    let mut msg = Message::new("%1", None, None, false);
    msg.subs_u64(5, 4, 10, '0');
    let out = msg.apply_subs("%1");
    assert_eq!(out, "0005");
}

#[test]
fn test_subs_u64_zero_and_max() {
    let mut msg = Message::new("%1 %2", None, None, false);
    msg.subs_u64(0, 0, 10, ' ');
    msg.subs_u64(u64::MAX, 0, 10, ' ');
    let out = msg.apply_subs("%1 %2");
    assert!(out.starts_with("0 "));
}

// 浮点覆盖 --------------------------------------------------------------------

#[test]
fn test_subs_f64_negative_and_zero() {
    let mut msg = Message::new("%1 %2", None, None, false);
    msg.subs_f64(-1.5, 0, 'f', 1, ' ');
    msg.subs_f64(0.0, 0, 'f', 0, ' ');
    let out = msg.apply_subs("%1 %2");
    assert!(out.starts_with("-1.5 0"));
}

#[test]
fn test_subs_f32_basic() {
    let mut msg = Message::new("%1", None, None, false);
    msg.subs_f32(2.5f32, 0, 'f', 1, ' ');
    let out = msg.apply_subs("%1");
    assert!(out.starts_with("2.5"));
}

#[test]
fn test_subs_f64_infinity_and_nan() {
    let mut msg = Message::new("%1 %2 %3", None, None, false);
    msg.subs_f64(f64::INFINITY, 0, 'g', -1, ' ');
    msg.subs_f64(f64::NEG_INFINITY, 0, 'g', -1, ' ');
    msg.subs_f64(f64::NAN, 0, 'g', -1, ' ');
    let out = msg.apply_subs("%1 %2 %3");
    assert!(out.contains("inf") || out.contains("Inf"));
}

// 字符串与字符覆盖 -----------------------------------------------------------

#[test]
fn test_subs_string_basic_and_empty() {
    let mut msg = Message::new("%1-%2", None, None, false);
    msg.subs_string("hello", 0, ' ');
    msg.subs_string("", 0, ' ');
    let out = msg.apply_subs("%1-%2");
    assert_eq!(out, "hello-");
}

#[test]
fn test_subs_string_unicode_and_newline() {
    let mut msg = Message::new("%1|%2", None, None, false);
    msg.subs_string("你好", 0, ' ');
    msg.subs_string("line1\nline2", 0, ' ');
    let out = msg.apply_subs("%1|%2");
    assert!(out.contains("你好"));
    assert!(out.contains("line1\nline2"));
}

#[test]
fn test_subs_string_very_long_and_tabs() {
    let mut long = String::new();
    for _ in 0..1000 {
        long.push('x');
    }
    let mut msg = Message::new("%1|%2", None, None, false);
    msg.subs_string(&long, 0, ' ');
    msg.subs_string("a\tb\tc", 0, ' ');
    let out = msg.apply_subs("%1|%2");
    assert!(out.starts_with(&long));
    assert!(out.contains("a\tb\tc"));
}

#[test]
fn test_subs_char_basic_and_unicode() {
    let mut msg = Message::new("%1%2%3", None, None, false);
    msg.subs_char('A', 0, ' ');
    msg.subs_char('字', 0, ' ');
    msg.subs_char('1', 0, ' ');
    let out = msg.apply_subs("%1%2%3");
    assert_eq!(out, "A字1");
}

// 嵌套 Message 覆盖 ----------------------------------------------------------

#[test]
fn test_subs_message_basic() {
    let inner = {
        let mut m = Message::new("inner %1", None, None, false);
        m.subs_string("X", 0, ' ');
        m
    };
    let mut outer = Message::new("outer [%1]", None, None, false);
    outer.subs_message(inner, 0, ' ');

    // 使用一个 dummy catalog，使 finalize 能走完整流水线
    let header = "nplurals=2; plural=(n != 1);";
    let path = common::write_test_mo("subs_message.mo", header, &[("outer [%1]", "outer [%1]"), ("inner %1", "inner %1")]);
    let cat = ki18n_rs::catalog::Catalog::load_mo(std::path::Path::new(&path), Some("en"))
        .expect("load mo");
    let out = outer.finalize(&cat, &vec!["en".to_string()]).unwrap();
    assert_eq!(out, "outer [inner X]");
}

#[test]
fn test_subs_message_empty_inner_and_cjk() {
    let inner = Message::new("", None, None, false);
    let mut outer = Message::new("外部[%1]", None, None, false);
    outer.subs_message(inner, 0, ' ');

    let header = "nplurals=1; plural=0;";
    let path = common::write_test_mo("subs_message_cjk.mo", header, &[("外部[%1]", "外部[%1]")]);
    let cat = ki18n_rs::catalog::Catalog::load_mo(std::path::Path::new(&path), Some("zh"))
        .expect("load mo");
    let out = outer.finalize(&cat, &vec!["zh".to_string()]).unwrap();
    assert!(out.starts_with("外部["));
}

// 占位符边界覆盖 --------------------------------------------------------------

#[test]
fn test_placeholder_double_and_triple_digit() {
    let mut msg = Message::new("%1 %10 %100", None, None, false);
    // 只给前两个参数，第三个缺失
    msg.subs_i64(1, 0, 10, ' ');
    msg.subs_i64(10, 0, 10, ' ');
    let out = msg.apply_subs("%1 %10 %100");
    assert!(out.contains("1"));
    assert!(out.contains("10"));
    assert!(out.contains("%100"));
    assert!(out.contains("(I18N_ARGUMENT_MISSING)"));
}

#[test]
fn test_placeholder_localized_index() {
    let mut msg = Message::new("%L1", None, None, false);
    msg.subs_i64(5, 0, 10, ' ');
    let out = msg.apply_subs("%L1");
    assert_eq!(out, "5");
}

#[test]
fn test_duplicate_and_reverse_indices() {
    let mut msg = Message::new("%2 %1 %2", None, None, false);
    msg.subs_string("one", 0, ' ');
    msg.subs_string("two", 0, ' ');
    let out = msg.apply_subs("%2 %1 %2");
    assert_eq!(out, "two one two");
}

#[test]
fn test_placeholder_sequence_reverse_and_gaps() {
    let mut msg = Message::new("%3 %1", None, None, false);
    msg.subs_string("one", 0, ' ');
    msg.subs_string("two", 0, ' ');
    msg.subs_string("three", 0, ' ');
    let out = msg.apply_subs("%3 %1");
    assert!(out.contains("three one"));
    // 由于 2 未被使用，应出现 GAP 标记
    assert!(out.contains("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
}

#[test]
fn test_placeholder_no_gaps_when_all_used() {
    let mut msg = Message::new("%1 %2 %3", None, None, false);
    msg.subs_string("a", 0, ' ');
    msg.subs_string("b", 0, ' ');
    msg.subs_string("c", 0, ' ');
    let out = msg.apply_subs("%1 %2 %3");
    assert_eq!(out, "a b c");
    assert!(!out.contains("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
}

#[test]
fn test_placeholder_duplicate_allowed_without_gap() {
    let mut msg = Message::new("%1 %1 %2", None, None, false);
    msg.subs_string("x", 0, ' ');
    msg.subs_string("y", 0, ' ');
    let out = msg.apply_subs("%1 %1 %2");
    assert_eq!(out, "x x y");
    // 1 和 2 都被使用，不应标记 GAP
    assert!(!out.contains("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
}

/// 对齐 `KLocalizedStringTest::correctSubs`：singular 无 %1 时第一个数值 subs 仍占复数位，不触发 GAPS。
#[test]
fn test_plural_first_numeric_not_in_singular_template_no_gap() {
    let mut msg = Message::new("A pod left on %2", Some("%1 pods left on %2"), None, false);
    msg.subs_i64(1, 0, 10, ' ');
    msg.subs_string("Discovery", 0, ' ');
    let out = msg.apply_subs("A pod left on %2");
    assert_eq!(out, "A pod left on Discovery");
    assert!(!out.contains("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
}

#[test]
fn test_all_placeholders_missing_markers() {
    let msg = Message::new("A %1 B %2 C %3", None, None, false);
    let out = msg.apply_subs("A %1 B %2 C %3");
    assert!(out.contains("(I18N_ARGUMENT_MISSING)"));
}

proptest! {
    // property: 对于十进制整型，subs_i64 应等价于 format!("{}", v)
    #[test]
    fn prop_subs_i64_matches_format(v in -1_000_000i64..1_000_000i64) {
        let mut msg = Message::new("%1", None, None, false);
        msg.subs_i64(v, 0, 10, ' ');
        let out = msg.apply_subs("%1");
        prop_assert_eq!(out, format!("{}", v));
    }
}

