#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

"""从上游 ki18n 的 spatial_index_data.cpp / spatial_index_properties.cpp 生成
`data/spatial_index_entries.bin`，并为 `data/spatial_index_properties.json` 写入与属性表对齐的
`subdivision_by_property`（与 `tools/localedata-gen` 的语义一致）。"""
import json
import re
import struct
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    ki18n_data = root.parent / "ki18n" / "src" / "localedata" / "data"
    if not ki18n_data.is_dir():
        print("缺少上游 ki18n:", ki18n_data, file=sys.stderr)
        return 1

    props_cpp = (ki18n_data / "spatial_index_properties.cpp").read_text(
        encoding="utf-8", errors="replace"
    )
    subdiv_list = []
    for line in props_cpp.splitlines():
        line = line.strip().rstrip(",")
        if not line.startswith("{Tz::"):
            continue
        m = re.match(r"\{Tz::\w+,\s*\"([^\"]+)\"\s*\}$", line)
        if m:
            lit = m.group(1)
            if "-" in lit and len(lit) >= 4:
                subdiv_list.append(lit)
            else:
                subdiv_list.append(None)
        else:
            subdiv_list.append(None)

    data_cpp = (ki18n_data / "spatial_index_data.cpp").read_text(
        encoding="utf-8", errors="replace"
    )
    pairs = re.findall(r"\{(\d+),\s*(\d+)\}", data_cpp)
    out_bin = root / "data" / "spatial_index_entries.bin"
    out_bin.parent.mkdir(parents=True, exist_ok=True)
    with out_bin.open("wb") as f:
        for z_s, p_s in pairs:
            f.write(struct.pack("<IH", int(z_s), int(p_s)))

    js_path = root / "data" / "spatial_index_properties.json"
    doc = json.loads(js_path.read_text(encoding="utf-8"))
    if len(subdiv_list) != len(doc["tz_by_property"]):
        print(
            "长度不一致: subdivision",
            len(subdiv_list),
            "tz",
            len(doc["tz_by_property"]),
            file=sys.stderr,
        )
        return 1
    doc["subdivision_by_property"] = subdiv_list
    js_path.write_text(json.dumps(doc, ensure_ascii=False), encoding="utf-8")
    print("wrote", out_bin, "entries", len(pairs), "subdiv cells", sum(1 for x in subdiv_list if x))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
