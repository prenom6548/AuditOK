import logging
import math
import json
import os
import re
import shutil
import subprocess
import tempfile
import time
import ast
import select
import urllib.parse
from typing import Optional

from PIL import Image

logger = logging.getLogger(__name__)

_WAYLAND_SCREEN_RECT_CACHE: tuple[float, tuple[int, int, int, int]] | None = None
_WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE: tuple[float, tuple[int, int, int, int]] | None = None
_WAYLAND_SCREEN_RECT_CACHE_TTL = 60.0
_LAST_SCREENSHOT_BACKEND = ""
_LAST_CAPTURE_RECT: tuple[int, int, int, int] | None = None
_LAST_CAPTURE_ORIGIN = (0, 0)
_BLACK_PADDING_THRESHOLD = 8
_BLACK_PADDING_MIN_RATIO = 0.98
_WAYLAND_GRIM_PNG_LEVEL = "1"
_WAYLAND_GRIM_FORMAT = "ppm"
_ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def _wayland_screen_rect_cache_ttl() -> float:
    raw = os.getenv("KYLIN_WAYLAND_SCREEN_RECT_CACHE_TTL", str(_WAYLAND_SCREEN_RECT_CACHE_TTL)).strip()
    try:
        value = float(raw)
    except ValueError:
        logger.warning("Ignoring invalid KYLIN_WAYLAND_SCREEN_RECT_CACHE_TTL=%r; expected seconds.", raw)
        return _WAYLAND_SCREEN_RECT_CACHE_TTL
    return value if value > 0 else _WAYLAND_SCREEN_RECT_CACHE_TTL


def _parse_target_screen(raw: Optional[str]) -> int:
    """Parse 1-based screen selector; defaults to 1."""
    if not raw:
        return 1
    value = raw.strip().lower()
    m = re.match(r"screen\s*(\d+)$", value)
    if m:
        return max(1, int(m.group(1)))
    if value.isdigit():
        return max(1, int(value))
    return 1


def list_screens() -> list[tuple[int, int, int, int]]:
    """
    Return monitor rectangles as (x, y, width, height), ordered by x then y.
    Linux implementation uses xrandr.
    """
    try:
        # Monitor geometry is in X root coordinates, including RandR scaling.
        monitors = subprocess.run(
            ["xrandr", "--listactivemonitors"], capture_output=True,
            text=True, check=False, timeout=5,
        )
        if monitors.returncode == 0:
            rects = []
            for line in monitors.stdout.splitlines():
                match = re.search(r"(\d+)/\d+x(\d+)/\d+([+-]\d+)([+-]\d+)", line)
                if match:
                    w, h, x, y = map(int, match.groups())
                    if w > 0 and h > 0:
                        rects.append((x, y, w, h))
            if rects:
                return sorted(rects, key=lambda rect: (rect[0], rect[1]))
        proc = subprocess.run(
            ["xrandr", "--query"],
            capture_output=True,
            text=True,
            check=False,
        )
        if proc.returncode != 0:
            return []

        screens: list[tuple[int, int, int, int]] = []
        pattern = re.compile(r" connected(?: primary)? (\d+)x(\d+)([+-]\d+)([+-]\d+)")
        for line in proc.stdout.splitlines():
            m = pattern.search(line)
            if not m:
                continue
            w, h, x, y = map(int, m.groups())
            if w > 0 and h > 0:
                screens.append((x, y, w, h))
        screens.sort(key=lambda s: (s[0], s[1]))
        return screens
    except Exception:
        return []


def _parse_kscreen_doctor_rects(stdout: str) -> list[tuple[int, int, int, int]]:
    rects: list[tuple[int, int, int, int]] = []
    current_enabled = True
    current_connected = True
    current_geometry: tuple[int, int, int, int] | None = None

    def flush_current() -> None:
        if current_geometry and current_enabled and current_connected:
            rects.append(current_geometry)

    for raw_line in stdout.splitlines():
        line = _ANSI_ESCAPE_RE.sub("", raw_line).strip()
        if not line:
            continue
        if line.startswith("Output:"):
            flush_current()
            current_enabled = True
            current_connected = True
            current_geometry = None
        if re.search(r"\bdisabled\b", line):
            current_enabled = False
        elif re.search(r"\benabled\b", line):
            current_enabled = True
        if re.search(r"\bdisconnected\b", line):
            current_connected = False
        elif re.search(r"\bconnected\b", line):
            current_connected = True

        m = re.search(r"Geometry:\s*(-?\d+),(-?\d+)\s+(\d+)x(\d+)", line)
        if not m:
            continue
        x, y, w, h = map(int, m.groups())
        if w > 0 and h > 0:
            current_geometry = (x, y, w, h)

    flush_current()
    rects.sort(key=lambda s: (s[0], s[1]))
    return rects


