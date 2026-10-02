#!/usr/bin/env python3
"""Smoke-test the UKUI command catalog on a target openKylin machine.

Default mode is non-destructive: it checks parser behavior, required binaries,
and gsettings schema/key existence. Use --execute-safe to run a small reversible
subset of commands and restore previous values.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import resolve  # noqa: E402


SKILL_DIR = Path(__file__).resolve().parents[1]
CATALOG = SKILL_DIR / "references" / "intent_catalog.json"

SAMPLE_SLOTS = {
    "value": "50",
    "size": "medium",
    "size_value": "36",
    "type": "lunar",
    "day": "monday",
    "format": "24",
    "language": "zh_CN",
    "scheme": "1",
    "path": "/tmp/ukui-skill-smoke.png",
    "days": "7",
    "color": "'#2A88FE'",
    "color_value": "#2A88FE",
    "skill_dir": shlex.quote(str(SKILL_DIR)),
}

SAFE_REVERSIBLE = {
    "bluetooth.tray.hide": ("gsettings get org.ukui.bluetooth tray-show", "gsettings set org.ukui.bluetooth tray-show {old}"),
    "mouse.cursor.size": ("gsettings get org.ukui.peripherals-mouse cursor-size", "gsettings set org.ukui.peripherals-mouse cursor-size {old}"),
    "mouse.accel.off": ("gsettings get org.ukui.peripherals-mouse mouse-accel", "gsettings set org.ukui.peripherals-mouse mouse-accel {old}"),
    "mouse.locate_pointer.on": ("gsettings get org.ukui.peripherals-mouse locate-pointer", "gsettings set org.ukui.peripherals-mouse locate-pointer {old}"),
    "keyboard.cursor_blink.off": ("gsettings get org.mate.interface cursor-blink", "gsettings set org.mate.interface cursor-blink {old}"),
    "keyboard.repeat_keys.on": ("gsettings get org.ukui.peripherals-keyboard repeat", "gsettings set org.ukui.peripherals-keyboard repeat {old}"),
    "keyboard.repeat_notification.on": ("gsettings get org.ukui.control-center.osd show-lock-tip", "gsettings set org.ukui.control-center.osd show-lock-tip {old}"),
    "locale.time_format": ("gsettings get org.ukui.control-center.panel.plugins hoursystem", "gsettings set org.ukui.control-center.panel.plugins hoursystem {old}"),
    "calendar.firstday": ("gsettings get org.ukui.control-center.panel.plugins firstday", "gsettings set org.ukui.control-center.panel.plugins firstday {old}"),
    "locale.date_format": ("gsettings get org.ukui.control-center.panel.plugins date", "gsettings set org.ukui.control-center.panel.plugins date {old}"),
    "theme.dark": [
        ("gsettings get org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic", "gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic {old}"),
        ("gsettings get org.ukui.style style-name", "gsettings set org.ukui.style style-name {old}"),
        ("gsettings get org.mate.interface gtk-theme", "gsettings set org.mate.interface gtk-theme {old}"),
    ],
    "theme.accent": [
        ("gsettings get org.ukui.style theme-color", "gsettings set org.ukui.style theme-color {old}"),
        ("gsettings get org.ukui.style custom-highlight-color", "gsettings set org.ukui.style custom-highlight-color {old}"),
        ("gsettings get org.ukui.style use-custom-highlight-color", "gsettings set org.ukui.style use-custom-highlight-color {old}"),
    ],
    "brightness.set": ("gsettings get org.ukui.power-manager brightness-ac", "gsettings set org.ukui.power-manager brightness-ac {old}"),
    "security.password_on_sleep.on": ("gsettings get org.ukui.screensaver sleep-activation-enabled", "gsettings set org.ukui.screensaver sleep-activation-enabled {old}"),
}


def run(command: str, timeout: int = 10) -> tuple[int, str, str]:
    env = os.environ.copy()
    env.setdefault("DISPLAY", ":0")
    uid = os.getuid() if hasattr(os, "getuid") else None
    if uid is not None:
        env.setdefault("XDG_RUNTIME_DIR", f"/run/user/{uid}")
        env.setdefault("DBUS_SESSION_BUS_ADDRESS", f"unix:path=/run/user/{uid}/bus")
    proc = subprocess.run(command, shell=True, text=True, capture_output=True, timeout=timeout, env=env)
    return proc.returncode, proc.stdout.strip(), proc.stderr.strip()


def load_catalog() -> list[dict[str, object]]:
    return json.loads(CATALOG.read_text(encoding="utf-8"))


def render_command(command: str) -> str:
    try:
        return command.format(**SAMPLE_SLOTS)
    except Exception:
        return command


def binary_name(command: str) -> str | None:
    if not command:
        return None
    try:
        return shlex.split(command)[0]
    except ValueError:
        return command.split()[0]


def binary_names(command: str) -> set[str]:
    names: set[str] = set()
    if re.search(r"\bgtk-launch\b", command):
        names.add("gtk-launch")
    if re.search(r"\bpython3\b", command):
        names.add("python3")
    first = binary_name(command)
    if first and "=" not in first and first not in {"command", "test", "[", "("}:
        names.add(first)
    return names


def transaction_steps(item: dict[str, object]) -> list[dict[str, str]]:
    rendered = []
    for step in item.get("transaction") or []:
        if not isinstance(step, dict):
            continue
        value = str(step.get("value", ""))
        try:
            value = value.format(**SAMPLE_SLOTS)
        except Exception:
            pass
        rendered.append({"schema": str(step.get("schema", "")), "key": str(step.get("key", "")), "value": value})
    return rendered


def check_binaries(catalog: list[dict[str, object]]) -> list[str]:
    failures: list[str] = []
    binaries = set()
    for item in catalog:
        command = render_command(str(item.get("command", "")))
        if item.get("route") == "actuator":
            binaries.add("kylin-actuator")
        elif command:
            binaries.update(binary_names(command))
    for name in sorted(binaries):
        rc, out, _ = run(f"command -v {shlex.quote(name)}")
        if rc == 0:
            print(f"BIN OK   {name} -> {out}")
        else:
            print(f"BIN BAD  {name}")
            failures.append(name)
    return failures


def check_gsettings(catalog: list[dict[str, object]]) -> list[str]:
    failures: list[str] = []
    for item in catalog:
        command = render_command(str(item.get("command", "")))
        ident = str(item.get("id"))
        pairs = [(step["schema"], step["key"]) for step in transaction_steps(item)]
        pairs += [(match.group(1), match.group(2)) for match in re.finditer(r"gsettings\s+set\s+(\S+)\s+(\S+)\s+", command)]
        for schema, key in pairs:
            rc, out, err = run(f"gsettings writable {schema} {key}")
            if rc == 0:
                print(f"GSET OK  {ident} {schema} {key} writable={out}")
            else:
                print(f"GSET BAD {ident} {schema} {key} {err}")
                failures.append(ident)
    return failures


def check_parse(catalog: list[dict[str, object]]) -> list[str]:
    failures: list[str] = []
    for item in catalog:
        patterns = item.get("patterns") or []
        if not patterns:
            continue
        intent = str(patterns[0])
        result = resolve.resolve(intent, use_csv_fallback=False)
        if result.get("id") == item.get("id"):
            print(f"PARSE OK {item.get('id')} <- {intent}")
        else:
            print(f"PARSE BAD {item.get('id')} <- {intent}; got {result.get('id')}")
            failures.append(str(item.get("id")))
    return failures


def execute_safe(catalog: list[dict[str, object]]) -> list[str]:
    failures: list[str] = []
    by_id = {str(item.get("id")): item for item in catalog}
    for ident, checks in SAFE_REVERSIBLE.items():
        item = by_id.get(ident)
        if not item:
            continue
        if isinstance(checks, tuple):
            checks = [checks]
        command = render_command(str(item.get("command", "")))
        old_values: list[tuple[str, str]] = []
        failed = False
        for get_cmd, restore_template in checks:
            rc, old, err = run(get_cmd)
            if rc != 0:
                print(f"EXEC BAD {ident} get failed: {err}")
                failures.append(ident)
                failed = True
                break
            old_values.append((old, restore_template))
        if failed:
            continue
        rc, _, err = run(command)
        if rc != 0:
            print(f"EXEC BAD {ident} command failed: {err}")
            failures.append(ident)
            continue
        for old, restore_template in reversed(old_values):
            restore = restore_template.format(old=old)
            rc, _, err = run(restore)
            if rc != 0:
                print(f"EXEC BAD {ident} restore failed: {err}")
                failures.append(ident)
                failed = True
                break
        if failed:
            continue
        print(f"EXEC OK  {ident}")
    return failures


def check_actuator_help() -> list[str]:
    rc, out, err = run("timeout 8s kylin-actuator '{show versioninfo}'", timeout=12)
    if rc == 0 and "<AI>" in out:
        print("ACT OK   kylin-actuator '{show versioninfo}'")
        return []
    print(f"ACT BAD  rc={rc} out={out} err={err}")
    return ["kylin-actuator"]


def main() -> int:
    parser = argparse.ArgumentParser(description="Smoke-test the UKUI direct-control skill.")
    parser.add_argument("--execute-safe", action="store_true", help="Run reversible low-risk commands and restore old values")
    parser.add_argument("--skip-actuator", action="store_true", help="Skip the kylin-actuator smoke test")
    args = parser.parse_args()

    catalog = load_catalog()
    failures: list[str] = []
    failures += check_parse(catalog)
    failures += check_binaries(catalog)
    failures += check_gsettings(catalog)
    if not args.skip_actuator:
        failures += check_actuator_help()
    if args.execute_safe:
        failures += execute_safe(catalog)

    if failures:
        print(f"\nFAIL {len(failures)} issue(s): {', '.join(failures)}")
        return 1
    print("\nPASS smoke test")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
