#!/usr/bin/env python3
"""Resolve application names to installed .desktop launch commands."""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
from dataclasses import dataclass
from pathlib import Path
from typing import Any


DESKTOP_ROOTS = [
    Path.home() / ".local" / "share" / "applications",
    Path("/usr/local/share/applications"),
    Path("/usr/share/applications"),
]

ALIASES = [
    ["设置", "系统设置", "控制中心", "控制面板", "settings", "control center", "ukui-control-center"],
    ["计算器", "calculator", "kylin-calculator"],
    ["计算机", "我的电脑", "此电脑", "computer", "peony-computer"],
    ["文件管理器", "文件", "file manager", "peony"],
    ["文本编辑器", "text editor", "editor", "pluma", "gedit"],
    ["终端", "terminal", "mate-terminal"],
    ["软件商店", "应用商店", "software store", "kylin-software-center"],
    ["浏览器", "网页浏览器", "browser", "chrome", "firefox"],
]

FIELD_CODE_RE = re.compile(r"(?<!%)%[fFuUdDnNickvm]")
TOKEN_RE = re.compile(r"[\u4e00-\u9fff]{2,}|[A-Za-z0-9_.+-]+")


@dataclass(frozen=True)
class DesktopApp:
    desktop_id: str
    path: str
    fields: dict[str, str]
    no_display: bool
    exec_line: str

    @property
    def display_name(self) -> str:
        return self.fields.get("Name[zh_CN]") or self.fields.get("Name") or self.desktop_id

    @property
    def terms(self) -> list[str]:
        result = [self.desktop_id, Path(self.desktop_id).stem, Path(self.exec_line.split(" ", 1)[0]).name]
        for key, value in self.fields.items():
            if key.startswith(("Name", "GenericName", "Keywords", "Comment")):
                result.extend(part for part in re.split(r"[;,\n]", value) if part)
        return [item.strip() for item in result if item and item.strip()]


def normalize(text: str) -> str:
    return re.sub(r"\s+", " ", text.strip()).lower()


def compact(text: str) -> str:
    return re.sub(r"[\s_\-./]+", "", normalize(text))


def tokens(text: str) -> set[str]:
    return {normalize(token) for token in TOKEN_RE.findall(text)}


def desktop_roots() -> list[Path]:
    override = os.environ.get("UKUI_DESKTOP_APP_ROOTS")
    if override:
        return [Path(item) for item in override.split(os.pathsep) if item]
    return DESKTOP_ROOTS


def desktop_id_for(path: Path, root: Path) -> str:
    try:
        rel = path.relative_to(root)
    except ValueError:
        rel = path.name
    desktop_id = str(rel).replace(os.sep, "-").replace("/", "-")
    if desktop_id.endswith(".desktop"):
        desktop_id = desktop_id[:-8]
    return desktop_id


def parse_desktop_file(path: Path, root: Path) -> DesktopApp | None:
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return None

    fields: dict[str, str] = {}
    in_entry = False
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("[") and line.endswith("]"):
            if line == "[Desktop Entry]":
                in_entry = True
                continue
            if in_entry:
                break
            continue
        if not in_entry or "=" not in line:
            continue
        key, value = line.split("=", 1)
        fields.setdefault(key.strip(), value.strip())

    if fields.get("Type", "Application") != "Application":
        return None
    if fields.get("Hidden", "").lower() == "true":
        return None
    exec_line = fields.get("Exec", "")
    if not exec_line:
        return None
    return DesktopApp(
        desktop_id=desktop_id_for(path, root),
        path=str(path),
        fields=fields,
        no_display=fields.get("NoDisplay", "").lower() == "true",
        exec_line=exec_line,
    )


def iter_apps(include_nodisplay: bool = False) -> list[DesktopApp]:
    seen: set[str] = set()
    apps: list[DesktopApp] = []
    for root in desktop_roots():
        if not root.exists():
            continue
        for path in sorted(root.rglob("*.desktop")):
            app = parse_desktop_file(path, root)
            if not app or app.desktop_id in seen:
                continue
            if app.no_display and not include_nodisplay:
                continue
            seen.add(app.desktop_id)
            apps.append(app)
    return apps


