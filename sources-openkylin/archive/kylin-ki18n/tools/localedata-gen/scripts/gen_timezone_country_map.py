#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import json
import os
import re
import sys
from typing import Dict, List, Tuple


def load_text(path: str) -> str:
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def parse_tz_name_table(tz_name_table_cpp: str) -> List[str]:
    # timezone_name_table.cpp:
    #   static constexpr const char timezone_name_table[] =
    #       "Africa/Abidjan\0"
    #       "Africa/Accra\0"
    #       ...
    text = load_text(tz_name_table_cpp)
    # Capture string content before the literal "\0"
    # Note: in the .cpp file, "\0" is two chars: backslash and zero.
    tzs = re.findall(r'"([^"]+)\\0"', text)
    if not tzs:
        raise RuntimeError(f"failed to parse timezone_name_table from {tz_name_table_cpp}")
    return tzs


def parse_tz_offsets(timezone_names_p_h: str) -> Dict[str, int]:
    # timezone_names_p.h:
    #   enum Tz : uint16_t {
    #       Africa_Abidjan = 0,
    #       Africa_Accra = 15,
    #       ...
    #   };
    text = load_text(timezone_names_p_h)
    # Capture: TokenName = number
    pairs = re.findall(r'^\s*([A-Za-z0-9_]+)\s*=\s*(\d+)\s*,\s*$', text, flags=re.MULTILINE)
    if not pairs:
        raise RuntimeError(f"failed to parse Tz offsets from {timezone_names_p_h}")
    out: Dict[str, int] = {}
    for token, off_s in pairs:
        if token == "Undefined":
            # ki18n uses Undefined at offset 0. We can keep it; it's harmless.
            pass
        out[token] = int(off_s)
    return out


def parse_timezone_country_map(timezone_country_map_cpp: str) -> List[Tuple[str, str]]:
    # timezone_country_map.cpp:
    #   static constexpr const MapEntry<uint16_t> timezone_country_map[] = {
    #       {Tz::Africa_Abidjan, IsoCodes::alpha2CodeToKey("CI")},
    #       ...
    #       // Tz::Asia_Bangkok   (missing => ambiguous/empty in ki18n)
    #   };
    text = load_text(timezone_country_map_cpp)
    pat = re.compile(
        r'\{\s*Tz::([A-Za-z0-9_]+)\s*,\s*IsoCodes::alpha2CodeToKey\("([A-Z]{2})"\)\s*\}',
        re.MULTILINE,
    )
    out: List[Tuple[str, str]] = [(m.group(1), m.group(2)) for m in pat.finditer(text)]
    if not out:
        raise RuntimeError(f"failed to parse timezone_country_map from {timezone_country_map_cpp}")
    return out


def build_offset_to_tz(tz_list: List[str]) -> Dict[int, str]:
    # timezone_names_p.h enum offsets correspond to byte offsets into the
    # concatenated timezone_name_table[] char array.
    offset = 0
    out: Dict[int, str] = {}
    for tz in tz_list:
        out[offset] = tz
        offset += len(tz.encode("utf-8")) + 1  # +1 for the literal '\0' terminator in the char array
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate ki18n-like timezone->country map JSON")
    ap.add_argument("--timezone-country-map-cpp", required=True, help="ki18n timezone_country_map.cpp path")
    ap.add_argument("--out-file", required=True, help="output timezone_country_map.json path")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    data_dir = os.path.dirname(os.path.abspath(args.timezone_country_map_cpp))
    tz_names_p_h = os.path.join(data_dir, "timezone_names_p.h")
    tz_name_table_cpp = os.path.join(data_dir, "timezone_name_table.cpp")

    if not os.path.exists(tz_names_p_h):
        print(f"missing: {tz_names_p_h}", file=sys.stderr)
        return 2
    if not os.path.exists(tz_name_table_cpp):
        print(f"missing: {tz_name_table_cpp}", file=sys.stderr)
        return 2

    tz_list = parse_tz_name_table(tz_name_table_cpp)
    offset_to_tz = build_offset_to_tz(tz_list)
    tz_offsets = parse_tz_offsets(tz_names_p_h)
    tz_country_pairs = parse_timezone_country_map(args.timezone_country_map_cpp)

    out: Dict[str, str] = {}
    missing = 0
    for token, alpha2 in tz_country_pairs:
        off = tz_offsets.get(token)
        if off is None:
            missing += 1
            continue
        tz_id = offset_to_tz.get(off)
        if tz_id is None:
            missing += 1
            continue
        out[tz_id] = alpha2

    if missing and args.verbose:
        print(f"gen_timezone_country_map: skipped {missing} entries (unresolved offsets)", file=sys.stderr)

    os.makedirs(os.path.dirname(os.path.abspath(args.out_file)), exist_ok=True)
    with open(args.out_file, "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
        f.write("\n")

    if args.verbose:
        print(f"generated {args.out_file}: {len(out)} timezone->country mappings")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

