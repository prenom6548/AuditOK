import os
os.environ["ANONYMIZED_TELEMETRY"] = "false"
os.environ.setdefault("KYLIN_STATUS_BACKEND", "qt")
os.environ.setdefault("KYLIN_X11_SCREENSHOT_BACKEND", "pillow,pyautogui,scrot")
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
os.environ.setdefault("QT_OPENGL", "software")
os.environ.setdefault("QT_QUICK_BACKEND", "software")
if os.environ.get("DISPLAY"):
    os.environ["GDK_BACKEND"] = "x11"
    os.environ["QT_QPA_PLATFORM"] = "xcb"
elif (
    os.environ.get("XDG_SESSION_TYPE", "").strip().lower() == "wayland"
    or os.environ.get("WAYLAND_DISPLAY")
):
    os.environ["GDK_BACKEND"] = "wayland"
    os.environ["QT_QPA_PLATFORM"] = "wayland"
os.environ.setdefault(
    "QTWEBENGINE_CHROMIUM_FLAGS",
    "--disable-gpu --disable-vulkan --disable-features=Vulkan",
)

def _prevent_stdio_pipe_inheritance():
    """Keep parent capture output usable without letting GUI children hold pipes open."""
    if os.environ.get("KYLIN_INHERIT_STDIO", "").strip().lower() in {"1", "true", "yes"}:
        return
    for fd in (1, 2):
        try:
            os.set_inheritable(fd, False)
        except Exception:
            pass

_prevent_stdio_pipe_inheritance()

# Clear proxy environment variables to avoid httpx errors with ollama client
# httpx does not support socks:// scheme, so we remove these before importing langchain_ollama
for proxy_var in ["http_proxy", "https_proxy", "HTTP_PROXY", "HTTPS_PROXY", "all_proxy", "ALL_PROXY"]:
    os.environ.pop(proxy_var, None)

import sys
import json
import logging
import argparse
import asyncio
import glob
import shutil
import platform
import subprocess
from typing import Any
from pathlib import Path
from pynput import keyboard
from datetime import datetime
# Add the project root to Python path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

# Load config early to set logging level before importing src
def load_config_early(path: Path) -> dict:
    """Load configuration from JSON file early to set logging level."""
    if not path.exists():
        raise FileNotFoundError(f"Config file {path} not found.")
    with path.open("r", encoding="utf-8") as fp:
        return json.load(fp)

# Set logging level environment variable before importing src
config_path = Path(__file__).parent / "config.json"
if config_path.exists():
    early_cfg = load_config_early(config_path)
    logging_level = early_cfg.get("logging_level", "INFO").lower()
    os.environ["kylin_LOGGING_LEVEL"] = logging_level

# Monkey-patch tkinter before importing status_window (which uses tkinter)
# This allows Kylin-CUA to run headlessly without python3-tk installed
import sys
class _FakeTkinter:
    def __getattr__(self, name):
        if name == 'TkVersion':
            return 8.6
        return _FakeTkinter()
    def __call__(self, *a, **kw):
        return _FakeTkinter()
    def __lt__(self, other):
        return False
    def __gt__(self, other):
        return True
    def __le__(self, other):
        return True
    def __ge__(self, other):
        return False
    def __eq__(self, other):
        return True
    def __ne__(self, other):
        return False
sys.modules['tkinter'] = _FakeTkinter()
sys.modules['tkinter.font'] = _FakeTkinter()
sys.modules['tkinter.ttk'] = _FakeTkinter()

from langchain_openai import ChatOpenAI
# from langchain_google_genai import ChatGoogleGenerativeAI  # optional, requires pip install langchain-google-genai
# from langchain_anthropic import ChatAnthropic  # optional, requires pip install langchain-anthropic
# from langchain_ollama import ChatOllama  # optional, requires pip install langchain-ollama

from src import Agent
from src.controller.service import Controller
from src.utils.run_identity import create_run_id
from src.utils.local_model import LocalTransformersModel
from src.utils.status_window import create_status_window, get_status_window, is_stop_requested, add_log, update_step, close_status_window

# Global cache for shared model instances (key: model_path, value: model_instance)
_model_cache: dict[str, Any] = {}

# ---------- Utilities -------------------------------------------------------
LOG_LEVEL_MAP = {
    "CRITICAL": logging.CRITICAL,
    "ERROR":    logging.ERROR,
    "WARNING":  logging.WARNING,
    "INFO":     logging.INFO,
    "DEBUG":    logging.DEBUG,
}

HOTKEY_ALIASES = {
    "command": "<cmd>",
    "cmd": "<cmd>",
    "super": "<cmd>",
    "shift": "<shift>",
    "control": "<ctrl>",
    "ctrl": "<ctrl>",
    "option": "<alt>",
    "alt": "<alt>",
    "opt": "<alt>",
    "win": "<cmd>",
    "windows": "<cmd>",
}

