#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import json
import os
import re
import sys
from typing import Any, Dict, List, Optional, Set

import yaml


def load_json(path: str) -> Any:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def to_alpha2_emoji(alpha2: str) -> Optional[str]:
    if len(alpha2) != 2 or not alpha2.isalpha():
        return None
    base = 0x1F1E6
    chars = []
    for c in alpha2.upper():
        chars.append(chr(base + (ord(c) - ord("A"))))
    return "".join(chars)


def parse_numeric(value: Any) -> Optional[int]:
    if value is None:
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        stripped = value.strip()
        if stripped.isdigit():
            n = int(stripped)
            if 0 <= n <= 65535:
                return n
    return None


def load_country_timezones_from_tz_tab(path: str) -> Dict[str, Set[str]]:
    country_to_tz: Dict[str, Set[str]] = {}
    if not os.path.exists(path):
        return country_to_tz
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 3:
                continue
            countries = parts[0].strip()
            tz_id = parts[2].strip()
            if not tz_id:
                continue
            for cc in countries.split(","):
                code = cc.strip().upper()
                if len(code) != 2 or not code.isalpha():
                    continue
                if code not in country_to_tz:
                    country_to_tz[code] = set()
                country_to_tz[code].add(tz_id)
    return country_to_tz


def load_ki18n_timezone_ids(path: str) -> Set[str]:
    ids: Set[str] = set()
    if not path or not os.path.exists(path):
        return ids
    pattern = re.compile(r'"([^"]+)\\0"')
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            m = pattern.search(line)
            if not m:
                continue
            tz = m.group(1)
            if "/" in tz:
                ids.add(tz)
    return ids


def build_iso_3166_1(doc: Dict[str, Any], include_emoji: bool, strict_missing_name: bool) -> List[Dict[str, Any]]:
    result: List[Dict[str, Any]] = []
    entries = doc.get("3166-1", [])
    for entry in entries:
        alpha2 = (entry.get("alpha_2") or "").strip().upper()
        alpha3 = (entry.get("alpha_3") or "").strip().upper()
        name = (entry.get("name") or "").strip()
        if not alpha2 or not alpha3:
            continue
        if not name and strict_missing_name:
            raise ValueError(f"missing country name for {alpha2}")

        rec = {
            "alpha2": alpha2,
            "alpha3": alpha3,
            "name": name,
            "numeric": parse_numeric(entry.get("numeric")),
            "currency": None,
            "emoji": to_alpha2_emoji(alpha2) if include_emoji else None,
            "timezones": [],
        }
        result.append(rec)
    result.sort(key=lambda x: x["alpha2"])
    return result


def attach_country_timezones(
    countries: List[Dict[str, Any]],
    country_to_tz: Dict[str, Set[str]],
    allowed_tz: Optional[Set[str]] = None,
) -> None:
    for rec in countries:
        code = rec["alpha2"]
        tzs = country_to_tz.get(code, set())
        if allowed_tz is not None and allowed_tz:
            tzs = {tz for tz in tzs if tz in allowed_tz}
        tzs = sorted(tzs)
        rec["timezones"] = tzs


def build_iso_3166_2(doc: Dict[str, Any], fill_timezones_empty: bool) -> List[Dict[str, Any]]:
    result: List[Dict[str, Any]] = []
    entries = doc.get("3166-2", [])
    for entry in entries:
        code = (entry.get("code") or "").strip().upper()
        name = (entry.get("name") or "").strip()
        parent = (entry.get("parent") or "").strip().upper()
        if not code:
            continue
        if "-" not in code:
            continue
        country_alpha2 = code.split("-", 1)[0]
        rec = {
            "code": code,
            "name": name,
            "country_alpha2": country_alpha2,
            "parent_code": parent or None,
            "timezone_ids": [] if fill_timezones_empty else None,
        }
        if rec["timezone_ids"] is None:
            rec["timezone_ids"] = []
        result.append(rec)
    result.sort(key=lambda x: x["code"])
    return result


def write_json(path: str, obj: Any) -> None:
    with open(path, "w", encoding="utf-8") as f:
        json.dump(obj, f, ensure_ascii=False, indent=2)
        f.write("\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate kylin-ki18n ISO locale JSON files")
    parser.add_argument("--iso-codes-dir", required=True, help="Path to iso-codes json directory")
    parser.add_argument("--out-dir", required=True, help="Output directory")
    parser.add_argument(
        "--tz-tab",
        required=False,
        default="/usr/share/zoneinfo/zone.tab",
        help="tzdb zone.tab path used to populate country timezones",
    )
    parser.add_argument(
        "--ki18n-timezone-table",
        required=False,
        default="",
        help="Optional ki18n timezone_name_table.cpp path to filter timezone IDs",
    )
    parser.add_argument("--config", required=False, help="YAML config path")
    parser.add_argument("--fail-on-missing-field", action="store_true", help="Enable strict mode")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    cfg: Dict[str, Any] = {}
    if args.config:
        with open(args.config, "r", encoding="utf-8") as f:
            cfg = yaml.safe_load(f) or {}
    gen_cfg = cfg.get("generator", {})

    include_emoji = bool(gen_cfg.get("include_emoji", True))
    fill_timezones_empty = bool(gen_cfg.get("fill_timezones_empty", True))
    strict_missing_name = bool(gen_cfg.get("strict_missing_name", False)) or args.fail_on_missing_field

    src_1 = os.path.join(args.iso_codes_dir, "iso_3166-1.json")
    src_2 = os.path.join(args.iso_codes_dir, "iso_3166-2.json")
    if not os.path.exists(src_1):
        print(f"missing input file: {src_1}", file=sys.stderr)
        return 2
    if not os.path.exists(src_2):
        print(f"missing input file: {src_2}", file=sys.stderr)
        return 2

    country_to_tz: Dict[str, Set[str]] = {}
    allowed_tz: Set[str] = set()
    try:
        doc1 = load_json(src_1)
        doc2 = load_json(src_2)
        out1 = build_iso_3166_1(doc1, include_emoji=include_emoji, strict_missing_name=strict_missing_name)
        out2 = build_iso_3166_2(doc2, fill_timezones_empty=fill_timezones_empty)
        country_to_tz = load_country_timezones_from_tz_tab(args.tz_tab)
        allowed_tz = load_ki18n_timezone_ids(args.ki18n_timezone_table)
        attach_country_timezones(out1, country_to_tz, allowed_tz if allowed_tz else None)
    except Exception as e:
        print(f"generation failed: {e}", file=sys.stderr)
        return 1

    ensure_dir(args.out_dir)
    path1 = os.path.join(args.out_dir, "iso_3166_1.json")
    path2 = os.path.join(args.out_dir, "iso_3166_2.json")
    write_json(path1, out1)
    write_json(path2, out2)

    if args.verbose:
        print(f"generated {path1}: {len(out1)} records")
        print(f"generated {path2}: {len(out2)} records")
        if country_to_tz:
            print(f"loaded country-timezone mappings from {args.tz_tab}: {len(country_to_tz)} countries")
        if args.ki18n_timezone_table and allowed_tz:
            print(f"filtered country timezones by ki18n table: {len(allowed_tz)} timezone ids")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
