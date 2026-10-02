#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import json
import os
import sys
from typing import Any, List, Tuple


TARGETS: List[Tuple[str, str, bool]] = [
    ("iso_3166_1.json", "iso_3166_1.schema.json", True),
    ("iso_3166_2.json", "iso_3166_2.schema.json", True),
    ("timezone_coords.json", "timezone_coords.schema.json", True),

    # Fine-parity artifacts: in strong CI mode we require them.
    ("timezone_country_map.json", "timezone_country_map.schema.json", True),
    ("spatial_index_properties.json", "spatial_index_properties.schema.json", True),
    ("country_timezone_map.json", "country_timezone_map.schema.json", True),
    ("country_timezones_from_subdiv.json", "country_timezones_from_subdiv.schema.json", True),
]


def load_json(path: str):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def _is_upper_alpha(s: Any, n: int) -> bool:
    return isinstance(s, str) and len(s) == n and s.isalpha() and s.upper() == s


def validate_iso_3166_1(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, list):
        return [f"{file_name}: must be an array"]
    required = {"alpha2", "alpha3", "name", "numeric", "currency", "emoji", "timezones"}
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            errs.append(f"{file_name}::{i}: item must be an object")
            continue
        unknown = set(item.keys()) - required
        missing = required - set(item.keys())
        if unknown:
            errs.append(f"{file_name}::{i}: unknown keys: {sorted(unknown)}")
        if missing:
            errs.append(f"{file_name}::{i}: missing keys: {sorted(missing)}")
        if not _is_upper_alpha(item.get("alpha2"), 2):
            errs.append(f"{file_name}::{i}.alpha2: must be 2 uppercase letters")
        if not _is_upper_alpha(item.get("alpha3"), 3):
            errs.append(f"{file_name}::{i}.alpha3: must be 3 uppercase letters")
        if not isinstance(item.get("name"), str):
            errs.append(f"{file_name}::{i}.name: must be string")
        numeric = item.get("numeric")
        if numeric is not None and (not isinstance(numeric, int) or numeric < 0 or numeric > 65535):
            errs.append(f"{file_name}::{i}.numeric: must be null or integer 0..65535")
        if item.get("currency") is not None and not isinstance(item.get("currency"), str):
            errs.append(f"{file_name}::{i}.currency: must be null or string")
        if item.get("emoji") is not None and not isinstance(item.get("emoji"), str):
            errs.append(f"{file_name}::{i}.emoji: must be null or string")
        tzs = item.get("timezones")
        if not isinstance(tzs, list) or any(not isinstance(t, str) for t in tzs):
            errs.append(f"{file_name}::{i}.timezones: must be array of strings")
    return errs


def validate_iso_3166_2(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, list):
        return [f"{file_name}: must be an array"]
    required = {"code", "name", "country_alpha2", "parent_code", "timezone_ids"}
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            errs.append(f"{file_name}::{i}: item must be an object")
            continue
        unknown = set(item.keys()) - required
        missing = required - set(item.keys())
        if unknown:
            errs.append(f"{file_name}::{i}: unknown keys: {sorted(unknown)}")
        if missing:
            errs.append(f"{file_name}::{i}: missing keys: {sorted(missing)}")
        code = item.get("code")
        if not isinstance(code, str) or "-" not in code or not _is_upper_alpha(code.split("-", 1)[0], 2):
            errs.append(f"{file_name}::{i}.code: must be like AA-...")
        if not isinstance(item.get("name"), str):
            errs.append(f"{file_name}::{i}.name: must be string")
        if not _is_upper_alpha(item.get("country_alpha2"), 2):
            errs.append(f"{file_name}::{i}.country_alpha2: must be 2 uppercase letters")
        parent = item.get("parent_code")
        if parent is not None and not isinstance(parent, str):
            errs.append(f"{file_name}::{i}.parent_code: must be null or string")
        tzs = item.get("timezone_ids")
        if not isinstance(tzs, list) or any(not isinstance(t, str) for t in tzs):
            errs.append(f"{file_name}::{i}.timezone_ids: must be array of strings")
    return errs


