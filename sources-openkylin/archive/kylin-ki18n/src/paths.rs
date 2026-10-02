// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

//! 仓库内打包的上游数据路径（不依赖并列 `ki18n` 源码树）。

use std::path::PathBuf;

/// 上游 `ki18n/autotests` 等价目录。
///
/// 解析顺序：`KI18N_AUTOTESTS_DIR` 环境变量 → 仓库内 `testdata/ki18n-autotests`
/// （由维护脚本从上游同步）→ 开发布局 `../ki18n/autotests`。
pub fn ki18n_autotests_dir() -> PathBuf {
    if let Ok(p) = std::env::var("KI18N_AUTOTESTS_DIR") {
        return PathBuf::from(p);
    }
    let manifest = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let bundled = manifest.join("testdata/ki18n-autotests");
    if bundled.join("test.js").is_file() {
        return bundled;
    }
    manifest.join("../ki18n/autotests")
}
