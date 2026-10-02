#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 使用上游 autotest「同一套」fixtures（autotests/test.js、ktranscript-test.ini 等），
# 分别在上游 .so 与 kylin-ki18n .so 下各跑一遍 ki18n-ktranscripttest，并 diff 输出。
#
# 用法:
#   export KI18N_UPSTREAM_BUILD=/path/to/ki18n/build
#   ./diff_ktranscript_ctest_upstream_vs_rs.sh
#
# 退出码: diff 无差异为 0；有差异为 1。

set -euo pipefail

UPSTREAM="${KI18N_UPSTREAM_BUILD:-}"
if [[ -z "$UPSTREAM" ]]; then
  echo "请设置 KI18N_UPSTREAM_BUILD。" >&2
  exit 1
fi

RSROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT="$RSROOT/scripts/run_upstream_ctest_with_rs_so.sh"
A=$(mktemp)
B=$(mktemp)

(
  cd "$UPSTREAM"
  export LD_LIBRARY_PATH="$UPSTREAM/bin:${LD_LIBRARY_PATH:-}"
  ctest -R ki18n-ktranscripttest -V 2>&1 | tee "$A"
)

bash "$SCRIPT" -R ki18n-ktranscripttest -V 2>&1 | tee "$B"

# 仅比较各数据行的 PASS/FAIL（忽略 dbg/warn 前缀、Totals 内耗时等）
filter_qtest() { grep -E '^4: (PASS|FAIL|SKIP|XFAIL)[[:space:]]' "$1" || true; }

if diff -u <(filter_qtest "$A") <(filter_qtest "$B"); then
  echo "=== ktranscript：各用例 PASS/FAIL 行与上游一致（已忽略日志前缀与耗时）==="
  rm -f "$A" "$B"
  exit 0
fi
echo "=== 完整日志: $A vs $B（未删）===" >&2
rm -f "$A" "$B"
exit 1