def validate_timezone_coords(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, list):
        return [f"{file_name}: must be an array"]
    required = {"lat", "lon", "timezone"}
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            errs.append(f"{file_name}::{i}: item must be an object")
            continue
        unknown = set(item.keys()) - required
        missing = required - set(item.keys())
        if unknown:
            errs.append(f"{file_name}::{i}: unknown keys: {sorted(unknown)}")
        if missing:
            errs.append(f"{file_name}::{i}: missing keys: {sorted(missing)}")
        lat = item.get("lat")
        lon = item.get("lon")
        tz = item.get("timezone")
        if not isinstance(lat, (int, float)) or lat < -90 or lat > 90:
            errs.append(f"{file_name}::{i}.lat: must be number in [-90, 90]")
        if not isinstance(lon, (int, float)) or lon < -180 or lon > 180:
            errs.append(f"{file_name}::{i}.lon: must be number in [-180, 180]")
        if not isinstance(tz, str) or not tz:
            errs.append(f"{file_name}::{i}.timezone: must be non-empty string")
    return errs


def validate_spatial_index_properties(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, dict):
        return [f"{file_name}: must be an object"]
    for key in ["tz_by_property", "country_by_property", "subdivision_by_property"]:
        v = data.get(key)
        if v is None:
            continue
        if not isinstance(v, list):
            errs.append(f"{file_name}.{key}: must be an array")
            continue
        for i, item in enumerate(v):
            if item is None:
                continue
            if not isinstance(item, str) or not item:
                errs.append(f"{file_name}.{key}::{i}: must be null or non-empty string")
    return errs


def validate_country_timezone_map(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, dict):
        return [f"{file_name}: must be an object"]
    for k, v in data.items():
        if not isinstance(k, str):
            errs.append(f"{file_name}: object key must be string")
            continue
        if not isinstance(v, str) or not v:
            errs.append(f"{file_name}[{k}]: value must be non-empty string")
    return errs


def validate_country_timezones_from_subdiv(data: Any, file_name: str) -> List[str]:
    errs: List[str] = []
    if not isinstance(data, dict):
        return [f"{file_name}: must be an object"]
    for k, v in data.items():
        if not isinstance(k, str):
            errs.append(f"{file_name}: object key must be string")
            continue
        if not isinstance(v, list) or any((not isinstance(t, str) or not t) for t in v):
            errs.append(f"{file_name}[{k}]: must be array of non-empty strings")
    return errs