def _task_to_slug(task: str, max_len: int = 40) -> str:
    """将任务转换为文件名友好的 slug"""
    import re
    task = task.strip().lower()
    task = re.sub(r"[^a-z0-9]+", "_", task)
    task = task.strip("_")
    if not task:
        task = "task"
    return task[:max_len]

def resolve_output_dir(cfg: dict, config_path: Path) -> Path:
    output_dir = (
        os.getenv("KYLIN_OUTPUT_DIR")
        or cfg.get("output_dir")
        or cfg.get("agent", {}).get("output_dir")
    )
    if output_dir:
        path = Path(output_dir).expanduser()
        if not path.is_absolute():
            path = (Path(config_path).parent / path).resolve()
    else:
        path = (project_root / ".kylin_tmp").resolve()
    path.mkdir(parents=True, exist_ok=True)
    return path

def resolve_artifact_path(raw_path: str | None, output_dir: Path) -> str | None:
    if not raw_path:
        return None
    path = Path(raw_path).expanduser()
    if not path.is_absolute():
        path = output_dir / path
    if path.parent:
        path.parent.mkdir(parents=True, exist_ok=True)
    return str(path)

def load_config(path: Path) -> dict:
    """Load configuration from JSON file."""
    if not path.exists():
        raise FileNotFoundError(f"Config file {path} not found.")
    with path.open("r", encoding="utf-8") as fp:
        return json.load(fp)

def normalize_hotkey(hotkey: str) -> str:
    if not hotkey:
        return ""
    parts = [p for p in hotkey.replace(" ", "").split("+") if p]
    normalized = []
    for part in parts:
        token = part.strip()
        lower = token.lower()
        if lower in HOTKEY_ALIASES:
            normalized.append(HOTKEY_ALIASES[lower])
        elif lower.startswith("<") and lower.endswith(">"):
            normalized.append(lower)
        else:
            normalized.append(lower)
    return "+".join(normalized)

def register_force_stop_hotkey(
    loop: asyncio.AbstractEventLoop,
    agent: Agent,
    agent_task: asyncio.Task,
    hotkey: str,
    log: logging.Logger,
):
    def _force_stop():
        log.warning("Force-stop hotkey pressed. Stopping agent now.")
        agent.stop("force-stop hotkey")
        if not agent_task.done():
            agent_task.cancel()

    def _on_activate():
        loop.call_soon_threadsafe(_force_stop)

    listener = keyboard.GlobalHotKeys({hotkey: _on_activate})
    listener.start()
    return listener


def configure_llm_capabilities(llm, *, supports_tool_calling: bool, supports_response_format: bool):
    setattr(llm, "_kylin_supports_tool_calling", supports_tool_calling)
    setattr(llm, "_kylin_supports_response_format", supports_response_format)
    return llm


def cleanup_previous_runs(working_dir_base: str):
    """Move legacy training artifacts out of the current working directory."""
    # 只移动历史训练数据和截图产物，不处理当前 output_dir。
    move_patterns = ['training_data.jsonl','training_data_cv.jsonl', 'images', 'evaluation_data.jsonl','evaluation_data_cv.jsonl']

    current_dir = os.getcwd()
    matches_to_move = []
    for pattern in move_patterns:
        matches_to_move.extend(glob.glob(os.path.join(current_dir, pattern)))

    if not matches_to_move:
        return

    # Find unique working directory name
    n = 1
    working_dir = f"{working_dir_base}_{n}"
    while os.path.exists(working_dir):
        n += 1
        working_dir = f"{working_dir_base}_{n}"

    # Create working directory
    os.makedirs(working_dir, exist_ok=True)

    # Move specified files/directories
    for match in matches_to_move:
        try:
            dest = os.path.join(working_dir, os.path.basename(match))
            shutil.move(match, dest)
            print(f"Moved: {match} -> {dest}")
        except Exception as e:
            print(f"Error moving {match}: {e}")

def show_desktop_on_start(log: logging.Logger) -> None:
    """Desktop reveal is disabled to avoid minimizing the monitor window on X11."""
    log.info("Show desktop skipped: desktop reveal is disabled")

def _merge_openai_kwargs(cfg: dict, enable_thinking: bool | None = None) -> tuple[dict, dict | None]:
    model_kwargs = cfg.get("model_kwargs")
    if not isinstance(model_kwargs, dict):
        model_kwargs = {}
    else:
        model_kwargs = dict(model_kwargs)

    extra_body = model_kwargs.pop("extra_body", None)
    extra_body_merged = {}
    if isinstance(extra_body, dict):
        extra_body_merged.update(extra_body)

    existing_extra_body = cfg.get("extra_body")
    if isinstance(existing_extra_body, dict) and existing_extra_body:
        extra_body_merged.update(existing_extra_body)

    chat_template_kwargs = cfg.get("chat_template_kwargs")
    if isinstance(chat_template_kwargs, dict):
        merged_chat_template_kwargs = dict(chat_template_kwargs)
    else:
        merged_chat_template_kwargs = {}

    if enable_thinking is not None:
        merged_chat_template_kwargs["enable_thinking"] = bool(enable_thinking)

    if merged_chat_template_kwargs:
        extra_body_merged["chat_template_kwargs"] = merged_chat_template_kwargs

    return model_kwargs, extra_body_merged or None


