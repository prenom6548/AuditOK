#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

"""从上游 ki18n 的 timezonedata 源文件生成 kylin-ki18n/data/subdivision_timezone_map.json。"""
import json
import re
import sys
from collections import defaultdict
from pathlib import Path


_ERR_PARSE_TABLE = "无法解析 timezone_name_table"


def main() -> int:
    ki18n_root = Path(__file__).resolve().parents[2] / "ki18n" / "src" / "localedata" / "data"
    out = Path(__file__).resolve().parents[1] / "data" / "subdivision_timezone_map.json"
    if not ki18n_root.is_dir():
        print("缺少上游 ki18n 目录:", ki18n_root, file=sys.stderr)
        return 1

    h = (ki18n_root / "timezone_names_p.h").read_text(encoding="utf-8", errors="replace")
    block = h.split("enum Tz", 1)[1].split("};", 1)[0]
    tz_vals = {}
    for m in re.finditer(r"\b([A-Za-z][A-Za-z0-9_]*)\s*=\s*(\d+),", block):
        tz_vals["Tz::" + m.group(1)] = int(m.group(2))

    tbl = (ki18n_root / "timezone_name_table.cpp").read_text(encoding="utf-8", errors="replace")
    # 使用字符串操作代替正则表达式，避免回溯风险
    start = tbl.find("timezone_name_table[]")
    if start == -1:
        print(_ERR_PARSE_TABLE, file=sys.stderr)
        return 1
    eq_pos = tbl.find("=", start)
    if eq_pos == -1:
        print(_ERR_PARSE_TABLE, file=sys.stderr)
        return 1
    # 找到第一个分号（假设表定义在一行或用分号结束）
    semicolon_pos = tbl.find(";", eq_pos)
    if semicolon_pos == -1:
        print(_ERR_PARSE_TABLE, file=sys.stderr)
        return 1
    content = tbl[eq_pos + 1:semicolon_pos]
    parts = re.findall(r'"((?:[^"\\]|\\.)*)"', content)
    blob = "".join(bytes(s, "utf-8").decode("unicode_escape") for s in parts)

    def iana_for_tz(tz_token: str) -> str:
        off = tz_vals[tz_token]
        end = blob.find("\0", off)
        return blob[off:end] if end >= 0 else blob[off:]

    sub_cpp = (ki18n_root / "subdivision_timezone_map.cpp").read_text(
        encoding="utf-8", errors="replace"
    )
    pairs = re.findall(
        r'subdivisionCodeToKey\("([A-Z]{2}-[^"]+)"\)\s*,\s*(Tz::[A-Za-z0-9_]+)',
        sub_cpp,
    )
    ordered: dict[str, list[str]] = defaultdict(list)
    for code, tz in pairs:
        ordered[code].append(iana_for_tz(tz))

    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", encoding="utf-8") as f:
        json.dump({k: ordered[k] for k in sorted(ordered.keys())}, f, indent=2, ensure_ascii=False)
        f.write("\n")
    print("wrote", out, "codes", len(ordered), "rows", len(pairs))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
