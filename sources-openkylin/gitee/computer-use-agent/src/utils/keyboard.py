"""Keyboard key-name normalization helpers shared across platforms.

The action model sometimes emits capitalized, aliased, or prefixed key names
(e.g. ``Control`` + ``V``, ``Ctrl``, ``Key.enter``, ``cmd``).  These helpers
collapse them into one canonical form so hotkey routing and dispatch are
case/alias-insensitive and behave the same on X11, Wayland, and Windows.
"""

from __future__ import annotations

import re
from typing import Optional

# Canonical names produced by :func:`normalize_key`.  Modifiers resolve to
# ``ctrl`` / ``shift`` / ``alt`` / ``super``; ``cmd``/``command`` are treated as
# the macOS primary modifier and mapped to ``ctrl`` for cross-platform app
# shortcuts, while ``win``/``meta``/``super`` keep their Super-key meaning.
_KEY_ALIASES: dict[str, str] = {
    # Ctrl family
    "control": "ctrl",
    "ctrl_l": "ctrl",
    "ctrl_r": "ctrl",
    "leftctrl": "ctrl",
    "rightctrl": "ctrl",
    "lctrl": "ctrl",
    "rctrl": "ctrl",
    "ctrlleft": "ctrl",
    "ctrlright": "ctrl",
    # Shift family
    "shift_l": "shift",
    "shift_r": "shift",
    "leftshift": "shift",
    "rightshift": "shift",
    "shiftleft": "shift",
    "shiftright": "shift",
    # Alt family
    "alt_l": "alt",
    "alt_r": "alt",
    "leftalt": "alt",
    "rightalt": "alt",
    "altleft": "alt",
    "altright": "alt",
    "option": "alt",
    # Command (macOS primary modifier) -> Ctrl on Linux/Windows
    "cmd": "ctrl",
    "command": "ctrl",
    # Super / Windows / Meta key
    "win": "super",
    "winleft": "super",
    "winright": "super",
    "meta": "super",
    # Enter / Escape / Delete
    "return": "enter",
    "esc": "escape",
    "del": "delete",
    # Page navigation
    "pgup": "pageup",
    "pgdn": "pagedown",
    "spacebar": "space",
}

# Canonical -> xdotool keysym name.  xdotool uses X11 keysym spellings, which
# differ from the canonical names for several keys.
_XDOTOOL_KEY_NAMES: dict[str, str] = {
    "enter": "Return",
    "escape": "Escape",
    "backspace": "BackSpace",
    "tab": "Tab",
    "delete": "Delete",
    "insert": "Insert",
    "home": "Home",
    "end": "End",
    "pageup": "Prior",
    "pagedown": "Next",
    "up": "Up",
    "down": "Down",
    "left": "Left",
    "right": "Right",
    "capslock": "Caps_Lock",
    "numlock": "Num_Lock",
    "scrolllock": "Scroll_Lock",
    "pause": "Pause",
    "printscreen": "Print",
    "ctrl": "ctrl",
    "shift": "shift",
    "alt": "alt",
    "super": "super",
}

# Canonical -> PyAutoGUI key name.  PyAutoGUI calls the Super key "win".
_PYAUTOGUI_KEY_NAMES: dict[str, str] = {
    "super": "win",
}


def normalize_key(key: Optional[str]) -> str:
    """Return the canonical, lower-cased key name for ``key``.

    Strips ``Key.``/``Keys.`` prefixes and surrounding quotes, then applies
    alias resolution.  Unknown keys are returned lower-cased.
    """
    if key is None:
        return ""
    text = str(key).strip().strip("'\"")
    for prefix in ("Keys.", "Key.", "keys.", "key."):
        if text.startswith(prefix):
            text = text[len(prefix):]
            break
    text = text.strip().strip("'\"").strip()
    lowered = text.lower()
    return _KEY_ALIASES.get(lowered, lowered)


def split_hotkey(key: Optional[str]) -> list[str]:
    """Parse a combined hotkey string into normalized key parts.

    Accepts ``"ctrl+v"``, ``"ctrl v"``, ``"Control+Shift+T"``, or
    ``"Key.enter"`` and returns e.g. ``["ctrl", "v"]``, ``["ctrl", "shift",
    "t"]``, or ``["enter"]``.
    """
    if key is None:
        return []
    text = str(key).strip().strip("'\"")
    for prefix in ("Keys.", "Key.", "keys.", "key."):
        if text.startswith(prefix):
            text = text[len(prefix):]
            break
    parts: list[str] = []
    for chunk in re.split(r"[+\s]+", text.strip()):
        normalized = normalize_key(chunk)
        if normalized:
            parts.append(normalized)
    return parts


def xdotool_key_name(key: str) -> str:
    """Map a canonical key name to the xdotool keysym name."""
    if len(key) == 1:
        return key
    if key.startswith("f") and key[1:].isdigit():
        return key.upper()
    return _XDOTOOL_KEY_NAMES.get(key, key)


def pyautogui_key_name(key: str) -> str:
    """Map a canonical key name to a PyAutoGUI-compatible key name."""
    return _PYAUTOGUI_KEY_NAMES.get(key, key)
