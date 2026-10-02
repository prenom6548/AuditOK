#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# 白盒门禁：执行单元测试（见 docs/白盒单元测试指导手册.docx）。
# 本工程为 Rust + CMake；单测由 cargo test 统一执行。退出码与 cargo 一致。

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit 1

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"
export QT_LOGGING_RULES="${QT_LOGGING_RULES:-qt.qpa.*=false}"
export QT_FATAL_WARNINGS="${QT_FATAL_WARNINGS:-0}"

# 避免无 TTY 时部分用例不稳定；单测勿依赖显示。
export CARGO_TERM_COLOR="${CARGO_TERM_COLOR:-always}"

echo "===== kylin-ki18n: cargo test (ROOT=$ROOT) ====="

if ! command -v cargo >/dev/null 2>&1; then
  echo "错误：未找到 cargo，请先执行 tests/auto_test.sh 安装依赖。" >&2
  exit 1
fi

# 默认不运行 #[ignore] 用例；需要时: export KI18N_RUN_IGNORED_TESTS=1
EXTRA=( )
if [[ "${KI18N_RUN_IGNORED_TESTS:-}" == "1" ]]; then
  EXTRA+=( -- --include-ignored )
fi

if ! cargo test --manifest-path "$ROOT/Cargo.toml" "${EXTRA[@]}"; then
  echo "Error: cargo test failed." >&2
  exit 1
fi

echo "===== kylin-ki18n: cargo test finished successfully ====="
exit 0
