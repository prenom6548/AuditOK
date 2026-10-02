#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import json
import os
import re
from typing import Dict, List, Set


def load_generated_tz_ids(path: str) -> Set[str]:
    with open(path, "r", encoding="utf-8") as f:
        arr = json.load(f)
    return {item["timezone"] for item in arr if isinstance(item, dict) and "timezone" in item}


def load_ki18n_tz_ids(path: str) -> Set[str]:
    ids: Set[str] = set()
    pattern = re.compile(r'"([^"]+)\\0"')
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            m = pattern.search(raw)
            if not m:
                continue
            content = m.group(1)
            if "/" in content:
                ids.add(content)
    return ids


def load_whitelist(path: str) -> Set[str]:
    if not path or not os.path.exists(path):
        return set()
    out: Set[str] = set()
    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            out.add(line)
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare generated timezone IDs with ki18n timezone table")
    parser.add_argument("--generated", required=True, help="Path to generated timezone_coords.json")
    parser.add_argument("--ki18n-table", required=True, help="Path to ki18n timezone_name_table.cpp")
    parser.add_argument("--min-coverage", type=float, default=0.75, help="Minimum coverage threshold")
    parser.add_argument("--whitelist-file", default="", help="Optional whitelist file for accepted tz diffs")
    parser.add_argument("--report-out", default="", help="Optional output JSON report path")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    if not os.path.exists(args.generated):
        print(f"missing generated file: {args.generated}")
        return 2
    if not os.path.exists(args.ki18n_table):
        print(f"missing ki18n table: {args.ki18n_table}")
        return 2

    gen = load_generated_tz_ids(args.generated)
    ref = load_ki18n_tz_ids(args.ki18n_table)
    whitelist = load_whitelist(args.whitelist_file)
    if not ref:
        print("ki18n reference list is empty")
        return 2

    overlap = gen & ref
    coverage = len(overlap) / len(ref)
    missing_all = sorted(ref - gen)
    extra_all = sorted(gen - ref)
    missing = sorted(x for x in missing_all if x not in whitelist)
    extra = sorted(x for x in extra_all if x not in whitelist)

    print(f"generated={len(gen)} ref={len(ref)} overlap={len(overlap)} coverage={coverage:.4f}")
    if args.verbose and missing:
        print(f"missing_preview={', '.join(missing[:20])}")
    if args.verbose and extra:
        print(f"extra_preview={', '.join(extra[:20])}")

    if args.report_out:
        report = {
            "generated_count": len(gen),
            "reference_count": len(ref),
            "overlap_count": len(overlap),
            "coverage": coverage,
            "whitelist_count": len(whitelist),
            "missing_all": missing_all,
            "extra_all": extra_all,
            "missing_after_whitelist": missing,
            "extra_after_whitelist": extra,
        }
        os.makedirs(os.path.dirname(args.report_out), exist_ok=True)
        with open(args.report_out, "w", encoding="utf-8") as f:
            json.dump(report, f, ensure_ascii=False, indent=2)
            f.write("\n")

    return 0 if coverage >= args.min_coverage and not missing and not extra else 1


if __name__ == "__main__":
    raise SystemExit(main())
