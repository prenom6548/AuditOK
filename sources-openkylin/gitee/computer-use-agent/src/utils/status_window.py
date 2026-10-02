"""
状态监控窗口模块
提供一个始终置顶的小窗口，显示 Agent 执行的每一步信息
可以通过关闭窗口来终止程序
"""
import tkinter as tk
import tkinter.font as tkfont
from tkinter import ttk
import threading
import queue
import multiprocessing
from typing import Optional, Callable
from datetime import datetime
import logging
import json
import html as html_lib
import base64
import codecs
import io
import os
import platform
import re
import shutil
import subprocess
import socket
import tempfile
import time
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import quote

logger = logging.getLogger("kylin")

os.environ.setdefault("KYLIN_STATUS_BACKEND", "qt")
if os.environ.get("KYLIN_STATUS_BACKEND", "").strip().lower() in {"browser", "html", "web"}:
    os.environ["KYLIN_STATUS_BACKEND"] = "qt"
if os.environ.get("KYLIN_DISABLE_BROWSER_SANDBOX", "").strip().lower() in {"1", "true", "yes"}:
    os.environ.setdefault("QTWEBENGINE_DISABLE_SANDBOX", "1")
os.environ.setdefault(
    "QTWEBENGINE_CHROMIUM_FLAGS",
    "--disable-gpu --disable-vulkan --disable-features=Vulkan",
)
if (
    os.environ.get("XDG_SESSION_TYPE", "").strip().lower() == "wayland"
    and os.environ.get("DISPLAY")
):
    os.environ.setdefault("QT_QPA_PLATFORM", "xcb")
os.environ.setdefault("QT_ENABLE_HIGHDPI_SCALING", "1")
os.environ.setdefault("QT_AUTO_SCREEN_SCALE_FACTOR", "1")
os.environ.setdefault("QT_OPENGL", "software")
os.environ.setdefault("QT_QUICK_BACKEND", "software")
os.environ.setdefault("LIBGL_ALWAYS_SOFTWARE", "1")
os.environ.setdefault("LIBGL_DRI3_DISABLE", "1")
if any(
    os.path.exists(path)
    for path in (
        "/usr/lib/x86_64-linux-gnu/dri/llvmpipe_dri.so",
        "/usr/lib/dri/llvmpipe_dri.so",
        "/usr/lib64/dri/llvmpipe_dri.so",
    )
):
    os.environ.setdefault("MESA_LOADER_DRIVER_OVERRIDE", "llvmpipe")

def _redirect_process_stdio_to_devnull():
    """Prevent GUI child processes from keeping parent capture pipes open."""
    if os.environ.get("KYLIN_STATUS_INHERIT_STDIO", "").strip().lower() in {"1", "true", "yes"}:
        return
    try:
        fd = os.open(os.devnull, os.O_RDWR)
        try:
            os.dup2(fd, 1)
            os.dup2(fd, 2)
        finally:
            os.close(fd)
    except Exception:
        pass

QT_BINDING = None
QtCore = None
QtGui = None
QtWidgets = None
QtWebEngineWidgets = None
QtWebEngineCore = None
QtWebChannel = None
_QT_SIGNAL = None
_QT_SLOT = None

if os.environ.get("KYLIN_STATUS_BACKEND", "").strip().lower() not in {"browser", "html", "web"}:
    for _binding in ("PySide6", "PyQt6", "PyQt5", "PySide2"):
        try:
            if _binding == "PySide6":
                from PySide6 import QtCore as _QtCore, QtGui as _QtGui, QtWidgets as _QtWidgets
                from PySide6 import QtWebEngineWidgets as _QtWebEngineWidgets
                from PySide6 import QtWebEngineCore as _QtWebEngineCore
                from PySide6 import QtWebChannel as _QtWebChannel
                _signal = _QtCore.Signal
                _slot = _QtCore.Slot
            elif _binding == "PyQt6":
                from PyQt6 import QtCore as _QtCore, QtGui as _QtGui, QtWidgets as _QtWidgets
                from PyQt6 import QtWebEngineWidgets as _QtWebEngineWidgets
                from PyQt6 import QtWebEngineCore as _QtWebEngineCore
                from PyQt6 import QtWebChannel as _QtWebChannel
                _signal = _QtCore.pyqtSignal
                _slot = _QtCore.pyqtSlot
            elif _binding == "PySide2":
                from PySide2 import QtCore as _QtCore, QtGui as _QtGui, QtWidgets as _QtWidgets
                from PySide2 import QtWebEngineWidgets as _QtWebEngineWidgets
                from PySide2 import QtWebChannel as _QtWebChannel
                _QtWebEngineCore = _QtWebEngineWidgets
                _signal = _QtCore.Signal
                _slot = _QtCore.Slot
            else:
                from PyQt5 import QtCore as _QtCore, QtGui as _QtGui, QtWidgets as _QtWidgets
                from PyQt5 import QtWebEngineWidgets as _QtWebEngineWidgets
                from PyQt5 import QtWebChannel as _QtWebChannel
                _QtWebEngineCore = _QtWebEngineWidgets
                _signal = _QtCore.pyqtSignal
                _slot = _QtCore.pyqtSlot
            QtCore = _QtCore
            QtGui = _QtGui
            QtWidgets = _QtWidgets
            QtWebEngineWidgets = _QtWebEngineWidgets
            QtWebEngineCore = _QtWebEngineCore
            QtWebChannel = _QtWebChannel
            _QT_SIGNAL = _signal
            _QT_SLOT = _slot
            QT_BINDING = _binding
            break
        except Exception:
            continue

try:
    from fontTools.ttLib import TTCollection, TTFont
except Exception:
    TTCollection = None
    TTFont = None

try:
    from PIL import Image, ImageDraw, ImageFont
except Exception:
    Image = None
    ImageDraw = None
    ImageFont = None

# 根据平台选择合适字体
_FONT_CACHE: dict[int, tuple[str, int]] = {}
_FONT_FILE_CACHE: dict[str, tuple[str, int]] = {}
_FONT_COVERAGE_CACHE: dict[tuple[str, int], set[int]] = {}
_UNICODE_ESCAPE_RE = re.compile(r"(?:\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8})+")


def _env_float(*names: str) -> Optional[float]:
    for name in names:
        raw = os.environ.get(name, "").strip()
        if not raw:
            continue
        try:
            value = float(raw)
        except ValueError:
            logger.warning("忽略无效的缩放值 %s=%r", name, raw)
            continue
        if value > 0:
            return value
    return None


def _dimension_scale(width: int = 0, height: int = 0) -> float:
    if width <= 0 or height <= 0:
        return 1.0
    return max(1.0, min(width / 1920.0, height / 1080.0, 2.2))


def _status_ui_scale(width: int = 0, height: int = 0, dpi_scale: float = 1.0) -> float:
    override = _env_float("KYLIN_STATUS_SCALE", "STATUS_WINDOW_SCALE", "DESKTOP_AGENT_STATUS_SCALE")
    if override:
        return max(0.75, min(override, 3.0))
    return max(1.0, min(max(dpi_scale, _dimension_scale(width, height)), 2.2))


def _scaled(value: int | float, scale: float) -> int:
    return max(1, int(round(value * scale)))


def _decode_unicode_escapes(text: str) -> str:
    """Decode visible JSON-style unicode escapes for display only."""
    if "\\u" not in text and "\\U" not in text:
        return text
    text = re.sub(r"\\+(?=[uU](?:[0-9a-fA-F]{4}|[0-9a-fA-F]{8}))", r"\\", text)

    def replace(match: re.Match) -> str:
        try:
            decoded = codecs.decode(match.group(0), "unicode_escape")
            return decoded.encode("utf-16", "surrogatepass").decode("utf-16", "replace")
        except Exception:
            return match.group(0)

    return _UNICODE_ESCAPE_RE.sub(replace, text)


def _status_font_env() -> str:
    """Return an explicit UI font from either generic or legacy env names."""
    for name in ("STATUS_WINDOW_FONT", "DESKTOP_AGENT_STATUS_FONT", "KYLIN_STATUS_FONT"):
        value = os.environ.get(name, "").strip()
        if value:
            return value
    return ""


def _primary_cjk_font_pattern() -> str:
    return os.environ.get(
        "STATUS_WINDOW_PRIMARY_FONT",
        os.environ.get("KYLIN_STATUS_PRIMARY_FONT", "Noto Sans CJK SC:style=Regular:lang=zh-cn"),
    ).strip()


def _extended_cjk_font_pattern() -> str:
    return os.environ.get(
        "STATUS_WINDOW_EXTENDED_FONT",
        os.environ.get(
            "KYLIN_STATUS_EXTENDED_FONT",
            "Noto Sans CJK SC:style=Regular:lang=zh-cn",
        ),
    ).strip()


def _font_size_to_pixels(size: int) -> int:
    return max(9, size)


def _font_stack_css(point_size: Optional[int] = None) -> str:
    explicit_font = _status_font_env()
    families = []
    if explicit_font:
        families.append(explicit_font)
    families.extend(
        [
            "Noto Sans CJK SC",
            "Source Han Sans SC",
            "Microsoft YaHei UI",
            "Microsoft YaHei",
            "PingFang SC",
            "Hiragino Sans GB",
            "Heiti SC",
            "STHeiti",
            "WenQuanYi Micro Hei",
            "WenQuanYi Zen Hei",
            "Droid Sans Fallback",
            "SimHei",
            "SimSun",
            "Segoe UI",
            "Arial Unicode MS",
            "Helvetica Neue",
            "Arial",
            "sans-serif",
        ]
    )
    seen = set()
    css_families = []
    for family in families:
        family = family.strip()
        if not family or family in seen:
            continue
        seen.add(family)
        css_families.append(family if family == "sans-serif" else f'"{family}"')
    css = f"font-family: {', '.join(css_families)};"
    if point_size is not None:
        css += f" font-size: {point_size}pt;"
    return css


def _configure_qt_high_dpi():
    if not QtWidgets or not QtCore:
        return
    try:
        if hasattr(QtCore.Qt, "HighDpiScaleFactorRoundingPolicy"):
            policy = QtCore.Qt.HighDpiScaleFactorRoundingPolicy.PassThrough
            QtWidgets.QApplication.setHighDpiScaleFactorRoundingPolicy(policy)
    except Exception:
        pass
    try:
        if hasattr(QtCore.Qt, "AA_EnableHighDpiScaling"):
            QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling, True)
        if hasattr(QtCore.Qt, "AA_UseHighDpiPixmaps"):
            QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps, True)
    except Exception:
        pass


def _place_qt_window_bottom_right(app, window, margin: int = 10):
    screen = app.primaryScreen().availableGeometry()
    x = max(screen.left() + margin, screen.right() - window.width() - margin)
    y = max(screen.top() + margin, screen.bottom() - window.height() - margin)
    window.setGeometry(x, y, window.width(), window.height())
    window.move(x, y)
    try:
        window.raise_()
        window.activateWindow()
    except Exception:
        pass


def _schedule_qt_window_bottom_right(app, window):
    _place_qt_window_bottom_right(app, window)
    if not QtCore:
        return
    for delay in (0, 200, 800, 1600):
        QtCore.QTimer.singleShot(delay, lambda app=app, window=window: _place_qt_window_bottom_right(app, window))


def _qt_screen_scale(app) -> float:
    try:
        screen = app.primaryScreen()
        geom = screen.availableGeometry()
        dpi_scale = max(1.0, float(screen.logicalDotsPerInch()) / 96.0)
        return _status_ui_scale(geom.width(), geom.height(), dpi_scale)
    except Exception:
        return _status_ui_scale()


def _qt_window_flag(name: str):
    value = getattr(QtCore.Qt, name, None)
    if value is not None:
        return value
    window_type = getattr(QtCore.Qt, "WindowType", None)
    return getattr(window_type, name, None) if window_type is not None else None


def _set_always_on_top(window):
    flags = window.windowFlags()
    for flag_name in ("WindowStaysOnTopHint", "Tool"):
        flag = _qt_window_flag(flag_name)
        if flag is not None:
            flags |= flag
    window.setWindowFlags(flags)


def _qt_context_menu_policy(name: str):
    value = getattr(QtCore.Qt, name, None)
    if value is not None:
        return value
    policy = getattr(QtCore.Qt, "ContextMenuPolicy", None)
    return getattr(policy, name, None) if policy is not None else None


def _qt_cursor_shape(name: str):
    value = getattr(QtCore.Qt, name, None)
    if value is not None:
        return value
    shape = getattr(QtCore.Qt, "CursorShape", None)
    return getattr(shape, name, None) if shape is not None else None


def _fontconfig_match_family(pattern: str) -> Optional[str]:
    try:
        result = subprocess.run(
            ["fc-match", "-f", "%{family}\n", pattern],
            capture_output=True,
            text=True,
            timeout=1,
            check=False,
        )
    except Exception:
        return None

    family_line = result.stdout.splitlines()[0].strip() if result.stdout else ""
    if not family_line:
        return None
    families = [family.strip() for family in family_line.split(",") if family.strip()]
    if "Light" in pattern:
        for family in families:
            if "Light" in family:
                return family
    if "DemiLight" in pattern:
        for family in families:
            if "DemiLight" in family:
                return family
    return families[0] if families else None


