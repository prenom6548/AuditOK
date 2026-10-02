// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::os::raw::c_int;

/// 错误码枚举（必须与 C++ 文档一致）
#[repr(i32)]
#[derive(Debug, Clone, Copy)]
#[allow(non_camel_case_types)]
pub enum Ki18nRsError {
    OK = 0,
    GENERAL = 1,
    INVALID_ARGUMENT = 2,
    OUT_OF_MEMORY = 3,
    NOT_FOUND = 4,
    INTERNAL = 5,
}

impl From<Ki18nRsError> for c_int {
    fn from(e: Ki18nRsError) -> c_int {
        e as c_int
    }
}

