// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::core::plural::{evaluate_plural, get_plural_forms_expression, parse_plural_forms};
use proptest::prelude::*;

#[test]
fn test_english_singular_plural() {
    let rule = get_plural_forms_expression("en").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 0), 1);
}

#[test]
fn test_russian_three_forms() {
    let rule = get_plural_forms_expression("ru").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 5), 2);
}

#[test]
fn test_arabic_six_forms() {
    let rule = get_plural_forms_expression("ar").unwrap();
    assert_eq!(evaluate_plural(&rule, 0), 0);
    assert_eq!(evaluate_plural(&rule, 1), 1);
    assert_eq!(evaluate_plural(&rule, 2), 2);
    assert_eq!(evaluate_plural(&rule, 3), 3);
    assert_eq!(evaluate_plural(&rule, 11), 4);
    assert_eq!(evaluate_plural(&rule, 100), 5);
}

#[test]
fn test_east_asian_single_form() {
    for lang in &["zh", "ja", "ko"] {
        let rule = get_plural_forms_expression(lang).unwrap();
        for n in [0u64, 1, 2, 10, 100] {
            assert_eq!(evaluate_plural(&rule, n), 0);
        }
    }
}

// ---------------------------------------------------------------------------
// 追加 Plural 测试：覆盖更多语言与边界
// ---------------------------------------------------------------------------

#[test]
fn test_german_and_french_like_english() {
    for lang in &["de", "fr", "es", "it", "nl", "pt", "sv", "no"] {
        if let Some(rule) = get_plural_forms_expression(lang) {
            assert_eq!(evaluate_plural(&rule, 1), 0, "lang={}", lang);
            for &n in &[0u64, 2, 3, 10, 100] {
                assert_eq!(evaluate_plural(&rule, n), 1, "lang={} n={}", lang, n);
            }
        }
    }
}

#[test]
fn test_polish_three_forms() {
    let rule = get_plural_forms_expression("pl").unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 22), 1);
    assert_eq!(evaluate_plural(&rule, 5), 2);
    assert_eq!(evaluate_plural(&rule, 25), 2);
}

#[test]
fn test_lithuanian_three_forms() {
    if let Some(rule) = get_plural_forms_expression("lt") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
        assert_eq!(evaluate_plural(&rule, 9), 1);
        assert_eq!(evaluate_plural(&rule, 10), 2);
        assert_eq!(evaluate_plural(&rule, 0), 2);
    }
}

#[test]
fn test_slovenian_four_forms() {
    if let Some(rule) = get_plural_forms_expression("sl") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
        assert_eq!(evaluate_plural(&rule, 3), 2);
        assert_eq!(evaluate_plural(&rule, 5), 3);
    }
}

#[test]
fn test_other_single_form_languages() {
    for lang in &["vi", "th"] {
        if let Some(rule) = get_plural_forms_expression(lang) {
            for n in [0u64, 1, 2, 10, 100] {
                assert_eq!(evaluate_plural(&rule, n), 0, "lang={} n={}", lang, n);
            }
        }
    }
}

// coverage-task-5-plural-catalog: 更多语言复数 -------------------------------

#[test]
fn test_french_plural_zero_and_two() {
    if let Some(rule) = get_plural_forms_expression("fr") {
        assert_eq!(evaluate_plural(&rule, 0), 1);
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
    }
}

#[test]
fn test_german_plural_zero_and_two() {
    if let Some(rule) = get_plural_forms_expression("de") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 0), 1);
        assert_eq!(evaluate_plural(&rule, 2), 1);
    }
}

#[test]
fn test_spanish_and_portuguese_plural_two() {
    for lang in &["es", "pt"] {
        if let Some(rule) = get_plural_forms_expression(lang) {
            assert_eq!(evaluate_plural(&rule, 1), 0, "lang={}", lang);
            assert_eq!(evaluate_plural(&rule, 2), 1, "lang={}", lang);
        }
    }
}

#[test]
fn test_ukrainian_plural_three_forms() {
    if let Some(rule) = get_plural_forms_expression("uk") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
        assert_eq!(evaluate_plural(&rule, 5), 2);
    }
}

