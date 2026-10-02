import asyncio
import configparser
import logging
import os
import re
import shlex
import shutil
import subprocess
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, Optional

from rapidfuzz import fuzz

from src.linux.screen import get_primary_screen_rect

logger = logging.getLogger(__name__)

_APP_INDEX: Optional[list[dict]] = None
WINDOW_CMD_TIMEOUT = 1.5
WINDOW_ACTIVATION_VERIFY_TIMEOUT = 0.35
WINDOW_ACTIVATION_POLL_INTERVAL = 0.05
DESKTOP_LAUNCHER_EXIT_TIMEOUT = 2.0
DIRECT_EXEC_EXIT_CHECK_TIMEOUT = 0.8
APP_RESOLVE_MIN_SCORE = 88
APP_RESOLVE_AMBIGUITY_MARGIN = 10
APP_RESOLVE_CANDIDATE_LIMIT = 5
APP_PRERELEASE_SCORE_PENALTY = 20
APP_PRERELEASE_MARKERS = ("beta", "alpha", "nightly", "preview", "dev")
EXEC_WRAPPER_BASENAMES = {
    "bash",
    "env",
    "flatpak",
    "java",
    "kaiming",
    "python",
    "python3",
    "sh",
    "snap",
    "wine",
    "wine64",
}

COMMON_APP_ALIASES: dict[str, list[str]] = {
    "firefox": ["firefox", "mozilla firefox"],
    "google chrome": ["google chrome", "google-chrome", "chrome"],
    "chrome": ["google chrome", "google-chrome", "chrome"],
    "chromium": ["chromium", "chromium browser", "chromium-browser"],
    "edge": ["microsoft edge", "microsoft-edge", "edge"],
    "vscode": ["visual studio code", "vscode", "code"],
    "visual studio code": ["visual studio code", "vscode", "code"],
    "火狐": ["firefox", "mozilla firefox"],
    "火狐浏览器": ["firefox", "mozilla firefox"],
    "火狐浏览器(官方版)": ["firefox", "mozilla firefox"],
    "浏览器": ["web browser", "browser", "firefox", "chromium"],
    "记事本": ["文本编辑器", "text editor", "editor", "pluma"],
    "文本编辑器": ["text editor", "editor", "pluma"],
    "text editor": ["text editor", "pluma", "gedit", "kate", "kwrite", "xed", "mousepad", "featherpad"],
    "终端": [
        "terminal",
        "terminal emulator",
        "gnome terminal",
        "konsole",
    ],
    "文件管理器": ["files", "file manager", "dolphin", "nautilus"],
    "微信": ["wechat", "weixin"],
    "terminal": [
        "terminal",
        "terminal emulator",
        "gnome terminal",
        "konsole",
        "xfce terminal",
        "kitty",
        "alacritty",
        "wezterm",
    ],
    "files": [
        "files",
        "file manager",
        "nautilus",
        "dolphin",
        "thunar",
        "nemo",
        "pcmanfm",
    ],
}

DIRECT_COMMAND_HINTS: dict[str, list[str]] = {
    "firefox": ["firefox"],
    "google chrome": ["google-chrome", "google-chrome-stable", "chrome"],
    "chrome": ["google-chrome", "google-chrome-stable", "chrome"],
    "chromium": ["chromium", "chromium-browser"],
    "edge": ["microsoft-edge", "microsoft-edge-stable"],
    "vscode": ["code", "codium"],
    "visual studio code": ["code", "codium"],
    "terminal": [
        "x-terminal-emulator",
        "gnome-terminal",
        "konsole",
        "xfce4-terminal",
        "kitty",
        "alacritty",
        "wezterm",
        "tilix",
        "mate-terminal",
        "lxterminal",
    ],
    "files": ["nautilus", "dolphin", "thunar", "nemo", "pcmanfm"],
    "记事本": ["pluma", "gedit", "kate", "kwrite", "xed", "mousepad", "featherpad", "l3afpad"],
    "文本编辑器": ["pluma", "gedit", "kate", "kwrite", "xed", "mousepad", "featherpad", "l3afpad"],
    "text editor": ["pluma", "gedit", "kate", "kwrite", "xed", "mousepad", "featherpad", "l3afpad"],
}

TERMINAL_WRAPPERS: list[tuple[str, list[str], bool]] = [
    ("x-terminal-emulator", ["-e"], False),
    ("gnome-terminal", ["--"], False),
    ("konsole", ["-e"], False),
    ("xfce4-terminal", ["--command"], True),
    ("kitty", ["--"], False),
    ("alacritty", ["-e"], False),
    ("wezterm", ["start", "--"], False),
    ("tilix", ["-e"], True),
    ("mate-terminal", ["-e"], True),
    ("lxterminal", ["-e"], True),
]


def _parse_bool(value: Optional[str]) -> bool:
    return (value or "").strip().lower() in {"1", "true", "yes", "on"}


def _normalize_text(value: str) -> str:
    normalized = value.strip().lower()
    normalized = normalized.replace("_", " ").replace("-", " ").replace(".", " ")
    normalized = re.sub(r"\s+", " ", normalized)
    return normalized.strip()


def _run_quick(args: list[str], **kwargs) -> subprocess.CompletedProcess:
    """Run best-effort window-control commands without allowing them to hang."""
    kwargs.setdefault("check", False)
    kwargs.setdefault("timeout", WINDOW_CMD_TIMEOUT)
    return subprocess.run(args, **kwargs)


def _unique_keep_order(values: list[str]) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for value in values:
        if value and value not in seen:
            seen.add(value)
            out.append(value)
    return out


def _xdg_application_dirs() -> list[Path]:
    xdg_data_home = Path(
        os.environ.get("XDG_DATA_HOME", str(Path.home() / ".local/share"))
    )
    xdg_data_dirs = [
        Path(p)
        for p in os.environ.get("XDG_DATA_DIRS", "/usr/local/share:/usr/share").split(":")
        if p
    ]

    candidates = [xdg_data_home / "applications"]
    candidates.extend(base / "applications" for base in xdg_data_dirs)

    candidates.extend(
        [
            Path("/var/lib/snapd/desktop/applications"),
            Path("/var/lib/flatpak/exports/share/applications"),
            Path.home() / ".local/share/flatpak/exports/share/applications",
        ]
    )

    deduped: list[Path] = []
    seen: set[str] = set()
    for path in candidates:
        key = str(path)
        if key not in seen:
            seen.add(key)
            deduped.append(path)
    return deduped


APPLICATION_DIRS = _xdg_application_dirs()


