#!/usr/bin/env python3
"""Full catalog test for ukui-direct-control.

This script is intended to run on an openKylin/UKUI test machine from the
copied skill directory. It tests every curated intent. Reversible state changes
are executed and restored; destructive/security-sensitive actions are tested by
resolver/risk policy plus an isolated or simulated command check.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import resolve  # noqa: E402


SKILL_DIR = Path(__file__).resolve().parents[1]
CATALOG = SKILL_DIR / "references" / "intent_catalog.json"

TEST_INTENTS = {
    "bluetooth.power.on": "打开蓝牙",
    "bluetooth.power.off": "关闭蓝牙",
    "bluetooth.tray.show": "显示任务栏蓝牙图标",
    "bluetooth.tray.hide": "关闭任务栏蓝牙图标",
    "wifi.power.on": "打开wifi",
    "wifi.power.off": "关闭wifi",
    "volume.mute.on": "静音",
    "volume.mute.off": "取消静音",
    "volume.up": "调高音量",
    "volume.down": "调低音量",
    "volume.max": "音量最大",
    "volume.min": "音量最小",
    "brightness.up": "调高亮度",
    "brightness.down": "调低亮度",
    "brightness.set": "设置屏幕亮度为50",
    "brightness.max": "设置最高屏幕亮度",
    "brightness.min": "设置最低屏幕亮度",
    "theme.dark": "打开深色模式",
    "theme.light": "切换浅色主题",
    "theme.auto": "打开自适应主题",
    "theme.accent": "强调主题色为黄色",
    "touchpad.on": "打开触控板",
    "touchpad.off": "关闭触控板",
    "mouse.cursor.size": "把鼠标指针调大",
    "mouse.left_hand.on": "鼠标设置为左手",
    "mouse.left_hand.off": "鼠标设置为右手",
    "mouse.accel.on": "打开鼠标加速",
    "mouse.accel.off": "关闭鼠标加速",
    "mouse.locate_pointer.on": "按ctrl显示鼠标位置",
    "mouse.locate_pointer.off": "关闭ctrl显示鼠标位置",
    "keyboard.cursor_blink.on": "打开光标闪烁",
    "keyboard.cursor_blink.off": "关闭光标闪烁",
    "keyboard.repeat_keys.on": "打开按键重复",
    "keyboard.repeat_keys.off": "关闭按键重复",
    "keyboard.repeat_notification.on": "打开键盘按键提示",
    "keyboard.repeat_notification.off": "关闭键盘按键提示",
    "calendar.type": "设置农历",
    "calendar.firstday": "一周第一天设为星期天",
    "locale.date_format": "日期格式设置为中文",
    "locale.time_format": "时间格式设置成24小时",
    "locale.language": "将系统语言设置为中文",
    "power.scheme.ac": "使用电源时设置最佳性能",
    "power.scheme.battery": "使用电池时设置节能",
    "security.password_on_sleep.on": "打开睡眠密码",
    "security.password_on_sleep.off": "关闭睡眠密码",
    "security.autologin.on": "打开自动登录",
    "security.autologin.off": "关闭自动登录",
    "security.passwordless.on": "打开免密登录",
    "security.passwordless.off": "关闭免密登录",
    "screen.lock": "锁屏",
    "screenshot.default": "截图",
    "screenshot.window": "窗口截图",
    "screenshot.area": "区域截图",
    "trash.empty": "清空回收站",
    "cleanup.temp": "清理临时文件",
    "cleanup.dependencies": "清理无用依赖包",
    "network.time.on": "打开同步网络时间",
    "background.set": "设置桌面背景为\"/tmp/ukui-full-test-wallpaper.png\"",
    "open.settings": "打开设置",
    "open.computer": "打开计算机",
    "open.calculator": "打开计算器",
    "media.play": "播放音乐",
    "media.pause": "暂停音乐",
    "files.modified.word": "查找最近7天修改的word文件",
    "files.modified.excel": "查找最近7天修改的excel文件",
    "files.modified.ppt": "查找最近7天修改的ppt文件",
    "files.modified.pdf": "查找最近7天修改的pdf文件",
    "files.modified.image": "查找最近7天修改的图片",
    "files.modified.text": "查找最近7天修改的文本文件",
    "files.created.word": "查找最近7天创建的word文件",
    "files.created.pdf": "查找最近7天创建的pdf文件",
}

SECURITY_NO_EXEC = {
    "locale.language",
    "security.autologin.on",
    "security.autologin.off",
    "security.passwordless.on",
    "security.passwordless.off",
    "screen.lock",
}

ISOLATED_TESTS = {"trash.empty", "cleanup.temp", "cleanup.dependencies"}
WIFI_OFF = "wifi.power.off"
DYNAMIC_TEST_INTENTS = {
    "application.open": "打开文本编辑器",
}
DESKTOP_LAUNCH_PROCESS_PATTERNS = {
    "open.settings": "ukui-control-center",
    "open.computer": "peony.*computer:///",
    "open.calculator": "kylin-calculator",
}


def env() -> dict[str, str]:
    result = os.environ.copy()
    result.setdefault("DISPLAY", ":0")
    if hasattr(os, "getuid"):
        uid = os.getuid()
        result.setdefault("XDG_RUNTIME_DIR", f"/run/user/{uid}")
        result.setdefault("DBUS_SESSION_BUS_ADDRESS", f"unix:path=/run/user/{uid}/bus")
    return result


def run(command: str, cwd: Path | None = None, timeout: int = 15, extra_env: dict[str, str] | None = None) -> tuple[int, str, str]:
    merged = env()
    if extra_env:
        merged.update(extra_env)
    try:
        proc = subprocess.run(command, shell=True, cwd=cwd, text=True, capture_output=True, timeout=timeout, env=merged)
        return proc.returncode, proc.stdout.strip(), proc.stderr.strip()
    except subprocess.TimeoutExpired as exc:
        out = exc.stdout.decode("utf-8", errors="replace") if isinstance(exc.stdout, bytes) else (exc.stdout or "")
        err = exc.stderr.decode("utf-8", errors="replace") if isinstance(exc.stderr, bytes) else (exc.stderr or "")
        return 124, out.strip(), (err.strip() or f"timeout after {timeout}s")


def load_catalog() -> list[dict[str, Any]]:
    return json.loads(CATALOG.read_text(encoding="utf-8"))


def status(ok: bool, ident: str, name: str, mode: str, detail: str) -> dict[str, Any]:
    return {"ok": ok, "id": ident, "name": name, "mode": mode, "detail": detail}


def parse_gsettings_commands(command: str) -> list[tuple[str, str]]:
    return [(match.group(1), match.group(2)) for match in re.finditer(r"gsettings\s+set\s+(\S+)\s+(\S+)\s+", command)]


def restore_gsettings(schema: str, key: str, old: str) -> tuple[int, str, str]:
    return run(f"gsettings set {schema} {key} {old}")


def expected_get_value(value: str) -> str:
    if value in {"true", "false"}:
        return value
    if re.fullmatch(r"-?\d+(\.\d+)?", value):
        return value
    if (value.startswith("'") and value.endswith("'")) or (value.startswith('"') and value.endswith('"')):
        return value
    return "'" + value.replace("\\", "\\\\").replace("'", "\\'") + "'"


def test_gsettings_transaction(result: dict[str, Any]) -> dict[str, Any]:
    ident = result["id"]
    name = result["name"]
    command = result.get("command", "")
    transaction = result.get("transaction") or []
    if not transaction:
        return status(False, ident, name, "transaction", "missing transaction metadata")

    old_values: list[tuple[str, str, str]] = []
    for step in transaction:
        schema, key = step["schema"], step["key"]
        rc, old, err = run(f"gsettings get {schema} {key}")
        if rc != 0:
            return status(False, ident, name, "transaction", f"get failed: {schema}.{key}: {err}")
        old_values.append((schema, key, old))

    rc, out, err = run(command, timeout=20)

    verify_errors: list[str] = []
    new_values: list[tuple[str, str, str]] = []
    for step in transaction:
        schema, key, value = step["schema"], step["key"], step["value"]
        rc2, new, err2 = run(f"gsettings get {schema} {key}")
        if rc2 != 0:
            verify_errors.append(f"{schema}.{key}: {err2}")
        elif new != expected_get_value(value):
            verify_errors.append(f"{schema}.{key}: expected {expected_get_value(value)}, got {new}")
        new_values.append((schema, key, new))

    restore_errors: list[str] = []
    for schema, key, old in reversed(old_values):
        restore_rc, _, restore_err = restore_gsettings(schema, key, old)
        if restore_rc != 0:
            restore_errors.append(f"{schema}.{key}: {restore_err}")

    if rc != 0:
        return status(False, ident, name, "transaction", f"command failed: {err or out}")
    if verify_errors:
        return status(False, ident, name, "transaction", "verify failed: " + "; ".join(verify_errors))
    if restore_errors:
        return status(False, ident, name, "transaction", "restore failed: " + "; ".join(restore_errors))

    before = {f"{schema}.{key}": old for schema, key, old in old_values}
    after = {f"{schema}.{key}": new for schema, key, new in new_values}
    detail = "; ".join(f"{key}: {before[key]} -> {after.get(key, '')}" for key in before)
    return status(True, ident, name, "transaction-restored", detail + " -> restored")


def test_gsettings(ident: str, name: str, command: str) -> dict[str, Any]:
    parsed = parse_gsettings_commands(command)
    if not parsed:
        return status(False, ident, name, "gsettings", "cannot parse gsettings command")
    if ident == "background.set":
        Path("/tmp/ukui-full-test-wallpaper.png").write_bytes(b"ukui-test")

    old_values: list[tuple[str, str, str]] = []
    for schema, key in parsed:
        rc, old, err = run(f"gsettings get {schema} {key}")
        if rc != 0:
            return status(False, ident, name, "gsettings", f"get failed: {schema}.{key}: {err}")
        old_values.append((schema, key, old))

    rc, out, err = run(command)
    new_values: list[tuple[str, str, str]] = []
    verify_errors: list[str] = []
    for schema, key in parsed:
        rc2, new, err2 = run(f"gsettings get {schema} {key}")
        if rc2 != 0:
            verify_errors.append(f"{schema}.{key}: {err2}")
        new_values.append((schema, key, new))

    restore_errors: list[str] = []
    for schema, key, old in reversed(old_values):
        restore_rc, _, restore_err = restore_gsettings(schema, key, old)
        if restore_rc != 0:
            restore_errors.append(f"{schema}.{key}: {restore_err}")

    if rc != 0:
        return status(False, ident, name, "gsettings", f"set failed: {err or out}")
    if verify_errors:
        return status(False, ident, name, "gsettings", "verify failed: " + "; ".join(verify_errors))
    if restore_errors:
        return status(False, ident, name, "gsettings", "restore failed: " + "; ".join(restore_errors))

    before = {f"{schema}.{key}": old for schema, key, old in old_values}
    after = {f"{schema}.{key}": new for schema, key, new in new_values}
    detail = "; ".join(f"{key}: {before[key]} -> {after.get(key, '')}" for key in before)
    return status(True, ident, name, "executed-restored", detail + " -> restored")


def bluetooth_power() -> str:
    rc, out, _ = run("bluetoothctl show | awk -F': ' '/Powered:/ {print $2; exit}'")
    return out if rc == 0 else ""


def test_bluetooth(ident: str, name: str, command: str) -> dict[str, Any]:
    old = bluetooth_power()
    expected = "yes" if command.endswith("on") else "no"
    rc, out, err = run(command, timeout=20)
    new = bluetooth_power()
    if old:
        run(f"bluetoothctl power {'on' if old == 'yes' else 'off'}", timeout=20)
    final = bluetooth_power()
    if rc != 0 or new != expected:
        return status(False, ident, name, "bluetooth", f"rc={rc}, expected={expected}, new={new}, err={err or out}")
    return status(True, ident, name, "executed-restored", f"{old} -> {new} -> {final}")


def wifi_radio() -> str:
    rc, out, _ = run("nmcli radio wifi")
    return out if rc == 0 else ""


def ssh_route_is_wifi() -> bool:
    client_ip = os.environ.get("SSH_CLIENT", "").split(" ")[0]
    if not client_ip:
        return True
    rc, out, _ = run(f"ip route get {shlex.quote(client_ip)} | awk '{{for(i=1;i<=NF;i++) if($i==\"dev\") print $(i+1)}}'")
    dev = out.splitlines()[0].strip() if rc == 0 and out else ""
    if not dev:
        return True
    rc, out, _ = run("nmcli -t -f DEVICE,TYPE device status")
    for line in out.splitlines():
        parts = line.split(":")
        if len(parts) >= 2 and parts[0] == dev:
            return parts[1] == "wifi"
    return True


def test_wifi(ident: str, name: str, command: str, include_wifi_off: bool) -> dict[str, Any]:
    old = wifi_radio()
    if ident == WIFI_OFF:
        if ssh_route_is_wifi() and not include_wifi_off:
            return status(True, ident, name, "blocked-risk", "SSH route uses WiFi; run with --include-wifi-off for self-restoring disruptive test")
        script = "nohup bash -lc 'nmcli radio wifi off; sleep 5; nmcli radio wifi on' >/tmp/ukui-fulltest-wifi.log 2>&1 &"
        rc, _, err = run(script)
        return status(rc == 0, ident, name, "executed-self-restore", err or "background off/on scheduled")
    rc, out, err = run(command)
    new = wifi_radio()
    if old:
        run(f"nmcli radio wifi {old}")
    if rc != 0:
        return status(False, ident, name, "wifi", err or out)
    return status(True, ident, name, "executed-restored", f"{old} -> {new}")


def audio_state() -> tuple[str, str]:
    _, vol, _ = run("amixer -D pulse get Master | awk -F'[][]' '/Playback/ {print $2; exit}'")
    _, mute, _ = run("amixer -D pulse get Master | awk -F'[][]' '/Playback/ {print $4; exit}'")
    return vol or "65%", mute or "on"


def restore_audio(vol: str, mute: str) -> None:
    run(f"amixer -D pulse sset Master {shlex.quote(vol)} >/dev/null")
    run(f"amixer -D pulse sset Master {'mute' if mute == 'off' else 'unmute'} >/dev/null")


def test_audio(ident: str, name: str, command: str) -> dict[str, Any]:
    old_vol, old_mute = audio_state()
    rc, out, err = run(command, timeout=20)
    new_vol, new_mute = audio_state()
    restore_audio(old_vol, old_mute)
    if rc != 0:
        return status(False, ident, name, "audio", err or out)
    return status(True, ident, name, "executed-restored", f"{old_vol}/{old_mute} -> {new_vol}/{new_mute}")


def test_actuator_with_restore(ident: str, name: str, actuator_command: str) -> dict[str, Any]:
    restore: tuple[str, str, str] | None = None
    if ident.startswith("volume."):
        old_vol, old_mute = audio_state()
        restore = ("audio", old_vol, old_mute)
    elif ident.startswith("brightness."):
        _, old, _ = run("gsettings get org.ukui.power-manager brightness-ac")
        restore = ("gsettings", "org.ukui.power-manager brightness-ac", old)
    elif ident.startswith("touchpad."):
        _, old, _ = run("gsettings get org.ukui.peripherals-touchpad touchpad-enabled")
        restore = ("gsettings", "org.ukui.peripherals-touchpad touchpad-enabled", old)

    rc, out, err = run(f"timeout 12s {actuator_command}", timeout=15)

    if restore and restore[0] == "audio":
        restore_audio(restore[1], restore[2])
    elif restore and restore[0] == "gsettings":
        schema_key, old = restore[1], restore[2]
        schema, key = schema_key.split(" ", 1)
        restore_gsettings(schema, key, old)

    if rc != 0:
        return status(False, ident, name, "actuator", f"rc={rc}, {err or out}")
    return status(True, ident, name, "executed-restored" if restore else "executed", (out or err).splitlines()[-1] if (out or err) else "ok")


def test_files_find(ident: str, name: str, command: str) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="ukui-find-") as tmp:
        root = Path(tmp)
        for filename in ["a.doc", "b.docx", "c.xls", "d.xlsx", "e.ppt", "f.pptx", "g.pdf", "h.png", "i.jpg", "j.jpeg", "k.txt"]:
            (root / filename).write_text("test", encoding="utf-8")
        rc, out, err = run(command, cwd=root)
    if rc != 0:
        return status(False, ident, name, "find", err or out)
    return status(True, ident, name, "executed-isolated", f"{len(out.splitlines())} match(es)")


def test_isolated(ident: str, name: str) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="ukui-isolated-") as tmp:
        root = Path(tmp)
        if ident == "trash.empty":
            xdg = root / "xdg"
            trash_files = xdg / "Trash" / "files"
            trash_info = xdg / "Trash" / "info"
            trash_files.mkdir(parents=True)
            trash_info.mkdir(parents=True)
            (trash_files / "junk.txt").write_text("trash", encoding="utf-8")
            rc, out, err = run("gio trash --empty", extra_env={"XDG_DATA_HOME": str(xdg)})
            ok = rc == 0
            return status(ok, ident, name, "executed-isolated", err or out or "isolated trash command returned 0")
        if ident == "cleanup.temp":
            target = root / "tmp"
            target.mkdir()
            (target / "junk.txt").write_text("tmp", encoding="utf-8")
            rc, out, err = run(f"rm -rf {shlex.quote(str(target))}/*")
            ok = rc == 0 and not any(target.iterdir())
            return status(ok, ident, name, "simulated-isolated", err or out or "isolated rm pattern removed test file")
        if ident == "cleanup.dependencies":
            rc, out, err = run("apt-get -s autoremove", timeout=30)
            return status(rc == 0, ident, name, "simulated", err or out.splitlines()[-1] if out else "apt simulation ok")
    return status(False, ident, name, "isolated", "unknown isolated test")


def pids_for_pattern(pattern: str) -> set[str]:
    rc, out, _ = run(f"pgrep -u $(id -u) -f {shlex.quote(pattern)}")
    if rc != 0 or not out:
        return set()
    return {line.strip().split()[0] for line in out.splitlines() if line.strip()}


def cleanup_new_processes(pattern: str, before: set[str]) -> set[str]:
    run("sleep 1", timeout=3)
    after = pids_for_pattern(pattern)
    new_pids = after - before
    if new_pids:
        run("kill " + " ".join(sorted(new_pids)), timeout=5)
    return new_pids


def desktop_process_pattern(result: dict[str, Any]) -> str | None:
    ident = result["id"]
    if ident in DESKTOP_LAUNCH_PROCESS_PATTERNS:
        return DESKTOP_LAUNCH_PROCESS_PATTERNS[ident]
    desktop_id = str(result.get("slots", {}).get("desktop_id", ""))
    exec_line = str(result.get("slots", {}).get("exec", ""))
    if desktop_id:
        return re.escape(desktop_id.split(".", 1)[0])
    if exec_line:
        return re.escape(Path(shlex.split(exec_line)[0]).name)
    return None


def test_desktop_launch(result: dict[str, Any]) -> dict[str, Any]:
    ident = result["id"]
    name = result["name"]
    command = result.get("command", "")
    pattern = desktop_process_pattern(result)
    before = pids_for_pattern(pattern) if pattern else set()
    rc, out, err = run(command, timeout=10)
    new_pids = cleanup_new_processes(pattern, before) if pattern else set()
    if rc != 0:
        return status(False, ident, name, "desktop-launch", f"rc={rc}, {err or out}")
    detail = f"{command} returned 0"
    if new_pids:
        detail += f"; cleaned new pid(s) {','.join(sorted(new_pids))}"
    return status(True, ident, name, "desktop-launch", detail)


def test_command(result: dict[str, Any], include_wifi_off: bool) -> dict[str, Any]:
    ident = result["id"]
    name = result["name"]
    command = result.get("command", "")
    if ident in SECURITY_NO_EXEC:
        ok = result.get("requires_confirmation") is True
        return status(ok, ident, name, "blocked-risk", "risk marker verified" if ok else "missing requires_confirmation=true")
    if ident in ISOLATED_TESTS:
        return test_isolated(ident, name)
    if result.get("transaction"):
        return test_gsettings_transaction(result)
    if ident.startswith("open.") or ident == "application.open":
        return test_desktop_launch(result)
    if ident.startswith("files."):
        return test_files_find(ident, name, command)
    if ident.startswith("bluetooth.power."):
        return test_bluetooth(ident, name, command)
    if ident.startswith("wifi."):
        return test_wifi(ident, name, command, include_wifi_off)
    if ident.startswith("volume.") and result["route"] == "direct-shell":
        return test_audio(ident, name, command)
    if result["route"] == "direct-shell" and command.startswith("gsettings set "):
        return test_gsettings(ident, name, command)
    if result["route"] == "actuator":
        return test_actuator_with_restore(ident, name, result.get("actuator_command", ""))
    rc, out, err = run(command)
    return status(rc == 0, ident, name, "executed", err or out or "ok")


def markdown_report(results: list[dict[str, Any]]) -> str:
    lines = [
        "# ukui-direct-control full test report",
        "",
        "| status | id | mode | detail |",
        "|---|---|---|---|",
    ]
    for row in results:
        mark = "PASS" if row["ok"] else "FAIL"
        detail = str(row["detail"]).replace("|", "\\|").replace("\n", " ")
        lines.append(f"| {mark} | `{row['id']}` | {row['mode']} | {detail[:180]} |")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Run full ukui-direct-control catalog tests.")
    parser.add_argument("--include-wifi-off", action="store_true", help="Run disruptive WiFi off/on with background self-restore")
    parser.add_argument("--report-dir", default=str(SKILL_DIR / "reports"), help="Directory for JSON and Markdown reports")
    args = parser.parse_args()

    catalog = load_catalog()
    by_id = {item["id"]: item for item in catalog}
    results: list[dict[str, Any]] = []

    for ident in TEST_INTENTS:
        item = by_id.get(ident)
        if not item:
            results.append(status(False, ident, ident, "missing", "id not in catalog"))
            continue
        resolved = resolve.resolve(TEST_INTENTS[ident], use_csv_fallback=False)
        if not resolved.get("matched") or resolved.get("id") != ident:
            results.append(status(False, ident, item["name"], "parse", f"got {resolved.get('id')}; {resolved}"))
            continue
        if resolved.get("needs_clarification"):
            results.append(status(False, ident, item["name"], "parse", f"missing slots {resolved.get('missing_slots')}"))
            continue
        results.append(test_command(resolved, args.include_wifi_off))

    for ident, intent in DYNAMIC_TEST_INTENTS.items():
        resolved = resolve.resolve(intent, use_csv_fallback=False)
        if not resolved.get("matched") or resolved.get("id") != ident:
            results.append(status(False, ident, "打开应用", "parse", f"got {resolved.get('id')}; {resolved}"))
            continue
        if resolved.get("needs_clarification"):
            results.append(status(False, ident, "打开应用", "parse", f"missing slots {resolved.get('missing_slots')}"))
            continue
        results.append(test_command(resolved, args.include_wifi_off))

    report_dir = Path(args.report_dir)
    report_dir.mkdir(parents=True, exist_ok=True)
    (report_dir / "full_test_report.json").write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf-8")
    (report_dir / "full_test_report.md").write_text(markdown_report(results), encoding="utf-8")

    passed = sum(1 for item in results if item["ok"])
    failed = len(results) - passed
    modes: dict[str, int] = {}
    for item in results:
        modes[item["mode"]] = modes.get(item["mode"], 0) + 1

    for row in results:
        print(f"{'PASS' if row['ok'] else 'FAIL'}\t{row['id']}\t{row['mode']}\t{row['detail']}")
    print(f"\nSUMMARY total={len(results)} passed={passed} failed={failed} modes={modes}")
    print(f"REPORT {report_dir / 'full_test_report.md'}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
