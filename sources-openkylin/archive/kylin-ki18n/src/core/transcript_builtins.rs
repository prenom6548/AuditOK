// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

//! 内置 Transcript 调用名称，供 transcript 与 js_engine 共享（Ts.hascall 需同时检查 builtin 与 JS 注册）

pub const BUILTIN_NAMES: &[&str] = &[
    "arg", "plural", "gender", "language", "country", "msgstrf", "msgid", "msgctxt", "msgkey",
    "nsubs", "subs", "dynctxt", "localeCountry", "contains", "startsWith", "endsWith",
    "upperFirst", "lowerFirst", "pluralForm", "argRange", "languageName", "capitalize",
    "upper", "lower", "trim", "sort", "reverse", "replace", "substring", "removeAccelerator",
    "script", "normKey", "getConfString", "getConfBool", "getConfNumber", "case",
    "loadProps", "getProp", "setProp",
];