def build_openai_compatible_llm(
    *,
    model_name: str,
    api_key: str | None,
    base_url: str | None,
    temperature: float = 0.0,
    model_kwargs: dict | None = None,
    extra_body: dict | None = None,
    max_tokens: int | None = None,
    timeout: float | int | None = None,
):
    """
    Build an OpenAI-compatible LLM.
    Uses 'base_url' for new langchain-openai versions, falls back to 'openai_api_base' for older ones.
    """
    if not model_name:
        raise ValueError("OpenAI-compatible provider requires 'model_name'.")
    kwargs = {
        "model": model_name,
        "api_key": api_key,
        "temperature": temperature,
    }
    # New versions of langchain-openai use 'base_url', older ones use 'openai_api_base'
    if base_url:
        # Try base_url first (newer API)
        kwargs["base_url"] = base_url
    if model_kwargs:
        kwargs["model_kwargs"] = model_kwargs
    if extra_body:
        kwargs["extra_body"] = extra_body
    if max_tokens is not None:
        kwargs["max_tokens"] = max_tokens
    if timeout is not None:
        kwargs["timeout"] = timeout
    return ChatOpenAI(**kwargs)


def build_llm(cfg: dict, *, enable_thinking: bool | None = None):
    """Build LLM based on configuration."""
    provider = cfg["provider"].lower()
    api_key = cfg.get("api_key") or os.getenv("API_KEY") or os.getenv("OPENAI_API_KEY")
    model = cfg.get("model_name", "kylin-model")
    base_url = cfg.get("base_url")
    temperature = cfg.get("temperature", 0.0)
    model_kwargs, extra_body = _merge_openai_kwargs(cfg, enable_thinking=enable_thinking)
    max_tokens = cfg.get("max_tokens")
    timeout = cfg.get("timeout")

    if provider == "kylin":
        if not base_url:
            raise ValueError("Kylin provider requires 'base_url'.")
        return build_openai_compatible_llm(
            model_name=model,
            api_key=api_key,
            base_url=base_url,
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens,
            timeout=timeout,
        )

    if provider == "deepseek":
        return build_openai_compatible_llm(
            model_name=model,
            api_key=api_key,
            base_url=base_url or "https://api.deepseek.com/v1",
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens,
            timeout=timeout,
        )

    if provider == "minimax":
        return build_openai_compatible_llm(
            model_name=model,
            api_key=api_key,
            base_url=base_url or "https://api.minimax.chat/v1",
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens,
            timeout=timeout,
        )

    if provider == "kimi":
        return build_openai_compatible_llm(
            model_name=model,
            api_key=api_key,
            base_url=base_url or "https://api.moonshot.cn/v1",
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens,
            timeout=timeout,
        )

    if provider == "google_pro_stable":
        return ChatGoogleGenerativeAI(
            model="gemini-2.5-pro-preview-05-06",
            api_key=api_key,
            temperature=temperature,
        )

    if provider == "google_flash":
        return ChatGoogleGenerativeAI(
            model="gemini-2.5-flash",
            api_key=api_key,
            temperature=temperature,
        )

    if provider in {"openai", "gpt"}:
        return build_openai_compatible_llm(
            model_name=model if provider == "openai" else (model or "gpt-4.1-mini"),
            api_key=api_key,
            base_url=base_url,
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens,
            timeout=timeout,
        )

    if provider == "anthropic":
        return ChatAnthropic(
            model=model,
            api_key=api_key,
            temperature=temperature,
        )

    if provider == "ollama":
        if not model:
            raise ValueError("Ollama provider requires 'model_name'.")
        ollama_kwargs = {"model": model, "temperature": temperature}
        if base_url:
            ollama_kwargs["base_url"] = base_url
        llm = ChatOllama(**ollama_kwargs)
        return configure_llm_capabilities(
            llm,
            supports_tool_calling=bool(cfg.get("supports_tool_calling", True)),
            # Disabled by default because some Ollama model/runtime combos fail with:
            # "failed to load model vocabulary required for format"
            supports_response_format=bool(cfg.get("supports_response_format", False)),
        )

    if provider == "local_transformers":
        if not model:
            raise ValueError("local_transformers provider requires 'model_name' (path to local model).")

        # Create cache key from model path and quantization settings
        cache_key = f"{model}__4bit{cfg.get('load_in_4bit', False)}__8bit{cfg.get('load_in_8bit', False)}"

        # Return cached instance if available
        if cache_key in _model_cache:
            logger = logging.getLogger("kylin")
            logger.info(f"Reusing cached model instance: {cache_key}")
            cached_model = _model_cache[cache_key]
            # Return a new wrapper with different temperature if needed
            return LocalTransformersModelWrapper(
                cached_model,
                temperature=temperature,
                max_tokens=max_tokens if max_tokens is not None else 2048,
            )

        # Create new instance and cache it
        new_model = LocalTransformersModel(
            model_path=model,
            temperature=temperature,
            max_tokens=max_tokens if max_tokens is not None else 2048,
            device_map=cfg.get("device_map", "auto"),
            torch_dtype=cfg.get("torch_dtype", "bfloat16"),
            trust_remote_code=cfg.get("trust_remote_code", True),
            load_in_8bit=cfg.get("load_in_8bit", False),
            load_in_4bit=cfg.get("load_in_4bit", False),
            cpu_offload=cfg.get("cpu_offload", True),
        )
        _model_cache[cache_key] = new_model
        logger = logging.getLogger("kylin")
        logger.info(f"Loaded and cached new model instance: {cache_key}")
        return new_model

    if provider == "openai_compatible":
        """
        OpenAI-compatible API provider for remote model servers (e.g., Ollama, vLLM, LM Studio).
        Supports vision models with base64 image encoding.
        Matches the API format from: curl -sS "$SERVER_URL/v1/chat/completions"
        
        Config example:
        {
            "provider": "openai_compatible",
            "model_name": "holo3:35b",
            "base_url": "http://127.0.0.1:8080",
            "api_key": "not-needed",
            "temperature": 0.2,
            "max_tokens": 512
        }
        """
        if not model:
            raise ValueError("openai_compatible provider requires 'model_name'.")
        if not base_url:
            raise ValueError("openai_compatible provider requires 'base_url'.")
        
        llm = build_openai_compatible_llm(
            model_name=model,
            api_key=api_key or "not-needed",
            base_url=base_url,
            temperature=temperature,
            model_kwargs=model_kwargs,
            extra_body=extra_body,
            max_tokens=max_tokens if max_tokens is not None else 512,
            timeout=timeout,
        )
        return configure_llm_capabilities(
            llm,
            supports_tool_calling=bool(cfg.get("supports_tool_calling", False)),
            # Most OpenAI-compatible providers implement /chat/completions, but not
            # OpenAI's structured parse path. Keep it opt-in per provider.
            supports_response_format=bool(cfg.get("supports_response_format", False)),
        )

    raise ValueError(f"Unknown llm provider '{provider}'")

