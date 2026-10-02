#!/usr/bin/env python3
"""Regression tests for the UKUI direct-control resolver."""

from __future__ import annotations

import os
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import resolve  # noqa: E402


TESTS = [
    ("打开蓝牙", {"id": "bluetooth.power.on", "command": "bluetoothctl power on", "route": "direct-shell"}),
    ("关闭蓝牙", {"id": "bluetooth.power.off", "requires_confirmation": True}),
    ("关闭任务栏蓝牙图标", {"id": "bluetooth.tray.hide", "command": "gsettings set org.ukui.bluetooth tray-show false"}),
    ("打开wifi", {"id": "wifi.power.on", "command": "nmcli radio wifi on &"}),
    ("关闭 wifi", {"id": "wifi.power.off", "requires_confirmation": True}),
    ("静音", {"id": "volume.mute.on", "command": "amixer -D pulse sset Master mute"}),
    ("取消静音", {"id": "volume.mute.off", "command": "amixer -D pulse sset Master unmute"}),
    ("调高音量", {"id": "volume.up", "route": "actuator", "actuator_directive": "open volumeup"}),
    ("调高音量", {"id": "volume.up", "actuator_command": "kylin-actuator '{open volumeup}'"}),
    ("设置屏幕亮度为80", {"id": "brightness.set", "command": "gsettings set org.ukui.power-manager brightness-ac 80"}),
    ("打开深色模式", {"id": "theme.dark", "command_contains": "gsettings set org.ukui.style style-name ukui-dark"}),
    ("关闭深色模式", {"id": "theme.light", "command_contains": "gsettings set org.ukui.style style-name ukui-light"}),
    ("打开自适应主题", {"id": "theme.auto", "command_contains": "gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic true"}),
    ("设置强调色为#2A88FE", {"id": "theme.accent", "command_contains": "gsettings set org.ukui.style theme-color '#2A88FE'", "slots_contains": {"color_value": "#2A88FE"}}),
    ("设置强调色为rgb(42,136,254)", {"id": "theme.accent", "command_contains": "gsettings set org.ukui.style theme-color '#2A88FE'", "slots_contains": {"color_value": "#2A88FE"}}),
    ("设置强调色为蓝色", {"id": "theme.accent", "slots_contains": {"color_value": "#2A88FE"}}),
    ("强调主题色为黄色", {"id": "theme.accent", "command_contains": "gsettings set org.ukui.style theme-color '#FFFF00'", "slots_contains": {"color_value": "#FFFF00"}}),
    ("把主题强调色改为黄", {"id": "theme.accent", "command_contains": "gsettings set org.ukui.style theme-color '#FFFF00'", "slots_contains": {"color_value": "#FFFF00"}}),
    ("设置强调色为金黄色", {"id": "theme.accent", "slots_contains": {"color_value": "#FFC107"}}),
    ("设置强调色", {"id": "theme.accent", "needs_clarification": True}),
    ("把鼠标指针调大", {"id": "mouse.cursor.size", "command": "gsettings set org.ukui.peripherals-mouse cursor-size 48"}),
    ("时间格式设置成24小时", {"id": "locale.time_format", "command": "gsettings set org.ukui.control-center.panel.plugins hoursystem 24"}),
    ("设置农历", {"id": "calendar.type", "actuator_directive": "set calendar lunar"}),
    ("一周第一天设为星期天", {"id": "calendar.firstday", "command": "gsettings set org.ukui.control-center.panel.plugins firstday sunday"}),
    ("打开键盘按键提示", {"id": "keyboard.repeat_notification.on", "command": "gsettings set org.ukui.control-center.osd show-lock-tip true"}),
    ("关闭睡眠密码", {"id": "security.password_on_sleep.off", "command": "gsettings set org.ukui.screensaver sleep-activation-enabled false", "requires_confirmation": True}),
    ("使用电源时设置最佳性能", {"id": "power.scheme.ac", "command": "gsettings set org.ukui.power-manager power-policy-ac 0"}),
    ("使用电池时设置节能", {"id": "power.scheme.battery", "command": "gsettings set org.ukui.power-manager power-policy-battery 2"}),
    ("清空回收站", {"id": "trash.empty", "requires_confirmation": True}),
    ("查找最近3天修改的PDF文件", {"id": "files.modified.pdf", "command": "find . -name '*.pdf' -mtime -3"}),
    ("查找最近创建的Word文件", {"id": "files.created.word", "command": "find . \\( -name '*.doc' -o -name '*.docx' \\) -ctime -7"}),
    ("查找最近7天创建的word文件", {"id": "files.created.word", "command": "find . \\( -name '*.doc' -o -name '*.docx' \\) -ctime -7"}),
    ("查找最近7天创建的pdf文件", {"id": "files.created.pdf", "command": "find . -name '*.pdf' -ctime -7"}),
    ("设置桌面背景", {"id": "background.set", "needs_clarification": True}),
    ("打开设置", {"id": "open.settings", "command": "command -v gtk-launch >/dev/null && test -f /usr/share/applications/ukui-control-center.desktop && (gtk-launch ukui-control-center >/dev/null 2>&1 &)", "route": "direct-shell"}),
    ("打开计算机", {"id": "open.computer", "command": "command -v gtk-launch >/dev/null && test -f /usr/share/applications/peony-computer.desktop && (gtk-launch peony-computer >/dev/null 2>&1 &)", "route": "direct-shell"}),
    ("打开计算器", {"id": "open.calculator", "command": "command -v gtk-launch >/dev/null && test -f /usr/share/applications/kylin-calculator.desktop && (gtk-launch kylin-calculator >/dev/null 2>&1 &)", "route": "direct-shell"}),
    ("打开文本编辑器", {"id": "application.open", "source": "desktop_app", "execute_directly": True}),
    ("打开应用", {"id": "application.open", "source": "desktop_app", "needs_clarification": True}),
    ("打开一个不存在的应用", {"matched": False, "source": "desktop_app"}),
    ("播放音乐", {"id": "media.play", "command": "xdotool key XF86AudioPlay", "route": "direct-shell"}),
    ("暂停音乐", {"id": "media.pause", "command": "xdotool key XF86AudioPause", "route": "direct-shell"}),
]