def validate_fine_parity_semantics(data_dir: str) -> List[str]:
    """
    Semantic checks (beyond schema) for fine-parity locale data.
    These checks are intentionally best-effort: if some files are missing,
    we skip them (to keep minimal environments workable).
    """
    errs: List[str] = []

    def load_json(path: str) -> Any:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)

    # 1) spatial_index_properties.json: sanity of first entry (Undefined => null)
    p_spatial = os.path.join(data_dir, "spatial_index_properties.json")
    if os.path.exists(p_spatial):
        try:
            v = load_json(p_spatial)
            tz_arr = v.get("tz_by_property")
            cc_arr = v.get("country_by_property")
            if not isinstance(tz_arr, list) or not isinstance(cc_arr, list):
                errs.append("spatial_index_properties.json: missing arrays")
            else:
                if len(tz_arr) == 0 or len(cc_arr) == 0:
                    errs.append("spatial_index_properties.json: empty arrays")
                else:
                    if tz_arr[0] is not None:
                        errs.append("spatial_index_properties.json: tz_by_property[0] must be null")
                    if cc_arr[0] is not None:
                        errs.append(
                            "spatial_index_properties.json: country_by_property[0] must be null"
                        )
                    sub_arr = v.get("subdivision_by_property")
                    if isinstance(sub_arr, list) and len(sub_arr) != len(tz_arr):
                        errs.append(
                            "spatial_index_properties.json: subdivision_by_property length must match tz_by_property"
                        )
        except Exception as e:
            errs.append(f"spatial_index_properties.json: failed to parse/validate: {e}")

    # 2) timezone_country_map.json: ambiguity holes + a couple known keys
    p_tz_country = os.path.join(data_dir, "timezone_country_map.json")
    if os.path.exists(p_tz_country):
        try:
            m = load_json(p_tz_country)
            if not isinstance(m, dict):
                errs.append("timezone_country_map.json: must be an object")
            else:
                # Ambiguous mapping must be missing (ki18n hole semantics).
                if "Asia/Bangkok" in m:
                    errs.append("timezone_country_map.json: Asia/Bangkok must be absent")
                # Known 1:1 mapping
                if m.get("Europe/Brussels") != "BE":
                    errs.append("timezone_country_map.json: Europe/Brussels must map to BE")
        except Exception as e:
            errs.append(f"timezone_country_map.json: failed to parse/validate: {e}")

    # 3) country_timezone_map.json + country_timezones_from_subdiv.json: key order/exclusions
    p_country_tz = os.path.join(data_dir, "country_timezone_map.json")
    p_country_tz_from_subdiv = os.path.join(data_dir, "country_timezones_from_subdiv.json")
    if os.path.exists(p_country_tz) and os.path.exists(p_country_tz_from_subdiv):
        try:
            m1 = load_json(p_country_tz)
            m2 = load_json(p_country_tz_from_subdiv)
            if not isinstance(m1, dict) or not isinstance(m2, dict):
                errs.append("country timezone json: must be objects")
            else:
                if m1.get("DE") != "Europe/Berlin":
                    errs.append("country_timezone_map.json: DE must map to Europe/Berlin")
                if m1.get("BE") != "Europe/Brussels":
                    errs.append("country_timezone_map.json: BE must map to Europe/Brussels")

                # ES should be handled via subdivisions (so primary map should not contain it)
                if "ES" in m1:
                    errs.append("country_timezone_map.json: ES must be absent (subdiv-driven)")

                es_list = m2.get("ES")
                if es_list != ["Europe/Madrid", "Africa/Ceuta", "Atlantic/Canary"]:
                    errs.append("country_timezones_from_subdiv.json: ES list order mismatch")
        except Exception as e:
            errs.append(f"country timezone json: failed semantic validation: {e}")

    return errs


def validate_one(data_path: str, schema_path: str, file_name: str) -> List[str]:
    data = load_json(data_path)
    # Keep schema file existence contract in place for pipeline consistency.
    if not os.path.exists(schema_path):
        return [f"missing schema file: {schema_path}"]
    if file_name == "iso_3166_1.json":
        return validate_iso_3166_1(data, file_name)
    if file_name == "iso_3166_2.json":
        return validate_iso_3166_2(data, file_name)
    if file_name == "timezone_coords.json":
        return validate_timezone_coords(data, file_name)
    if file_name == "timezone_country_map.json":
        # same type shape as other country maps: object string -> non-empty string
        return validate_country_timezone_map(data, file_name)
    if file_name == "spatial_index_properties.json":
        return validate_spatial_index_properties(data, file_name)
    if file_name == "country_timezone_map.json":
        return validate_country_timezone_map(data, file_name)
    if file_name == "country_timezones_from_subdiv.json":
        return validate_country_timezones_from_subdiv(data, file_name)
    return [f"{file_name}: no validator implemented"]


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate generated locale JSON files")
    parser.add_argument("--schema-dir", required=True, help="Schema directory path")
    parser.add_argument("--data-dir", required=True, help="Generated data directory path")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    missing: List[str] = []
    all_errors: List[str] = []
    for data_name, schema_name, required in TARGETS:
        data_path = os.path.join(args.data_dir, data_name)
        schema_path = os.path.join(args.schema_dir, schema_name)
        if not os.path.exists(data_path):
            if required:
                missing.append(data_path)
            continue
        if not os.path.exists(schema_path):
            if required:
                missing.append(schema_path)
            continue
        errs = validate_one(data_path, schema_path, data_name)
        all_errors.extend(errs)
        if args.verbose and not errs:
            print(f"validated {data_name}")

    # Fine-parity semantic checks across multiple files.
    all_errors.extend(validate_fine_parity_semantics(args.data_dir))

    if missing:
        for p in missing:
            print(f"missing required file: {p}", file=sys.stderr)
        return 2

    if all_errors:
        for msg in all_errors:
            print(msg, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