def _fontconfig_match_file(pattern: str) -> Optional[tuple[str, int]]:
    if pattern in _FONT_FILE_CACHE:
        return _FONT_FILE_CACHE[pattern]
    try:
        result = subprocess.run(
            ["fc-match", "-f", "%{file}|%{index}\n", pattern],
            capture_output=True,
            text=True,
            timeout=1,
            check=False,
        )
    except Exception:
        return None
    line = result.stdout.splitlines()[0].strip() if result.stdout else ""
    if not line or "|" not in line:
        return None
    file_path, index_text = line.rsplit("|", 1)
    try:
        index = int(index_text or 0)
    except ValueError:
        index = 0
    if os.path.exists(file_path):
        _FONT_FILE_CACHE[pattern] = (file_path, index)
        return _FONT_FILE_CACHE[pattern]
    return None


def _font_coverage(file_path: str, index: int = 0) -> set[int]:
    key = (file_path, index)
    if key in _FONT_COVERAGE_CACHE:
        return _FONT_COVERAGE_CACHE[key]
    coverage: set[int] = set()
    if TTFont is None:
        return coverage
    try:
        if file_path.lower().endswith(".ttc") and TTCollection is not None:
            collection = TTCollection(file_path)
            font = collection.fonts[index]
        else:
            font = TTFont(file_path, fontNumber=index)
        for table in font["cmap"].tables:
            coverage.update(table.cmap.keys())
    except Exception as e:
        logger.debug("读取字体覆盖范围失败：%s", e)
    _FONT_COVERAGE_CACHE[key] = coverage
    return coverage


def _rich_text_html(text: str) -> str:
    value = StatusWindow._localize_display_text(text)
    return html_lib.escape(value)


def _file_uri(path: str) -> str:
    return "file://" + quote(os.path.abspath(path))




def _load_pil_font(pattern: str, size: int):
    if ImageFont is None:
        return None
    matched = _fontconfig_match_file(pattern)
    if not matched:
        return None
    file_path, index = matched
    try:
        return ImageFont.truetype(file_path, size=size, index=index)
    except TypeError:
        try:
            return ImageFont.truetype(file_path, size=size)
        except Exception:
            return None
    except Exception:
        return None


def _pil_text_width(draw, text: str, font) -> int:
    if not text:
        return 0
    try:
        return int(draw.textlength(text, font=font))
    except Exception:
        box = draw.textbbox((0, 0), text, font=font)
        return box[2] - box[0]


def _text_image_data_url(text: str, width: int, height: int, font_size: int = 15, scale: int = 3) -> str:
    if Image is None or ImageDraw is None or ImageFont is None:
        return ""

    value = StatusWindow._localize_display_text(text)
    primary_pattern = _primary_cjk_font_pattern()
    extended_pattern = _extended_cjk_font_pattern()
    primary_match = _fontconfig_match_file(primary_pattern)
    extended_match = _fontconfig_match_file(extended_pattern)
    render_width = width * scale
    render_height = height * scale
    render_font_size = font_size * scale

    primary_font = _load_pil_font(primary_pattern, render_font_size)
    extended_font = _load_pil_font(extended_pattern, render_font_size)
    fallback_font = primary_font or extended_font or ImageFont.load_default()
    primary_coverage = _font_coverage(*primary_match) if primary_match else set()
    extended_coverage = _font_coverage(*extended_match) if extended_match else set()

    def font_for(ch: str):
        codepoint = ord(ch)
        if codepoint in primary_coverage and primary_font:
            return primary_font
        if codepoint in extended_coverage and extended_font:
            return extended_font
        return fallback_font

    img = Image.new("RGBA", (render_width, render_height), (255, 255, 255, 0))
    draw = ImageDraw.Draw(img)
    x = 0
    y = 0
    line_height = max(render_font_size + (6 * scale), 21 * scale)
    max_y = render_height - line_height
    text_color = (31, 41, 51, 255)

    for ch in value:
        if ch == "\n":
            x = 0
            y += line_height
            if y > max_y:
                break
            continue
        font = font_for(ch)
        char_width = _pil_text_width(draw, ch, font)
        if x and x + char_width > render_width:
            x = 0
            y += line_height
            if y > max_y:
                break
        draw.text((x, y), ch, font=font, fill=text_color)
        x += char_width

    output = io.BytesIO()
    img.save(output, format="PNG")
    encoded = base64.b64encode(output.getvalue()).decode("ascii")
    return f"data:image/png;base64,{encoded}"


