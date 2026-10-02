import asyncio
import logging
import math
import os
import shutil
import subprocess
import threading
import time
from typing import Optional, Tuple

try:
    import pyautogui
except Exception as exc:
    pyautogui = None
    _PYAUTOGUI_IMPORT_ERROR = exc
else:
    _PYAUTOGUI_IMPORT_ERROR = None
import pyperclip

from src.linux.screen import capture_target_screenshot, get_input_screen_rect, get_target_screen_rect, normalized_to_screen, get_input_desktop_origin
from src.utils.keyboard import normalize_key, pyautogui_key_name, xdotool_key_name

logger = logging.getLogger(__name__)
ACTION_CMD_TIMEOUT = 3.0
TYPE_CMD_TIMEOUT = 60.0
WAYLAND_YDOTOOL_COORD_FACTOR = 2.4
WAYLAND_CLICK_RELATIVE_OFFSET = 2
WAYLAND_SCROLL_RELATIVE_OFFSET = 2
WAYLAND_SCROLL_STEP_DELAY = 0.04
WAYLAND_COORD_OFFSET_X_RATIO = -0.006
WAYLAND_COORD_OFFSET_Y_RATIO = -0.006

_YDOTOOL_KEY_CODES = {
    "esc": 1,
    "escape": 1,
    "1": 2,
    "2": 3,
    "3": 4,
    "4": 5,
    "5": 6,
    "6": 7,
    "7": 8,
    "8": 9,
    "9": 10,
    "0": 11,
    "-": 12,
    "=": 13,
    "backspace": 14,
    "tab": 15,
    "q": 16,
    "w": 17,
    "e": 18,
    "r": 19,
    "t": 20,
    "y": 21,
    "u": 22,
    "i": 23,
    "o": 24,
    "p": 25,
    "[": 26,
    "]": 27,
    "enter": 28,
    "return": 28,
    "ctrl": 29,
    "control": 29,
    "leftctrl": 29,
    "control_l": 29,
    "ctrl_l": 29,
    "a": 30,
    "s": 31,
    "d": 32,
    "f": 33,
    "g": 34,
    "h": 35,
    "j": 36,
    "k": 37,
    "l": 38,
    ";": 39,
    "'": 40,
    "`": 41,
    "shift": 42,
    "leftshift": 42,
    "\\": 43,
    "z": 44,
    "x": 45,
    "c": 46,
    "v": 47,
    "b": 48,
    "n": 49,
    "m": 50,
    ",": 51,
    ".": 52,
    "/": 53,
    "rightshift": 54,
    "alt": 56,
    "leftalt": 56,
    "alt_l": 56,
    "space": 57,
    "capslock": 58,
    "f1": 59,
    "f2": 60,
    "f3": 61,
    "f4": 62,
    "f5": 63,
    "f6": 64,
    "f7": 65,
    "f8": 66,
    "f9": 67,
    "f10": 68,
    "home": 102,
    "up": 103,
    "pageup": 104,
    "pgup": 104,
    "left": 105,
    "right": 106,
    "end": 107,
    "down": 108,
    "pagedown": 109,
    "pgdn": 109,
    "insert": 110,
    "delete": 111,
    "del": 111,
    "mute": 113,
    "volumedown": 114,
    "volumeup": 115,
    "super": 125,
    "win": 125,
    "meta": 125,
    "cmd": 125,
    "rightctrl": 97,
    "control_r": 97,
    "ctrl_r": 97,
    "rightalt": 100,
    "alt_r": 100,
    "f11": 87,
    "f12": 88,
}


