---
name: ukui-direct-control
description: Resolve and execute Chinese openKylin/UKUI desktop-control requests without vector search. Use when Codex or another agent needs to turn intents such as 打开蓝牙, 关闭任务栏蓝牙图标, 打开 wifi, 打开深色模式, 打开亮色模式, 强调主题色为黄色, 设置强调色为#2A88FE, 打开设置, 打开计算机, 打开文本编辑器, 设置屏幕亮度为80, 静音, 清空回收站, or 查找最近修改的 PDF 文件 into a direct shell command, a transactional theme command, a .desktop application launch, a gsettings/bluetoothctl/nmcli/amixer/find command, or a kylin-actuator directive.
---

# UKUI Direct Control

## Contract

Use this skill to produce an executable desktop action, not a generic command mapping.

Always resolve the user request with `scripts/resolve.py` first. The resolver is deterministic and uses:

1. `references/intent_catalog.json` for tested core system operations.
2. `scripts/desktop_app.py` for fuzzy "打开/启动/运行某个应用" requests through installed `.desktop` launchers.

Do not use `references/command_alignment.csv` as an execution source by default. It is kept only as a source-aligned development reference and is available through `--csv-fallback` when extending the curated catalog.

```bash
python scripts/resolve.py "打开蓝牙"
```

The resolver returns JSON with:

- `matched`: whether a deterministic intent was found
- `route`: `direct-shell` or `actuator`
- `command`: shell command to run when `route=direct-shell`; multi-key operations may contain multiple lines and every line must be executed
- `commands`: `command` split into executable lines for agents that prefer arrays
- `actuator_directive`: directive to send to kylin-actuator when `route=actuator`
- `actuator_command`: complete actuator CLI command, for example `kylin-actuator '{open volumeup}'`
- `requires_confirmation`: whether explicit user confirmation is required before execution
- `needs_clarification` and `missing_slots`: whether a required parameter is missing
- `verify`: optional commands for checking the result

## Workflow

1. Run `scripts/resolve.py "<user intent>"`.
2. If `matched=false`, ask one short clarification question or say the intent is unsupported.
3. If `needs_clarification=true`, ask only for the missing parameter listed in `missing_slots`.
4. If the user asks only for the command, return `command` or `actuator_directive`; do not execute.
5. If the user asks to run the action, execute only when the current machine is the target UKUI/openKylin machine.
6. If `requires_confirmation=true`, ask for explicit confirmation before running.
7. After execution, run the first suitable `verify` command when available and report the result briefly.

## Routing

Use `route=direct-shell` when the resolver returns a concrete command such as:

```bash
gsettings set org.ukui.bluetooth tray-show false
bluetoothctl power on
nmcli radio wifi on &
amixer -D pulse sset Master mute
gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic false
gsettings set org.ukui.style style-name ukui-dark
gsettings set org.mate.interface gtk-theme ukui-black
gsettings set org.ukui.style theme-color '#FFFF00'
gsettings set org.ukui.style custom-highlight-color '#FFFF00'
gsettings set org.ukui.style use-custom-highlight-color true
command -v gtk-launch >/dev/null && test -f /usr/share/applications/ukui-control-center.desktop && (gtk-launch ukui-control-center >/dev/null 2>&1 &)
command -v gtk-launch >/dev/null && test -f /usr/share/applications/peony-computer.desktop && (gtk-launch peony-computer >/dev/null 2>&1 &)
```

Use `route=actuator` when UKUI behavior is wrapped by kylin-actuator or the command is a DBus/media-key description. In this case, run `actuator_command` or send `actuator_directive` to kylin-actuator as `{<directive>}`. Do not try to execute the descriptive `DBus call ...` text directly.

For theme operations whose `transaction` field is present, run every line in the returned `command` exactly. Do not reduce an accent-color operation to only `theme-color`; it must also set `custom-highlight-color` and `use-custom-highlight-color`.

For requests like `打开文本编辑器`, `启动软件商店`, or `open calculator`, let `resolve.py` fall through to `desktop_app.py`. It searches `~/.local/share/applications`, `/usr/local/share/applications`, and `/usr/share/applications`, parses the `[Desktop Entry]` fields, scores `Name`/`Name[zh_CN]`/`GenericName`/`Keywords`/desktop id, and returns a `gtk-launch <desktop-id>` command. It skips hidden launchers and skips `NoDisplay=true` entries unless they are modeled as explicit catalog operations such as `open.computer`.

When running over SSH, set the desktop session environment first:

```bash
export DISPLAY=:0
export XDG_RUNTIME_DIR=/run/user/$(id -u)
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus
```

If the result source is `command_alignment.csv`, treat it as a fallback hit: inspect placeholders and risk before execution.

## Safety

Do not execute system-changing commands on non-UKUI hosts. Return the resolved command for the target machine instead.

Always confirm before running actions that can interrupt the session, weaken security, or delete data:

- closing WiFi or Bluetooth
- automatic login or passwordless login changes
- disabling wake/sleep password
- cleaning trash, temporary files, dependencies, or any command containing `rm`, `sudo`, or package-management operations
- shutdown, reboot, suspend, hibernate, or lock actions when remote access could be affected

## Parameter Rules

Let the resolver fill common parameters:

