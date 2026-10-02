#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import datetime as dt
import hashlib
import json
import os
from typing import Dict, List

import yaml


TRACKED = [
    "iso_3166_1.json",
    "iso_3166_2.json",
    "timezone_coords.json",
    "timezone_country_map.json",
    # Fine-parity data files.
    "spatial_index_properties.json",
    "country_timezone_map.json",
    "country_timezones_from_subdiv.json",
    "subdivision_timezone_map.json",
]


def sha256_of(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            chunk = f.read(1024 * 1024)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()


def json_count(path: str) -> int:
    with open(path, "r", encoding="utf-8") as f:
        obj = json.load(f)
    return len(obj) if isinstance(obj, list) else 1


def main() -> int:
    parser = argparse.ArgumentParser(description="Build MANIFEST.json for localedata")
    parser.add_argument("--data-dir", required=True, help="Data directory")
    parser.add_argument("--out-file", required=True, help="Output MANIFEST.json path")
    parser.add_argument("--config", required=False, help="YAML config path")
    parser.add_argument("--source-tag", required=False, default="", help="Optional source version tag")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    cfg: Dict = {}
    if args.config and os.path.exists(args.config):
        with open(args.config, "r", encoding="utf-8") as f:
            cfg = yaml.safe_load(f) or {}
    mf_cfg = cfg.get("manifest", {})

    files: List[Dict] = []
    for name in TRACKED:
        path = os.path.join(args.data_dir, name)
        if not os.path.exists(path):
            continue
        files.append(
            {
                "name": name,
                "sha256": sha256_of(path),
                "records": json_count(path),
            }
        )

    manifest = {
        "generated_at": dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "generator_version": mf_cfg.get("generator_version", "0.1.0"),
        "source_versions": mf_cfg.get("source_versions", {}),
        "source_tag": args.source_tag,
        "files": files,
    }

    os.makedirs(os.path.dirname(args.out_file), exist_ok=True)
    with open(args.out_file, "w", encoding="utf-8") as f:
        json.dump(manifest, f, ensure_ascii=False, indent=2)
        f.write("\n")

    if args.verbose:
        print(f"generated {args.out_file}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
