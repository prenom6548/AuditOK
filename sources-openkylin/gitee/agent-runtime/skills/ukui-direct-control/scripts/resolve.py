#!/usr/bin/env python3
"""Resolve Chinese UKUI/openKylin desktop intents to executable actions.

The script does not execute commands. It returns a deterministic JSON decision
that an agent can either run directly or route through kylin-actuator.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import shlex
from pathlib import Path
from typing import Any

import desktop_app


SKILL_DIR = Path(__file__).resolve().parents[1]
CATALOG_PATH = SKILL_DIR / "references" / "intent_catalog.json"
CSV_PATH = SKILL_DIR / "references" / "command_alignment.csv"


ACTION_WORDS = {
    "open": ["打开", "开启", "启用", "显示", "启动", "接通", "开"],
    "close": ["关闭", "关掉", "禁用", "停用", "隐藏", "取消", "关"],
    "increase": ["增加", "增大", "调高", "提高", "加大", "上调", "升高"],
    "decrease": ["减少", "减小", "调低", "降低", "下调", "变小"],
    "set": ["设置", "设为", "调到", "调整", "改成", "切换", "改为"],
    "empty": ["清空", "清理", "删除"],
    "search": ["查找", "搜索", "找出", "找一下"],
    "screenshot": ["截图", "截屏"],
}


FILE_ALIASES = {
    "word": ["word", "doc", "文档"],
    "excel": ["excel", "execl", "xlsx", "表格"],
    "ppt": ["ppt", "演示", "幻灯片"],
    "pdf": ["pdf"],
    "image": ["图片", "照片", "png", "jpg", "jpeg"],
    "text": ["文本", "txt"],
}

PAGE_OPEN_WORDS = ["设置页面", "设置页", "设置界面", "页面"]


def normalize(text: str) -> str:
    text = text.replace("Ｗ", "W").replace("ｗ", "w")
    text = re.sub(r"\s+", " ", text.strip())
    return text


def text_lc(text: str) -> str:
    return normalize(text).lower()


def contains_any(text: str, words: list[str]) -> bool:
    lower = text_lc(text)
    return any(word.lower() in lower for word in words)


def infer_action(intent: str) -> str:
    for action, words in ACTION_WORDS.items():
        if contains_any(intent, words):
            return action
    if re.search(r"\d+\s*%?", intent):
        return "set"
    return "default"


def load_catalog() -> list[dict[str, Any]]:
    with CATALOG_PATH.open("r", encoding="utf-8") as f:
        return json.load(f)


def load_csv_rows() -> list[dict[str, str]]:
    if not CSV_PATH.exists():
        return []
    with CSV_PATH.open("r", encoding="utf-8-sig", newline="") as f:
        return list(csv.DictReader(f))


def score_entry(intent: str, entry: dict[str, Any]) -> tuple[int, list[str]]:
    lower = text_lc(intent)
    reasons: list[str] = []
    entry_id = str(entry.get("id", ""))
    command = str(entry.get("command", ""))

    if contains_any(intent, PAGE_OPEN_WORDS) and not entry_id.startswith("open."):
        return -10_000, ["blocked by page-open request"]

    for anti in entry.get("anti_patterns", []):
        if anti.lower() in lower:
            return -10_000, [f"blocked by anti_pattern={anti}"]

    score = 0
    for pattern in entry.get("patterns", []):
        p = pattern.lower()
        if lower == p:
            score += 160
            reasons.append(f"exact pattern={pattern}")
        elif p in lower:
            score += 110
            reasons.append(f"pattern={pattern}")

    for domain in entry.get("domains", []):
        if domain.lower() in lower:
            score += 24
            reasons.append(f"domain={domain}")

    inferred = infer_action(intent)
    actions = entry.get("actions", [])
    if inferred in actions:
        score += 35
        reasons.append(f"action={inferred}")
    elif "default" in actions:
        score += 10
        reasons.append("default action")

    for keyword in entry.get("keywords", []):
        if keyword.lower() in lower:
            score += 12
            reasons.append(f"keyword={keyword}")

    # Prefer exact file type for recent-file searches.
    file_type = entry.get("file_type")
    if file_type and contains_any(intent, FILE_ALIASES.get(file_type, [])):
        score += 30
        reasons.append(f"file_type={file_type}")

    if contains_any(intent, ["创建", "新建"]):
        if ".created." in entry_id or "-ctime" in command:
            score += 55
            reasons.append("time_kind=created")
        if ".modified." in entry_id or "-mtime" in command:
            score -= 55
            reasons.append("penalty=modified")
    if contains_any(intent, ["修改", "改动", "变更"]):
        if ".modified." in entry_id or "-mtime" in command:
            score += 55
            reasons.append("time_kind=modified")
        if ".created." in entry_id or "-ctime" in command:
            score -= 55
            reasons.append("penalty=created")

    anchored = any(
        reason.startswith(("exact pattern=", "pattern=", "domain=", "file_type=", "time_kind="))
        for reason in reasons
    )
    if score > 0 and not anchored:
        return 0, reasons + ["no strong anchor"]

    return score, reasons


def choose_entry(intent: str, catalog: list[dict[str, Any]]) -> tuple[dict[str, Any] | None, int, list[str]]:
    ranked: list[tuple[int, list[str], dict[str, Any]]] = []
    for entry in catalog:
        score, reasons = score_entry(intent, entry)
        ranked.append((score, reasons, entry))
    ranked.sort(key=lambda item: item[0], reverse=True)
    if not ranked or ranked[0][0] < 45:
        return None, ranked[0][0] if ranked else 0, ranked[0][1] if ranked else []
    return ranked[0][2], ranked[0][0], ranked[0][1]


def extract_percent(intent: str) -> tuple[str | None, str | None]:
    match = re.search(r"(\d{1,3})\s*%?", intent)
    if not match:
        return None, "需要提供 0-100 的数值"
    value = max(0, min(100, int(match.group(1))))
    note = None
    if value != int(match.group(1)):
        note = f"数值已限制到 {value}"
    return str(value), note


def extract_cursor_size(intent: str) -> tuple[dict[str, str] | None, str | None]:
    if contains_any(intent, ["最大", "调大", "加大", "大号", "大的", "大"]):
        return {"size": "large", "size_value": "48"}, None
    if contains_any(intent, ["最小", "调小", "减小", "小号", "小的", "小"]):
        return {"size": "small", "size_value": "24"}, None
    if contains_any(intent, ["中", "默认", "适中", "普通"]):
        return {"size": "medium", "size_value": "36"}, None
    return None, "需要说明鼠标指针大小：小/中/大"


def extract_calendar_type(intent: str) -> tuple[str | None, str | None]:
    if contains_any(intent, ["农历", "阴历"]):
        return "lunar", None
    if contains_any(intent, ["阳历", "公历"]):
        return "solarlunar", None
    return None, "需要说明日历类型：农历或阳历"


def extract_firstday(intent: str) -> tuple[str | None, str | None]:
    if contains_any(intent, ["星期天", "周日", "礼拜天", "sunday"]):
        return "sunday", None
    if contains_any(intent, ["星期一", "周一", "礼拜一", "monday"]):
        return "monday", None
    return None, "需要说明一周第一天：星期天或星期一"


def extract_date_format(intent: str) -> tuple[str | None, str | None]:
    if contains_any(intent, ["中文", "中国", "cn"]):
        return "cn", None
    if contains_any(intent, ["英文", "美国", "en"]):
        return "en", None
    return None, "需要说明日期格式：中文/cn 或 英文/en"


def extract_time_format(intent: str) -> tuple[str | None, str | None]:
    if re.search(r"12|十二", intent):
        return "12", None
    if re.search(r"24|二十四", intent):
        return "24", None
    return None, "需要说明时间格式：12 或 24 小时"


def extract_power_scheme(intent: str) -> tuple[str | None, str | None]:
    if contains_any(intent, ["性能", "最佳性能", "高性能"]):
        return "0", None
    if contains_any(intent, ["平衡", "默认"]):
        return "1", None
    if contains_any(intent, ["节能", "省电", "最佳能效", "能效"]):
        return "2", None
    return None, "需要说明电源策略：节能/平衡/性能"


def extract_language(intent: str) -> tuple[str | None, str | None]:
    if contains_any(intent, ["中文", "中国", "简体", "zh"]):
        return "zh_CN", None
    if contains_any(intent, ["英文", "英语", "美国", "en"]):
        return "en_US", None
    return None, "需要说明语言：中文或英文"


def extract_days(intent: str) -> tuple[str, str | None]:
    match = re.search(r"(\d{1,3})\s*天", intent)
    if match:
        return str(max(1, min(365, int(match.group(1))))), None
    return "7", "未指定天数，按最近 7 天处理"


def extract_wallpaper(intent: str) -> tuple[str | None, str | None]:
    quoted = re.search(r"[\"'“](.+?)[\"'”]", intent)
    if quoted:
        return shlex.quote(quoted.group(1)), None
    path_like = re.search(r"(/[^\s]+|[A-Za-z]:\\[^\s]+)", intent)
    if path_like:
        return shlex.quote(path_like.group(1)), None
    return None, "需要提供图片路径"


COLOR_NAME_MAP = {
    "蓝色": "#2A88FE",
    "蓝": "#2A88FE",
    "红色": "#F44336",
    "红": "#F44336",
    "绿色": "#4CAF50",
    "绿": "#4CAF50",
    "琥珀色": "#FFC107",
    "琥珀": "#FFC107",
    "金黄色": "#FFC107",
    "金黄": "#FFC107",
    "黄色": "#FFFF00",
    "黄": "#FFFF00",
    "紫色": "#9C27B0",
    "紫": "#9C27B0",
    "橙色": "#FF9800",
    "橙": "#FF9800",
    "粉色": "#E91E63",
    "粉": "#E91E63",
    "青色": "#00BCD4",
    "青": "#00BCD4",
    "黑色": "#000000",
    "黑": "#000000",
    "白色": "#FFFFFF",
    "白": "#FFFFFF",
    "灰色": "#9E9E9E",
    "灰": "#9E9E9E",
    "blue": "#2A88FE",
    "red": "#F44336",
    "green": "#4CAF50",
    "amber": "#FFC107",
    "golden": "#FFC107",
    "yellow": "#FFFF00",
    "purple": "#9C27B0",
    "orange": "#FF9800",
    "pink": "#E91E63",
    "cyan": "#00BCD4",
    "black": "#000000",
    "white": "#FFFFFF",
    "gray": "#9E9E9E",
    "grey": "#9E9E9E",
}


def color_slots(value: str) -> dict[str, str]:
    color = value.upper()
    return {"color": shlex.quote(color), "color_value": color}


def extract_color(intent: str) -> tuple[dict[str, str] | None, str | None]:
    text = normalize(intent).replace("＃", "#")
    hex_match = re.search(r"#?([0-9A-Fa-f]{6})", text)
    if hex_match:
        return color_slots("#" + hex_match.group(1)), None

    rgb_match = re.search(r"rgb\s*\(\s*(\d{1,3})\s*[,，]\s*(\d{1,3})\s*[,，]\s*(\d{1,3})\s*\)", text, re.IGNORECASE)
    if not rgb_match:
        rgb_match = re.search(r"(?:rgb|RGB|颜色|色值|强调色|高亮色)[^\d]*(\d{1,3})\s*[,，]\s*(\d{1,3})\s*[,，]\s*(\d{1,3})", text)
    if rgb_match:
        channels = [int(rgb_match.group(i)) for i in range(1, 4)]
        if any(value > 255 for value in channels):
            return None, "RGB 颜色值需要在 0-255 范围内"
        return color_slots("#" + "".join(f"{value:02X}" for value in channels)), None

    lower = text_lc(text)
    for name, color in COLOR_NAME_MAP.items():
        if name.lower() in lower:
            return color_slots(color), None
    return None, "需要提供 #RRGGBB 或 rgb(r,g,b) 格式的强调色"


APP_CUE_WORDS = ["应用", "软件", "程序", "app", "application", "software"]
APP_BLOCK_WORDS = [
    "蓝牙",
    "wifi",
    "wi-fi",
    "无线",
    "网络",
    "音量",
    "声音",
    "亮度",
    "鼠标",
    "键盘",
    "触摸板",
    "任务栏",
    "日历",
    "日期",
    "时间",
    "语言",
    "电源",
    "睡眠",
    "休眠",
    "免密",
    "自动登录",
    "回收站",
]


def clean_application_query(text: str) -> str:
    value = normalize(text)
    value = re.sub(r"^(一个|这个|那个|名为|叫做)\s*", "", value, flags=re.IGNORECASE)
    value = re.sub(r"(这个|那个)?(应用|软件|程序|app|application|software)$", "", value, flags=re.IGNORECASE).strip()
    value = re.sub(r"(一下|起来|吧|。|！|!|please)$", "", value, flags=re.IGNORECASE).strip()
    return value


def extract_application_query(intent: str) -> tuple[str | None, bool]:
    text = normalize(intent)
    lower = text_lc(text)

    zh = re.match(r"^(?:请|帮我|麻烦你?|麻烦)?(?:给我)?(?:打开|启动|运行|开启)\s*(.+)$", text)
    en = re.match(r"^(?:please\s+)?(?:open|launch|start|run)\s+(.+)$", lower)
    raw_query = zh.group(1) if zh else (en.group(1) if en else None)
    if raw_query is None:
        return None, False

    query = clean_application_query(raw_query)
    if not query or query in APP_CUE_WORDS:
        return "", True

    has_app_cue = contains_any(text, APP_CUE_WORDS)
    if not has_app_cue and contains_any(query, APP_BLOCK_WORDS):
        return None, False
    if len(query) < 2:
        return "", True
    return query, False


def application_result_base(intent: str) -> dict[str, Any]:
    return {
        "intent": intent,
        "matched": True,
        "source": "desktop_app",
        "id": "application.open",
        "name": "打开应用",
        "route": "direct-shell",
        "actuator_directive": "",
        "actuator_command": "",
        "command": "",
        "requires_confirmation": False,
        "risk": "low",
        "execute_directly": False,
        "needs_clarification": True,
        "missing_slots": ["application"],
        "slots": {},
        "verify": [],
        "desktop_env": {
            "DISPLAY": ":0",
            "XDG_RUNTIME_DIR": "/run/user/$(id -u)",
            "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/$(id -u)/bus",
        },
        "match_score": 0,
        "reason": "application name missing",
        "notes": ["需要说明要打开哪个应用"],
    }


def desktop_application_fallback(intent: str) -> dict[str, Any] | None:
    app_query, needs_application = extract_application_query(intent)
    if app_query is None:
        return None
    if needs_application:
        return application_result_base(intent)

    resolved = desktop_app.resolve_application(app_query)
    if not resolved.get("matched"):
        return {
            "intent": intent,
            "matched": False,
            "source": "desktop_app",
            "reason": f"no installed desktop application matched: {app_query}",
            "candidates": resolved.get("candidates", []),
        }

    return {
        "intent": intent,
        "matched": True,
        "source": "desktop_app",
        "id": "application.open",
        "name": f"打开应用：{resolved['name']}",
        "route": "direct-shell",
        "actuator_directive": "",
        "actuator_command": "",
        "command": resolved["command"],
        "requires_confirmation": False,
        "risk": "low",
        "execute_directly": True,
        "needs_clarification": False,
        "missing_slots": [],
        "slots": {
            "application": app_query,
            "desktop_id": resolved["desktop_id"],
            "desktop_file": resolved["path"],
            "exec": resolved["exec"],
        },
        "verify": [],
        "desktop_env": {
            "DISPLAY": ":0",
            "XDG_RUNTIME_DIR": "/run/user/$(id -u)",
            "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/$(id -u)/bus",
        },
        "match_score": resolved["score"],
        "reason": resolved.get("reason", ""),
        "notes": ["通过目标机器的 .desktop 启动器解析；跨机器执行时应在目标 openKylin 会话内运行 resolver"],
    }


SLOT_EXTRACTORS = {
    "percent": lambda intent: extract_percent(intent),
    "cursor_size": lambda intent: extract_cursor_size(intent),
    "calendar_type": lambda intent: extract_calendar_type(intent),
    "firstday": lambda intent: extract_firstday(intent),
    "date_format": lambda intent: extract_date_format(intent),
    "time_format": lambda intent: extract_time_format(intent),
    "power_scheme": lambda intent: extract_power_scheme(intent),
    "language": lambda intent: extract_language(intent),
    "days": lambda intent: extract_days(intent),
    "wallpaper": lambda intent: extract_wallpaper(intent),
    "color": lambda intent: extract_color(intent),
}


def resolve_slots(intent: str, entry: dict[str, Any]) -> tuple[dict[str, str], list[str], list[str]]:
    slots: dict[str, str] = {"skill_dir": shlex.quote(str(SKILL_DIR))}
    missing: list[str] = []
    notes: list[str] = []

    for slot in entry.get("slots", []):
        slot_name = slot["name"]
        slot_type = slot["type"]
        extractor = SLOT_EXTRACTORS[slot_type]
        value, note = extractor(intent)
        if isinstance(value, dict):
            slots.update(value)
        elif value is not None:
            slots[slot_name] = value
        else:
            missing.append(slot_name)
        if note:
            notes.append(note)

    return slots, missing, notes


def format_template(value: str, slots: dict[str, str]) -> str:
    if not value:
        return ""
    try:
        return value.format(**slots)
    except KeyError:
        return value


def format_transaction(entry: dict[str, Any], slots: dict[str, str]) -> list[dict[str, str]]:
    transaction = []
    for step in entry.get("transaction", []):
        transaction.append(
            {
                "schema": step.get("schema", ""),
                "key": step.get("key", ""),
                "value": format_template(step.get("value", ""), slots),
            }
        )
    return transaction


def is_concrete_shell(command: str) -> bool:
    if not command:
        return False
    if command.startswith("DBus call"):
        return False
    if re.search(r"<[^>]+>", command):
        return False
    if "{" in command or "}" in command:
        return False
    return True


def actuator_command(directive: str) -> str:
    if not directive:
        return ""
    task = "{" + directive + "}"
    return "kylin-actuator " + shlex.quote(task)


def result_from_entry(intent: str, entry: dict[str, Any], score: int, reasons: list[str]) -> dict[str, Any]:
    slots, missing, notes = resolve_slots(intent, entry)
    command = format_template(entry.get("command", ""), slots)
    commands = [line.strip() for line in command.splitlines() if line.strip()]
    directive = format_template(entry.get("actuator_directive", ""), slots)
    verify = [format_template(v, slots) for v in entry.get("verify", [])]
    transaction = format_transaction(entry, slots)
    needs_confirmation = bool(entry.get("confirm", False))
    route = entry.get("route", "direct-shell")
    concrete_shell = route == "direct-shell" and is_concrete_shell(command)

    return {
        "intent": intent,
        "matched": True,
        "source": "intent_catalog",
        "id": entry.get("id"),
        "name": entry.get("name"),
        "route": route,
        "actuator_directive": directive,
        "actuator_command": actuator_command(directive),
        "command": command,
        "commands": commands,
        "requires_confirmation": needs_confirmation,
        "risk": entry.get("risk", "low"),
        "execute_directly": concrete_shell and not needs_confirmation and not missing,
        "needs_clarification": bool(missing),
        "missing_slots": missing,
        "slots": slots,
        "verify": verify,
        "transaction": transaction,
        "desktop_env": {
            "DISPLAY": ":0",
            "XDG_RUNTIME_DIR": "/run/user/$(id -u)",
            "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/$(id -u)/bus",
        },
        "match_score": score,
        "reason": "; ".join(reasons[:5]),
        "notes": notes + entry.get("notes", []),
    }


def score_csv_row(intent: str, row: dict[str, str]) -> int:
    corpus = " ".join(
        [
            row.get("中文命令", ""),
            row.get("执行器指令", ""),
            row.get("系统命令", ""),
            row.get("归属模块", ""),
            row.get("备注", ""),
        ]
    ).lower()
    score = 0
    for token in re.findall(r"[\u4e00-\u9fff]{2,}|[A-Za-z0-9_+-]+", intent):
        if token.lower() in corpus:
            score += max(4, len(token) * 2)
    action = infer_action(intent)
    directive = row.get("执行器指令", "")
    if action in {"open", "close", "set", "empty"} and directive.startswith(action):
        score += 16
    if "任务栏" in intent and "bluetoothicon" in directive:
        score += 40
    return score


def csv_fallback(intent: str) -> dict[str, Any] | None:
    rows = load_csv_rows()
    if not rows:
        return None
    ranked = sorted(rows, key=lambda row: score_csv_row(intent, row), reverse=True)
    best = ranked[0]
    score = score_csv_row(intent, best)
    if score < 18:
        return None
    command = best.get("系统命令", "")
    route = "direct-shell" if is_concrete_shell(command) else "actuator"
    directive = best.get("执行器指令", "")
    needs_confirmation = bool(re.search(r"\bsudo\b|rm\s+-rf|gio trash --empty", command))
    return {
        "intent": intent,
        "matched": True,
        "source": "command_alignment.csv",
        "id": None,
        "name": best.get("中文命令", ""),
        "route": route,
        "actuator_directive": directive,
        "actuator_command": actuator_command(directive),
        "command": command,
        "requires_confirmation": needs_confirmation,
        "risk": "high" if needs_confirmation else "medium",
        "execute_directly": route == "direct-shell" and not needs_confirmation,
        "needs_clarification": False,
        "missing_slots": [],
        "slots": {},
        "verify": [],
        "desktop_env": {
            "DISPLAY": ":0",
            "XDG_RUNTIME_DIR": "/run/user/$(id -u)",
            "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/$(id -u)/bus",
        },
        "match_score": score,
        "reason": "deterministic CSV fallback",
        "notes": ["fallback 命中时，执行前人工检查命令和参数"],
    }


def suggest_candidates(intent: str, catalog: list[dict[str, Any]]) -> list[dict[str, Any]]:
    ranked = []
    for entry in catalog:
        score, _ = score_entry(intent, entry)
        if score > 0:
            ranked.append({"id": entry.get("id"), "name": entry.get("name"), "score": score})
    ranked.sort(key=lambda row: row["score"], reverse=True)
    return ranked[:5]


def resolve(intent: str, use_csv_fallback: bool = False) -> dict[str, Any]:
    normalized = normalize(intent)
    catalog = load_catalog()
    entry, score, reasons = choose_entry(normalized, catalog)
    if entry:
        return result_from_entry(normalized, entry, score, reasons)

    app_result = desktop_application_fallback(normalized)
    if app_result:
        return app_result

    if use_csv_fallback:
        fallback = csv_fallback(normalized)
        if fallback:
            return fallback

    return {
        "intent": normalized,
        "matched": False,
        "source": None,
        "reason": "no deterministic intent matched",
        "candidates": suggest_candidates(normalized, catalog),
    }


def print_text(result: dict[str, Any]) -> None:
    if not result["matched"]:
        print(f"未匹配: {result['intent']}")
        if result.get("candidates"):
            print("候选:")
            for item in result["candidates"]:
                print(f"- {item['id']} {item['name']} score={item['score']}")
        return
    print(f"意图: {result['intent']}")
    print(f"匹配: {result.get('id') or result.get('name')} ({result['source']})")
    print(f"路线: {result['route']}")
    print(f"执行器指令: {result.get('actuator_directive', '')}")
    print(f"执行器命令: {result.get('actuator_command', '')}")
    print(f"系统命令: {result.get('command', '')}")
    print(f"需确认: {result['requires_confirmation']}")
    if result.get("needs_clarification"):
        print(f"缺少参数: {', '.join(result['missing_slots'])}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Resolve a Chinese UKUI intent to a command.")
    parser.add_argument("intent", nargs="?", help="Chinese natural-language UKUI control request")
    parser.add_argument("--format", choices=["json", "text"], default="json")
    parser.add_argument("--csv-fallback", action="store_true", help="Allow uncurated command_alignment.csv fallback")
    parser.add_argument("--no-csv-fallback", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--list", action="store_true", help="List supported curated intents")
    args = parser.parse_args()

    if args.list:
        catalog = load_catalog()
        rows = [
            {
                "id": item.get("id"),
                "name": item.get("name"),
                "route": item.get("route"),
                "risk": item.get("risk", "low"),
                "confirm": item.get("confirm", False),
            }
            for item in catalog
        ]
        rows.append(
            {
                "id": "application.open",
                "name": "打开应用（.desktop 模糊匹配）",
                "route": "direct-shell",
                "risk": "low",
                "confirm": False,
                "dynamic": True,
            }
        )
        if args.format == "json":
            print(json.dumps(rows, ensure_ascii=False, indent=2))
        else:
            for row in rows:
                mark = " confirm" if row["confirm"] else ""
                print(f"{row['id']}\t{row['name']}\t{row['route']}\t{row['risk']}{mark}")
        return

    if not args.intent:
        parser.error("intent is required unless --list is used")

    result = resolve(args.intent, use_csv_fallback=args.csv_fallback and not args.no_csv_fallback)
    if args.format == "json":
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        print_text(result)


if __name__ == "__main__":
    main()