def list_wayland_screens() -> list[tuple[int, int, int, int]]:
    """
    Return Wayland logical monitor rectangles when the compositor exposes them.
    kscreen-doctor reports logical geometry. An explicit rectangle is available
    for compositors without a supported geometry query (x,y,width,height).
    """
    explicit = os.getenv("KYLIN_WAYLAND_SCREEN_RECTS")
    if explicit:
        rects = json.loads(explicit)
        if not isinstance(rects, list) or not rects or any(
            not isinstance(r, list) or len(r) != 4
            or any(type(v) is not int for v in r) or r[2] <= 0 or r[3] <= 0
            for r in rects
        ):
            raise ValueError("KYLIN_WAYLAND_SCREEN_RECTS must be [[x,y,width,height], ...] in logical coordinates")
        return sorted((tuple(r) for r in rects), key=lambda r: (r[0], r[1]))
    kscreen_doctor = shutil.which("kscreen-doctor")
    if not kscreen_doctor:
        return []
    try:
        proc = subprocess.run(
            [kscreen_doctor, "-o"],
            capture_output=True,
            text=True,
            check=False,
            timeout=5,
        )
        if proc.returncode != 0:
            logger.debug("kscreen-doctor failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            return []
        return _parse_kscreen_doctor_rects(proc.stdout)
    except Exception as e:
        logger.debug("kscreen-doctor error: %s", e)
        return []


def get_primary_screen_rect() -> tuple[int, int, int, int]:
    """
    Return primary monitor rect as (x, y, width, height).
    Falls back to first detected monitor, then full-screen fallback.
    """
    try:
        proc = subprocess.run(
            ["xrandr", "--query"],
            capture_output=True,
            text=True,
            check=False,
        )
        if proc.returncode == 0:
            pattern = re.compile(r" connected primary (\d+)x(\d+)([+-]\d+)([+-]\d+)")
            for line in proc.stdout.splitlines():
                m = pattern.search(line)
                if not m:
                    continue
                w, h, x, y = map(int, m.groups())
                if w > 0 and h > 0:
                    return (x, y, w, h)
    except Exception:
        pass

    screens = list_screens()
    if screens:
        return screens[0]

    try:
        import pyautogui

        w, h = pyautogui.size()
        return (0, 0, int(w), int(h))
    except Exception:
        return (0, 0, 1920, 1080)


def get_target_screen_rect() -> tuple[int, int, int, int]:
    """
    Return target screen region as (x, y, width, height).
    Uses env KYLIN_TARGET_SCREEN as 1-based index: 1, 2, ... or screen1/screen2.
    """
    if is_wayland_session():
        return get_wayland_screen_rect()

    screens = list_screens()
    if not screens:
        # Fallback to full screen region from X root geometry.
        try:
            import pyautogui

            w, h = pyautogui.size()
            return (0, 0, int(w), int(h))
        except Exception:
            return (0, 0, 1920, 1080)

    target_1_based = _parse_target_screen(os.getenv("KYLIN_TARGET_SCREEN"))
    idx = min(max(target_1_based - 1, 0), len(screens) - 1)
    return screens[idx]


def get_input_screen_rect() -> tuple[int, int, int, int]:
    """Use the geometry of the observation that produced the current action."""
    return _LAST_CAPTURE_RECT or get_target_screen_rect()


def normalized_to_screen(x: float, y: float) -> tuple[float, float]:
    """Map the documented 0..1000 protocol to screenshot desktop coordinates.

    Image pixels may be scaled for HiDPI or model input; fractions of the
    captured region stay the same. Never apply the display DPI a second time.
    """
    if not all(math.isfinite(v) and 0 <= v <= 1000 for v in (x, y)):
        raise ValueError("Coordinates must be finite numbers in the range 0..1000")
    left, top, width, height = get_input_screen_rect()
    return (left + min(width - 1, width * x / 1000),
            top + min(height - 1, height * y / 1000))


def get_input_desktop_origin() -> tuple[int, int]:
    """ydotool absolute movement starts at the desktop's top-left corner."""
    return _LAST_CAPTURE_ORIGIN


def is_wayland_session() -> bool:
    session_type = (os.getenv("XDG_SESSION_TYPE") or "").strip().lower()
    if session_type == "wayland":
        return True
    if session_type == "x11":
        return False
    return bool(os.getenv("WAYLAND_DISPLAY"))


def _load_image(path: str) -> Image.Image:
    image = Image.open(path)
    image.load()
    return image.convert("RGB")


def _crop_region(image: Image.Image, region: Optional[tuple[int, int, int, int]]) -> Image.Image:
    if region is None:
        return image
    x, y, w, h = region
    if w <= 0 or h <= 0:
        return image
    return image.crop((x, y, x + w, y + h))


def _crop_wayland_desktop(image: Image.Image, region: Optional[tuple[int, int, int, int]]) -> Image.Image:
    """Full-desktop backends don't accept logical crop coordinates themselves."""
    if region is None:
        return image
    screens = list_wayland_screens()
    if not screens:
        raise RuntimeError("Missing logical geometry for Wayland screenshot crop")
    left = min(r[0] for r in screens)
    top = min(r[1] for r in screens)
    width = max(r[0] + r[2] for r in screens) - left
    height = max(r[1] + r[3] for r in screens) - top
    sx, sy = image.width / width, image.height / height
    if abs(sx - sy) > max(1 / width, 1 / height):
        raise RuntimeError("Wayland screenshot has ambiguous mixed-scale geometry; use grim")
    if len(screens) > 1 and image.size != (width, height):
        raise RuntimeError("Scaled multi-monitor screenshot requires region-aware grim backend")
    x, y, w, h = region
    box = (round((x - left) * sx), round((y - top) * sy),
           round((x + w - left) * sx), round((y + h - top) * sy))
    if box[0] < 0 or box[1] < 0 or box[2] > image.width or box[3] > image.height:
        raise RuntimeError("Screenshot does not cover the selected monitor")
    return image.crop(box)


def _resize_to_region_size(
    image: Image.Image,
    region: Optional[tuple[int, int, int, int]],
) -> Image.Image:
    if region is None:
        return image
    _, _, w, h = region
    target_size = (int(w), int(h))
    if w <= 0 or h <= 0 or image.size == target_size:
        return image
    if image.width <= target_size[0] and image.height <= target_size[1]:
        return image
    if hasattr(Image, "Resampling"):
        resample = Image.Resampling.LANCZOS
    else:
        resample = Image.LANCZOS
    logger.info(
        "Resized Wayland screenshot from %sx%s to logical region %sx%s.",
        image.width,
        image.height,
        target_size[0],
        target_size[1],
    )
    return image.resize(target_size, resample=resample)


def _is_nearly_black(pixel: tuple[int, ...]) -> bool:
    return all(channel < _BLACK_PADDING_THRESHOLD for channel in pixel[:3])


def _trim_trailing_black_padding(image: Image.Image) -> tuple[Image.Image, tuple[int, int] | None]:
    """
    Some Wayland/KScreen combinations report a scaled physical monitor size to
    grim while the compositor returns only the logical framebuffer, padded with
    black on the right/bottom. Trim only that trailing padding.
    """
    if image.mode != "RGB":
        image = image.convert("RGB")
    width, height = image.size
    if width <= 1 or height <= 1:
        return image, None

    pixels = image.load()
    x_stride = max(1, width // 240)
    y_stride = max(1, height // 240)

    def row_is_padding(y: int) -> bool:
        samples = [_is_nearly_black(pixels[x, y]) for x in range(0, width, x_stride)]
        return bool(samples) and (sum(samples) / len(samples)) >= _BLACK_PADDING_MIN_RATIO

    def col_is_padding(x: int) -> bool:
        samples = [_is_nearly_black(pixels[x, y]) for y in range(0, height, y_stride)]
        return bool(samples) and (sum(samples) / len(samples)) >= _BLACK_PADDING_MIN_RATIO

    new_width = width
    while new_width > 1 and col_is_padding(new_width - 1):
        new_width -= 1

    new_height = height
    while new_height > 1 and row_is_padding(new_height - 1):
        new_height -= 1

    removed_width = width - new_width
    removed_height = height - new_height
    if removed_width <= 1 and removed_height <= 1:
        return image, None
    if new_width < width * 0.5 or new_height < height * 0.5:
        logger.debug(
            "Ignoring suspicious Wayland black-padding trim from %sx%s to %sx%s.",
            width,
            height,
            new_width,
            new_height,
        )
        return image, None

    trimmed = image.crop((0, 0, new_width, new_height))
    logger.info(
        "Trimmed Wayland screenshot black padding from %sx%s to %sx%s.",
        width,
        height,
        new_width,
        new_height,
    )
    return trimmed, (new_width, new_height)


def _remember_wayland_effective_rect(
    region: Optional[tuple[int, int, int, int]],
    trimmed_size: tuple[int, int] | None,
) -> None:
    if trimmed_size is None:
        return
    global _WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE
    if region is None:
        trimmed_w, trimmed_h = trimmed_size
        if trimmed_w <= 0 or trimmed_h <= 0:
            return
        _WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE = (
            time.monotonic(),
            (0, 0, int(trimmed_w), int(trimmed_h)),
        )
        return
    x, y, w, h = region
    trimmed_w, trimmed_h = trimmed_size
    if trimmed_w <= 0 or trimmed_h <= 0:
        return
    if trimmed_w >= w and trimmed_h >= h:
        return
    _WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE = (time.monotonic(), (x, y, int(trimmed_w), int(trimmed_h)))


def _wayland_subprocess_env() -> dict[str, str]:
    env = os.environ.copy()
    uid = os.getuid()
    env.setdefault("XDG_RUNTIME_DIR", f"/run/user/{uid}")
    if not env.get("DBUS_SESSION_BUS_ADDRESS") and os.path.exists(
        os.path.join(env["XDG_RUNTIME_DIR"], "bus")
    ):
        env["DBUS_SESSION_BUS_ADDRESS"] = f"unix:path={env['XDG_RUNTIME_DIR']}/bus"
    env["GDK_BACKEND"] = "wayland"
    env["QT_QPA_PLATFORM"] = "wayland"
    env.pop("DISPLAY", None)
    return env


def _optional_env_positive_number(name: str) -> Optional[str]:
    raw = os.getenv(name)
    if raw is None or raw.strip() == "":
        return None
    raw = raw.strip()
    try:
        value = float(raw)
    except ValueError:
        logger.warning("Ignoring invalid %s=%r; expected a positive number.", name, raw)
        return None
    if value <= 0:
        logger.warning("Ignoring invalid %s=%r; expected a positive number.", name, raw)
        return None
    return raw


def _wayland_grim_scale_is_configured() -> bool:
    return _optional_env_positive_number("KYLIN_WAYLAND_GRIM_SCALE") is not None


def _env_int_in_range(name: str, default: str, minimum: int, maximum: int) -> str:
    raw = os.getenv(name, default).strip()
    try:
        value = int(raw)
    except ValueError:
        logger.warning("Ignoring invalid %s=%r; expected an integer.", name, raw)
        return default
    if value < minimum or value > maximum:
        logger.warning(
            "Ignoring invalid %s=%r; expected an integer from %s to %s.",
            name,
            raw,
            minimum,
            maximum,
        )
        return default
    return raw


def _wayland_grim_format() -> str:
    raw = os.getenv("KYLIN_WAYLAND_GRIM_FORMAT", _WAYLAND_GRIM_FORMAT).strip().lower()
    if raw in {"ppm", "png", "jpeg"}:
        return raw
    logger.warning("Ignoring invalid KYLIN_WAYLAND_GRIM_FORMAT=%r; expected ppm, png, or jpeg.", raw)
    return _WAYLAND_GRIM_FORMAT


def _capture_with_grim(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    grim = shutil.which("grim")
    if not grim:
        return None

    image_format = _wayland_grim_format()
    suffix = ".jpg" if image_format == "jpeg" else f".{image_format}"
    with tempfile.NamedTemporaryFile(suffix=suffix, delete=False) as tmp:
        tmp_path = tmp.name
    try:
        cmd = [grim]
        scale = _optional_env_positive_number("KYLIN_WAYLAND_GRIM_SCALE")
        png_level = _env_int_in_range("KYLIN_WAYLAND_GRIM_PNG_LEVEL", _WAYLAND_GRIM_PNG_LEVEL, 0, 9)
        if scale is not None:
            cmd.extend(["-s", scale])
        cmd.extend(["-t", image_format])
        if image_format == "png":
            cmd.extend(["-l", png_level])
        if region is not None:
            x, y, w, h = region
            cmd.extend(["-g", f"{int(x)},{int(y)} {int(w)}x{int(h)}"])
        cmd.append(tmp_path)
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
            env=_wayland_subprocess_env(),
        )
        if proc.returncode != 0:
            if image_format != "png":
                logger.debug(
                    "grim screenshot with %s failed (code=%s), falling back to png: %s",
                    image_format,
                    proc.returncode,
                    proc.stderr.strip(),
                )
                return _capture_with_grim_png(region)
            logger.warning("grim screenshot failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            return None
        return _load_image(tmp_path)
    except Exception as e:
        logger.warning("grim screenshot error: %s", e)
        return None
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


def _capture_with_grim_png(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    grim = shutil.which("grim")
    if not grim:
        return None

    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = tmp.name
    try:
        cmd = [grim]
        scale = _optional_env_positive_number("KYLIN_WAYLAND_GRIM_SCALE")
        png_level = _env_int_in_range("KYLIN_WAYLAND_GRIM_PNG_LEVEL", _WAYLAND_GRIM_PNG_LEVEL, 0, 9)
        if scale is not None:
            cmd.extend(["-s", scale])
        cmd.extend(["-t", "png", "-l", png_level])
        if region is not None:
            x, y, w, h = region
            cmd.extend(["-g", f"{int(x)},{int(y)} {int(w)}x{int(h)}"])
        cmd.append(tmp_path)
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
            env=_wayland_subprocess_env(),
        )
        if proc.returncode != 0:
            logger.warning("grim screenshot failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            return None
        return _load_image(tmp_path)
    except Exception as e:
        logger.warning("grim png fallback screenshot error: %s", e)
        return None
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


def _capture_with_gnome_screenshot(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    gnome_screenshot = shutil.which("gnome-screenshot")
    if not gnome_screenshot:
        return None

    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = tmp.name
    try:
        proc = subprocess.run(
            [gnome_screenshot, "-f", tmp_path],
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
            env=_wayland_subprocess_env(),
        )
        if proc.returncode != 0:
            logger.warning(
                "gnome-screenshot failed (code=%s): %s",
                proc.returncode,
                proc.stderr.strip(),
            )
            return None
        return _crop_wayland_desktop(_load_image(tmp_path), region)
    except Exception as e:
        logger.warning("gnome-screenshot error: %s", e)
        return None
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


def _x11_subprocess_env() -> dict[str, str]:
    env = os.environ.copy()
    if env.get("DISPLAY"):
        env["GDK_BACKEND"] = "x11"
        env["QT_QPA_PLATFORM"] = "xcb"
    return env


def _capture_with_scrot(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    scrot = shutil.which("scrot")
    if not scrot:
        return None

    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = tmp.name
    try:
        cmd = [scrot]
        if region is not None:
            x, y, w, h = region
            cmd.extend(["-a", f"{int(x)},{int(y)},{int(w)},{int(h)}"])
        cmd.append(tmp_path)
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
            timeout=5,
            env=_x11_subprocess_env(),
        )
        if proc.returncode != 0:
            logger.debug("scrot screenshot failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            return None
        return _load_image(tmp_path)
    except Exception as e:
        logger.debug("scrot screenshot error: %s", e)
        return None
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


def _capture_with_mss(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    try:
        import mss
    except Exception:
        return None

    try:
        with mss.mss() as sct:
            if region is None:
                monitor = sct.monitors[0]
            else:
                x, y, w, h = region
                monitor = {
                    "left": int(x),
                    "top": int(y),
                    "width": int(w),
                    "height": int(h),
                }
            raw = sct.grab(monitor)
            return Image.frombytes("RGB", raw.size, raw.rgb)
    except Exception as e:
        logger.debug("mss screenshot error: %s", e)
        return None


def _capture_with_pillow(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    try:
        from PIL import ImageGrab
    except Exception:
        return None

    try:
        bbox = None
        if region is not None:
            x, y, w, h = region
            bbox = (int(x), int(y), int(x + w), int(y + h))
        image = ImageGrab.grab(bbox=bbox)
        image.load()
        return image.convert("RGB")
    except Exception as e:
        logger.debug("Pillow ImageGrab screenshot error: %s", e)
        return None


def get_last_screenshot_backend() -> str:
    return _LAST_SCREENSHOT_BACKEND


def _parse_gdbus_response(stdout: str) -> Optional[dict]:
    match = re.search(r"\(\s*(\d+)\s*,\s*({.*})\s*\)", stdout.strip(), re.S)
    if not match or int(match.group(1)) != 0:
        return None
    try:
        return ast.literal_eval(match.group(2).replace("<", "").replace(">", ""))
    except Exception:
        return None


def _capture_with_portal(region: Optional[tuple[int, int, int, int]]) -> Optional[Image.Image]:
    gdbus = shutil.which("gdbus")
    if not gdbus:
        return None

    handle_token = f"kylin_screenshot_{os.getpid()}_{int(time.time() * 1000)}"
    signal_cmd = [
        gdbus,
        "monitor",
        "--session",
        "--dest",
        "org.freedesktop.portal.Desktop",
    ]
    monitor = None
    try:
        monitor = subprocess.Popen(
            signal_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            env=_wayland_subprocess_env(),
        )
        call_cmd = [
            gdbus,
            "call",
            "--session",
            "--dest",
            "org.freedesktop.portal.Desktop",
            "--object-path",
            "/org/freedesktop/portal/desktop",
            "--method",
            "org.freedesktop.portal.Screenshot.Screenshot",
            "",
            "{'interactive': <false>, 'handle_token': <%r>}" % handle_token,
        ]
        proc = subprocess.run(
            call_cmd,
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
            env=_wayland_subprocess_env(),
        )
        if proc.returncode != 0:
            logger.warning("portal screenshot request failed (code=%s): %s", proc.returncode, proc.stderr.strip())
            return None

        deadline = time.monotonic() + 20
        output = ""
        assert monitor.stdout is not None
        while time.monotonic() < deadline:
            readable, _, _ = select.select([monitor.stdout], [], [], 0.2)
            if not readable:
                continue
            line = monitor.stdout.readline()
            if not line:
                continue
            output += line
            parsed = _parse_gdbus_response(output)
            if not parsed:
                continue
            uri = parsed.get("uri")
            if not isinstance(uri, str) or not uri.startswith("file://"):
                logger.warning("portal screenshot response did not include a file URI.")
                return None
            path = urllib.parse.unquote(urllib.parse.urlparse(uri).path)
            return _crop_wayland_desktop(_load_image(path), region)

        logger.warning("portal screenshot timed out waiting for response.")
        return None
    except Exception as e:
        logger.warning("portal screenshot error: %s", e)
        return None
    finally:
        if monitor is not None and monitor.poll() is None:
            try:
                monitor.terminate()
                monitor.wait(timeout=0.5)
            except Exception:
                try:
                    monitor.kill()
                except Exception:
                    pass


def get_wayland_screen_rect() -> tuple[int, int, int, int]:
    global _WAYLAND_SCREEN_RECT_CACHE

    now = time.monotonic()
    # Refresh before each observation: resolution, scale and target may change.

    screens = list_wayland_screens()
    if screens:
        target_1_based = _parse_target_screen(os.getenv("KYLIN_TARGET_SCREEN"))
        idx = min(max(target_1_based - 1, 0), len(screens) - 1)
        rect = screens[idx]
        _WAYLAND_SCREEN_RECT_CACHE = (now, rect)
        return rect

    raise RuntimeError(
        "Cannot determine Wayland logical monitor geometry. Install/configure "
        "kscreen-doctor or set KYLIN_WAYLAND_SCREEN_RECTS; screenshot pixel dimensions cannot safely determine "
        "input coordinates on a scaled display."
    )


def get_wayland_effective_screen_rect() -> tuple[int, int, int, int]:
    return get_wayland_screen_rect()


def get_cached_wayland_effective_screen_rect() -> tuple[int, int, int, int] | None:
    now = time.monotonic()
    cache_ttl = _wayland_screen_rect_cache_ttl()
    if _WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE is not None:
        cached_at, rect = _WAYLAND_EFFECTIVE_SCREEN_RECT_CACHE
        if now - cached_at < cache_ttl:
            return rect
    return None


def _capture_screenshot(region: Optional[tuple[int, int, int, int]] = None) -> Image.Image:
    """
    Capture a screenshot using the backend appropriate for the Linux session.
    Wayland uses compositor screenshot tools because PyAutoGUI relies on X11.
    """
    global _LAST_SCREENSHOT_BACKEND
    _LAST_SCREENSHOT_BACKEND = ""

    if is_wayland_session():
        image = _capture_with_grim(region)
        if image is not None:
            _LAST_SCREENSHOT_BACKEND = "grim"
            return image
        image = _capture_with_gnome_screenshot(region)
        if image is not None:
            _LAST_SCREENSHOT_BACKEND = "gnome-screenshot"
            return image
        image = _capture_with_portal(region)
        if image is not None:
            _LAST_SCREENSHOT_BACKEND = "xdg-desktop-portal"
            return image
        raise RuntimeError(
            "Wayland screenshot failed: grim, gnome-screenshot, and xdg-desktop-portal "
            "all failed. Check WAYLAND_DISPLAY, XDG_RUNTIME_DIR, DBUS_SESSION_BUS_ADDRESS, "
            "and whether the process is running inside a sandbox."
        )

    backend_order = [
        item.strip().lower()
        for item in os.getenv("KYLIN_X11_SCREENSHOT_BACKEND", "pillow,pyautogui,scrot").split(",")
        if item.strip()
    ]
    for backend in backend_order:
        if backend in {"pillow", "imagegrab", "pil"}:
            image = _capture_with_pillow(region)
            if image is not None:
                _LAST_SCREENSHOT_BACKEND = "pillow"
                return image
        elif backend == "mss":
            image = _capture_with_mss(region)
            if image is not None:
                _LAST_SCREENSHOT_BACKEND = "mss"
                return image
        elif backend == "scrot":
            image = _capture_with_scrot(region)
            if image is not None:
                _LAST_SCREENSHOT_BACKEND = "scrot"
                return image
        elif backend == "pyautogui":
            import pyautogui

            if region is not None:
                _LAST_SCREENSHOT_BACKEND = "pyautogui"
                return pyautogui.screenshot(region=region)
            _LAST_SCREENSHOT_BACKEND = "pyautogui"
            return pyautogui.screenshot()

    import pyautogui

    if region is not None:
        _LAST_SCREENSHOT_BACKEND = "pyautogui"
        return pyautogui.screenshot(region=region)
    _LAST_SCREENSHOT_BACKEND = "pyautogui"
    return pyautogui.screenshot()


def capture_screenshot(region: Optional[tuple[int, int, int, int]] = None) -> Image.Image:
    global _LAST_CAPTURE_RECT, _LAST_CAPTURE_ORIGIN
    region = region if region is not None else get_target_screen_rect()
    _LAST_CAPTURE_RECT = None
    origin = (0, 0)
    if is_wayland_session():
        screens = list_wayland_screens()
        if not screens:
            raise RuntimeError("Cannot map screenshot without Wayland logical geometry")
        origin = (min(r[0] for r in screens), min(r[1] for r in screens))
    image = _capture_screenshot(region)
    if image.width <= 0 or image.height <= 0:
        raise RuntimeError("Empty screenshot")
    if is_wayland_session():
        original_width, original_height = image.size
        image, trimmed_size = _trim_trailing_black_padding(image)
        if trimmed_size is not None:
            # Convert the crop from physical image pixels to logical coordinates.
            # Keep the monitor origin: this heuristic removes only right/bottom.
            x, y, width, height = region
            region = (x, y,
                      max(1, round(width * image.width / original_width)),
                      max(1, round(height * image.height / original_height)))
        if get_last_screenshot_backend() == "grim" and not _wayland_grim_scale_is_configured():
            image = _resize_to_region_size(image, region)
    _LAST_CAPTURE_RECT = region
    _LAST_CAPTURE_ORIGIN = origin
    logger.debug("Screenshot mapping: pixels=%s, desktop_rect=%s", image.size, region)
    return image


def capture_target_screenshot() -> Image.Image:
    return capture_screenshot(region=get_target_screen_rect())
