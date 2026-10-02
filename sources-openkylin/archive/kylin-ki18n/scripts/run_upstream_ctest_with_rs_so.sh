#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 将本仓库构建的共享库覆盖到上游 ki18n build/bin 后运行 ctest（不重链测试程序）。
# 用法: export KI18N_UPSTREAM_BUILD=/path/to/ki18n/build
#       export KI18N_RS_BUILD=/path/to/kylin-ki18n/build  # 可选，默认 ../build
#       ./run_upstream_ctest_with_rs_so.sh [ctest args]
# 依赖: 上游构建目录已完成配置并生成 autotests。
# 详情: 符号与失败项分析见 scripts/analyze_kf5i18n_dynamic_symbols.sh。

set -euo pipefail

UPSTREAM="${KI18N_UPSTREAM_BUILD:-}"
RSROOT="$(cd "$(dirname "$0")/.." && pwd)"
RSBUILD="${KI18N_RS_BUILD:-$RSROOT/build}"
BIN="${UPSTREAM}/bin"

if [[ -z "$UPSTREAM" || ! -d "$BIN" ]]; then
  echo "请设置 KI18N_UPSTREAM_BUILD 为上游 ki18n 的 CMake 构建目录（含 bin/）。" >&2
  exit 1
fi

RSLIB="$RSBUILD/cpp/src"
RSLOC="$RSBUILD/cpp/localedata"
for f in "$RSLIB/libKF5I18n.so.5.92.0" "$RSLIB/ktranscript.so" "$RSLOC/libKF5I18nLocaleData.so.5.92.0"; do
  if [[ ! -f "$f" ]]; then
    echo "缺少 $f ，请先在 kylin-ki18n 目录 cargo build --release 并 cmake --build build。" >&2
    exit 1
  fi
done

BACKUP=$(mktemp -d)
cleanup() {
  if [[ -d "$BACKUP" ]] && compgen -G "$BACKUP/*" >/dev/null; then
    cp -a "$BACKUP/"* "$BIN/"
  fi
  rm -rf "$BACKUP"
}
trap cleanup EXIT

cp -a "$BIN/libKF5I18n.so"* "$BIN/ktranscript.so" "$BIN/libKF5I18nLocaleData.so"* "$BACKUP/" 2>/dev/null || true
cp -a "$RSLIB/libKF5I18n.so"* "$RSLIB/ktranscript.so" "$BIN/"
cp -a "$RSLOC/libKF5I18nLocaleData.so"* "$BIN/"

export LD_LIBRARY_PATH="$BIN:${LD_LIBRARY_PATH:-}"
cd "$UPSTREAM"
ctest --output-on-failure "$@"