def _available_font_families() -> set[str]:
    families: set[str] = set()
    try:
        families.update(str(name) for name in tkfont.families())
    except Exception:
        pass

    if platform.system() == "Linux":
        try:
            result = subprocess.run(
                ["fc-list", ":", "family"],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
            for line in result.stdout.splitlines():
                for family in line.split(","):
                    family = family.strip()
                    if family:
                        families.add(family)
        except Exception:
            pass

    return families


def get_system_font(size=8):
    """获取系统字体，优先选择完整中文字体，减少中文显示为方块。"""
    if size in _FONT_CACHE:
        return _FONT_CACHE[size]

    env_font = _status_font_env()
    font_size = _font_size_to_pixels(size)
    if env_font:
        _FONT_CACHE[size] = (env_font, font_size)
        return _FONT_CACHE[size]

    system = platform.system()
    if system == "Linux":
        for pattern in (
            "Noto Sans CJK SC:style=Regular:lang=zh-cn",
            "Source Han Sans SC:style=Regular:lang=zh-cn",
            "Noto Sans CJK:style=Regular:lang=zh-cn",
            "Noto Serif CJK SC:style=Regular:lang=zh-cn",
            "Droid Sans Fallback:style=Regular:lang=zh-cn",
        ):
            matched = _fontconfig_match_family(pattern)
            if matched:
                _FONT_CACHE[size] = (matched, font_size)
                return _FONT_CACHE[size]
        candidates = [
            "Noto Sans CJK SC",
            "Noto Sans CJK",
            "Noto Serif CJK SC",
            "Source Han Sans SC",
            "Microsoft YaHei",
            "WenQuanYi Micro Hei",
            "WenQuanYi Zen Hei",
            "Droid Sans Fallback",
            "GB_SS_GB18030_Extended",
            "GB_SS_GB18030",
            "国标宋体",
            "STHeiti",
            "GB_HT_GB18030",
            "国标黑体",
            "Noto Sans Mono CJK SC",
            "SimHei",
            "FreeSans",
        ]
    elif system == "Darwin":
        candidates = ["PingFang SC", "Heiti SC", "Songti SC", "Arial Unicode MS", "Helvetica"]
    else:  # Windows
        candidates = ["Microsoft YaHei UI", "Microsoft YaHei", "SimHei", "SimSun", "Segoe UI"]

    available = _available_font_families()
    for family in candidates:
        if family in available:
            _FONT_CACHE[size] = (family, font_size)
            return _FONT_CACHE[size]

    fallback = "TkDefaultFont"
    if system == "Linux" and "DejaVu Sans" in available:
        fallback = "DejaVu Sans"
    _FONT_CACHE[size] = (fallback, font_size)
    return _FONT_CACHE[size]


def _is_emoji_or_symbol_codepoint(codepoint: int) -> bool:
    return (
        0x1F000 <= codepoint <= 0x1FAFF
        or 0x2600 <= codepoint <= 0x27BF
        or 0xFE00 <= codepoint <= 0xFE0F
    )


_DISPLAY_PHRASES: tuple[tuple[str, str], ...] = (
    ("Screenshot captured", "截图已完成"),
    ("Brain analysis complete", "Brain 分析完成"),
    ("Brain inference", "Brain 推理"),
    ("Brain Total", "Brain 总耗时"),
    ("Brain LLM", "Brain 推理"),
    ("Actor inference", "Actor 推理"),
    ("Actor LLM", "Actor 推理"),
    ("Actions executed", "动作执行完成"),
    ("Action Exec", "动作执行"),
    ("Task completed successfully", "任务已成功完成"),
    ("Starting agent...", "正在启动智能体..."),
    ("Max steps reached", "已达到最大步骤数"),
    ("Total execution time", "总执行耗时"),
    ("Task Complete", "任务完成"),
    ("Task completed", "任务完成"),
    ("Task:", "任务："),
    ("Error:", "错误："),
    ("Agent stopped", "智能体已停止"),
    ("Log saved", "日志已保存"),
    ("Initializing...", "正在初始化..."),
    ("Complete", "完成"),
    ("Failed", "失败"),
    ("Waiting", "等待中"),
)

_DISPLAY_WORDS: tuple[tuple[str, str], ...] = (
    ("input_text", "输入文本"),
    ("record_info", "记录信息"),
    ("scroll_down", "下滚"),
    ("scroll_up", "上滚"),
    ("move_mouse", "移动鼠标"),
    ("open_app", "打开应用"),
    ("DoubleClick", "双击"),
    ("RightSingle", "右键点击"),
    ("Click", "左键点击"),
    ("Hotkey", "快捷键"),
    ("Wait", "等待"),
    ("Brain", "分析模型"),
    ("Actor", "动作模型"),
    ("action", "动作"),
    ("actions", "动作"),
    ("position", "坐标"),
    ("amount", "幅度"),
    ("text", "文本"),
    ("key", "按键"),
    ("result", "结果"),
    ("error", "错误"),
    ("status", "状态"),
    ("goal", "目标"),
    ("step", "步骤"),
)


class StatusWindow:
    """
    状态监控窗口类
    提供一个始终置顶的小窗口，显示 Agent 执行状态
    """

    def __init__(self, title: str = "任务监视器", on_close: Optional[Callable[[], None]] = None,
                 save_dir: Optional[str] = None):
        if tk.__class__.__name__ == "_FakeTkinter":
            raise RuntimeError("Tkinter is not available")
        self.title = title
        self.on_close = on_close
        self.save_dir = save_dir
        self._stop_requested = False
        self._closing = False
        self._message_queue = queue.Queue()
        self._window_ready = threading.Event()
        self._window: Optional[tk.Tk] = None
        self._step_label = None
        self._status_label = None
        self._goal_text = None
        self._log_text = None
        self._detail_text = None
        self._time_label = None
        self._ui_scale = 1.0
        self._start_time: Optional[datetime] = None
        self._step_records = []
        self._session_data = {
            "start_time": None,
            "end_time": None,
            "end_reason": None,
            "total_steps": 0,
            "task": ""
        }

        # 启动 GUI 线程
        self._gui_thread = threading.Thread(target=self._run_gui, daemon=True, name="StatusWindowGUI")
        self._gui_thread.start()

        # 等待窗口就绪
        self._window_ready.wait(timeout=5.0)
        if not self._window_ready.is_set():
            logger.warning("Status window timeout")

    @staticmethod
    def _clean_display_text(text: str) -> str:
        """清理控制字符，避免 Tk 文本框显示异常。"""
        if text is None:
            return ""
        value = _decode_unicode_escapes(str(text))
        cleaned = []
        for ch in value:
            codepoint = ord(ch)
            if ch in "\n\t":
                cleaned.append(ch)
            elif codepoint < 32:
                cleaned.append(" ")
            elif 0xE000 <= codepoint <= 0xF8FF:
                cleaned.append(" ")
            elif _is_emoji_or_symbol_codepoint(codepoint):
                cleaned.append(" ")
            else:
                cleaned.append(ch)
        return "".join(cleaned)

    @classmethod
    def _localize_display_text(cls, text: str) -> str:
        """仅本地化窗口展示文本，不修改底层协议、日志文件或动作数据。"""
        value = cls._clean_display_text(text)
        for source, target in _DISPLAY_PHRASES:
            value = value.replace(source, target)

        value = re.sub(r"\bStep\s+(\d+)\b", r"步骤 \1", value, flags=re.IGNORECASE)
        value = re.sub(
            r"Executing\s+(\d+)\s+actions?\.{0,3}",
            r"正在执行 \1 个动作",
            value,
            flags=re.IGNORECASE,
        )
        value = re.sub(r"\b(\d+)\s+actions?\b", r"\1 个动作", value, flags=re.IGNORECASE)

        for source, target in _DISPLAY_WORDS:
            value = re.sub(rf"\b{re.escape(source)}\b", target, value)
        return re.sub(r"[ \t]{2,}", " ", value).strip()

    @classmethod
    def _is_goal_duplicate_operation(cls, text: str) -> bool:
        value = cls._localize_display_text(text)
        return "正在执行" in value and "动作" in value

    @classmethod
    def _is_primary_operation_message(cls, text: str, level: str = "info") -> bool:
        value = cls._localize_display_text(text)
        if not value or value == "---":
            return False
        if cls._is_goal_duplicate_operation(value):
            return False
        if "截图" in value:
            return False
        if level != "detail":
            return False
        if any(marker in value for marker in ("分析模型推理", "动作模型推理", "动作执行", "结果解析", "截图抓取", "截图保存", "代码执行开销")):
            return True
        if re.match(r"^动作\s*\d+\s*[:：]", value):
            return True
        if re.match(r"^步骤\s*\d+\s*总计[:：]", value):
            return True
        return False

    def _configure_default_fonts(self):
        """统一 Tk 默认字体，避免部分控件落到缺字字体。"""
        family, size = get_system_font(size=_scaled(9, getattr(self, "_ui_scale", 1.0)))
        font_names = (
            "TkDefaultFont",
            "TkTextFont",
            "TkFixedFont",
            "TkMenuFont",
            "TkHeadingFont",
            "TkCaptionFont",
            "TkSmallCaptionFont",
            "TkIconFont",
            "TkTooltipFont",
        )
        for font_name in font_names:
            try:
                font = tkfont.nametofont(font_name)
                font.configure(family=family, size=size, weight="normal")
            except Exception:
                pass
        try:
            self._window.option_add("*Font", f"{{{family}}} {size}")
        except Exception:
            pass

    def _run_gui(self):
        """在独立的线程中运行 Tkinter 主循环"""
        try:
            self._window = tk.Tk()
            self._window.title("任务监视器")

            screen_width = self._window.winfo_screenwidth()
            screen_height = self._window.winfo_screenheight()
            dpi_scale = max(1.0, self._window.winfo_fpixels("1i") / 96.0)
            self._ui_scale = _status_ui_scale(screen_width, screen_height, dpi_scale)
            self._configure_default_fonts()

            window_width = _scaled(380, self._ui_scale)
            window_height = _scaled(230, self._ui_scale)
            x = max(10, screen_width - window_width - 10)
            y = max(10, screen_height - window_height - 10)

            self._window.geometry(f"{window_width}x{window_height}+{x}+{y}")
            self._window.minsize(_scaled(360, self._ui_scale), _scaled(220, self._ui_scale))
            self._window.attributes("-topmost", True)
            self._window.resizable(True, True)
            self._window.configure(bg='#f7f8fa')

            # 设置窗口关闭协议
            self._window.protocol("WM_DELETE_WINDOW", self._on_window_close)

            # 创建 UI
            self._create_ui()

            # 记录启动时间
            self._start_time = datetime.now()
            self._session_data["start_time"] = self._start_time.isoformat()

            # 标记窗口就绪
            self._window_ready.set()

            # 启动消息处理
            self._process_queue()

            # 运行主循环
            self._window.mainloop()

        except Exception as e:
            logger.exception(f"Status window error: {e}")
            self._window_ready.set()

    def _create_ui(self):
        """创建用户界面"""
        if not self._window:
            return

        page_bg = "#f7f8fa"
        panel_bg = "#ffffff"
        text_bg = "#ffffff"
        border_color = "#e1e5eb"
        primary = "#1f5f99"
        text_color = "#20242a"
        muted_text = "#657080"
        self._create_compact_ui(page_bg, panel_bg, text_bg, border_color, primary, text_color, muted_text)
        return

    def _create_compact_ui(
        self,
        page_bg: str,
        panel_bg: str,
        text_bg: str,
        border_color: str,
        primary: str,
        text_color: str,
        muted_text: str,
    ):
        """创建不依赖浏览器的紧凑 Tk 监视窗口。"""
        scale = getattr(self, "_ui_scale", 1.0)
        main_frame = tk.Frame(self._window, bg=page_bg, padx=_scaled(10, scale), pady=_scaled(9, scale))
        main_frame.pack(fill=tk.BOTH, expand=True)

        title_font, title_font_size = get_system_font(size=_scaled(11, scale))
        title = tk.Label(
            main_frame,
            text="任务监视器",
            font=(title_font, title_font_size, "normal"),
            bg=page_bg,
            fg=text_color,
            anchor=tk.W,
        )
        title.pack(fill=tk.X, pady=(0, _scaled(6, scale)))

        label_font, label_font_size = get_system_font(size=_scaled(9, scale))
        body_font, body_font_size = get_system_font(size=_scaled(10, scale))

        def make_section(title_text: str, height: int, bottom: int = 7) -> tk.Frame:
            section = tk.Frame(main_frame, bg=page_bg, height=_scaled(height, scale))
            section.pack(fill=tk.X, pady=(0, bottom))
            section.pack_propagate(False)
            tk.Label(
                section,
                text=title_text,
                font=(label_font, label_font_size, "normal"),
                bg=page_bg,
                fg=muted_text,
                anchor=tk.W,
            ).pack(fill=tk.X, pady=(0, _scaled(3, scale)))
            body = tk.Frame(
                section,
                bg=text_bg,
                highlightbackground=border_color,
                highlightcolor=border_color,
                highlightthickness=1,
                bd=0,
            )
            body.pack(fill=tk.BOTH, expand=True)
            return body

        goal_text_frame = make_section("当前目标", 56)
        self._goal_text = tk.Text(
            goal_text_frame,
            height=2,
            wrap=tk.CHAR,
            font=(body_font, body_font_size),
            bg=text_bg,
            fg=text_color,
            relief=tk.FLAT,
            borderwidth=0,
            padx=_scaled(9, scale),
            pady=_scaled(6, scale),
            spacing1=1,
            spacing2=0,
            spacing3=1,
            insertwidth=1,
        )
        self._goal_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self._goal_text.insert(tk.END, "等待任务...")
        self._goal_text.config(state=tk.DISABLED)

        detail_text_frame = make_section("当前操作", 88)
        self._detail_text = tk.Text(
            detail_text_frame,
            height=2,
            wrap=tk.CHAR,
            font=(body_font, body_font_size),
            bg=text_bg,
            fg=text_color,
            relief=tk.FLAT,
            borderwidth=0,
            padx=_scaled(9, scale),
            pady=_scaled(6, scale),
            spacing1=1,
            spacing2=0,
            spacing3=1,
            insertwidth=1,
        )
        self._detail_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self._detail_text.config(state=tk.DISABLED)
        self._log_text = self._detail_text
        for tag, color in {
            "info": text_color,
            "success": "#22863a",
            "warning": "#b7791f",
            "error": "#c53030",
            "timing": primary,
        }.items():
            self._detail_text.tag_config(tag, foreground=color)

        bottom_frame = tk.Frame(main_frame, bg=page_bg)
        bottom_frame.pack(fill=tk.X)
        bottom_font, bottom_font_size = get_system_font(size=_scaled(10, scale))

        tk.Label(
            bottom_frame,
            text="步骤：",
            font=(bottom_font, bottom_font_size),
            bg=page_bg,
            fg=muted_text,
        ).pack(side=tk.LEFT)
        self._step_label = tk.Label(
            bottom_frame,
            text="0",
            font=(bottom_font, bottom_font_size),
            bg=page_bg,
            fg=primary,
        )
        self._step_label.pack(side=tk.LEFT, padx=(_scaled(2, scale), _scaled(10, scale)))
        self._status_label = tk.Label(
            bottom_frame,
            text="就绪",
            font=(bottom_font, bottom_font_size),
            bg=page_bg,
            fg="#22863a",
        )
        self._status_label.pack(side=tk.LEFT)

        self._time_label = tk.Label(
            bottom_frame,
            text="00:00:00",
            font=(bottom_font, bottom_font_size),
            bg=page_bg,
            fg=muted_text,
        )
        self._time_label.pack(side=tk.RIGHT, padx=(_scaled(8, scale), 0))

        self._stop_button = tk.Button(
            bottom_frame,
            text="停止",
            font=(bottom_font, bottom_font_size, "normal"),
            bg="#c53030",
            fg="white",
            activebackground="#9b2c2c",
            activeforeground="white",
            relief=tk.FLAT,
            padx=_scaled(12, scale),
            pady=_scaled(2, scale),
            cursor="hand2",
            command=self._on_stop_button_click,
        )
        self._stop_button.pack(side=tk.RIGHT)

        self._update_elapsed_time()
        return

        # 主框架
        main_frame = tk.Frame(self._window, bg=page_bg, padx=10, pady=9)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # === 标题栏 ===
        title_font, title_font_size = get_system_font(size=12)
        title_frame = tk.Frame(main_frame, bg=page_bg, height=32)
        title_frame.pack(fill=tk.X, pady=(0, 7))
        title_frame.pack_propagate(False)

        tk.Label(
            title_frame,
            text="任务监视器",
            font=(title_font, title_font_size, "normal"),
            bg=page_bg,
            fg=text_color,
        ).pack(side=tk.LEFT, padx=2, pady=3)

        # === 状态区域 ===
        status_frame = tk.Frame(
            main_frame,
            bg=panel_bg,
            highlightbackground=border_color,
            highlightcolor=border_color,
            highlightthickness=1,
            bd=0,
        )
        status_frame.pack(fill=tk.X, pady=(0, 8))

        # 获取系统字体
        label_font, label_font_size = get_system_font(size=10)
        
        # 步骤行
        step_row = tk.Frame(status_frame, bg=panel_bg)
        step_row.pack(fill=tk.X, padx=10, pady=6)

        tk.Label(
            step_row,
            text="步骤：",
            font=(label_font, label_font_size, "normal"),
            bg=panel_bg,
            fg=muted_text
        ).pack(side=tk.LEFT)

        self._step_label = tk.Label(
            step_row,
            text="0",
            font=(label_font, label_font_size, "normal"),
            bg=panel_bg,
            fg=primary
        )
        self._step_label.pack(side=tk.LEFT, padx=(5, 0))

        # 状态标签
        self._status_label = tk.Label(
            step_row,
            text="就绪",
            font=(label_font, label_font_size, "normal"),
            bg=panel_bg,
            fg='#22863a'
        )
        self._status_label.pack(side=tk.RIGHT)

        def make_section(title: str, height: int, bottom: int = 7) -> tk.Frame:
            section = tk.Frame(main_frame, bg=page_bg)
            section.pack(fill=tk.X, pady=(0, bottom))
            section.pack_propagate(False)
            section.configure(height=height)

            header_font, header_font_size = get_system_font(size=9)
            tk.Label(
                section,
                text=title,
                font=(header_font, header_font_size, "normal"),
                bg=page_bg,
                fg=muted_text,
                anchor=tk.W,
            ).pack(fill=tk.X, pady=(0, 3))

            body = tk.Frame(
                section,
                bg=text_bg,
                highlightbackground=border_color,
                highlightcolor=border_color,
                highlightthickness=1,
                bd=0,
            )
            body.pack(fill=tk.BOTH, expand=True)
            return body

        # === 当前目标区域 ===
        goal_font, goal_font_size = get_system_font(size=10)
        goal_text_frame = make_section("当前目标", 72)
        self._goal_text = tk.Text(
            goal_text_frame,
            height=3,
            wrap=tk.CHAR,
            font=(goal_font, goal_font_size),
            bg=text_bg,
            fg=text_color,
            relief=tk.FLAT,
            borderwidth=0,
            padx=10,
            pady=6,
            spacing1=2,
            spacing2=0,
            spacing3=2,
            insertwidth=1
        )
        goal_scroll = ttk.Scrollbar(goal_text_frame, orient=tk.VERTICAL, command=self._goal_text.yview)
        self._goal_text.configure(yscrollcommand=goal_scroll.set)
        self._goal_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        goal_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._goal_text.insert(tk.END, "等待任务...")
        self._goal_text.config(state=tk.DISABLED)

        # === 步骤详情区域 ===
        # 获取系统字体
        ui_font, ui_font_size = get_system_font(size=10)
        log_font, log_font_size = get_system_font(size=9)

        detail_text_frame = make_section("步骤详情", 180)
        self._detail_text = tk.Text(
            detail_text_frame,
            height=5,
            wrap=tk.CHAR,
            font=(ui_font, ui_font_size),
            bg=text_bg,
            fg=text_color,
            relief=tk.FLAT,
            borderwidth=0,
            padx=10,
            pady=6,
            spacing1=2,
            spacing2=0,
            spacing3=2,
            insertwidth=1
        )
        detail_scroll = ttk.Scrollbar(detail_text_frame, orient=tk.VERTICAL, command=self._detail_text.yview)
        self._detail_text.configure(yscrollcommand=detail_scroll.set)
        self._detail_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        detail_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._detail_text.config(state=tk.DISABLED)

        # === 日志区域 ===
        log_text_frame = make_section("事件日志", 105)
        self._log_text = tk.Text(
            log_text_frame,
            height=3,
            wrap=tk.CHAR,
            font=(log_font, log_font_size),
            bg=text_bg,
            fg=text_color,
            relief=tk.FLAT,
            borderwidth=0,
            padx=10,
            pady=6,
            spacing1=2,
            spacing2=0,
            spacing3=2,
            insertwidth=1
        )
        log_scroll = ttk.Scrollbar(log_text_frame, orient=tk.VERTICAL, command=self._log_text.yview)
        self._log_text.configure(yscrollcommand=log_scroll.set)
        self._log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._log_text.config(state=tk.DISABLED)

        # 配置日志颜色
        self._log_text.tag_config("info", foreground=text_color)
        self._log_text.tag_config("success", foreground='#22863a')
        self._log_text.tag_config("warning", foreground='#b7791f')
        self._log_text.tag_config("error", foreground='#c53030')
        self._log_text.tag_config("timing", foreground=primary)

        # === 底部状态栏 ===
        bottom_frame = tk.Frame(main_frame, bg=page_bg)
        bottom_frame.pack(fill=tk.X)

        button_font, button_font_size = get_system_font(size=10)

        # 总运行时间
        self._time_label = tk.Label(
            bottom_frame,
            text="总耗时：00:00:00",
            font=(button_font, button_font_size),
            bg=page_bg,
            fg=muted_text
        )
        self._time_label.pack(side=tk.LEFT, padx=2, pady=4)

        # 保存按钮
        save_button = tk.Button(
            bottom_frame,
            text="保存日志",
            font=(button_font, button_font_size),
            bg='#5d6b7a',
            fg='white',
            relief=tk.FLAT,
            padx=12,
            pady=3,
            cursor='hand2',
            command=self._save_current_log
        )
        save_button.pack(side=tk.RIGHT, padx=3, pady=3)

        # 停止按钮
        self._stop_button = tk.Button(
            bottom_frame,
            text="停止",
            font=(button_font, button_font_size, "normal"),
            bg='#c53030',
            fg='white',
            activebackground='#9b2c2c',
            activeforeground='white',
            relief=tk.FLAT,
            padx=14,
            pady=3,
            cursor='hand2',
            command=self._on_stop_button_click
        )
        self._stop_button.pack(side=tk.RIGHT, padx=3, pady=3)

        # 启动计时器
        self._update_elapsed_time()

    def _update_elapsed_time(self):
        """更新经过时间显示"""
        if self._window and self._start_time:
            elapsed = datetime.now() - self._start_time
            hours, remainder = divmod(int(elapsed.total_seconds()), 3600)
            minutes, seconds = divmod(remainder, 60)
            time_str = f"{hours:02d}:{minutes:02d}:{seconds:02d}"
            self._time_label.config(text=f"总耗时：{time_str}")

        if self._window:
            self._window.after(1000, self._update_elapsed_time)

    def _process_queue(self):
        """处理消息队列中的消息"""
        try:
            while True:
                try:
                    msg = self._message_queue.get_nowait()
                    self._handle_message(msg)
                except queue.Empty:
                    break
        except Exception as e:
            logger.exception(f"Message queue error: {e}")

        if self._window:
            self._window.after(50, self._process_queue)

    def _handle_message(self, msg: dict):
        """处理单条消息"""
        msg_type = msg.get("type", "")

        if msg_type == "update_step":
            self._update_step_display(
                msg.get("step_number", 0),
                msg.get("goal", ""),
                msg.get("status", "pending")
            )
        elif msg_type == "add_log":
            self._add_log_entry(
                msg.get("message", ""),
                msg.get("level", "info")
            )
        elif msg_type == "add_detail":
            self._add_detail_entry(msg.get("message", ""))
        elif msg_type == "save_record":
            self._save_step_record(msg.get("record", {}))
        elif msg_type == "set_task":
            self._session_data["task"] = msg.get("task", "")
        elif msg_type == "finalize_session":
            self._finalize_session(msg.get("reason", "unknown"))

    def _update_step_display(self, step_number: int, goal: str, status: str):
        """更新步骤显示"""
        if self._step_label:
            self._step_label.config(text=str(step_number))

        if self._status_label:
            status_config = {
                "pending": {"text": "等待中", "color": '#6c757d'},
                "running": {"text": "运行中", "color": '#2c5aa0'},
                "success": {"text": "已完成", "color": '#28a745'},
                "failed": {"text": "失败", "color": '#ff9800'},
                "error": {"text": "错误", "color": '#dc3545'},
                "warning": {"text": "警告", "color": '#ff9800'},
            }
            config = status_config.get(status, {"text": self._localize_display_text(status), "color": '#333'})
            self._status_label.config(text=config["text"], fg=config["color"])

        if self._goal_text:
            self._goal_text.config(state=tk.NORMAL)
            self._goal_text.delete("1.0", tk.END)
            self._goal_text.insert(tk.END, self._localize_display_text(goal) if goal else "等待任务...")
            self._goal_text.see("1.0")
            self._goal_text.config(state=tk.DISABLED)

    def _add_detail_entry(self, message: str):
        """添加详情条目"""
        if not self._is_primary_operation_message(message, "detail"):
            return
        if self._detail_text:
            self._detail_text.config(state=tk.NORMAL)
            self._detail_text.insert(tk.END, self._localize_display_text(message) + "\n", "timing")
            self._detail_text.see(tk.END)
            self._detail_text.config(state=tk.DISABLED)

    def _add_log_entry(self, message: str, level: str = "info"):
        """添加日志条目"""
        if not self._is_primary_operation_message(message, level):
            return
        if self._log_text:
            self._log_text.config(state=tk.NORMAL)

            timestamp = datetime.now().strftime("%H:%M:%S")
            log_line = f"[{timestamp}] {self._localize_display_text(message)}\n"

            tag = level
            if level not in ["info", "success", "warning", "error", "timing"]:
                tag = "info"

            self._log_text.insert(tk.END, log_line, tag)
            self._log_text.see(tk.END)
            self._log_text.config(state=tk.DISABLED)

    def _save_current_log(self):
        """保存当前日志"""
        self._save_all_records("manual_save")

    def _save_all_records(self, reason: str):
        """保存当前会话日志。"""
        self._finalize_session(reason)

    def _save_step_record(self, record: dict = None):
        """保存步骤记录"""
        if record:
            self._step_records.append(record)
            self._session_data["total_steps"] = len(self._step_records)
            # 每条记录都自动保存到文件
            self._write_records_to_file()

    def _write_records_to_file(self):
        """写入记录到文件"""
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                save_path = os.path.join(self.save_dir, "step_records.json")
                with open(save_path, 'w', encoding='utf-8') as f:
                    json.dump(self._step_records, f, indent=2, ensure_ascii=False)
                logger.debug(f"Step records saved: {save_path}")
            except Exception as e:
                logger.debug(f"Failed to save step records: {e}")

    def _finalize_session(self, reason: str):
        """完成会话并保存最终日志"""
        self._session_data["end_time"] = datetime.now().isoformat()
        self._session_data["end_reason"] = reason

        # 计算总时间
        if self._start_time:
            end_dt = datetime.fromisoformat(self._session_data["end_time"])
            total_duration = (end_dt - self._start_time).total_seconds()
            self._session_data["total_duration_seconds"] = total_duration
            self._session_data["total_duration_formatted"] = self._format_duration(total_duration)

        # 保存完整日志
        self._save_complete_log(reason)

    def _format_duration(self, seconds: float) -> str:
        """格式化时间"""
        hours, remainder = divmod(int(seconds), 3600)
        minutes, secs = divmod(remainder, 60)
        return f"{hours:02d}:{minutes:02d}:{secs:02d}"

    def _save_complete_log(self, reason: str):
        """保存完整日志"""
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)

                # 保存详细日志
                log_data = {
                    "session": self._session_data,
                    "steps": self._step_records,
                    "summary": self._generate_summary()
                }

                # 主日志文件
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                main_log_path = os.path.join(self.save_dir, f"session_log_{timestamp}.json")
                with open(main_log_path, 'w', encoding='utf-8') as f:
                    json.dump(log_data, f, indent=2, ensure_ascii=False)
                logger.info(f"Session log saved: {main_log_path}")

                # 同时更新 step_records.json (保持向后兼容)
                step_records_path = os.path.join(self.save_dir, "step_records.json")
                with open(step_records_path, 'w', encoding='utf-8') as f:
                    json.dump(self._step_records, f, indent=2, ensure_ascii=False)

                # 添加日志条目
                self._add_log_entry(f"日志已保存：{main_log_path}", "success")

            except Exception as e:
                logger.debug(f"Failed to save complete log: {e}")

    def _generate_summary(self) -> dict:
        """生成执行摘要"""
        if not self._step_records:
            return {
                "total_steps": 0,
                "total_time": self._session_data.get("total_duration_formatted", "00:00:00"),
                "avg_step_time": "0.0s",
                "status": self._session_data.get("end_reason", "unknown")
            }

        # 计算总步骤时间（使用中文键名"计时"和"步骤总计"）
        total_step_time = sum(
            r.get("计时", {}).get("步骤总计", 0) for r in self._step_records
        )
        avg_step_time = total_step_time / len(self._step_records) if self._step_records else 0

        # 统计各阶段平均时间（使用中文键名）
        stage_totals = {}
        stage_counts = {}
        total_actions = 0

        for record in self._step_records:
            # 使用中文键名"计时"
            details = record.get("计时", {})
            for stage, time_val in details.items():
                # 跳过嵌套的系统开销字典和步骤总计
                if stage in ["步骤总计", "系统开销"]:
                    continue
                if isinstance(time_val, (int, float)):
                    stage_totals[stage] = stage_totals.get(stage, 0) + time_val
                    stage_counts[stage] = stage_counts.get(stage, 0) + 1

            # 统计动作数量（使用中文键名"动作"）
            actions = record.get("动作", [])
            total_actions += len(actions)

        stage_averages = {}
        for stage in stage_totals:
            if stage_counts[stage] > 0:
                stage_averages[stage] = stage_totals[stage] / stage_counts[stage]

        # 验证时间总和：所有阶段（不含 step_total）的平均值之和应等于 avg_step_time
        verified_total = sum(stage_averages.values())

        return {
            "total_steps": len(self._step_records),
            "total_time": self._session_data.get("total_duration_formatted", "00:00:00"),
            "total_duration_seconds": self._session_data.get("total_duration_seconds", 0),
            "avg_step_time": f"{avg_step_time:.2f}s",
            "stage_averages": {k: f"{v:.2f}s" for k, v in stage_averages.items()},
            "verified_total_avg": f"{verified_total:.2f}s",
            "time_verification": "OK" if abs(verified_total - avg_step_time) < 0.1 else f"MISMATCH (diff: {abs(verified_total - avg_step_time):.2f}s)",
            "total_actions": total_actions,
            "avg_actions_per_step": f"{total_actions / len(self._step_records):.1f}" if self._step_records else "0",
            "status": self._session_data.get("end_reason", "unknown"),
            "task": self._session_data.get("task", "")[:200]
        }

    def _on_window_close(self):
        """处理窗口关闭事件"""
        logger.info("Status window closed, requesting stop")
        self._stop_requested = True
        self._finalize_session("user_closed_window")
        if self.on_close:
            try:
                self.on_close()
            except Exception as e:
                logger.exception(f"Close callback error: {e}")

    def _on_stop_button_click(self):
        """处理停止按钮点击"""
        logger.info("User clicked stop button")
        self._on_window_close()

    # ========== 公共方法（线程安全） ==========

    def set_task(self, task: str):
        """设置任务描述"""
        self._message_queue.put({
            "type": "set_task",
            "task": task
        })

    def update_step(self, step_number: int, goal: str, status: str = "running"):
        """更新当前步骤信息"""
        self._message_queue.put({
            "type": "update_step",
            "step_number": step_number,
            "goal": goal,
            "status": status
        })

    def add_log(self, message: str, level: str = "info"):
        """添加日志条目"""
        self._message_queue.put({
            "type": "add_log",
            "message": message,
            "level": level
        })

    def add_detail(self, message: str):
        """添加详情条目"""
        self._message_queue.put({
            "type": "add_detail",
            "message": message
        })

    def save_record(self, record: dict):
        """保存步骤记录"""
        self._message_queue.put({
            "type": "save_record",
            "record": record
        })

    def finalize_session(self, reason: str):
        """完成会话并保存日志"""
        self._message_queue.put({
            "type": "finalize_session",
            "reason": reason
        })

    def is_stop_requested(self) -> bool:
        """检查是否请求停止"""
        return self._stop_requested

    def close(self):
        """关闭窗口"""
        if self._window:
            try:
                self._window.after(0, self._window.quit)
                self._window.after(0, self._window.destroy)
            except RuntimeError:
                logger.debug("Tk status window close requested outside main loop")
            except Exception as e:
                logger.debug("Failed to close Tk status window: %s", e)

    def bring_to_front(self):
        if not self._window:
            return
        try:
            self._window.after(0, self._window.lift)
            self._window.after(0, lambda: self._window.attributes("-topmost", True))
        except Exception as e:
            logger.debug("Failed to bring Tk status window to front: %s", e)