def write_desktop_fixture(root: Path) -> None:
    (root / "pluma.desktop").write_text(
        """[Desktop Entry]
Type=Application
Name=Pluma
Name[zh_CN]=文本编辑器
GenericName=Text Editor
GenericName[zh_CN]=文本编辑器
Exec=pluma %U
""",
        encoding="utf-8",
    )


def check_case(intent: str, expected: dict[str, object]) -> tuple[bool, str]:
    result = resolve.resolve(intent, use_csv_fallback=False)
    for key, value in expected.items():
        if key == "command_contains":
            command = str(result.get("command", ""))
            if str(value) not in command:
                return False, f"{intent}: expected command to contain {value!r}, got {command!r}; result={result}"
            continue
        if key == "slots_contains":
            slots = result.get("slots") or {}
            for slot_key, slot_value in dict(value).items():
                if slots.get(slot_key) != slot_value:
                    return False, f"{intent}: expected slots.{slot_key}={slot_value!r}, got {slots.get(slot_key)!r}; result={result}"
            continue
        if result.get(key) != value:
            return False, f"{intent}: expected {key}={value!r}, got {result.get(key)!r}; result={result}"
    return True, intent


def main() -> int:
    failures: list[str] = []
    old_roots = os.environ.get("UKUI_DESKTOP_APP_ROOTS")
    with tempfile.TemporaryDirectory(prefix="ukui-desktop-fixture-") as tmp:
        root = Path(tmp)
        write_desktop_fixture(root)
        os.environ["UKUI_DESKTOP_APP_ROOTS"] = str(root)
        for intent, expected in TESTS:
            ok, message = check_case(intent, expected)
            if ok:
                print(f"PASS {message}")
            else:
                print(f"FAIL {message}")
                failures.append(message)
    if old_roots is None:
        os.environ.pop("UKUI_DESKTOP_APP_ROOTS", None)
    else:
        os.environ["UKUI_DESKTOP_APP_ROOTS"] = old_roots
    if failures:
        print(f"\n{len(failures)} failed, {len(TESTS) - len(failures)} passed")
        return 1
    print(f"\n{len(TESTS)} passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
