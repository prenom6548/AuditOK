#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import os
import re
import struct
import json
import sys
from typing import Dict, List, Optional, Tuple


def parse_timezone_name_table(tz_name_table_cpp: str) -> List[str]:
    with open(tz_name_table_cpp, "r", encoding="utf-8") as f:
        text = f.read()
    # Example lines:
    #   "Africa/Abidjan\0"
    ids = re.findall(r'"([^"]+)\\0"', text)
    if not ids:
        raise RuntimeError(f"failed to parse timezone_name_table: {tz_name_table_cpp}")
    return ids


def parse_timezone_names_p_h(tz_names_p_h: str) -> Dict[str, int]:
    with open(tz_names_p_h, "r", encoding="utf-8") as f:
        text = f.read()
    # 先处理每行，去除首尾空白，避免回溯问题
    lines = [line.strip() for line in text.splitlines()]
    pairs = []
    for line in lines:
        m = re.match(r'(\w+)\s*=\s*(\d+),?$', line)
        if m:
            pairs.append((m.group(1), m.group(2)))
    if not pairs:
        raise RuntimeError(f"failed to parse timezone_names_p.h: {tz_names_p_h}")
    return {k: int(v) for k, v in pairs}


def parse_token_to_tz_id(
    tz_names_p_h: str, tz_name_table_cpp: str
) -> Dict[str, str]:
    tz_list = parse_timezone_name_table(tz_name_table_cpp)
    offset_to_tz: Dict[int, str] = {}
    offset = 0
    for tz in tz_list:
        offset_to_tz[offset] = tz
        # In timezone_name_table.cpp each string is stored as "....\0"
        offset += len(tz.encode("utf-8")) + 1

    token_to_offset = parse_timezone_names_p_h(tz_names_p_h)
    out: Dict[str, str] = {}
    for token, off in token_to_offset.items():
        tz_id = offset_to_tz.get(off)
        if tz_id is not None:
            out[token] = tz_id
    return out


def parse_spatial_index_entries(spatial_index_data_cpp: str) -> List[Tuple[int, int]]:
    # Each entry is: {z, propertyIdx}
    # Parse the array region rather than trusting line formatting.
    text = open(spatial_index_data_cpp, "r", encoding="utf-8").read()
    start = text.find("spatial_index[]")
    if start == -1:
        raise RuntimeError(f"failed to locate spatial_index[] in {spatial_index_data_cpp}")
    region = text[start:]
    # cut to initializer end if possible
    end = region.find("};")
    if end != -1:
        region = region[:end]

    # Typical entry: {40960, 3395},
    # allow trailing comma and whitespace
    m = re.findall(r'\{\s*(\d+)\s*,\s*(\d+)\s*\}\s*,?', region)
    if not m:
        raise RuntimeError(f"no entries parsed from {spatial_index_data_cpp}")
    entries: List[Tuple[int, int]] = [(int(a), int(b)) for a, b in m]
    return entries


