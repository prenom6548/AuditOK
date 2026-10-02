#!/usr/bin/env python3
"""Apply UKUI theme-related GSettings changes transactionally."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from typing import Any


RGB_RE = re.compile(r"^#([0-9A-Fa-f]{6})$")


@dataclass(frozen=True)
class SettingChange:
    schema: str
    key: str
    value: str


def run_gsettings(args: list[str]) -> tuple[int, str, str]:
    proc = subprocess.run(["gsettings", *args], text=True, capture_output=True)
    return proc.returncode, proc.stdout.strip(), proc.stderr.strip()


def get_value(schema: str, key: str) -> tuple[bool, str]:
    rc, out, err = run_gsettings(["get", schema, key])
    return rc == 0, out if rc == 0 else err


def set_value(schema: str, key: str, value: str) -> tuple[bool, str]:
    rc, out, err = run_gsettings(["set", schema, key, value])
    return rc == 0, out if rc == 0 else err


def expected_get_value(value: str) -> str:
    if value in {"true", "false"}:
        return value
    if re.fullmatch(r"-?\d+(\.\d+)?", value):
        return value
    if (value.startswith("'") and value.endswith("'")) or (value.startswith('"') and value.endswith('"')):
        return value
    return "'" + value.replace("\\", "\\\\").replace("'", "\\'") + "'"


def rollback(old_values: list[tuple[SettingChange, str]]) -> list[str]:
    errors: list[str] = []
    for change, old in reversed(old_values):
        ok, detail = set_value(change.schema, change.key, old)
        if not ok:
            errors.append(f"{change.schema}.{change.key}: {detail}")
    return errors


def apply_transaction(operation: str, changes: list[SettingChange]) -> dict[str, Any]:
    old_values: list[tuple[SettingChange, str]] = []

    for change in changes:
        ok, old = get_value(change.schema, change.key)
        if not ok:
            return {"ok": False, "operation": operation, "stage": "read", "error": f"{change.schema}.{change.key}: {old}"}
        old_values.append((change, old))

    applied: list[dict[str, str]] = []
    for change in changes:
        ok, detail = set_value(change.schema, change.key, change.value)
        if not ok:
            return {
                "ok": False,
                "operation": operation,
                "stage": "write",
                "error": f"{change.schema}.{change.key}: {detail}",
                "rollback_errors": rollback(old_values),
            }
        applied.append({"schema": change.schema, "key": change.key, "value": change.value})

    verify_errors: list[str] = []
    for change in changes:
        ok, current = get_value(change.schema, change.key)
        expected = expected_get_value(change.value)
        if not ok:
            verify_errors.append(f"{change.schema}.{change.key}: {current}")
        elif current != expected:
            verify_errors.append(f"{change.schema}.{change.key}: expected {expected}, got {current}")

    if verify_errors:
        return {
            "ok": False,
            "operation": operation,
            "stage": "verify",
            "error": "; ".join(verify_errors),
            "rollback_errors": rollback(old_values),
        }

    return {"ok": True, "operation": operation, "applied": applied}


def normalize_color(value: str) -> str:
    match = RGB_RE.fullmatch(value.strip())
    if not match:
        raise ValueError("accent color must use #RRGGBB")
    return "#" + match.group(1).upper()


def mode_changes(mode: str) -> list[SettingChange]:
    if mode == "dark":
        return [
            SettingChange("org.ukui.SettingsDaemon.plugins.color", "theme-schedule-automatic", "false"),
            SettingChange("org.ukui.style", "style-name", "ukui-dark"),
            SettingChange("org.mate.interface", "gtk-theme", "ukui-black"),
        ]
    if mode == "light":
        return [
            SettingChange("org.ukui.SettingsDaemon.plugins.color", "theme-schedule-automatic", "false"),
            SettingChange("org.ukui.style", "style-name", "ukui-light"),
            SettingChange("org.mate.interface", "gtk-theme", "ukui-white"),
        ]
    if mode == "auto":
        return [SettingChange("org.ukui.SettingsDaemon.plugins.color", "theme-schedule-automatic", "true")]
    raise ValueError(f"unsupported mode: {mode}")


def accent_changes(color: str) -> list[SettingChange]:
    value = normalize_color(color)
    return [
        SettingChange("org.ukui.style", "theme-color", value),
        SettingChange("org.ukui.style", "custom-highlight-color", value),
        SettingChange("org.ukui.style", "use-custom-highlight-color", "true"),
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description="Apply UKUI theme settings transactionally.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    mode_parser = subparsers.add_parser("mode", help="Set dark, light, or auto theme mode")
    mode_parser.add_argument("mode", choices=["dark", "light", "auto"])

    accent_parser = subparsers.add_parser("accent", help="Set accent color")
    accent_parser.add_argument("color", help="Accent color in #RRGGBB")

    args = parser.parse_args()
    try:
        if args.command == "mode":
            operation = f"theme.mode.{args.mode}"
            result = apply_transaction(operation, mode_changes(args.mode))
        else:
            color = normalize_color(args.color)
            result = apply_transaction("theme.accent", accent_changes(color))
    except ValueError as exc:
        result = {"ok": False, "stage": "parse", "error": str(exc)}

    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result.get("ok") else 1


if __name__ == "__main__":
    raise SystemExit(main())