def _locale_candidates() -> list[str]:
    raw_values: list[str] = []
    for env_name in ("LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"):
        raw = os.environ.get(env_name, "")
        if raw:
            raw_values.extend(part for part in raw.split(":") if part)

    candidates: list[str] = []
    for raw in raw_values:
        value = raw.split(".", 1)[0]
        value = value.split("@", 1)[0]
        if not value:
            continue
        candidates.append(value)
        if "_" in value:
            candidates.append(value.split("_", 1)[0])

    return _unique_keep_order(candidates)


def _get_localized_value(section: configparser.SectionProxy, key: str) -> str:
    for locale_key in _locale_candidates():
        candidate = f"{key}[{locale_key}]"
        if candidate in section and section.get(candidate, "").strip():
            return section.get(candidate, "").strip()

    if key in section and section.get(key, "").strip():
        return section.get(key, "").strip()

    for candidate in section.keys():
        if candidate.startswith(f"{key}[") and section.get(candidate, "").strip():
            return section.get(candidate, "").strip()

    return ""


def _get_all_localized_values(
    section: configparser.SectionProxy,
    key: str,
) -> list[str]:
    """Collect the base value and every localized variant from a desktop file."""
    values: list[str] = []
    if key in section and section.get(key, "").strip():
        values.append(section.get(key, "").strip())
    for candidate in section.keys():
        if candidate.startswith(f"{key}["):
            value = section.get(candidate, "").strip()
            if value:
                values.append(value)
    localized = _get_localized_value(section, key)
    if localized:
        values.insert(0, localized)
    return _unique_keep_order(values)


def _split_desktop_list(raw: str) -> list[str]:
    return [item.strip() for item in raw.split(";") if item.strip()]


def _current_desktops() -> set[str]:
    raw = os.environ.get("XDG_CURRENT_DESKTOP", "")
    return {
        part.strip().lower()
        for part in re.split(r"[:;]", raw)
        if part and part.strip()
    }


def _should_show_in_current_desktop(section: configparser.SectionProxy) -> bool:
    current = _current_desktops()
    if not current:
        return True

    only_show = {item.lower() for item in _split_desktop_list(section.get("OnlyShowIn", ""))}
    not_show = {item.lower() for item in _split_desktop_list(section.get("NotShowIn", ""))}

    if only_show and current.isdisjoint(only_show):
        return False
    if not_show and not current.isdisjoint(not_show):
        return False
    return True


def _desktop_id_from_path(path: Path, root: Path) -> str:
    try:
        relative = path.relative_to(root).as_posix()
    except ValueError:
        relative = path.name
    return relative.replace("/", "-")


def _is_executable_available(command: str) -> bool:
    command = command.strip()
    if not command:
        return False

    expanded = os.path.expanduser(os.path.expandvars(command))
    if os.path.isabs(expanded):
        return os.path.isfile(expanded) and os.access(expanded, os.X_OK)
    return shutil.which(expanded) is not None


