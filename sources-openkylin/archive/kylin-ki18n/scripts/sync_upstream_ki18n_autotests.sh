#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 将上游 ki18n/autotests 同步到本仓库 testdata，便于 Rust 测试与 ctest 共用同一套
# test.js、cities.pmap、ktranscript.ini 等。
#
# 用法:
#   export KI18N_UPSTREAM=/path/to/ki18n   # 默认: 本仓库父目录下的 ki18n
#   ./sync_upstream_ki18n_autotests.sh
#
# 同步目标: kylin-ki18n/testdata/ki18n-autotests/

set -euo pipefail

RSROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${KI18N_UPSTREAM:-${RSROOT}/../ki18n}"
SRC="${UPSTREAM}/autotests"
DST="${RSROOT}/testdata/ki18n-autotests"

if [[ ! -d "$SRC" ]]; then
  echo "找不到上游 autotests: $SRC" >&2
  echo "请设置 KI18N_UPSTREAM 或把 ki18n 源码放在与 kylin-ki18n 并列目录。" >&2
  exit 1
fi

mkdir -p "$DST"
rsync -a --delete \
  --exclude='CMakeLists.txt' \
  --exclude='*.cpp' \
  --exclude='*.h' \
  "$SRC/" "$DST/"
echo "已同步 $SRC -> $DST"
