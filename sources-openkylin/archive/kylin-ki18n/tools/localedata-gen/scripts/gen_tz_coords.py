#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import json
import os
import re
import sys
from typing import Any, Dict, List, Optional, Tuple


# tzdb coordinate format:
#   latitude  = +/- DDMM[SS]
#   longitude = +/- DDDMM[SS]
COORD_RE = re.compile(r"^([+-])(\d{2})(\d{2})(\d{0,2})([+-])(\d{3})(\d{2})(\d{0,2})$")


def _parse_dms(sign: str, deg: str, minute: str, second: str) -> float:
    d = int(deg)
    m = int(minute)
    s = int(second) if second else 0
    val = d + (m / 60.0) + (s / 3600.0)
    return -val if sign == "-" else val


def parse_zone1970_coord(coord: str) -> Optional[Tuple[float, float]]:
    m = COORD_RE.match(coord.strip())
    if not m:
        return None
    lat = _parse_dms(m.group(1), m.group(2), m.group(3), m.group(4))
    lon = _parse_dms(m.group(5), m.group(6), m.group(7), m.group(8))
    if lat < -90 or lat > 90 or lon < -180 or lon > 180:
        return None
    return lat, lon


def load_ki18n_timezone_ids(path: str) -> List[str]:
    ids: List[str] = []
    pattern = re.compile(r'"([^"]+)\\0"')
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            m = pattern.search(line)
            if not m:
                continue
            content = m.group(1)
            if "/" in content:
                ids.append(content)
    return sorted(set(ids))


def parse_tz_tab(path: str) -> Dict[str, Tuple[float, float]]:
    out: Dict[str, Tuple[float, float]] = {}
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 3:
                continue
            coord = parts[1].strip()
            tz_id = parts[2].strip()
            parsed = parse_zone1970_coord(coord)
            if parsed is None:
                continue
            out[tz_id] = parsed
    return out


def parse_tz_links(paths: List[str]) -> Dict[str, str]:
    links: Dict[str, str] = {}
    for path in paths:
        if not path or not os.path.exists(path):
            continue
        with open(path, "r", encoding="utf-8") as f:
            for raw in f:
                line = raw.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split()
                if len(parts) >= 3 and parts[0] in ("Link", "L"):
                    target = parts[1]
                    alias = parts[2]
                    links[alias] = target
    return links


def resolve_target(tz_id: str, links: Dict[str, str]) -> str:
    seen = set()
    cur = tz_id
    while cur in links and cur not in seen:
        seen.add(cur)
        cur = links[cur]
    return cur


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate timezone_coords.json from timezone source")
    parser.add_argument("--tz-source", required=False, help="Timezone source path (zone1970.tab for zone1970 mode)")
    parser.add_argument("--out-file", required=True, help="Output timezone_coords.json path")
    parser.add_argument("--mode", default="zone1970", choices=["placeholder", "zone1970"], help="Generation mode")
    parser.add_argument(
        "--ki18n-timezone-table",
        required=False,
        help="Optional path to ki18n timezone_name_table.cpp to filter output IDs",
    )
    parser.add_argument(
        "--tz-link-files",
        required=False,
        default="/usr/share/zoneinfo/backward,/usr/share/zoneinfo/backzone,/usr/share/zoneinfo/tzdata.zi",
        help="Comma-separated tz link files containing 'Link target alias' records",
    )
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    out: List[Any] = []
    if args.mode == "zone1970":
        # Prefer zone.tab for broader ID coverage, fallback can be set via --tz-source.
        tz_source = args.tz_source or "/usr/share/zoneinfo/zone.tab"
        if not os.path.exists(tz_source):
            print(f"missing tz source file: {tz_source}", file=sys.stderr)
            return 2
        tz_map = parse_tz_tab(tz_source)

        if args.ki18n_timezone_table:
            if not os.path.exists(args.ki18n_timezone_table):
                print(f"missing ki18n timezone table: {args.ki18n_timezone_table}", file=sys.stderr)
                return 2
            allowed = set(load_ki18n_timezone_ids(args.ki18n_timezone_table))
            # Fill missing IDs via tz alias links (Link target alias).
            link_files = [p.strip() for p in args.tz_link_files.split(",") if p.strip()]
            links = parse_tz_links(link_files)
            filled: Dict[str, Tuple[float, float]] = {}
            for tz_id in sorted(allowed):
                if tz_id in tz_map:
                    filled[tz_id] = tz_map[tz_id]
                    continue
                target = resolve_target(tz_id, links)
                if target in tz_map:
                    filled[tz_id] = tz_map[target]
                    continue
                if tz_id == "Etc/UTC":
                    # tzdb tab files usually do not provide coordinates for Etc/UTC.
                    filled[tz_id] = (0.0, 0.0)
            tz_ids = sorted(filled.keys())
        else:
            tz_ids = sorted(tz_map.keys())

        for tz_id in tz_ids:
            if args.ki18n_timezone_table:
                lat, lon = filled[tz_id]
            else:
                lat, lon = tz_map[tz_id]
            out.append({"lat": lat, "lon": lon, "timezone": tz_id})

    os.makedirs(os.path.dirname(args.out_file), exist_ok=True)
    with open(args.out_file, "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
        f.write("\n")

    if args.verbose:
        print(f"generated {args.out_file}: {len(out)} records (mode={args.mode})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
