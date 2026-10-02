#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 检查 libKF5I18n.so 动态符号，确认链入 Rust 后未出现异常导出。
# 用法: ./analyze_kf5i18n_dynamic_symbols.sh [libKF5I18n.so.5.92.0]
# 当前策略: C++ 使用 --gc-sections/--exclude-libs + hidden；Rust release 使用 thin LTO。
# 注意: cpp/src/visibility.map 未直接启用，避免 `_Z*` 过宽放行。
# 若出现大量 `_R` / `__rust` / `std::` 符号，再评估 version-script 白名单。

set -euo pipefail

RSROOT="$(cd "$(dirname "$0")/.." && pwd)"
SO="${1:-$RSROOT/build/cpp/src/libKF5I18n.so.5.92.0}"

if [[ ! -f "$SO" ]]; then
  echo "缺少 $SO ，请先 cmake --build build。" >&2
  exit 1
fi

echo "== File: $SO"
if command -v readelf >/dev/null; then
  echo -n "== SONAME: "
  readelf -d "$SO" | grep SONAME || true
fi

echo "== Defined dynamic symbol count"
nm -D --defined-only "$SO" | wc -l

echo "== ki18n_rs_* (C ABI) in dynamic table (expect 0 若仅经 C++ 包装引用)"
nm -D --defined-only "$SO" | grep -c ' ki18n_rs_' || true

echo "== Rust new mangling (_R*) / __rust (expect empty)"
nm -D --defined-only "$SO" | awk '$3 ~ /^_R[^N]/ || $3 ~ /^__rust/ {print}' | head -20

echo "== Legacy rustc hash suffix sample (17h........E) (expect empty)"
nm -D --defined-only "$SO" | awk '$3 ~ /17h[0-9a-f]{16}E/ {print}' | head -10

echo "== KLocalized / KCatalog / Kuit / load_transcript (spot check)"
nm -D --defined-only "$SO" | grep -E '_ZN16KLocalizedString|_ZN8KCatalog|_ZN9KuitSetup|load_transcript' | head -15

echo "== Done"