def parse_spatial_index_properties(
    spatial_index_properties_cpp: str,
    token_to_tz_id: Dict[str, str],
) -> Tuple[List[Optional[str]], List[Optional[str]]]:
    # propertyIdx -> tzId / countryAlpha2
    text = open(spatial_index_properties_cpp, "r", encoding="utf-8").read()
    start = text.find("spatial_index_properties[]")
    if start == -1:
        raise RuntimeError(
            f"failed to locate spatial_index_properties[] in {spatial_index_properties_cpp}"
        )
    region = text[start:]
    end = region.find("};")
    if end != -1:
        region = region[:end]

    # Typical entry:
    #   {Tz::Undefined},
    #   {Tz::Africa_Abidjan, "CI"},
    # 使用更安全的解析方法，避免复杂正则表达式的回溯风险
    entries = []
    for line in region.splitlines():
        line = line.strip()
        if not line or not line.startswith("{"):
            continue
        # 移除大括号
        content = line.strip("{}").strip()
        parts = content.split(",", 1)
        if not parts:
            continue
        tz_token = parts[0].strip().replace("Tz::", "")
        code = None
        if len(parts) > 1:
            code_part = parts[1].strip()
            # 提取引号内的内容
            if code_part.startswith('"') and code_part.endswith('"'):
                code = code_part[1:-1]
        entries.append((tz_token, code))
    
    if not entries:
        raise RuntimeError(
            f"no spatial properties parsed from {spatial_index_properties_cpp}"
        )

    tz_by_prop: List[Optional[str]] = []
    country_by_prop: List[Optional[str]] = []
    for tz_token, code in entries:
        tz_id: Optional[str]
        if tz_token == "Undefined":
            tz_id = None
        else:
            tz_id = token_to_tz_id.get(tz_token)
            if tz_id is None:
                raise RuntimeError(
                    f"missing token->tz mapping: {tz_token} (from {spatial_index_properties_cpp})"
                )

        alpha2: Optional[str]
        if code is None:
            alpha2 = None
        else:
            # code is either "US" or "US-CA" or possibly "USA".
            alpha2 = code[:2] if len(code) >= 2 else None

        tz_by_prop.append(tz_id)
        country_by_prop.append(alpha2)

    # Final sanity check (fail fast if parsing got truncated)
    if not tz_by_prop:
        raise RuntimeError(
            f"no spatial properties parsed from {spatial_index_properties_cpp}"
        )

    return tz_by_prop, country_by_prop


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate ki18n-like spatial index lookup data")
    ap.add_argument("--spatial-index-data-cpp", required=True)
    ap.add_argument("--spatial-index-properties-cpp", required=True)
    ap.add_argument("--out-entries-bin", required=True)
    ap.add_argument("--out-properties-json", required=True)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    data_dir = os.path.dirname(os.path.abspath(args.spatial_index_data_cpp))
    tz_names_p_h = os.path.join(data_dir, "timezone_names_p.h")
    tz_name_table_cpp = os.path.join(data_dir, "timezone_name_table.cpp")

    if not os.path.exists(tz_names_p_h) or not os.path.exists(tz_name_table_cpp):
        print(
            f"missing timezone_name_table/tz_names_p.h under {data_dir}",
            file=sys.stderr,
        )
        return 2

    if args.verbose:
        print(f"Parsing spatial_index_data... {args.spatial_index_data_cpp}")
    entries = parse_spatial_index_entries(args.spatial_index_data_cpp)
    if len(entries) < 1000:
        raise RuntimeError(
            f"spatial_index_entries seems too small: {len(entries)} (from {args.spatial_index_data_cpp})"
        )

    if args.verbose:
        print(f"Parsing timezone token mapping...")
    token_to_tz_id = parse_token_to_tz_id(tz_names_p_h, tz_name_table_cpp)

    if args.verbose:
        print(f"Parsing spatial_index_properties... {args.spatial_index_properties_cpp}")
    tz_by_prop, country_by_prop = parse_spatial_index_properties(
        args.spatial_index_properties_cpp, token_to_tz_id
    )
    if len(tz_by_prop) < 100:
        raise RuntimeError(
            f"spatial_index_properties seems too small: {len(tz_by_prop)} (from {args.spatial_index_properties_cpp})"
        )

    # Ensure propertyIdx referenced by entries exists in parsed property table.
    if entries:
        max_prop = max(p for _, p in entries)
        if max_prop >= len(tz_by_prop):
            raise RuntimeError(
                f"parsed properties truncated: max propertyIdx={max_prop}, parsed properties={len(tz_by_prop)}"
            )

    # Write entries binary: repeated (z:u32 LE, propIdx:u16 LE)
    os.makedirs(os.path.dirname(os.path.abspath(args.out_entries_bin)), exist_ok=True)
    with open(args.out_entries_bin, "wb") as f:
        for z, prop in entries:
            if prop < 0 or prop > 0xFFFF:
                raise RuntimeError(f"propertyIdx out of u16 range: {prop}")
            f.write(struct.pack("<IH", z, prop))

    # Write properties json
    os.makedirs(os.path.dirname(os.path.abspath(args.out_properties_json)), exist_ok=True)
    with open(args.out_properties_json, "w", encoding="utf-8") as f:
        json.dump(
            {
                "tz_by_property": tz_by_prop,
                "country_by_property": country_by_prop,
            },
            f,
            ensure_ascii=False,
        )
        f.write("\n")

    if args.verbose:
        print(f"Wrote entries: {len(entries)}")
        print(f"Wrote properties: {len(tz_by_prop)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