class _QtSignalBridge(QtCore.QObject if QtCore else object):
    if _QT_SIGNAL:
        set_task_signal = _QT_SIGNAL(str)
        update_step_signal = _QT_SIGNAL(int, str, str)
        add_log_signal = _QT_SIGNAL(str, str)
        add_detail_signal = _QT_SIGNAL(str)
        save_record_signal = _QT_SIGNAL(dict)
        finalize_signal = _QT_SIGNAL(str)
        close_signal = _QT_SIGNAL()
        render_signal = _QT_SIGNAL(dict)


class _QtStatusWidget(QtWidgets.QWidget if QtWidgets else object):
    def __init__(self, owner):
        super().__init__()
        self._owner = owner

    def closeEvent(self, event):
        self._owner._on_window_close()
        event.accept()


class _StatusWebPage(QtWebEngineCore.QWebEnginePage if QtWebEngineCore else object):
    def __init__(self, owner):
        super().__init__()
        self._owner = owner

    def acceptNavigationRequest(self, url, nav_type, is_main_frame):
        url_text = url.toString()
        if url_text.startswith("kylin://stop") or url_text.startswith("https://kylin.local/stop"):
            self._owner._on_stop_button_click()
            return False
        return super().acceptNavigationRequest(url, nav_type, is_main_frame)


class _QtStopBridge(QtCore.QObject if QtCore else object):
    def __init__(self, owner):
        super().__init__()
        self._owner = owner

    if _QT_SLOT:
        @_QT_SLOT()
        def stop(self):
            self._owner._on_stop_button_click()