def setup_logging(logging_level: str, log_file: str | None = None):
    """设置日志（使用新的简化日志系统）"""
    from src.logging_config import setup_logging as setup_session_logging
    setup_session_logging(log_file=log_file, level=logging_level)
    print(f"日志级别设置为：{logging_level.upper()}")
    if log_file:
        print(f"日志文件：{log_file}")


# ---------- Main ------------------------------------------------------------
def main(config_path: str = "config.json", task: str | None = None):
    """Main function to run the agent."""
    if platform.system() not in {"Windows", "Linux"}:
        print("This script currently supports Windows and Linux.")
        sys.exit(1)

    # Make config path relative to script location if it's a relative path
    if not Path(config_path).is_absolute():
        config_path = Path(__file__).parent / config_path

    cfg = load_config(Path(config_path))
    
    # Override task from command line if provided
    if task:
        cfg["agent"]["task"] = task
    output_dir = resolve_output_dir(cfg, Path(config_path))
    agent_cfg = cfg["agent"]
    resume = bool(agent_cfg.get("resume", False))
    configured_agent_id = agent_cfg.get("agent_id")
    if resume and not configured_agent_id:
        raise ValueError("agent.agent_id is required when agent.resume is true")

    if resume:
        run_id = str(configured_agent_id)
        invocation_id = create_run_id(
            agent_cfg.get("task", "task"),
            base_id=f"{run_id}-resume",
        )
    else:
        run_id = create_run_id(
            agent_cfg.get("task", "task"),
            base_id=configured_agent_id,
        )
        invocation_id = run_id
    brain_enable_thinking = cfg.get("brain_enable_thinking")
    if not isinstance(brain_enable_thinking, bool):
        thinking_cfg = cfg.get("thinking")
        if isinstance(thinking_cfg, dict) and isinstance(thinking_cfg.get("brain"), bool):
            brain_enable_thinking = thinking_cfg.get("brain")
        else:
            brain_enable_thinking = False
    
    # Update environment variable if different config was passed
    current_logging_level = cfg.get("logging_level", "INFO").lower()
    if os.environ.get("kylin_LOGGING_LEVEL") != current_logging_level:
        os.environ["kylin_LOGGING_LEVEL"] = current_logging_level
        print(f"Updated logging level to: {current_logging_level.upper()}")

    # --- 日志设置 ---
    # Keep runtime logging on the console only.  The output directory should
    # contain just user-facing artifacts: images and step_records.
    setup_logging(cfg.get("logging_level", "INFO"), log_file=None)
    log = logging.getLogger("kylin")
    log.info("Run ID: %s", run_id)
    if invocation_id != run_id:
        log.info("Resume invocation ID: %s", invocation_id)

    wayland_coordinate_settings = {
        "wayland_ydotool_socket": "YDOTOOL_SOCKET",
        "wayland_ydotool_factor": "KYLIN_WAYLAND_YDOTOOL_FACTOR",
        "wayland_coord_scale_x": "KYLIN_WAYLAND_COORD_SCALE_X",
        "wayland_coord_scale_y": "KYLIN_WAYLAND_COORD_SCALE_Y",
        "wayland_coord_offset_x": "KYLIN_WAYLAND_COORD_OFFSET_X",
        "wayland_coord_offset_y": "KYLIN_WAYLAND_COORD_OFFSET_Y",
    }
    for config_key, env_name in wayland_coordinate_settings.items():
        value = agent_cfg.get(config_key)
        if value is not None:
            os.environ[env_name] = str(value)

    text_input_settings = {
        "text_focus_max_age": "KYLIN_TEXT_FOCUS_MAX_AGE",
        "paste_settle_delay": "KYLIN_PASTE_SETTLE_DELAY",
    }
    for config_key, env_name in text_input_settings.items():
        value = agent_cfg.get(config_key)
        if value is not None:
            os.environ[env_name] = str(value)

    if agent_cfg.get("show_desktop_on_start", False):
        show_desktop_on_start(log)

    # --- Cleanup previous runs ---------------------------------------------
    if cfg.get("cleanup_previous_runs", False):
        working_dir_base = cfg.get("working_dir_base", "Your_directory_name")
        try:
            cleanup_previous_runs(working_dir_base)
        except Exception as e:
            pass
    # --- Build LLM & Agent --------------------------------------------------
    use_plan = cfg.get("agent", {}).get("use_plan", False)
    brain_llm = build_llm(cfg["brain_llm"], enable_thinking=brain_enable_thinking)
    actor_llm = build_llm(cfg["actor_llm"], enable_thinking=False)
    memory_llm = build_llm(cfg["memory_llm"], enable_thinking=False)
    planner_llm = build_llm(cfg["planner_llm"], enable_thinking=True) if use_plan else None
    log.info(
        "Thinking config => brain=%s, actor=%s, memory=%s, planner=%s",
        brain_enable_thinking,
        False,
        False,
        bool(use_plan),
    )
    target_screen = agent_cfg.get("target_screen")
    if target_screen is not None:
        os.environ["KYLIN_TARGET_SCREEN"] = str(target_screen)
        log.info("Target screen set to: %s", os.environ["KYLIN_TARGET_SCREEN"])
    skills_dir = agent_cfg.get("skills_dir")
    if skills_dir:
        skills_dir_path = Path(skills_dir)
        if not skills_dir_path.is_absolute():
            config_relative = (Path(config_path).parent / skills_dir_path).resolve()
            if config_relative.exists():
                skills_dir_path = config_relative
            else:
                project_relative = (project_root / skills_dir_path).resolve()
                skills_dir_path = project_relative
        skills_dir = skills_dir_path
    controller = Controller(
        action_interval=agent_cfg.get("action_interval", 0.1),
        remember_clipboard_after_click=agent_cfg.get("remember_clipboard_after_click", False),
    )
    raw_hotkey = agent_cfg.get("force_stop_hotkey")
    force_stop_hotkey = normalize_hotkey(raw_hotkey) if raw_hotkey else ""
    save_brain_conversation_path = None
    save_actor_conversation_path = None
    save_planner_conversation_path = None
    memory_budget_tokens = agent_cfg.get("memory_budget_tokens", agent_cfg.get("memory_budget", 500))
    summary_memory_budget_tokens = agent_cfg.get(
        "summary_memory_budget_tokens",
        agent_cfg.get("summary_memory_budget"),
    )

    agent = Agent(
        task=agent_cfg["task"],
        brain_llm=brain_llm,
        actor_llm=actor_llm,
        memory_llm=memory_llm,
        planner_llm=planner_llm,
        memory_budget=memory_budget_tokens,
        summary_memory_budget=summary_memory_budget_tokens,
        controller=controller,
        max_actions_per_step=agent_cfg.get("max_actions_per_step", 5),
        max_failures=agent_cfg.get("max_failures", 8),
        no_progress_max_steps=agent_cfg.get("no_progress_max_steps", 6),
        web_navigation_switch_steps=agent_cfg.get("web_navigation_switch_steps", 3),
        web_navigation_max_stalled_steps=agent_cfg.get("web_navigation_max_stalled_steps", 7),
        grounding_hotspot_failure_threshold=agent_cfg.get("grounding_hotspot_failure_threshold", 2),
        human_assistance_failure_window=agent_cfg.get("human_assistance_failure_window", 18),
        human_assistance_failure_threshold=agent_cfg.get("human_assistance_failure_threshold", 14),
        human_assistance_guard_threshold=agent_cfg.get("human_assistance_guard_threshold", 6),
        human_assistance_obstacle_threshold=agent_cfg.get("human_assistance_obstacle_threshold", 3),
        human_assistance_min_steps=agent_cfg.get("human_assistance_min_steps", 10),
        human_assistance_min_strategies=agent_cfg.get("human_assistance_min_strategies", 4),
        step_delay=agent_cfg.get("step_delay", 0.2),
        use_search=agent_cfg.get("use_search", True),
        use_skills=agent_cfg.get("use_skills", False),
        skills_dir=str(skills_dir) if skills_dir else None,
        skills_max_chars=agent_cfg.get("skills_max_chars", 4000),
        resume=agent_cfg.get("resume", False),
        agent_id=agent_cfg.get("agent_id"),
        run_id=run_id,
        save_brain_conversation_path=save_brain_conversation_path,
        save_brain_conversation_path_encoding=agent_cfg.get(
            "save_brain_conversation_path_encoding", "utf-8"
        ),
        save_actor_conversation_path=save_actor_conversation_path,
        save_actor_conversation_path_encoding=agent_cfg.get(
            "save_actor_conversation_path_encoding", "utf-8"
        ),
        save_planner_conversation_path=save_planner_conversation_path,
        save_planner_conversation_path_encoding=agent_cfg.get(
            "save_planner_conversation_path_encoding", "utf-8"
        ),
        artifacts_dir=str(output_dir),
        status_window_enabled=agent_cfg.get("status_window_enabled", True),
        brain_include_previous_screenshot=agent_cfg.get("brain_include_previous_screenshot", False),
        combined_step_enabled=agent_cfg.get("combined_step_enabled", True),
        completion_verification_enabled=agent_cfg.get(
            "completion_verification_enabled", True
        ),
        completion_required_confirmations=agent_cfg.get(
            "completion_required_confirmations", 2
        ),
        execution_journal_enabled=agent_cfg.get("execution_journal_enabled", False),
        persist_run_state=agent_cfg.get("persist_run_state", False),
        cleanup_images_on_finish=agent_cfg.get("cleanup_images_on_finish", True),
    )
    run_result_path = output_dir / "run_results" / invocation_id / "result.json"
    agent.run_result_path = run_result_path
    log.info("Run result path: %s", run_result_path)
    print(f"KYLIN_RESULT_JSON={run_result_path}", flush=True)

    def on_window_close():
        """窗口关闭回调 - 停止 Agent"""
        log.warning("状态窗口已关闭，正在停止 Agent...")
        agent.stop("status window closed")

    def ensure_status_window():
        """Create the monitor after show-desktop/hide commands have run."""
        if not agent_cfg.get("status_window_enabled", True):
            return
        if get_status_window():
            agent._status_window = get_status_window()
            return
        try:
            create_status_window(
                title="任务监视器",
                on_close=on_window_close,
                save_dir=None
            )
            # 让 Agent 获取窗口实例
            agent._status_window = get_status_window()
            log.info("状态窗口已创建并关联到 Agent")
            add_log("Agent starting...", "info")
        except Exception as e:
            log.warning(f"创建状态窗口失败：{e}，将继续运行但不显示窗口")

    async def runner():
        import signal
        loop = asyncio.get_running_loop()
        agent_task = None
        
        # lhh 添加 kylin-agent 可见性控制，确保在 Agent 运行期间隐藏窗口，结束后恢复
        async def _run_kylin_agent_visibility(command_arg: str):
            try:
                proc = await asyncio.create_subprocess_exec(
                    "kylin-agent",
                    command_arg,
                    stdout=asyncio.subprocess.PIPE,
                    stderr=asyncio.subprocess.PIPE,
                )
                stdout, stderr = await proc.communicate()
                if stdout:
                    log.info("kylin-agent %s: %s", command_arg, stdout.decode(errors="replace").strip())
                if proc.returncode != 0:
                    log.warning(
                        "kylin-agent %s failed with code %s: %s",
                        command_arg,
                        proc.returncode,
                        stderr.decode(errors="replace").strip(),
                    )
                elif stderr:
                    log.debug("kylin-agent %s stderr: %s", command_arg, stderr.decode(errors="replace").strip())
            except FileNotFoundError:
                log.warning("kylin-agent command not found; skip %s", command_arg)
            except Exception:
                log.exception("Failed to execute kylin-agent %s", command_arg)

        #添加执行指令kylin-agent --hide  lhh
        await _run_kylin_agent_visibility("--hide")
        ensure_status_window()
        agent_task = asyncio.create_task(agent.run(max_steps=agent_cfg.get("max_steps", 100)))
        
        listener = None
        if force_stop_hotkey:
            try:
                listener = register_force_stop_hotkey(loop, agent, agent_task, force_stop_hotkey, log)
                log.info("Force-stop hotkey registered: %s", force_stop_hotkey)
            except Exception:
                log.exception("Failed to register force-stop hotkey: %s", raw_hotkey)

        # 监控窗口关闭和 Agent 任务
        try:
            # 定期检查窗口状态
            while not agent_task.done():
                # 检查是否通过窗口请求停止
                if is_stop_requested():
                    log.warning("检测到窗口关闭请求，正在停止 Agent...")
                    agent.stop("status window closed")
                    agent_task.cancel()
                    break
                # 等待一小段时间再检查
                await asyncio.sleep(0.5)

            # 等待任务完成（如果还未完成）
            if not agent_task.done():
                try:
                    await agent_task
                except asyncio.CancelledError:
                    pass
        except asyncio.CancelledError:
            log.warning("Agent task cancelled.")
        except KeyboardInterrupt:
            log.warning("Interrupted by user. Stopping gracefully...")
            agent.stop("user interrupt")
            if agent_task and not agent_task.done():
                agent_task.cancel()
            try:
                if agent_task:
                    await agent_task
            except asyncio.CancelledError:
                pass
        finally:
            if listener:
                try:
                    listener.stop()
                except Exception:
                    pass
            log.info("Cleaning up resources...")
            #添加执行指令kylin-agent --show lhh
            await _run_kylin_agent_visibility("--show")
            close_status_window()

    asyncio.run(runner())
    print(f"KYLIN_RESULT_JSON={run_result_path}", flush=True)
    try:
        result_payload = json.loads(run_result_path.read_text(encoding="utf-8"))
        print(f"KYLIN_STATUS={result_payload.get('status', 'unknown')}", flush=True)
        user_action_required = str(result_payload.get("user_action_required") or "").strip()
        if user_action_required:
            print(f"KYLIN_USER_ACTION_REQUIRED={user_action_required}", flush=True)
        resume_condition = str(result_payload.get("resume_condition") or "").strip()
        if resume_condition:
            print(f"KYLIN_RESUME_CONDITION={resume_condition}", flush=True)
    except Exception as e:
        log.warning("Failed to print run result summary: %s", e)