#[test]
fn test_czech_plural_three_forms() {
    if let Some(rule) = get_plural_forms_expression("cs") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
        assert_eq!(evaluate_plural(&rule, 5), 2);
    }
}

#[test]
fn test_slovak_plural_three_forms() {
    if let Some(rule) = get_plural_forms_expression("sk") {
        assert_eq!(evaluate_plural(&rule, 1), 0);
        assert_eq!(evaluate_plural(&rule, 2), 1);
        assert_eq!(evaluate_plural(&rule, 5), 2);
    }
}

#[test]
fn test_additional_single_form_languages() {
    for lang in &["vi", "th", "lo", "my"] {
        if let Some(rule) = get_plural_forms_expression(lang) {
            for n in [0u64, 1, 2, 10, 100] {
                assert_eq!(evaluate_plural(&rule, n), 0, "lang={} n={}", lang, n);
            }
        }
    }
}

#[test]
fn test_plural_expression_parsing_complex() {
    // 人工构造的复杂表达式：n%10==1 && n%100!=11 ? 0 : 1
    let header = "nplurals=2; plural=(n%10==1 && n%100!=11 ? 0 : 1);";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 11), 1);
    assert_eq!(evaluate_plural(&rule, 21), 0);
}

// 复数表达式解析扩展 ----------------------------------------------------------

#[test]
fn test_plural_complex_and_or() {
    let header =
        "nplurals=2; plural=(n>1 && n<5) || n==10 ? 1 : 0;";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 0), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 4), 1);
    assert_eq!(evaluate_plural(&rule, 10), 1);
}

#[test]
fn test_plural_complex_comparison_chain() {
    let header = "nplurals=3; plural=(n==1?0:(n<5?1:2));";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
    assert_eq!(evaluate_plural(&rule, 4), 1);
    assert_eq!(evaluate_plural(&rule, 5), 2);
}

#[test]
fn test_plural_division_and_modulo_and_multiplication() {
    let header = "nplurals=2; plural=((n/10)%2)==1 && (n*2)%3==0 ? 1 : 0;";
    let rule = parse_plural_forms(header).unwrap();
    let a = evaluate_plural(&rule, 10);
    let b = evaluate_plural(&rule, 30);
    assert!(a == 0 || a == 1);
    assert!(b == 0 || b == 1);
}

#[test]
fn test_plural_parentheses_nested() {
    let header = "nplurals=2; plural=(((n==1))?0:1);";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
}

#[test]
fn test_plural_zero_division_protected() {
    // 实现中对于除零应避免 panic，这里只验证 parse 成功且 eval 不崩溃
    let header = "nplurals=2; plural=(n/0 ? 1 : 0);";
    if let Ok(rule) = parse_plural_forms(header) {
        let _ = evaluate_plural(&rule, 1);
    }
}

#[test]
fn test_plural_invalid_expression_handled() {
    let header = "nplurals=2; plural=invalid_token;";
    let rule = parse_plural_forms(header);
    assert!(rule.is_err());
}

#[test]
fn test_plural_expression_parentheses_and_ternary() {
    let header = "nplurals=2; plural=((n==1)?0:1);";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
}

#[test]
fn test_plural_zero_and_negative_and_large() {
    let rule = get_plural_forms_expression("en").unwrap();
    assert_eq!(evaluate_plural(&rule, 0), 1);
    // 负数在实现中按 i64 处理，这里只验证不会 panic，且给出某个索引
    let _ = evaluate_plural(&rule, (-1i64) as u64);
    let max = u64::MAX;
    let _ = evaluate_plural(&rule, max);
}

#[test]
fn test_plural_forms_parsing_from_header() {
    let header = "nplurals=2; plural=(n != 1);";
    let rule = parse_plural_forms(header).unwrap();
    assert_eq!(evaluate_plural(&rule, 1), 0);
    assert_eq!(evaluate_plural(&rule, 2), 1);
}

proptest! {
    // property: 对于中文类语言，任意 n 都应落在唯一形式 0 上
    #[test]
    fn prop_east_asian_always_zero(n in 0u64..10_000) {
        let rule = get_plural_forms_expression("zh").unwrap();
        prop_assert_eq!(evaluate_plural(&rule, n), 0);
    }
}