class QtStatusWindow:
    """Qt 状态窗口，优先用于更稳定的中文字体渲染。"""

    def __init__(self, title: str = "任务监视器", on_close: Optional[Callable[[], None]] = None,
                 save_dir: Optional[str] = None):
        if not QtWidgets or not QtCore or not QtGui:
            raise RuntimeError("Qt binding is not available")
        if QtWebEngineWidgets is None:
            raise RuntimeError("Qt WebEngine is not available")

        self.title = title
        self.on_close = on_close
        self.save_dir = save_dir
        self._stop_requested = False
        self._window_ready = threading.Event()
        self._window = None
        self._app = None
        self._ui_scale = 1.0
        self._bridge = None
        self._web_view = None
        self._page_ready = False
        self._state = {
            "title": title,
            "goal": "等待任务...",
            "goal_html": _rich_text_html("等待任务..."),
            "operation": "等待开始...",
            "operation_html": _rich_text_html("等待开始..."),
            "status": "就绪",
            "status_html": _rich_text_html("就绪"),
            "status_key": "pending",
        }
        self._operation_lines: list[str] = []
        self._status_label = None
        self._goal_text = None
        self._operation_text = None
        self._timer = None
        self._start_time: Optional[datetime] = None
        self._step_records = []
        self._session_data = {
            "start_time": None,
            "end_time": None,
            "end_reason": None,
            "total_steps": 0,
            "task": ""
        }

        self._gui_thread = threading.Thread(target=self._run_gui, daemon=True, name="StatusWindowQtGUI")
        self._gui_thread.start()
        self._window_ready.wait(timeout=5.0)
        if not self._window_ready.is_set():
            logger.warning("Qt status window timeout")

    @staticmethod
    def _ui_font(point_size: int = 10, scale: float = 1.0):
        family = _status_font_env() or get_system_font(point_size)[0]
        font = QtGui.QFont(family, max(1, int(round(point_size * scale))))
        font.setStyleStrategy(QtGui.QFont.PreferAntialias)
        font.setWeight(QtGui.QFont.Normal)
        return font

    @staticmethod
    def _text_css(point_size: int = 10, scale: float = 1.0) -> str:
        return _font_stack_css(max(1, int(round(point_size * scale))))

    @staticmethod
    def _web_html(scale: float = 1.0) -> str:
        # QtWebEngine setHtml() blocks file:// font URLs in some environments.
        # Use installed font family names directly so Chromium can pick the
        # clearest native CJK font without a blocked web-font fallback.
        font_faces = ""
        html = """<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <style>
    __FONT_FACES__
    :root {
      --bg: #f7f8fa;
      --panel: #ffffff;
      --line: #dde4ee;
      --text: #1f2933;
      --muted: #647184;
      --primary: #1f5f99;
      --err: #c53030;
    }
    * { box-sizing: border-box; }
    html, body {
      margin: 0;
      width: 100%;
      height: 100%;
      overflow: hidden;
      background: var(--bg);
      color: var(--text);
      __FONT_STACK__
      font-size: __BASE_FONT_PX__px;
      line-height: 1.45;
      text-rendering: optimizeLegibility;
      -webkit-font-smoothing: antialiased;
    }
    .font-primary {
      font-family: "Noto Sans CJK SC", "Source Han Sans SC", "KylinPrimaryCJK",
        "STHeiti", "华文黑体", sans-serif;
    }
    .font-extended {
      font-family: "KylinExtendedCJK", "GB_SS_GB18030_Extended",
        "Noto Sans CJK SC", "Source Han Sans SC", sans-serif;
    }
    .font-fallback {
      font-family: "Noto Sans CJK SC", "Source Han Sans SC", "KylinExtendedCJK",
        "KylinPrimaryCJK", "GB_SS_GB18030_Extended", sans-serif;
    }
    .wrap {
      width: 100%;
      height: 100%;
      padding: 10px;
      display: grid;
      grid-template-rows: 0.62fr 1.38fr auto;
      gap: 7px;
    }
    .label {
      color: var(--muted);
      font-size: __LABEL_FONT_PX__px;
      line-height: __LABEL_LINE_PX__px;
      margin-bottom: 3px;
    }
    .box {
      min-height: 0;
      display: flex;
      flex-direction: column;
    }
    .content {
      flex: 1;
      overflow: auto;
      white-space: pre-wrap;
      overflow-wrap: anywhere;
      background: var(--panel);
      border: 1px solid var(--line);
      padding: 7px 9px;
    }
    #status {
      color: var(--primary);
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
      min-height: __STATUS_MIN_PX__px;
    }
  </style>
</head>
<body>
  <div class="wrap">
    <section class="box"><div class="label">目标</div><div id="goal" class="content">等待任务...</div></section>
    <section class="box"><div class="label">当前操作</div><div id="operation" class="content">等待开始...</div></section>
    <div id="status">就绪</div>
  </div>
  <script>
    window.stopRequested = false;
    function setHtml(id, html) {
      const el = document.getElementById(id);
      if (el) {
        el.innerHTML = html || '';
        if (id === 'operation') {
          requestAnimationFrame(() => { el.scrollTop = el.scrollHeight; });
        }
      }
    }
    window.updateStatus = function(state) {
      setHtml('goal', state.goal_html || '');
      setHtml('operation', state.operation_html || '');
      setHtml('status', state.status_html || '');
    };
  </script>
</body>
</html>"""
        return (
            html.replace("__FONT_FACES__", font_faces)
            .replace("__FONT_STACK__", _font_stack_css(max(1, int(round(10 * scale)))))
            .replace("__BASE_FONT_PX__", str(_scaled(14, scale)))
            .replace("__LABEL_FONT_PX__", str(_scaled(12, scale)))
            .replace("__LABEL_LINE_PX__", str(_scaled(18, scale)))
            .replace("__STATUS_MIN_PX__", str(_scaled(20, scale)))
        )

    def _run_gui(self):
        try:
            _configure_qt_high_dpi()

            self._app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
            self._ui_scale = _qt_screen_scale(self._app)
            self._app.setFont(self._ui_font(10, self._ui_scale))
            self._bridge = _QtSignalBridge()
            self._build_ui()
            self._connect_signals()

            self._start_time = datetime.now()
            self._session_data["start_time"] = self._start_time.isoformat()

            self._window_ready.set()
            if hasattr(self._app, "exec"):
                self._app.exec()
            else:
                self._app.exec_()
        except Exception as e:
            logger.exception(f"Qt status window error: {e}")
            self._window_ready.set()

    def _build_ui(self):
        if QtWebEngineWidgets is not None:
            self._build_webengine_ui()
            return

        page_bg = "#f7f8fa"
        panel_bg = "#ffffff"
        border = "#dfe5ec"
        text_color = "#1f2933"
        primary = "#1f5f99"

        self._window = _QtStatusWidget(self)
        self._window.setWindowTitle("任务监视器")
        _set_always_on_top(self._window)
        self._window.setMinimumSize(_scaled(360, self._ui_scale), _scaled(220, self._ui_scale))
        self._window.resize(_scaled(420, self._ui_scale), _scaled(260, self._ui_scale))
        self._window.setStyleSheet(f"""
            QWidget {{
                background: {page_bg};
                color: {text_color};
                {self._text_css(10, self._ui_scale)}
            }}
            QLabel {{
                background: transparent;
            }}
            QTextEdit {{
                background: {panel_bg};
                color: {text_color};
                border: 1px solid {border};
                padding: 8px 10px;
                line-height: 1.4;
                selection-background-color: #cfe3f7;
                {self._text_css(10, self._ui_scale)}
            }}
            QPushButton#stopButton {{
                color: white;
                border: 0;
                padding: 5px 12px;
                background: #c53030;
                {self._text_css(10, self._ui_scale)}
            }}
        """)

        layout = QtWidgets.QVBoxLayout(self._window)
        layout.setContentsMargins(*([_scaled(10, self._ui_scale)] * 4))
        layout.setSpacing(_scaled(6, self._ui_scale))

        title = QtWidgets.QLabel("任务监视器")
        title.setFont(self._ui_font(11, self._ui_scale))
        title.setStyleSheet(f"color: {text_color};")
        layout.addWidget(title)

        self._goal_text = self._add_text_section(layout, "目标", _scaled(44, self._ui_scale), 10)
        self._goal_text.setPlainText("等待任务...")
        self._operation_text = self._add_text_section(layout, "当前操作", _scaled(98, self._ui_scale), 10)
        self._operation_text.setPlainText("等待开始...")

        bottom = QtWidgets.QHBoxLayout()
        self._status_label = QtWidgets.QLabel("就绪")
        self._status_label.setStyleSheet(f"color: {primary};")
        bottom.addWidget(self._status_label)
        bottom.addStretch(1)
        stop_button = QtWidgets.QPushButton("停止")
        stop_button.setObjectName("stopButton")
        stop_button.clicked.connect(self._on_stop_button_click)
        bottom.addWidget(stop_button)
        layout.addLayout(bottom)

        self._window.show()
        _schedule_qt_window_bottom_right(self._app, self._window)

    def _build_webengine_ui(self):
        self._window = _QtStatusWidget(self)
        self._window.setWindowTitle("任务监视器")
        _set_always_on_top(self._window)
        self._window.setMinimumSize(_scaled(360, self._ui_scale), _scaled(220, self._ui_scale))
        self._window.resize(_scaled(420, self._ui_scale), _scaled(260, self._ui_scale))

        layout = QtWidgets.QVBoxLayout(self._window)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        self._web_view = QtWebEngineWidgets.QWebEngineView()
        self._web_view.setPage(_StatusWebPage(self))
        context_menu_policy = _qt_context_menu_policy("NoContextMenu")
        if context_menu_policy is not None:
            self._web_view.setContextMenuPolicy(context_menu_policy)
        self._web_view.loadFinished.connect(self._on_web_loaded)
        self._web_view.setHtml(self._web_html(self._ui_scale))
        layout.addWidget(self._web_view)

        self._window.show()
        _schedule_qt_window_bottom_right(self._app, self._window)

    def _on_web_loaded(self, ok: bool):
        self._page_ready = bool(ok)
        self._render_web_state()

    def _render_web_state(self):
        if not self._page_ready or not self._web_view:
            return
        payload = json.dumps(self._state, ensure_ascii=False)
        self._web_view.page().runJavaScript(f"window.updateStatus({payload});")

    def _add_text_section(self, layout, title: str, height: int, point_size: int):
        label = QtWidgets.QLabel(title)
        label.setFont(self._ui_font(9, self._ui_scale))
        label.setStyleSheet("color: #657080;")
        layout.addWidget(label)
        text = QtWidgets.QTextEdit()
        text.setReadOnly(True)
        text.setAcceptRichText(False)
        text.setMinimumHeight(height)
        text.setFont(self._ui_font(point_size, self._ui_scale))
        layout.addWidget(text)
        return text

    def _connect_signals(self):
        self._bridge.set_task_signal.connect(self._handle_set_task)
        self._bridge.update_step_signal.connect(self._update_step_display)
        self._bridge.add_log_signal.connect(self._add_log_entry)
        self._bridge.add_detail_signal.connect(self._add_detail_entry)
        self._bridge.save_record_signal.connect(self._save_step_record)
        self._bridge.finalize_signal.connect(self._finalize_session)
        self._bridge.close_signal.connect(self._close_in_gui)
        self._bridge.render_signal.connect(self._render_web_payload)

    def _render_web_payload(self, state: dict):
        self._state.update(state)
        self._render_web_state()

    def _handle_set_task(self, task: str):
        self._session_data["task"] = task

    def _update_step_display(self, step_number: int, goal: str, status: str):
        status_config = {
            "pending": ("等待中", "#6c757d"),
            "running": ("运行中", "#1f5f99"),
            "success": ("已完成", "#22863a"),
            "failed": ("失败", "#b7791f"),
            "error": ("错误", "#c53030"),
            "warning": ("警告", "#b7791f"),
        }
        text, color = status_config.get(status, (StatusWindow._localize_display_text(status), "#333333"))
        goal_text = StatusWindow._localize_display_text(goal) if goal else "等待任务..."
        status_text = f"步骤 {step_number} · {text}"
        if self._web_view:
            self._bridge.render_signal.emit({
                "goal": goal_text,
                "goal_html": _rich_text_html(goal_text),
                "status": status_text,
                "status_html": _rich_text_html(status_text),
                "status_key": status,
            })
        else:
            self._status_label.setText(status_text)
            self._status_label.setStyleSheet(f"color: {color};")
            self._goal_text.setPlainText(goal_text)

    def _append_text(self, widget, text: str):
        widget.moveCursor(QtGui.QTextCursor.End)
        widget.insertPlainText(text)
        widget.moveCursor(QtGui.QTextCursor.End)

    def _set_operation_message(self, message: str):
        operation = StatusWindow._localize_display_text(message)
        if not operation:
            return
        self._operation_lines.append(operation)
        self._operation_lines = self._operation_lines[-40:]
        text = "\n".join(self._operation_lines) if self._operation_lines else "等待开始..."
        if self._web_view:
            self._bridge.render_signal.emit({
                "operation": text,
                "operation_html": _rich_text_html(text),
            })
            return
        if self._operation_text:
            self._operation_text.setPlainText(text)

    def _add_detail_entry(self, message: str):
        if not StatusWindow._is_primary_operation_message(message, "detail"):
            return
        self._set_operation_message(message)

    def _add_log_entry(self, message: str, level: str = "info"):
        if not StatusWindow._is_primary_operation_message(message, level):
            return
        self._set_operation_message(message)

    def _save_current_log(self):
        self._save_all_records("manual_save")

    def _save_all_records(self, reason: str):
        self._finalize_session(reason)

    def _save_step_record(self, record: dict = None):
        if record:
            self._step_records.append(record)
            self._session_data["total_steps"] = len(self._step_records)
            self._write_records_to_file()

    def _write_records_to_file(self):
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                save_path = os.path.join(self.save_dir, "step_records.json")
                with open(save_path, 'w', encoding='utf-8') as f:
                    json.dump(self._step_records, f, indent=2, ensure_ascii=False)
            except Exception as e:
                logger.debug(f"Failed to save step records: {e}")

    def _finalize_session(self, reason: str):
        self._session_data["end_time"] = datetime.now().isoformat()
        self._session_data["end_reason"] = reason
        self._save_complete_log(reason)

    def _format_duration(self, seconds: float) -> str:
        hours, remainder = divmod(int(seconds), 3600)
        minutes, secs = divmod(remainder, 60)
        return f"{hours:02d}:{minutes:02d}:{secs:02d}"

    def _save_complete_log(self, reason: str):
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                log_data = {
                    "session": self._session_data,
                    "steps": self._step_records,
                    "summary": {},
                }
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                main_log_path = os.path.join(self.save_dir, f"session_log_{timestamp}.json")
                with open(main_log_path, 'w', encoding='utf-8') as f:
                    json.dump(log_data, f, indent=2, ensure_ascii=False)
                self._add_log_entry(f"日志已保存：{main_log_path}", "success")
            except Exception as e:
                logger.debug(f"Failed to save complete log: {e}")

    def _on_window_close(self):
        self._stop_requested = True
        if self._timer:
            self._timer.stop()
        if self.on_close:
            try:
                self.on_close()
            except Exception as e:
                logger.exception(f"Close callback error: {e}")

    def _on_stop_button_click(self):
        self._on_window_close()
        if self._window:
            self._window.close()

    def _close_in_gui(self):
        if self._window:
            self._window.close()
        if self._app:
            self._app.quit()

    def set_task(self, task: str):
        self._bridge.set_task_signal.emit(task)

    def update_step(self, step_number: int, goal: str, status: str = "running"):
        self._bridge.update_step_signal.emit(step_number, goal, status)

    def add_log(self, message: str, level: str = "info"):
        self._bridge.add_log_signal.emit(message, level)

    def add_detail(self, message: str):
        self._bridge.add_detail_signal.emit(message)

    def save_record(self, record: dict):
        self._bridge.save_record_signal.emit(record)

    def finalize_session(self, reason: str):
        self._bridge.finalize_signal.emit(reason)

    def is_stop_requested(self) -> bool:
        return self._stop_requested

    def close(self):
        if self._bridge:
            self._bridge.close_signal.emit()

    def bring_to_front(self):
        if self._app and self._window:
            try:
                QtCore.QTimer.singleShot(
                    0,
                    lambda app=self._app, window=self._window: _place_qt_window_bottom_right(app, window),
                )
            except Exception as e:
                logger.debug("Failed to bring Qt status window to front: %s", e)


