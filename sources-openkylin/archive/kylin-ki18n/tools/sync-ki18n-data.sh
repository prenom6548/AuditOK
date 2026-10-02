#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 可选：从上游 ki18n 刷新 ki18n-data/ 与 testdata/ki18n-autotests/（仓库已含初始快照）。
# 用法: sync-ki18n-data.sh [/path/to/ki18n] [/path/to/ki18n-build]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
KI18N="${1:-$ROOT/../ki18n}"
KI18N_BUILD="${2:-$KI18N/build}"
if [[ ! -f "$KI18N/CMakeLists.txt" ]]; then
  echo "用法: $0 [/path/to/ki18n] [/path/to/ki18n-build]" >&2
  exit 1
fi
D="$ROOT/ki18n-data"
mkdir -p "$D/cmake" "$D/headers/localedata" "$D/docs" "$D/qlogging-categories"
cp -a "$KI18N/cmake/KF5I18nMacros.cmake.in" \
  "$KI18N/cmake/build-pofiles.cmake" \
  "$KI18N/cmake/build-tsfiles.cmake" \
  "$KI18N/cmake/ts-pmap-compile.py" \
  "$KI18N/cmake/kf5i18nuic.cmake" \
  "$D/cmake/"
rm -rf "$D/po"
cp -a "$KI18N/po" "$D/"
cp -a "$KI18N/src/localedata/kcountry.h" \
  "$KI18N/src/localedata/kcountrysubdivision.h" \
  "$KI18N/src/localedata/ktimezone.h" \
  "$D/headers/localedata/"
cp -a "$KI18N/docs/programmers-guide.md" "$KI18N/docs/translators-guide.md" "$KI18N/README.md" "$D/docs/"
if [[ -f "$KI18N_BUILD/src/ki18n.categories" && -f "$KI18N_BUILD/src/ki18n.renamecategories" ]]; then
  cp -a "$KI18N_BUILD/src/ki18n.categories" "$KI18N_BUILD/src/ki18n.renamecategories" "$D/qlogging-categories/"
  echo "已更新 qlogging（$KI18N_BUILD）"
else
  echo "未找到构建目录下的 ki18n.categories，保留 ki18n-data/qlogging-categories 现有文件"
fi
T="$ROOT/testdata/ki18n-autotests"
rm -rf "$T"
cp -a "$KI18N/autotests" "$T"
echo "已同步 -> $D 与 $T"