def query_variants(query: str) -> list[str]:
    base = normalize(query)
    variants = {base, compact(base)}
    for group in ALIASES:
        normalized_group = [normalize(item) for item in group]
        compact_group = [compact(item) for item in group]
        if any(item and (item in base or item in compact(base)) for item in normalized_group + compact_group):
            variants.update(normalized_group)
            variants.update(compact_group)
    return [item for item in variants if item]


def score_app(query: str, app: DesktopApp) -> tuple[int, list[str]]:
    variants = query_variants(query)
    query_tokens = tokens(query)
    score = 0
    reasons: list[str] = []

    for term in app.terms:
        normalized_term = normalize(term)
        compact_term = compact(term)
        for variant in variants:
            compact_variant = compact(variant)
            if not variant or not normalized_term:
                continue
            if variant == normalized_term or compact_variant == compact_term:
                score = max(score, 240)
                reasons.append(f"exact={term}")
            elif normalized_term.startswith(variant) or compact_term.startswith(compact_variant):
                score = max(score, 175)
                reasons.append(f"prefix={term}")
            elif variant in normalized_term or compact_variant in compact_term:
                score = max(score, 135)
                reasons.append(f"contains={term}")
            elif normalized_term in variant and len(normalized_term) >= 3:
                score = max(score, 95)
                reasons.append(f"reverse={term}")

        overlap = query_tokens & tokens(term)
        if overlap:
            token_score = 35 + len(overlap) * 15
            score = max(score, token_score)
            reasons.append(f"token={','.join(sorted(overlap))}")

    if app.desktop_id in {"ukui-control-center", "kylin-calculator", "peony", "pluma", "mate-terminal"}:
        score += 8
    if app.no_display:
        score -= 40
    return score, reasons[:5]


def sanitize_exec(exec_line: str) -> str:
    placeholder = "\0PERCENT\0"
    value = exec_line.replace("%%", placeholder)
    value = FIELD_CODE_RE.sub("", value).replace(placeholder, "%")
    return re.sub(r"\s+", " ", value).strip()


def launch_command(app: DesktopApp) -> str:
    return (
        "command -v gtk-launch >/dev/null && "
        f"test -f {shlex.quote(app.path)} && "
        f"(gtk-launch {shlex.quote(app.desktop_id)} >/dev/null 2>&1 &)"
    )


def resolve_application(query: str, include_nodisplay: bool = False, min_score: int = 80) -> dict[str, Any]:
    ranked: list[tuple[int, list[str], DesktopApp]] = []
    for app in iter_apps(include_nodisplay=include_nodisplay):
        score, reasons = score_app(query, app)
        if score > 0:
            ranked.append((score, reasons, app))
    ranked.sort(key=lambda item: item[0], reverse=True)

    candidates = [
        {
            "desktop_id": app.desktop_id,
            "name": app.display_name,
            "command": launch_command(app),
            "score": score,
            "path": app.path,
        }
        for score, _, app in ranked[:5]
    ]

    if not ranked or ranked[0][0] < min_score:
        return {"matched": False, "query": query, "candidates": candidates}

    score, reasons, app = ranked[0]
    return {
        "matched": True,
        "query": query,
        "desktop_id": app.desktop_id,
        "name": app.display_name,
        "path": app.path,
        "command": launch_command(app),
        "exec": sanitize_exec(app.exec_line),
        "score": score,
        "reason": "; ".join(reasons),
        "candidates": candidates,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Resolve an application name through .desktop files.")
    parser.add_argument("query", help="Application name, e.g. 计算器 or text editor")
    parser.add_argument("--include-nodisplay", action="store_true", help="Include NoDisplay=true desktop entries")
    args = parser.parse_args()

    result = resolve_application(args.query, include_nodisplay=args.include_nodisplay)
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result.get("matched") else 1


if __name__ == "__main__":
    raise SystemExit(main())