def _run_qt_webengine_status_process(command_queue, event_queue, title: str):
    """Run Qt WebEngine in a child process main thread."""
    _redirect_process_stdio_to_devnull()
    try:
        if not QtWidgets or not QtCore or not QtWebEngineWidgets:
            event_queue.put({"type": "error", "message": "Qt WebEngine is not available"})
            return

        _configure_qt_high_dpi()

        app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
        ui_scale = _qt_screen_scale(app)
        app.setFont(QtStatusWindow._ui_font(10, ui_scale))

        class Controller:
            def __init__(self):
                self.page_ready = False
                self.web_view = None
                self.window = None
                self.closing = False
                self.state = {
                    "title": title,
                    "goal_html": _rich_text_html("等待任务..."),
                    "operation_html": _rich_text_html("等待开始..."),
                    "status_html": _rich_text_html("就绪"),
                }

            def _on_stop_button_click(self):
                if self.closing:
                    return
                self.closing = True
                self.state["status_html"] = _rich_text_html("已停止")
                self.render()
                event_queue.put({"type": "stop"})
                if self.window:
                    self.window.hide()
                QtCore.QTimer.singleShot(0, app.quit)

            def _on_window_close(self):
                self._on_stop_button_click()

            def render(self):
                if not self.page_ready or not self.web_view:
                    return
                payload = json.dumps(self.state, ensure_ascii=False)
                self.web_view.page().runJavaScript(f"window.updateStatus({payload});")

        controller = Controller()
        window = _QtStatusWidget(controller)
        controller.window = window
        window.setWindowTitle("任务监视器")
        _set_always_on_top(window)
        window.setMinimumSize(_scaled(360, ui_scale), _scaled(250, ui_scale))
        window.resize(_scaled(420, ui_scale), _scaled(292, ui_scale))

        layout = QtWidgets.QVBoxLayout(window)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        web_view = QtWebEngineWidgets.QWebEngineView()
        controller.web_view = web_view
        page = _StatusWebPage(controller)
        if QtWebChannel is not None:
            channel = QtWebChannel.QWebChannel(page)
            bridge = _QtStopBridge(controller)
            channel.registerObject("kylinBridge", bridge)
            page.setWebChannel(channel)
            controller._web_channel = channel
            controller._stop_bridge = bridge
        web_view.setPage(page)
        context_menu_policy = _qt_context_menu_policy("NoContextMenu")
        if context_menu_policy is not None:
            web_view.setContextMenuPolicy(context_menu_policy)
        web_view.loadFinished.connect(lambda ok: (setattr(controller, "page_ready", bool(ok)), controller.render()))
        web_view.setHtml(QtStatusWindow._web_html(ui_scale))
        web_view.setMinimumHeight(_scaled(220, ui_scale))
        layout.addWidget(web_view)

        footer = QtWidgets.QFrame(window)
        footer.setMinimumHeight(_scaled(34, ui_scale))
        footer.setStyleSheet("QFrame { background: #f7f8fa; }")
        footer_layout = QtWidgets.QHBoxLayout(footer)
        footer_layout.setContentsMargins(_scaled(10, ui_scale), _scaled(4, ui_scale), _scaled(10, ui_scale), _scaled(4, ui_scale))
        footer_layout.addStretch(1)

        stop_button = QtWidgets.QPushButton("停止", footer)
        cursor_shape = _qt_cursor_shape("PointingHandCursor")
        if cursor_shape is not None:
            stop_button.setCursor(cursor_shape)
        stop_button.setMinimumSize(_scaled(52, ui_scale), _scaled(28, ui_scale))
        stop_button.setStyleSheet(
            "QPushButton { background: #c53030; color: white; border: 0; "
            f"{_font_stack_css(_scaled(10, ui_scale))} padding: {_scaled(4, ui_scale)}px {_scaled(12, ui_scale)}px; }}"
            "QPushButton:pressed { background: #9b2c2c; }"
        )
        stop_button.clicked.connect(controller._on_stop_button_click)
        footer_layout.addWidget(stop_button)
        layout.addWidget(footer)
        controller.stop_button = stop_button

        window.show()
        _schedule_qt_window_bottom_right(app, window)
        event_queue.put({"type": "ready"})

        def pump_commands():
            try:
                while True:
                    msg = command_queue.get_nowait()
                    msg_type = msg.get("type")
                    if msg_type == "render":
                        controller.state.update(msg.get("state", {}))
                        controller.render()
                    elif msg_type == "close":
                        controller.closing = True
                        app.quit()
                        return
            except queue.Empty:
                pass
            QtCore.QTimer.singleShot(50, pump_commands)

        QtCore.QTimer.singleShot(50, pump_commands)
        if hasattr(app, "exec"):
            app.exec()
        else:
            app.exec_()
    except Exception as e:
        try:
            event_queue.put({"type": "error", "message": str(e)})
        except Exception:
            pass


class QtWebEngineProcessStatusWindow:
    """Qt WebEngine status window hosted in a child process main thread."""

    def __init__(self, title: str = "任务监视器", on_close: Optional[Callable[[], None]] = None,
                 save_dir: Optional[str] = None):
        if not QtWidgets or not QtCore or not QtWebEngineWidgets:
            raise RuntimeError("Qt WebEngine is not available")

        self.title = title
        self.on_close = on_close
        self.save_dir = save_dir
        self._stop_requested = False
        self._closing = False
        self._start_time = datetime.now()
        self._step_records = []
        self._session_data = {
            "start_time": self._start_time.isoformat(),
            "end_time": None,
            "end_reason": None,
            "total_steps": 0,
            "task": "",
        }
        self._state = {
            "goal_html": _rich_text_html("等待任务..."),
            "operation_html": _rich_text_html("等待开始..."),
            "status_html": _rich_text_html("就绪"),
        }
        self._operation_lines: list[str] = []

        ctx = multiprocessing.get_context("fork" if platform.system() != "Windows" else "spawn")
        self._command_queue = ctx.Queue()
        self._event_queue = ctx.Queue()
        self._process = ctx.Process(
            target=_run_qt_webengine_status_process,
            args=(self._command_queue, self._event_queue, title),
            daemon=True,
        )
        self._process.start()
        self._wait_until_ready()
        self._event_thread = threading.Thread(
            target=self._event_loop,
            daemon=True,
            name="QtStatusWindowEvents",
        )
        self._event_thread.start()

    def _wait_until_ready(self):
        deadline = time.time() + 5
        saw_process_alive = False
        while time.time() < deadline:
            if self._poll_events():
                return
            if self._process and self._process.is_alive():
                saw_process_alive = True
            elif saw_process_alive:
                break
            time.sleep(0.05)
        if self._process and not self._process.is_alive():
            raise RuntimeError("Qt WebEngine status process exited")
        if saw_process_alive:
            logger.warning("Qt WebEngine status process is alive but did not report ready")

    def _poll_events(self):
        handled_ready = False
        try:
            while True:
                event = self._event_queue.get_nowait()
                if event.get("type") in {"ready", "error"}:
                    handled_ready = True
                self._handle_event(event)
        except queue.Empty:
            return handled_ready

    def _event_loop(self):
        while self._process and self._process.is_alive() and not self._stop_requested:
            try:
                event = self._event_queue.get(timeout=0.2)
            except queue.Empty:
                continue
            self._handle_event(event)
        self._poll_events()
        if self._process and not self._process.is_alive() and not self._closing and not self._stop_requested:
            self._handle_event({"type": "stop"})

    def _handle_event(self, event: dict):
        if event.get("type") == "stop":
            if self._stop_requested:
                return
            self._stop_requested = True
            if self.on_close:
                try:
                    self.on_close()
                except Exception as e:
                    logger.exception(f"Close callback error: {e}")
        elif event.get("type") == "error":
            logger.warning("Qt WebEngine status process error: %s", event.get("message"))
        elif event.get("type") == "ready":
            logger.info("Qt WebEngine status window is visible")

    def _render(self, state: dict):
        self._state.update(state)
        if self._process and self._process.is_alive():
            self._command_queue.put({"type": "render", "state": state})

    def _render_operation(self):
        text = "\n".join(self._operation_lines[-120:]) if self._operation_lines else "等待开始..."
        self._render({"operation_html": _rich_text_html(text)})

    def _append_operation(self, message: str):
        operation = StatusWindow._localize_display_text(message)
        if not operation or operation == "---":
            return
        if operation.startswith("->"):
            operation = operation
        elif operation.startswith("  ->"):
            operation = operation.strip()
        self._operation_lines.append(operation)
        self._operation_lines = self._operation_lines[-120:]
        self._render_operation()

    def set_task(self, task: str):
        self._session_data["task"] = task

    def update_step(self, step_number: int, goal: str, status: str = "running"):
        status_config = {
            "pending": "等待中",
            "running": "运行中",
            "success": "已完成",
            "failed": "失败",
            "error": "错误",
            "warning": "警告",
        }
        status_text = f"步骤 {step_number} · {status_config.get(status, StatusWindow._localize_display_text(status))}"
        goal_text = StatusWindow._localize_display_text(goal) if goal else "等待任务..."
        self._render({
            "goal_html": _rich_text_html(goal_text),
            "status_html": _rich_text_html(status_text),
        })

    def add_log(self, message: str, level: str = "info"):
        if StatusWindow._is_primary_operation_message(message, level):
            self._append_operation(message)

    def add_detail(self, message: str):
        if StatusWindow._is_primary_operation_message(message, "detail"):
            self._append_operation(message)

    def save_record(self, record: dict):
        if record:
            self._step_records.append(record)
            self._session_data["total_steps"] = len(self._step_records)
            self._write_records_to_file()

    def _write_records_to_file(self):
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                save_path = os.path.join(self.save_dir, "step_records.json")
                with open(save_path, "w", encoding="utf-8") as f:
                    json.dump(self._step_records, f, indent=2, ensure_ascii=False)
            except Exception as e:
                logger.debug(f"Failed to save step records: {e}")

    def finalize_session(self, reason: str):
        self._session_data["end_time"] = datetime.now().isoformat()
        self._session_data["end_reason"] = reason
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                main_log_path = os.path.join(self.save_dir, f"session_log_{timestamp}.json")
                with open(main_log_path, "w", encoding="utf-8") as f:
                    json.dump({"session": self._session_data, "steps": self._step_records, "summary": {}}, f, indent=2, ensure_ascii=False)
            except Exception as e:
                logger.debug(f"Failed to save complete log: {e}")

    def is_stop_requested(self) -> bool:
        self._poll_events()
        if self._process and not self._process.is_alive() and not self._closing:
            self._handle_event({"type": "stop"})
        return self._stop_requested

    def close(self):
        self._closing = True
        process = self._process
        if process and process.is_alive():
            try:
                self._command_queue.put({"type": "close"})
                process.join(timeout=1.0)
            except Exception:
                pass
            if process.is_alive():
                try:
                    process.terminate()
                    process.join(timeout=1.0)
                except Exception:
                    pass
            if process.is_alive():
                try:
                    process.kill()
                    process.join(timeout=0.5)
                except Exception:
                    pass
        self._process = None

    def bring_to_front(self):
        if self._process and self._process.is_alive():
            try:
                self._command_queue.put({"type": "raise"})
            except Exception as e:
                logger.debug("Failed to bring Qt WebEngine status window to front: %s", e)


