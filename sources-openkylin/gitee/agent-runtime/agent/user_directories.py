"""Resolve platform-managed user directories without assuming English names."""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path
from typing import Mapping


_XDG_NAMES = {
    "desktop": "XDG_DESKTOP_DIR",
    "documents": "XDG_DOCUMENTS_DIR",
    "downloads": "XDG_DOWNLOAD_DIR",
}


def _expand_xdg_value(value: str, home: Path) -> Path | None:
    """Expand the deliberately small variable syntax used by user-dirs.dirs."""
    value = value.strip()
    if len(value) < 2 or value[0] != '"' or value[-1] != '"':
        return None
    value = value[1:-1].replace(r"\"", '"').replace(r"\\", "\\")
    if value == "$HOME":
        return home
    if value.startswith("$HOME/"):
        return home / value[len("$HOME/") :]
    # The XDG format permits absolute paths, but no arbitrary shell expansion.
    path = Path(value)
    return path if path.is_absolute() else None


def _linux_user_directories(home: Path, env: Mapping[str, str]) -> dict[str, Path]:
    config_home = Path(env.get("XDG_CONFIG_HOME", str(home / ".config")))
    config_file = config_home / "user-dirs.dirs"
    try:
        content = config_file.read_text(encoding="utf-8")
    except (OSError, UnicodeError):
        return {}

    result: dict[str, Path] = {}
    for line in content.splitlines():
        match = re.match(r"^\s*(XDG_[A-Z]+_DIR)\s*=\s*(.+?)\s*$", line)
        if not match:
            continue
        key, raw_value = match.groups()
        path = _expand_xdg_value(raw_value, home)
        if path is None:
            continue
        for friendly_name, xdg_name in _XDG_NAMES.items():
            if key == xdg_name:
                result[friendly_name] = path
                break
    return result


def _windows_user_directories() -> dict[str, Path]:
    """Resolve redirected Known Folders (including OneDrive redirection)."""
    try:
        import ctypes
        from ctypes import wintypes

        folder_ids = {
            "desktop": "{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}",
            "documents": "{FDD39AD0-238F-46AF-ADB4-6C85480369C7}",
            "downloads": "{374DE290-123F-4565-9164-39C4925E467B}",
        }

        class GUID(ctypes.Structure):
            _fields_ = [
                ("Data1", wintypes.DWORD),
                ("Data2", wintypes.WORD),
                ("Data3", wintypes.WORD),
                ("Data4", ctypes.c_ubyte * 8),
            ]

        ole32 = ctypes.windll.ole32
        shell32 = ctypes.windll.shell32
        ole32.CLSIDFromString.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(GUID)]
        shell32.SHGetKnownFolderPath.argtypes = [
            ctypes.POINTER(GUID), wintypes.DWORD, wintypes.HANDLE,
            ctypes.POINTER(wintypes.LPWSTR),
        ]
        result: dict[str, Path] = {}
        for name, guid_text in folder_ids.items():
            guid = GUID()
            if ole32.CLSIDFromString(guid_text, ctypes.byref(guid)) != 0:
                continue
            raw_path = wintypes.LPWSTR()
            if shell32.SHGetKnownFolderPath(
                ctypes.byref(guid), 0, None, ctypes.byref(raw_path)
            ) == 0:
                try:
                    result[name] = Path(raw_path.value)
                finally:
                    ole32.CoTaskMemFree(raw_path)
        return result
    except (AttributeError, OSError, ValueError):
        return {}


def get_user_directories(
    *, home: Path | None = None, env: Mapping[str, str] | None = None,
    platform: str | None = None,
) -> dict[str, Path]:
    """Return system-configured common directories with conservative fallbacks."""
    env = os.environ if env is None else env
    home = Path(os.path.expanduser("~")) if home is None else Path(home)
    platform = sys.platform if platform is None else platform

    if platform == "win32":
        resolved = _windows_user_directories()
    elif platform.startswith("linux"):
        resolved = _linux_user_directories(home, env)
    else:
        resolved = {}

    defaults = {
        "desktop": home / "Desktop",
        "documents": home / "Documents",
        "downloads": home / "Downloads",
    }
    return {name: resolved.get(name, path) for name, path in defaults.items()}