class LinuxActions:
    """Linux UI actions with X11/PyAutoGUI and Wayland/ydotool backends."""

    def __init__(self) -> None:
        self.session_type = self._detect_session_type()
        self._ydotool_path = shutil.which("ydotool")
        self._use_ydotool = self.session_type == "wayland"
        self._last_click_window_token: Optional[str] = None
        self._last_click_time: Optional[float] = None
        self._last_click_position: Optional[Tuple[float, float]] = None
        self._xclip_process: Optional[subprocess.Popen] = None
        self._wl_copy_process: Optional[subprocess.Popen] = None
        self.last_input_error: Optional[str] = None
        if self._use_ydotool and not self._ydotool_path:
            logger.warning("Wayland session detected but ydotool is not installed.")
        elif self._use_ydotool:
            logger.info("Wayland session detected; LinuxActions will use ydotool.")
            _, _, screen_w, screen_h = get_input_screen_rect()
            factor = self._wayland_ydotool_factor()
            offset_x, offset_y = self._wayland_coord_offsets(screen_w, screen_h, factor)
            logger.info(
                "Wayland ydotool coordinate mapping: logical_size=(%s, %s), factor=%.3f, "
                "max=(%.1f, %.1f), fine_scale=(%.3f, %.3f), offset=(%.1f, %.1f)",
                screen_w,
                screen_h,
                factor,
                screen_w / factor,
                screen_h / factor,
                self._env_float("KYLIN_WAYLAND_COORD_SCALE_X", 1.0),
                self._env_float("KYLIN_WAYLAND_COORD_SCALE_Y", 1.0),
                offset_x,
                offset_y,
            )
        else:
            logger.info("LinuxActions will use PyAutoGUI for session type '%s'.", self.session_type)

    @staticmethod
    def _detect_session_type() -> str:
        session_type = (os.getenv("XDG_SESSION_TYPE") or "").strip().lower()
        if session_type in {"wayland", "x11"}:
            return session_type
        if os.getenv("WAYLAND_DISPLAY"):
            return "wayland"
        if os.getenv("DISPLAY"):
            return "x11"
        return "unknown"

    @property
    def _is_wayland(self) -> bool:
        return self._use_ydotool

    def _pyautogui(self):
        if pyautogui is None:
            raise RuntimeError(f"PyAutoGUI is unavailable: {_PYAUTOGUI_IMPORT_ERROR}")
        return pyautogui

    def _stop_xclip_owner(self) -> None:
        proc = self._xclip_process
        self._xclip_process = None
        if proc is None or proc.poll() is not None:
            return
        try:
            proc.terminate()
            proc.wait(timeout=0.5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass

    def _stop_wl_copy_owner(self) -> None:
        proc = self._wl_copy_process
        self._wl_copy_process = None
        if proc is None or proc.poll() is not None:
            return
        try:
            proc.terminate()
            proc.wait(timeout=0.5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass

    async def _write_x11_clipboard_text(self, text: str) -> bool:
        if not shutil.which("xclip"):
            return False

        def start_owner() -> bool:
            self._stop_xclip_owner()
            proc = subprocess.Popen(
                ["xclip", "-selection", "clipboard"],
                stdin=subprocess.PIPE,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
                text=True,
                close_fds=True,
            )
            write_error: list[Exception] = []

            def feed_stdin() -> None:
                try:
                    assert proc.stdin is not None
                    proc.stdin.write(text)
                    proc.stdin.close()
                except Exception as e:
                    write_error.append(e)

            writer = threading.Thread(target=feed_stdin, daemon=True)
            writer.start()
            writer.join(timeout=0.5)
            if writer.is_alive():
                proc.kill()
                logger.warning("xclip write timed out while feeding clipboard text.")
                return False
            if write_error:
                proc.kill()
                logger.warning("xclip write error: %s", write_error[0])
                return False
            time.sleep(0.05)
            if proc.poll() is not None and proc.returncode != 0:
                stderr = proc.stderr.read().strip() if proc.stderr else ""
                logger.warning("xclip write failed (code=%s): %s", proc.returncode, stderr)
                return False
            if proc.poll() is None:
                self._xclip_process = proc
            return True

        if not start_owner():
            return False
        pasted = await self._read_clipboard_text()
        if pasted != text:
            self._stop_xclip_owner()
            logger.warning("X11 clipboard verification failed.")
            return False
        return True

    async def _active_window_token(self) -> Optional[str]:
        """Return a stable active-window token when the platform exposes one."""
        if self._is_wayland or shutil.which("xdotool") is None:
            return None
        try:
            proc = await asyncio.to_thread(
                subprocess.run,
                ["xdotool", "getactivewindow"],
                capture_output=True,
                text=True,
                check=False,
                timeout=ACTION_CMD_TIMEOUT,
            )
            if proc.returncode == 0 and proc.stdout.strip():
                return proc.stdout.strip()
        except Exception as e:
            logger.debug("Failed to query active window: %s", e)
        return None

    async def _record_click_focus(self) -> None:
        self._last_click_time = time.monotonic()
        self._last_click_window_token = await self._active_window_token()

    async def _validate_text_focus(self) -> bool:
        max_age = max(
            1.0,
            self._env_float("KYLIN_TEXT_FOCUS_MAX_AGE", 180.0),
        )
        if self._last_click_time is None:
            self.last_input_error = (
                "Text input blocked: no recent framework-recorded focus click."
            )
            return False
        if time.monotonic() - self._last_click_time > max_age:
            self.last_input_error = (
                "Text input blocked: the focus click is stale; click the field again."
            )
            return False

        active_token = await self._active_window_token()
        if (
            self._last_click_window_token is not None
            and active_token is not None
            and active_token != self._last_click_window_token
        ):
            self.last_input_error = (
                "Text input blocked: focus moved to a different window after "
                "the target field was clicked."
            )
            return False
        return True

    async def _reactivate_recorded_click_window(self) -> bool:
        if (
            self._is_wayland
            or not self._last_click_window_token
            or shutil.which("xdotool") is None
        ):
            return False
        try:
            proc = await asyncio.to_thread(
                subprocess.run,
                ["xdotool", "windowactivate", "--sync", self._last_click_window_token],
                capture_output=True,
                text=True,
                check=False,
                timeout=ACTION_CMD_TIMEOUT,
            )
            if proc.returncode == 0:
                return True
            logger.debug(
                "Failed to reactivate text target window %s: %s",
                self._last_click_window_token,
                proc.stderr.strip(),
            )
        except Exception as e:
            logger.debug("Failed to reactivate text target window: %s", e)
        return False

    async def _restore_text_focus(self, reclick_if_unverifiable: bool = True) -> bool:
        """Return focus to the last framework-clicked field before pasting."""
        if await self._validate_text_focus():
            return True

        previous_error = self.last_input_error
        if not self._last_click_position:
            return False

        if self._last_click_window_token:
            await self._reactivate_recorded_click_window()

        x, y = self._last_click_position
        if not await self.click(x, y, "left"):
            self.last_input_error = previous_error
            return False
        await asyncio.sleep(0.05)
        return await self._validate_text_focus()

    async def _run_ydotool(self, args: list[str]) -> bool:
        if not self._ydotool_path:
            logger.error("ydotool is required for Wayland actions but was not found in PATH.")
            return False
        try:
            proc = await asyncio.to_thread(
                subprocess.run,
                [self._ydotool_path, *args],
                capture_output=True,
                text=True,
                check=False,
                timeout=ACTION_CMD_TIMEOUT,
            )
            if proc.returncode == 0:
                return True
            logger.error("ydotool %s failed (code=%s): %s", " ".join(args), proc.returncode, proc.stderr.strip())
            return False
        except Exception as e:
            logger.error("ydotool %s error: %s", " ".join(args), e)
            return False

    async def _run_ydotool_with_input(self, args: list[str], input_text: str, timeout: float = TYPE_CMD_TIMEOUT) -> bool:
        if not self._ydotool_path:
            logger.error("ydotool is required for Wayland actions but was not found in PATH.")
            return False
        try:
            proc = await asyncio.to_thread(
                subprocess.run,
                [self._ydotool_path, *args],
                input=input_text,
                capture_output=True,
                text=True,
                check=False,
                timeout=timeout,
            )
            if proc.returncode == 0:
                return True
            logger.error("ydotool %s failed (code=%s): %s", " ".join(args), proc.returncode, proc.stderr.strip())
            return False
        except Exception as e:
            logger.error("ydotool %s error: %s", " ".join(args), e)
            return False

    @staticmethod
    def _text_operation_timeout(text: str, *, seconds_per_char: float = 0.02) -> float:
        return max(ACTION_CMD_TIMEOUT, min(TYPE_CMD_TIMEOUT, 2.0 + (len(text) * seconds_per_char)))

    @staticmethod
    def _ydotool_button(button: str) -> Optional[str]:
        button_map = {
            "left": "0xC0",
            "right": "0xC1",
            "middle": "0xC2",
        }
        return button_map.get(button)

    @staticmethod
    def _ydotool_key_code(key: str) -> Optional[int]:
        normalized = str(key).replace("Key.", "").strip("'\"").lower()
        normalized = normalized.replace("control", "ctrl")
        return _YDOTOOL_KEY_CODES.get(normalized)

    async def _ydotool_key(self, *keys: str) -> bool:
        codes: list[int] = []
        for key in keys:
            code = self._ydotool_key_code(key)
            if code is None:
                logger.error("Unsupported ydotool key name: %s", key)
                return False
            codes.append(code)

        if len(codes) == 1:
            return await self._run_ydotool(["key", f"{codes[0]}:1", f"{codes[0]}:0"])

        events = [f"{code}:1" for code in codes]
        events.extend(f"{code}:0" for code in reversed(codes))
        logger.debug("ydotool hotkey batch: keys=%s events=%s", keys, events)
        return await self._run_ydotool(["key", *events])

    async def _ydotool_type_text(self, text: str) -> bool:
        if not text:
            return True
        timeout = self._text_operation_timeout(text, seconds_per_char=0.08)
        return await self._run_ydotool_with_input(["type", "-d", "20", "-f", "-"], text, timeout=timeout)

    async def _ydotool_move(self, x: float, y: float) -> bool:
        calibrated_x, calibrated_y = self._calibrate_wayland_point(x, y)
        logger.debug(
            "ydotool absolute move: raw=(%.2f, %.2f), calibrated=(%.2f, %.2f)",
            x,
            y,
            calibrated_x,
            calibrated_y,
        )
        return await self._run_ydotool(
            ["mousemove", "--absolute", str(int(round(calibrated_x))), str(int(round(calibrated_y)))]
        )

    async def _ydotool_move_for_click(self, x: float, y: float) -> bool:
        return await self._ydotool_enter_target(
            x,
            y,
            "KYLIN_WAYLAND_CLICK_RELATIVE_OFFSET",
            WAYLAND_CLICK_RELATIVE_OFFSET,
            "click",
        )

    async def _ydotool_move_for_scroll(self, x: float, y: float) -> bool:
        return await self._ydotool_enter_target(
            x,
            y,
            "KYLIN_WAYLAND_SCROLL_RELATIVE_OFFSET",
            WAYLAND_SCROLL_RELATIVE_OFFSET,
            "scroll",
        )

    async def _ydotool_enter_target(
        self,
        x: float,
        y: float,
        offset_env: str,
        default_offset: float,
        action_label: str,
    ) -> bool:
        calibrated_x, calibrated_y = self._calibrate_wayland_point(x, y)
        offset = max(
            0,
            int(round(self._env_float(offset_env, default_offset))),
        )
        target_x = int(round(calibrated_x))
        target_y = int(round(calibrated_y))

        if offset <= 0:
            return await self._run_ydotool(["mousemove", "--absolute", str(target_x), str(target_y)])

        start_x = max(0, target_x - offset)
        relative_x = target_x - start_x
        logger.debug(
            "ydotool %s pre-move: raw=(%.2f, %.2f), calibrated=(%s, %s), "
            "start=(%s, %s), relative=(%s, 0)",
            action_label,
            x,
            y,
            target_x,
            target_y,
            start_x,
            target_y,
            relative_x,
        )
        if not await self._run_ydotool(["mousemove", "--absolute", str(start_x), str(target_y)]):
            return False
        await asyncio.sleep(0.03)
        if relative_x and not await self._run_ydotool(["mousemove", str(relative_x), "0"]):
            return False
        await asyncio.sleep(0.08)
        return True

    def getscreen_size(self) -> Tuple[int, int]:
        try:
            _, _, screen_width, screen_height = (
                get_input_screen_rect()
            )
            return int(screen_width), int(screen_height)
        except Exception as e:
            logger.error(f"Error getting screen size: {e}")
            return 1920, 1080

    def _to_pixel(self, x: float, y: float) -> tuple[float, float]:
        offset_x, offset_y, screen_w, screen_h = get_input_screen_rect()
        pos = normalized_to_screen(x, y)
        logger.debug(
            "coordinate conversion: input=(%s, %s), rect=(%s, %s, %s, %s), pixel=(%.2f, %.2f)",
            x,
            y,
            offset_x,
            offset_y,
            screen_w,
            screen_h,
            pos[0],
            pos[1],
        )
        return pos

    @staticmethod
    def _env_float(name: str, default: float) -> float:
        raw = os.getenv(name)
        if raw is None or raw == "":
            return default
        try:
            value = float(raw)
            if not math.isfinite(value):
                raise ValueError("non-finite calibration")
            return value
        except ValueError:
            logger.warning("Ignoring invalid %s=%r; expected a number.", name, raw)
            return default

    def _wayland_ydotool_factor(self) -> float:
        factor = self._env_float("KYLIN_WAYLAND_YDOTOOL_FACTOR", WAYLAND_YDOTOOL_COORD_FACTOR)
        if factor <= 0:
            logger.warning(
                "Ignoring invalid KYLIN_WAYLAND_YDOTOOL_FACTOR=%s; expected a positive number.",
                factor,
            )
            return WAYLAND_YDOTOOL_COORD_FACTOR
        return factor

    def _wayland_coord_offsets(
        self,
        screen_w: Optional[float] = None,
        screen_h: Optional[float] = None,
        factor: Optional[float] = None,
    ) -> tuple[float, float]:
        if factor is None:
            factor = self._wayland_ydotool_factor()
        if screen_w is None or screen_h is None:
            _, _, screen_w, screen_h = get_input_screen_rect()
        default_x = (float(screen_w) / factor) * WAYLAND_COORD_OFFSET_X_RATIO
        default_y = (float(screen_h) / factor) * WAYLAND_COORD_OFFSET_Y_RATIO
        offset_x = self._env_float("KYLIN_WAYLAND_COORD_OFFSET_X", default_x)
        offset_y = self._env_float("KYLIN_WAYLAND_COORD_OFFSET_Y", default_y)
        return offset_x, offset_y

    def _calibrate_wayland_point(self, x: float, y: float) -> tuple[float, float]:
        if not self._is_wayland:
            return x, y
        factor = self._wayland_ydotool_factor()
        scale_x = self._env_float("KYLIN_WAYLAND_COORD_SCALE_X", 1.0)
        scale_y = self._env_float("KYLIN_WAYLAND_COORD_SCALE_Y", 1.0)
        offset_x, offset_y = self._wayland_coord_offsets(factor=factor)
        origin_x, origin_y = get_input_desktop_origin()
        return (((x - origin_x) / factor) * scale_x) + offset_x, (((y - origin_y) / factor) * scale_y) + offset_y

    @staticmethod
    def _is_ascii_text(text: str) -> bool:
        try:
            text.encode("ascii")
            return True
        except UnicodeEncodeError:
            return False

    async def _type_with_xdotool(self, text: str) -> bool:
        """使用剪贴板 + 粘贴方式输入文本，避免输入法干扰。"""
        if not text:
            return True
        return await self._send_paste_hotkey()

    async def _xdotool_key(self, *keys: str) -> bool:
        """Send one or more normalized keys as a single xdotool key sequence.

        ``--clearmodifiers`` clears any stuck modifiers (Caps Lock, Num Lock,
        a lingering Ctrl) before dispatch, which makes combined keys such as
        Ctrl+C/Ctrl+V reliable on X11.
        """
        if not keys:
            return True
        combo = "+".join(xdotool_key_name(k) for k in keys)
        try:
            proc = await asyncio.to_thread(
                subprocess.run,
                ["xdotool", "key", "--clearmodifiers", combo],
                capture_output=True,
                text=True,
                check=False,
                timeout=ACTION_CMD_TIMEOUT,
            )
            if proc.returncode == 0:
                return True
            logger.warning(
                "xdotool key '%s' failed (code=%s): %s",
                combo,
                proc.returncode,
                proc.stderr.strip(),
            )
            return False
        except Exception as e:
            logger.warning("xdotool key '%s' error: %s", combo, e)
            return False

    async def _send_paste_hotkey(self) -> bool:
        return await self.press_hotkey("ctrl", "v")

    async def _read_clipboard_text(self) -> Optional[str]:
        if self._is_wayland and shutil.which("wl-paste"):
            try:
                proc = await asyncio.to_thread(
                    subprocess.run,
                    ["wl-paste", "--no-newline"],
                    capture_output=True,
                    text=True,
                    check=False,
                    timeout=TYPE_CMD_TIMEOUT,
                )
                if proc.returncode == 0:
                    return proc.stdout
                logger.debug("wl-paste failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            except Exception as e:
                logger.debug("wl-paste error: %s", e)

        if not self._is_wayland and shutil.which("xclip"):
            try:
                proc = await asyncio.to_thread(
                    subprocess.run,
                    ["xclip", "-selection", "clipboard", "-out"],
                    capture_output=True,
                    text=True,
                    check=False,
                    timeout=ACTION_CMD_TIMEOUT,
                )
                if proc.returncode == 0:
                    return proc.stdout
                logger.debug("xclip read failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            except Exception as e:
                logger.debug("xclip read error: %s", e)
            return None

        try:
            return await asyncio.to_thread(pyperclip.paste)
        except Exception as e:
            logger.warning("Failed to read clipboard: %s", e)
            return None

    @staticmethod
    def _clipboard_text_matches(actual: Optional[str], expected: str) -> bool:
        if actual == expected:
            return True
        if actual is None:
            return False
        return actual.rstrip("\n") == expected.rstrip("\n")

    async def _write_clipboard_text(self, text: str) -> bool:
        if self._is_wayland and shutil.which("wl-copy"):
            try:
                self._stop_wl_copy_owner()
                proc = subprocess.Popen(
                    ["wl-copy", "--type", "text/plain;charset=utf-8"],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                    text=True,
                    close_fds=True,
                )

                write_error: list[Exception] = []

                def feed_stdin() -> None:
                    try:
                        assert proc.stdin is not None
                        proc.stdin.write(text)
                        proc.stdin.close()
                    except Exception as e:
                        write_error.append(e)

                writer = threading.Thread(target=feed_stdin, daemon=True)
                writer.start()
                writer.join(timeout=0.5)
                if writer.is_alive():
                    proc.kill()
                    logger.warning("wl-copy write timed out while feeding clipboard text.")
                    return False
                if write_error:
                    proc.kill()
                    logger.warning("wl-copy write error: %s", write_error[0])
                    return False

                await asyncio.sleep(0.05)
                if proc.poll() is not None and proc.returncode != 0:
                    stderr = proc.stderr.read().strip() if proc.stderr else ""
                    logger.warning("wl-copy failed (code=%s): %s", proc.returncode, stderr)
                    return False
                self._wl_copy_process = proc
                if shutil.which("wl-paste"):
                    pasted = await self._read_clipboard_text()
                    if not self._clipboard_text_matches(pasted, text):
                        logger.warning("Wayland clipboard verification failed.")
                        return False
                return True
            except Exception as e:
                logger.warning("wl-copy error: %s", e)
                return False

        if not self._is_wayland and shutil.which("xclip"):
            return await self._write_x11_clipboard_text(text)

        try:
            await asyncio.to_thread(pyperclip.copy, text)
            await asyncio.sleep(0.05)
            if await asyncio.to_thread(pyperclip.paste) != text:
                raise ValueError("Clipboard verification failed")
            return True
        except Exception as e:
            logger.warning("Failed to set clipboard: %s", e)
            return False

    async def _restore_clipboard_text(self, text: Optional[str]) -> None:
        if text is None:
            return
        for attempt in range(5):
            if await self._write_clipboard_text(text):
                return
            await asyncio.sleep(0.1 * (attempt + 1))

    async def click(self, x: float, y: float, button: str = "left") -> bool:
        pos_x, pos_y = self._to_pixel(x, y)
        try:
            if self._is_wayland:
                if button == "double":
                    success = (
                        await self._ydotool_move_for_click(pos_x, pos_y)
                        and await self._run_ydotool(["click", "-r", "2", "0xC0"])
                    )
                    if success:
                        self._last_click_position = (x, y)
                        await self._record_click_focus()
                    return success
                ydotool_button = self._ydotool_button(button)
                if ydotool_button is None:
                    logger.error("Unknown click button '%s'", button)
                    return False
                success = await self._ydotool_move_for_click(pos_x, pos_y) and await self._run_ydotool(["click", ydotool_button])
                if success:
                    if button in {"left", "double"}:
                        self._last_click_position = (x, y)
                    await self._record_click_focus()
                return success

            gui = self._pyautogui()
            if button == "left":
                gui.click(pos_x, pos_y, duration=0.05)
            elif button == "right":
                gui.rightClick(pos_x, pos_y, duration=0.05)
            elif button == "double":
                gui.doubleClick(pos_x, pos_y, duration=0.05)
            else:
                logger.error("Unknown click button '%s'", button)
                return False

            await asyncio.sleep(0.03)
            if button in {"left", "double"}:
                self._last_click_position = (x, y)
            await self._record_click_focus()
            return True
        except Exception as e:
            logger.error(f"Error clicking at ({x}, {y}): {e}")
            return False

    async def drag(
        self,
        start_x: float,
        start_y: float,
        end_x: float,
        end_y: float,
        duration: float = 0.5,
    ) -> bool:
        pos_start_x, pos_start_y = self._to_pixel(start_x, start_y)
        pos_end_x, pos_end_y = self._to_pixel(end_x, end_y)
        try:
            if self._is_wayland:
                steps = max(4, int(duration * 20))
                if not await self._ydotool_move_for_click(pos_start_x, pos_start_y):
                    return False
                if not await self._run_ydotool(["click", "0x40"]):
                    return False
                await asyncio.sleep(0.05)
                for step in range(1, steps + 1):
                    ratio = step / steps
                    cur_x = pos_start_x + ((pos_end_x - pos_start_x) * ratio)
                    cur_y = pos_start_y + ((pos_end_y - pos_start_y) * ratio)
                    if not await self._ydotool_move(cur_x, cur_y):
                        await self._run_ydotool(["click", "0x80"])
                        return False
                    await asyncio.sleep(duration / steps)
                released = await self._run_ydotool(["click", "0x80"])
                if released:
                    await asyncio.sleep(0.05)
                return released

            gui = self._pyautogui()
            gui.moveTo(pos_start_x, pos_start_y, duration=0.2)
            gui.dragTo(pos_end_x, pos_end_y, duration=duration, button="left")
            return True
        except Exception as e:
            logger.error(
                "Error dragging from (%s, %s) to (%s, %s): %s",
                start_x,
                start_y,
                end_x,
                end_y,
                e,
            )
            return False

    async def scroll(self, x: float, y: float, clicks: int) -> bool:
        pos_x, pos_y = self._to_pixel(x, y)
        try:
            if self._is_wayland:
                amount = int(clicks)
                if not await self._ydotool_move_for_scroll(pos_x, pos_y):
                    return False
                if amount == 0:
                    return True
                step = 1 if amount > 0 else -1
                delay = max(
                    0.0,
                    self._env_float("KYLIN_WAYLAND_SCROLL_STEP_DELAY", WAYLAND_SCROLL_STEP_DELAY),
                )
                for _ in range(abs(amount)):
                    if not await self._run_ydotool(["mousemove", "--wheel", "0", str(step)]):
                        return False
                    if delay:
                        await asyncio.sleep(delay)
                return True
            self._pyautogui().scroll(clicks, x=pos_x, y=pos_y)
            return True
        except Exception as e:
            logger.error(f"Error scrolling at ({x}, {y}): {e}")
            return False

    async def type_text(self, text: str, press_enter: bool = False, select_all: bool = True) -> bool:
        """
        Type text using clipboard method to handle both English and Chinese characters.
        When select_all is true, it selects all existing text (Ctrl+A) before pasting to replace the content; otherwise it pastes at the current cursor position.
        """
        original_clipboard = None
        self.last_input_error = None
        try:
            if not text:
                return True

            if not await self._restore_text_focus():
                logger.warning(self.last_input_error or "Text input focus validation failed.")
                return False

            original_clipboard = await self._read_clipboard_text()

            if self._is_wayland and not shutil.which("wl-copy"):
                logger.warning(
                    "wl-copy is not available; falling back to ydotool type for Wayland text input."
                )
                if not await self._restore_text_focus():
                    logger.warning(self.last_input_error or "Text input focus validation failed.")
                    return False
                if select_all:
                    if not await self.press_hotkey("ctrl", "a"):
                        return False
                    await asyncio.sleep(0.05)
                typed = await self._ydotool_type_text(text)
                if typed and press_enter:
                    return await self.press_key("enter")
                return typed

            # 将文本复制到剪贴板
            for attempt in range(3):
                if await self._write_clipboard_text(text):
                    break
                logger.warning("Attempt %s: Failed to set clipboard.", attempt + 1)
                await asyncio.sleep(0.2 * (attempt + 1))
            else:
                if self._is_wayland:
                    typed = await self._ydotool_type_text(text)
                    if typed and press_enter:
                        if not await self.press_key("enter"):
                            await self._restore_clipboard_text(original_clipboard)
                            return False
                    await self._restore_clipboard_text(original_clipboard)
                    return typed
                await self._restore_clipboard_text(original_clipboard)
                return False

            if not await self._restore_text_focus():
                logger.warning(self.last_input_error or "Text input focus validation failed.")
                await self._restore_clipboard_text(original_clipboard)
                return False
            # Keep the risky select-all window as short as possible: prepare
            # and verify the clipboard first, then select and paste immediately.
            if select_all:
                if self._is_wayland:
                    if not await self.press_hotkey("ctrl", "a"):
                        await self._restore_clipboard_text(original_clipboard)
                        return False
                else:
                    self._pyautogui().hotkey("ctrl", "a")
                await asyncio.sleep(0.05)
            if not await self._restore_text_focus(reclick_if_unverifiable=False):
                logger.warning(self.last_input_error or "Text input focus validation failed.")
                await self._restore_clipboard_text(original_clipboard)
                return False
            # 使用 xdotool 发送粘贴快捷键（已改为剪贴板方式，避免输入法干扰）
            if await self._type_with_xdotool(text):
                await asyncio.sleep(
                    max(0.2, self._env_float("KYLIN_PASTE_SETTLE_DELAY", 0.6))
                )
                if press_enter and not await self.press_key("enter"):
                    await self._restore_clipboard_text(original_clipboard)
                    return False
                await self._restore_clipboard_text(original_clipboard)
                return True

            if self._is_wayland:
                typed = await self._ydotool_type_text(text)
                if typed and press_enter:
                    if not await self.press_key("enter"):
                        await self._restore_clipboard_text(original_clipboard)
                        return False
                await self._restore_clipboard_text(original_clipboard)
                return typed

            # xdotool 失败时回退到 pyautogui 发送粘贴快捷键
            if not await self._send_paste_hotkey():
                await self._restore_clipboard_text(original_clipboard)
                return False
            await asyncio.sleep(
                max(0.2, self._env_float("KYLIN_PASTE_SETTLE_DELAY", 0.6))
            )
            if press_enter and not await self.press_key("enter"):
                await self._restore_clipboard_text(original_clipboard)
                return False

            await self._restore_clipboard_text(original_clipboard)
            return True
        except Exception as e:
            logger.error(f"Error typing text '{text}': {e}")
            await self._restore_clipboard_text(original_clipboard)
            return False

    async def press_key(self, key: str) -> bool:
        normalized = normalize_key(key)
        if not normalized:
            logger.error("Cannot press an empty key name.")
            return False
        try:
            if self._is_wayland:
                return await self._ydotool_key(normalized)
            if shutil.which("xdotool") is not None:
                if await self._xdotool_key(normalized):
                    return True
                logger.warning(
                    "xdotool press_key '%s' failed; falling back to PyAutoGUI.",
                    normalized,
                )
            self._pyautogui().press(pyautogui_key_name(normalized))
            return True
        except Exception as e:
            logger.error(f"Error pressing key '{key}': {e}")
            return False

    async def press_hotkey(self, key1: str, key2: str, key3: Optional[str] = None) -> bool:
        keys = [normalize_key(k) for k in (key1, key2, key3)]
        keys = [k for k in keys if k]
        if not keys:
            logger.error("Cannot press an empty hotkey.")
            return False
        try:
            if self._is_wayland:
                return await self._ydotool_key(*keys)

            if shutil.which("xdotool") is not None:
                if await self._xdotool_key(*keys):
                    return True
                logger.warning(
                    "xdotool hotkey '%s' failed; falling back to PyAutoGUI.",
                    "+".join(keys),
                )

            gui = self._pyautogui()
            await asyncio.to_thread(gui.hotkey, *(pyautogui_key_name(k) for k in keys))
            return True
        except Exception as e:
            logger.error(f"Error pressing hotkey {key1, key2, key3}: {e}")
            return False

    async def take_screenshot(self, save_path: Optional[str] = None) -> bool:
        try:
            screenshot = capture_target_screenshot()
            if save_path:
                screenshot.save(save_path)
            return True
        except Exception as e:
            logger.error(f"Error taking screenshot: {e}")
            return False

    async def get_mouse_position(self) -> Tuple[int, int]:
        try:
            if self._is_wayland:
                logger.warning("Getting mouse position is not supported by ydotool.")
                return (0, 0)
            return self._pyautogui().position()
        except Exception as e:
            logger.error(f"Error getting mouse position: {e}")
            return (0, 0)

    async def move_mouse(self, x: float, y: float, duration: float = 0.0) -> bool:
        pos_x, pos_y = self._to_pixel(x, y)
        try:
            if self._is_wayland:
                return await self._ydotool_move(pos_x, pos_y)
            self._pyautogui().moveTo(pos_x, pos_y, duration=duration)
            return True
        except Exception as e:
            logger.error(f"Error moving mouse to ({x}, {y}): {e}")
            return False