- brightness: clamp numeric values to `0..100`
- mouse pointer size: `small=24`, `medium=36`, `large=48`
- calendar: `农历=lunar`, `阳历/公历=solarlunar`
- first day of week: `星期天=sunday`, `星期一=monday`
- date format: `中文/cn=cn`, `英文/en=en`
- time format: `12小时=12`, `24小时=24`
- recent file search: default to recent 7 days when the user omits a day count
- accent color: accept `#RRGGBB`, `rgb(r,g,b)`, or common color names; always normalize to uppercase `#RRGGBB` before execution. For example, `强调主题色为黄色` must become `#FFFF00`, not the literal string `黄色`.
- common accent color names: `蓝色/#2A88FE`, `红色/#F44336`, `绿色/#4CAF50`, `黄色/#FFFF00`, `金黄色或琥珀色/#FFC107`, `紫色/#9C27B0`, `橙色/#FF9800`, `粉色/#E91E63`, `青色/#00BCD4`, `黑色/#000000`, `白色/#FFFFFF`, `灰色/#9E9E9E`

If a parameter cannot be inferred, ask for it instead of guessing.

## Core Resources

- `references/intent_catalog.json`: curated executable intent catalog used first.
- `references/command_alignment.csv`: extracted source-aligned command table used only as reference or explicit `--csv-fallback`.
- `references/supported_operations_mapping.csv`: supported operation mapping table for manual agent testing.
- `scripts/resolve.py`: deterministic resolver. It does not execute commands.
- `scripts/desktop_app.py`: deterministic `.desktop` application resolver for fuzzy app-open requests.
- `scripts/theme_control.py`: optional rollback-safe theme writer kept for validation and extension; catalog results return direct GSettings commands by default.
- `scripts/run_tests.py`: regression tests for representative Chinese intents.
- `scripts/smoke_test.py`: target-machine smoke test for parser coverage, command dependencies, gsettings keys, actuator reachability, and reversible safe execution.
- `scripts/full_test.py`: full target-machine catalog test. It tests every curated intent, executes reversible commands with restore, isolates file/deletion tests, and writes Markdown/JSON reports.

## Pitfalls

- **Theme patterns must be symmetrical**: When you add a pattern to `theme.dark` (e.g., "打开深色模式"), add the corresponding "亮色" variant to `theme.light` (e.g., "打开亮色模式"). Otherwise, users requesting the light counterpart get `matched=false` or worse — the resolver matches to the wrong theme entry. For example, "切换亮色主题" can incorrectly match `theme.dark` when `theme.light` lacks "亮色" patterns.
- **The resolver uses pattern matching, not semantic understanding**: A query like "亮色模式" that matches no pattern gets no result. Always add common user-facing variants (e.g., "亮色模式", "浅色模式", "亮色主题") to the `patterns` array of the corresponding intent.
- **Parameterized volume requests may match the wrong intent**: "设置音量为80%" matched `volume.max` (which sets 100%) instead of a parameterized volume set. If `resolve.py` returns a volume command with a hardcoded value that doesn't match the user's requested percentage, override the percentage in the `amixer` command directly: `amixer -D pulse sset Master <N>%`.

## Validation

On a real openKylin/UKUI test machine, copy the whole skill folder to the Desktop and run:

```bash
cd ~/桌面/ukui-direct-control
export DISPLAY=:0
export XDG_RUNTIME_DIR=/run/user/$(id -u)
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus
PYTHONUTF8=1 python3 scripts/full_test.py
```

If SSH is not using WiFi or a temporary disconnect is acceptable, also test WiFi off/on with self-restore:

```bash
PYTHONUTF8=1 python3 scripts/full_test.py --include-wifi-off
```

Inspect `reports/full_test_report.md` after the run.

## Examples

`打开蓝牙` resolves to:

```json
{
  "route": "direct-shell",
  "actuator_directive": "open bluetooth",
  "command": "bluetoothctl power on"
}
```

`关闭任务栏蓝牙图标` resolves to:

```json
{
  "route": "direct-shell",
  "actuator_directive": "close bluetoothicon",
  "command": "gsettings set org.ukui.bluetooth tray-show false"
}
```

`调高音量` resolves to:

```json
{
  "route": "actuator",
  "actuator_directive": "open volumeup",
  "actuator_command": "kylin-actuator '{open volumeup}'"
}
```

`打开深色模式` resolves to `theme.dark`:

```json
{
  "route": "direct-shell",
  "actuator_directive": "set dark",
  "command": "gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic false\\ngsettings set org.ukui.style style-name ukui-dark\\ngsettings set org.mate.interface gtk-theme ukui-black"
}
```

`打开亮色模式` resolves to `theme.light`:

```json
{
  "route": "direct-shell",
  "actuator_directive": "set light",
  "command": "gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic false\\ngsettings set org.ukui.style style-name ukui-light\\ngsettings set org.mate.interface gtk-theme ukui-white"
}
```

`强调主题色为黄色` resolves to:

```json
{
  "route": "direct-shell",
  "id": "theme.accent",
  "command": "gsettings set org.ukui.style theme-color '#FFFF00'\ngsettings set org.ukui.style custom-highlight-color '#FFFF00'\ngsettings set org.ukui.style use-custom-highlight-color true",
  "transaction": [
    {"schema": "org.ukui.style", "key": "theme-color", "value": "#FFFF00"},
    {"schema": "org.ukui.style", "key": "custom-highlight-color", "value": "#FFFF00"},
    {"schema": "org.ukui.style", "key": "use-custom-highlight-color", "value": "true"}
  ]
}
```

`打开文本编辑器` resolves on a machine with `pluma.desktop` installed to:

```json
{
  "source": "desktop_app",
  "id": "application.open",
  "route": "direct-shell",
  "command": "command -v gtk-launch >/dev/null && test -f /usr/share/applications/pluma.desktop && (gtk-launch pluma >/dev/null 2>&1 &)"
}
```