# ---------- Model Wrapper for Shared Instances ------------------------------
class LocalTransformersModelWrapper:
    """
    Wrapper around a cached LocalTransformersModel instance.
    Allows different temperature/max_tokens settings while sharing the underlying model.
    """
    # Class-level flag for type detection in to_structured()
    _is_local_transformers_wrapper = True
    # Tool calling support flag
    _kylin_supports_tool_calling = False
    _kylin_supports_response_format = True
    
    def __init__(self, base_model: LocalTransformersModel, temperature: float = 0.1, max_tokens: int = 2048):
        self._base_model = base_model
        self._temperature = temperature
        self._max_tokens = max_tokens
        self._model_name = base_model.model_name
        self._bound_kwargs = {}

    @property
    def model_name(self) -> str:
        return self._model_name

    def bind(self, **kwargs: Any) -> "LocalTransformersModelWrapper":
        """Bind parameters for future calls"""
        # Merge with existing bound kwargs
        merged = {**self._bound_kwargs, **kwargs}
        new_wrapper = LocalTransformersModelWrapper(
            self._base_model,
            temperature=self._temperature,
            max_tokens=self._max_tokens,
        )
        # Store merged bound parameters
        new_wrapper._bound_kwargs = merged
        logger = logging.getLogger("kylin")
        logger.debug(f"LocalTransformersModelWrapper.bind called with: {list(merged.keys())}")
        return new_wrapper

    def with_structured_output(self, schema: dict | type) -> "LocalTransformersModelWrapper":
        """Support structured output - handles both dict schemas and Pydantic classes"""
        logger = logging.getLogger("kylin")
        logger.info("LocalTransformersModelWrapper.with_structured_output called with schema type: %s", type(schema).__name__)
        
        # If schema is a Pydantic class, extract its JSON schema
        if isinstance(schema, type):
            try:
                # Pydantic v2: use model_json_schema()
                if hasattr(schema, "model_json_schema"):
                    json_schema_dict = schema.model_json_schema()
                # Pydantic v1: use schema()
                elif hasattr(schema, "schema"):
                    json_schema_dict = schema.schema()
                else:
                    logger.warning("Cannot extract JSON schema from %s, using fallback", schema)
                    json_schema_dict = {"type": "object"}
                
                logger.info("Extracted JSON schema from Pydantic class %s - keys: %s", 
                           getattr(schema, "__name__", "unknown"), list(json_schema_dict.keys()))
                logger.debug("JSON schema structure: %s", json.dumps(json_schema_dict, ensure_ascii=False, indent=2)[:500])
                
                response_format = {
                    "type": "json_schema",
                    "json_schema": {
                        "name": getattr(schema, "__name__", "output_schema"),
                        "schema": json_schema_dict,
                        "strict": False
                    }
                }
            except Exception as e:
                logger.warning("Failed to extract JSON schema from %s: %s", schema, e)
                response_format = {"type": "json_schema", "json_schema": schema}
        else:
            # Schema is already a dict (like BRAIN_RESPONSE_FORMAT)
            response_format = {"type": "json_schema", "json_schema": schema}
            logger.debug("Using dict schema directly")
        
        return self.bind(response_format=response_format)

    async def ainvoke(self, messages: Any, **kwargs: Any) -> Any:
        """Async invoke - delegate to base model with overridden settings"""
        merged_kwargs = {**self._bound_kwargs, **kwargs}

        # Override temperature and max_tokens if not explicitly set
        if "temperature" not in merged_kwargs:
            merged_kwargs["temperature"] = self._temperature
        if "max_tokens" not in merged_kwargs:
            merged_kwargs["max_tokens"] = self._max_tokens

        logger = logging.getLogger("kylin")
        logger.info("[LocalTransformersModelWrapper] ainvoke called with keys: %s", list(merged_kwargs.keys()))
        if "response_format" in merged_kwargs:
            rf = merged_kwargs["response_format"]
            if isinstance(rf, dict) and "json_schema" in rf:
                schema_name = rf.get("json_schema", {}).get("name", "unknown")
                logger.info("[LocalTransformersModelWrapper] response_format schema name: %s", schema_name)

        # Call base model
        logger.info("[LocalTransformersModelWrapper] Calling base_model.ainvoke with keys: %s", list(merged_kwargs.keys()))
        response = await self._base_model.ainvoke(messages, **merged_kwargs)

        # Wrap response to preserve 'parsed' attribute for structured output
        return _WrappedResponse(response)

    def invoke(self, messages: Any, **kwargs: Any) -> Any:
        """Sync invoke - delegate to base model with overridden settings"""
        merged_kwargs = {**self._bound_kwargs, **kwargs}
        
        if "temperature" not in merged_kwargs:
            merged_kwargs["temperature"] = self._temperature
        if "max_tokens" not in merged_kwargs:
            merged_kwargs["max_tokens"] = self._max_tokens
        
        logger = logging.getLogger("kylin")
        logger.debug(f"LocalTransformersModelWrapper.invoke called, bound_kwargs keys: {list(self._bound_kwargs.keys())}")
        
        response = self._base_model.invoke(messages, **merged_kwargs)
        return _WrappedResponse(response)

    def _generate(self, messages: Any, **kwargs: Any) -> Any:
        """Generate - delegate to base model"""
        merged_kwargs = {**self._bound_kwargs, **kwargs}
        
        if "temperature" not in merged_kwargs:
            merged_kwargs["temperature"] = self._temperature
        if "max_tokens" not in merged_kwargs:
            merged_kwargs["max_tokens"] = self._max_tokens
        
        return self._base_model._generate(messages, **merged_kwargs)


class _WrappedResponse:
    """
    Wrapper to preserve 'parsed' attribute from base model response.
    """
    def __init__(self, response: Any):
        self._response = response
        self.content = getattr(response, "content", None)
        self.response_metadata = getattr(response, "response_metadata", None)
        # Preserve 'parsed' attribute for structured output
        if hasattr(response, "parsed"):
            self.parsed = response.parsed
        else:
            self.parsed = None

    def __getattr__(self, name: str) -> Any:
        # Delegate any other attributes to the wrapped response
        return getattr(self._response, name)

    def __repr__(self) -> str:
        return f"_WrappedResponse(content={self.content!r}, parsed={self.parsed!r})"


# ---------- CLI -------------------------------------------------------------
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run the Kylin desktop agent.")
    parser.add_argument(
        "-c", "--config", default="config.json",
        help="Path to configuration JSON file"
    )
    parser.add_argument(
        "task", nargs="?", default=None,
        help="Task description (e.g., '打开微信，给华哥发送：你好')"
    )
    args = parser.parse_args()
    main(args.config, task=args.task)