class BrowserStatusWindow:
    """浏览器/HTML 状态窗口，使用浏览器字体渲染避免 Tk/Qt 中文显示问题。"""

    def __init__(self, title: str = "任务监视器", on_close: Optional[Callable[[], None]] = None,
                 save_dir: Optional[str] = None):
        self.title = title
        self.on_close = on_close
        self.save_dir = save_dir
        self._stop_requested = False
        self._lock = threading.RLock()
        self._server = None
        self._server_thread = None
        self._browser_process = None
        self._browser_profile_dir = None
        self._keep_visible_stop = threading.Event()
        self._keep_visible_thread = None
        self._start_time = datetime.now()
        screen_size = self._screen_size() or (0, 0)
        self._ui_scale = _status_ui_scale(screen_size[0], screen_size[1])
        self._step_records = []
        self._state = {
            "title": title,
            "task": "",
            "step": 0,
            "status": "就绪",
            "status_key": "pending",
            "goal": "等待任务...",
            "details": [],
            "logs": [],
            "elapsed": "00:00:00",
            "stopped": False,
        }
        self._session_data = {
            "start_time": self._start_time.isoformat(),
            "end_time": None,
            "end_reason": None,
            "total_steps": 0,
            "task": "",
        }
        self._start_server()

    def _html(self) -> str:
        scale = getattr(self, "_ui_scale", 1.0)
        return """<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>任务监视器 · Browser</title>
  <style>
    :root {
      color-scheme: light;
      --bg: #f6f8fb;
      --panel: #ffffff;
      --line: #dde4ee;
      --text: #1f2933;
      --muted: #647184;
      --primary: #1f5f99;
      --ok: #1f7a3f;
      --warn: #a66a00;
      --err: #b42318;
    }
    * { box-sizing: border-box; }
    html, body {
      margin: 0;
      width: 100%;
      height: 100%;
      overflow: hidden;
      background: var(--bg);
      color: var(--text);
      __FONT_STACK__
      font-size: __BASE_FONT_PX__px;
      line-height: 1.45;
      text-rendering: optimizeLegibility;
      -webkit-font-smoothing: antialiased;
    }
    body { padding: 0; }
    .app {
      width: 100%;
      height: 100%;
      padding: 10px;
      display: grid;
      grid-template-rows: 24px 0.7fr 1.3fr 30px;
      gap: 7px;
    }
    .title {
      font-size: __TITLE_FONT_PX__px;
      line-height: __TITLE_LINE_PX__px;
      margin: 0;
      font-weight: 500;
      letter-spacing: 0;
      white-space: nowrap;
    }
    .status {
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .status .spacer { flex: 1; }
    .muted { color: var(--muted); }
    .step { color: var(--primary); }
    .badge { color: var(--primary); }
    .badge.success { color: var(--ok); }
    .badge.warning, .badge.failed { color: var(--warn); }
    .badge.error { color: var(--err); }
    .section {
      min-height: 0;
      display: flex;
      flex-direction: column;
    }
    .section-title {
      color: var(--muted);
      font-size: __LABEL_FONT_PX__px;
      line-height: __LABEL_LINE_PX__px;
      margin-bottom: 3px;
    }
    .box {
      flex: 1;
      min-height: 0;
      background: var(--panel);
      border: 1px solid var(--line);
      padding: 7px 9px;
      overflow: auto;
      white-space: pre-wrap;
      overflow-wrap: anywhere;
    }
    .bottom {
      display: flex;
      align-items: center;
      gap: 8px;
      min-width: 0;
    }
    .bottom .spacer { flex: 1; }
    #footerStatus {
      min-width: 0;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
    button {
      border: 0;
      color: #fff;
      background: #5d6b7a;
      padding: 5px 12px;
      font: inherit;
      cursor: pointer;
    }
    button.stop { background: #c53030; }
  </style>
</head>
<body>
  <main class="app">
    <h1 class="title">任务监视器 · Browser</h1>
    <section class="section"><div class="section-title">当前目标</div><div id="goal" class="box">等待任务...</div></section>
    <section class="section"><div class="section-title">步骤详情</div><div id="operation" class="box">等待开始...</div></section>
    <div class="bottom">
      <span class="muted">步骤：</span><span id="step" class="step">0</span>
      <span id="footerStatus" class="badge">就绪</span>
      <span class="spacer"></span>
      <button class="stop" onclick="fetch('/stop', {method:'POST'})">停止</button>
    </div>
  </main>
  <script>
    const byId = (id) => document.getElementById(id);
    function setText(id, value) { byId(id).textContent = value || ""; }
    function setScrollableText(id, value) {
      const el = byId(id);
      el.textContent = value || "";
      requestAnimationFrame(() => { el.scrollTop = el.scrollHeight; });
    }
    function operationText(s) {
      const details = s.details || [];
      const logs = s.logs || [];
      const lines = details.length ? details : logs;
      return lines.join('\\n') || '等待开始...';
    }
    async function refresh() {
      try {
        const r = await fetch('/state', {cache: 'no-store'});
        const s = await r.json();
        document.title = '任务监视器 · Browser';
        setText('step', String(s.step ?? 0));
        setText('footerStatus', s.status || '就绪');
        byId('footerStatus').className = 'badge ' + (s.status_key || '');
        setText('goal', s.goal || '等待任务...');
        setScrollableText('operation', operationText(s));
      } catch (e) {}
    }
    refresh();
    setInterval(refresh, 500);
  </script>
</body>
</html>""".replace("__FONT_STACK__", _font_stack_css(_scaled(10, scale))).replace(
            "__BASE_FONT_PX__", str(_scaled(14, scale))
        ).replace("__TITLE_FONT_PX__", str(_scaled(16, scale))).replace(
            "__TITLE_LINE_PX__", str(_scaled(24, scale))
        ).replace("__LABEL_FONT_PX__", str(_scaled(12, scale))).replace(
            "__LABEL_LINE_PX__", str(_scaled(18, scale))
        )

    def _start_server(self):
        owner = self

        class Handler(BaseHTTPRequestHandler):
            def log_message(self, format, *args):
                return

            def _send(self, code: int, body: bytes, content_type: str):
                self.send_response(code)
                self.send_header("Content-Type", content_type)
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Cache-Control", "no-store")
                self.end_headers()
                self.wfile.write(body)

            def do_GET(self):
                if self.path == "/" or self.path.startswith("/index"):
                    self._send(200, owner._html().encode("utf-8"), "text/html; charset=utf-8")
                elif self.path.startswith("/state"):
                    self._send(200, json.dumps(owner._snapshot(), ensure_ascii=False).encode("utf-8"), "application/json; charset=utf-8")
                else:
                    self._send(404, b"Not found", "text/plain; charset=utf-8")

            def do_POST(self):
                if self.path.startswith("/stop"):
                    owner._on_stop_button_click()
                    self._send(200, b"ok", "text/plain; charset=utf-8")
                elif self.path.startswith("/save"):
                    owner._save_current_log()
                    self._send(200, b"ok", "text/plain; charset=utf-8")
                else:
                    self._send(404, b"Not found", "text/plain; charset=utf-8")

        self._server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        self._server_thread = threading.Thread(target=self._server.serve_forever, daemon=True, name="StatusWindowBrowserHTTP")
        self._server_thread.start()
        url = f"http://127.0.0.1:{self._server.server_port}/"
        logger.info("浏览器状态窗口地址：%s", url)
        threading.Thread(target=lambda: self._open_app_window(url), daemon=True).start()

    def _screen_size(self) -> Optional[tuple[int, int]]:
        try:
            result = subprocess.run(
                ["xrandr", "--current"],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
            match = re.search(r"current\s+(\d+)\s+x\s+(\d+)", result.stdout)
            if match:
                return int(match.group(1)), int(match.group(2))
        except Exception:
            pass
        try:
            result = subprocess.run(
                ["xdotool", "getdisplaygeometry"],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
            parts = result.stdout.strip().split()
            if len(parts) >= 2 and parts[0].isdigit() and parts[1].isdigit():
                return int(parts[0]), int(parts[1])
        except Exception:
            pass
        return None

    def _browser_binary(self) -> Optional[str]:
        preferred = os.environ.get("KYLIN_STATUS_BROWSER", "").strip()
        candidates = [
            preferred,
            "google-chrome",
            "google-chrome-stable",
            "chromium",
            "chromium-browser",
            "microsoft-edge",
        ]
        for candidate in candidates:
            if not candidate:
                continue
            path = shutil.which(candidate)
            if path:
                return path
        return None

    def _browser_window_rect(self) -> tuple[int, int, int, int]:
        scale = getattr(self, "_ui_scale", 1.0)
        width, height = _scaled(380, scale), _scaled(230, scale)
        screen_size = self._screen_size()
        if screen_size:
            screen_width, screen_height = screen_size
            x = max(10, screen_width - width - 10)
            y = max(10, screen_height - height - 10)
        else:
            x = 10
            y = 10
        return width, height, x, y

    def _xdotool_window_ids(self, *args: str) -> list[str]:
        xdotool = shutil.which("xdotool")
        if not xdotool:
            return []
        try:
            result = subprocess.run(
                [xdotool, *args],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
        except Exception:
            return []
        if result.returncode != 0:
            return []
        return [line.strip() for line in result.stdout.splitlines() if line.strip().isdigit()]

    def _wmctrl_window_ids(self) -> list[str]:
        wmctrl = shutil.which("wmctrl")
        if not wmctrl:
            return []
        try:
            result = subprocess.run(
                [wmctrl, "-lx"],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
        except Exception:
            return []
        if result.returncode != 0:
            return []
        ids: list[str] = []
        for line in result.stdout.splitlines():
            if "KylinStatusWindow" in line or "任务监视器" in line:
                parts = line.split(None, 1)
                if parts:
                    ids.append(parts[0])
        return ids

    def _status_browser_window_ids(self) -> list[str]:
        ids: list[str] = []
        ids.extend(self._xdotool_window_ids("search", "--class", "KylinStatusWindow"))
        ids.extend(self._xdotool_window_ids("search", "--name", "任务监视器"))
        ids.extend(self._wmctrl_window_ids())
        if self._browser_process:
            ids.extend(
                self._xdotool_window_ids(
                    "search",
                    "--pid",
                    str(self._browser_process.pid),
                )
            )
        return list(dict.fromkeys(ids))

    def _wmctrl_place_window(self, window_id: str, width: int, height: int, x: int, y: int) -> bool:
        wmctrl = shutil.which("wmctrl")
        if not wmctrl:
            return False
        ok = False
        commands = [
            [wmctrl, "-i", "-r", window_id, "-b", "add,above,sticky,skip_taskbar"],
            [wmctrl, "-i", "-r", window_id, "-b", "remove,maximized_vert,maximized_horz"],
            [wmctrl, "-i", "-r", window_id, "-e", f"0,{x},{y},{width},{height}"],
        ]
        for cmd in commands:
            try:
                result = subprocess.run(
                    cmd,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    timeout=1,
                    check=False,
                )
                ok = ok or result.returncode == 0
            except Exception:
                continue
        return ok

    def _xdotool_place_window(self, window_id: str, width: int, height: int, x: int, y: int) -> bool:
        xdotool = shutil.which("xdotool")
        if not xdotool:
            return False
        ok = False
        commands = [
            [xdotool, "windowsize", window_id, str(width), str(height)],
            [xdotool, "windowmove", window_id, str(x), str(y)],
            [xdotool, "windowraise", window_id],
        ]
        for cmd in commands:
            try:
                result = subprocess.run(
                    cmd,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    timeout=1,
                    check=False,
                )
                ok = ok or result.returncode == 0
            except Exception:
                continue
        return ok

    def _place_browser_window_bottom_right(self, width: int, height: int, x: int, y: int):
        if not shutil.which("xdotool") and not shutil.which("wmctrl"):
            return
        deadline = time.time() + 5.0
        while time.time() < deadline:
            window_ids = self._status_browser_window_ids()
            if window_ids:
                placed = False
                for window_id in window_ids:
                    placed = self._wmctrl_place_window(window_id, width, height, x, y) or placed
                    placed = self._xdotool_place_window(window_id, width, height, x, y) or placed
                if placed:
                    return
            time.sleep(0.1)

    def _start_keep_visible_thread(self):
        if self._keep_visible_thread and self._keep_visible_thread.is_alive():
            return

        def keep_visible():
            while not self._keep_visible_stop.wait(1.0):
                if self._stop_requested:
                    return
                width, height, x, y = self._browser_window_rect()
                self._place_browser_window_bottom_right(width, height, x, y)

        self._keep_visible_thread = threading.Thread(
            target=keep_visible,
            daemon=True,
            name="StatusWindowBrowserKeepVisible",
        )
        self._keep_visible_thread.start()

    def bring_to_front(self):
        width, height, x, y = self._browser_window_rect()
        threading.Thread(
            target=lambda: self._place_browser_window_bottom_right(width, height, x, y),
            daemon=True,
        ).start()

    def _open_app_window(self, url: str):
        browser = self._browser_binary()
        if not browser:
            webbrowser.open(url, new=1)
            return

        width, height, x, y = self._browser_window_rect()
        self._browser_profile_dir = tempfile.mkdtemp(prefix="kylin-status-browser-")
        cmd = [
            browser,
            f"--app={url}",
            f"--user-data-dir={self._browser_profile_dir}",
            f"--window-size={width},{height}",
            f"--window-position={x},{y}",
            "--class=KylinStatusWindow",
            "--ozone-platform=x11",
            "--no-first-run",
            "--no-default-browser-check",
            "--disable-session-crashed-bubble",
            "--disable-extensions",
            "--disable-gpu",
            "--disable-vulkan",
            "--disable-features=Vulkan",
            "--disable-dev-shm-usage",
        ]
        if os.environ.get("KYLIN_DISABLE_BROWSER_SANDBOX", "").strip().lower() in {"1", "true", "yes"}:
            cmd.append("--no-sandbox")
        try:
            self._browser_process = subprocess.Popen(
                cmd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                start_new_session=True,
            )
            time.sleep(0.8)
            if self._browser_process.poll() is not None:
                logger.warning(
                    "独立浏览器窗口启动后退出，回退到默认浏览器。命令：%s",
                    " ".join(cmd),
                )
                webbrowser.open(url, new=1)
                return
            self._place_browser_window_bottom_right(width, height, x, y)
            self._start_keep_visible_thread()
        except Exception as e:
            logger.warning("启动独立浏览器窗口失败，回退到默认浏览器：%s", e)
            webbrowser.open(url, new=1)

    def _elapsed_text(self) -> str:
        elapsed = datetime.now() - self._start_time
        hours, remainder = divmod(int(elapsed.total_seconds()), 3600)
        minutes, seconds = divmod(remainder, 60)
        return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

    def _snapshot(self) -> dict:
        with self._lock:
            state = dict(self._state)
            state["details"] = list(self._state["details"])
            state["logs"] = list(self._state["logs"])
            state["elapsed"] = self._elapsed_text()
            return state

    def _status_text(self, status: str) -> str:
        return {
            "pending": "等待中",
            "running": "运行中",
            "success": "已完成",
            "failed": "失败",
            "error": "错误",
            "warning": "警告",
        }.get(status, StatusWindow._localize_display_text(status))

    def set_task(self, task: str):
        with self._lock:
            self._state["task"] = task
            self._session_data["task"] = task

    def update_step(self, step_number: int, goal: str, status: str = "running"):
        with self._lock:
            self._state["step"] = step_number
            self._state["goal"] = StatusWindow._localize_display_text(goal) if goal else "等待任务..."
            self._state["status"] = self._status_text(status)
            self._state["status_key"] = status

    def add_log(self, message: str, level: str = "info"):
        if not StatusWindow._is_primary_operation_message(message, level):
            return
        timestamp = datetime.now().strftime("%H:%M:%S")
        line = f"[{timestamp}] {StatusWindow._localize_display_text(message)}"
        with self._lock:
            self._state["logs"].append(line)
            self._state["logs"] = self._state["logs"][-120:]

    def add_detail(self, message: str):
        if not StatusWindow._is_primary_operation_message(message, "detail"):
            return
        with self._lock:
            self._state["details"].append(StatusWindow._localize_display_text(message))
            self._state["details"] = self._state["details"][-120:]

    def save_record(self, record: dict):
        with self._lock:
            self._step_records.append(record)
            self._session_data["total_steps"] = len(self._step_records)
        self._write_records_to_file()

    def _write_records_to_file(self):
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                save_path = os.path.join(self.save_dir, "step_records.json")
                with open(save_path, "w", encoding="utf-8") as f:
                    json.dump(self._step_records, f, indent=2, ensure_ascii=False)
            except Exception as e:
                logger.debug(f"Failed to save step records: {e}")

    def _save_current_log(self):
        self._save_all_records("manual_save")

    def _save_all_records(self, reason: str):
        self.finalize_session(reason)

    def finalize_session(self, reason: str):
        self._session_data["end_time"] = datetime.now().isoformat()
        self._session_data["end_reason"] = reason
        if self.save_dir:
            try:
                os.makedirs(self.save_dir, exist_ok=True)
                log_data = {
                    "session": self._session_data,
                    "steps": self._step_records,
                    "summary": {},
                }
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                main_log_path = os.path.join(self.save_dir, f"session_log_{timestamp}.json")
                with open(main_log_path, "w", encoding="utf-8") as f:
                    json.dump(log_data, f, indent=2, ensure_ascii=False)
                self.add_log(f"日志已保存：{main_log_path}", "success")
            except Exception as e:
                logger.debug(f"Failed to save complete log: {e}")

    def _on_stop_button_click(self):
        self._stop_requested = True
        with self._lock:
            self._state["stopped"] = True
            self._state["status"] = "已停止"
            self._state["status_key"] = "warning"
        if self.on_close:
            try:
                self.on_close()
            except Exception as e:
                logger.exception(f"Close callback error: {e}")

    def is_stop_requested(self) -> bool:
        return self._stop_requested

    def close(self):
        self._keep_visible_stop.set()
        process = self._browser_process
        if process and process.poll() is None:
            try:
                process.terminate()
                process.wait(timeout=1.0)
            except Exception:
                try:
                    process.kill()
                    process.wait(timeout=0.5)
                except Exception:
                    pass
        self._browser_process = None
        if self._browser_profile_dir:
            try:
                shutil.rmtree(self._browser_profile_dir, ignore_errors=True)
            except Exception:
                pass
            self._browser_profile_dir = None
        if self._server:
            self._server.shutdown()
            self._server.server_close()
            self._server = None


class StatusWindowManager:
    """状态窗口管理器 - 单例模式"""
    _instance: Optional["StatusWindowManager"] = None
    _window = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if not hasattr(self, "_initialized"):
            self._initialized = True

    def create_window(
        self,
        title: str = "任务监视器",
        on_close: Optional[Callable[[], None]] = None,
        save_dir: Optional[str] = None
    ):
        """创建状态窗口"""
        if self._window:
            try:
                if self._window.is_stop_requested():
                    logger.info("旧状态窗口已停止，重新创建")
                    self.close()
                else:
                    logger.warning("Status window already exists")
                    return self._window
            except Exception:
                logger.debug("旧状态窗口状态不可用，重新创建", exc_info=True)
                self.close()

        if self._window:
            logger.warning("Status window already exists")
            return self._window

        backend = os.environ.get("KYLIN_STATUS_BACKEND", "qt").strip().lower()
        if backend in {"browser", "html", "web"}:
            logger.warning(
                "浏览器状态窗口后端已禁用，改用非浏览器后端。"
            )
            backend = "qt"

        if backend != "tk" and QtWidgets is not None and QtWebEngineWidgets is not None:
            try:
                self._window = QtWebEngineProcessStatusWindow(title=title, on_close=on_close, save_dir=save_dir)
                logger.info("%s Qt WebEngine 子进程状态窗口已启动", QT_BINDING or "")
                return self._window
            except Exception as e:
                logger.warning("Qt 状态窗口启动失败，回退到 Tk：%s", e)
        elif backend != "tk":
            logger.warning("Qt WebEngine 不可用，回退到 Tk")

        try:
            self._window = StatusWindow(title=title, on_close=on_close, save_dir=save_dir)
            logger.info("Tk 状态窗口已启动")
            return self._window
        except Exception:
            self._window = None
            raise

    def get_window(self):
        """获取当前窗口实例"""
        return self._window

    def set_task(self, task: str):
        """设置任务"""
        if self._window:
            self._window.set_task(task)

    def update_step(self, step_number: int, goal: str, status: str = "running"):
        """更新步骤信息"""
        if self._window:
            self._window.update_step(step_number, goal, status)

    def add_log(self, message: str, level: str = "info"):
        """添加日志"""
        if self._window:
            self._window.add_log(message, level)

    def add_detail(self, message: str):
        """添加详情"""
        if self._window:
            self._window.add_detail(message)

    def save_record(self, record: dict):
        """保存记录"""
        if self._window:
            self._window.save_record(record)

    def finalize_session(self, reason: str):
        """完成会话"""
        if self._window:
            self._window.finalize_session(reason)

    def is_stop_requested(self) -> bool:
        """检查是否请求停止"""
        return self._window.is_stop_requested() if self._window else False

    def bring_to_front(self):
        """保持监视窗口可见并放回右上角。"""
        if self._window and hasattr(self._window, "bring_to_front"):
            try:
                self._window.bring_to_front()
            except Exception as e:
                logger.debug("Failed to bring status window to front: %s", e)

    def close(self):
        """关闭窗口"""
        if self._window:
            try:
                self._window.close()
            finally:
                self._window = None


# 便捷函数
def get_status_window():
    return StatusWindowManager().get_window()


def create_status_window(
    title: str = "任务监视器",
    on_close: Optional[Callable[[], None]] = None,
    save_dir: Optional[str] = None
):
    return StatusWindowManager().create_window(title=title, on_close=on_close, save_dir=save_dir)


def set_task(task: str):
    StatusWindowManager().set_task(task)


def update_step(step_number: int, goal: str, status: str = "running"):
    StatusWindowManager().update_step(step_number, goal, status)


def add_log(message: str, level: str = "info"):
    StatusWindowManager().add_log(message, level)


def add_detail(message: str):
    StatusWindowManager().add_detail(message)


def save_record(record: dict):
    StatusWindowManager().save_record(record)


def finalize_session(reason: str):
    StatusWindowManager().finalize_session(reason)


def is_stop_requested() -> bool:
    return StatusWindowManager().is_stop_requested()


def _status_window_x11_ids() -> list[str]:
    if platform.system().lower() != "linux":
        return []

    ids: list[str] = []
    seen: set[str] = set()

    def add_window_id(window_id: str):
        window_id = window_id.strip()
        if window_id and window_id not in seen:
            seen.add(window_id)
            ids.append(window_id)

    xdotool = shutil.which("xdotool")
    if xdotool:
        for args in (
            [xdotool, "search", "--name", "任务监视器"],
            [xdotool, "search", "--class", "KylinStatusWindow"],
        ):
            try:
                result = subprocess.run(
                    args,
                    capture_output=True,
                    text=True,
                    timeout=1,
                    check=False,
                )
            except Exception:
                continue
            if result.returncode == 0:
                for line in result.stdout.splitlines():
                    add_window_id(line)

    wmctrl = shutil.which("wmctrl")
    if wmctrl:
        try:
            result = subprocess.run(
                [wmctrl, "-lx"],
                capture_output=True,
                text=True,
                timeout=1,
                check=False,
            )
        except Exception:
            result = None
        if result and result.returncode == 0:
            for line in result.stdout.splitlines():
                if "任务监视器" in line or "KylinStatusWindow" in line:
                    add_window_id(line.split(maxsplit=1)[0])

    return ids


def _raise_status_window_x11() -> bool:
    restored = False
    wmctrl = shutil.which("wmctrl")
    xdotool = shutil.which("xdotool")

    if wmctrl:
        try:
            result = subprocess.run(
                [wmctrl, "-k", "off"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=1,
                check=False,
            )
            restored = restored or result.returncode == 0
        except Exception:
            pass

    for window_id in _status_window_x11_ids():
        if wmctrl:
            for args in (
                [wmctrl, "-i", "-r", window_id, "-b", "remove,hidden"],
                [wmctrl, "-i", "-r", window_id, "-b", "add,above,sticky,skip_taskbar"],
            ):
                try:
                    result = subprocess.run(
                        args,
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL,
                        timeout=1,
                        check=False,
                    )
                    restored = restored or result.returncode == 0
                except Exception:
                    pass
        if xdotool:
            for args in (
                [xdotool, "windowmap", window_id],
                [xdotool, "windowraise", window_id],
            ):
                try:
                    result = subprocess.run(
                        args,
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL,
                        timeout=1,
                        check=False,
                    )
                    restored = restored or result.returncode == 0
                except Exception:
                    pass

    return restored


def bring_status_window_to_front():
    StatusWindowManager().bring_to_front()
    _raise_status_window_x11()


def close_status_window():
    StatusWindowManager().close()