def _strip_exec_field_codes(exec_cmd: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(exec_cmd):
        ch = exec_cmd[i]
        if ch == "%" and i + 1 < len(exec_cmd):
            nxt = exec_cmd[i + 1]
            if nxt == "%":
                out.append("%")
                i += 2
                continue
            if nxt.isalpha():
                i += 2
                continue
        out.append(ch)
        i += 1
    return re.sub(r"\s+", " ", "".join(out)).strip()


def _split_exec_args(exec_cmd: str) -> list[str]:
    cleaned = _strip_exec_field_codes(exec_cmd)
    if not cleaned:
        return []
    try:
        return shlex.split(cleaned)
    except Exception:
        return []


def _exec_basename(exec_cmd: str) -> str:
    args = _split_exec_args(exec_cmd)
    if not args:
        return ""
    return Path(args[0]).name.lower().replace(".desktop", "").replace(".exe", "")


def _parse_desktop_file(path: Path, root: Path) -> Optional[dict]:
    parser = configparser.ConfigParser(interpolation=None, strict=False)
    parser.optionxform = str

    try:
        with path.open("r", encoding="utf-8", errors="ignore") as f:
            parser.read_file(f)
    except Exception as e:
        logger.debug("Failed to parse desktop file %s: %s", path, e)
        return None

    if "Desktop Entry" not in parser:
        return None

    section = parser["Desktop Entry"]

    if section.get("Type", "Application").strip() != "Application":
        return None
    if _parse_bool(section.get("Hidden")):
        return None
    if _parse_bool(section.get("NoDisplay")):
        return None
    if not _should_show_in_current_desktop(section):
        return None

    name = _get_localized_value(section, "Name")
    if not name:
        return None

    try_exec = section.get("TryExec", "").strip()
    if try_exec and not _is_executable_available(try_exec):
        return None

    dbus_activatable = _parse_bool(section.get("DBusActivatable"))
    exec_cmd = section.get("Exec", "").strip()
    exec_cmd = _strip_exec_field_codes(exec_cmd)

    if not exec_cmd and not dbus_activatable:
        return None

    working_dir = os.path.expanduser(os.path.expandvars(section.get("Path", "").strip()))
    if not working_dir:
        working_dir = ""

    keywords = _split_desktop_list(_get_localized_value(section, "Keywords"))

    return {
        "name": name,
        "localized_names": _get_all_localized_values(section, "Name"),
        "generic_name": _get_localized_value(section, "GenericName"),
        "comment": _get_localized_value(section, "Comment"),
        "keywords": keywords,
        "exec": exec_cmd,
        "try_exec": try_exec,
        "path": working_dir,
        "terminal": _parse_bool(section.get("Terminal")),
        "startup_wm_class": section.get("StartupWMClass", "").strip(),
        "dbus_activatable": dbus_activatable,
        "x_kaiming": section.get("X-Kaiming", "").strip(),
        "desktop_path": str(path),
        "desktop_id": _desktop_id_from_path(path, root),
        "source_dir": str(root),
    }


def _scan_applications() -> list[dict]:
    apps: list[dict] = []
    seen_desktop_ids: set[str] = set()

    for directory in APPLICATION_DIRS:
        if not directory.exists():
            continue

        try:
            desktop_files = directory.rglob("*.desktop")
        except Exception as e:
            logger.debug("Failed to iterate application directory %s: %s", directory, e)
            continue

        for desktop_file in desktop_files:
            if not desktop_file.is_file():
                continue

            app = _parse_desktop_file(desktop_file, directory)
            if not app:
                continue

            desktop_id = app["desktop_id"]
            if desktop_id in seen_desktop_ids:
                continue

            seen_desktop_ids.add(desktop_id)
            apps.append(app)

    return apps


def build_index(force: bool = True) -> None:
    global _APP_INDEX
    if _APP_INDEX is None or force:
        _APP_INDEX = _scan_applications()


def _load_index(force: bool = False) -> list[dict]:
    if _APP_INDEX is None or force:
        build_index(force=True)
    return _APP_INDEX or []


def list_applications() -> list[str]:
    names = sorted({app["name"] for app in _load_index(force=False)}, key=str.lower)
    return names


def _record_search_fields(app: dict) -> list[str]:
    desktop_id = app.get("desktop_id", "")
    values: list[str] = [
        app.get("name", ""),
        app.get("generic_name", ""),
        app.get("comment", ""),
        app.get("startup_wm_class", ""),
        desktop_id,
        desktop_id.removesuffix(".desktop"),
        _exec_basename(app.get("exec", "")),
    ]
    values.extend(app.get("localized_names", []))
    values.extend(app.get("keywords", []))
    return [v for v in (_normalize_text(x) for x in values) if v]


def _expanded_queries(app_name: str) -> list[str]:
    normalized_name = _normalize_text(app_name)
    queries = [normalized_name]

    queries.extend(_normalize_text(alias) for alias in COMMON_APP_ALIASES.get(normalized_name, []))
    queries.extend(_normalize_text(cmd) for cmd in DIRECT_COMMAND_HINTS.get(normalized_name, []))

    return _unique_keep_order([q for q in queries if q])


def _score_record(query: str, app: dict) -> int:
    fields = _record_search_fields(app)
    if not fields:
        return 0

    score = 0
    normalized_name = _normalize_text(app.get("name", ""))
    exec_name = _normalize_text(_exec_basename(app.get("exec", "")))
    startup_wm_class = _normalize_text(app.get("startup_wm_class", ""))

    if query == normalized_name:
        score = max(score, 140)
    if exec_name and query == exec_name:
        score = max(score, 132)
    if startup_wm_class and query == startup_wm_class:
        score = max(score, 130)

    for field in fields:
        if query == field:
            score = max(score, 125)
        elif field.startswith(query) or query.startswith(field):
            score = max(score, 108)
        elif query in field:
            score = max(score, 96)
        else:
            fuzzy_score = int(fuzz.WRatio(query, field))
            score = max(score, fuzzy_score)

    app_identity = " ".join(
        (
            _normalize_text(app.get("name", "")),
            _normalize_text(app.get("desktop_id", "")),
            _normalize_text(_exec_basename(app.get("exec", ""))),
        )
    )
    is_prerelease = any(marker in app_identity for marker in APP_PRERELEASE_MARKERS)
    prerelease_requested = any(marker in query for marker in APP_PRERELEASE_MARKERS)
    if is_prerelease and not prerelease_requested:
        score = max(0, score - APP_PRERELEASE_SCORE_PENALTY)

    return score


@dataclass(frozen=True)
class AppCandidate:
    app: dict
    score: int
    matched_query: str


@dataclass(frozen=True)
class AppResolution:
    status: Literal["resolved", "ambiguous", "not_found"]
    query: str
    selected: Optional[AppCandidate]
    candidates: tuple[AppCandidate, ...]


def _rank_app_candidates(name: str, index: list[dict]) -> list[AppCandidate]:
    queries = _expanded_queries(name)
    ranked: list[AppCandidate] = []
    for app in index:
        scored_queries = [(_score_record(query, app), query) for query in queries]
        score, matched_query = max(scored_queries, default=(0, ""))
        if score > 0:
            ranked.append(
                AppCandidate(
                    app=app,
                    score=score,
                    matched_query=matched_query,
                )
            )
    ranked.sort(
        key=lambda candidate: (
            -candidate.score,
            _normalize_text(candidate.app.get("name", "")),
            candidate.app.get("desktop_id", ""),
        )
    )
    return ranked


def _app_identity_tokens(app: dict) -> set[str]:
    return {
        token
        for token in (
            _normalize_text(app.get("startup_wm_class", "")),
            _normalize_text(_exec_basename(app.get("exec", ""))),
        )
        if token
    }


def _same_app_family(first: dict, second: dict) -> bool:
    first_tokens = _app_identity_tokens(first)
    second_tokens = _app_identity_tokens(second)
    return bool(first_tokens and second_tokens and first_tokens.intersection(second_tokens))


def _resolve_from_index(name: str, index: list[dict]) -> AppResolution:
    query = name.strip()
    ranked = _rank_app_candidates(query, index)
    visible_candidates = tuple(ranked[:APP_RESOLVE_CANDIDATE_LIMIT])
    if not ranked or ranked[0].score < APP_RESOLVE_MIN_SCORE:
        return AppResolution(
            status="not_found",
            query=query,
            selected=None,
            candidates=visible_candidates,
        )

    selected = ranked[0]
    competitor = next(
        (
            candidate
            for candidate in ranked[1:]
            if not _same_app_family(selected.app, candidate.app)
        ),
        None,
    )
    if (
        competitor is not None
        and selected.score - competitor.score < APP_RESOLVE_AMBIGUITY_MARGIN
    ):
        return AppResolution(
            status="ambiguous",
            query=query,
            selected=None,
            candidates=visible_candidates,
        )

    return AppResolution(
        status="resolved",
        query=query,
        selected=selected,
        candidates=visible_candidates,
    )


def resolve_application(name: str) -> AppResolution:
    query = name.strip()
    if not query:
        return AppResolution(
            status="not_found",
            query="",
            selected=None,
            candidates=(),
        )

    resolution = _resolve_from_index(query, _load_index(force=False))
    if resolution.status == "resolved":
        return resolution
    return _resolve_from_index(query, _load_index(force=True))


def resolve_app(name: str) -> Optional[dict]:
    """Backward-compatible wrapper returning only an unambiguous match."""
    resolution = resolve_application(name)
    return resolution.selected.app if resolution.selected else None


def _format_resolution_candidates(candidates: tuple[AppCandidate, ...]) -> str:
    if not candidates:
        return "none"
    return ", ".join(
        (
            f"{candidate.app.get('name', '?')}"
            f"[{candidate.app.get('desktop_id', 'no-desktop-id')};"
            f"score={candidate.score}]"
        )
        for candidate in candidates
    )


def _build_match_tokens(app_name: str, rec: Optional[dict]) -> list[str]:
    tokens: list[str] = [_normalize_text(app_name)]
    tokens.extend(_expanded_queries(app_name))

    if rec:
        tokens.extend(
            [
                _normalize_text(rec.get("name", "")),
                _normalize_text(rec.get("generic_name", "")),
                _normalize_text(rec.get("startup_wm_class", "")),
                _normalize_text(rec.get("desktop_id", "").replace(".desktop", "")),
                _normalize_text(_exec_basename(rec.get("exec", ""))),
            ]
        )

    # Desktop Keywords are useful when resolving a human app name, but are not
    # window identity. For example Pluma advertises the keyword "MATE", which
    # also appears in mate-terminal's WM_CLASS. Including it here can activate a
    # completely different application instead of launching the selected one.

    if "firefox" in tokens:
        tokens.extend(
            [
                "mozilla firefox",
                "navigator firefox",
                "navigator.firefox",
                "firefox firefox",
            ]
        )
    if "google chrome" in tokens or "chrome" in tokens:
        tokens.extend(["google chrome", "google-chrome", "chrome"])
    if "visual studio code" in tokens or "vscode" in tokens or "code" in tokens:
        tokens.extend(["visual studio code", "vscode", "code"])

    deduped = _unique_keep_order([t for t in tokens if t])
    deduped.sort(key=lambda item: (-len(item), item))
    return deduped


def _build_window_identity_tokens(app_name: str, rec: Optional[dict]) -> list[str]:
    """Build tokens that may safely identify a process/window class.

    Semantic aliases, generic names and Desktop Keywords deliberately stay out
    of this list. They are appropriate for selecting a desktop entry, not for
    deciding that an existing window belongs to that selected application.
    """
    if rec:
        values = [
            rec.get("startup_wm_class", ""),
            rec.get("desktop_id", "").removesuffix(".desktop"),
            _exec_basename(rec.get("exec", "")),
        ]
    else:
        values = [app_name, *_expanded_queries(app_name)]
        values.extend(_direct_command_candidates(app_name))

    tokens = _unique_keep_order(
        _normalize_text(value)
        for value in values
        if value and _normalize_text(value)
    )
    tokens.sort(key=lambda item: (-len(item), item))
    return tokens


def _can_use_wmctrl() -> bool:
    return shutil.which("wmctrl") is not None


def _can_use_xdotool() -> bool:
    return shutil.which("xdotool") is not None


def _has_window_control_backend() -> bool:
    return _can_use_wmctrl() or _can_use_xdotool()


def _prepare_primary_screen_launch_context() -> None:
    """Best-effort: move pointer to primary monitor so new windows open there."""
    try:
        import pyautogui

        x, y, w, h = get_primary_screen_rect()
        target_x = int(x + (w * 0.5))
        target_y = int(y + (h * 0.5))
        pyautogui.moveTo(target_x, target_y, duration=0.05)
        pyautogui.click(target_x, target_y)
    except Exception:
        # Launch should continue even if this hint cannot be applied.
        return


async def _request_running_app_activation(rec: Optional[dict], query: str) -> tuple[bool, str]:
    """
    Best-effort activation request when no window-control backend is available.
    Uses desktop launchers which may route to an existing single-instance app.
    """
    if rec:
        ok, msg = await _launch_desktop_entry(rec)
        if ok:
            return True, f"activation request sent via desktop entry ({msg})"
        return False, msg

    for candidate in _direct_command_candidates(query):
        ok, msg = await _launch_exec(candidate)
        if ok:
            return True, f"activation request sent via command ({candidate})"
    return False, "no activation request path available"


def _list_windows() -> list[dict]:
    windows: list[dict] = []

    if _can_use_wmctrl():
        try:
            proc = _run_quick(
                ["wmctrl", "-lxp"],
                capture_output=True,
                text=True,
            )
            if proc.returncode == 0:
                for line in proc.stdout.splitlines():
                    parts = line.split(None, 5)
                    if len(parts) < 5:
                        continue
                    pid = ""
                    if len(parts) >= 5:
                        pid = parts[2]
                    # wmctrl -lxp columns are:
                    # id, desktop, pid, WM_CLASS, client-machine, title
                    wm_class = parts[3] if len(parts) >= 4 else ""
                    title = parts[5] if len(parts) > 5 else ""
                    windows.append(
                        {
                            "id": parts[0],
                            "pid": pid,
                            "wm_class": wm_class,
                            "title": title,
                        }
                    )
                return windows
        except Exception:
            pass

    if not _can_use_xdotool():
        return []

    try:
        proc = _run_quick(
            ["xdotool", "search", "--all", "--name", ".*"],
            capture_output=True,
            text=True,
        )
    except Exception:
        return []

    if proc.returncode != 0:
        return []

    for raw_id in proc.stdout.splitlines():
        raw_id = raw_id.strip()
        if not raw_id:
            continue

        try:
            win_id = hex(int(raw_id))
        except Exception:
            continue

        title = ""
        wm_class = ""

        try:
            name_proc = _run_quick(
                ["xdotool", "getwindowname", raw_id],
                capture_output=True,
                text=True,
            )
            if name_proc.returncode == 0:
                title = (name_proc.stdout or "").strip()
        except Exception:
            pass

        if shutil.which("xprop") is not None:
            try:
                class_proc = _run_quick(
                    ["xprop", "-id", str(int(raw_id)), "WM_CLASS"],
                    capture_output=True,
                    text=True,
                )
                if class_proc.returncode == 0:
                    # Example: WM_CLASS(STRING) = "Navigator", "firefox"
                    classes = re.findall(r'"([^"]+)"', class_proc.stdout or "")
                    if classes:
                        wm_class = ".".join(classes)
            except Exception:
                pass

        windows.append(
            {
                "id": win_id,
                "pid": "",
                "wm_class": wm_class,
                "title": title,
            }
        )

    return windows


def _normalize_window_id(value: str) -> str:
    raw = (value or "").strip().lower()
    if not raw:
        return ""
    try:
        base = 16 if raw.startswith("0x") else 10
        return hex(int(raw, base))
    except Exception:
        return raw


def _active_window_id() -> str:
    """Return the canonical X11 id of the currently active window."""
    if shutil.which("xprop") is not None:
        try:
            proc = _run_quick(
                ["xprop", "-root", "_NET_ACTIVE_WINDOW"],
                capture_output=True,
                text=True,
            )
            if proc.returncode == 0:
                ids = re.findall(r"0x[0-9a-fA-F]+", proc.stdout or "")
                if ids:
                    active_id = _normalize_window_id(ids[-1])
                    if active_id != "0x0":
                        return active_id
        except Exception:
            pass

    if _can_use_xdotool():
        try:
            proc = _run_quick(
                ["xdotool", "getactivewindow"],
                capture_output=True,
                text=True,
            )
            if proc.returncode == 0 and (proc.stdout or "").strip():
                return _normalize_window_id((proc.stdout or "").strip())
        except Exception:
            pass
    return ""


def _window_hidden_state(window_id: str) -> Optional[bool]:
    """Return True/False when EWMH hidden state is available, otherwise None."""
    if shutil.which("xprop") is None:
        return None
    try:
        proc = _run_quick(
            ["xprop", "-id", _normalize_window_id(window_id), "_NET_WM_STATE"],
            capture_output=True,
            text=True,
        )
    except Exception:
        return None
    if proc.returncode != 0:
        return None
    output = proc.stdout or ""
    if "_NET_WM_STATE" not in output or "not found" in output.lower():
        return None
    return "_NET_WM_STATE_HIDDEN" in output


def _window_viewable_state(window_id: str) -> Optional[bool]:
    """Return whether X reports the window as viewable, if xwininfo is present."""
    if shutil.which("xwininfo") is None:
        return None
    try:
        proc = _run_quick(
            ["xwininfo", "-id", _normalize_window_id(window_id)],
            capture_output=True,
            text=True,
        )
    except Exception:
        return None
    if proc.returncode != 0:
        return None
    match = re.search(r"Map State:\s*(\S+)", proc.stdout or "", re.IGNORECASE)
    if not match:
        return None
    return match.group(1).lower() == "isviewable"


def _window_is_front_and_usable(window_id: str) -> bool:
    """Require the target to be active and not known to be hidden/unviewable."""
    target_id = _normalize_window_id(window_id)
    if not target_id or _active_window_id() != target_id:
        return False
    if _window_hidden_state(target_id) is True:
        return False
    if _window_viewable_state(target_id) is False:
        return False
    return True


def _wait_for_window_front(
    window_id: str,
    timeout: float = WINDOW_ACTIVATION_VERIFY_TIMEOUT,
) -> bool:
    deadline = time.monotonic() + max(0.0, timeout)
    while True:
        if _window_is_front_and_usable(window_id):
            return True
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return False
        time.sleep(min(WINDOW_ACTIVATION_POLL_INTERVAL, remaining))


def _current_desktop_index() -> Optional[str]:
    if _can_use_xdotool():
        try:
            proc = _run_quick(
                ["xdotool", "get_desktop"],
                capture_output=True,
                text=True,
            )
            value = (proc.stdout or "").strip()
            if proc.returncode == 0 and value.isdigit():
                return value
        except Exception:
            pass

    if _can_use_wmctrl():
        try:
            proc = _run_quick(["wmctrl", "-d"], capture_output=True, text=True)
            if proc.returncode == 0:
                for line in (proc.stdout or "").splitlines():
                    parts = line.split()
                    if len(parts) >= 2 and parts[1] == "*" and parts[0].isdigit():
                        return parts[0]
        except Exception:
            pass
    return None


def _window_stacking_order() -> dict[str, int]:
    if shutil.which("xprop") is None:
        return {}

    try:
        proc = _run_quick(
            ["xprop", "-root", "_NET_CLIENT_LIST_STACKING"],
            capture_output=True,
            text=True,
        )
    except Exception:
        return {}

    if proc.returncode != 0:
        return {}

    # xprop returns ids from bottom -> top, we use larger index as more recent/top.
    ids = re.findall(r"0x[0-9a-fA-F]+", proc.stdout or "")
    return {_normalize_window_id(win_id): idx for idx, win_id in enumerate(ids)}


def _window_score(
    window: dict,
    title_tokens: list[str],
    class_tokens: Optional[list[str]] = None,
) -> int:
    wm_class = _normalize_text(window.get("wm_class", ""))
    title = _normalize_text(window.get("title", ""))
    identity_tokens = title_tokens if class_tokens is None else class_tokens

    best = 0
    for token in identity_tokens:
        if not token:
            continue

        if wm_class and token == wm_class:
            best = max(best, 140)
        elif wm_class and token in wm_class:
            best = max(best, 118)
        elif len(token) >= 4 and wm_class:
            best = max(best, int(fuzz.WRatio(token, wm_class) * 0.82))

    for token in title_tokens:
        if not token:
            continue
        if title and token == title:
            best = max(best, 102)
        elif title and token in title:
            best = max(best, 82)
        elif len(token) >= 4 and title:
            best = max(best, int(fuzz.WRatio(token, title) * 0.60))

    return best


def _find_matching_window(
    tokens: list[str],
    class_tokens: Optional[list[str]] = None,
) -> Optional[str]:
    windows = _list_windows()
    if not windows or not tokens:
        return None

    stacking = _window_stacking_order()
    best_id: Optional[str] = None
    best_score = 0
    best_stack = -1
    normalized_tokens = [_normalize_text(t) for t in tokens if t]
    normalized_class_tokens = (
        [_normalize_text(t) for t in class_tokens if t]
        if class_tokens is not None
        else None
    )

    for window in windows:
        score = _window_score(window, normalized_tokens, normalized_class_tokens)
        if score < 90:
            continue

        window_id = _normalize_window_id(window.get("id", ""))
        stack_rank = stacking.get(window_id, -1)

        if score > best_score or (score == best_score and stack_rank > best_stack):
            best_score = score
            best_stack = stack_rank
            best_id = window_id or window["id"]

    if best_id:
        return best_id

    # Fallback: relaxed containment matching to avoid false "not running" cases.
    # This prefers the most recently used/top window when multiple windows match.
    for window in windows:
        wm_class = _normalize_text(window.get("wm_class", ""))
        title = _normalize_text(window.get("title", ""))
        if not wm_class and not title:
            continue

        if normalized_class_tokens is None:
            joined = f"{wm_class} {title}".strip()
            matched = any(
                token and len(token) >= 2 and token in joined
                for token in normalized_tokens
            )
        else:
            # Once a desktop entry has been resolved, only strong identity may
            # match WM_CLASS. A semantic title is accepted only when it is exact.
            matched = any(
                token and len(token) >= 2 and token in wm_class
                for token in normalized_class_tokens
            ) or any(token and token == title for token in normalized_tokens)
        if not matched:
            continue

        window_id = _normalize_window_id(window.get("id", ""))
        stack_rank = stacking.get(window_id, -1)
        if stack_rank > best_stack:
            best_stack = stack_rank
            best_id = window_id or window["id"]

    if best_id:
        return best_id
    return None


def _activate_window_by_id(window_id: str) -> bool:
    raw = _normalize_window_id(window_id)
    if not raw:
        return False
    numeric = str(int(raw, 16)) if raw.startswith("0x") else raw

    if _can_use_wmctrl():
        try:
            _run_quick(
                ["wmctrl", "-ir", window_id, "-b", "remove,hidden"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            result = _run_quick(
                ["wmctrl", "-ia", raw],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            if result.returncode == 0 and _wait_for_window_front(raw):
                return True
        except Exception:
            pass

    # Recovery after a command-level success that did not actually foreground the
    # app. Hidden/off-desktop windows are common with KWin and single-instance
    # apps, so move the known window to the current desktop before the stronger
    # map/raise/synchronous-activate sequence.
    logger.warning("Window %s was not foregrounded; attempting activation recovery.", raw)
    if _can_use_wmctrl():
        try:
            desktop = _current_desktop_index()
            if desktop is not None:
                _run_quick(
                    ["wmctrl", "-ir", raw, "-t", desktop],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
            _run_quick(
                ["wmctrl", "-ir", raw, "-b", "remove,hidden"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        except Exception:
            pass

    if _can_use_xdotool():
        try:
            _run_quick(
                ["xdotool", "windowmap", numeric],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            _run_quick(
                ["xdotool", "windowraise", numeric],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            result = _run_quick(
                ["xdotool", "windowactivate", "--sync", numeric],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            if result.returncode == 0 and _wait_for_window_front(raw):
                logger.info("Window %s foreground activation recovered.", raw)
                return True
        except Exception:
            pass

    # xdotool may be absent. Give the unhide/current-desktop recovery one final
    # EWMH activation attempt, but still require the same postcondition.
    if _can_use_wmctrl():
        try:
            result = _run_quick(
                ["wmctrl", "-ia", raw],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            if result.returncode == 0 and _wait_for_window_front(raw):
                logger.info("Window %s foreground activation recovered via wmctrl.", raw)
                return True
        except Exception:
            pass
    return False


def _move_window_to_primary_screen(window_id: str) -> bool:
    try:
        x, y, _, _ = get_primary_screen_rect()
    except Exception:
        return False

    if _can_use_wmctrl():
        try:
            result = _run_quick(
                ["wmctrl", "-ir", window_id, "-e", f"0,{x},{y},-1,-1"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            if result.returncode == 0:
                return True
        except Exception:
            pass

    if not _can_use_xdotool():
        return False

    raw = _normalize_window_id(window_id)
    try:
        numeric = str(int(raw, 16)) if raw.startswith("0x") else raw
        result = _run_quick(
            ["xdotool", "windowmove", numeric, str(x), str(y)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return result.returncode == 0
    except Exception:
        return False


async def _wait_for_window(
    tokens: list[str],
    attempts: int = 40,
    delay: float = 0.1,
    move_to_primary: bool = False,
    class_tokens: Optional[list[str]] = None,
) -> bool:
    last_activation_attempt = -attempts
    last_window_id = ""
    retry_every = max(1, int(0.8 / max(delay, 0.01)))
    for attempt in range(attempts):
        win_id = _find_matching_window(tokens, class_tokens)
        if win_id:
            if _window_is_front_and_usable(win_id):
                return True
            normalized_id = _normalize_window_id(win_id)
            should_activate = (
                normalized_id != last_window_id
                or attempt - last_activation_attempt >= retry_every
            )
            if should_activate:
                if move_to_primary:
                    _move_window_to_primary_screen(win_id)
                if _activate_window_by_id(win_id):
                    return True
                last_window_id = normalized_id
                last_activation_attempt = attempt
        await asyncio.sleep(delay)
    return False


def _launch_subprocess(
    args: list[str],
    cwd: Optional[str] = None,
) -> tuple[bool, str]:
    if not args:
        return False, "empty command"

    try:
        subprocess.Popen(
            args,
            cwd=cwd or None,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            start_new_session=True,
        )
        return True, "launched"
    except Exception as e:
        return False, f"launch failed: {e}"


async def _launch_subprocess_checked(
    args: list[str],
    cwd: Optional[str] = None,
    timeout: float = DESKTOP_LAUNCHER_EXIT_TIMEOUT,
) -> tuple[bool, str]:
    """Start a process and surface a quick non-zero exit without blocking on apps."""
    if not args:
        return False, "empty command"

    # Create the child on the event-loop thread. Some desktop/Qt runtimes can
    # deadlock when fork/exec itself is initiated from an asyncio worker thread.
    output = tempfile.TemporaryFile(mode="w+t", encoding="utf-8", errors="replace")
    try:
        process = subprocess.Popen(
            args,
            cwd=cwd or None,
            stdout=output,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
    except Exception as e:
        output.close()
        return False, f"launch failed: {e}"

    try:
        try:
            # This wait is deliberately bounded. Reaping a desktop child from
            # an asyncio worker thread hangs on some Kylin/Qt runtimes.
            return_code = process.wait(timeout=max(0.0, timeout))
        except subprocess.TimeoutExpired:
            return True, f"started and still running after {timeout:.1f}s"

        if return_code == 0:
            return True, "launcher exited successfully"

        output.flush()
        output.seek(0)
        detail = re.sub(r"\s+", " ", output.read(1200)).strip()
        suffix = f": {detail}" if detail else ""
        return False, f"launcher exited with code {return_code}{suffix}"
    finally:
        # The child keeps its own descriptor if it outlives the observation
        # window; closing the parent's temporary file cannot block the child.
        output.close()


async def _launch_exec(
    exec_cmd: str,
    cwd: Optional[str] = None,
    check_early_exit: bool = False,
) -> tuple[bool, str]:
    args = _split_exec_args(exec_cmd)
    if check_early_exit:
        return await _launch_subprocess_checked(
            args,
            cwd=cwd,
            timeout=DIRECT_EXEC_EXIT_CHECK_TIMEOUT,
        )
    return _launch_subprocess(args, cwd=cwd)


def _is_wrapper_exec(rec: dict) -> bool:
    """Identify desktop entries whose fixed argv is essential to launching."""
    args = _split_exec_args(rec.get("exec", ""))
    if len(args) < 2:
        return False
    if rec.get("x_kaiming"):
        return True

    executable = Path(args[0]).name.lower()
    if executable in EXEC_WRAPPER_BASENAMES:
        return True
    return re.fullmatch(r"python\d+(?:\.\d+)?", executable) is not None


async def _launch_in_terminal(exec_cmd: str, cwd: Optional[str] = None) -> tuple[bool, str]:
    args = _split_exec_args(exec_cmd)
    if not args:
        return False, "empty command"

    last_error = "no terminal wrapper found"
    cmd_str = " ".join(shlex.quote(arg) for arg in args)

    for terminal_bin, prefix, expects_single_string in TERMINAL_WRAPPERS:
        if not shutil.which(terminal_bin):
            continue

        if expects_single_string:
            launch_cmd = [terminal_bin, *prefix, cmd_str]
        else:
            launch_cmd = [terminal_bin, *prefix, *args]

        ok, msg = _launch_subprocess(launch_cmd, cwd=cwd)
        if ok:
            return True, f"launched via {terminal_bin}"
        last_error = msg

    return False, last_error


async def _launch_desktop_entry(rec: dict) -> tuple[bool, str]:
    desktop_path = rec.get("desktop_path", "")
    desktop_id = rec.get("desktop_id", "")
    cwd = rec.get("path") or None
    exec_cmd = rec.get("exec", "")
    launch_errors: list[str] = []
    direct_wrapper_attempted = False

    # Kylin's GIO -> ProcessManager integration can reduce a wrapped Exec line
    # to argv[0]. Start known wrappers with the complete parsed argv first.
    if exec_cmd and not rec.get("terminal") and _is_wrapper_exec(rec):
        direct_wrapper_attempted = True
        ok, msg = await _launch_exec(
            exec_cmd,
            cwd=cwd,
            check_early_exit=True,
        )
        if ok:
            wrapper = Path(_split_exec_args(exec_cmd)[0]).name
            return True, f"direct wrapper exec via {wrapper} ({msg})"
        launch_errors.append(f"direct wrapper exec: {msg}")
        logger.warning(
            "Direct wrapper launch failed for %s: %s",
            rec.get("desktop_id", rec.get("name", "unknown")),
            msg,
        )

    if desktop_path and shutil.which("gio"):
        ok, msg = await _launch_subprocess_checked(
            ["gio", "launch", desktop_path],
            cwd=cwd,
        )
        if ok:
            return True, f"gio launch accepted ({msg})"
        launch_errors.append(f"gio launch: {msg}")
        logger.debug("gio launch failed for %s: %s", desktop_path, msg)

    if desktop_id and shutil.which("gtk-launch"):
        ok, msg = await _launch_subprocess_checked(
            ["gtk-launch", desktop_id],
            cwd=cwd,
        )
        if ok:
            return True, f"gtk-launch accepted ({msg})"
        launch_errors.append(f"gtk-launch: {msg}")
        logger.debug("gtk-launch failed for %s: %s", desktop_id, msg)

    if not exec_cmd:
        launch_errors.append("no exec command")
        return False, "; ".join(launch_errors[-3:])

    if rec.get("terminal"):
        ok, msg = await _launch_in_terminal(exec_cmd, cwd=cwd)
        if ok:
            return True, msg
        launch_errors.append(f"terminal exec: {msg}")

    if not direct_wrapper_attempted:
        ok, msg = await _launch_exec(
            exec_cmd,
            cwd=cwd,
            check_early_exit=True,
        )
        if ok:
            return True, f"direct exec ({msg})"
        launch_errors.append(f"direct exec: {msg}")

    return False, "; ".join(launch_errors[-3:])


def _looks_like_resource(target: str) -> bool:
    value = target.strip()
    lower = value.lower()
    if lower.startswith(("http://", "https://", "file://", "mailto:")):
        return True
    if value.startswith("/") or value.startswith("~/"):
        return True

    expanded = os.path.expanduser(os.path.expandvars(value))
    return Path(expanded).exists()


def _direct_command_candidates(app_name: str) -> list[str]:
    """Return installed executables only; never execute a natural-language name."""
    normalized_name = _normalize_text(app_name)
    candidates: list[str] = []

    for candidate in DIRECT_COMMAND_HINTS.get(normalized_name, []):
        if _is_executable_available(candidate):
            candidates.append(candidate)

    raw_args = _split_exec_args(app_name.strip())
    if len(raw_args) == 1 and _is_executable_available(raw_args[0]):
        candidates.append(raw_args[0])

    return _unique_keep_order(candidates)


def _has_named_direct_command_hint(app_name: str) -> bool:
    return _normalize_text(app_name) in DIRECT_COMMAND_HINTS


def _build_process_tokens(app_name: str, rec: Optional[dict]) -> list[str]:
    tokens: list[str] = []

    def add_token(raw: str) -> None:
        normalized = _normalize_text(raw)
        if normalized:
            tokens.append(normalized)

    if rec:
        exec_name = _exec_basename(rec.get("exec", ""))
        if exec_name:
            add_token(exec_name)
        try_exec_name = Path(rec.get("try_exec", "")).name.strip().lower()
        if try_exec_name:
            add_token(try_exec_name)
        startup_name = rec.get("startup_wm_class", "").strip().lower()
        if startup_name and " " not in startup_name:
            add_token(startup_name)

    for cmd in _direct_command_candidates(app_name):
        args = _split_exec_args(cmd)
        if args:
            add_token(Path(args[0]).name.strip().lower())
        else:
            add_token(Path(cmd).name.strip().lower())

    return _unique_keep_order(
        [
            token
            for token in tokens
            if len(token) >= 3
            and " " not in token
            and token not in {"python", "bash", "sh", "env", "gio", "gtk-launch", "xdg-open"}
        ]
    )


def _list_processes() -> list[tuple[str, str]]:
    try:
        proc = _run_quick(
            ["ps", "-eo", "comm=,args="],
            capture_output=True,
            text=True,
            timeout=3.0,
        )
    except Exception:
        return []

    if proc.returncode != 0:
        return []

    rows: list[tuple[str, str]] = []
    for line in proc.stdout.splitlines():
        parts = line.strip().split(None, 1)
        if not parts:
            continue
        comm = _normalize_text(parts[0])
        args = _normalize_text(parts[1] if len(parts) > 1 else "")
        rows.append((comm, args))
    return rows


def _is_existing_instance_running(tokens: list[str]) -> bool:
    if not tokens:
        return False

    processes = _list_processes()
    if not processes:
        return False

    for comm, _args in processes:
        if not comm:
            continue
        for token in tokens:
            # Strict process-name match to avoid false positives.
            if token == comm:
                return True
            if comm.startswith(token + "-"):
                return True
    return False


def _running_pids_for_tokens(tokens: list[str]) -> set[str]:
    if not tokens:
        return set()
    try:
        proc = _run_quick(
            ["ps", "-eo", "pid=,comm="],
            capture_output=True,
            text=True,
            timeout=3.0,
        )
    except Exception:
        return set()
    if proc.returncode != 0:
        return set()

    pids: set[str] = set()
    for line in proc.stdout.splitlines():
        parts = line.strip().split(None, 1)
        if len(parts) != 2:
            continue
        pid, comm = parts[0], _normalize_text(parts[1])
        if not comm:
            continue
        for token in tokens:
            if token == comm or comm.startswith(token + "-"):
                pids.add(pid)
                break
    return pids


def _find_window_by_pids(pids: set[str]) -> Optional[str]:
    if not pids:
        return None
    windows = _list_windows()
    if not windows:
        return None

    stacking = _window_stacking_order()
    best_id: Optional[str] = None
    best_stack = -1

    for window in windows:
        win_pid = str(window.get("pid", "")).strip()
        if not win_pid or win_pid not in pids:
            continue
        window_id = _normalize_window_id(window.get("id", ""))
        stack_rank = stacking.get(window_id, -1)
        if stack_rank > best_stack:
            best_stack = stack_rank
            best_id = window_id or window.get("id")

    return best_id


def _activate_existing_window_by_class_hint(tokens: list[str]) -> bool:
    hints = _unique_keep_order([_normalize_text(t) for t in tokens if t])
    if not hints:
        return False

    stacking = _window_stacking_order()
    windows = _list_windows()
    if windows:
        best_id: Optional[str] = None
        best_stack = -1
        for window in windows:
            wm_class = _normalize_text(window.get("wm_class", ""))
            if not wm_class:
                continue
            if not any(len(hint) >= 2 and hint in wm_class for hint in hints):
                continue
            window_id = _normalize_window_id(window.get("id", ""))
            stack_rank = stacking.get(window_id, -1)
            if stack_rank > best_stack:
                best_stack = stack_rank
                best_id = window_id or window.get("id")
        if best_id and _activate_window_by_id(best_id):
            return True

    if _can_use_xdotool():
        for hint in hints:
            if len(hint) < 2:
                continue
            try:
                search = _run_quick(
                    ["xdotool", "search", "--all", "--class", hint],
                    capture_output=True,
                    text=True,
                )
                if search.returncode != 0:
                    continue
                best_raw_id = ""
                best_stack = -1
                for raw_id in search.stdout.splitlines():
                    raw_id = raw_id.strip()
                    if not raw_id:
                        continue
                    normalized = _normalize_window_id(hex(int(raw_id)))
                    stack_rank = stacking.get(normalized, -1)
                    if stack_rank > best_stack:
                        best_stack = stack_rank
                        best_raw_id = raw_id
                if best_raw_id:
                    if _activate_window_by_id(hex(int(best_raw_id))):
                        return True
            except Exception:
                continue

    return False


async def _finalize_launch_result(
    launch_label: str,
    bring_to_front: bool,
    tokens: list[str],
    class_tokens: Optional[list[str]] = None,
) -> tuple[bool, str]:
    if not bring_to_front:
        return True, f"launched {launch_label}"
    if await _wait_for_window(
        tokens,
        move_to_primary=True,
        class_tokens=class_tokens,
    ):
        return True, f"status=launched_and_activated; app={launch_label}"
    return (
        False,
        f"status=verification_failed; app={launch_label}; "
        "process launch returned successfully but no matching front, viewable window was confirmed",
    )


async def open_application_by_name(
    app_name: str,
    bring_to_front: bool = True,
) -> tuple[bool, str]:
    query = app_name.strip()
    if not query:
        return False, "empty app name"

    resolution = resolve_application(query)
    direct_candidates = _direct_command_candidates(query)
    if resolution.status == "ambiguous":
        if direct_candidates and _has_named_direct_command_hint(query):
            logger.warning(
                "Desktop entry resolution ambiguous for '%s'; trying explicit direct command hints: %s",
                query,
                ", ".join(direct_candidates),
            )
        else:
            return (
                False,
                f"status=ambiguous; query={query!r}; candidates="
                f"{_format_resolution_candidates(resolution.candidates)}",
            )

    rec = resolution.selected.app if resolution.selected else None
    is_resource = _looks_like_resource(query)
    if rec is None and not direct_candidates and not is_resource:
        return (
            False,
            f"status=not_found; query={query!r}; candidates="
            f"{_format_resolution_candidates(resolution.candidates)}",
        )

    tokens = _build_match_tokens(query, rec)
    identity_tokens = _build_window_identity_tokens(query, rec)
    window_control_available = _has_window_control_backend()
    process_tokens = _build_process_tokens(query, rec)

    if bring_to_front:
        win_id = _find_matching_window(tokens, identity_tokens)
        if not win_id and _can_use_wmctrl():
            win_id = _find_window_by_pids(_running_pids_for_tokens(process_tokens))
        if win_id:
            target_name = rec["name"] if rec else query
            if _activate_window_by_id(win_id):
                return True, f"activated existing window for {target_name}"

            # A matching window exists, but the window manager may have ignored
            # the first activation request. Retry through the class path and then
            # ask the desktop entry to activate its running single instance.
            if _activate_existing_window_by_class_hint(identity_tokens):
                return (
                    True,
                    f"status=activation_recovered; app={target_name}; method=class_hint",
                )

            recovery_detail = "class/window recovery did not foreground the app"
            if rec:
                requested, request_detail = await _request_running_app_activation(rec, query)
                recovery_detail = request_detail
                if requested and await _wait_for_window(
                    tokens,
                    attempts=15,
                    delay=0.15,
                    class_tokens=identity_tokens,
                ):
                    return (
                        True,
                        f"status=activation_recovered; app={target_name}; "
                        "method=desktop_entry_reactivation",
                    )

            # Do not claim success merely because any activation command exited
            # zero. All recovery paths must satisfy the same foreground state.
            return (
                False,
                f"status=activation_failed; app={target_name}; "
                "matching window found but activation and foreground recovery failed; "
                f"recovery={recovery_detail}",
            )
        if _activate_existing_window_by_class_hint(identity_tokens):
            target_name = rec["name"] if rec else query
            return True, f"activated existing window by class hint for {target_name}"

    if bring_to_front and not window_control_available:
        if _is_existing_instance_running(process_tokens):
            # No window backend: best effort to ask app to focus existing instance.
            target_name = rec["name"] if rec else query
            ok, detail = await _request_running_app_activation(rec, query)
            if ok:
                return (
                    False,
                    f"status=verification_unavailable; app={target_name}; "
                    f"activation requested but no window backend can confirm it: {detail}",
                )
            return (
                False,
                f"status=activation_failed; app={target_name}; "
                "process is running but no window could be activated",
            )
        # Bias new app launch onto primary screen in multi-monitor setups.
        _prepare_primary_screen_launch_context()

    launch_errors: list[str] = []

    if rec:
        ok, msg = await _launch_desktop_entry(rec)
        if ok:
            verified, detail = await _finalize_launch_result(
                rec["name"],
                bring_to_front,
                tokens,
                identity_tokens,
            )
            if verified:
                return True, detail
            launch_errors.append(f"desktop entry verification: {detail}")
            logger.warning("Desktop entry launch for '%s' was accepted but not verified: %s", query, detail)
        else:
            logger.warning("Desktop entry launch failed for '%s': %s", query, msg)
            launch_errors.append(f"desktop entry: {msg}")

    for candidate in direct_candidates:
        ok, msg = await _launch_exec(candidate)
        if ok:
            return await _finalize_launch_result(
                candidate,
                bring_to_front,
                tokens,
                identity_tokens,
            )

        launch_errors.append(f"{candidate}: {msg}")

    if is_resource:
        ok, msg = _launch_subprocess(["xdg-open", query])
        if ok:
            return True, f"xdg-open {query}"
        launch_errors.append(f"xdg-open: {msg}")

    if launch_errors:
        return False, "; ".join(launch_errors[-3:])

    return False, f"status=launch_failed; query={query!r}"
