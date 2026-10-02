#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import os
import re
import json
import sys
from typing import Dict, List, Optional, Tuple


def parse_timezone_name_table(tz_name_table_cpp: str) -> List[str]:
    with open(tz_name_table_cpp, "r", encoding="utf-8") as f:
        text = f.read()
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


def build_token_to_tz_id(tz_names_p_h: str, tz_name_table_cpp: str) -> Dict[str, str]:
    tz_list = parse_timezone_name_table(tz_name_table_cpp)
    offset_to_tz: Dict[int, str] = {}
    offset = 0
    for tz in tz_list:
        offset_to_tz[offset] = tz
        offset += len(tz.encode("utf-8")) + 1

    token_to_offset = parse_timezone_names_p_h(tz_names_p_h)
    out: Dict[str, str] = {}
    for token, off in token_to_offset.items():
        tz_id = offset_to_tz.get(off)
        if tz_id is not None:
            out[token] = tz_id
    return out


def parse_country_timezone_map(
    country_timezone_map_cpp: str, token_to_tz_id: Dict[str, str]
) -> Dict[str, str]:
    # {IsoCodes::alpha2CodeToKey("DE"), Tz::Europe_Berlin},
    out: Dict[str, str] = {}

    text = open(country_timezone_map_cpp, "r", encoding="utf-8").read()
    # allow trailing commas, whitespace, inline comments by matching only inside {}
    pat = re.compile(
        r'\{\s*IsoCodes::alpha2CodeToKey\("([A-Z]{2})"\)\s*,\s*Tz::([A-Za-z0-9_]+)\s*\}\s*,?',
        re.MULTILINE,
    )
    for m in pat.finditer(text):
        alpha2 = m.group(1)
        tz_token = m.group(2)
        tz_id = token_to_tz_id.get(tz_token)
        if tz_id is None:
            raise RuntimeError(f"missing tz mapping for token {tz_token}")
        out[alpha2] = tz_id
    if not out:
        raise RuntimeError(f"no entries parsed from {country_timezone_map_cpp}")
    return out


def parse_subdivision_timezone_map_to_country_ordered_list(
    subdivision_timezone_map_cpp: str, token_to_tz_id: Dict[str, str]
) -> Dict[str, List[str]]:
    # {IsoCodes::subdivisionCodeToKey("AU-ACT"), Tz::Australia_Sydney},
    out: Dict[str, List[str]] = {}

    text = open(subdivision_timezone_map_cpp, "r", encoding="utf-8").read()
    pat = re.compile(
        r'\{\s*IsoCodes::subdivisionCodeToKey\("([A-Z]{2}-[A-Za-z0-9\-]+)"\)\s*,\s*Tz::([A-Za-z0-9_]+)\s*\}\s*,?',
        re.MULTILINE,
    )
    for m in pat.finditer(text):
        sub_code = m.group(1)  # like DE-NW
        tz_token = m.group(2)
        tz_id = token_to_tz_id.get(tz_token)
        if tz_id is None:
            raise RuntimeError(f"missing tz mapping for token {tz_token}")
        alpha2 = sub_code[:2]
        lst = out.setdefault(alpha2, [])
        if tz_id not in lst:
            lst.append(tz_id)
    if not out:
        raise RuntimeError(f"no entries parsed from {subdivision_timezone_map_cpp}")
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate Country timezone maps for KCountry::timeZoneIds()")
    ap.add_argument("--country-timezone-map-cpp", required=True)
    ap.add_argument("--subdivision-timezone-map-cpp", required=True)
    ap.add_argument("--out-country-timezone-map-json", required=True)
    ap.add_argument("--out-country-timezones-from-subdiv-json", required=True)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    data_dir = os.path.dirname(os.path.abspath(args.country_timezone_map_cpp))
    tz_names_p_h = os.path.join(data_dir, "timezone_names_p.h")
    tz_name_table_cpp = os.path.join(data_dir, "timezone_name_table.cpp")
    if not os.path.exists(tz_names_p_h) or not os.path.exists(tz_name_table_cpp):
        print(f"missing timezone_name_table/tz_names_p.h under {data_dir}", file=sys.stderr)
        return 2

    if args.verbose:
        print(f"Building tz token map...")
    token_to_tz_id = build_token_to_tz_id(tz_names_p_h, tz_name_table_cpp)

    if args.verbose:
        print(f"Parsing country timezone map...")
    country_map = parse_country_timezone_map(args.country_timezone_map_cpp, token_to_tz_id)
    if len(country_map) < 50:
        raise RuntimeError(
            f"country_timezone_map seems too small: {len(country_map)} (from {args.country_timezone_map_cpp})"
        )

    if args.verbose:
        print(f"Parsing subdivision timezone map...")
    subdiv_map = parse_subdivision_timezone_map_to_country_ordered_list(
        args.subdivision_timezone_map_cpp, token_to_tz_id
    )
    if len(subdiv_map) < 10:
        raise RuntimeError(
            f"subdivision_timezone_map seems too small: {len(subdiv_map)} (from {args.subdivision_timezone_map_cpp})"
        )

    os.makedirs(os.path.dirname(os.path.abspath(args.out_country_timezone_map_json)), exist_ok=True)
    with open(args.out_country_timezone_map_json, "w", encoding="utf-8") as f:
        json.dump(country_map, f, ensure_ascii=False, indent=2)
        f.write("\n")

    os.makedirs(os.path.dirname(os.path.abspath(args.out_country_timezones_from_subdiv_json)), exist_ok=True)
    with open(args.out_country_timezones_from_subdiv_json, "w", encoding="utf-8") as f:
        json.dump(subdiv_map, f, ensure_ascii=False, indent=2)
        f.write("\n")

    if args.verbose:
        print(f"Wrote: {args.out_country_timezone_map_json} ({len(country_map)} countries)")
        print(f"Wrote: {args.out_country_timezones_from_subdiv_json} ({len(subdiv_map)} countries)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

