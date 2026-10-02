from __future__ import annotations
import asyncio
import base64
import io
import json
import logging
import os
import platform
import shutil
import tempfile
import time
from pathlib import Path
import re
from datetime import datetime
from typing import Any, Callable, Optional, Type, TypeVar
from collections import OrderedDict, deque

from dotenv import load_dotenv
from langchain_core.language_models.chat_models import BaseChatModel
from langchain_openai import ChatOpenAI, AzureChatOpenAI
try:
    from langchain_anthropic import ChatAnthropic
except ImportError:
    ChatAnthropic = None
try:
    from langchain_google_genai import ChatGoogleGenerativeAI
except ImportError:
    ChatGoogleGenerativeAI = None
try:
    from langchain_ollama import ChatOllama
except ImportError:
    ChatOllama = None
from langchain_core.messages import AIMessage, BaseMessage, HumanMessage
from openai import RateLimitError
from pydantic import BaseModel, ValidationError
from PIL import Image, ImageChops, ImageStat

from src.agent.message_manager.service import MessageManager
from src.agent.prompts import (
    BrainPrompt_kylin,
    ActorPrompt_kylin,
    CombinedPrompt_kylin,
    MemoryPrompt,
    PlannerPrompt,
)
from src.agent.views import (
    ActionResult,
    AgentError,
    AgentHistory,
    AgentHistoryList,
    AgentOutput,
    AgentStepInfo,
    AgentBrain,
)
from src.utils.record_store import RecordStore
from src.utils.brain_search import BrainSearchFlow
from src.utils.skills import (
    load_skill_metadata,
    load_skill_contents,
    format_skill_catalog,
    format_skill_context,
)
from src.agent.planner_service import Planner
from src.controller.service import Controller
from src.utils import time_execution_async
from src.utils.token_counter import TokenCounter
from src.utils.keyboard import normalize_key, split_hotkey
from src.utils.run_identity import create_run_id
from src.agent.output_schemas import OutputSchemas
from src.agent.structured_llm import *
from src.agent.execution_guard import CompletionGuard, ExecutionJournal
from src.logger import get_session_logger

load_dotenv()
logger = logging.getLogger(__name__)
session_logger = None  # 将在 agent 初始化时设置

T = TypeVar("T", bound=BaseModel)

TASK_ID_MAX_LEN = 60
MAX_PENDING_LINES = 20
MAX_BRAIN_CONTEXT_ENTRIES = 50
# Mean absolute grayscale difference (0-255) across a 64x64 thumbnail above
# which two screenshots are considered "visibly different".
_SCREEN_DIFF_MEAN_THRESHOLD = 3.0
# Actions that are passive/navigation-only and never lose or mis-dispatch
# unconfirmed text; they are always allowed through the text-result gate.
_TEXT_GATE_PASSIVE_ACTIONS = {"Click", "DoubleClick", "wait", "move_mouse", "scroll_up", "scroll_down"}
_COMBINED_REQUIRED_FIELDS = ("analysis", "current_state", "action")
_COMBINED_RESPONSE_ARCHIVE_LIMIT = 50000
_COMBINED_ACTION_STRING_PARAMETERS = {
    "open_app": "app_name",
    "input_text": "text",
    "Hotkey": "key",
    "run_apple_script": "script",
}
_COMBINED_ACTION_POSITION_PARAMETERS = {
    "Click",
    "DoubleClick",
    "RightSingle",
    "move_mouse",
}


def _combined_response_text(response: Any) -> str:
    """Extract readable model output across LangChain/provider response shapes."""
    content = getattr(response, "content", response)
    if isinstance(content, str):
        return content
    if isinstance(content, dict):
        return json.dumps(content, ensure_ascii=False)
    if isinstance(content, list):
        parts: list[str] = []
        for block in content:
            if isinstance(block, dict):
                value = block.get("text", block.get("content", ""))
            else:
                value = getattr(block, "text", getattr(block, "content", ""))
            if value:
                parts.append(str(value))
        if parts:
            return "\n".join(parts)
    return str(content)


def _normalize_combined_action_item(action: Any) -> Any:
    """Canonicalize common prompt-only action shapes without changing meaning."""
    if not isinstance(action, dict):
        return action

    if isinstance(action.get("action"), str):
        action_name = action["action"].strip()
        explicit_params = action.get("params")
        params = (
            dict(explicit_params)
            if isinstance(explicit_params, dict)
            else {
                key: value
                for key, value in action.items()
                if key not in {"action", "params"}
            }
        )
        return {action_name: params}

    if len(action) != 1:
        return action

    action_name, params = next(iter(action.items()))
    if params is None:
        return {action_name: {}}
    if isinstance(params, str) and action_name in _COMBINED_ACTION_STRING_PARAMETERS:
        return {action_name: {_COMBINED_ACTION_STRING_PARAMETERS[action_name]: params}}
    if isinstance(params, (list, tuple)) and action_name in _COMBINED_ACTION_POSITION_PARAMETERS:
        return {action_name: {"position": list(params)}}
    if isinstance(params, (int, float)) and action_name == "wait":
        return {action_name: {"seconds": params}}
    return action


def _normalize_prompt_only_combined_payload(
    payload: Any,
    *,
    default_sop_check: Optional[str] = None,
) -> Any:
    """Normalize harmless local-model JSON variations before strict validation.

    This deliberately does not invent step_evaluate, ask_human or next_goal.
    Missing semantic state therefore still triggers the constrained retry.
    """
    if isinstance(payload, BaseModel):
        payload = payload.model_dump(exclude_none=True, exclude_unset=True)
    if not isinstance(payload, dict):
        return payload

    normalized = dict(payload)
    changes: list[str] = []

    if "action" not in normalized and "actions" in normalized:
        normalized["action"] = normalized.pop("actions")
        changes.append("actions->action")

    analysis = normalized.get("analysis")
    if isinstance(analysis, str):
        normalized_analysis: dict[str, Any] = {"analysis": analysis}
        if default_sop_check is not None:
            normalized_analysis["sop_check"] = default_sop_check
        normalized["analysis"] = normalized_analysis
        changes.append("analysis:string->object")
    elif isinstance(analysis, dict):
        normalized_analysis = dict(analysis)
        if "analysis" not in normalized_analysis and isinstance(
            normalized_analysis.get("reasoning"), str
        ):
            normalized_analysis["analysis"] = normalized_analysis.pop("reasoning")
            changes.append("analysis.reasoning->analysis.analysis")
        if "sop_check" not in normalized_analysis and default_sop_check is not None:
            normalized_analysis["sop_check"] = default_sop_check
            changes.append("analysis.sop_check:default")
        normalized["analysis"] = normalized_analysis

    current_state = normalized.get("current_state")
    if current_state is None:
        lifted_state = {
            key: normalized.pop(key)
            for key in ("step_evaluate", "ask_human", "next_goal")
            if key in normalized
        }
        if lifted_state:
            normalized["current_state"] = lifted_state
            current_state = lifted_state
            changes.append("top-level-state->current_state")
    if isinstance(current_state, dict):
        normalized_state = dict(current_state)
        if "step_evaluate" not in normalized_state and isinstance(
            normalized_state.get("evaluation_previous_goal"), str
        ):
            normalized_state["step_evaluate"] = normalized_state.pop(
                "evaluation_previous_goal"
            )
            changes.append("evaluation_previous_goal->step_evaluate")
        normalized["current_state"] = normalized_state

    actions = normalized.get("action")
    if isinstance(actions, dict):
        actions = [actions]
        normalized["action"] = actions
        changes.append("action:object->array")
    if isinstance(actions, list):
        canonical_actions = [_normalize_combined_action_item(action) for action in actions]
        if canonical_actions != actions:
            normalized["action"] = canonical_actions
            changes.append("action-items:canonicalized")

    if changes:
        logger.info("[Combined] Normalized prompt-only output: %s", ", ".join(changes))
    return normalized


def _validate_combined_payload(payload: Any) -> dict:
    if isinstance(payload, BaseModel):
        payload = payload.model_dump(exclude_none=True, exclude_unset=True)
    if not isinstance(payload, dict):
        raise ValueError(
            f"Combined response must be a JSON object, got {type(payload).__name__}."
        )

    missing = [field for field in _COMBINED_REQUIRED_FIELDS if field not in payload]
    if missing:
        raise ValueError(
            "Combined response missing required fields after read-files handling: "
            + ", ".join(missing)
        )
    if not isinstance(payload.get("current_state"), dict):
        raise ValueError("Combined current_state must be a JSON object.")
    if not isinstance(payload.get("action"), list):
        raise ValueError("Combined action must be a JSON array.")
    try:
        validated = CombinedOutput.model_validate(payload)
    except ValidationError as validation_error:
        raise ValueError(
            f"Combined response failed schema validation: {validation_error}"
        ) from validation_error
    return validated.model_dump(exclude_none=True, exclude_unset=True)


def _build_combined_retry_messages(
    messages: list[BaseMessage],
    invalid_response: str,
    reason: str,
) -> list[BaseMessage]:
    correction = (
        "上一条 Combined 输出未通过格式校验："
        f"{reason[:600]}\n"
        "请只重试一次，只返回一个 JSON 对象，不要 Markdown、解释或前后缀。"
        "正常步骤必须严格使用："
        '{"analysis":{"analysis":"中文分析","sop_check":"None"},'
        '"current_state":{"step_evaluate":"Success",'
        '"ask_human":"No","next_goal":"中文目标"},'
        '"action":[{"wait":{}}]}。'
        "不要使用 actions，不要把 analysis 写成字符串。"
        "如果确实需要读文件，则只返回："
        '{"read_files":{"files":["file_a.txt"]}}。'
    )
    return [
        *messages,
        AIMessage(content=(invalid_response or "<empty response>")[:_COMBINED_RESPONSE_ARCHIVE_LIMIT]),
        HumanMessage(content=correction),
    ]

if platform.system() == "Linux":
    from src.linux.screen import (
        capture_screenshot,
        capture_target_screenshot,
        get_cached_wayland_effective_screen_rect,
        get_last_screenshot_backend,
        get_target_screen_rect,
        is_wayland_session,
    )
else:
    capture_screenshot = None
    capture_target_screenshot = None
    get_cached_wayland_effective_screen_rect = None
    get_last_screenshot_backend = None
    get_target_screen_rect = None
    is_wayland_session = None


def _take_pyautogui_screenshot(region: Optional[tuple[int, int, int, int]] = None):
    import pyautogui

    if region is not None:
        return pyautogui.screenshot(region=region)
    return pyautogui.screenshot()


def _capture_screenshot_with_breakdown() -> tuple[Any, dict[str, float]]:
    timings = {
        "screenshot_rect_time": 0.0,
        "screenshot_capture_time": 0.0,
    }

    rect = None
    if get_target_screen_rect is not None and capture_screenshot is not None:
        if (
            is_wayland_session is not None
            and is_wayland_session()
            and not os.getenv("KYLIN_TARGET_SCREEN")
            and get_cached_wayland_effective_screen_rect is not None
            and get_cached_wayland_effective_screen_rect() is None
        ):
            capture_start = time.time()
            screenshot = capture_screenshot()
            timings["screenshot_capture_time"] = time.time() - capture_start
            if get_last_screenshot_backend is not None:
                timings["screenshot_backend"] = get_last_screenshot_backend()
            return screenshot, timings

        rect_start = time.time()
        rect = get_target_screen_rect()
        timings["screenshot_rect_time"] = time.time() - rect_start

        capture_start = time.time()
        screenshot = capture_screenshot(region=rect)
        timings["screenshot_capture_time"] = time.time() - capture_start
        if get_last_screenshot_backend is not None:
            timings["screenshot_backend"] = get_last_screenshot_backend()
        return screenshot, timings

    capture_start = time.time()
    screenshot = _take_pyautogui_screenshot()
    timings["screenshot_capture_time"] = time.time() - capture_start
    timings["screenshot_backend"] = "pyautogui"
    return screenshot, timings


def _task_to_slug(task: str, max_len: int = TASK_ID_MAX_LEN) -> str:
    task = task.strip().lower()
    task = re.sub(r"[^a-z0-9]+", "-", task)
    task = task.strip("-")
    if not task:
        task = "task"
    return task[:max_len]


def _default_agent_id(task: str, now: datetime) -> str:
    """Create an execution-scoped ID instead of a day/task-scoped ID."""
    return create_run_id(task, now=now)


def screenshot_to_dataurl(screenshot):
    img_byte_arr = io.BytesIO()
    screenshot.save(img_byte_arr, format="PNG")
    base64_encoded = base64.b64encode(img_byte_arr.getvalue()).decode("utf-8")
    return f"data:image/png;base64,{base64_encoded}"


def _append_labeled_screenshot_frames(
    state_content: list[dict[str, Any]],
    *,
    current_screenshot_dataurl: Optional[str],
    previous_screenshot_dataurl: Optional[str] = None,
) -> None:
    """Label temporal screenshot roles so vision models do not inspect the old frame."""
    if previous_screenshot_dataurl:
        state_content.append(
            {
                "type": "text",
                "content": (
                    "PREVIOUS SCREENSHOT (context only; captured before the previous "
                    "action, do not use it as the current result):"
                ),
            }
        )
        state_content.append(
            {
                "type": "image_url",
                "image_url": {"url": previous_screenshot_dataurl},
            }
        )
    if current_screenshot_dataurl:
        state_content.append(
            {
                "type": "text",
                "content": (
                    "CURRENT SCREENSHOT (authoritative newest frame captured after the "
                    "previous action; evaluate success/failure from this image):"
                ),
            }
        )
        state_content.append(
            {
                "type": "image_url",
                "image_url": {"url": current_screenshot_dataurl},
            }
        )


def _llm_identity_text(llm: Optional[BaseChatModel]) -> str:
    if llm is None:
        return ""
    parts = [
        llm.__class__.__name__,
        getattr(llm, "model_name", ""),
        getattr(llm, "model", ""),
        getattr(llm, "openai_api_base", ""),
        getattr(llm, "base_url", ""),
    ]
    return " ".join(str(part).lower() for part in parts if part)


def llm_supports_response_format(llm: Optional[BaseChatModel]) -> bool:
    if llm is None:
        return False
    explicit = getattr(llm, "_kylin_supports_response_format", None)
    if explicit is not None:
        return bool(explicit)

    if isinstance(llm, (ChatOpenAI, AzureChatOpenAI)):
        identity = _llm_identity_text(llm)
        unsupported_tokens = ("deepseek", "minimax", "m2.5", "moonshot", "kimi")
        return not any(token in identity for token in unsupported_tokens)
    return True


def downscale_screenshot_by_tier(screenshot):
    """
    Apply tiered screenshot downscaling:
    - 720p/1080p: keep original
    - 2K/4K (and >2200px, <8K): divide width/height by 2
    - 8K/16K: divide width/height by 4
    """
    width, height = screenshot.size
    max_dim = max(width, height)
    scale_factor = 1

    if max_dim >= 7680:
        scale_factor = 4
    elif max_dim > 2200:
        scale_factor = 2

    if scale_factor == 1:
        return screenshot

    target_size = (max(1, width // scale_factor), max(1, height // scale_factor))
    if hasattr(Image, "Resampling"):
        resample = Image.Resampling.LANCZOS
    else:
        resample = Image.LANCZOS
    resized = screenshot.resize(target_size, resample=resample)
    logger.debug(
        "Downscaled screenshot from %sx%s to %sx%s (scale factor: %s)",
        width,
        height,
        resized.width,
        resized.height,
        scale_factor,
    )
    return resized


def to_structured(
    llm: Optional[BaseChatModel], Schema, Structured_Output
) -> Optional[BaseChatModel]:
    """
    Wrap any LangChain chat model with the right structured-output mechanism:

    - ChatOpenAI / AzureChatOpenAI -> bind(response_format=...) (OpenAI style)
    - ChatAnthropic / ChatGoogleGenerativeAI -> with_structured_output(...) (Claude/Gemini style)
    - ChatOllama -> bind(format=<json schema>) (Ollama json schema, when enabled)
    - LocalTransformersModel -> bind(response_format=...) (custom)
    - anything else -> returned unchanged
    """
    OPENAI_CLASSES: tuple[Type[BaseChatModel], ...] = (ChatOpenAI, AzureChatOpenAI)
    ANTHROPIC_OR_GEMINI: tuple[Type[BaseChatModel], ...] = tuple(
        cls for cls in [ChatAnthropic, ChatGoogleGenerativeAI] if cls is not None
    )
    OLLAMA_CLASSES: tuple[Type[BaseChatModel], ...] = tuple(cls for cls in [ChatOllama] if cls is not None)

    # Import LocalTransformersModel if available
    LOCAL_MODEL_CLASS = None
    try:
        from src.utils.local_model import LocalTransformersModel
        LOCAL_MODEL_CLASS = LocalTransformersModel
    except ImportError:
        pass

    if isinstance(llm, OPENAI_CLASSES):
        if not llm_supports_response_format(llm):
            logger.info(
                "Structured response_format is disabled for model '%s'; falling back to prompt-only JSON.",
                getattr(llm, "model_name", getattr(llm, "model", "unknown")),
            )
            return llm

        # OpenAI cloud endpoint expects flattened json_schema fields under response_format
        # (type/name/schema/strict), while many OpenAI-compatible backends accept the nested
        # {"type":"json_schema","json_schema":{...}} shape.
        response_format = Schema
        base_url = str(
            getattr(llm, "openai_api_base", "") or getattr(llm, "base_url", "") or ""
        ).lower()
        is_openai_cloud = (not base_url) or ("api.openai.com" in base_url)
        if is_openai_cloud and isinstance(Schema, dict):
            schema_type = Schema.get("type")
            json_schema = Schema.get("json_schema")
            if schema_type == "json_schema" and isinstance(json_schema, dict):
                flat = {"type": "json_schema"}
                if json_schema.get("name"):
                    flat["name"] = json_schema.get("name")
                if json_schema.get("schema") is not None:
                    flat["schema"] = json_schema.get("schema")
                if json_schema.get("strict") is not None:
                    flat["strict"] = json_schema.get("strict")
                response_format = flat
        return llm.bind(response_format=response_format)

    if isinstance(llm, ANTHROPIC_OR_GEMINI):
        return llm.with_structured_output(Structured_Output)

    if isinstance(llm, OLLAMA_CLASSES):
        if not llm_supports_response_format(llm):
            logger.info(
                "Structured response_format is disabled for Ollama model '%s'; falling back to prompt-only JSON.",
                getattr(llm, "model_name", getattr(llm, "model", "unknown")),
            )
            return llm
        # Ollama expects a raw JSON schema in the "format" param.
        schema = None
        if isinstance(Schema, dict):
            json_schema = Schema.get("json_schema")
            if isinstance(json_schema, dict):
                schema = json_schema.get("schema")
        return llm.bind(format=schema or "json")

    # Handle LocalTransformersModel
    if LOCAL_MODEL_CLASS and isinstance(llm, LOCAL_MODEL_CLASS):
        logger.info("Using local model with structured output support")
        return llm.bind(response_format=Schema)

    # Handle LocalTransformersModelWrapper (from examples/main.py model sharing)
    # Check for the class-level flag instead of importing the class
    if getattr(llm, "_is_local_transformers_wrapper", False):
        logger.info("Using shared local model wrapper with structured output support")
        return llm.with_structured_output(Structured_Output)

    return llm


class Agent:
    def __init__(
        self,
        task: str,
        brain_llm: BaseChatModel,
        actor_llm: BaseChatModel,
        memory_llm: BaseChatModel,
        controller: Optional[Controller] = None,
        use_search: bool = True,
        use_skills: bool = False,
        skills_dir: Optional[str] = None,
        skills_max_chars: int = 4000,
        planner_llm: Optional[BaseChatModel] = None,
        save_planner_conversation_path: Optional[str] = None,
        save_planner_conversation_path_encoding: Optional[str] = "utf-8",
        save_brain_conversation_path: Optional[str] = None,
        save_brain_conversation_path_encoding: Optional[str] = "utf-8",
        save_actor_conversation_path: Optional[str] = None,
        save_actor_conversation_path_encoding: Optional[str] = "utf-8",
        artifacts_dir: Optional[str] = None,
        max_failures: int = 8,
        memory_budget: int = 500,
        summary_memory_budget: Optional[int] = None,
        retry_delay: int = 10,
        max_input_tokens: int = 32000,
        resume: bool = False,
        include_attributes: list[str] = [
            "title",
            "type",
            "name",
            "role",
            "tabindex",
            "aria-label",
            "placeholder",
            "value",
            "alt",
            "aria-expanded",
        ],
        max_error_length: int = 400,
        max_actions_per_step: int = 10,
        no_progress_max_steps: int = 6,
        web_navigation_switch_steps: int = 3,
        web_navigation_max_stalled_steps: int = 7,
        grounding_hotspot_failure_threshold: int = 2,
        human_assistance_failure_window: int = 18,
        human_assistance_failure_threshold: int = 14,
        human_assistance_guard_threshold: int = 6,
        human_assistance_obstacle_threshold: int = 3,
        human_assistance_min_steps: int = 10,
        human_assistance_min_strategies: int = 4,
        step_delay: float = 0.2,
        register_new_step_callback: Callable[["str", "AgentOutput", int], None] | None = None,
        register_done_callback: Callable[["AgentHistoryList"], None] | None = None,
        tool_calling_method: Optional[str] = "auto",
        agent_id: Optional[str] = None,
        status_window_enabled: bool = True,
        brain_include_previous_screenshot: bool = False,
        combined_step_enabled: bool = False,
        completion_verification_enabled: bool = True,
        completion_required_confirmations: int = 2,
        execution_journal_enabled: bool = True,
        run_id: Optional[str] = None,
        persist_run_state: bool = False,
        cleanup_images_on_finish: bool = True,
    ):
        global session_logger
        self.wait_this_step = False
        self.current_time = datetime.now()
        if resume:
            if not agent_id:
                raise ValueError("Agent ID is required for resuming a task.")
            if run_id and run_id != agent_id:
                raise ValueError("A resumed task must reuse its original Agent ID.")
            self.agent_id = agent_id
        else:
            # ``agent_id`` is only a readable prefix for a fresh run. Never use
            # it as the directory key directly, otherwise repeated/concurrent
            # launches overwrite screenshots, memory and execution reports.
            self.agent_id = run_id or create_run_id(
                task,
                now=self.current_time,
                base_id=agent_id,
            )
        self.task = task
        self.artifacts_dir = Path(artifacts_dir).expanduser().resolve() if artifacts_dir else None
        self.persist_run_state = bool(persist_run_state)
        self.cleanup_images_on_finish = bool(cleanup_images_on_finish)
        if self.artifacts_dir:
            self.images_dir = str(self.artifacts_dir / "images" / self.agent_id)
            self.save_temp_file_path = (
                str(self.artifacts_dir / "temp_files") if self.persist_run_state else None
            )
        else:
            self.images_dir = "images"
            self.save_temp_file_path = (
                os.path.join(os.path.dirname(__file__), "temp_files")
                if self.persist_run_state
                else None
            )
        self.original_task = task
        self.resume = resume
        self.status_window_enabled = status_window_enabled
        self._status_window = None
        self._step_log_path = None  # 兼容旧版外部注入的步骤文本日志路径；默认不再生成
        self._step_record_json_path: Optional[Path] = None
        self._training_dataset_dir: Optional[Path] = (
            self.artifacts_dir / "datasets" / "openkylin_cua" / self.agent_id
            if self.artifacts_dir
            else None
        )
        self.run_result_path: Optional[Path] = None
        self._step_records: list[dict[str, Any]] = []
        self._step_errors: dict[int, str] = {}
        self.assistance_request: Optional[dict[str, Any]] = None

        # 获取状态窗口实例
        if self.status_window_enabled:
            try:
                from src.utils.status_window import get_status_window
                self._status_window = get_status_window()
                if self._status_window:
                    logger.info("状态窗口已关联到 Agent")
            except Exception as e:
                logger.warning(f"获取状态窗口失败：{e}")
                self._status_window = None
        
        # 初始化会话日志器
        session_logger = get_session_logger()
        if session_logger:
            session_logger.session_start(
                task=task,
                config={
                    "agent_id": self.agent_id,
                    "max_steps": max_input_tokens,
                    "memory_budget": memory_budget,
                    "use_search": use_search,
                    "use_skills": use_skills,
                    "brain_include_previous_screenshot": brain_include_previous_screenshot,
                }
            )
        
        self._combined_provider_schema_enabled = bool(
            llm_supports_response_format(brain_llm)
        )
        self.memory_llm = to_structured(memory_llm, OutputSchemas.MEMORY_RESPONSE_FORMAT, MemoryOutput)
        self.brain_llm = to_structured(brain_llm, OutputSchemas.BRAIN_RESPONSE_FORMAT, BrainOutput)
        self.actor_llm = to_structured(actor_llm, OutputSchemas.ACTION_RESPONSE_FORMAT, ActorOutput)
        self.combined_step_enabled = bool(combined_step_enabled)
        self.combined_llm = (
            to_structured(brain_llm, OutputSchemas.COMBINED_RESPONSE_FORMAT, CombinedOutput)
            if self.combined_step_enabled
            else None
        )
        if self.combined_step_enabled:
            mode = (
                "provider_json_schema"
                if self._combined_provider_schema_enabled
                else "prompt_json_with_local_normalization"
            )
            logger.info("[Combined] Output protocol mode: %s", mode)
        self.planner_llm_raw = planner_llm
        self.planner_llm = to_structured(planner_llm, OutputSchemas.PLANNER_RESPONSE_FORMAT, PlannerOutput)
        self.token_counter = TokenCounter(brain_llm)
        self.memory_budget_tokens = max(1, int(memory_budget))
        default_summary_budget = max(1, self.memory_budget_tokens * 4)
        self.summary_memory_budget_tokens = max(
            1,
            int(summary_memory_budget) if summary_memory_budget is not None else default_summary_budget,
        )
        # Backward-compatible aliases used by older run artifacts/config assumptions.
        self.memory_budget = self.memory_budget_tokens
        self.summary_memory_budget = self.summary_memory_budget_tokens
        self.memory_warn_ratio = 0.7
        self.memory_hard_ratio = 1.0

        self.save_actor_conversation_path = save_actor_conversation_path
        self.save_actor_conversation_path_encoding = save_actor_conversation_path_encoding
        self.save_brain_conversation_path = save_brain_conversation_path
        self.save_brain_conversation_path_encoding = save_brain_conversation_path_encoding
        self.save_planner_conversation_path = save_planner_conversation_path
        self.save_planner_conversation_path_encoding = save_planner_conversation_path_encoding or "utf-8"

        self.include_attributes = include_attributes
        self.max_error_length = max_error_length
        self.screenshot_annotated = None
        self.max_input_tokens = max_input_tokens
        self.brain_include_previous_screenshot = brain_include_previous_screenshot
        self.completion_guard = CompletionGuard(
            enabled=completion_verification_enabled,
            required_confirmations=max(2, int(completion_required_confirmations)),
        )
        self.use_search = use_search
        self.use_skills = use_skills
        self.skills_dir = Path(skills_dir).expanduser() if skills_dir else None
        self.skills_max_chars = max(0, skills_max_chars or 0)
        self.available_skills = []
        self.selected_skills = []
        self.skill_context = ""
        self.next_goal = ""
        self.brain_thought = ""
        self._pending_text_focus: Optional[dict[str, Any]] = None
        self._pending_text_verification: Optional[dict[str, Any]] = None
        self._pending_content_dispatch_verification: Optional[dict[str, Any]] = None
        self._text_focus_review_passed: Optional[bool] = None
        self._text_result_review_passed: Optional[bool] = None
        self._text_result_review_step: Optional[int] = None
        self._content_dispatch_review_passed: Optional[bool] = None
        self._completion_prerequisite_block_reason: Optional[str] = None
        self._completion_prerequisite_block_count = 0
        self._completion_prerequisite_escape_threshold = 2
        self._post_action_observation_required = False
        self._deferred_assistance_request: Optional[dict[str, Any]] = None
        self.workflow_events: list[dict[str, Any]] = []
        self._last_action_guard_block: Optional[dict[str, Any]] = None

        self.controller = controller or Controller()
        self.max_actions_per_step = max(1, int(max_actions_per_step))
        self.no_progress_max_steps = max(1, int(no_progress_max_steps))
        self.web_navigation_switch_steps = max(1, int(web_navigation_switch_steps))
        self.web_navigation_max_stalled_steps = max(
            self.web_navigation_switch_steps,
            int(web_navigation_max_stalled_steps),
        )
        self.grounding_hotspot_failure_threshold = max(
            1,
            int(grounding_hotspot_failure_threshold),
        )
        self._no_progress_count = 0
        self._last_no_progress_signature: Optional[str] = None
        self._last_no_progress_strategy_signature: Optional[str] = None
        self._strategy_observation_pending: Optional[dict[str, Any]] = None
        self._stalled_failure_count = 0
        self._stalled_navigation_count = 0
        self._stalled_navigation_strategy_switch_requested = False
        self._stalled_navigation_block_count = 0
        self._stalled_navigation_strategies: set[str] = set()
        self._grounding_hotspot_failures: dict[str, dict[str, Any]] = {}
        self._recent_step_health = deque(maxlen=max(1, int(human_assistance_failure_window)))
        self.human_assistance_failure_threshold = max(
            1,
            min(int(human_assistance_failure_threshold), self._recent_step_health.maxlen),
        )
        self.human_assistance_guard_threshold = max(1, int(human_assistance_guard_threshold))
        self.human_assistance_obstacle_threshold = max(1, int(human_assistance_obstacle_threshold))
        self.human_assistance_min_steps = max(1, int(human_assistance_min_steps))
        self.human_assistance_min_strategies = max(1, int(human_assistance_min_strategies))
        self.step_delay = max(0.0, float(step_delay))
        self.last_step_action = None
        self._recent_action_signatures = deque(maxlen=8)
        self._recent_normalized_action_signatures = deque(maxlen=12)
        self._blocked_action_strategy_signatures = deque(maxlen=24)
        self._failed_action_retries: dict[str, int] = {}
        self._max_failed_action_retries = 2
        self.goal_action_memory = OrderedDict()

        self.last_goal = None
        self.brain_context = OrderedDict()
        self.status = "success"
        self._setup_action_models()
        # self._set_model_names()

        if self.save_temp_file_path:
            self.save_temp_file_path = os.path.join(self.save_temp_file_path, f"{self.agent_id}")
            self.record_dir = os.path.join(self.save_temp_file_path, "records")
            self.memory_snapshot_dir = os.path.join(self.save_temp_file_path, "memory_snapshots")
            journal_dir = Path(self.save_temp_file_path) / "execution"
        else:
            self.record_dir = None
            self.memory_snapshot_dir = None
            journal_dir = Path(os.devnull)
        self.record_store = RecordStore(
            self.record_dir,
            encoding=self.save_brain_conversation_path_encoding or "utf-8",
        )
        self.memory_snapshot_store = RecordStore(
            self.memory_snapshot_dir,
            encoding=self.save_brain_conversation_path_encoding or "utf-8",
        )
        self.brain_search = BrainSearchFlow(self.record_store)
        logger.info("Agent ID: %s", self.agent_id)
        if self.save_temp_file_path:
            logger.info("Agent memory path: %s", self.save_temp_file_path)
        else:
            logger.info("Agent persistent run state is disabled")
        self.execution_journal = ExecutionJournal(
            journal_dir,
            enabled=bool(execution_journal_enabled and self.save_temp_file_path),
        )
        self.task_obligations = self._derive_task_obligations()
        self.action_evidence: list[dict[str, Any]] = []

        if self.use_skills and self.skills_dir:
            self.available_skills = load_skill_metadata(self.skills_dir)
            if not self.available_skills:
                logger.info("No skills loaded from %s", self.skills_dir)
            else:
                skill_names = ", ".join(skill.name for skill in self.available_skills)
                logger.info(
                    "Loaded %d skill(s) from %s: %s",
                    len(self.available_skills),
                    self.skills_dir,
                    skill_names,
                )
        elif self.use_skills:
            logger.info("Skills enabled but no skills directory provided.")

        if self.planner_llm:
            skill_catalog = ""
            if self.use_skills and self.available_skills:
                skill_catalog = format_skill_catalog(self.available_skills)
            planner_preplan_llm = self.planner_llm_raw if (self.use_search or self.use_skills) else None
            self.planner = Planner(
                planner_llm=self.planner_llm,
                task=self.task,
                max_input_tokens=self.max_input_tokens,
                preplan_llm=planner_preplan_llm,
                use_search=self.use_search,
                skill_catalog=skill_catalog,
                use_skills=self.use_skills,
                available_skills=self.available_skills,
                skills_max_chars=self.skills_max_chars,
                save_planner_conversation_path=self.save_planner_conversation_path,
                save_planner_conversation_path_encoding=self.save_planner_conversation_path_encoding,
            )
        elif self.use_skills:
            logger.info("Skills enabled but planner is disabled. Set agent.use_plan=true to select skills.")

        self.initiate_messages()
        self._last_result = None

        self.register_new_step_callback = register_new_step_callback
        self.register_done_callback = register_done_callback

        self.history: AgentHistoryList = AgentHistoryList(history=[])
        self.n_steps = 1
        self.consecutive_failures = 0
        self.max_failures = max_failures
        self.retry_delay = retry_delay
        self._paused = False
        self._stopped = False
        self._stop_reason: Optional[str] = None
        self.brain_memory = ""
        self.summary_memory = ""
        self.recent_memory = ""
        # Pending step lines are finalized in the next brain step.
        self.pending_recent_memory = ""
        self.memory_snapshot_files: list[dict[str, Any]] = []
        self.infor_memory = []
        self.last_pid = None
        self.ask_for_help = False

    def _screenshot_path_for_step(self, step_id: int) -> Path:
        if self._training_dataset_dir:
            return self._training_dataset_dir / "screenshots" / f"step_{step_id:04d}.png"
        return Path(self.images_dir).expanduser() / f"screenshot_{step_id}.png"

    def _request_user_assistance(
        self,
        *,
        reason_code: str,
        reason: str,
        user_action_required: str,
        resume_condition: str,
        confidence: float = 0.8,
    ) -> None:
        """Pause this run with a structured, resumable human-assistance request."""
        request = {
            "reason_code": reason_code,
            "reason": reason,
            "user_action_required": user_action_required,
            "resume_condition": resume_condition,
            "confidence": confidence,
            "step": self.n_steps,
            "next_goal": self.next_goal,
        }
        if self._post_action_observation_required:
            self._deferred_assistance_request = request
            if getattr(self, "execution_journal", None):
                self.execution_journal.event(
                    "user_assistance_deferred_until_post_action_observation",
                    step=self.n_steps,
                    reason_code=reason_code,
                    reason=reason,
                    resume_condition=resume_condition,
                )
            self._add_log_window("动作后需要先截图复核，暂缓请求用户协助", "warning")
            return
        self.assistance_request = {
            "reason_code": reason_code,
            "reason": reason,
            "user_action_required": user_action_required,
            "resume_condition": resume_condition,
            "confidence": confidence,
            "step": self.n_steps,
            "next_goal": self.next_goal,
        }
        if getattr(self, "execution_journal", None):
            self.execution_journal.event(
                "user_assistance_requested",
                step=self.n_steps,
                reason_code=reason_code,
                reason=reason,
                resume_condition=resume_condition,
            )
        self._add_log_window(f"需要用户协助：{user_action_required}", "warning")
        self.stop(reason)

    def _complete_post_action_observation(self, *, step: int) -> bool:
        """Allow stop/assistance decisions only after Brain reviewed the post-action screen."""
        if not self._post_action_observation_required:
            return False

        self._post_action_observation_required = False
        pending = self._deferred_assistance_request
        self._deferred_assistance_request = None
        self.execution_journal.event(
            "post_action_observation_completed",
            step=step,
            evaluation=(
                self.current_state.get("step_evaluate", "")
                if isinstance(getattr(self, "current_state", None), dict)
                else ""
            ),
            deferred_assistance=bool(pending),
        )
        if not pending:
            return False

        if self._post_action_observation_clears_assistance(pending):
            self.execution_journal.event(
                "deferred_user_assistance_cancelled_after_observation",
                step=step,
                reason_code=pending.get("reason_code"),
            )
            return False

        refreshed_reason = (
            "动作后的最新截图已完成分析，任务仍未恢复。"
            f" 最新分析：{str(self.brain_thought or '')[:500]}"
        )
        self._request_user_assistance(
            reason_code=str(pending.get("reason_code") or "post_action_observed_block"),
            reason=refreshed_reason,
            user_action_required=str(pending.get("user_action_required") or ""),
            resume_condition=str(pending.get("resume_condition") or ""),
            confidence=float(pending.get("confidence") or 0.8),
        )
        return True

    def _post_action_observation_clears_assistance(self, pending: dict[str, Any]) -> bool:
        """Return True only when the newest screen actually resolves a deferred block."""
        if self.next_goal and "[TASK_COMPLETE]" in self.next_goal:
            return True
        if not self._latest_state_success():
            return False

        reason_code = str(pending.get("reason_code") or "")
        obstacle = self._manual_obstacle_reason()
        if obstacle:
            if obstacle[0] == "login_required" and self._task_contains_usable_login_credentials():
                return True
            return obstacle[0] != reason_code

        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        ask_human = str(self.current_state.get("ask_human", "") or "").strip()
        return not ask_human or ask_human.lower() in {"no", "none", "n/a", "否", "不需要"}

    def _maybe_request_user_assistance_from_state(self) -> bool:
        if self.assistance_request:
            return True
        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        ask_human = str(self.current_state.get("ask_human", "") or "").strip()
        if not ask_human or ask_human.lower() in {"no", "none", "n/a", "否", "不需要"}:
            return False
        credentials_available = self._task_contains_usable_login_credentials()
        obstacle = self._manual_obstacle_reason()
        if obstacle and obstacle[0] == "login_required" and credentials_available:
            self.execution_journal.event(
                "model_login_assistance_deferred_credentials_available",
                step=self.n_steps,
                ask_human=ask_human[:500],
            )
            return False
        login_decision_terms = () if credentials_available else ("登录", "登陆", "login", "sign in")
        needs_user_decision = any(
            term in ask_human
            for term in (
                "确认",
                "选择",
                "偏好",
                "提供",
                "输入验证码",
                "扫码",
                *login_decision_terms,
                "授权",
                "支付",
            )
        )
        if not obstacle and not needs_user_decision:
            self.execution_journal.event(
                "model_human_request_deferred",
                step=self.n_steps,
                ask_human=ask_human[:500],
            )
            return False
        reason = f"模型判断当前任务需要用户协助或确认：{ask_human}"
        self._request_user_assistance(
            reason_code=obstacle[0] if obstacle else "model_requested_human_assistance",
            reason=reason,
            user_action_required=ask_human,
            resume_condition=(
                "用户完成上述操作后，当前屏幕应处于可继续执行原任务的状态；"
                "再次启动 CUA 时需先检查屏幕是否满足该条件。"
            ),
            confidence=0.7,
        )
        return True

    def _manual_obstacle_reason(self) -> Optional[tuple[str, str]]:
        text = self._current_failure_text()
        obstacle_groups = (
            (
                "verification_required",
                ("验证码", "验证吗", "滑块", "拖动验证", "扫码验证", "人机验证", "安全验证", "captcha"),
            ),
            (
                "login_required",
                ("登录", "登陆", "login", "sign in", "账号", "密码"),
            ),
            (
                "permission_required",
                ("权限", "授权", "允许", "permission", "authorize", "access denied"),
            ),
            (
                "payment_or_confirmation_required",
                ("支付", "付款", "确认付款", "payment", "pay now"),
            ),
        )
        for reason_code, terms in obstacle_groups:
            if any(term in text for term in terms):
                if reason_code == "login_required" and self._task_contains_usable_login_credentials():
                    return None
                return reason_code, text[:500]
        return None

    def _task_contains_usable_login_credentials(self) -> bool:
        """Return True when the user's task already carries account login details."""
        text = f"{self.original_task}\n{self.task}"
        lowered = text.lower()
        has_password = bool(
            re.search(
                r"(?:密码|口令|password|passcode|pwd)\s*[:：=是为]\s*\S{3,}",
                text,
                flags=re.IGNORECASE,
            )
        )
        has_account = bool(
            re.search(
                r"(?:账号|账户|用户名|手机号|邮箱|email|e-mail|username|user|account|login)\s*[:：=是为]\s*\S{2,}",
                text,
                flags=re.IGNORECASE,
            )
            or re.search(r"[\w.+-]+@[\w-]+(?:\.[\w-]+)+", text)
            or re.search(r"(?<!\d)1[3-9]\d{9}(?!\d)", text)
        )
        has_credential_context = any(term in lowered for term in ("登录", "登陆", "login", "sign in"))
        return has_password and (has_account or has_credential_context)

    def _assistance_user_action_for_reason(self, reason_code: str) -> str:
        actions = {
            "verification_required": (
                "请用户手动完成当前页面的人机验证、滑块验证、扫码验证或验证码输入，"
                "直到验证弹窗消失并回到可继续操作的页面。"
            ),
            "login_required": (
                "请用户手动完成当前页面的登录、账号选择或密码/验证码输入，"
                "直到页面进入登录后的可操作状态。"
            ),
            "permission_required": (
                "请用户手动完成当前页面或系统弹窗中的授权、权限允许或访问确认，"
                "直到权限提示消失并回到可继续操作的页面。"
            ),
            "payment_or_confirmation_required": (
                "请用户手动完成付款、支付确认或高风险确认；完成后停留在结果页或可继续操作的页面。"
            ),
            "element_not_found": (
                "请用户手动把目标页面、目标控件或目标元素调整到屏幕可见位置；"
                "如果该元素不存在，请告知主智能体。"
            ),
            "web_navigation_stalled": (
                "请用户手动确认网络和浏览器是否可用，或手动打开目标网页到可继续操作的页面。"
            ),
        }
        return actions.get(
            reason_code,
            (
                "请用户手动处理当前阻塞点，或把界面调整到可以继续执行原任务的位置；"
                "如果是验证码、登录、授权、权限确认或找不到目标元素，请由用户手动完成。"
            ),
        )

    def _assistance_resume_condition_for_reason(self, reason_code: str) -> str:
        conditions = {
            "verification_required": (
                "验证弹窗/验证码/滑块/扫码验证已经消失，屏幕显示验证后的业务页面或可继续操作的目标页面。"
            ),
            "login_required": "登录流程已完成，屏幕显示登录后的页面或原任务需要继续操作的页面。",
            "permission_required": "授权或权限弹窗已处理完成，屏幕回到可继续执行原任务的页面。",
            "payment_or_confirmation_required": "用户已完成确认，屏幕显示明确结果页或可继续操作的页面。",
            "element_not_found": "原任务所需的目标控件、页面、列表项或结果已经在屏幕上可见。",
            "web_navigation_stalled": "目标网页已经成功加载，或屏幕上出现可继续执行原任务的目标页面。",
        }
        return conditions.get(
            reason_code,
            "当前屏幕不再显示上述阻塞状态，并且能看到原任务下一步需要操作的页面、控件或结果。",
        )

    def _record_step_health_and_maybe_request_assistance(
        self,
        *,
        step_id: int,
        action_error: bool,
        guard_blocked: bool,
        stalled_step: bool,
        no_visual_progress: bool,
        strategy_signature: Optional[str] = None,
        reason_hint: str = "",
    ) -> bool:
        failed_eval = self._previous_step_failed()
        failed_or_blocked = failed_eval or action_error or guard_blocked or stalled_step
        obstacle = self._manual_obstacle_reason() if failed_or_blocked else None
        self._recent_step_health.append(
            {
                "step": step_id,
                "failed_eval": failed_eval,
                "action_error": action_error,
                "guard_blocked": guard_blocked,
                "stalled_step": stalled_step,
                "no_visual_progress": no_visual_progress,
                "manual_obstacle": obstacle[0] if obstacle else None,
                "strategy": strategy_signature,
            }
        )

        failures = sum(
            1
            for item in self._recent_step_health
            if item["failed_eval"] or item["action_error"] or item["guard_blocked"] or item["stalled_step"]
        )
        guard_blocks = sum(1 for item in self._recent_step_health if item["guard_blocked"])
        obstacle_hits = sum(1 for item in self._recent_step_health if item["manual_obstacle"])
        distinct_strategies = {
            str(item.get("strategy"))
            for item in self._recent_step_health
            if item.get("strategy")
        }
        has_enough_attempts = (
            len(self._recent_step_health) >= self.human_assistance_min_steps
            and len(distinct_strategies) >= self.human_assistance_min_strategies
        )

        reason_code = ""
        reason = ""
        if failed_or_blocked and obstacle and obstacle_hits >= self.human_assistance_obstacle_threshold:
            reason_code = obstacle[0]
            reason = (
                "当前界面疑似存在需要人工处理的验证、登录、授权或确认障碍，"
                f"最近 {len(self._recent_step_health)} 步中已有 {obstacle_hits} 步命中该类障碍。"
            )
        elif guard_blocks >= self.human_assistance_guard_threshold and has_enough_attempts:
            reason_code = "repeated_guard_blocks"
            reason = (
                "多个不同自动化策略已被安全/无进展 guard 拦截，继续尝试容易重复无效或误操作。"
                f"最近 {len(self._recent_step_health)} 步中有 {guard_blocks} 步被拦截，"
                f"已尝试 {len(distinct_strategies)} 种不同策略。"
            )
        elif failures >= self.human_assistance_failure_threshold and has_enough_attempts:
            reason_code = "high_failure_density"
            reason = (
                "任务虽然尝试了不同策略，但整体失败密度过高，说明当前界面状态不适合继续自动执行。"
                f"最近 {len(self._recent_step_health)} 步中有 {failures} 步失败或无有效进展，"
                f"已尝试 {len(distinct_strategies)} 种不同策略。"
            )

        if not reason_code:
            return False

        detail = reason_hint or self._current_failure_text()[:500]
        if detail:
            reason = f"{reason} 最新线索：{detail}"
        self._request_user_assistance(
            reason_code=reason_code,
            reason=reason,
            user_action_required=self._assistance_user_action_for_reason(reason_code),
            resume_condition=self._assistance_resume_condition_for_reason(reason_code),
            confidence=0.78,
        )
        return True

    def _has_enough_assistance_attempts(self) -> bool:
        distinct_strategies = {
            str(item.get("strategy"))
            for item in self._recent_step_health
            if item.get("strategy")
        }
        return (
            len(self._recent_step_health) >= self.human_assistance_min_steps
            and len(distinct_strategies) >= self.human_assistance_min_strategies
        )

    def _write_run_result(
        self,
        *,
        status: str,
        reason: str,
        total_run_time: float,
        max_steps: Optional[int] = None,
        final_evidence: Optional[str] = None,
    ) -> None:
        if not self.run_result_path:
            return
        assistance = self.assistance_request if isinstance(self.assistance_request, dict) else None
        requires_user_assistance = status == "need_user_assistance" or bool(assistance)
        user_action_required = str(assistance.get("user_action_required", "") or "") if assistance else ""
        resume_condition = str(assistance.get("resume_condition", "") or "") if assistance else ""
        reason_code = str(assistance.get("reason_code", "") or "") if assistance else ""
        current_state = self.current_state if isinstance(getattr(self, "current_state", None), dict) else {}
        payload = {
            "status": status,
            "reason": reason,
            "reason_code": reason_code,
            "task": self.original_task,
            "agent_id": getattr(self, "agent_id", None),
            "step": self.n_steps,
            "max_steps": max_steps,
            "duration_seconds": round(max(0.0, total_run_time), 3),
            "final_evidence": final_evidence,
            "final_step_evaluate": str(current_state.get("step_evaluate", "") or ""),
            "final_analysis": str(self.brain_thought or ""),
            "final_next_goal": str(current_state.get("next_goal", self.next_goal) or ""),
            "requires_user_assistance": requires_user_assistance,
            "user_action_required": user_action_required,
            "resume_condition": resume_condition,
            "next_action_for_main_agent": (
                "ask_user_then_resume"
                if requires_user_assistance
                else "report_status"
            ),
            "assistance_request": assistance,
            "resume_instruction": None,
        }
        if requires_user_assistance:
            payload["resume_instruction"] = (
                "主智能体反馈前必须先读取本 result.json，并在 final_evidence 存在且可读时查看该最终截图，"
                "结合截图校验 reason、user_action_required 和 resume_condition 是否与当前屏幕一致；"
                "若文本字段与截图冲突，以最终截图为更高优先级证据说明真实阻塞点。"
                "然后把 top-level user_action_required 原样返回给用户，等待用户明确确认完成后才能再次启动 CUA。"
                "再次启动 CUA 时，任务文本必须带上原始任务、已完成摘要、上次阻塞原因和 resume_condition，"
                "并要求 CUA 先验证恢复条件再继续。"
            )
        elif status != "success":
            payload["resume_instruction"] = (
                "主智能体反馈失败、超时或未完成状态前必须先读取本 result.json，"
                "并在 final_evidence 存在且可读时查看该最终截图；"
                "结合截图内容校验 reason 是否准确，若文本原因与截图冲突，以最终截图为更高优先级证据。"
            )
        try:
            self.run_result_path.parent.mkdir(parents=True, exist_ok=True)
            tmp_path = self.run_result_path.with_suffix(".json.tmp")
            with open(tmp_path, "w", encoding="utf-8") as f:
                json.dump(payload, f, indent=2, ensure_ascii=False, default=str)
            os.replace(tmp_path, self.run_result_path)
            logger.info("Run result written: %s", self.run_result_path)
        except Exception as e:
            logger.warning("写入运行结果失败：%s", e)

    async def _run_final_diagnosis(self, *, status: str, reason: str) -> str:
        """Use the final screenshot to produce the non-success result reported upstream."""
        if status == "success" or self.screenshot_annotated is None:
            return reason

        prompt = (
            "FINAL DIAGNOSIS ONLY. You are diagnosing why a GUI automation task did not finish. "
            "Do not propose or execute any further desktop action. Inspect the attached final screenshot "
            "and the task context, then return ONLY valid JSON using the normal Brain schema. "
            "Set current_state.step_evaluate to Failed unless the screenshot clearly proves the original task is complete. "
            "Set current_state.ask_human to No unless the final screenshot shows a real user-only blocker "
            "such as captcha, QR scan, missing credential, authorization, permission dialog, payment, or a choice only the user can make. "
            "Set current_state.next_goal to a concise Chinese final diagnosis containing the visible evidence and what should happen next. "
            "If user assistance is needed, say exactly what the user must do in ask_human."
        )
        state = (
            f"Original task: {self.original_task}\n"
            f"Current non-success status: {status}\n"
            f"Prior stop reason: {reason}\n"
            f"Last goal: {self.next_goal}\n"
            f"Last Brain analysis: {self.brain_thought}\n"
            f"Recent operations:\n{self._recent_operations_summary(max_items=10)}"
        )
        messages = [
            HumanMessage(
                content=[
                    {"type": "text", "text": f"{prompt}\n\n{state}"},
                    {
                        "type": "image_url",
                        "image_url": {"url": screenshot_to_dataurl(self.screenshot_annotated)},
                    },
                ]
            )
        ]
        try:
            response = await self.brain_llm.ainvoke(messages)
            if hasattr(response, "parsed") and response.parsed is not None:
                parsed = response.parsed
                if hasattr(parsed, "model_dump"):
                    parsed = parsed.model_dump(exclude_unset=True)
            else:
                parsed = self.brain_search.parse_response(str(getattr(response, "content", response)), label="Final diagnosis")
            if not isinstance(parsed, dict):
                raise ValueError("Final diagnosis response is not a dict.")
            analysis = parsed.get("analysis")
            current_state = parsed.get("current_state")
            if not isinstance(current_state, dict):
                raise ValueError("Final diagnosis response missing current_state.")
            if isinstance(analysis, dict):
                self.brain_thought = str(
                    analysis.get("analysis")
                    or analysis.get("sop_check")
                    or json.dumps(analysis, ensure_ascii=False)
                )
            self.current_state = current_state
            self.next_goal = str(current_state.get("next_goal", self.next_goal) or "")

            ask_human = str(current_state.get("ask_human", "") or "").strip()
            if ask_human and ask_human.lower() not in {"no", "none", "n/a", "否", "不需要"}:
                reason_code = self._manual_obstacle_reason()
                self.assistance_request = {
                    "reason_code": reason_code[0] if reason_code else "final_diagnosis_user_assistance",
                    "reason": self.next_goal or reason,
                    "user_action_required": ask_human,
                    "resume_condition": (
                        "用户完成上述操作后，屏幕应显示可继续执行原任务的页面、控件或结果。"
                    ),
                    "confidence": 0.82,
                    "step": self.n_steps,
                    "next_goal": self.next_goal,
                }
            else:
                self.assistance_request = None

            final_reason = self.next_goal or reason
            self.execution_journal.event(
                "final_diagnosis_completed",
                step=self.n_steps,
                status=status,
                reason=final_reason[:500],
            )
            return final_reason
        except Exception as e:
            logger.warning("Final diagnosis failed; keeping prior stop reason: %s", e)
            self.execution_journal.event(
                "final_diagnosis_failed",
                step=self.n_steps,
                status=status,
                error=str(e)[:500],
            )
            return reason

    def _set_model_names(self) -> None:
        self.chat_model_library = self.llm.__class__.__name__
        if hasattr(self.llm, "model_name"):
            self.model_name = self.llm.model_name  # type: ignore
        elif hasattr(self.llm, "model"):
            self.model_name = self.llm.model  # type: ignore
        else:
            self.model_name = "Unknown"

    def set_tool_calling_method(self, tool_calling_method: Optional[str]) -> Optional[str]:
        if tool_calling_method == "auto":
            if self.chat_model_library == "ChatGoogleGenerativeAI":
                return None
            if self.chat_model_library == "ChatOpenAI":
                return "function_calling"
            if self.chat_model_library == "AzureChatOpenAI":
                return "function_calling"
            return None

    def _setup_action_models(self) -> None:
        """Setup dynamic action models from controller's registry"""
        self.ActionModel = self.controller.registry.create_action_model()
        self.AgentOutput = AgentOutput.type_with_custom_actions(self.ActionModel)

    def get_last_pid(self) -> Optional[int]:
        latest_pid = self.last_pid
        if self._last_result:
            for r in self._last_result:
                if r.current_app_pid:
                    latest_pid = r.current_app_pid
        return latest_pid

    def _recent_operations_summary(self, max_items: int = 8) -> str:
        """Concise log of recently executed operations (goal -> action -> result).

        Fed to the models so that, when deciding the next goal/action, they can see
        what has already been attempted and avoid repeating a no-progress operation.
        """
        lines: list[str] = []
        for item in self.history.history[-max_items:]:
            step_id = int(getattr(item, "step_id", None) or 0)
            goal = str(getattr(item, "next_goal", "") or "").strip()
            goal_short = goal[:120] + ("..." if len(goal) > 120 else "")
            actions: list[str] = []
            model_output = getattr(item, "model_output", None)
            for action in getattr(model_output, "action", []) or []:
                action_name, params = self._action_name_and_params(action)
                if not action_name:
                    continue
                actions.append(self._compact_action_description(action_name, params))
            actions_text = ", ".join(actions) if actions else "-"
            status = self._history_result_status(item)
            lines.append(
                f"Step {step_id} | 目标: {goal_short} | 动作: {actions_text} | 结果: {status}"
            )
        return "\n".join(lines)

    @staticmethod
    def _compact_action_description(action_name: str, params: Any) -> str:
        if not isinstance(params, dict):
            return action_name
        if action_name in {"Click", "DoubleClick", "RightSingle", "move_mouse"}:
            position = params.get("position")
            if isinstance(position, (list, tuple)) and len(position) == 2:
                return f"{action_name}[{int(position[0])},{int(position[1])}]"
            return action_name
        if action_name == "input_text":
            text = str(params.get("text", ""))
            return f"input_text({text[:40]!r})"
        if action_name == "wait":
            return f"wait({params.get('seconds')})"
        if action_name in {"Hotkey", "multi_Hotkey"}:
            keys = [
                str(params.get(key))
                for key in ("key", "key1", "key2", "key3")
                if params.get(key) is not None
            ]
            return f"{action_name}({','.join(keys)})" if keys else action_name
        return action_name

    @staticmethod
    def _history_result_status(item: Any) -> str:
        results = getattr(item, "result", None) or []
        if not results:
            return "?"
        if any(getattr(r, "is_done", False) for r in results):
            return "done"
        if any(getattr(r, "error", None) for r in results):
            return "error"
        return "ok"

    def _refresh_brain_memory(self) -> None:
        parts = []
        operations = self._recent_operations_summary()
        if operations:
            parts.append("Recently executed operations:\n" + operations)
        workflow_memory = self._workflow_memory()
        if workflow_memory:
            parts.append("Workflow state:\n" + workflow_memory)
        if self.summary_memory:
            parts.append(
                "Summarized memory (compressed from earlier steps, details may be approximate):\n"
                + self.summary_memory
            )
        if self.recent_memory:
            parts.append("Recent steps:\n" + self.recent_memory)
        if self.pending_recent_memory:
            parts.append("Pending steps (not yet evaluated):\n" + self.pending_recent_memory)
        self.brain_memory = "\n\n".join(parts).strip()

    def _workflow_memory(self) -> str:
        notes = []
        completed_events = [
            item
            for item in self.workflow_events
            if item.get("status") == "completed" and item.get("goal")
        ][-8:]
        if completed_events:
            lines = []
            for item in completed_events:
                step = item.get("step", "?")
                goal = str(item.get("goal", "")).strip()
                if goal:
                    lines.append(f"- Step {step} completed: {goal[:220]}")
            if lines:
                notes.append(
                    "Generic workflow progress already completed; do not redo the same stage unless the newest screenshot clearly shows the state regressed:\n"
                    + "\n".join(lines)
                )
        attempted_events = [
            item
            for item in self.workflow_events
            if item.get("status") == "attempted" and item.get("goal")
        ][-5:]
        if attempted_events:
            lines = []
            for item in attempted_events:
                step = item.get("step", "?")
                goal = str(item.get("goal", "")).strip()
                if goal:
                    lines.append(f"- Step {step} attempted: {goal[:180]}")
            if lines:
                notes.append(
                    "Recent workflow attempts; use them to avoid no-progress repetition:\n"
                    + "\n".join(lines)
                )
        if self._last_action_guard_block:
            block = self._last_action_guard_block
            notes.append(
                "RECOVERY REQUIRED FROM BLOCKED LOOP:\n"
                f"- Blocked at step {block.get('step', '?')}: {block.get('reason', '')}\n"
                f"- Blocked goal: {str(block.get('goal', ''))[:220]}\n"
                "- The next action must be materially different from the blocked action. "
                "Inspect the current screen, choose another visible path/control, wait for new evidence, navigate back/refresh/search/refocus when appropriate, or ask the user if no safe route is visible."
            )
        stalled_navigation_count = int(getattr(self, "_stalled_navigation_count", 0) or 0)
        web_navigation_switch_steps = int(getattr(self, "web_navigation_switch_steps", 3) or 3)
        if stalled_navigation_count >= web_navigation_switch_steps:
            stalled_navigation_strategies = getattr(self, "_stalled_navigation_strategies", set())
            notes.append(
                "WEB NAVIGATION STALL DETECTED:\n"
                f"- Stalled attempts: {stalled_navigation_count}/{getattr(self, 'web_navigation_max_stalled_steps', 7)}\n"
                f"- Distinct strategies tried: {len(stalled_navigation_strategies)}\n"
                "- Do not keep using the same browser page/search box/search button. "
                "Use a materially different route now, preferably another installed browser or a different confirmed web entry. "
                "If a different route also fails to load, stop and report the web access failure reason."
            )
        blocked_action_strategy_signatures = getattr(
            self,
            "_blocked_action_strategy_signatures",
            [],
        )
        if blocked_action_strategy_signatures:
            notes.append(
                "BLOCKED ACTION STRATEGIES FOR THE CURRENT UNCHANGED SCREEN:\n"
                + "\n".join(f"- {item[:220]}" for item in list(blocked_action_strategy_signatures)[-8:])
                + "\nDo not reuse any of these action strategies while the screen has not visibly changed. Pick a different action family, target area, or interaction route."
            )
        hotspot_items = [
            item
            for item in getattr(self, "_grounding_hotspot_failures", {}).values()
            if int(item.get("failures", 0) or 0) >= getattr(self, "grounding_hotspot_failure_threshold", 2)
        ][-5:]
        if hotspot_items:
            lines = []
            for item in hotspot_items:
                lines.append(
                    f"- Step {item.get('last_failed_step', '?')} failed at {item.get('description', 'unknown position')}: "
                    f"{str(item.get('goal', ''))[:180]}"
                )
            notes.append(
                "GROUNDING CLICK POINTS TO AVOID:\n"
                + "\n".join(lines)
                + "\nDo not click these same coordinates again for the same intent. A nearby but visually distinct target is allowed when the newest screenshot supports it; otherwise re-ground using another visible target, scrolling, keyboard/direct input, or ask the user if no reliable target is visible."
            )
        if self._has_verified_wechat_group_evidence("wechat_group_creation_started"):
            notes.append(
                "微信建群入口已经使用过：已经从微信主界面上方 `+` / “发起群聊” / “新建群聊”进入建群流程。"
                "后续不要重新点击 `+` 创建第二个群；应继续在当前群聊/成员选择界面添加成员、确认建群，或执行用户初始任务要求的后续操作。"
            )
        if self._has_verified_wechat_group_evidence("wechat_group_chat_ready"):
            notes.append(
                "微信群聊已经创建或当前已处于新群聊天窗口。后续只需在当前群聊中执行用户初始任务要求的后续操作；禁止再次创建群聊。"
            )
        return "\n".join(notes)

    def _record_workflow_event(
        self,
        status: str,
        *,
        step: int,
        goal: Optional[str],
        details: Optional[dict[str, Any]] = None,
    ) -> None:
        goal_text = str(goal or "").strip()
        if not goal_text:
            return
        signature = self._normalized_goal_signature(goal_text)
        for item in reversed(self.workflow_events[-20:]):
            if item.get("status") == status and item.get("goal_signature") == signature:
                return
        record = {
            "status": status,
            "step": step,
            "goal": goal_text,
            "goal_signature": signature,
            "details": details or {},
            "time": time.time(),
        }
        self.workflow_events.append(record)
        if len(self.workflow_events) > 80:
            self.workflow_events = self.workflow_events[-80:]
        if status == "completed":
            self._record_verified_wechat_group_progress(goal_text, step=step)
        self.execution_journal.event("workflow_event", **record)
        self._refresh_brain_memory()

    def _record_action_guard_block(
        self,
        *,
        step: int,
        reason: str,
        actions: list[Any],
        goal: Optional[str] = None,
    ) -> None:
        goal_text = goal if goal is not None else self.next_goal
        self._last_action_guard_block = {
            "step": step,
            "reason": reason,
            "goal": goal_text,
            "action_signature": self._normalized_action_batch_signature(actions),
            "strategy_signature": self._action_strategy_batch_signature(actions),
            "time": time.time(),
        }
        strategy_signature = self._last_action_guard_block.get("strategy_signature")
        if strategy_signature:
            self._blocked_action_strategy_signatures.append(str(strategy_signature))
        self._record_workflow_event(
            "blocked",
            step=step,
            goal=goal_text,
            details={"reason": reason},
        )
        self.execution_journal.event(
            "action_guard_blocked_loop",
            step=step,
            reason=reason,
            goal=goal_text,
            action_signature=self._last_action_guard_block.get("action_signature"),
        )

    def _build_recovery_goal(self, current_goal: str, reason: str) -> str:
        if current_goal and current_goal.startswith("恢复执行"):
            # Already a recovery goal from an earlier block; do not nest another
            # wrapper, otherwise the goal text keeps growing and re-triggers loop
            # detection on the embedded original goal.
            return current_goal
        return (
            "恢复执行：刚才的目标或动作已经形成重复/无进展循环。"
            f"不要再次执行同一目标或同一区域动作。原目标：{current_goal}。"
            f"拦截原因：{reason}。"
            "请先检查当前屏幕结果，再选择实质不同的方法推进：换一个入口/控件、返回或刷新、使用搜索/地址栏、重新聚焦明确字段、等待新证据，或在没有安全路径时询问用户。"
        )

    @property
    def total_memory_tokens(self) -> int:
        return (
            self.token_counter.count(self.summary_memory)
            + self.token_counter.count(self.recent_memory)
            + self.token_counter.count(self.pending_recent_memory)
        )

    def _log_memory_metrics(self) -> None:
        logger.info(
            "[Memory] Step %d | recent=%d tokens | summary=%d tokens | pending=%d tokens | total=%d/%d tokens | info_files=%d",
            self.n_steps,
            self.token_counter.count(self.recent_memory),
            self.token_counter.count(self.summary_memory),
            self.token_counter.count(self.pending_recent_memory),
            self.total_memory_tokens,
            self.memory_budget_tokens + self.summary_memory_budget_tokens,
            len(self.infor_memory),
        )

    def _action_signature(self, action: Any) -> Optional[str]:
        if action is None:
            return None
        try:
            action_dict = action.model_dump(exclude_unset=True) if hasattr(action, "model_dump") else action
            return json.dumps(action_dict, ensure_ascii=False, sort_keys=True)
        except Exception:
            return str(action)

    def _normalized_goal_signature(self, goal: str) -> str:
        text = str(goal or "").lower()
        text = re.sub(
            r"(try a different approach|consider an alternative method|look for other ways to complete this task|check if the previous action had the expected effect)",
            "",
            text,
        )
        text = re.sub(r"恢复执行：.*?原目标[:：]", "", text, flags=re.DOTALL)
        text = re.sub(r"拦截原因[:：].*", "", text, flags=re.DOTALL)
        text = re.sub(r"\s+", "", text)
        return text[:160]

    @staticmethod
    def _bucket_number(value: Any, size: int = 64) -> Optional[int]:
        try:
            return int(round(float(value) / size) * size)
        except (TypeError, ValueError):
            return None

    def _normalized_action_signature(self, action: Any) -> Optional[str]:
        action_name, params = self._action_name_and_params(action)
        if not action_name:
            return None
        goal_sig = self._normalized_goal_signature(self.next_goal)
        if action_name in {"Click", "DoubleClick", "RightSingle", "MoveTo"} and isinstance(params, dict):
            position = params.get("position")
            if isinstance(position, list) and len(position) == 2:
                x = self._bucket_number(position[0])
                y = self._bucket_number(position[1])
                return f"{action_name}:pos:{x},{y}:goal:{goal_sig}"
        if action_name == "input_text" and isinstance(params, dict):
            text = str(params.get("text", ""))
            press_enter = bool(params.get("press_enter", False))
            select_all = bool(params.get("select_all", True))
            return f"input_text:{len(text)}:{text[:32]}:enter:{press_enter}:select_all:{select_all}:goal:{goal_sig}"
        if action_name in {"Hotkey", "multi_Hotkey", "Press"} and isinstance(params, dict):
            keys = ",".join(f"{key}={params.get(key)}" for key in sorted(params))
            return f"{action_name}:{keys}:goal:{goal_sig}"
        if action_name == "wait":
            return f"wait:goal:{goal_sig}"
        return f"{action_name}:goal:{goal_sig}"

    def _normalized_action_batch_signature(self, actions: list[Any]) -> Optional[str]:
        signatures = [
            signature
            for action in actions or []
            if (signature := self._normalized_action_signature(action))
        ]
        if not signatures:
            return None
        return "|".join(signatures)

    def _action_strategy_signature(self, action: Any) -> Optional[str]:
        action_name, params = self._action_name_and_params(action)
        if not action_name:
            return None
        target_sig = self._goal_target_signature(self.next_goal)
        target_suffix = f":target:{target_sig}" if target_sig else ""
        if action_name in {"Click", "DoubleClick", "RightSingle", "MoveTo"} and isinstance(params, dict):
            position = params.get("position")
            if isinstance(position, list) and len(position) == 2:
                x = self._bucket_number(position[0])
                y = self._bucket_number(position[1])
                return f"{action_name}:pos:{x},{y}{target_suffix}"
            region = params.get("region")
            if isinstance(region, list) and len(region) == 4:
                buckets = [self._bucket_number(value) for value in region]
                return f"{action_name}:region:{buckets}{target_suffix}"
        if action_name == "input_text" and isinstance(params, dict):
            text = str(params.get("text", ""))
            press_enter = bool(params.get("press_enter", False))
            select_all = bool(params.get("select_all", True))
            return f"input_text:{len(text)}:{text[:32]}:enter:{press_enter}:select_all:{select_all}{target_suffix}"
        if action_name in {"Hotkey", "multi_Hotkey", "Press"} and isinstance(params, dict):
            keys = ",".join(f"{key}={params.get(key)}" for key in sorted(params))
            return f"{action_name}:{keys}{target_suffix}"
        if action_name == "open_app" and isinstance(params, dict):
            return f"open_app:{self._normalized_goal_signature(str(params.get('app_name', '')))}"
        if action_name == "wait":
            seconds = params.get("seconds") if isinstance(params, dict) else None
            return f"wait:{seconds}"
        return f"{action_name}{target_suffix}"

    def _action_strategy_batch_signature(self, actions: list[Any]) -> Optional[str]:
        signatures = [
            signature
            for action in actions or []
            if (signature := self._action_strategy_signature(action))
        ]
        if not signatures:
            return None
        return "|".join(signatures)

    def _grounding_intent_signature(self, goal: Optional[str]) -> str:
        text = str(goal or "").lower()
        compact = re.sub(r"\s+", "", text)
        intent_terms = (
            ("calendar-date", ("date", "calendar", "depart", "return", "日期", "日历", "出发", "返程", "往返", "号")),
            ("field-focus", ("input", "field", "textbox", "搜索框", "输入框", "字段", "填写", "输入")),
            ("selection", ("select", "choose", "pick", "选择", "选中", "点击", "打开")),
            ("submit", ("submit", "send", "search", "confirm", "提交", "发送", "搜索", "确认", "完成")),
            ("navigation", ("back", "forward", "refresh", "tab", "返回", "前进", "刷新", "标签页")),
        )
        for intent, terms in intent_terms:
            if any(term in text or term in compact for term in terms):
                target = self._goal_target_signature(goal)
                return f"{intent}:{target}" if target else intent
        normalized = re.sub(r"\d+", "#", compact)
        return normalized[:80] or "generic"

    def _goal_target_signature(self, goal: Optional[str]) -> str:
        """Extract the current object being acted on for repeat-guard keys.

        Many apps reuse the same coordinates for different list results after a
        search or filter changes. A click at the same row should be considered a
        different attempt when the goal names a different contact, file, item, or
        other object.
        """
        text = str(goal or "").strip()
        if not text:
            return ""
        text = re.sub(r"恢复执行：.*?原目标[:：]", "", text, flags=re.DOTALL)
        text = re.sub(r"拦截原因[:：].*", "", text, flags=re.DOTALL)

        # Prefer explicit quoted values because goals often carry exact user
        # inputs or item names there.
        quoted = re.findall(r"['\"“”‘’]([^'\"“”‘’]{1,40})['\"“”‘’]", text)
        for value in quoted:
            value = re.sub(r"\s+", "", value)
            if len(value) >= 2:
                return value[:40].lower()

        object_patterns = (
            r"输入(.{1,40}?)(?:搜索|查找|筛选|$)",
            r"搜索(.{1,40}?)(?:并|后|，|,|$)",
            r"查找(.{1,40}?)(?:并|后|，|,|$)",
            r"勾选(.{1,40}?)(?:联系人|成员|文件|项目|条目|结果|$)",
            r"选择(.{1,40}?)(?:联系人|成员|文件|项目|条目|结果|$)",
            r"选中(.{1,40}?)(?:联系人|成员|文件|项目|条目|结果|$)",
            r"中的(.{1,40}?)(?:联系人|成员|文件|项目|条目|结果|$)",
        )
        cleanup_terms = (
            "搜索结果",
            "搜索框",
            "联系人",
            "成员",
            "文件",
            "项目",
            "条目",
            "结果",
            "中的",
            "在",
            "中",
            "然后",
            "并",
            "他",
            "她",
            "它",
        )
        for pattern in object_patterns:
            match = re.search(pattern, text)
            if not match:
                continue
            value = re.sub(r"\s+", "", match.group(1))
            for term in cleanup_terms:
                value = value.replace(term, "")
            if len(value) >= 2:
                return value[:40].lower()

        compact = re.sub(r"\s+", "", text)
        stop_words = {
            "搜索",
            "勾选",
            "选择",
            "选中",
            "联系人",
            "成员",
            "文件",
            "结果",
            "点击",
            "完成",
            "创建",
            "添加",
            "发送",
            "输入框",
            "搜索框",
            "search",
            "select",
            "choose",
            "click",
            "contact",
            "member",
            "file",
            "item",
        }
        candidates: list[str] = []
        candidates.extend(re.findall(r"[\u4e00-\u9fffA-Za-z0-9_.-]{2,40}", compact))
        for candidate in candidates:
            value = candidate.strip()
            if not value:
                continue
            if value.lower() in stop_words:
                continue
            if any(word in value for word in stop_words):
                for word in stop_words:
                    value = value.replace(word, "")
            if len(value) >= 2:
                return value[:40].lower()
        return ""

    def _click_hotspot_key(self, action: Any, goal: Optional[str] = None) -> Optional[str]:
        action_name, params = self._action_name_and_params(action)
        if action_name not in {"Click", "DoubleClick"} or not isinstance(params, dict):
            return None
        position = params.get("position")
        if not isinstance(position, list) or len(position) != 2:
            return None
        # Keep a small tolerance for coordinate jitter, but do not blacklist the
        # surrounding UI region; a valid target may be close to the failed point.
        x = self._bucket_number(position[0], size=8)
        y = self._bucket_number(position[1], size=8)
        if x is None or y is None:
            return None
        intent = self._grounding_intent_signature(goal if goal is not None else self.next_goal)
        return f"{action_name}:hotspot:{x},{y}:intent:{intent}"

    def _click_hotspot_description(self, action: Any) -> str:
        _, params = self._action_name_and_params(action)
        if isinstance(params, dict):
            position = params.get("position")
            if isinstance(position, list) and len(position) == 2:
                return f"position={position}"
        return "unknown position"

    def _history_item_for_step(self, step_id: int) -> Optional[AgentHistory]:
        for item in reversed(getattr(self.history, "history", []) or []):
            if getattr(item, "step_id", None) == step_id:
                return item
        return None

    def _record_grounding_feedback(
        self,
        step_id: int,
        final_status: str,
        goal: Optional[str],
    ) -> None:
        if final_status not in {"success", "failed"}:
            return
        history_item = self._history_item_for_step(step_id)
        model_output = getattr(history_item, "model_output", None) if history_item else None
        actions = getattr(model_output, "action", None) or []
        if not actions:
            return

        changed = False
        for action in actions:
            key = self._click_hotspot_key(action, goal)
            if not key:
                continue
            if final_status == "success":
                if key in self._grounding_hotspot_failures:
                    self._grounding_hotspot_failures.pop(key, None)
                    changed = True
                continue

            record = self._grounding_hotspot_failures.get(key, {})
            failures = int(record.get("failures", 0) or 0) + 1
            self._grounding_hotspot_failures[key] = {
                "failures": failures,
                "last_failed_step": step_id,
                "goal": str(goal or "")[:240],
                "description": self._click_hotspot_description(action),
                "time": time.time(),
            }
            changed = True
            self.execution_journal.event(
                "grounding_hotspot_failed",
                step=step_id,
                key=key,
                failures=failures,
                goal=goal,
            )

        if len(self._grounding_hotspot_failures) > 64:
            newest = sorted(
                self._grounding_hotspot_failures.items(),
                key=lambda item: float(item[1].get("time", 0) or 0),
            )[-64:]
            self._grounding_hotspot_failures = dict(newest)
        if changed:
            self._refresh_brain_memory()

    def _check_grounding_hotspot_failures(self, actions: list[Any]) -> Optional[str]:
        if self._screens_differ() and self._grounding_hotspot_failures:
            self._grounding_hotspot_failures.clear()
            self._refresh_brain_memory()
            return None
        threshold = max(1, int(getattr(self, "grounding_hotspot_failure_threshold", 2)))
        for action in actions or []:
            key = self._click_hotspot_key(action)
            if not key:
                continue
            record = self._grounding_hotspot_failures.get(key)
            if not record:
                continue
            failures = int(record.get("failures", 0) or 0)
            if failures < threshold:
                continue
            description = record.get("description") or self._click_hotspot_description(action)
            return (
                f"Grounding hotspot blocked after {failures} failed visual evaluations "
                f"for the same click point ({description}). Do not click this same "
                "coordinate again for the same intent. A nearby but visually distinct "
                "target is allowed when the newest screenshot supports it; otherwise "
                "re-inspect the screen and choose a materially different grounding "
                "strategy: click a different visible control, scroll the target panel "
                "after moving the mouse into it, use direct input/keyboard navigation, "
                "or ask for help if no reliable target is visible."
            )
        return None

    def _previous_step_failed(self) -> bool:
        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        evaluation = str(self.current_state.get("step_evaluate", "")).lower()
        return "fail" in evaluation or "失败" in evaluation or "error" in evaluation

    def _current_failure_text(self) -> str:
        if not isinstance(getattr(self, "current_state", None), dict):
            return ""
        parts = [
            str(self.current_state.get("step_evaluate", "") or ""),
            str(self.current_state.get("next_goal", "") or ""),
            str(getattr(self, "next_goal", "") or ""),
        ]
        analysis = getattr(self, "brain_thought", "")
        if analysis:
            parts.append(str(analysis))
        return " ".join(parts).lower()

    def _goal_is_web_navigation(self, goal: Optional[str] = None) -> bool:
        goal_text = str(goal or self.next_goal or "")
        if self._is_searchable_selection_goal(goal_text):
            return False
        text = f"{goal_text} {self.original_task or self.task or ''}".lower()
        compact = re.sub(r"\s+", "", text)
        strong_navigation_terms = (
            "browser",
            "google",
            "webpage",
            "website",
            "url",
            "address bar",
            "openkylin",
            "浏览器",
            "网页",
            "网站",
            "官网",
            "地址栏",
            "网址",
        )
        search_or_load_terms = (
            "search",
            "load",
            "page",
            "搜索",
            "加载",
            "页面",
        )
        if any(term in text or term in compact for term in strong_navigation_terms):
            return True
        has_search_or_load = any(term in text or term in compact for term in search_or_load_terms)
        browser_context_terms = (
            "browser",
            "chrome",
            "chromium",
            "firefox",
            "edge",
            "address bar",
            "浏览器",
            "地址栏",
        )
        return has_search_or_load and any(term in text or term in compact for term in browser_context_terms)

    def _action_is_browser_switch(self, action: Any) -> bool:
        action_name, params = self._action_name_and_params(action)
        if action_name != "open_app" or not isinstance(params, dict):
            return False
        app_name = str(params.get("app_name", "") or "").lower()
        browser_terms = (
            "firefox",
            "edge",
            "chrome",
            "chromium",
            "browser",
            "浏览器",
            "火狐",
            "奇安信",
            "360",
        )
        return any(term in app_name for term in browser_terms)

    def _actions_include_browser_switch(self, actions: list[Any]) -> bool:
        return any(self._action_is_browser_switch(action) for action in actions or [])

    def _direct_navigation_attempt_allowed(self, actions: list[Any]) -> bool:
        for action in actions or []:
            action_name, params = self._action_name_and_params(action)
            if action_name != "input_text" or not isinstance(params, dict):
                continue
            text = str(params.get("text", "") or "").lower()
            if "." in text or text.startswith(("http://", "https://")):
                return True
        return False

    def _reset_no_progress_budgets(self) -> None:
        self.consecutive_failures = 0
        self._no_progress_count = 0
        self._last_no_progress_signature = None
        self._last_no_progress_strategy_signature = None
        self._stalled_failure_count = 0

    def _reset_stalled_navigation_budgets(self) -> None:
        self._stalled_navigation_count = 0
        self._stalled_navigation_strategy_switch_requested = False
        self._stalled_navigation_block_count = 0
        self._stalled_navigation_strategies.clear()

    def _is_material_strategy_shift(self, actions: list[Any]) -> bool:
        """Return True when a dispatched action is a new recovery strategy.

        The health checker runs before the next screenshot is available.  If a
        controller-successful action is materially different from the failed or
        blocked strategy that led to recovery, give it one observation turn
        instead of charging it with the previous screen's failure.
        """
        strategy = self._action_strategy_batch_signature(actions)
        if not strategy:
            return False

        recovery_active = bool(
            getattr(self, "_last_action_guard_block", None)
            or getattr(self, "_stalled_navigation_strategy_switch_requested", False)
            or (
                self._model_reports_previous_no_progress()
                and (
                    getattr(self, "_no_progress_count", 0)
                    or getattr(self, "consecutive_failures", 0)
                )
            )
        )
        if not recovery_active:
            return False

        blocked_strategies = {
            str(item)
            for item in list(getattr(self, "_blocked_action_strategy_signatures", []))
        }
        failed_strategy = getattr(self, "_last_no_progress_strategy_signature", None)
        stalled_strategies = {
            str(item)
            for item in getattr(self, "_stalled_navigation_strategies", set())
        }
        if strategy == failed_strategy or strategy in blocked_strategies:
            return False
        if self._goal_is_web_navigation() and strategy in stalled_strategies:
            return False
        return True

    def _stalled_navigation_failure_reason(self) -> str:
        failure_text = self._current_failure_text()
        if any(term in failure_text for term in ("proxy", "代理")):
            detail = "页面疑似受代理或网络配置影响无法加载。"
        elif any(term in failure_text for term in ("load", "加载", "未跳转", "无变化", "停留")):
            detail = "浏览器页面在多次搜索/回车/点击后仍未跳转或加载。"
        else:
            detail = "网页搜索或访问阶段连续无可见进展。"
        return (
            f"{detail} 已尝试 {self._stalled_navigation_count} 次网页导航相关操作，"
            f"覆盖 {len(self._stalled_navigation_strategies)} 种策略，仍未出现有效页面结果。"
        )

    def _check_stalled_navigation_strategy(self, actions: list[Any]) -> Optional[str]:
        if self._stalled_navigation_count < self.web_navigation_switch_steps:
            return None
        if not self._goal_is_web_navigation():
            return None
        if self._actions_include_browser_switch(actions) or self._direct_navigation_attempt_allowed(actions):
            self._stalled_navigation_strategy_switch_requested = False
            return None
        self._stalled_navigation_block_count += 1
        if self._stalled_navigation_block_count >= 2:
            reason = (
                self._stalled_navigation_failure_reason()
                + " 模型在收到换入口/换浏览器要求后仍继续重复当前浏览器内操作。"
            )
            self._request_user_assistance(
                reason_code="web_navigation_stalled",
                reason=reason,
                user_action_required=self._assistance_user_action_for_reason("web_navigation_stalled"),
                resume_condition=self._assistance_resume_condition_for_reason("web_navigation_stalled"),
                confidence=0.85,
            )
            return reason
        self._stalled_navigation_strategy_switch_requested = True
        return (
            "Web navigation appears stalled in the current browser. Stop repeating "
            "search-box, Enter, refresh, Tab, or same-page clicks in this browser. "
            "Try a materially different route now: open another installed browser "
            "(for example Firefox/Edge/Chrome/Chromium if available), use its address "
            "bar/search, or use a direct known official URL only if it is already "
            "known from the task or visible evidence. If that also fails, report that "
            "the task cannot be completed because the web page cannot be reached."
        )

    def _model_reports_previous_no_progress(self) -> bool:
        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        evaluation = str(self.current_state.get("step_evaluate", "") or "").strip().lower()
        if not evaluation:
            return False
        no_progress_terms = (
            "fail",
            "failed",
            "error",
            "no progress",
            "unchanged",
            "unsuccessful",
            "uncertain",
            "失败",
            "错误",
            "无进展",
            "未成功",
            "未完成",
            "不确定",
            "无变化",
        )
        return any(term in evaluation for term in no_progress_terms)

    def _screens_differ(self) -> bool:
        """Return True when the latest two screenshots differ meaningfully.

        Compares the screenshot captured before the previous action was executed
        (``previous_screenshot``) against the latest one (``screenshot_annotated``).
        Small noise such as a blinking caret or a ticking clock is tolerated so a
        real UI change (dialog opened, text inserted, page navigated) is what
        counts as new evidence.
        """
        previous = self.previous_screenshot
        current = self.screenshot_annotated
        if previous is None or current is None or previous is current:
            return False
        try:
            size = (64, 64)
            thumb_a = previous.convert("L").resize(size)
            thumb_b = current.convert("L").resize(size)
            diff = ImageChops.difference(thumb_a, thumb_b)
            return float(ImageStat.Stat(diff).mean[0]) > _SCREEN_DIFF_MEAN_THRESHOLD
        except Exception:
            logger.debug("Screen diff comparison failed", exc_info=True)
            return False

    def _action_name_and_params(self, action: Any) -> tuple[Optional[str], Any]:
        action_dict = action.model_dump(exclude_unset=True) if hasattr(action, "model_dump") else action
        if not isinstance(action_dict, dict) or not action_dict:
            return None, None
        action_name = next(iter(action_dict.keys()))
        return action_name, action_dict.get(action_name)

    def _is_wechat_group_name_modification_goal(self, goal: Optional[str] = None) -> bool:
        text = f"{goal or ''} {getattr(self, 'original_task', '') or ''}".lower()
        has_wechat = "微信" in text or "wechat" in text
        has_group_name = any(
            term in text
            for term in (
                "群聊名称",
                "微信群名称",
                "微信群名",
                "群名称",
                "群名",
                "group name",
            )
        )
        has_modify = any(
            term in text
            for term in (
                "修改",
                "更改",
                "改名",
                "重命名",
                "命名为",
                "设置",
                "change",
                "rename",
            )
        )
        return has_wechat and has_group_name and has_modify

    def _ensure_wechat_group_name_input_commits(self, actions: list[Any]) -> list[Any]:
        if not actions or not self._is_wechat_group_name_modification_goal(self.next_goal):
            return actions

        changed = False
        for action in actions:
            action_name, params = self._action_name_and_params(action)
            if action_name != "input_text":
                continue

            if isinstance(action, dict):
                action_params = action.get("input_text")
                if isinstance(action_params, dict) and not action_params.get("press_enter", False):
                    action_params["press_enter"] = True
                    changed = True
                continue

            input_text_action = getattr(action, "input_text", None)
            if input_text_action is not None and not getattr(input_text_action, "press_enter", False):
                try:
                    input_text_action.press_enter = True
                    changed = True
                except Exception:
                    logger.debug("Unable to set press_enter on WeChat group-name input action", exc_info=True)

        if changed:
            logger.info("[ActionPolicy] Added press_enter=true for WeChat group name input_text.")
        return actions

    def _is_select_all_hotkey(self, action: Any) -> bool:
        action_name, params = self._action_name_and_params(action)
        if action_name == "Hotkey" and isinstance(params, dict):
            return split_hotkey(params.get("key", "")) == ["ctrl", "a"]

        if action_name == "multi_Hotkey" and isinstance(params, dict):
            keys = [
                normalize_key(params.get("key1", "")),
                normalize_key(params.get("key2", "")),
                normalize_key(params.get("key3", "") or ""),
            ]
            keys = [key for key in keys if key]
            return len(keys) == 2 and set(keys) == {"ctrl", "a"}

        return False

    def _input_text_select_all(self, action: Any) -> bool:
        action_name, params = self._action_name_and_params(action)
        if action_name == "input_text" and isinstance(params, dict):
            return bool(params.get("select_all", True))
        return True

    def _remove_redundant_select_all_before_input(self, actions: list[Any]) -> list[Any]:
        if not actions:
            return actions

        filtered_actions: list[Any] = []
        removed_count = 0
        for index, action in enumerate(actions):
            next_action = actions[index + 1] if index + 1 < len(actions) else None
            next_action_name, _ = self._action_name_and_params(next_action)
            if (
                self._is_select_all_hotkey(action)
                and next_action_name == "input_text"
                and self._input_text_select_all(next_action)
            ):
                removed_count += 1
                continue
            filtered_actions.append(action)

        if removed_count:
            logger.debug(
                "[ActionOptimizer] Removed %d redundant Ctrl+A action(s) before input_text",
                removed_count,
            )
        return filtered_actions

    def _task_requires_content_delivery(self) -> bool:
        task_text = (self.original_task or self.task or "").lower()
        delivery_terms = (
            "发送",
            "发给",
            "发到",
            "转发",
            "分享",
            "回复",
            "私信",
            "邮件",
            "send",
            "share",
            "forward",
            "reply",
            "message",
            "email",
            "mail",
            "post",
            "dm",
        )
        return any(term in task_text for term in delivery_terms)

    def _task_requires_wechat_group_creation(self) -> bool:
        task_text = (self.original_task or self.task or "").lower()
        wechat_terms = ("微信", "wechat")
        chinese_creation_terms = (
            "建群",
            "创建",
            "新建",
            "发起",
            "组建",
            "建立",
        )
        english_creation_terms = (
            "create a group",
            "create group",
            "new group chat",
            "start a group chat",
        )
        has_wechat = any(term in task_text for term in wechat_terms)
        has_chinese_creation = "群" in task_text and any(
            term in task_text for term in chinese_creation_terms
        )
        has_english_creation = any(term in task_text for term in english_creation_terms)
        return has_wechat and (has_chinese_creation or has_english_creation)

    def _derive_task_obligations(self) -> list[str]:
        obligations: list[str] = []
        if self._task_requires_content_delivery():
            obligations.extend(["content_inserted", "content_dispatched"])
        return obligations

    def _is_selection_search_input_goal(self, goal: str) -> bool:
        goal_text = str(goal or "").lower()
        if not goal_text:
            return False
        search_terms = (
            "搜索",
            "查找",
            "筛选",
            "输入姓名",
            "输入联系人",
            "输入成员",
            "输入收件人",
            "搜索框",
            "查找框",
            "search",
            "find",
            "filter",
        )
        selection_targets = (
            "联系人",
            "成员",
            "好友",
            "收件人",
            "抄送",
            "用户",
            "人员",
            "候选",
            "列表项",
            "条目",
            "文件名",
            "文件",
            "群成员",
            "群聊成员",
            "添加",
            "勾选",
            "选择",
            "发起群聊",
            "新建群聊",
            "创建群聊",
            "contact",
            "member",
            "participant",
            "recipient",
            "user",
            "people",
            "person",
            "item",
            "file",
        )
        delivery_input_targets = (
            "聊天输入框",
            "消息输入框",
            "正文输入框",
            "评论输入框",
            "回复输入框",
            "邮件正文",
            "帖子正文",
            "群输入框",
            "发送内容",
            "发送消息",
            "粘贴到",
            "输入到",
            "send message",
            "chat input",
            "message box",
            "body",
            "comment box",
            "reply box",
        )
        if any(term in goal_text for term in delivery_input_targets):
            return False
        return any(term in goal_text for term in search_terms) and any(
            term in goal_text for term in selection_targets
        )

    def _add_action_evidence(
        self,
        evidence_type: str,
        *,
        step: int,
        action: Optional[str] = None,
        goal: Optional[str] = None,
        details: Optional[dict[str, Any]] = None,
    ) -> None:
        record = {
            "sequence": len(self.action_evidence) + 1,
            "type": evidence_type,
            "step": step,
            "action": action,
            "goal": goal or self.next_goal,
            "details": details or {},
            "time": time.time(),
        }
        self.action_evidence.append(record)
        if len(self.action_evidence) > 200:
            self.action_evidence = self.action_evidence[-200:]
        self.execution_journal.event("action_evidence", **record)

    def _record_action_evidence(
        self,
        actions: list[Any],
        results: list[ActionResult],
        *,
        step: int,
    ) -> None:
        content_insert_seen = self._has_evidence("content_inserted")
        for index, action in enumerate(actions or []):
            result = results[index] if index < len(results) else None
            if result is not None and getattr(result, "error", None):
                continue
            action_name, params = self._action_name_and_params(action)
            if not action_name:
                continue
            if action_name == "input_text":
                text = params.get("text", "") if isinstance(params, dict) else ""
                text = str(text)
                if self._is_selection_search_input_goal(self.next_goal):
                    self._add_action_evidence(
                        "selection_search_input",
                        step=step,
                        action=action_name,
                        details={
                            "text": text,
                            "text_length": len(text),
                            "role": "selection_search",
                        },
                    )
                elif self._task_requires_content_delivery() and self._is_task_content_input_goal(self.next_goal):
                    self._add_action_evidence(
                        "content_inserted",
                        step=step,
                        action=action_name,
                        details={
                            "text": text,
                            "text_length": len(text),
                            "role": "task_input_candidate",
                        },
                    )
                    content_insert_seen = True
                if self._is_enter_or_submit_click_action(action, self.next_goal) and content_insert_seen:
                    self._add_action_evidence(
                        "content_dispatched",
                        step=step,
                        action=action_name,
                        details={"dispatch_goal": self.next_goal, "source": "input_text.press_enter"},
                    )
            elif self._is_paste_hotkey_action(action):
                if self._is_selection_search_input_goal(self.next_goal):
                    self._add_action_evidence(
                        "selection_search_input",
                        step=step,
                        action=action_name,
                        details={"source": "clipboard_paste", "role": "selection_search"},
                    )
                elif self._task_requires_content_delivery() and self._is_task_content_input_goal(self.next_goal):
                    self._add_action_evidence(
                        "content_inserted",
                        step=step,
                        action=action_name,
                        details={"source": "clipboard_paste", "role": "task_input_candidate"},
                    )
                    content_insert_seen = True
            elif self._is_enter_or_submit_click_action(action, self.next_goal) and content_insert_seen:
                self._add_action_evidence(
                    "content_dispatched",
                    step=step,
                    action=action_name,
                    details={"dispatch_goal": self.next_goal},
                )
    def _has_evidence(self, evidence_type: str) -> bool:
        return any(item.get("type") == evidence_type for item in self.action_evidence)

    def _has_verified_wechat_group_evidence(self, evidence_type: str) -> bool:
        if not self._task_requires_wechat_group_creation():
            return False
        return any(
            item.get("type") == evidence_type
            and isinstance(item.get("details"), dict)
            and item["details"].get("source") == "completed_workflow"
            for item in self.action_evidence
        )

    def _record_verified_wechat_group_progress(self, goal: str, *, step: int) -> None:
        if not self._task_requires_wechat_group_creation():
            return
        creation_goal = self._is_wechat_group_creation_goal(goal)
        if (
            creation_goal
            and not self._has_verified_wechat_group_evidence("wechat_group_creation_started")
        ):
            self._add_action_evidence(
                "wechat_group_creation_started",
                step=step,
                goal=goal,
                details={"goal": goal, "source": "completed_workflow"},
            )
        if (
            not creation_goal
            and self._is_wechat_group_ready_goal(goal)
            and self._has_verified_wechat_group_evidence("wechat_group_creation_started")
            and not self._has_verified_wechat_group_evidence("wechat_group_chat_ready")
        ):
            self._add_action_evidence(
                "wechat_group_chat_ready",
                step=step,
                goal=goal,
                details={"goal": goal, "source": "completed_workflow"},
            )

    def _is_wechat_group_creation_goal(self, goal: str) -> bool:
        if not self._task_requires_wechat_group_creation():
            return False
        goal_text = str(goal or "")
        if not any(term in goal_text for term in ("群聊", "微信群", "建群", "新群")):
            return False
        creation_terms = ("发起群聊", "新建群聊", "创建群聊", "创建微信群", "微信建群")
        plus_terms = ("上方", "+", "加号")
        return any(term in goal_text for term in creation_terms) or (
            "群" in goal_text and any(term in goal_text for term in plus_terms)
        )

    def _is_wechat_group_ready_goal(self, goal: str) -> bool:
        if not self._task_requires_wechat_group_creation():
            return False
        goal_text = str(goal or "")
        group_terms = (
            "微信群",
            "群聊",
            "新群",
            "当前群",
            "群聊天窗口",
            "群输入框",
            "群消息",
            "群成员",
        )
        if not any(term in goal_text for term in group_terms):
            return False
        ready_terms = (
            "创建完成",
            "建群完成",
            "确认建群",
            "进入",
            "打开",
            "发送",
            "发给",
            "发到",
            "粘贴",
            "输入",
            "当前群聊",
            "新群",
            "群聊天窗口",
            "群输入框",
        )
        return any(term in goal_text for term in ready_terms)

    def _has_content_dispatch_after_insert(self) -> bool:
        insert_sequence = None
        for item in self.action_evidence:
            if item.get("type") == "content_inserted" and not self._is_selection_search_input_goal(
                str(item.get("goal", ""))
            ):
                insert_sequence = int(item.get("sequence", 0) or 0)
            elif item.get("type") == "content_dispatched" and insert_sequence is not None:
                if int(item.get("sequence", 0) or 0) > insert_sequence:
                    return True
        return False

    def _has_visual_dispatch_confirmation_after_latest_dispatch(self) -> bool:
        return any(
            item.get("type") == "content_dispatch_visually_confirmed"
            for item in self.action_evidence
        )

    def _backfill_content_delivery_evidence_from_history(self) -> None:
        if "content_inserted" not in self.task_obligations:
            return
        saw_insert = self._has_evidence("content_inserted")
        saw_dispatch_after_insert = self._has_content_dispatch_after_insert()
        if saw_insert and saw_dispatch_after_insert:
            return

        history_insert_seen = saw_insert
        for position, action, goal in self._iter_history_actions():
            action_name, params = self._action_name_and_params(action)
            if action_name == "input_text" or self._is_paste_hotkey_action(action):
                if self._is_selection_search_input_goal(goal) or not self._is_task_content_input_goal(goal):
                    continue
                history_insert_seen = True
                if not saw_insert:
                    details: dict[str, Any] = {
                        "source": "history_backfill",
                        "role": "task_input_candidate",
                    }
                    if action_name == "input_text" and isinstance(params, dict):
                        text = str(params.get("text", ""))
                        details.update({"text": text, "text_length": len(text)})
                    self._add_action_evidence(
                        "content_inserted",
                        step=position,
                        action=action_name,
                        goal=goal,
                        details=details,
                    )
                    saw_insert = True
                if (
                    history_insert_seen
                    and not saw_dispatch_after_insert
                    and self._is_enter_or_submit_click_action(action, goal)
                ):
                    self._add_action_evidence(
                        "content_dispatched",
                        step=position,
                        action=action_name,
                        goal=goal,
                        details={
                            "dispatch_goal": goal,
                            "source": "history_backfill_input_text.press_enter",
                        },
                    )
                    saw_dispatch_after_insert = True
                continue

            if (
                history_insert_seen
                and not saw_dispatch_after_insert
                and self._is_enter_or_submit_click_action(action, goal)
            ):
                self._add_action_evidence(
                    "content_dispatched",
                    step=position,
                    action=action_name,
                    goal=goal,
                    details={"dispatch_goal": goal, "source": "history_backfill"},
                )
                saw_dispatch_after_insert = True

    def _dispatch_review_has_specific_evidence(self) -> bool:
        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        evaluation = str(self.current_state.get("step_evaluate", "")).lower()
        if "success" not in evaluation and "成功" not in evaluation:
            return False
        analysis = str(self.brain_thought or "")
        weak_terms = (
            "输入框为空",
            "输入框已清空",
            "输入框中",
            "输入框内",
            "输入框显示",
            "输入框里",
            "仍在输入框",
            "按钮变灰",
            "发送按钮灰",
            "发送按钮亮",
            "发送按钮可",
            "发送按钮仍",
            "待发送",
            "尚未发送",
            "还未发送",
            "未发送",
            "未看到",
            "没有看到",
            "看不到",
            "不确定",
            "无法确认",
            "ambiguous",
            "uncertain",
        )
        analysis_lower = analysis.lower()
        if any(term in analysis_lower for term in weak_terms):
            return False
        positive_terms = (
            "消息区",
            "聊天区",
            "结果区",
            "记录区",
            "气泡",
            "新消息",
            "已发送",
            "成功发送",
            "均已成功发送",
            "发送成功",
            "提交成功",
            "发布成功",
            "分享成功",
            "出现在",
            "显示在",
            "任务已完成",
            "已经完成",
            "已完成",
            "目标已达成",
            "完成任务",
            "delivered",
            "sent",
            "submitted",
            "posted",
            "accepted",
            "success",
        )
        return any(term.lower() in analysis_lower for term in positive_terms)

    def _original_task_description(self) -> str:
        return json.dumps(
            self.original_task or self.task or "the user's original task",
            ensure_ascii=False,
        )

    def _parsed_action_contains_done(self, parsed: dict[str, Any]) -> bool:
        for action in parsed.get("action", []) or []:
            action_name, _ = self._action_name_and_params(action)
            if action_name == "done":
                return True
        return False

    def _replace_parsed_actions_with_wait(self, parsed: dict[str, Any]) -> None:
        if "action" in parsed:
            parsed["action"] = [{"wait": {"seconds": 0.5}}]

    def _dispatch_review_corrective_goal(self) -> str:
        original_task = self._original_task_description()
        return (
            f"复核用户初始任务{original_task}是否已经完成：必须根据当前可见界面确认初始任务要求的目标已经达成，"
            "不能把联系人/成员/文件/收件人搜索框中的筛选文字当成已发送内容，"
            "也不能只依据输入框变空、按钮变灰、旧历史记录或模型推测。"
            "若证据不足，选择纠正路径；若已明确送达，保持当前界面并下一轮完成。"
        )

    def _completion_prerequisite_next_goal(self) -> str:
        self._backfill_content_delivery_evidence_from_history()
        if not self._has_evidence("content_inserted"):
            return "继续完成用户任务：当前缺少本次内容输入记录，请先聚焦目标输入区并输入本次任务内容。"
        if not self._has_content_dispatch_after_insert():
            if self._completion_prerequisite_block_count >= self._completion_prerequisite_escape_threshold:
                return (
                    "复核用户任务是否已经完成：本次内容已输入，但结构化历史里缺少发送/提交动作证据。"
                    "不要重复发送或重复输入；只根据当前可见界面判断内容是否已经送达、提交成功或目标值已生效。"
                    "若已明确完成，下一轮输出完成；若未完成，再选择一个不同的发送/提交路径。"
                )
            return "继续完成用户任务：本次内容已输入，但缺少发送/提交动作证据，请确认目标和内容后执行发送/提交。"
        return (
            "继续复核发送/提交结果：本次内容已输入并执行过发送/提交，"
            "现在只检查最新界面是否有明确送达/成功证据；证据明确时下一轮完成。"
        )

    def _handle_content_dispatch_review_result(self, parsed: dict[str, Any], *, step: int) -> None:
        completion_requested = bool(self.next_goal and "[TASK_COMPLETE]" in self.next_goal) or self._parsed_action_contains_done(parsed)
        pending = self._pending_content_dispatch_verification or {}
        evidence_ok = self._dispatch_review_has_specific_evidence()
        self.execution_journal.event(
            "content_dispatch_visually_reviewed",
            step=step,
            evaluation=self.current_state.get("step_evaluate", ""),
            completion_requested=completion_requested,
            evidence_accepted=evidence_ok,
        )
        self._content_dispatch_review_passed = evidence_ok
        if evidence_ok:
            self._add_action_evidence(
                "content_dispatch_visually_confirmed",
                step=step,
                action="visual_review",
                details={
                    "reviewed_dispatch_step": pending.get("dispatch_step"),
                    "original_task": self.original_task or self.task,
                    "analysis": str(self.brain_thought or "")[:500],
                },
            )
        if completion_requested:
            self.next_goal = (
                "[TASK_COMPLETE] The task has been completed successfully."
                if evidence_ok
                else self._dispatch_review_corrective_goal()
            )
            self.current_state["next_goal"] = self.next_goal
            self.current_state["step_evaluate"] = "Success" if evidence_ok else "Failed"
            parsed["current_state"] = self.current_state
            if evidence_ok:
                parsed["action"] = [{"done": {}}]
            else:
                self._replace_parsed_actions_with_wait(parsed)
        elif not evidence_ok:
            self.execution_journal.metrics.verification_rejections += 1
            self.execution_journal.event(
                "content_dispatch_visual_confirmation_rejected",
                step=step,
                reviewed_dispatch_step=pending.get("dispatch_step"),
                analysis=str(self.brain_thought or "")[:500],
            )
        self._pending_content_dispatch_verification = None

    def _is_paste_hotkey_action(self, action: Any) -> bool:
        action_name, params = self._action_name_and_params(action)
        if action_name == "Hotkey" and isinstance(params, dict):
            return split_hotkey(params.get("key", "")) == ["ctrl", "v"]
        if action_name == "multi_Hotkey" and isinstance(params, dict):
            keys = [
                normalize_key(params.get("key1", "")),
                normalize_key(params.get("key2", "")),
                normalize_key(params.get("key3", "") or ""),
            ]
            keys = [key for key in keys if key]
            return "ctrl" in keys and "v" in keys
        return False

    def _is_dispatch_goal(self, goal: str) -> bool:
        goal_text = (goal or "").lower()
        dispatch_terms = (
            "发送",
            "发出",
            "发给",
            "发到",
            "提交",
            "发布",
            "分享",
            "转发",
            "回复",
            "确认发送",
            "send",
            "submit",
            "post",
            "share",
            "forward",
            "reply",
            "dispatch",
        )
        return any(term in goal_text for term in dispatch_terms)

    def _is_task_content_input_goal(self, goal: str) -> bool:
        goal_text = str(goal or "").lower()
        if not goal_text or self._is_selection_search_input_goal(goal_text):
            return False
        content_input_terms = (
            "聊天输入框",
            "消息输入框",
            "正文输入框",
            "评论输入框",
            "回复输入框",
            "邮件正文",
            "帖子正文",
            "群输入框",
            "粘贴",
            "输入到",
            "填入",
            "发送内容",
            "发送消息",
            "消息内容",
            "待发送",
            "send message",
            "chat input",
            "message box",
            "message content",
            "body",
            "comment box",
            "reply box",
            "paste",
        )
        source_collection_terms = (
            "地址栏",
            "网址",
            "百度",
            "搜索框",
            "搜索",
            "查询",
            "复制",
            "选中",
            "查看",
            "打开",
            "访问",
            "url",
            "address bar",
            "search box",
            "search",
            "query",
            "copy",
            "select",
            "open",
            "visit",
            "navigate",
        )
        if any(term in goal_text for term in content_input_terms):
            return True
        if any(term in goal_text for term in source_collection_terms):
            return False
        return self._is_dispatch_goal(goal_text)

    def _is_enter_or_submit_click_action(self, action: Any, goal: str = "") -> bool:
        action_name, params = self._action_name_and_params(action)
        if action_name == "input_text" and isinstance(params, dict):
            return bool(params.get("press_enter", False)) and self._is_dispatch_goal(goal)
        if action_name == "Hotkey" and isinstance(params, dict):
            return split_hotkey(params.get("key", "")) == ["enter"] and self._is_dispatch_goal(goal)
        if action_name == "multi_Hotkey" and isinstance(params, dict):
            keys = [
                normalize_key(params.get("key1", "")),
                normalize_key(params.get("key2", "")),
                normalize_key(params.get("key3", "") or ""),
            ]
            return any(key == "enter" for key in keys if key) and self._is_dispatch_goal(goal)
        return action_name == "Click" and self._is_dispatch_goal(goal)

    def _history_goal_for_position(self, position_1_based: int) -> str:
        step_context = self.brain_context.get(position_1_based, {}) if hasattr(self, "brain_context") else {}
        if isinstance(step_context, dict):
            current_state = step_context.get("current_state", {})
            if isinstance(current_state, dict):
                return str(current_state.get("next_goal", "") or "")
        return ""

    def _iter_history_actions(self) -> list[tuple[int, Any, str]]:
        actions: list[tuple[int, Any, str]] = []
        for index, item in enumerate(self.history.history, start=1):
            model_output = getattr(item, "model_output", None)
            if not model_output:
                continue
            step_id = int(getattr(item, "step_id", None) or index)
            goal = str(getattr(item, "next_goal", "") or self._history_goal_for_position(step_id))
            for action in getattr(model_output, "action", []) or []:
                actions.append((step_id, action, goal))
        return actions

    def _content_delivery_progress(self) -> tuple[bool, bool]:
        """Return whether this run inserted content and later dispatched it."""
        saw_content_insert = False
        saw_dispatch_after_insert = False
        for position, action, goal in self._iter_history_actions():
            action_name, _ = self._action_name_and_params(action)
            if action_name == "input_text" or self._is_paste_hotkey_action(action):
                if self._is_task_content_input_goal(goal) and self._task_requires_content_delivery():
                    saw_content_insert = True
                if self._is_enter_or_submit_click_action(action, goal):
                    saw_dispatch_after_insert = True
                continue
            if saw_content_insert and self._is_enter_or_submit_click_action(
                action,
                goal,
            ):
                saw_dispatch_after_insert = True
        return saw_content_insert, saw_dispatch_after_insert

    def _completion_prerequisite_error(self) -> Optional[str]:
        if "content_inserted" not in self.task_obligations:
            return None
        self._backfill_content_delivery_evidence_from_history()
        if self.next_goal and "[TASK_COMPLETE]" in self.next_goal and self._dispatch_review_has_specific_evidence():
            if not self._has_visual_dispatch_confirmation_after_latest_dispatch():
                self._add_action_evidence(
                    "content_dispatch_visually_confirmed",
                    step=self.n_steps,
                    action="completion_frame_visual_review",
                    details={
                        "original_task": self.original_task or self.task,
                        "analysis": str(self.brain_thought or "")[:500],
                        "source": "completion_prerequisite_original_task_review",
                    },
                )
            return None
        if not self._has_evidence("content_inserted"):
            return (
                "Completion blocked: the user task requires sending/sharing content, "
                "but this run has no recoverable evidence of input_text or paste into "
                "the target destination."
            )
        if not self._has_content_dispatch_after_insert():
            return (
                "Completion blocked: this run inserted content, but there is no structured "
                "evidence of a later Enter or submit/send click. Verify the destination and "
                "dispatch the inserted content before claiming task completion."
            )
        if self._pending_content_dispatch_verification:
            return (
                "Completion blocked: this run dispatched content, but the resulting screen "
                "has not yet been visually reviewed."
            )
        if not self._has_visual_dispatch_confirmation_after_latest_dispatch():
            if self.next_goal and "[TASK_COMPLETE]" in self.next_goal and self._dispatch_review_has_specific_evidence():
                self._add_action_evidence(
                    "content_dispatch_visually_confirmed",
                    step=self.n_steps,
                    action="completion_frame_visual_review",
                    details={
                        "original_task": self.original_task or self.task,
                        "analysis": str(self.brain_thought or "")[:500],
                        "source": "completion_prerequisite_backfill",
                    },
                )
                return None
            return (
                "Completion blocked: this run has insert and dispatch actions, but no fresh "
                "visual confirmation after the latest dispatch."
            )
        return None

    def _latest_state_success(self) -> bool:
        if not isinstance(getattr(self, "current_state", None), dict):
            return False
        evaluation = str(self.current_state.get("step_evaluate", "") or "").strip().lower()
        return evaluation.startswith("success") or "成功" in evaluation

    def _record_completion_prerequisite_block(self, reason: str) -> int:
        if reason == self._completion_prerequisite_block_reason:
            self._completion_prerequisite_block_count += 1
        else:
            self._completion_prerequisite_block_reason = reason
            self._completion_prerequisite_block_count = 1
        return self._completion_prerequisite_block_count

    def _reset_completion_prerequisite_blocks(self) -> None:
        self._completion_prerequisite_block_reason = None
        self._completion_prerequisite_block_count = 0

    def _completion_prerequisite_block_can_be_accepted(self, reason: str) -> bool:
        if self._completion_prerequisite_block_count < self._completion_prerequisite_escape_threshold:
            return False
        if not (self.next_goal and "[TASK_COMPLETE]" in self.next_goal):
            return False
        if not self._latest_state_success():
            return False
        if not self._has_evidence("content_inserted"):
            return False
        if not self._dispatch_review_has_specific_evidence():
            return False
        # Repeated visual success can break a stale structured-evidence loop,
        # but never invent the initial content entry.
        return (
            "no structured evidence of a later Enter or submit/send click" in reason
            or "no fresh visual confirmation after the latest dispatch" in reason
        )

    def _accept_repeated_completion_prerequisite_block(self, reason: str) -> None:
        if not self._has_content_dispatch_after_insert():
            self._add_action_evidence(
                "content_dispatched",
                step=self.n_steps,
                action="completion_gate_repeated_visual_success",
                details={
                    "source": "completion_prerequisite_repeated_visual_success",
                    "reason": reason,
                },
            )
        if not self._has_visual_dispatch_confirmation_after_latest_dispatch():
            self._add_action_evidence(
                "content_dispatch_visually_confirmed",
                step=self.n_steps,
                action="completion_gate_repeated_visual_success",
                details={
                    "source": "completion_prerequisite_repeated_visual_success",
                    "analysis": str(self.brain_thought or "")[:500],
                    "reason": reason,
                },
            )
        self._reset_completion_prerequisite_blocks()

    def _completion_has_fresh_delivery_confirmation(self) -> bool:
        return (
            "content_inserted" in self.task_obligations
            and self._latest_state_success()
            and self._has_content_dispatch_after_insert()
            and self._has_visual_dispatch_confirmation_after_latest_dispatch()
        )

    def _extract_text_target_from_goal(self, goal: str) -> Optional[str]:
        if not goal:
            return None

        quote = r"[\"'“”‘’]"
        target_patterns = (
            rf"(?:完整[^，。；;]*为|完整[^，。；;]*是)\s*{quote}([^\"'“”‘’]+){quote}",
            rf"(?:直接输入|输入|键入|录入|填入)\s*{quote}([^\"'“”‘’]+){quote}",
            rf"(?:改为|更改为|修改为|设置为|替换为|变为)\s*{quote}([^\"'“”‘’]+){quote}",
            rf"(?:type|enter|input)\s*{quote}([^\"'“”‘’]+){quote}",
            rf"(?:set(?: it)? to|change(?: it)? to|replace(?: it)? with)\s*{quote}([^\"'“”‘’]+){quote}",
        )
        for pattern in target_patterns:
            matches = re.findall(pattern, goal, flags=re.IGNORECASE)
            if matches:
                return matches[-1].strip()

        time_target = self._extract_time_target_from_goal(goal)
        if time_target:
            return time_target

        if self._is_sensitive_text_goal(goal):
            unquoted_sensitive_patterns = (
                r"(?:密码|口令|验证码|校验码|pin|password|passcode|verification code|otp)[^，。；;\n]*?(?:设置为|设为|填入|输入|为|是)\s*([A-Za-z0-9@#._-]{3,64})",
                r"(?:设置|输入|填入|enter|input|set)[^，。；;\n]*?(?:密码|口令|验证码|校验码|pin|password|passcode|verification code|otp)[^，。；;\n]*?([A-Za-z0-9@#._-]{3,64})",
            )
            for pattern in unquoted_sensitive_patterns:
                matches = re.findall(pattern, goal, flags=re.IGNORECASE)
                if matches:
                    return matches[-1].strip()

        quoted_values = [value.strip() for value in re.findall(r"[\"'“‘]([^\"'”’]+)[\"'”’]", goal)]
        if not quoted_values:
            return None
        if any(term in goal for term in ("输入框", "文本框", "搜索框", "密码", "主题", "时间", "日期", "选择框", "下拉", "下拉按钮", "下拉箭头")):
            return quoted_values[-1]
        return None

    def _extract_time_target_from_goal(self, goal: str) -> Optional[str]:
        if not self._is_time_edit_goal(goal):
            return None

        time_matches = re.findall(r"(?<!\d)([0-2]?\d[:：][0-5]\d)(?!\d)", goal)
        if time_matches:
            return time_matches[-1].replace("：", ":")

        hour_matches = re.findall(r"(?:改为|更改为|修改为|设置为|输入|选择|to)\s*([0-2]?\d)(?!\d)", goal, flags=re.IGNORECASE)
        if hour_matches:
            return hour_matches[-1]
        return None

    def _is_click_action(self, action: Any) -> bool:
        action_name, _ = self._action_name_and_params(action)
        return action_name in {"Click", "DoubleClick"}

    def _input_text_values(self, actions: list[Any]) -> list[str]:
        values: list[str] = []
        for action in actions:
            action_name, params = self._action_name_and_params(action)
            if action_name == "input_text" and isinstance(params, dict):
                text = params.get("text")
                if isinstance(text, str) and text.strip():
                    values.append(text.strip())
        return values

    @staticmethod
    def _compact_text_for_match(text: str) -> str:
        return re.sub(r"\s+", "", text or "").lower()

    def _is_scroll_action(self, action: Any) -> bool:
        action_name, _ = self._action_name_and_params(action)
        return action_name in {"scroll_up", "scroll_down"}

    @staticmethod
    def _same_position(left: Any, right: Any) -> bool:
        if not (
            isinstance(left, (list, tuple))
            and isinstance(right, (list, tuple))
            and len(left) == 2
            and len(right) == 2
        ):
            return False
        try:
            return abs(float(left[0]) - float(right[0])) <= 1 and abs(float(left[1]) - float(right[1])) <= 1
        except (TypeError, ValueError):
            return False

    def _ensure_scroll_targets_mouse_position(self, actions: list[Any]) -> list[Any]:
        """Ensure every scroll is preceded by moving the pointer into that area."""
        if not actions:
            return actions

        normalized: list[Any] = []
        inserted = 0
        for action in actions:
            action_name, params = self._action_name_and_params(action)
            if action_name in {"scroll_up", "scroll_down"} and isinstance(params, dict):
                position = params.get("position")
                previous_name, previous_params = (
                    self._action_name_and_params(normalized[-1])
                    if normalized
                    else (None, None)
                )
                previous_position = (
                    previous_params.get("position")
                    if previous_name == "move_mouse" and isinstance(previous_params, dict)
                    else None
                )
                if position is not None and not self._same_position(previous_position, position):
                    normalized.append({"move_mouse": {"position": list(position)}})
                    inserted += 1
            normalized.append(action)

        if inserted:
            logger.info(
                "[ActionOptimizer] Inserted %d move_mouse action(s) before scroll.",
                inserted,
            )
            if hasattr(self, "execution_journal"):
                self.execution_journal.event(
                    "scroll_mouse_targeting_inserted",
                    step=self.n_steps,
                    inserted_count=inserted,
                )
        return normalized

    def _is_sensitive_text_goal(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        sensitive_terms = (
            "密码",
            "口令",
            "验证码",
            "校验码",
            "pin",
            "password",
            "passcode",
            "verification code",
            "otp",
        )
        return any(term in goal_lower for term in sensitive_terms)

    def _sensitive_goal_includes_enabling_control(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        enable_terms = (
            "开启",
            "打开",
            "启用",
            "勾选",
            "选中",
            "切换",
            "enable",
            "turn on",
            "toggle",
            "check",
        )
        return self._is_sensitive_text_goal(goal) and any(term in goal_lower for term in enable_terms)

    def _is_compact_selector_goal(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        selector_terms = (
            "时间",
            "日期",
            "小时",
            "分钟",
            "时长",
            "选择器",
            "选择框",
            "下拉",
            "下拉框",
            "下拉按钮",
            "下拉箭头",
            "带下拉",
            "time",
            "date",
            "hour",
            "minute",
            "duration",
            "selector",
            "dropdown",
            "dropdown button",
            "dropdown arrow",
            "combobox",
            "combo box",
        )
        return any(term in goal_lower for term in selector_terms)

    def _is_time_edit_goal(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        return any(term in goal_lower for term in ("时间", "小时", "分钟", "time", "hour", "minute"))

    def _goal_allows_direct_input_completion(self, goal: str) -> bool:
        goal_lower = goal.lower()
        direct_terms = (
            "输入",
            "键入",
            "录入",
            "填入",
            "选择",
            "直接",
            "改为",
            "更改为",
            "修改为",
            "设置为",
            "替换",
            "type",
            "enter",
            "input",
            "select",
            "change",
            "set",
            "replace",
        )
        editable_terms = (
            "输入框",
            "文本框",
            "搜索框",
            "密码",
            "主题",
            "时间",
            "日期",
            "选择框",
            "下拉",
            "下拉框",
            "下拉按钮",
            "下拉箭头",
            "带下拉",
            "field",
            "box",
            "dropdown",
            "dropdown button",
            "dropdown arrow",
            "combobox",
            "combo box",
        )
        return any(term in goal_lower for term in direct_terms) and any(term in goal_lower for term in editable_terms)

    def _is_searchable_selection_goal(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        selection_terms = (
            "选择",
            "选中",
            "添加",
            "邀请",
            "查找",
            "寻找",
            "搜索",
            "加入",
            "select",
            "add",
            "invite",
            "find",
            "search",
            "choose",
        )
        target_terms = (
            "联系人",
            "群成员",
            "成员",
            "好友",
            "用户",
            "人员",
            "收件人",
            "文件",
            "条目",
            "列表",
            "contact",
            "member",
            "user",
            "person",
            "people",
            "recipient",
            "file",
            "item",
            "list",
            "picker",
        )
        return any(term in goal_lower for term in selection_terms) and any(term in goal_lower for term in target_terms)

    def _limit_compact_selector_scroll(self, actions: list[Any]) -> list[Any]:
        if not actions or not self._is_compact_selector_goal(self.next_goal):
            return actions

        optimized_actions: list[Any] = []
        changed = False
        for action in actions:
            action_name, params = self._action_name_and_params(action)
            if action_name in {"scroll_up", "scroll_down"} and isinstance(params, dict):
                limited_params = dict(params)
                raw_dy = limited_params.get("dy", 2)
                try:
                    dy = int(abs(float(raw_dy)))
                except (TypeError, ValueError):
                    dy = 2
                limited_params["dy"] = min(max(dy, 1), 2)
                limited_params["dx"] = 0
                optimized_actions.append({action_name: limited_params})
                changed = changed or limited_params != params
                continue
            optimized_actions.append(action)

        if changed:
            logger.debug("[ActionOptimizer] Limited compact selector scroll magnitude to 1-2 ticks")
        return optimized_actions

    def _stage_text_input_actions(self, actions: list[Any]) -> list[Any]:
        """Split text entry into focus, paste, and post-paste verification stages.

        A screenshot/Brain observation must occur after a focus click and after
        the paste itself.  This prevents Ctrl+A from running against an
        unverified selector or the wrong window, and prevents Enter/Tab from
        hiding a value before its presence has been checked.
        """
        if not actions:
            return actions

        input_index = next(
            (
                index
                for index, action in enumerate(actions)
                if self._action_name_and_params(action)[0] == "input_text"
            ),
            None,
        )
        if input_index is None:
            return actions

        _, input_params = self._action_name_and_params(actions[input_index])
        expected_text = (
            str(input_params.get("text", ""))
            if isinstance(input_params, dict)
            else ""
        )
        prefix = actions[:input_index]
        prefix_has_click = any(self._is_click_action(action) for action in prefix)
        prefix_has_focus_attempt = any(
            self._action_name_and_params(action)[0] not in {None, "wait"}
            for action in prefix
        )

        if prefix_has_focus_attempt and self._text_focus_review_passed is True:
            actions = actions[input_index:]
            input_index = 0
            prefix = []
            prefix_has_click = False
            prefix_has_focus_attempt = False

        if prefix_has_click or (prefix_has_focus_attempt and self._text_focus_review_passed is not True):
            self._pending_text_focus = {
                "text": expected_text,
                "requested_step": self.n_steps,
                "source": "click" if prefix_has_click else "prefixed_action",
            }
            self._text_focus_review_passed = None
            logger.info(
                "[TextInputGuard] Deferred input until the focus attempt is "
                "visually confirmed editable."
            )
            # Execute the focus/navigation prefix only. The next Brain frame
            # decides whether input_text is appropriate for the active control.
            return prefix

        if input_index < len(actions) - 1:
            logger.info(
                "[TextInputGuard] Deferred post-input actions until pasted text "
                "is visually confirmed stable."
            )
        # Execute only through input_text. Enter/Tab/click/submit follows after
        # the next screenshot confirms that the value remains present.
        return actions[: input_index + 1]

    def _is_once_only_goal(self, goal: str) -> bool:
        goal_lower = (goal or "").lower()
        if any(term in goal_lower for term in ("input", "type", "输入", "文本", "字段", "field")):
            return False
        once_only_terms = (
            "复制",
            "copy",
            "发送",
            "send",
            "提交",
            "submit",
            "预定",
            "schedule",
        )
        return any(term in goal_lower for term in once_only_terms)

    def _allow_failed_action_retry(self, normalized_signature: str) -> bool:
        """Allow a limited number of retries of a just-failed action before blocking.

        A transient failure (click not registered, slow app, network hiccup) is a
        legitimate reason to retry the same control. Only escalate to a no-progress
        loop once the retry quota is exhausted AND the screen has not changed between
        attempts.
        """
        if self._screens_differ():
            # The previous attempt actually changed the screen, so there is fresh
            # evidence; reset this action's retry counter and let it continue.
            self._failed_action_retries.pop(normalized_signature, None)
            return True
        retries = self._failed_action_retries.get(normalized_signature, 0)
        if retries >= self._max_failed_action_retries:
            return False
        if len(self._failed_action_retries) > 16:
            self._failed_action_retries.clear()
        self._failed_action_retries[normalized_signature] = retries + 1
        return True

    def _check_repeated_actions(self, actions: list[Any]) -> Optional[str]:
        if not actions:
            return None
        signatures = [sig for action in actions if (sig := self._action_signature(action))]
        if not signatures:
            return None

        current_signature = "|".join(signatures)
        current_normalized = self._normalized_action_batch_signature(actions)

        # Failed-action retry allowance: a failed step may legitimately retry the
        # same control a few times. Only block once the screen stays unchanged and
        # the retry quota is exhausted. Once-only goals (send/submit/copy) are
        # non-idempotent and are never blindly retried.
        if (
            current_normalized
            and self._recent_normalized_action_signatures
            and self._previous_step_failed()
            and self._recent_normalized_action_signatures[-1] == current_normalized
        ):
            if not self._is_once_only_goal(self.next_goal) and self._allow_failed_action_retry(
                current_normalized
            ):
                return None
            return (
                "No-progress action blocked: the previous step was evaluated as failed, "
                "and the new action targets the same control/area with the same goal. "
                "Do not repeat it; choose a different UI path, refocus/search, wait for "
                "new evidence, or ask for help if the required state is blocked."
            )

        repeated_count = 0
        for previous_signature in reversed(self._recent_action_signatures):
            if previous_signature != current_signature:
                break
            repeated_count += 1

        threshold = 1 if self._is_once_only_goal(self.next_goal) else 2
        if repeated_count >= threshold:
            return (
                f"Repeated action blocked after {repeated_count + 1} identical attempts. "
                "Do not repeat the same click/action again; assume one-shot actions succeeded, "
                "verify the current screen, and move to the next task step or choose a different method."
            )
        if current_normalized and self._recent_normalized_action_signatures:
            normalized_repeat_count = 0
            for previous_normalized in reversed(self._recent_normalized_action_signatures):
                if previous_normalized != current_normalized:
                    break
                normalized_repeat_count += 1
            normalized_threshold = 1 if self._is_once_only_goal(self.next_goal) else 2
            if normalized_repeat_count >= normalized_threshold:
                return (
                    f"Near-duplicate action blocked after {normalized_repeat_count + 1} attempts. "
                    "The target/action is effectively the same even if coordinates or wording "
                    "changed slightly. Inspect the screen and choose a genuinely different strategy."
                )

            recent = list(self._recent_normalized_action_signatures)
            if (
                len(recent) >= 3
                and recent[-2] == current_normalized
                and recent[-1] != current_normalized
                and recent[-3] == recent[-1]
            ):
                return (
                    "Alternating action loop blocked: recent actions are toggling between "
                    "two equivalent states. Stop alternating and choose a new strategy based "
                    "on the current screenshot."
                )
        return None

    def _check_blocked_action_strategy_reuse(self, actions: list[Any]) -> Optional[str]:
        current_strategy = self._action_strategy_batch_signature(actions)
        if not current_strategy or not self._blocked_action_strategy_signatures:
            return None
        if self._screens_differ():
            self._blocked_action_strategy_signatures.clear()
            return None
        blocked = set(str(item) for item in self._blocked_action_strategy_signatures)
        if current_strategy in blocked:
            return (
                "Blocked action strategy reused on an unchanged screen. This exact "
                "interaction strategy was already blocked and must not be tried again "
                "until the UI visibly changes. Choose a different action family, target "
                "area, navigation route, or ask the user if no reliable path is visible."
            )
        current_parts = [
            part for part in current_strategy.split("|")
            if part and not part.startswith("wait:")
        ]
        for previous_strategy in blocked:
            previous_parts = {
                part for part in previous_strategy.split("|")
                if part and not part.startswith("wait:")
            }
            if current_parts and any(part in previous_parts for part in current_parts):
                return (
                    "Blocked action component reused on an unchanged screen. A prior "
                    "blocked attempt already used part of this strategy, so the recovery "
                    "must switch to a materially different control, action family, or "
                    "route instead of combining the same failed component again."
                )
        return None

    def _remember_action_attempt(self, actions: list[Any]) -> None:
        signatures = [sig for action in actions if (sig := self._action_signature(action))]
        if signatures:
            self._recent_action_signatures.append("|".join(signatures))
        normalized_signature = self._normalized_action_batch_signature(actions)
        if normalized_signature:
            self._recent_normalized_action_signatures.append(normalized_signature)

    def _check_text_input_focus(self, actions: list[Any], previous_actions: Optional[list[Any]] = None) -> Optional[str]:
        saw_focus_click = False
        previous_had_focus_click = any(self._is_click_action(action) for action in (previous_actions or []))
        click_count_before_input = 0
        saw_text_focus_action = False
        for action in actions:
            if self._is_select_all_hotkey(action):
                return (
                    "Standalone Ctrl+A was blocked. Never select all at the Actor "
                    "level; click and visually verify the editable field first, "
                    "then let input_text perform guarded replacement."
                )
            action_dict = action.model_dump(exclude_unset=True) if hasattr(action, "model_dump") else action
            if not isinstance(action_dict, dict):
                continue
            for action_name, params in action_dict.items():
                if params is None:
                    continue
                if action_name in {"Click", "DoubleClick"}:
                    saw_focus_click = True
                    click_count_before_input += 1
                    saw_text_focus_action = True
                elif action_name == "Hotkey" and isinstance(params, dict):
                    if split_hotkey(params.get("key", "")) == ["ctrl", "a"]:
                        saw_text_focus_action = True
                elif action_name == "input_text":
                    if self._text_focus_review_passed is False:
                        return (
                            "Text input blocked because the visual focus review did "
                            "not confirm an editable field. Click the correct text "
                            "control or use the selector interaction instead."
                        )
                    if not saw_focus_click:
                        if self._text_focus_review_passed is True:
                            continue
                        if previous_had_focus_click and not self._sensitive_goal_includes_enabling_control(self.next_goal):
                            continue
                        return (
                            "Text input blocked because there is no recent focus click for the target field. "
                            "Click only the exact field first, let Brain visually confirm it is editable, "
                            "then use input_text in the following step."
                        )
                    if (
                        self._sensitive_goal_includes_enabling_control(self.next_goal)
                        and click_count_before_input < 2
                    ):
                        return (
                            "Sensitive text input blocked because the action appears to click an enable/toggle control before typing. "
                            "First enable/show the field, then click the exact password/code input box before input_text."
                        )
        return None

    def _check_text_result_gate(self, actions: list[Any]) -> Optional[str]:
        if self._text_result_review_passed is not False or not actions:
            return None
        review_step = self._text_result_review_step
        if review_step is not None and self.n_steps > int(review_step) + 1:
            logger.info(
                "[TextInputGuard] Clearing stale failed text-result review from step %s before step %s.",
                review_step,
                self.n_steps,
            )
            self._text_result_review_passed = None
            self._text_result_review_step = None
            return None
        if self._screens_differ():
            logger.info("[TextInputGuard] Clearing failed text-result review after visible screen progress.")
            self._text_result_review_passed = None
            self._text_result_review_step = None
            return None
        first_name, _ = self._action_name_and_params(actions[0])
        if first_name in _TEXT_GATE_PASSIVE_ACTIONS:
            return None
        return (
            "Post-input action blocked because visual review did not confirm that "
            "the pasted value remained in the field. Refocus the correct field and "
            "retry input before Enter, Tab, submit, or continuing elsewhere."
        )

    def _check_searchable_selection_strategy(self, actions: list[Any]) -> Optional[str]:
        if not actions or not self._is_searchable_selection_goal(self.next_goal):
            return None
        input_values = self._input_text_values(actions)
        if input_values:
            source_text = self._compact_text_for_match(f"{self.original_task}\n{self.next_goal}")
            for value in input_values:
                compact_value = self._compact_text_for_match(value)
                if len(compact_value) >= 2 and compact_value not in source_text:
                    return (
                        "Searchable selection input blocked because the typed keyword/name does not appear in the original task or current goal. "
                        "Use the exact contact/member/user/file/item name from the task or current goal; do not infer, translate, abbreviate, or alter characters."
                    )
            return None
        if any(self._is_scroll_action(action) for action in actions):
            return (
                "Searchable selection blocked because the action scrolls through a selectable list without searching. "
                "If the target is visible, click it directly. If a search box is visible, click it and use input_text "
                "with the exact target name/keyword before selecting the result. Scroll only after confirming no search box is available or search returned no match."
            )
        return None

    def _is_summary_valid(self, original_text: str, summary_text: str, tier: str) -> bool:
        summary_tokens = self.token_counter.count(summary_text)
        original_tokens = self.token_counter.count(original_text)

        if summary_tokens < 10:
            logger.warning(
                "[Memory] %s summary too short (%d tokens). Keeping original.",
                tier,
                summary_tokens,
            )
            return False
        if original_tokens > 0 and summary_tokens >= original_tokens:
            logger.warning(
                "[Memory] %s summary (%d tokens) not shorter than original (%d tokens). Keeping original.",
                tier,
                summary_tokens,
                original_tokens,
            )
            return False
        return True

    def _extract_response_token_usage(self, response: Any) -> tuple[Optional[int], Optional[int], Optional[int]]:
        usage_candidates = []
        response_metadata = getattr(response, "response_metadata", None)
        if isinstance(response_metadata, dict):
            usage_candidates.append(response_metadata.get("token_usage"))
            usage_candidates.append(response_metadata.get("usage"))
        usage_metadata = getattr(response, "usage_metadata", None)
        if isinstance(usage_metadata, dict):
            usage_candidates.append(usage_metadata)
        if isinstance(response, dict):
            usage_candidates.append(response.get("token_usage"))
            usage_candidates.append(response.get("usage"))

        usage = next((item for item in usage_candidates if isinstance(item, dict)), {})
        if not isinstance(usage, dict):
            return None, None, None

        def _to_int(value: Any) -> Optional[int]:
            try:
                return int(value) if value is not None else None
            except (TypeError, ValueError):
                return None

        prompt_tokens = _to_int(usage.get("prompt_tokens"))
        if prompt_tokens is None:
            prompt_tokens = _to_int(usage.get("input_tokens"))

        completion_tokens = _to_int(usage.get("completion_tokens"))
        if completion_tokens is None:
            completion_tokens = _to_int(usage.get("output_tokens"))

        total_tokens = _to_int(usage.get("total_tokens"))
        if total_tokens is None:
            total_tokens = self.token_counter.count_from_api_usage(response)

        return prompt_tokens, completion_tokens, total_tokens

    def _extract_memory_payload(self, response: Any) -> dict:
        parsed = getattr(response, "parsed", None)
        if isinstance(parsed, dict):
            return parsed
        if isinstance(response, dict):
            return response
        memory_text = str(getattr(response, "content", response))
        cleaned_memory_response = re.sub(r"^```(json)?", "", memory_text.strip())
        cleaned_memory_response = re.sub(r"```$", "", cleaned_memory_response).strip()
        logger.debug("[Memory] Raw text: %s", cleaned_memory_response)
        return json.loads(cleaned_memory_response)

    async def _run_memory_summary(self, memory_text: str, context_label: str) -> tuple[str, str]:
        memory_content = [
            {
                "type": "text",
                "content": f"{context_label}\n\n{memory_text}",
            }
        ]
        self.memory_message_manager._remove_last_state_message()
        self.memory_message_manager._remove_last_AIntool_message()
        self.memory_message_manager.add_state_message(memory_content)
        memory_messages = self.memory_message_manager.get_messages()
        response = await self.memory_llm.ainvoke(memory_messages)
        prompt_tokens, completion_tokens, total_tokens = self._extract_response_token_usage(response)
        estimate_tokens = self.token_counter.count(memory_text)
        if total_tokens:
            logger.info(
                "[Memory] Summary call (%s) token usage | prompt=%s output=%s total=%s estimate_input=%d",
                context_label,
                prompt_tokens if prompt_tokens is not None else "?",
                completion_tokens if completion_tokens is not None else "?",
                total_tokens,
                estimate_tokens,
            )
        if prompt_tokens is not None:
            logger.debug(
                "[Memory] Input token estimate delta (%s): estimate=%d, prompt=%d, delta=%d",
                context_label,
                estimate_tokens,
                prompt_tokens,
                estimate_tokens - prompt_tokens,
            )
        if completion_tokens is not None and completion_tokens > max(128, self.memory_budget_tokens // 2):
            logger.warning(
                "[Memory] Summary output is large (%d tokens). Compression quality may be poor.",
                completion_tokens,
            )
        parsed = self._extract_memory_payload(response)
        summary = str(parsed.get("summary", "")).strip()
        file_name = str(parsed.get("file_name", "")).strip()
        return summary, file_name

    def _save_memory_snapshot(
        self,
        memory_text: str,
        file_name: str,
        source: str,
        step_override: Optional[int] = None,
    ) -> Optional[str]:
        if not memory_text or not self.memory_snapshot_dir:
            return None
        step_value = step_override if step_override is not None else self.n_steps
        safe_name = file_name or f"memory_snapshot_{source}_step_{step_value}.txt"
        saved_name = self.memory_snapshot_store.save(memory_text, safe_name, step=step_value)
        self.memory_snapshot_files.append(
            {
                "file_name": saved_name,
                "source": source,
                "step": step_value,
            }
        )
        return saved_name

    async def _summarise_memory(self) -> None:
        """
        Summarise recent memory to reduce its size without counting summaries in the budget.
        """
        await self._summarise_recent_memory()

    async def _summarise_recent_memory(self, step_override: Optional[int] = None) -> None:
        if not self.recent_memory:
            return
        try:
            summary, file_name = await self._run_memory_summary(
                self.recent_memory,
                "Summarize the following recent-step memory.",
            )
        except Exception:
            logger.exception("[Memory] Failed to summarize recent memory.")
            self._save_memory_snapshot(self.recent_memory, "", "recent", step_override=step_override)
            self._refresh_brain_memory()
            return

        self._save_memory_snapshot(self.recent_memory, file_name, "recent", step_override=step_override)
        if not summary:
            logger.warning("[Memory] Empty summary from memory model; keeping recent memory.")
            self._refresh_brain_memory()
            return
        if not self._is_summary_valid(self.recent_memory, summary, tier="Recent"):
            self._refresh_brain_memory()
            return

        if self.summary_memory:
            self.summary_memory = "\n".join([self.summary_memory, summary]).strip()
        else:
            self.summary_memory = summary
        self.recent_memory = ""
        await self._summarise_summary_memory(step_override=step_override)
        self._refresh_brain_memory()

    async def _summarise_summary_memory(self, step_override: Optional[int] = None) -> None:
        if not self.summary_memory:
            return
        if self.token_counter.count(self.summary_memory) <= self.summary_memory_budget_tokens:
            return
        try:
            summary, file_name = await self._run_memory_summary(
                self.summary_memory,
                "Summarize the following accumulated summaries into a higher-level summary.",
            )
        except Exception:
            logger.exception("[Memory] Failed to summarize accumulated summaries.")
            self._save_memory_snapshot(self.summary_memory, "", "summary", step_override=step_override)
            return

        self._save_memory_snapshot(self.summary_memory, file_name, "summary", step_override=step_override)
        if not summary:
            logger.warning("[Memory] Empty high-level summary; keeping existing summaries.")
            self._refresh_brain_memory()
            return
        if not self._is_summary_valid(self.summary_memory, summary, tier="Higher-level"):
            self._refresh_brain_memory()
            return
        self.summary_memory = summary
        self._refresh_brain_memory()

    async def _update_memory(self) -> None:
        """
        Update memory content.
        """
        sorted_steps = sorted(self.brain_context.keys(), reverse=True)
        if not sorted_steps:
            return
        current_state = self.brain_context[sorted_steps[0]]["current_state"]
        step_goal = current_state["next_goal"] if current_state else None
        step_id = sorted_steps[0]

        # Always write the current step as pending. The success/failed signal for step N
        # arrives in brain_step() of step (N+1). Pending lines do not count toward budget.
        line = f"Step {step_id} | Eval: pending | Goal: {step_goal}"
        result_note = self._format_last_result_for_memory()
        if result_note:
            line = f"{line} | Result: {result_note}"
        pending_lines = [ln for ln in self.pending_recent_memory.splitlines() if ln.strip()]
        pending_lines = [ln for ln in pending_lines if not ln.startswith(f"Step {step_id} |")]
        pending_lines.append(line)
        if len(pending_lines) > MAX_PENDING_LINES:
            pending_lines = pending_lines[-MAX_PENDING_LINES:]
            logger.warning("[Memory] Trimmed pending memory to last %d lines.", MAX_PENDING_LINES)
        self.pending_recent_memory = "\n".join(pending_lines).strip()
        self._refresh_brain_memory()
        self._log_memory_metrics()

    def _format_last_result_for_memory(self, max_chars: int = 300) -> str:
        if not self._last_result:
            return ""
        parts = []
        for item in self._last_result[:3]:
            error = getattr(item, "error", None)
            extracted = getattr(item, "extracted_content", None)
            if error:
                parts.append(f"error={error}")
            elif extracted:
                parts.append(str(extracted))
        return "; ".join(parts)[:max_chars]

    def save_memory(self) -> None:
        """Save the current memory to a file."""
        if not self.save_temp_file_path:
            return
        data = {
            "pid": self.get_last_pid(),
            "task": self.task,
            "next_goal": self.next_goal,
            "last_step_action": self.last_step_action,
            "infor_memory": self.infor_memory,
            "brain_context": self.brain_context,
            "step": self.n_steps,
            "summary_memory": self.summary_memory,
            "pending_recent_memory": self.pending_recent_memory,
            "recent_memory": self.recent_memory,
            "memory_budget_tokens": self.memory_budget_tokens,
            "summary_memory_budget_tokens": self.summary_memory_budget_tokens,
            # Backward-compatible fields
            "memory_budget": self.memory_budget_tokens,
            "summary_memory_budget": self.summary_memory_budget_tokens,
            "recent_memory_tokens": self.token_counter.count(self.recent_memory),
            "summary_memory_tokens": self.token_counter.count(self.summary_memory),
            "pending_memory_tokens": self.token_counter.count(self.pending_recent_memory),
            "total_memory_tokens": self.total_memory_tokens,
            "memory_snapshot_files": self.memory_snapshot_files,
            "completion_guard": self.completion_guard.to_dict(),
            "task_obligations": self.task_obligations,
            "action_evidence": self.action_evidence,
            "workflow_events": self.workflow_events,
            "last_action_guard_block": self._last_action_guard_block,
            "blocked_action_strategy_signatures": list(self._blocked_action_strategy_signatures),
            "grounding_hotspot_failures": self._grounding_hotspot_failures,
            "stalled_failure_count": self._stalled_failure_count,
            "stalled_navigation_count": self._stalled_navigation_count,
            "stalled_navigation_strategy_switch_requested": self._stalled_navigation_strategy_switch_requested,
            "stalled_navigation_block_count": self._stalled_navigation_block_count,
            "stalled_navigation_strategies": sorted(self._stalled_navigation_strategies),
            "pending_text_focus": self._pending_text_focus,
            "pending_text_verification": self._pending_text_verification,
            "text_focus_review_passed": self._text_focus_review_passed,
            "text_result_review_passed": self._text_result_review_passed,
            "text_result_review_step": self._text_result_review_step,
            "completion_prerequisite_block_reason": self._completion_prerequisite_block_reason,
            "completion_prerequisite_block_count": self._completion_prerequisite_block_count,
            "post_action_observation_required": self._post_action_observation_required,
            "deferred_assistance_request": self._deferred_assistance_request,
        }
        file_name = os.path.join(self.save_temp_file_path, "memory.jsonl")
        dir_name = os.path.dirname(file_name)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)
        fd, tmp_path = tempfile.mkstemp(dir=dir_name, suffix=".tmp")
        try:
            with os.fdopen(fd, "w", encoding=self.save_brain_conversation_path_encoding) as f:
                f.write(json.dumps(data, ensure_ascii=False, default=lambda o: list(o) if isinstance(o, set) else o) + "\n")
                # f.flush()
                # os.fsync(f.fileno())
            os.replace(tmp_path, file_name)
        except Exception:
            logger.exception("[Memory] Failed to save memory.")
            if os.path.exists(tmp_path):
                os.unlink(tmp_path)

    async def load_memory(self) -> None:
        """Load the current memory from a file."""
        if not self.save_temp_file_path:
            return
        file_name = os.path.join(self.save_temp_file_path, "memory.jsonl")
        if os.path.exists(file_name):
            with open(file_name, "r", encoding=self.save_brain_conversation_path_encoding) as f:
                lines = f.readlines()
            if len(lines) >= 1:
                data = json.loads(lines[-1])
                self.task = data.get("task", "")
                self.last_pid = data.get("pid", None)
                self.infor_memory = data.get("infor_memory", [])
                self.brain_context = data.get("brain_context", OrderedDict())
                if self.brain_context:
                    self.brain_context = OrderedDict({int(k): v for k, v in self.brain_context.items()})
                    while len(self.brain_context) > MAX_BRAIN_CONTEXT_ENTRIES:
                        self.brain_context.popitem(last=False)
                self.summary_memory = data.get("summary_memory", "")
                self.pending_recent_memory = data.get("pending_recent_memory", "")
                self.recent_memory = data.get("recent_memory", "")
                self.memory_budget_tokens = data.get(
                    "memory_budget_tokens",
                    data.get("memory_budget", self.memory_budget_tokens),
                )
                self.summary_memory_budget_tokens = data.get(
                    "summary_memory_budget_tokens",
                    data.get("summary_memory_budget", self.summary_memory_budget_tokens),
                )
                self.memory_budget_tokens = max(1, int(self.memory_budget_tokens))
                self.summary_memory_budget_tokens = max(1, int(self.summary_memory_budget_tokens))
                self.memory_budget = self.memory_budget_tokens
                self.summary_memory_budget = self.summary_memory_budget_tokens
                self.n_steps = int(data.get("step", 1))
                self.memory_snapshot_files = data.get("memory_snapshot_files", [])
                self.completion_guard.restore(data.get("completion_guard"))
                self.task_obligations = data.get(
                    "task_obligations",
                    self._derive_task_obligations(),
                )
                self.action_evidence = data.get("action_evidence", [])
                self.workflow_events = data.get("workflow_events", [])
                self._last_action_guard_block = data.get("last_action_guard_block")
                blocked_strategy_signatures = data.get("blocked_action_strategy_signatures")
                if isinstance(blocked_strategy_signatures, list):
                    self._blocked_action_strategy_signatures.clear()
                    self._blocked_action_strategy_signatures.extend(
                        str(item) for item in blocked_strategy_signatures[-24:]
                    )
                self._grounding_hotspot_failures = data.get("grounding_hotspot_failures", {})
                self._stalled_failure_count = int(data.get("stalled_failure_count", 0) or 0)
                self._stalled_navigation_count = int(data.get("stalled_navigation_count", 0) or 0)
                self._stalled_navigation_strategy_switch_requested = bool(
                    data.get("stalled_navigation_strategy_switch_requested", False)
                )
                self._stalled_navigation_block_count = int(
                    data.get("stalled_navigation_block_count", 0) or 0
                )
                stalled_strategies = data.get("stalled_navigation_strategies", [])
                if isinstance(stalled_strategies, list):
                    self._stalled_navigation_strategies = {str(item) for item in stalled_strategies}
                self._pending_text_focus = data.get("pending_text_focus")
                self._pending_text_verification = data.get("pending_text_verification")
                self._text_focus_review_passed = data.get(
                    "text_focus_review_passed"
                )
                self._text_result_review_passed = data.get(
                    "text_result_review_passed"
                )
                self._text_result_review_step = data.get("text_result_review_step")
                self._completion_prerequisite_block_reason = data.get(
                    "completion_prerequisite_block_reason"
                )
                self._completion_prerequisite_block_count = int(
                    data.get("completion_prerequisite_block_count", 0) or 0
                )
                self._post_action_observation_required = bool(
                    data.get("post_action_observation_required", False)
                )
                deferred_assistance = data.get("deferred_assistance_request")
                self._deferred_assistance_request = (
                    deferred_assistance if isinstance(deferred_assistance, dict) else None
                )
                # Back-compat: older runs may have stored pending lines in recent_memory.
                if self.recent_memory:
                    recent_lines = [ln for ln in self.recent_memory.splitlines() if ln.strip()]
                    keep_recent: list[str] = []
                    move_pending: list[str] = []
                    for ln in recent_lines:
                        if "| Eval: pending |" in ln:
                            move_pending.append(ln)
                        else:
                            keep_recent.append(ln)
                    if move_pending:
                        self.pending_recent_memory = "\n".join(
                            [ln for ln in [self.pending_recent_memory, "\n".join(move_pending)] if ln]
                        ).strip()
                        self.recent_memory = "\n".join(keep_recent).strip()
                pending_lines = [ln for ln in self.pending_recent_memory.splitlines() if ln.strip()]
                if len(pending_lines) > MAX_PENDING_LINES:
                    self.pending_recent_memory = "\n".join(pending_lines[-MAX_PENDING_LINES:]).strip()
                if "summary_memory" not in data and "recent_memory" not in data:
                    await self._rebuild_memory_from_context()
                else:
                    self._refresh_brain_memory()
                self._log_memory_metrics()
                self.last_step_action = data.get("last_step_action", None)
                self.next_goal = data.get("next_goal", "")
                logger.info("Loaded memory from %s", file_name)

    async def _rebuild_memory_from_context(self) -> None:
        self.summary_memory = ""
        self.recent_memory = ""
        self.memory_snapshot_files = []
        for step_id in sorted(self.brain_context.keys()):
            current_state = self.brain_context[step_id].get("current_state", {})
            evaluation = current_state.get("step_evaluate")
            step_goal = current_state.get("next_goal")
            line = f"Step {step_id} | Eval: {evaluation} | Goal: {step_goal}"
            self.recent_memory = "\n".join([ln for ln in [self.recent_memory, line] if ln]).strip()
            if self.token_counter.count(self.recent_memory) > self.memory_budget_tokens:
                await self._summarise_recent_memory(step_override=step_id)
        self._refresh_brain_memory()
        self._log_memory_metrics()

    @time_execution_async("--brain_step")
    async def brain_step(self) -> dict:
        step_id = self.n_steps
        focus_check_was_pending = self._pending_text_focus is not None
        text_check_was_pending = self._pending_text_verification is not None
        dispatch_check_was_pending = self._pending_content_dispatch_verification is not None
        self._step_errors.pop(step_id, None)
        if session_logger:
            session_logger.step_start(step_id, self.next_goal or "初始化任务分析")

        # 初始化步骤计时（如果还没有）
        if not hasattr(self, '_step_timing'):
            self._step_timing = {}
        self._step_timing[step_id] = {
            "step_start": time.time(),
            "brain_start": 0,
            "brain_end": 0,
            "brain_step_end": 0,
        }
        
        step_start_time = time.time()
        prev_step_id = step_id - 1
        
        try:
            # Screenshot timing
            screenshot_start = time.time()
            self._step_timing[step_id]["screenshot_start"] = screenshot_start
            self.previous_screenshot = self.screenshot_annotated
            screenshot, screenshot_timings = _capture_screenshot_with_breakdown()
            self._step_timing[step_id].update(screenshot_timings)
            downscale_start = time.time()
            screenshot = downscale_screenshot_by_tier(screenshot)
            self._step_timing[step_id]["screenshot_downscale_time"] = time.time() - downscale_start
            self.screenshot_annotated = screenshot
            save_start = time.time()
            current_screenshot_path = self._screenshot_path_for_step(step_id)
            current_screenshot_path.parent.mkdir(parents=True, exist_ok=True)
            screenshot.save(current_screenshot_path)
            self._current_screenshot_path = str(current_screenshot_path)
            self._step_timing[step_id]["screenshot_save_time"] = time.time() - save_start
            screenshot_end = time.time()
            screenshot_time = screenshot_end - screenshot_start
            self._step_timing[step_id]["screenshot_end"] = screenshot_end
            logger.debug(f"[Timing] Step {step_id}: Screenshot captured in {screenshot_time:.2f}s")

            # 记录截图时间供保存使用
            self._last_screenshot_time = screenshot_time

            # 更新状态窗口 - 截图完成
            self._add_log_window(f"步骤 {step_id}：截图已完成（{screenshot_time:.2f}s）", "timing")
            self._update_timing(step_id, screenshot_time)

            screenshot_dataurl = None
            previous_screenshot_dataurl = None
            if self.screenshot_annotated:
                screenshot_dataurl = screenshot_to_dataurl(self.screenshot_annotated)
            if self.brain_include_previous_screenshot and self.previous_screenshot:
                previous_screenshot_dataurl = screenshot_to_dataurl(self.previous_screenshot)
            info_files = "\n".join(str(item) for item in self.infor_memory) if self.infor_memory else "None"

            def build_state_content(
                read_files_content: Optional[str] = None,
                read_files_list: Optional[list[str]] = None,
            ) -> list[dict]:
                if step_id >= 2:
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                f"Previous step is {prev_step_id}.\n\n"
                                f"Recorded info files (filenames only):\n{info_files}\n\n"
                                f"Previous Actions Short History:\n{self.brain_memory}\n\n"
                            ),
                        }
                    ]
                else:
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                "This is the first step.\n\n"
                                "You should provide a JSON with a well-defined goal based on images information. "
                                "The other fields should be default value."
                            ),
                        }
                    ]
                if read_files_content:
                    files_label = ", ".join(read_files_list) if read_files_list else ""
                    read_label = (
                        f"Requested file contents for: {files_label}\n" if files_label else "Requested file contents:\n"
                    )
                    state_content.append(
                        {
                            "type": "text",
                            "content": f"{read_label}{read_files_content}",
                        }
                    )
                if self.completion_guard.pending:
                    state_content.append(
                        {
                            "type": "text",
                            "content": self.completion_guard.review_instruction(),
                        }
                    )
                if self._pending_text_focus:
                    expected = str(self._pending_text_focus.get("text", ""))
                    if self._is_sensitive_text_goal(self.next_goal):
                        expected_description = (
                            f"a sensitive value of length {len(expected)}"
                        )
                    else:
                        expected_description = json.dumps(
                            expected, ensure_ascii=False
                        )
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "TEXT INPUT FOCUS CHECK IS ACTIVE. Inspect the newest "
                                "screenshot after the click. Decide whether the focused "
                                "control is visibly an editable text field (caret, editable "
                                "text area, or other clear editing state), rather than a "
                                "read-only selector, dropdown button, list item, or unrelated "
                                "window. Only if editability is visually supported may the "
                                f"next goal use input_text with {expected_description}. "
                                "Otherwise mark the prior focus attempt Failed and choose the "
                                "correct control or selector interaction. Do not use Ctrl+A."
                            ),
                        }
                    )
                if self._pending_text_verification:
                    expected = str(
                        self._pending_text_verification.get("text", "")
                    )
                    sensitive = bool(
                        self._pending_text_verification.get("sensitive", False)
                    )
                    evidence = (
                        f"masked/non-empty content consistent with length {len(expected)}"
                        if sensitive
                        else f"the exact intended value {json.dumps(expected, ensure_ascii=False)}"
                    )
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "TEXT INPUT RESULT CHECK IS ACTIVE. Inspect the newest "
                                f"screenshot and confirm the field still contains {evidence}. "
                                "Do not continue to Enter, Tab, submit, or another field if "
                                "the value disappeared, was truncated, entered in the wrong "
                                "control, or triggered a dropdown selection. Mark the prior "
                                "step Failed and refocus/retry when necessary."
	                            ),
	                        }
	                    )
                if self._pending_content_dispatch_verification:
                    original_task = self._original_task_description()
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "CONTENT DISPATCH RESULT CHECK IS ACTIVE. The previous step performed a "
                                "send/submit/share/post-style action. Inspect the newest screenshot for "
                                f"fresh evidence that the user's original task {original_task} has been completed "
                                "after that action. Do not treat text typed into contact/member/recipient/file "
                                "search boxes as delivered content. Do not rely only on the button click, stale "
                                "history, an empty input box, or old matching content. If the original task outcome "
                                "is missing, an error is visible, or the result is ambiguous, mark the previous step "
                                "Failed and choose a corrective goal."
                            ),
                        }
                    )
                _append_labeled_screenshot_frames(
                    state_content,
                    previous_screenshot_dataurl=(
                        previous_screenshot_dataurl if step_id >= 2 else None
                    ),
                    current_screenshot_dataurl=screenshot_dataurl,
                )
                return state_content

            state_content = build_state_content()
            self.brain_message_manager._remove_last_state_message()
            self.brain_message_manager._remove_last_AIntool_message()
            self.brain_message_manager.add_state_message(state_content)
            brain_messages = self.brain_message_manager.get_messages()

            # Brain model inference timing
            brain_inference_start = time.time()
            self._step_timing[step_id]["brain_start"] = brain_inference_start
            try:
                response = await self.brain_llm.ainvoke(brain_messages)
            finally:
                brain_inference_end = time.time()
                self._step_timing[step_id]["brain_end"] = brain_inference_end
            brain_inference_time = brain_inference_end - brain_inference_start
            logger.debug(f"[Timing] Step {step_id}: Brain model inference completed in {brain_inference_time:.2f}s")
            
            # 记录 Brain 推理时间
            self._last_brain_inference_time = brain_inference_time
            
            # 更新状态窗口 - Brain 推理完成
            self._add_log_window(f"步骤 {step_id}：分析模型推理（{brain_inference_time:.2f}s）", "timing")
            self._add_detail_window(f"分析模型推理：{brain_inference_time:.2f}s")

            # 记录 LLM 响应到会话日志
            if session_logger:
                prompt_tokens, completion_tokens, total_tokens = self._extract_response_token_usage(response)
                session_logger.log_llm_response(
                    "Brain",
                    str(response.content)[:500] if response.content else "N/A",
                    tokens=(prompt_tokens, completion_tokens, total_tokens) if total_tokens else None,
                    elapsed=brain_inference_time
                )

            # Try to use parsed response if available (for local models)
            if hasattr(response, "parsed") and response.parsed is not None:
                logger.debug(f"[Brain] Using response.parsed: %s", type(response.parsed).__name__)
                parsed = response.parsed
            else:
                logger.debug(f"[Brain] response.parsed is None or not available, parsing from content")
                parsed = self.brain_search.parse_response(str(response.content))
            
            logger.debug(f"[Brain] Parsed type: %s, value preview: %s", type(parsed).__name__, str(parsed)[:200])

            parsed, brain_messages = await self.brain_search.maybe_reinvoke(
                parsed,
                build_state_content,
                self.brain_message_manager,
                self.brain_llm,
            )
            if "current_state" not in parsed or "analysis" not in parsed:
                raise ValueError("Brain response missing required fields after read-files handling.")
            self._save_brain_conversation(brain_messages, parsed, step=self.n_steps)
            self.brain_context[self.n_steps] = parsed
            while len(self.brain_context) > MAX_BRAIN_CONTEXT_ENTRIES:
                self.brain_context.popitem(last=False)

            self.next_goal = parsed["current_state"]["next_goal"]
            self.brain_thought = parsed["analysis"]
            self.current_state = parsed["current_state"]

            # The newest screenshot has now been reviewed by Brain. Clear the
            # consumed gates; actor staging below may create the next gate.
            if focus_check_was_pending:
                focus_evaluation = str(
                    self.current_state.get("step_evaluate", "")
                ).lower()
                self._text_focus_review_passed = (
                    "success" in focus_evaluation or "成功" in focus_evaluation
                )
                self.execution_journal.metrics.text_focus_visual_checks += 1
                self.execution_journal.event(
                    "text_focus_visually_reviewed",
                    step=step_id,
                    evaluation=self.current_state.get("step_evaluate", ""),
                )
                self._pending_text_focus = None
            if text_check_was_pending:
                text_evaluation = str(
                    self.current_state.get("step_evaluate", "")
                ).lower()
                self._text_result_review_passed = (
                    "success" in text_evaluation or "成功" in text_evaluation
                )
                self._text_result_review_step = step_id
                self.execution_journal.metrics.text_result_visual_checks += 1
                self.execution_journal.event(
                    "text_result_visually_reviewed",
                    step=step_id,
                    evaluation=self.current_state.get("step_evaluate", ""),
                )
                self._pending_text_verification = None
            if dispatch_check_was_pending:
                self._handle_content_dispatch_review_result(parsed, step=step_id)
            if self._complete_post_action_observation(step=step_id):
                return
            self._maybe_request_user_assistance_from_state()

            # Log total brain step time
            brain_step_end = time.time()
            brain_step_total = brain_step_end - step_start_time
            logger.debug(f"[Timing] Step {step_id}: Brain step completed in {brain_step_total:.2f}s")
            
            # 记录 Brain 总时间
            self._last_brain_total_time = brain_step_total
            self._step_timing[step_id]["brain_step_end"] = brain_step_end  # brain_step 结束时间
            
            # 更新状态窗口 - 显示 brain 分析结果和耗时
            self._add_log_window(f"步骤 {step_id}：分析模型处理完成（{brain_step_total:.2f}s）", "timing")
            self._add_detail_window(f"分析模型总耗时：{brain_step_total:.2f}s")
            self._update_status_window(
                step_id,
                self.next_goal[:80] + "..." if len(self.next_goal) > 80 else self.next_goal,
                "running"
            )

            # Finalize the previous step's memory line based on this response's evaluation signal.
            # Keep step N in pending_recent_memory until step (N+1) arrives, so it won't be summarized away.
            if prev_step_id >= 1:
                raw_eval = str(self.current_state.get("step_evaluate", "")).lower()
                if "success" in raw_eval:
                    final_status = "success"
                elif "fail" in raw_eval:
                    final_status = "failed"
                else:
                    final_status = "pending"

                pending_lines = [ln for ln in self.pending_recent_memory.splitlines() if ln.strip()]
                new_pending: list[str] = []
                goal_text: Optional[str] = None
                for ln in pending_lines:
                    if ln.startswith(f"Step {prev_step_id} |"):
                        if "| Goal: " in ln:
                            goal_text = ln.split("| Goal: ", 1)[1].strip()
                            if " | Result: " in goal_text:
                                goal_text = goal_text.split(" | Result: ", 1)[0].strip()
                        continue
                    new_pending.append(ln)
                self.pending_recent_memory = "\n".join(new_pending).strip()

                if goal_text is not None:
                    if final_status == "success":
                        self._record_workflow_event("completed", step=prev_step_id, goal=goal_text)
                        self._record_grounding_feedback(prev_step_id, final_status, goal_text)
                        self._last_action_guard_block = None
                        self._blocked_action_strategy_signatures.clear()
                        self._failed_action_retries.clear()
                    elif final_status == "failed":
                        self._record_workflow_event("failed", step=prev_step_id, goal=goal_text)
                        self._record_grounding_feedback(prev_step_id, final_status, goal_text)
                    final_line = f"Step {prev_step_id} | Eval: {final_status} | Goal: {goal_text}"
                    recent_lines = [ln for ln in self.recent_memory.splitlines() if ln.strip()]
                    recent_lines = [ln for ln in recent_lines if not ln.startswith(f"Step {prev_step_id} |")]
                    recent_lines.append(final_line)
                    self.recent_memory = "\n".join(recent_lines).strip()
                    recent_tokens = self.token_counter.count(self.recent_memory)
                    hard_limit = int(self.memory_budget_tokens * self.memory_hard_ratio)
                    warn_limit = int(self.memory_budget_tokens * self.memory_warn_ratio)
                    if recent_tokens > hard_limit:
                        await self._summarise_recent_memory(step_override=prev_step_id)
                    elif recent_tokens > warn_limit:
                        logger.info(
                            "[Memory] Recent memory at %.0f%% of budget (%d/%d tokens). Compression will trigger soon.",
                            (recent_tokens / self.memory_budget_tokens) * 100,
                            recent_tokens,
                            self.memory_budget_tokens,
                        )
                        self._refresh_brain_memory()
                    else:
                        self._refresh_brain_memory()
                else:
                    self._refresh_brain_memory()
            self._log_memory_metrics()

        except Exception as e:
            now = time.time()
            timing = self._step_timing.get(step_id, {})
            if timing.get("brain_start") and not timing.get("brain_end"):
                timing["brain_end"] = now
            timing["brain_step_end"] = now
            timing["brain_error"] = str(e)[:500]
            self._step_errors[step_id] = f"Brain error: {str(e)[:500]}"
            logger.exception("[Brain] Unexpected error in brain_step.")
            return {"Brain_text": {"step_evaluate": "unknown", "reason": str(e)}}

    def _archive_invalid_combined_response(
        self,
        raw_response: str,
        error: Exception,
        *,
        attempt: int,
    ) -> Optional[Path]:
        """Keep malformed Combined output in the run-isolated artifact directory."""
        if not self.save_temp_file_path:
            return None
        try:
            archive_path = Path(self.save_temp_file_path) / "combined_response_failures.jsonl"
            archive_path.parent.mkdir(parents=True, exist_ok=True)
            raw_response = raw_response or "<empty response>"
            record = {
                "timestamp": datetime.now().isoformat(timespec="milliseconds"),
                "step": self.n_steps,
                "attempt": attempt,
                "error_type": type(error).__name__,
                "error": str(error)[:2000],
                "response_truncated": len(raw_response) > _COMBINED_RESPONSE_ARCHIVE_LIMIT,
                "raw_response": raw_response[:_COMBINED_RESPONSE_ARCHIVE_LIMIT],
            }
            with archive_path.open("a", encoding="utf-8") as archive:
                archive.write(json.dumps(record, ensure_ascii=False) + "\n")
            return archive_path
        except Exception as archive_error:
            logger.warning(
                "[Combined] Could not archive invalid response: %s",
                archive_error,
            )
            return None

    @time_execution_async("--combined_step")
    async def combined_step(self) -> None:
        if not self.combined_llm or not self.combined_message_manager:
            await self.brain_step()
            await self.actor_step()
            return

        step_id = self.n_steps
        focus_check_was_pending = self._pending_text_focus is not None
        text_check_was_pending = self._pending_text_verification is not None
        dispatch_check_was_pending = self._pending_content_dispatch_verification is not None
        self._step_errors.pop(step_id, None)
        if session_logger:
            session_logger.step_start(step_id, self.next_goal or "初始化任务分析")

        if not hasattr(self, '_step_timing'):
            self._step_timing = {}
        self._step_timing[step_id] = {
            "step_start": time.time(),
            "brain_start": 0,
            "brain_end": 0,
            "brain_step_end": 0,
        }

        step_start_time = time.time()
        prev_step_id = step_id - 1

        try:
            screenshot_start = time.time()
            self._step_timing[step_id]["screenshot_start"] = screenshot_start
            self.previous_screenshot = self.screenshot_annotated
            screenshot, screenshot_timings = _capture_screenshot_with_breakdown()
            self._step_timing[step_id].update(screenshot_timings)
            downscale_start = time.time()
            screenshot = downscale_screenshot_by_tier(screenshot)
            self._step_timing[step_id]["screenshot_downscale_time"] = time.time() - downscale_start
            self.screenshot_annotated = screenshot
            save_start = time.time()
            current_screenshot_path = self._screenshot_path_for_step(step_id)
            current_screenshot_path.parent.mkdir(parents=True, exist_ok=True)
            screenshot.save(current_screenshot_path)
            self._current_screenshot_path = str(current_screenshot_path)
            self._step_timing[step_id]["screenshot_save_time"] = time.time() - save_start
            screenshot_end = time.time()
            screenshot_time = screenshot_end - screenshot_start
            self._step_timing[step_id]["screenshot_end"] = screenshot_end
            self._last_screenshot_time = screenshot_time
            self._add_log_window(f"步骤 {step_id}：截图已完成（{screenshot_time:.2f}s）", "timing")
            self._update_timing(step_id, screenshot_time)

            screenshot_dataurl = screenshot_to_dataurl(self.screenshot_annotated)
            previous_screenshot_dataurl = (
                screenshot_to_dataurl(self.previous_screenshot)
                if self.brain_include_previous_screenshot and self.previous_screenshot
                else None
            )
            info_files = "\n".join(str(item) for item in self.infor_memory) if self.infor_memory else "None"

            def build_state_content(
                read_files_content: Optional[str] = None,
                read_files_list: Optional[list[str]] = None,
            ) -> list[dict]:
                if step_id >= 2:
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                f"Previous step is {prev_step_id}.\n\n"
                                f"Recorded info files (filenames only):\n{info_files}\n\n"
                                f"Previous Actions Short History:\n{self.brain_memory}\n\n"
                            ),
                        }
                    ]
                else:
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                "This is the first step. Analyze the current screen, "
                                "choose the first executable goal, and output the action for it."
                            ),
                        }
                    ]
                if read_files_content:
                    files_label = ", ".join(read_files_list) if read_files_list else ""
                    read_label = (
                        f"Requested file contents for: {files_label}\n" if files_label else "Requested file contents:\n"
                    )
                    state_content.append({"type": "text", "content": f"{read_label}{read_files_content}"})
                if self.completion_guard.pending:
                    state_content.append({"type": "text", "content": self.completion_guard.review_instruction()})
                if self._pending_text_focus:
                    expected = str(self._pending_text_focus.get("text", ""))
                    expected_description = (
                        f"a sensitive value of length {len(expected)}"
                        if self._is_sensitive_text_goal(self.next_goal)
                        else json.dumps(expected, ensure_ascii=False)
                    )
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "TEXT INPUT FOCUS CHECK IS ACTIVE. Inspect the newest screenshot after the click. "
                                "Only if editability is visually supported may action use input_text with "
                                f"{expected_description}. Otherwise mark the prior focus attempt Failed and choose "
                                "the correct control or selector interaction. Do not use Ctrl+A."
                            ),
                        }
                    )
                if self._pending_text_verification:
                    expected = str(self._pending_text_verification.get("text", ""))
                    sensitive = bool(self._pending_text_verification.get("sensitive", False))
                    evidence = (
                        f"masked/non-empty content consistent with length {len(expected)}"
                        if sensitive
                        else f"the exact intended value {json.dumps(expected, ensure_ascii=False)}"
                    )
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "TEXT INPUT RESULT CHECK IS ACTIVE. Inspect the newest screenshot and confirm the "
                                f"field still contains {evidence}. Do not continue to Enter, Tab, submit, or another "
                                "field if the value disappeared, was truncated, entered in the wrong control, or "
                                "triggered a dropdown selection."
	                            ),
	                        }
	                    )
                if self._pending_content_dispatch_verification:
                    original_task = self._original_task_description()
                    state_content.append(
                        {
                            "type": "text",
                            "content": (
                                "CONTENT DISPATCH RESULT CHECK IS ACTIVE. The previous step performed a "
                                "send/submit/share/post-style action. Inspect the newest screenshot for "
                                f"fresh evidence that the user's original task {original_task} has been completed "
                                "after that action. Do not treat text typed into contact/member/recipient/file "
                                "search boxes as delivered content. Do not rely only on the button click, stale "
                                "history, an empty input box, or old matching content. If the original task outcome "
                                "is missing, an error is visible, or the result is ambiguous, mark the previous step "
                                "Failed and choose a corrective goal."
                            ),
                        }
                    )
                _append_labeled_screenshot_frames(
                    state_content,
                    previous_screenshot_dataurl=(
                        previous_screenshot_dataurl if step_id >= 2 else None
                    ),
                    current_screenshot_dataurl=screenshot_dataurl,
                )
                return state_content

            state_content = build_state_content()
            self.combined_message_manager._remove_last_state_message()
            self.combined_message_manager._remove_last_AIntool_message()
            self.combined_message_manager.add_state_message(state_content)
            combined_messages = self.combined_message_manager.get_messages()

            brain_inference_start = time.time()
            self._step_timing[step_id]["brain_start"] = brain_inference_start
            try:
                response = await self.combined_llm.ainvoke(combined_messages)
            finally:
                brain_inference_end = time.time()
                self._step_timing[step_id]["brain_end"] = brain_inference_end
            brain_inference_time = brain_inference_end - brain_inference_start
            self._last_brain_inference_time = brain_inference_time
            logger.debug(
                "[Timing] Step %s: Combined model inference completed in %.2fs",
                step_id,
                brain_inference_time,
            )

            if session_logger:
                prompt_tokens, completion_tokens, total_tokens = self._extract_response_token_usage(response)
                session_logger.log_llm_response(
                    "Combined",
                    _combined_response_text(response)[:500],
                    tokens=(prompt_tokens, completion_tokens, total_tokens) if total_tokens else None,
                    elapsed=brain_inference_time,
                )

            async def parse_and_validate_combined(
                model_response: Any,
                *,
                label: str,
                source_messages: list[BaseMessage],
                preserve_source_messages: bool = False,
            ) -> tuple[dict, list[BaseMessage]]:
                if hasattr(model_response, "parsed") and model_response.parsed is not None:
                    candidate = model_response.parsed
                else:
                    candidate = self.brain_search.parse_response(
                        _combined_response_text(model_response),
                        label=label,
                    )

                requested_files = (
                    self.brain_search.extract_read_files(candidate)
                    if isinstance(candidate, dict)
                    else None
                )
                candidate, effective_messages = await self.brain_search.maybe_reinvoke(
                    candidate,
                    build_state_content,
                    self.combined_message_manager,
                    self.combined_llm,
                )
                if not self._combined_provider_schema_enabled:
                    candidate = _normalize_prompt_only_combined_payload(
                        candidate,
                        default_sop_check=(
                            None
                            if getattr(self, "selected_skills", [])
                            else "None"
                        ),
                    )
                if preserve_source_messages and not requested_files:
                    effective_messages = source_messages
                return _validate_combined_payload(candidate), effective_messages

            raw_response = _combined_response_text(response)
            recoverable_output_errors = (ValueError, TypeError, KeyError)
            try:
                parsed, combined_messages = await parse_and_validate_combined(
                    response,
                    label="Combined",
                    source_messages=combined_messages,
                )
            except recoverable_output_errors as first_error:
                archive_path = self._archive_invalid_combined_response(
                    raw_response,
                    first_error,
                    attempt=1,
                )
                archive_note = f" archived={archive_path}" if archive_path else ""
                logger.warning(
                    "[Combined] Invalid model output at step %s (%s); retrying once with an explicit schema reminder.%s",
                    step_id,
                    first_error,
                    archive_note,
                )
                retry_messages = _build_combined_retry_messages(
                    combined_messages,
                    raw_response,
                    str(first_error),
                )
                retry_start = time.time()
                try:
                    retry_response = await self.combined_llm.ainvoke(retry_messages)
                finally:
                    retry_end = time.time()
                    retry_time = retry_end - retry_start
                    self._step_timing[step_id]["combined_retry_time"] = retry_time
                    self._step_timing[step_id]["brain_end"] = retry_end
                    brain_inference_time += retry_time
                    self._last_brain_inference_time = brain_inference_time

                if session_logger:
                    retry_prompt, retry_completion, retry_total = self._extract_response_token_usage(
                        retry_response
                    )
                    session_logger.log_llm_response(
                        "Combined retry",
                        _combined_response_text(retry_response)[:500],
                        tokens=(retry_prompt, retry_completion, retry_total) if retry_total else None,
                        elapsed=retry_time,
                    )

                retry_raw_response = _combined_response_text(retry_response)
                try:
                    parsed, combined_messages = await parse_and_validate_combined(
                        retry_response,
                        label="Combined retry",
                        source_messages=retry_messages,
                        preserve_source_messages=True,
                    )
                except recoverable_output_errors as retry_error:
                    self._archive_invalid_combined_response(
                        retry_raw_response,
                        retry_error,
                        attempt=2,
                    )
                    raise ValueError(
                        "Combined response remained invalid after one constrained retry: "
                        f"{retry_error}"
                    ) from retry_error

            self._save_brain_conversation(combined_messages, parsed, step=self.n_steps)
            self.brain_context[self.n_steps] = {
                "analysis": parsed["analysis"],
                "current_state": parsed["current_state"],
            }
            while len(self.brain_context) > MAX_BRAIN_CONTEXT_ENTRIES:
                self.brain_context.popitem(last=False)

            self.next_goal = parsed["current_state"]["next_goal"]
            self.brain_thought = parsed["analysis"]
            self.current_state = parsed["current_state"]

            if focus_check_was_pending:
                focus_evaluation = str(self.current_state.get("step_evaluate", "")).lower()
                self._text_focus_review_passed = "success" in focus_evaluation or "成功" in focus_evaluation
                self.execution_journal.metrics.text_focus_visual_checks += 1
                self.execution_journal.event(
                    "text_focus_visually_reviewed",
                    step=step_id,
                    evaluation=self.current_state.get("step_evaluate", ""),
                )
                self._pending_text_focus = None
            if text_check_was_pending:
                text_evaluation = str(self.current_state.get("step_evaluate", "")).lower()
                self._text_result_review_passed = "success" in text_evaluation or "成功" in text_evaluation
                self._text_result_review_step = step_id
                self.execution_journal.metrics.text_result_visual_checks += 1
                self.execution_journal.event(
                    "text_result_visually_reviewed",
                    step=step_id,
                    evaluation=self.current_state.get("step_evaluate", ""),
                )
                self._pending_text_verification = None
            if dispatch_check_was_pending:
                self._handle_content_dispatch_review_result(parsed, step=step_id)
            if self._complete_post_action_observation(step=step_id):
                return
            if self._maybe_request_user_assistance_from_state():
                return

            brain_step_end = time.time()
            brain_step_total = brain_step_end - step_start_time
            self._last_brain_total_time = brain_step_total
            self._step_timing[step_id]["brain_step_end"] = brain_step_end
            logger.debug(
                "[Timing] Step %s: Combined analysis completed in %.2fs",
                step_id,
                brain_step_total,
            )
            self._update_status_window(
                step_id,
                self.next_goal[:80] + "..." if len(self.next_goal) > 80 else self.next_goal,
                "running",
            )

            if prev_step_id >= 1:
                raw_eval = str(self.current_state.get("step_evaluate", "")).lower()
                if "success" in raw_eval:
                    final_status = "success"
                elif "fail" in raw_eval:
                    final_status = "failed"
                else:
                    final_status = "pending"

                pending_lines = [ln for ln in self.pending_recent_memory.splitlines() if ln.strip()]
                new_pending: list[str] = []
                goal_text: Optional[str] = None
                for ln in pending_lines:
                    if ln.startswith(f"Step {prev_step_id} |"):
                        if "| Goal: " in ln:
                            goal_text = ln.split("| Goal: ", 1)[1].strip()
                            if " | Result: " in goal_text:
                                goal_text = goal_text.split(" | Result: ", 1)[0].strip()
                        continue
                    new_pending.append(ln)
                self.pending_recent_memory = "\n".join(new_pending).strip()

                if goal_text is not None:
                    if final_status == "success":
                        self._record_workflow_event("completed", step=prev_step_id, goal=goal_text)
                        self._record_grounding_feedback(prev_step_id, final_status, goal_text)
                        self._last_action_guard_block = None
                        self._blocked_action_strategy_signatures.clear()
                        self._failed_action_retries.clear()
                    elif final_status == "failed":
                        self._record_workflow_event("failed", step=prev_step_id, goal=goal_text)
                        self._record_grounding_feedback(prev_step_id, final_status, goal_text)
                    final_line = f"Step {prev_step_id} | Eval: {final_status} | Goal: {goal_text}"
                    recent_lines = [ln for ln in self.recent_memory.splitlines() if ln.strip()]
                    recent_lines = [ln for ln in recent_lines if not ln.startswith(f"Step {prev_step_id} |")]
                    recent_lines.append(final_line)
                    self.recent_memory = "\n".join(recent_lines).strip()
                    recent_tokens = self.token_counter.count(self.recent_memory)
                    hard_limit = int(self.memory_budget_tokens * self.memory_hard_ratio)
                    warn_limit = int(self.memory_budget_tokens * self.memory_warn_ratio)
                    if recent_tokens > hard_limit:
                        await self._summarise_recent_memory(step_override=prev_step_id)
                    elif recent_tokens > warn_limit:
                        logger.info(
                            "[Memory] Recent memory at %.0f%% of budget (%d/%d tokens). Compression will trigger soon.",
                            (recent_tokens / self.memory_budget_tokens) * 100,
                            recent_tokens,
                            self.memory_budget_tokens,
                        )
                        self._refresh_brain_memory()
                    else:
                        self._refresh_brain_memory()
                else:
                    self._refresh_brain_memory()
            self._log_memory_metrics()

            if self._stopped:
                return

            model_output = self._coerce_action_record(parsed)
            self._log_response(model_output)
            self.combined_message_manager._remove_last_state_message()
            self.combined_message_manager.add_model_output(model_output)
            await self.actor_step(
                precomputed_model_output=model_output,
                precomputed_raw=parsed,
                precomputed_messages=combined_messages,
            )

        except Exception as e:
            now = time.time()
            timing = self._step_timing.get(step_id, {})
            if timing.get("brain_start") and not timing.get("brain_end"):
                timing["brain_end"] = now
            timing["brain_step_end"] = now
            timing["brain_error"] = str(e)[:500]
            self._step_errors[step_id] = f"Combined error: {str(e)[:500]}"
            logger.exception("[Combined] Unexpected error in combined_step; falling back to split step once.")
            await self.brain_step()
            await self.actor_step()

    def _execution_checkpoint_state(self, phase: str) -> dict[str, Any]:
        return {
            "phase": phase,
            "task": self.task,
            "step": self.n_steps,
            "next_goal": self.next_goal,
            "last_step_action": self.last_step_action,
            "completion_guard": self.completion_guard.to_dict(),
            "task_obligations": self.task_obligations,
            "action_evidence": self.action_evidence,
            "workflow_events": self.workflow_events,
            "last_action_guard_block": self._last_action_guard_block,
            "blocked_action_strategy_signatures": list(self._blocked_action_strategy_signatures),
            "grounding_hotspot_failures": self._grounding_hotspot_failures,
            "recent_normalized_action_signatures": list(self._recent_normalized_action_signatures),
            "no_progress_count": self._no_progress_count,
            "last_no_progress_signature": self._last_no_progress_signature,
            "stalled_failure_count": self._stalled_failure_count,
            "stalled_navigation_count": self._stalled_navigation_count,
            "stalled_navigation_strategy_switch_requested": self._stalled_navigation_strategy_switch_requested,
            "stalled_navigation_block_count": self._stalled_navigation_block_count,
            "stalled_navigation_strategies": sorted(self._stalled_navigation_strategies),
            "recent_step_health": list(self._recent_step_health),
            "latest_screenshot": getattr(self, "_current_screenshot_path", None),
            "pending_text_focus": self._pending_text_focus,
            "pending_text_verification": self._pending_text_verification,
            "pending_content_dispatch_verification": self._pending_content_dispatch_verification,
            "text_focus_review_passed": self._text_focus_review_passed,
            "text_result_review_passed": self._text_result_review_passed,
            "text_result_review_step": self._text_result_review_step,
            "content_dispatch_review_passed": self._content_dispatch_review_passed,
            "completion_prerequisite_block_reason": self._completion_prerequisite_block_reason,
            "completion_prerequisite_block_count": self._completion_prerequisite_block_count,
            "post_action_observation_required": self._post_action_observation_required,
            "deferred_assistance_request": self._deferred_assistance_request,
            "assistance_request": self.assistance_request,
        }

    async def _handle_completion_gate(self) -> bool:
        """Handle completion claims without invoking the Actor model.

        Returns True when this step was fully handled by the gate.
        """
        completion_requested = bool(
            self.next_goal and "[TASK_COMPLETE]" in self.next_goal
        )
        was_pending = self.completion_guard.pending
        step_evaluate = (
            self.current_state.get("step_evaluate", "")
            if isinstance(getattr(self, "current_state", None), dict)
            else ""
        )
        decision = self.completion_guard.evaluate(
            completion_requested=completion_requested,
            step_evaluate=step_evaluate,
            step=self.n_steps,
        )

        if was_pending and not completion_requested:
            self.execution_journal.metrics.verification_rejections += 1
            self.execution_journal.event(
                "completion_rejected",
                step=self.n_steps,
                next_goal=self.next_goal,
            )

        if not completion_requested:
            return False

        prerequisite_error = self._completion_prerequisite_error()
        if prerequisite_error:
            block_count = self._record_completion_prerequisite_block(prerequisite_error)
            if self._completion_prerequisite_block_can_be_accepted(prerequisite_error):
                self._accept_repeated_completion_prerequisite_block(prerequisite_error)
                self.completion_guard.pending = False
                self.completion_guard.confirmations = max(
                    int(self.completion_guard.confirmations),
                    max(2, int(self.completion_guard.required_confirmations)),
                )
                self.execution_journal.metrics.verified_completions += 1
                self.execution_journal.event(
                    "completion_prerequisite_loop_accepted",
                    step=self.n_steps,
                    repeated_blocks=block_count,
                    reason=prerequisite_error,
                )
                self._add_log_window("完成门禁：连续视觉复核成功，已放行完成状态", "success")
                result = [
                    ActionResult(
                        is_done=True,
                        extracted_content=(
                            "Task completed after repeated visual verification; "
                            "structured dispatch evidence was inferred from the stable final screen."
                        ),
                        include_in_memory=True,
                    )
                ]
                self.last_step_action = [{"done": {}}]
                self._last_result = result
                self._make_history_item(None, "completion_gate", result)
                self.n_steps += 1
                self.save_memory()
                self.execution_journal.checkpoint(
                    self._execution_checkpoint_state("completion_verified_after_repeated_review")
                )
                return True
            self.completion_guard.reset()
            self.execution_journal.metrics.verification_rejections += 1
            self.execution_journal.metrics.completion_claims_blocked += 1
            self.execution_journal.event(
                "completion_rejected_missing_fresh_delivery",
                step=self.n_steps,
                reason=prerequisite_error,
                repeated_blocks=block_count,
            )
            self.next_goal = self._completion_prerequisite_next_goal()
            if self.n_steps in self.brain_context:
                current_state = self.brain_context[self.n_steps].get("current_state", {})
                if isinstance(current_state, dict):
                    current_state["next_goal"] = self.next_goal
                    current_state["step_evaluate"] = "Failed"
            result = [ActionResult(extracted_content=prerequisite_error, include_in_memory=True)]
            self.last_step_action = []
            self._last_result = result
            await self._update_memory()
            self._make_history_item(None, "completion_prerequisite_gate", result)
            self.n_steps += 1
            self.save_memory()
            self.execution_journal.checkpoint(self._execution_checkpoint_state("completion_rejected"))
            return True

        self._reset_completion_prerequisite_blocks()
        self.execution_journal.metrics.completion_requests += 1
        if decision == "verify" and self._completion_has_fresh_delivery_confirmation():
            self.completion_guard.pending = False
            self.completion_guard.confirmations = max(
                int(self.completion_guard.confirmations),
                max(2, int(self.completion_guard.required_confirmations)),
            )
            decision = "confirmed"
            self.execution_journal.event(
                "completion_fast_confirmed_after_delivery_visual_review",
                step=self.n_steps,
            )
        if decision == "verify":
            self.execution_journal.metrics.completion_claims_blocked += 1
            self.execution_journal.metrics.visual_verification_rounds += 1
            self.execution_journal.event(
                "completion_blocked_for_visual_verification",
                step=self.n_steps,
                confirmations=self.completion_guard.confirmations,
            )
            message = (
                "Completion claim blocked by the framework; a fresh visual "
                "verification is required before the task may end."
            )
            self._add_log_window("完成门禁：已拦截结束请求，正在进行独立视觉复核", "warning")
            logger.info("[CompletionGuard] %s", message)
            result = [ActionResult(extracted_content=message, include_in_memory=True)]
        elif decision == "confirmed":
            self.execution_journal.metrics.verified_completions += 1
            self.execution_journal.event(
                "completion_verified",
                step=self.n_steps,
                step_evaluate=step_evaluate,
            )
            self._add_log_window("完成门禁：最终结果已通过视觉复核", "success")
            logger.info("[CompletionGuard] Task result independently verified.")
            result = [
                ActionResult(
                    is_done=True,
                    extracted_content="Task completed and independently verified",
                    include_in_memory=True,
                )
            ]
        else:
            # A completion signal paired with a failed/uncertain evaluation is
            # internally inconsistent and must never fall through to the old
            # unconditional completion path.
            self.execution_journal.metrics.verification_rejections += 1
            self.execution_journal.metrics.completion_claims_blocked += 1
            self.execution_journal.event(
                "completion_rejected_by_evaluation",
                step=self.n_steps,
                step_evaluate=step_evaluate,
            )
            message = (
                "Completion claim rejected because the latest result evaluation "
                "was not successful. Continue by inspecting and correcting the task."
            )
            result = [ActionResult(extracted_content=message, include_in_memory=True)]

        self.last_step_action = []
        self._last_result = result
        await self._update_memory()
        self._make_history_item(None, "completion_gate", result)
        self.n_steps += 1
        self.save_memory()
        self.execution_journal.checkpoint(
            self._execution_checkpoint_state(
                "completed" if decision == "confirmed" else "verifying_completion"
            )
        )
        return True

    def _latest_history_item(self) -> Optional[AgentHistory]:
        if not self.history.history:
            return None
        return self.history.history[-1]

    def _clear_done_flags(self, results: list[ActionResult]) -> int:
        cleared = 0
        for item in results or []:
            if item and item.is_done:
                item.is_done = False
                cleared += 1
        return cleared

    def _verified_completion_ready_to_stop(self) -> bool:
        latest = self._latest_history_item()
        if not latest or not latest.result or not latest.result[-1].is_done:
            return False
        if latest.state == "completion_gate":
            return True
        required = max(2, int(self.completion_guard.required_confirmations))
        return (
            not self.completion_guard.pending
            and self.completion_guard.confirmations >= required
            and self._completion_prerequisite_error() is None
        )

    async def _handle_history_done_before_stop(self) -> bool:
        """Return True only when the latest done marker is allowed to stop the run."""
        if not self.history.is_done():
            return False
        if self._verified_completion_ready_to_stop():
            return True

        prerequisite_error = self._completion_prerequisite_error()
        if prerequisite_error:
            block_count = self._record_completion_prerequisite_block(prerequisite_error)
            if self._completion_prerequisite_block_can_be_accepted(prerequisite_error):
                self._accept_repeated_completion_prerequisite_block(prerequisite_error)
                self.completion_guard.pending = False
                self.completion_guard.confirmations = max(
                    int(self.completion_guard.confirmations),
                    max(2, int(self.completion_guard.required_confirmations)),
                )
                self.execution_journal.event(
                    "history_done_prerequisite_loop_accepted",
                    step=self.n_steps,
                    repeated_blocks=block_count,
                    reason=prerequisite_error,
                )
                return True
        else:
            self._reset_completion_prerequisite_blocks()
        reason = prerequisite_error or (
            "Completion blocked: a done marker was produced before independent "
            "visual verification of the final task outcome."
        )
        latest = self._latest_history_item()
        if latest:
            self._clear_done_flags(latest.result)
            latest.result.append(ActionResult(extracted_content=reason, include_in_memory=True))

        self.completion_guard.reset()
        self.execution_journal.metrics.verification_rejections += 1
        self.execution_journal.metrics.completion_claims_blocked += 1
        self.execution_journal.event(
            "history_done_stop_blocked",
            step=self.n_steps,
            reason=reason,
        )
        self.next_goal = (
            self._completion_prerequisite_next_goal()
            if prerequisite_error
            else "继续复核最新界面：确认目标、内容、动作和结果证据都与本次任务一致。"
        )
        await self._update_memory()
        self.save_memory()
        self.execution_journal.checkpoint(
            self._execution_checkpoint_state("history_done_blocked")
        )
        return False

    @time_execution_async("--actor_step")
    async def actor_step(
        self,
        step_info: Optional[AgentStepInfo] = None,
        precomputed_model_output: Optional[AgentOutput] = None,
        precomputed_raw: Optional[dict[str, Any]] = None,
        precomputed_messages: Optional[list[BaseMessage]] = None,
    ) -> None:
        step_id = self.n_steps
        state = ""
        model_output = None
        result: list[ActionResult] = []
        prev_step_id = step_id - 1
        actor_step_start = time.time()
        self.wait_this_step = False

        if await self._handle_completion_gate():
            return
        
        # 初始化当前步骤的计时（如果还没有）
        if not hasattr(self, '_step_timing'):
            self._step_timing = {}
        if step_id not in self._step_timing:
            self._step_timing[step_id] = {"step_start": actor_step_start}
        
        # 记录 actor_step 时间点
        self._step_timing[step_id]["actor_start"] = actor_step_start
        self._step_timing[step_id]["actor_inference_start"] = 0
        self._step_timing[step_id]["actor_inference_end"] = 0
        self._step_timing[step_id]["actor_inference_precomputed"] = False
        self._step_timing[step_id]["action_exec_start"] = 0
        self._step_timing[step_id]["action_exec_end"] = 0
        self._step_timing[step_id]["actor_end"] = 0
        
        try:
            self.save_memory()
            if precomputed_model_output is not None:
                model_output = precomputed_model_output
                raw = precomputed_raw or model_output.model_dump(exclude_unset=True)
                actor_messages = precomputed_messages or []
                timing = self._step_timing[step_id]
                actor_inference_start = timing.get("brain_start") or time.time()
                actor_inference_end = timing.get("brain_end") or actor_inference_start
                self._step_timing[step_id]["actor_inference_start"] = actor_inference_start
                self._step_timing[step_id]["actor_inference_end"] = actor_inference_end
                self._step_timing[step_id]["actor_inference_precomputed"] = True
                actor_inference_time = max(0.0, actor_inference_end - actor_inference_start)
                logger.debug("[Actor] Using precomputed action output from combined step.")
            else:
                info_files = "\n".join(str(item) for item in self.infor_memory) if self.infor_memory else "None"

                # Debug: Log brain_thought and next_goal before building state_content
                logger.debug("[Actor] step_id=%d, brain_thought='%s', next_goal='%s'",
                            step_id,
                            str(self.brain_thought)[:100] if self.brain_thought else 'None',
                            str(self.next_goal)[:100] if self.next_goal else 'None')

                if self.n_steps >= 2:
                    operations = self._recent_operations_summary()
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                f"Recently executed operations:\n{operations}\n\n"
                                f"Recorded info files (filenames only): {info_files}\n\n"
                                f"Analysis to the current screen is: {self.brain_thought}.\n\n"
                                f"Your goal to achieve in this step is: {self.next_goal}\n\n"
                            ),
                        },
                        {"type": "image_url", "image_url": {"url": screenshot_to_dataurl(self.screenshot_annotated)}},
                    ]
                else:
                    state_content = [
                        {
                            "type": "text",
                            "content": (
                                f"Analysis to the current screen is: {self.brain_thought}. "
                                f"Your goal to achieve in this step is: {self.next_goal}"
                            ),
                        },
                        {"type": "image_url", "image_url": {"url": screenshot_to_dataurl(self.screenshot_annotated)}},
                    ]

                self.actor_message_manager._remove_last_AIntool_message()
                self.actor_message_manager._remove_last_state_message()
                self.actor_message_manager.add_state_message(state_content, step_info=step_info)

                actor_messages = self.actor_message_manager.get_messages()
                
                # Debug: Log actor message structure
                logger.debug("[Actor] Input messages count: %d", len(actor_messages))
                for i, msg in enumerate(actor_messages[-3:]):  # Last 3 messages
                    logger.debug("[Actor] Message %d: %s - content preview: %s", 
                               i, type(msg).__name__, 
                               str(msg.content)[:200] if hasattr(msg, 'content') else 'N/A')

                # Actor model inference timing
                actor_inference_start = time.time()
                self._step_timing[step_id]["actor_inference_start"] = actor_inference_start
                try:
                    model_output, raw = await self.get_next_action(actor_messages)
                finally:
                    actor_inference_end = time.time()
                    self._step_timing[step_id]["actor_inference_end"] = actor_inference_end
                actor_inference_time = actor_inference_end - actor_inference_start
            actor_inference_precomputed = bool(
                self._step_timing[step_id].get("actor_inference_precomputed")
            )
            if actor_inference_precomputed:
                logger.debug("[Timing] Step %s: Actor output reused from combined model.", step_id)
            else:
                logger.debug(f"[Timing] Step {step_id}: Actor model inference completed in {actor_inference_time:.2f}s")
            
            # 记录 Actor 推理时间
            self._last_actor_inference_time = actor_inference_time
            
            # 更新状态窗口 - Actor 推理完成
            self._add_log_window(f"步骤 {step_id}：动作模型推理（{actor_inference_time:.2f}s）", "timing")
            self._add_detail_window(f"动作模型推理：{actor_inference_time:.2f}s")

            if model_output and hasattr(model_output, "action"):
                model_output.action = self._ensure_wechat_group_name_input_commits(model_output.action)

            # 记录 LLM 响应到会话日志
            if session_logger and not actor_inference_precomputed:
                # 尝试提取 token 信息
                prompt_tokens, completion_tokens, total_tokens = None, None, None
                if hasattr(model_output, 'response_metadata'):
                    prompt_tokens, completion_tokens, total_tokens = self._extract_response_token_usage(model_output)
                elif hasattr(raw, 'response_metadata'):
                    prompt_tokens, completion_tokens, total_tokens = self._extract_response_token_usage(raw)

                session_logger.log_llm_response(
                    "Actor",
                    str(model_output.action)[:500] if model_output and hasattr(model_output, 'action') else "N/A",
                    tokens=(prompt_tokens, completion_tokens, total_tokens) if total_tokens else None,
                    elapsed=actor_inference_time
                )

            self.last_goal = self.next_goal
            if self.register_new_step_callback:
                self.register_new_step_callback(state, model_output, self.n_steps)
            self._save_actor_conversation(actor_messages, model_output, step=self.n_steps)

            self.actor_message_manager._remove_last_state_message()
            self.actor_message_manager.add_model_output(model_output)

            previous_step_action = self.last_step_action
            self.last_step_action = (
                [action.model_dump(exclude_unset=True) for action in model_output.action] if model_output else []
            )

            # Action execution timing - 为每个动作单独计时
            action_execution_start = time.time()
            self._step_timing[step_id]["action_exec_start"] = action_execution_start
            # 更新状态窗口 - 显示正在执行动作
            self._update_status_window(
                step_id,
                f"正在执行 {len(model_output.action) if model_output else 0} 个动作",
                "running"
            )
            self._add_log_window(f"步骤 {step_id}：正在执行 {len(model_output.action) if model_output else 0} 个动作", "info")
            if model_output and hasattr(model_output, 'action'):
                for index, action in enumerate(model_output.action, start=1):
                    action_dict = action.model_dump(exclude_unset=True)
                    for action_name, action_params in action_dict.items():
                        if action_params is not None:
                            params_text = json.dumps(action_params, ensure_ascii=False, default=str)
                            params_text = params_text[:120] + "..." if len(params_text) > 120 else params_text
                            self._add_detail_window(f"动作 {index}: {action_name} {params_text}")
            try:
                action_list = model_output.action if model_output else []
                action_guard_message = None
                if not action_list:
                    action_guard_message = (
                        "Model produced no executable action for this step. "
                        "Choose a concrete action, record visible information with record_info, "
                        "or request completion with done only when the task is fully verified."
                    )
                else:
                    action_guard_message = (
                        self._check_stalled_navigation_strategy(action_list)
                        or self._check_blocked_action_strategy_reuse(action_list)
                        or self._check_text_input_focus(action_list, previous_step_action)
                        or self._check_text_result_gate(action_list)
                        or self._check_searchable_selection_strategy(action_list)
                        or self._check_grounding_hotspot_failures(action_list)
                        or self._check_repeated_actions(action_list)
                    )
                if action_guard_message:
                    logger.warning("[ActionGuard] Step %s: %s", step_id, action_guard_message)
                    self._remember_action_attempt(action_list)
                    self._record_action_guard_block(
                        step=step_id,
                        reason=action_guard_message,
                        actions=action_list,
                    )
                    self.next_goal = self._build_recovery_goal(self.next_goal, action_guard_message)
                    result = [ActionResult(error=action_guard_message, include_in_memory=True)]
                    self._step_errors[step_id] = action_guard_message
                    self._step_timing[step_id]["repeat_guard"] = True
                    self._post_action_observation_required = True
                else:
                    result = await self.controller.multi_act(action_list)
                    if any(
                        self._action_name_and_params(action)[0] != "done"
                        for action in action_list or []
                    ):
                        self._post_action_observation_required = True
                    self._remember_action_attempt(action_list)
                    self._record_action_evidence(action_list, result, step=step_id)
                    if action_list and not any(item is None or item.error for item in result):
                        self._record_workflow_event(
                            "attempted",
                            step=step_id,
                            goal=self.next_goal,
                            details={
                                "action_count": len(action_list),
                                "source": "actor_execution",
                            },
                        )
            finally:
                action_execution_end = time.time()
                self._step_timing[step_id]["action_exec_end"] = action_execution_end
            action_execution_time = action_execution_end - action_execution_start
            logger.debug(f"[Timing] Step {step_id}: Action execution completed in {action_execution_time:.2f}s")

            # 记录动作执行时间
            self._last_action_execution_time = action_execution_time

            # 更新状态窗口 - 显示动作执行完成和耗时
            self._add_log_window(f"步骤 {step_id}：动作执行完成（{action_execution_time:.2f}s）", "timing")
            self._add_detail_window(f"动作执行：{action_execution_time:.2f}s")
            
            # 记录 actor_step 结束时间
            actor_step_end = time.time()
            self._step_timing[step_id]["actor_end"] = actor_step_end
            
            # 计算步骤总时间（从步骤开始到 actor 结束）
            step_total_time = actor_step_end - self._step_timing[step_id].get("step_start", actor_step_end)
            self._step_timing[step_id]["step_total"] = step_total_time
            
            # 显示动作详情
            if model_output and hasattr(model_output, 'action'):
                for action in model_output.action:
                    action_dict = action.model_dump(exclude_unset=True)
                    for action_name, action_params in action_dict.items():
                        if action_params is not None:
                            # 简化参数显示
                            params_str = str(action_params)[:60] + "..." if len(str(action_params)) > 60 else str(action_params)
                            self._add_detail_window(f"  -> {action_name}: {params_str}")

            # 记录执行的动作到会话日志（每个动作平均耗时）
            if session_logger and model_output and hasattr(model_output, 'action'):
                actions_count = len(model_output.action)
                avg_action_time = action_execution_time / actions_count if actions_count > 0 else 0
                for i, action in enumerate(model_output.action):
                    action_dict = action.model_dump(exclude_unset=True)
                    # 获取动作名称：从 dict 的 key 中获取（第一个非 None 的 key）
                    action_name = "unknown"
                    for key, val in action_dict.items():
                        if val is not None:
                            action_name = key
                            break

                    # 美化动作名称显示
                    session_logger.log_action(action_name, action_dict, elapsed=avg_action_time)
            
            # A controller/Actor "done" action is also only a completion claim.
            # Route it through the same framework gate so it cannot bypass the
            # visual result verification performed by Brain on the next screen.
            if any(item and item.is_done for item in result):
                decision = self.completion_guard.evaluate(
                    completion_requested=True,
                    step_evaluate=(
                        self.current_state.get("step_evaluate", "")
                        if isinstance(getattr(self, "current_state", None), dict)
                        else ""
                    ),
                    step=step_id,
                )
                if decision != "confirmed":
                    for item in result:
                        if item:
                            item.is_done = False
                    self.execution_journal.metrics.completion_requests += 1
                    self.execution_journal.metrics.completion_claims_blocked += 1
                    self.execution_journal.metrics.visual_verification_rounds += 1
                    self.execution_journal.event(
                        "actor_completion_blocked_for_visual_verification",
                        step=step_id,
                    )
                    result.append(
                        ActionResult(
                            extracted_content=(
                                "Actor completion claim blocked; verify the final "
                                "task result from a fresh screenshot."
                            ),
                            include_in_memory=True,
                        )
                    )

            input_values = self._input_text_values(action_list)
            if input_values:
                input_failed = any(item is None or item.error for item in result)
                if input_failed:
                    self.execution_journal.metrics.text_input_failures += 1
                    self._pending_text_verification = None
                    self.execution_journal.event(
                        "text_input_dispatch_failed",
                        step=step_id,
                    )
                else:
                    self.execution_journal.metrics.text_inputs_dispatched += 1
                    expected_text = input_values[-1]
                    self._pending_text_focus = None
                    self._text_focus_review_passed = None
                    self._text_result_review_passed = None
                    self._text_result_review_step = None
                    self._pending_text_verification = {
                        "text": expected_text,
                        "sensitive": self._is_sensitive_text_goal(self.next_goal),
                        "requested_step": step_id,
                    }
                    self.execution_journal.event(
                        "text_input_waiting_for_visual_verification",
                        step=step_id,
                        text_length=len(expected_text),
                    )
                    result.append(
                        ActionResult(
                            extracted_content=(
                                "Text paste dispatched. The framework will verify "
                                "that the value remains visible before continuing."
                            ),
                            include_in_memory=True,
                        )
                    )
            elif (
                any(self._is_click_action(action) for action in action_list)
                and self._goal_allows_direct_input_completion(self.next_goal)
            ):
                expected_text = self._extract_text_target_from_goal(self.next_goal)
                if expected_text:
                    self._pending_text_focus = {
                        "text": expected_text,
                        "requested_step": step_id,
                    }
                    self._text_focus_review_passed = None
                    self._text_result_review_passed = None
                    self.execution_journal.event(
                        "text_focus_waiting_for_visual_verification",
                        step=step_id,
                        text_length=len(expected_text),
                    )
                    result.append(
                        ActionResult(
                            extracted_content=(
                                "Focus click completed. The framework will inspect "
                                "the control's editability before text input."
                            ),
                            include_in_memory=True,
                        )
                    )
            elif self._text_result_review_passed is True:
                # A visually approved value has now advanced to the next action.
                self._text_result_review_passed = None
                self._text_result_review_step = None

            dispatch_succeeded = False
            for index, action in enumerate(action_list or []):
                if not self._is_enter_or_submit_click_action(action, self.next_goal):
                    continue
                action_result = result[index] if index < len(result) else None
                if action_result is not None and getattr(action_result, "error", None):
                    continue
                dispatch_succeeded = True
                break
            if (
                "content_dispatched" in self.task_obligations
                and dispatch_succeeded
                and self._has_content_dispatch_after_insert()
            ):
                self._pending_content_dispatch_verification = {
                    "dispatch_step": step_id,
                    "goal": self.next_goal,
                    "original_task": self.original_task or self.task,
                }
                self._content_dispatch_review_passed = None
                self.execution_journal.event(
                    "content_dispatch_waiting_for_visual_verification",
                    step=step_id,
                    original_task=self.original_task or self.task,
                )
                for item in result:
                    if item:
                        item.is_done = False
                result.append(
                    ActionResult(
                        extracted_content=(
                            "Content dispatch action completed. The framework will verify "
                            "fresh visible delivery/result evidence before completion."
                        ),
                        include_in_memory=True,
                    )
                )

            self._last_result = result
            self.execution_journal.metrics.actions_dispatched += len(
                model_output.action if model_output else []
            )
            self.execution_journal.metrics.action_errors += sum(
                1 for item in result if item and item.error
            )
            self.execution_journal.event(
                "actions_finished",
                step=step_id,
                action_count=len(model_output.action if model_output else []),
                error_count=sum(1 for item in result if item and item.error),
            )
            self._track_action_step_health(step_id, action_list, result)

            if len(self.last_step_action) == 0:
                self.wait_this_step = True
                if result:
                    self.wait_this_step = False
            elif "wait" in str(self.last_step_action[0]):
                self.wait_this_step = True
            else:
                self.wait_this_step = False
            if self.last_step_action and not self.wait_this_step:
                # 计时：内存更新
                memory_update_start = time.time()
                await self._update_memory()
                self.save_memory()
                memory_update_time = time.time() - memory_update_start
                self._step_timing[step_id]["memory_update_time"] = memory_update_time

        except Exception as e:
            now = time.time()
            timing = self._step_timing.get(step_id, {})
            if timing.get("actor_inference_start") and not timing.get("actor_inference_end"):
                timing["actor_inference_end"] = now
            if timing.get("action_exec_start") and not timing.get("action_exec_end"):
                timing["action_exec_end"] = now
            timing["actor_end"] = now
            timing["actor_error"] = str(e)[:500]
            self._step_errors[step_id] = f"Actor error: {str(e)[:500]}"
            result = await self._handle_step_error(e)
            self._last_result = result
            # 更新状态窗口 - 显示错误
            self._update_status_window(step_id, f"错误：{str(e)[:40]}", "error")
            self._add_log_window(f"步骤 {step_id} 失败：{str(e)[:50]}...", "error")
        finally:
            # Always create history item, even if result is empty
            if result or (self.next_goal and "[TASK_COMPLETE]" in self.next_goal):
                # 计时：历史记录创建
                history_start = time.time()
                self._make_history_item(model_output, state, result)
                history_time = time.time() - history_start
                self._step_timing[step_id]["history_create_time"] = history_time

            # 内存更新时间（在 try 块中已经执行）
            # 注意：_update_memory 和 save_memory 在 try 块中执行，需要在那里计时

            if not self.wait_this_step:
                self.n_steps += 1
            self.execution_journal.checkpoint(
                self._execution_checkpoint_state("running")
            )

            # Log total actor step time
            actor_step_total = time.time() - actor_step_start
            logger.debug(f"[Timing] Step {step_id}: Actor step completed in {actor_step_total:.2f}s")

            # 计算整个步骤的总时间（从 brain_step 开始到 actor_step 结束）
            step_total_time = time.time() - self._step_timing.get(step_id, {}).get("step_start", time.time())
            logger.debug(f"[Timing] Step {step_id}: Total step time: {step_total_time:.2f}s")
            breakdown = self._step_timing_breakdown(step_id, step_total_time)
            visible_step_total = round(step_total_time, 2)
            if breakdown["actor_inference_precomputed"]:
                visible_breakdown = {
                    "截图区域检测": round(breakdown["screenshot_rect_time"], 2),
                    "截图抓取": round(breakdown["screenshot_capture_time"], 2),
                    "截图缩放": round(breakdown["screenshot_downscale_time"], 2),
                    "截图保存": round(breakdown["screenshot_save_time"], 2),
                    "截图调度开销": round(breakdown["screenshot_overhead_time"], 2),
                    "结果解析": round(breakdown["brain_other_time"], 2),
                    "内存更新": round(breakdown["memory_update_time"], 2),
                    "历史记录创建": round(breakdown["history_create_time"], 2),
                    "代码执行开销": round(breakdown["other_overhead_time"], 2),
                }
            else:
                visible_breakdown = {
                    "截图区域检测": round(breakdown["screenshot_rect_time"], 2),
                    "截图抓取": round(breakdown["screenshot_capture_time"], 2),
                    "截图缩放": round(breakdown["screenshot_downscale_time"], 2),
                    "截图保存": round(breakdown["screenshot_save_time"], 2),
                    "截图调度开销": round(breakdown["screenshot_overhead_time"], 2),
                    "Brain 推理": round(breakdown["brain_inference_time"], 2),
                    "Brain 其他": round(breakdown["brain_other_time"], 2),
                    "内存更新": round(breakdown["memory_update_time"], 2),
                    "历史记录创建": round(breakdown["history_create_time"], 2),
                    "代码执行开销": round(breakdown["other_overhead_time"], 2),
                }
            visible_accounted = (
                round(breakdown["actor_inference_time"], 2)
                + round(breakdown["action_execution_time"], 2)
                + sum(visible_breakdown.values())
            )
            visible_delta = round(visible_step_total - visible_accounted, 2)
            visible_breakdown["代码执行开销"] = max(
                0.0,
                round(visible_breakdown["代码执行开销"] + visible_delta, 2),
            )

            # 更新状态窗口并计时
            status_update_start = time.time()
            step_failed = step_id in getattr(self, "_step_errors", {})
            step_status = "error" if step_failed else "success"
            self._add_log_window(
                f"步骤 {step_id}：{'失败' if step_failed else '完成'}（{step_total_time:.2f}s）",
                step_status,
            )
            for label, value in visible_breakdown.items():
                if value > 0:
                    self._add_detail_window(f"{label}：{value:.2f}s")
            self._add_detail_window(f"步骤 {step_id} 总计：{visible_step_total:.2f}s")
            self._add_detail_window("---")
            status_update_time = time.time() - status_update_start
            self._step_timing[step_id]["status_update_time"] = status_update_time

            # 保存步骤记录（使用整个步骤的总时间）并计时
            record_save_start = time.time()
            self._save_step_record(step_id, step_total_time)
            record_save_time = time.time() - record_save_start
            self._step_timing[step_id]["record_save_time"] = record_save_time

            # 记录步骤完成到会话日志并计时
            session_log_time = 0
            if session_logger:
                session_log_start = time.time()
                session_logger.step_end(success=not step_failed)
                session_log_time = time.time() - session_log_start
            self._step_timing[step_id]["session_log_time"] = session_log_time

            # 更新状态窗口 - 显示步骤完成并计时
            final_status_update_start = time.time()
            self._update_status_window(
                step_id,
                self.next_goal[:50] + "..." if len(self.next_goal) > 50 else self.next_goal,
                step_status,
            )
            final_status_update_time = time.time() - final_status_update_start
            self._step_timing[step_id]["final_status_update_time"] = final_status_update_time

    async def _handle_step_error(self, error: Exception) -> list[ActionResult]:
        include_trace = logger.isEnabledFor(logging.DEBUG)
        error_msg = AgentError.format_error(error, include_trace=include_trace)
        prefix = f"Result failed {self.consecutive_failures + 1}/{self.max_failures} times:\n "

        if isinstance(error, (ValidationError, ValueError)):
            logger.error("%s%s", prefix, error_msg)
            if "Max token limit reached" in error_msg:
                self.actor_message_manager.max_input_tokens -= 500
                logger.info(
                    "Reducing agent max input tokens: %s", self.actor_message_manager.max_input_tokens
                )
                self.actor_message_manager.cut_messages()
            elif "Could not parse response" in error_msg:
                error_msg += "\n\nReturn a valid JSON object with the required fields."
            self.consecutive_failures += 1

        elif isinstance(error, RateLimitError):
            logger.warning("%s%s", prefix, error_msg)
            await asyncio.sleep(self.retry_delay)
            self.consecutive_failures += 1

        else:
            logger.error("%s%s", prefix, error_msg)
            self.consecutive_failures += 1

        return [ActionResult(error=error_msg, include_in_memory=True)]

    def _make_history_item(
        self,
        model_output: AgentOutput | None,
        state: str,
        result: list[ActionResult],
    ) -> None:
        history_item = AgentHistory(
            model_output=model_output,
            result=result,
            state=state,
            step_id=self.n_steps,
            current_state=(
                dict(self.current_state)
                if isinstance(getattr(self, "current_state", None), dict)
                else None
            ),
            next_goal=self.next_goal,
        )
        self.history.history.append(history_item)

    @staticmethod
    def _strip_json_code_fence(content: str) -> str:
        text = str(content or "").strip()
        fence_match = re.fullmatch(r"```(?:json|JSON)?\s*(.*?)\s*```", text, flags=re.DOTALL)
        if fence_match:
            return fence_match.group(1).strip()
        text = re.sub(r"^\s*```(?:json|JSON)?\s*", "", text)
        text = re.sub(r"\s*```\s*$", "", text)
        return text.strip()

    @classmethod
    def _parse_model_json_content(cls, content: str) -> Any:
        text = cls._strip_json_code_fence(content)
        decoder = json.JSONDecoder()
        try:
            return decoder.decode(text)
        except json.JSONDecodeError as first_error:
            best_error = first_error

        for index, char in enumerate(text):
            if char not in "[{":
                continue
            try:
                value, _ = decoder.raw_decode(text[index:])
                return value
            except json.JSONDecodeError as exc:
                best_error = exc
        raise best_error

    @time_execution_async("--get_next_action")
    async def get_next_action(self, input_messages: list[BaseMessage]) -> AgentOutput:
        """
        Build a structured_llm approach on top of actor_llm.
        """
        response: dict[str, Any] = await self.actor_llm.ainvoke(input_messages)
        
        # Debug: Log response structure
        logger.debug("[Actor] Raw response type: %s", type(response).__name__)
        logger.debug("[Actor] Has 'parsed' attr: %s", hasattr(response, "parsed"))
        if hasattr(response, "parsed"):
            logger.debug("[Actor] response.parsed = %s", response.parsed)
        logger.debug("[Actor] response.content (first 500 chars): %s", str(response.content)[:500] if hasattr(response, "content") else "N/A")
        
        # Try to use parsed response if available (for local models)
        if hasattr(response, "parsed") and response.parsed is not None:
            logger.debug("[Actor] Using response.parsed")
            record = response.parsed
        else:
            logger.debug("[Actor] response.parsed is None, parsing from content")
            raw_content = str(response.content)
            try:
                record = self._parse_model_json_content(raw_content)
                logger.debug("[Actor] Parsed from content: %s", record)
            except json.JSONDecodeError as e:
                logger.error("[Actor] Failed to parse JSON from content: %s", e)
                logger.error("[Actor] Raw content: %s", raw_content[:1000])
                raise ValueError(f"Failed to parse actor response as JSON: {e}")

        # Ensure record is a dict
        if not isinstance(record, dict):
            logger.warning("Parsed record is not a dict: %s", type(record))
            record = {"action": record} if isinstance(record, list) else {}

        parsed = self._coerce_action_record(record)
        self._log_response(parsed)
        return parsed, record

    def _coerce_action_record(self, record: dict[str, Any]) -> AgentOutput:
        # Check if action list is empty - this indicates the model didn't generate any actions
        action_list = record.get("action", [])
        
        # Handle common model output variations
        if not action_list:
            # Try "actions" (plural) if "action" is not found
            if "actions" in record:
                logger.info("[Actor] Model used 'actions' (plural), converting to 'action'")
                action_list = record.get("actions", [])
                record["action"] = action_list
            
        if not action_list:
            logger.warning("Model returned empty action list. Raw response: %s", record)
            logger.warning("[Actor] Full record keys: %s", list(record.keys()) if isinstance(record, dict) else "N/A")
            raise ValueError(
                "Model returned empty action list. "
                "Please output at least one action in the format: "
                '{"action": [{"action_name": {"param1": "value1"}}]}'
            )

        # Validate action format - each action should be a dict
        normalized_actions = []
        for i, action in enumerate(action_list):
            # Skip non-dict items (like raw floats from position arrays)
            if not isinstance(action, dict):
                logger.warning(f"Skipping invalid action item {i}: {action} (type: {type(action)})")
                continue

            if not action:
                normalized_actions.append(action)
                continue

            outer_key = list(action.keys())[0]
            inner_value = action[outer_key] if isinstance(action, dict) else {}

            # Skip if outer_key looks like a position coordinate (float/int)
            if isinstance(outer_key, (float, int)):
                logger.warning(f"Skipping action with numeric key: {outer_key}")
                continue

            # Handle model output format: {"action": "Click", "position": [16, 281]}
            # Convert to expected format: {"Click": {"position": [16, 281]}}
            if outer_key == "action" and isinstance(inner_value, str):
                action_name = inner_value
                # Collect all other fields as parameters
                params = {k: v for k, v in action.items() if k != "action"}
                normalized_action = {action_name: params}
                logger.debug(f"[Actor] Converting action format: {action} -> {normalized_action}")
                action = normalized_action
                outer_key = action_name
                inner_value = params

            if outer_key == "record_info" and isinstance(inner_value, dict):
                information_stored = inner_value.get("text", "")
                file_name = inner_value.get("file_name", "")
                saved_name = self.record_store.save(
                    information_stored,
                    file_name,
                    screenshot=self.screenshot_annotated,
                    step=self.n_steps,
                )
                if saved_name and saved_name not in self.infor_memory:
                    self.infor_memory.append(saved_name)
                self._record_workflow_event(
                    "attempted",
                    step=self.n_steps,
                    goal=self.next_goal,
                    details={
                        "action": "record_info",
                        "file_name": saved_name or file_name,
                        "source": "record_info",
                    },
                )
            normalized_actions.append(action)

        # If no valid actions remain after filtering, treat it as malformed output.
        if not normalized_actions:
            logger.error(f"No valid actions found after filtering. Original: {action_list}")
            raise ValueError(
                f"No valid actions found. Model output may be malformed. "
                f"Expected format: [{{\"action_name\": {{\"param\": \"value\"}}}}], "
                f"got: {action_list}"
            )
        
        normalized_actions = self._remove_redundant_select_all_before_input(normalized_actions)
        normalized_actions = self._limit_compact_selector_scroll(normalized_actions)
        # Never auto-complete a click with input in the same visual frame.
        # The clicked control must first be inspected by Brain.
        normalized_actions = self._stage_text_input_actions(normalized_actions)
        normalized_actions = self._ensure_scroll_targets_mouse_position(normalized_actions)
        if len(normalized_actions) > self.max_actions_per_step:
            original_count = len(normalized_actions)
            normalized_actions = normalized_actions[: self.max_actions_per_step]
            logger.warning(
                "[ActionGuard] Truncated action batch from %d to %d actions.",
                original_count,
                self.max_actions_per_step,
            )
            if hasattr(self, "execution_journal"):
                self.execution_journal.event(
                    "action_batch_truncated",
                    step=self.n_steps,
                    original_count=original_count,
                    max_actions=self.max_actions_per_step,
                )
        parsed: AgentOutput | None = AgentOutput(action=normalized_actions)
        return parsed

    def _log_response(self, response: AgentOutput) -> None:
        if "Success" in self.current_state["step_evaluate"]:
            emoji = "OK"
        elif "Failed" in self.current_state["step_evaluate"]:
            emoji = "FAIL"
        else:
            emoji = "UNKNOWN"
        logger.info("%s Eval: %s", emoji, self.current_state["step_evaluate"])
        logger.info("Memory: %s", self.brain_memory)
        logger.info("Goal to achieve this step: %s", self.next_goal)
        for i, action in enumerate(response.action):
            action_payload = action.model_dump(exclude_unset=True)
            logger.info(
                "Action %s/%s: %s",
                i + 1,
                len(response.action),
                json.dumps(action_payload, ensure_ascii=False, default=str),
            )

    def _track_action_step_health(
        self,
        step_id: int,
        actions: list[Any],
        results: list[ActionResult],
    ) -> None:
        """Update failure and no-progress budgets after controller execution."""
        if any(item and item.is_done for item in results or []):
            self._reset_no_progress_budgets()
            self._reset_stalled_navigation_budgets()
            self._strategy_observation_pending = None
            return

        action_error = any(item is None or bool(item.error) for item in results or [])
        guard_blocked = step_id in getattr(self, "_step_errors", {})
        strategy_signature = self._action_strategy_batch_signature(actions)
        model_says_no_progress = self._model_reports_previous_no_progress()
        no_visual_progress = not self._screens_differ()
        stalled_step = bool(model_says_no_progress and no_visual_progress)

        if self._record_step_health_and_maybe_request_assistance(
            step_id=step_id,
            action_error=action_error,
            guard_blocked=guard_blocked,
            stalled_step=stalled_step,
            no_visual_progress=no_visual_progress,
            strategy_signature=strategy_signature,
            reason_hint=getattr(self, "_step_errors", {}).get(step_id, ""),
        ):
            return

        if not action_error and not guard_blocked and self._is_material_strategy_shift(actions):
            self._reset_no_progress_budgets()
            if self._goal_is_web_navigation():
                self._reset_stalled_navigation_budgets()
            self._strategy_observation_pending = {
                "step": step_id,
                "strategy": strategy_signature,
                "goal": self.next_goal,
                "time": time.time(),
            }
            self._add_log_window("已切换到新的执行策略，等待下一帧验证效果", "info")
            self._refresh_brain_memory()
            return

        if action_error or guard_blocked or stalled_step:
            self.consecutive_failures += 1
        else:
            self.consecutive_failures = 0
            self._stalled_failure_count = 0
            self._strategy_observation_pending = None

        no_progress_candidate = bool(guard_blocked or stalled_step or (action_error and model_says_no_progress))
        if not no_progress_candidate:
            self._no_progress_count = 0
            self._last_no_progress_signature = None
            self._last_no_progress_strategy_signature = None
            if not self._goal_is_web_navigation():
                self._reset_stalled_navigation_budgets()
            return

        signature = (
            self._normalized_action_batch_signature(actions)
            or self._normalized_goal_signature(self.next_goal)
            or "unknown"
        )
        strategy_signature = strategy_signature or signature
        self._stalled_failure_count += 1
        if self._goal_is_web_navigation():
            self._stalled_navigation_count += 1
            self._stalled_navigation_strategies.add(strategy_signature)
            if (
                self._stalled_navigation_count >= self.web_navigation_switch_steps
                and not self._stalled_navigation_strategy_switch_requested
                and not self._actions_include_browser_switch(actions)
                and not self._direct_navigation_attempt_allowed(actions)
            ):
                self._stalled_navigation_strategy_switch_requested = True
                self.next_goal = (
                    "当前浏览器网页搜索/加载已经连续无进展。请不要继续在同一个页面重复点击搜索框、"
                    "回车、刷新或Tab。下一步必须换用另一个已安装浏览器，或使用明显不同的入口/"
                    "可确认的官方地址；如果换入口后仍无法加载，应结束任务并说明网页无法访问。"
                )
                self._add_log_window("网页导航连续无进展，下一步将强制换浏览器或换入口", "warning")
                self._refresh_brain_memory()

            if self._stalled_navigation_count >= self.web_navigation_max_stalled_steps:
                reason = self._stalled_navigation_failure_reason()
                logger.error("Stopping stalled web navigation: %s", reason)
                self.execution_journal.event(
                    "web_navigation_stalled",
                    step=step_id,
                    count=self._stalled_navigation_count,
                    strategies=sorted(self._stalled_navigation_strategies),
                    reason=reason,
                )
                self._add_log_window(f"无法完成任务：{reason}", "error")
                self._request_user_assistance(
                    reason_code="web_navigation_stalled",
                    reason=reason,
                    user_action_required=self._assistance_user_action_for_reason("web_navigation_stalled"),
                    resume_condition=self._assistance_resume_condition_for_reason("web_navigation_stalled"),
                    confidence=0.85,
                )
                return

        if no_visual_progress and signature == self._last_no_progress_signature:
            self._no_progress_count += 1
        elif no_visual_progress:
            self._no_progress_count = 1
            self._last_no_progress_signature = signature
            self._last_no_progress_strategy_signature = strategy_signature
        else:
            self._no_progress_count = 0
            self._last_no_progress_signature = None
            self._last_no_progress_strategy_signature = None

        if self._no_progress_count >= self.no_progress_max_steps:
            reason = (
                "No-progress budget exhausted: repeated failed, blocked, or visually "
                "ineffective action attempts did not produce visible screen progress."
            )
            logger.error(
                "%s step=%s count=%s/%s signature=%s",
                reason,
                step_id,
                self._no_progress_count,
                self.no_progress_max_steps,
                signature[:240],
            )
            self.execution_journal.event(
                "no_progress_budget_exhausted",
                step=step_id,
                count=self._no_progress_count,
                threshold=self.no_progress_max_steps,
                model_evaluation=self.current_state.get("step_evaluate", "")
                if isinstance(getattr(self, "current_state", None), dict)
                else "",
                signature=signature,
            )
            if not self._has_enough_assistance_attempts():
                if strategy_signature:
                    self._blocked_action_strategy_signatures.append(strategy_signature)
                self.next_goal = self._build_recovery_goal(self.next_goal, reason)
                self._no_progress_count = 0
                self._last_no_progress_signature = None
                self._last_no_progress_strategy_signature = strategy_signature
                self._add_log_window("连续无进展，已要求切换到不同策略，暂不请求用户接管", "warning")
                self._refresh_brain_memory()
                return
            self._add_log_window("连续无进展，已停止自动执行，请人工检查当前界面", "error")
            self._request_user_assistance(
                reason_code="no_progress",
                reason=reason,
                user_action_required=self._assistance_user_action_for_reason("element_not_found"),
                resume_condition=self._assistance_resume_condition_for_reason("element_not_found"),
                confidence=0.8,
            )

    # ========== 以下方法已禁用，不再保存 LLM 对话记录 ==========
    # 如需启用，请注释掉下面的 return 语句
    
    def _save_brain_conversation(
        self,
        input_messages: list[BaseMessage],
        response: Any,
        step: int,
    ) -> None:
        """
        Write all the Brain agent conversation into a file.
        DISABLED: To save resources and reduce file clutter.
        """
        # DISABLED: Brain conversation logging is disabled by default
        # To re-enable: remove the 'return' statement below
        return
        
        if not self.save_brain_conversation_path:
            return
        file_name = f"{self.save_brain_conversation_path}_brain_{step}.txt"
        os.makedirs(os.path.dirname(file_name), exist_ok=True) if os.path.dirname(file_name) else None

        with open(file_name, "w", encoding=self.save_brain_conversation_path_encoding) as f:
            self._write_messages_to_file(f, input_messages)
            if response is not None:
                self._write_response_to_file(f, response)

        logger.info("Brain conversation saved to: %s", file_name)

    def _save_actor_conversation(
        self,
        input_messages: list[BaseMessage],
        response: Any,
        step: int,
    ) -> None:
        """
        Write all the Actor agent conversation into a file.
        DISABLED: To save resources and reduce file clutter.
        """
        # DISABLED: Actor conversation logging is disabled by default
        # To re-enable: remove the 'return' statement below
        return
        
        if not self.save_actor_conversation_path:
            return
        file_name = f"{self.save_actor_conversation_path}_actor_{step}.txt"
        os.makedirs(os.path.dirname(file_name), exist_ok=True) if os.path.dirname(file_name) else None

        with open(file_name, "w", encoding=self.save_actor_conversation_path_encoding) as f:
            self._write_messages_to_file(f, input_messages)
            if response is not None:
                self._write_response_to_file(f, response)

        logger.info("Actor conversation saved to: %s", file_name)

    def _write_messages_to_file(self, f: Any, messages: list[BaseMessage]) -> None:
        for message in messages:
            f.write(f"\n{message.__class__.__name__}\n{'-'*40}\n")
            if isinstance(message.content, list):
                for item in message.content:
                    if isinstance(item, dict):
                        if item.get("type") == "text":
                            txt = item.get("content") or item.get("text", "")
                            f.write(f"[Text Content]\n{txt.strip()}\n\n")
                        elif item.get("type") == "image_url":
                            image_url = item["image_url"]["url"]
                            f.write(f"[Image URL]\n{image_url[:100]}...\n\n")
            else:
                f.write(f"{str(message.content)}\n\n")
            f.write("\n" + "=" * 60 + "\n")

    def _write_response_to_file(self, f: Any, response: Any) -> None:
        f.write("RESPONSE\n")
        f.write(str(response) + "\n")
        f.write("\n" + "=" * 60 + "\n")

    def _log_agent_run(self) -> None:
        logger.info("Starting task: %s", self.task)

    def _init_step_log_file(self) -> None:
        """初始化可选步骤记录文件（每次运行前清空）"""
        self._init_step_record_json()
        if self._step_log_path:
            step_log_dir = os.path.dirname(self._step_log_path)
            if step_log_dir:
                os.makedirs(step_log_dir, exist_ok=True)
            # 清空文件（覆盖模式）
            with open(self._step_log_path, 'w', encoding='utf-8') as f:
                # 写入文件头注释
                f.write("# 步骤日志\n")
                f.write("# 每次任务执行前自动清空，每完成一步自动记录\n")
                f.write("=" * 80 + "\n\n")
            logger.info("步骤日志文件已初始化：%s", self._step_log_path)
        else:
            logger.debug("未设置步骤日志路径，跳过初始化")

    def _init_step_record_json(self) -> None:
        """初始化可选固定 JSON 步骤记录文件（每次运行前清空）"""
        self._step_records = []
        if not self._step_record_json_path:
            return
        data = {
            "任务": self.original_task,
            "开始时间": datetime.now().isoformat(),
            "更新时间": datetime.now().isoformat(),
            "智能体ID": self.agent_id,
            "步骤记录": [],
            "动作数量": 0,
            "任务总结": "",
        }
        try:
            self._step_record_json_path.parent.mkdir(parents=True, exist_ok=True)
            with open(self._step_record_json_path, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            logger.info("步骤 JSON 记录已初始化：%s", self._step_record_json_path)
        except Exception as e:
            logger.warning("初始化步骤 JSON 记录失败：%s", e)

    def _clear_step_log_file(self) -> None:
        """清空步骤记录文件（任务结束或中断时调用）"""
        if self._step_log_path:
            try:
                with open(self._step_log_path, 'w', encoding='utf-8') as f:
                    pass  # 完全清空，不写入任何内容
                logger.info("步骤日志文件已清空：%s", self._step_log_path)
            except Exception as e:
                logger.debug(f"清空步骤日志失败：{e}")

    def _cleanup_image_cache(self) -> None:
        """Remove screenshots produced by the current run after it finishes."""
        if not getattr(self, "cleanup_images_on_finish", True):
            return
        images_dir = getattr(self, "images_dir", None)
        if not images_dir:
            return

        try:
            path = Path(images_dir).expanduser()
            if not path.exists():
                return
            resolved = path.resolve()
            agent_id = str(getattr(self, "agent_id", "") or "")

            if agent_id and resolved.name == agent_id and resolved.parent.name == "images":
                shutil.rmtree(resolved)
                logger.info("图片缓存已清理：%s", resolved)
                return

            removed = 0
            for screenshot_path in resolved.glob("screenshot_*.png"):
                if screenshot_path.is_file():
                    screenshot_path.unlink()
                    removed += 1
            if removed:
                logger.info("图片缓存已清理：%s（%s 张截图）", resolved, removed)
        except Exception as e:
            logger.debug("清理图片缓存失败：%s", e)

    async def run(self, max_steps: int = 100) -> AgentHistoryList:
        run_start_time = time.time()
        final_status = "unknown"
        stop_reason = "unknown"

        try:
            self._log_agent_run()
            self.execution_journal.event(
                "run_started",
                task=self.task,
                step=self.n_steps,
                resume=self.resume,
            )
            self._write_run_result(
                status="running",
                reason="run_started",
                total_run_time=0,
                max_steps=max_steps,
                final_evidence=None,
            )
            if self.resume:
                recovered_state = self.execution_journal.load()
                if recovered_state:
                    self.completion_guard.restore(
                        recovered_state.get("completion_guard")
                    )
                    # Journal is a recovery fallback when the richer memory
                    # snapshot is unavailable or was interrupted mid-write.
                    self.n_steps = max(
                        self.n_steps,
                        int(recovered_state.get("step", self.n_steps)),
                    )
                    self.next_goal = recovered_state.get(
                        "next_goal", self.next_goal
                    )
                    self.last_step_action = recovered_state.get(
                        "last_step_action", self.last_step_action
                    )
                    self.task_obligations = recovered_state.get(
                        "task_obligations",
                        self.task_obligations,
                    )
                    self.action_evidence = recovered_state.get(
                        "action_evidence",
                        self.action_evidence,
                    )
                    self.workflow_events = recovered_state.get(
                        "workflow_events",
                        self.workflow_events,
                    )
                    self._last_action_guard_block = recovered_state.get(
                        "last_action_guard_block",
                        self._last_action_guard_block,
                    )
                    blocked_strategy_signatures = recovered_state.get(
                        "blocked_action_strategy_signatures"
                    )
                    if isinstance(blocked_strategy_signatures, list):
                        self._blocked_action_strategy_signatures.clear()
                        self._blocked_action_strategy_signatures.extend(
                            str(item) for item in blocked_strategy_signatures[-24:]
                        )
                    hotspot_failures = recovered_state.get("grounding_hotspot_failures")
                    if isinstance(hotspot_failures, dict):
                        self._grounding_hotspot_failures = hotspot_failures
                    normalized_signatures = recovered_state.get(
                        "recent_normalized_action_signatures"
                    )
                    if isinstance(normalized_signatures, list):
                        self._recent_normalized_action_signatures.clear()
                        self._recent_normalized_action_signatures.extend(
                            str(item) for item in normalized_signatures[-12:]
                        )
                    self._no_progress_count = int(
                        recovered_state.get("no_progress_count", self._no_progress_count)
                        or 0
                    )
                    self._last_no_progress_signature = recovered_state.get(
                        "last_no_progress_signature",
                        self._last_no_progress_signature,
                    )
                    self._stalled_failure_count = int(
                        recovered_state.get("stalled_failure_count", self._stalled_failure_count)
                        or 0
                    )
                    self._stalled_navigation_count = int(
                        recovered_state.get("stalled_navigation_count", self._stalled_navigation_count)
                        or 0
                    )
                    self._stalled_navigation_strategy_switch_requested = bool(
                        recovered_state.get(
                            "stalled_navigation_strategy_switch_requested",
                            self._stalled_navigation_strategy_switch_requested,
                        )
                    )
                    self._stalled_navigation_block_count = int(
                        recovered_state.get(
                            "stalled_navigation_block_count",
                            self._stalled_navigation_block_count,
                        )
                        or 0
                    )
                    stalled_strategies = recovered_state.get("stalled_navigation_strategies")
                    if isinstance(stalled_strategies, list):
                        self._stalled_navigation_strategies = {
                            str(item) for item in stalled_strategies
                        }
                    recent_step_health = recovered_state.get("recent_step_health")
                    if isinstance(recent_step_health, list):
                        self._recent_step_health.clear()
                        for item in recent_step_health[-self._recent_step_health.maxlen:]:
                            if isinstance(item, dict):
                                self._recent_step_health.append(item)
                    assistance_request = recovered_state.get("assistance_request")
                    if isinstance(assistance_request, dict):
                        self.assistance_request = assistance_request
                    self._pending_text_focus = recovered_state.get(
                        "pending_text_focus", self._pending_text_focus
                    )
                    self._pending_text_verification = recovered_state.get(
                        "pending_text_verification",
                        self._pending_text_verification,
                    )
                    self._pending_content_dispatch_verification = recovered_state.get(
                        "pending_content_dispatch_verification",
                        self._pending_content_dispatch_verification,
                    )
                    self._text_focus_review_passed = recovered_state.get(
                        "text_focus_review_passed",
                        self._text_focus_review_passed,
                    )
                    self._text_result_review_passed = recovered_state.get(
                        "text_result_review_passed",
                        self._text_result_review_passed,
                    )
                    self._text_result_review_step = recovered_state.get(
                        "text_result_review_step",
                        self._text_result_review_step,
                    )
                    self._content_dispatch_review_passed = recovered_state.get(
                        "content_dispatch_review_passed",
                        self._content_dispatch_review_passed,
                    )
                    self._completion_prerequisite_block_reason = recovered_state.get(
                        "completion_prerequisite_block_reason",
                        self._completion_prerequisite_block_reason,
                    )
                    self._completion_prerequisite_block_count = int(
                        recovered_state.get(
                            "completion_prerequisite_block_count",
                            self._completion_prerequisite_block_count,
                        )
                        or 0
                    )
                    self._post_action_observation_required = bool(
                        recovered_state.get(
                            "post_action_observation_required",
                            self._post_action_observation_required,
                        )
                    )
                    deferred_assistance = recovered_state.get("deferred_assistance_request")
                    self._deferred_assistance_request = (
                        deferred_assistance
                        if isinstance(deferred_assistance, dict)
                        else self._deferred_assistance_request
                    )
            self.execution_journal.checkpoint(
                self._execution_checkpoint_state("starting")
            )

            # 初始化步骤记录文件（每次运行前清空）
            self._init_step_log_file()

            # 更新状态窗口
            self._update_status_window(0, "正在初始化...", "running")
            self._add_log_window(f"任务：{self.task[:50]}...", "info")
            self._add_log_window("正在启动智能体...", "info")

            # 设置任务描述到窗口
            if self._status_window:
                from src.utils.status_window import set_task
                set_task(self.task)

            if self.planner_llm and not self.resume:
                await self.edit()

            final_status = "success"
            while True:
                # 检查停止标志
                if self._stopped:
                    final_status = (
                        "need_user_assistance"
                        if self.assistance_request
                        else "stopped"
                    )
                    stop_reason = self._stop_reason or "user_stopped"
                    # 用户停止，清空步骤日志
                    self._clear_step_log_file()
                    break

                if self.resume:
                    await self.load_memory()
                    self.resume = False
                if self._too_many_failures():
                    final_status = (
                        "need_user_assistance"
                        if self.assistance_request
                        else "max_failures"
                    )
                    stop_reason = self._stop_reason or "max_failures"
                    # 达到最大失败次数，清空步骤日志
                    self._clear_step_log_file()
                    break
                if not await self._handle_control_flags():
                    final_status = "stopped"
                    stop_reason = self._stop_reason or "control_flag"
                    # 控制标志停止，清空步骤日志
                    self._clear_step_log_file()
                    break

                if self.n_steps > max_steps and not self._post_action_observation_required:
                    logger.info("Failed to complete task in maximum steps")
                    final_status = "max_steps"
                    stop_reason = "max_steps_reached"
                    self._update_status_window(max_steps, "已达到最大步骤数", "warning")
                    self._add_log_window("已达到最大步骤数", "warning")
                    self._clear_step_log_file()
                    break

                observation_only = self.n_steps >= max_steps and self._post_action_observation_required

                # 更新步骤信息到状态窗口
                self._update_status_window(
                    self.n_steps,
                    getattr(self, 'next_goal', f'步骤 {self.n_steps}'),
                    "running"
                )

                if observation_only:
                    self._add_log_window("已达到最大步骤数，先执行动作后最终截图复核", "warning")
                    await self.brain_step()
                elif self.combined_step_enabled:
                    await self.combined_step()
                else:
                    await self.brain_step()
                    if self._stopped:
                        continue
                    await self.actor_step()

                if await self._handle_history_done_before_stop():
                    logger.info("Task completed successfully")
                    self._update_status_window(self.n_steps, "任务完成", "success")
                    self._add_log_window("任务已成功完成", "success")
                    if self.register_done_callback:
                        self.register_done_callback(self.history)
                    final_status = "success"
                    stop_reason = "task_completed"
                    # 任务完成，清空步骤日志
                    self._clear_step_log_file()
                    break
                if observation_only:
                    logger.info("Maximum steps reached after final post-action observation")
                    final_status = (
                        "need_user_assistance"
                        if self.assistance_request
                        else "max_steps"
                    )
                    stop_reason = self._stop_reason or "max_steps_reached_after_final_observation"
                    self._update_status_window(max_steps, "已达到最大步骤数", "warning")
                    self._add_log_window("已达到最大步骤数，最终截图复核已完成", "warning")
                    self._clear_step_log_file()
                    break
                self._write_run_result(
                    status=(
                        "need_user_assistance"
                        if self.assistance_request
                        else "running"
                    ),
                    reason=self._stop_reason or "running",
                    total_run_time=time.time() - run_start_time,
                    max_steps=max_steps,
                    final_evidence=(
                        None
                        if getattr(self, "cleanup_images_on_finish", True)
                        else getattr(self, "_current_screenshot_path", None)
                    ),
                )
                if self.step_delay > 0:
                    await asyncio.sleep(self.step_delay)
            # 计算总运行时间
            total_run_time = time.time() - run_start_time
            logger.info(f"Total run time: {total_run_time:.2f}s")
            self._add_log_window(f"总执行耗时：{total_run_time:.2f}s", "timing")
            final_evidence = (
                None
                if getattr(self, "cleanup_images_on_finish", True)
                else getattr(self, "_current_screenshot_path", None)
            )
            if final_status != "success":
                stop_reason = await self._run_final_diagnosis(
                    status=final_status,
                    reason=stop_reason,
                )
                if (
                    self._latest_state_success()
                    and self.next_goal
                    and "[TASK_COMPLETE]" in self.next_goal
                ):
                    final_status = "success"
                    stop_reason = "task_completed_after_final_diagnosis"
                    self.assistance_request = None
                elif self.assistance_request:
                    final_status = "need_user_assistance"

            # 记录会话结束
            if session_logger:
                session_logger.session_end(final_status)
            self.execution_journal.write_report(
                status=final_status,
                step=self.n_steps,
                final_evidence=final_evidence,
            )
            self._write_run_result(
                status=final_status,
                reason=stop_reason,
                total_run_time=total_run_time,
                max_steps=max_steps,
                final_evidence=final_evidence,
            )
            self._write_training_dataset(
                status=final_status,
                reason=stop_reason,
                total_run_time=total_run_time,
                max_steps=max_steps,
            )

            # 完成会话并保存日志
            if self._status_window:
                self._finalize_session(stop_reason, total_run_time)
                self._close_status_window_on_success(final_status, stop_reason)

            # 非正常结束（用户停止、错误等），清空步骤日志
            if final_status in ["stopped", "error"]:
                self._clear_step_log_file()

            return self.history
        except (Exception, KeyboardInterrupt, asyncio.CancelledError) as e:
            logger.exception("Error running agent")
            final_status = "error"
            stop_reason = f"error: {str(e)[:50]}"
            self._update_status_window(self.n_steps, f"错误：{str(e)[:30]}", "error")
            self._add_log_window(f"错误：{str(e)[:50]}...", "error")

            # 计算总运行时间
            total_run_time = time.time() - run_start_time

            if session_logger:
                session_logger.session_end("error")
            self.execution_journal.event(
                "run_failed",
                step=self.n_steps,
                error=str(e)[:500],
            )
            self.execution_journal.checkpoint(
                self._execution_checkpoint_state("interrupted")
            )
            self.execution_journal.write_report(
                status="error",
                step=self.n_steps,
                final_evidence=(
                    None
                    if getattr(self, "cleanup_images_on_finish", True)
                    else getattr(self, "_current_screenshot_path", None)
                ),
            )
            self._write_run_result(
                status="error",
                reason=stop_reason,
                total_run_time=total_run_time,
                max_steps=max_steps,
                final_evidence=(
                    None
                    if getattr(self, "cleanup_images_on_finish", True)
                    else getattr(self, "_current_screenshot_path", None)
                ),
            )
            self._write_training_dataset(
                status="error",
                reason=stop_reason,
                total_run_time=total_run_time,
                max_steps=max_steps,
            )

            # 完成会话并保存日志（即使出错）
            if self._status_window:
                self._finalize_session(stop_reason, total_run_time)

            # 发生错误，清空步骤日志
            self._clear_step_log_file()

            # 不再 raise，让调用者通过返回值判断状态
            return self.history
        finally:
            self._cleanup_image_cache()

    async def edit(self):
        result = await self.planner.edit_task()
        self._set_new_task(result.raw_text, result.payload)

    PREFIX = "The overall user's task is: "
    SUFFIX = "The step by step plan is: "

    def _set_new_task(self, generated_plan: str, plan_payload: Optional[dict] = None) -> None:
        """
        Build the final task string:
            "The overall plan is: <original task>\n\n<generated plan>"
        and update every MessageManager in one go.
        """
        plan_text = generated_plan
        if isinstance(plan_payload, dict):
            plan_text = self._format_plan_payload(plan_payload)
        if generated_plan.startswith(self.PREFIX):
            final_task = generated_plan
        else:
            final_task = f"{self.PREFIX}{self.original_task}\n{self.SUFFIX}\n{plan_text}"

        if self.use_skills and self.available_skills:
            selected = []
            if isinstance(plan_payload, dict):
                selected = plan_payload.get("selected_skills", []) or []
            if isinstance(selected, list):
                selected = [str(s) for s in selected if isinstance(s, str) and s.strip()]
            else:
                selected = []

            self.selected_skills = selected
            if self.selected_skills:
                logger.info("Planner selected skills: %s", ", ".join(self.selected_skills))
            else:
                logger.info("Planner selected no skills.")
            skill_contents = load_skill_contents(
                self.available_skills,
                self.selected_skills,
                max_chars=self.skills_max_chars or None,
            )
            self.skill_context = format_skill_context(skill_contents)
            if self.skill_context:
                final_task = (
                    f"{final_task}\n\nSelected skills (planner-chosen):\n"
                    f"{self.skill_context}"
                )

        self.task = final_task
        self.initiate_messages()

    def _format_plan_payload(self, payload: dict) -> str:
        lines: list[str] = []
        iteration = payload.get("iteration_info")
        if isinstance(iteration, dict):
            current = iteration.get("current_iteration")
            total = iteration.get("total_iterations")
            if current and total:
                lines.append(f"Iteration: {current}/{total}")

        search_summary = payload.get("search_summary")
        if isinstance(search_summary, str) and search_summary.strip():
            lines.append(f"Search summary: {search_summary.strip()}")

        selected = payload.get("selected_skills")
        if isinstance(selected, list):
            selected_clean = [str(s) for s in selected if isinstance(s, str) and s.strip()]
            if selected_clean:
                lines.append(f"Selected skills: {', '.join(selected_clean)}")

        natural_plan = payload.get("natural_language_plan")
        if isinstance(natural_plan, str) and natural_plan.strip():
            lines.append("Plan:")
            lines.append(natural_plan.strip())
        else:
            steps = payload.get("step_by_step_plan")
            if isinstance(steps, list) and steps:
                lines.append("Plan:")
                for step in steps:
                    if not isinstance(step, dict):
                        continue
                    desc = step.get("description") or ""
                    info = step.get("important_search_info") or ""
                    if not desc:
                        continue
                    if info:
                        lines.append(f"- {desc} (search: {info})")
                    else:
                        lines.append(f"- {desc}")

        return "\n".join(lines) if lines else json.dumps(payload, ensure_ascii=False)

    def _too_many_failures(self) -> bool:
        if self.consecutive_failures >= self.max_failures:
            if self._post_action_observation_required:
                self.execution_journal.event(
                    "max_failures_deferred_until_post_action_observation",
                    step=self.n_steps,
                    consecutive_failures=self.consecutive_failures,
                    max_failures=self.max_failures,
                )
                self._add_log_window("已达到失败阈值，但动作后仍需先截图复核", "warning")
                return False
            if not getattr(self, "_stop_reason", None):
                self._stop_reason = (
                    f"连续 {self.max_failures} 次失败或无有效进展，"
                    "已停止以避免继续重复无效策略。"
                )
            if not getattr(self, "assistance_request", None):
                self._request_user_assistance(
                    reason_code="max_failures",
                    reason=self._stop_reason,
                    user_action_required=self._assistance_user_action_for_reason("element_not_found"),
                    resume_condition=self._assistance_resume_condition_for_reason("element_not_found"),
                    confidence=0.75,
                )
            logger.error(
                "Stopping due to %s consecutive failures: %s",
                self.max_failures,
                self._stop_reason,
            )
            return True
        return False

    async def _handle_control_flags(self) -> bool:
        if self._stopped:
            logger.info("Agent stopped")
            return False

        while self._paused:
            await asyncio.sleep(0.2)
            if self._stopped:
                return False

        return True

    def stop(self, reason: Optional[str] = None) -> None:
        if reason:
            logger.warning("Stopping agent: %s", reason)
            self._stop_reason = reason
        self._stopped = True
        # 更新状态窗口显示停止原因
        if self._status_window and reason:
            self._add_log_window(f"智能体已停止：{reason}", "warning")

    def save_history(self, file_path: Optional[str | Path] = None) -> None:
        if not file_path:
            file_path = "AgentHistory.json"
        self.history.save_to_file(file_path)

    def _update_status_window(self, step_number: int, goal: str, status: str) -> None:
        """更新状态窗口（如果启用）"""
        if self._status_window:
            try:
                self._status_window.update_step(step_number, goal, status)
            except Exception as e:
                logger.debug(f"Failed to update status window: {e}")

    def _add_log_window(self, message: str, level: str = "info") -> None:
        """添加日志到状态窗口（如果启用）"""
        if self._status_window:
            try:
                self._status_window.add_log(message, level)
            except Exception as e:
                logger.debug(f"Failed to add log to status window: {e}")

    def _add_detail_window(self, message: str) -> None:
        """添加详情到状态窗口（如果启用）"""
        if self._status_window:
            try:
                self._status_window.add_detail(message)
            except Exception as e:
                logger.debug(f"Failed to add detail to status window: {e}")

    def _update_timing(self, step_number: int, elapsed: float) -> None:
        """更新耗时显示（如果启用）"""
        if self._status_window:
            try:
                self._status_window.update_timing(elapsed)
            except Exception as e:
                logger.debug(f"Failed to update timing: {e}")

    def _step_timing_breakdown(self, step_id: int, step_total_time: float) -> dict[str, Any]:
        """Return timing buckets that add up to the visible step total."""
        timing = getattr(self, '_step_timing', {}).get(step_id, {})
        screenshot_time = 0.0
        screenshot_rect_time = 0.0
        screenshot_capture_time = 0.0
        screenshot_downscale_time = 0.0
        screenshot_save_time = 0.0
        screenshot_backend = ""
        brain_inference_time = 0.0
        brain_other_time = 0.0
        actor_inference_time = 0.0
        action_execution_time = 0.0
        memory_update_time = 0.0
        history_create_time = 0.0

        if timing:
            actor_inference_precomputed = bool(timing.get("actor_inference_precomputed"))

            if timing.get("screenshot_start") and timing.get("screenshot_end"):
                screenshot_time = timing["screenshot_end"] - timing["screenshot_start"]
            screenshot_rect_time = float(timing.get("screenshot_rect_time", 0) or 0)
            screenshot_capture_time = float(timing.get("screenshot_capture_time", 0) or 0)
            screenshot_downscale_time = float(timing.get("screenshot_downscale_time", 0) or 0)
            screenshot_save_time = float(timing.get("screenshot_save_time", 0) or 0)
            screenshot_backend = str(timing.get("screenshot_backend", "") or "")

            if timing.get("brain_start") and timing.get("brain_end"):
                brain_inference_time = timing["brain_end"] - timing["brain_start"]

            if timing.get("screenshot_end") and timing.get("brain_start"):
                brain_other_time += timing["brain_start"] - timing["screenshot_end"]
            if timing.get("brain_step_end") and timing.get("brain_end"):
                brain_other_time += timing["brain_step_end"] - timing["brain_end"]

            if timing.get("actor_inference_start") and timing.get("actor_inference_end"):
                actor_inference_time = timing["actor_inference_end"] - timing["actor_inference_start"]
            if actor_inference_precomputed and brain_inference_time:
                actor_inference_time = brain_inference_time
                brain_inference_time = 0.0

            if timing.get("action_exec_start") and timing.get("action_exec_end"):
                action_execution_time = timing["action_exec_end"] - timing["action_exec_start"]

            memory_update_time = float(timing.get("memory_update_time", 0) or 0)
            history_create_time = float(timing.get("history_create_time", 0) or 0)

        accounted = (
            screenshot_time + brain_inference_time + brain_other_time +
            actor_inference_time + action_execution_time +
            memory_update_time + history_create_time
        )
        other_overhead_time = max(0.0, step_total_time - accounted)
        screenshot_accounted = (
            screenshot_rect_time + screenshot_capture_time +
            screenshot_downscale_time + screenshot_save_time
        )
        screenshot_overhead_time = max(0.0, screenshot_time - screenshot_accounted)

        return {
            "screenshot_time": screenshot_time,
            "screenshot_rect_time": screenshot_rect_time,
            "screenshot_capture_time": screenshot_capture_time,
            "screenshot_downscale_time": screenshot_downscale_time,
            "screenshot_save_time": screenshot_save_time,
            "screenshot_overhead_time": screenshot_overhead_time,
            "screenshot_backend": screenshot_backend,
            "brain_inference_time": brain_inference_time,
            "brain_other_time": brain_other_time,
            "actor_inference_time": actor_inference_time,
            "action_execution_time": action_execution_time,
            "memory_update_time": memory_update_time,
            "history_create_time": history_create_time,
            "other_overhead_time": other_overhead_time,
            "actor_inference_precomputed": bool(timing.get("actor_inference_precomputed")),
        }

    def _save_step_record(self, step_id: int, step_total_time: float) -> None:
        """保存步骤记录（如果启用）"""
        try:
            # 从 _step_timing 获取当前步骤的时间记录
            timing = getattr(self, '_step_timing', {}).get(step_id, {})

            # 使用传入的 step_total_time（从 actor_step 结束时计算）
            # 如果为 0 则从 timing 获取
            if step_total_time <= 0 and timing.get("step_total"):
                step_total_time = timing["step_total"]
            step_error = getattr(self, "_step_errors", {}).get(step_id, "")
            if timing.get("brain_error") and not step_error:
                step_error = f"Brain error: {timing['brain_error']}"
            if timing.get("actor_error") and not step_error:
                step_error = f"Actor error: {timing['actor_error']}"

            breakdown = self._step_timing_breakdown(step_id, step_total_time)

            details = {
                "截图区域检测": round(breakdown["screenshot_rect_time"], 3),
                "截图后端": breakdown["screenshot_backend"],
                "截图抓取": round(breakdown["screenshot_capture_time"], 3),
                "截图缩放": round(breakdown["screenshot_downscale_time"], 3),
                "截图保存": round(breakdown["screenshot_save_time"], 3),
                "截图调度开销": round(breakdown["screenshot_overhead_time"], 3),
            }
            if breakdown["actor_inference_precomputed"]:
                details["结果解析"] = round(breakdown["brain_other_time"], 3)
            else:
                details["Brain 推理"] = round(breakdown["brain_inference_time"], 3)
                details["Brain 其他"] = round(breakdown["brain_other_time"], 3)
            details.update(
                {
                    "Actor 推理": round(breakdown["actor_inference_time"], 3),
                    "动作执行": round(breakdown["action_execution_time"], 3),
                    "内存更新": round(breakdown["memory_update_time"], 3),
                    "历史记录创建": round(breakdown["history_create_time"], 3),
                    "代码执行开销": round(breakdown["other_overhead_time"], 3),
                    "系统开销": {
                        "内存更新": round(breakdown["memory_update_time"], 3),
                        "历史记录创建": round(breakdown["history_create_time"], 3),
                        "代码执行开销": round(breakdown["other_overhead_time"], 3),
                    },
                    "步骤总计": round(step_total_time, 3),
                }
            )

            # 验证：各分项之和应等于 step_total
            verified_sum = sum(
                value
                for key, value in details.items()
                if key not in {"步骤总计", "系统开销"} and isinstance(value, (int, float))
            )
                    
            if abs(verified_sum - step_total_time) > 0.01:
                logger.debug(f"Time mismatch for step {step_id}: {verified_sum:.3f} vs {step_total_time:.3f}")
                # 调整代码执行开销使总和等于 step_total
                delta = step_total_time - verified_sum
                details["代码执行开销"] = round(details["代码执行开销"] + delta, 3)
                details["系统开销"]["代码执行开销"] = round(details["系统开销"]["代码执行开销"] + delta, 3)

            # 收集动作信息
            actions_info = []
            last_action = getattr(self, 'last_step_action', None)
            if last_action:
                for action in last_action:
                    if isinstance(action, dict):
                        for action_name, action_params in action.items():
                            if action_params is not None:
                                action_record = {
                                    "动作": action_name,
                                    "参数": action_params
                                }
                                # 为点击动作添加点击类型信息
                                if action_name in ["Click", "RightSingle", "DoubleClick"]:
                                    click_type_map = {
                                        "Click": "左键点击",
                                        "RightSingle": "右键点击",
                                        "DoubleClick": "双击"
                                    }
                                    action_record["点击类型"] = click_type_map.get(action_name, action_name)
                                
                                actions_info.append(action_record)

            # 获取思考内容（从 brain_context 中获取）
            thinking_content = ""
            brain_context = getattr(self, 'brain_context', {})
            if step_id in brain_context:
                step_context = brain_context[step_id]
                # 提取 analysis 字段作为思考内容
                if isinstance(step_context, dict):
                    analysis = step_context.get("analysis", "")
                    if isinstance(analysis, dict):
                        # 如果 analysis 是字典，提取关键信息
                        thinking_content = json.dumps(analysis, ensure_ascii=False)[:2000]
                    elif isinstance(analysis, str):
                        thinking_content = analysis[:2000]

            # 获取步骤评估结果
            step_evaluate = ""
            if step_id in brain_context:
                step_context = brain_context[step_id]
                if isinstance(step_context, dict):
                    current_state = step_context.get("current_state", {})
                    if isinstance(current_state, dict):
                        step_evaluate = current_state.get("step_evaluate", "")
            if step_error and not step_evaluate:
                step_evaluate = "Error"

            # 构建完整的记录（使用中文键名）
            record = {
                "步骤序号": step_id,
                "时间戳": datetime.now().isoformat(),
                "目标": self.next_goal[:500] if self.next_goal else "",
                "思考": thinking_content[:2000] if thinking_content else "",
                "步骤评估": step_evaluate,
                "截图": getattr(self, "_current_screenshot_path", None),
                "计时": details,
                "动作": actions_info,
                "动作数量": len(actions_info),
                "状态": "error" if step_error else "success"
            }
            if step_error:
                record["错误"] = step_error

            # 保存到状态窗口（如果启用）
            if self._status_window:
                self._status_window.save_record(record)

            # 保存到 JSONL 文件
            self._write_step_to_jsonl(record)
            self._write_step_record_json(record)

        except Exception as e:
            logger.debug(f"保存步骤记录失败：{e}")

    def _write_step_record_json(self, record: dict) -> None:
        """写入固定 JSON 步骤记录文件，供外部工具读取最新运行记录。"""
        self._step_records.append(record)
        try:
            if self._step_record_json_path:
                total_actions = sum(int(item.get("动作数量", 0) or 0) for item in self._step_records)
                data = {
                    "任务": self.original_task,
                    "开始时间": self.current_time.isoformat(),
                    "更新时间": datetime.now().isoformat(),
                    "智能体ID": self.agent_id,
                    "步骤记录": self._step_records,
                    "动作数量": total_actions,
                    "任务总结": self._generate_step_record_summary(),
                }
                self._step_record_json_path.parent.mkdir(parents=True, exist_ok=True)
                tmp_path = self._step_record_json_path.with_suffix(".json.tmp")
                with open(tmp_path, "w", encoding="utf-8") as f:
                    json.dump(data, f, indent=2, ensure_ascii=False)
                os.replace(tmp_path, self._step_record_json_path)
            self._write_training_dataset(status="running", reason="running")
        except Exception as e:
            logger.debug("写入步骤 JSON 记录失败：%s", e)

    def _normalized_action(self, action: dict[str, Any]) -> dict[str, Any]:
        raw_name = str(action.get("动作", "") or "unknown")
        params = action.get("参数", {})
        if not isinstance(params, dict):
            params = {"value": params}

        name_map = {
            "Click": "click",
            "RightSingle": "right_click",
            "DoubleClick": "double_click",
            "input_text": "input_text",
            "Hotkey": "hotkey",
            "Press": "press_key",
            "Scroll": "scroll",
            "Wait": "wait",
            "open_app": "open_app",
        }
        normalized = {"type": name_map.get(raw_name, raw_name)}
        normalized.update(params)
        if raw_name == "Click":
            normalized.setdefault("button", "left")
        elif raw_name == "RightSingle":
            normalized.setdefault("button", "right")
        return normalized

    def _step_screenshot_source(self, record: dict[str, Any]) -> Optional[Path]:
        screenshot = record.get("截图")
        if screenshot:
            path = Path(str(screenshot)).expanduser()
            if path.exists():
                return path
        images_dir = getattr(self, "images_dir", None)
        step_id = record.get("步骤序号")
        if images_dir and step_id is not None:
            fallback = Path(images_dir).expanduser() / f"screenshot_{step_id}.png"
            if fallback.exists():
                return fallback
        return None

    def _copy_training_screenshot(
        self,
        record: dict[str, Any],
        screenshots_dir: Path,
    ) -> tuple[Optional[str], Optional[list[int]]]:
        source = self._step_screenshot_source(record)
        if source is None:
            return None, None

        step_id = int(record.get("步骤序号", 0) or 0)
        target_name = f"step_{step_id:04d}.png"
        target = screenshots_dir / target_name
        screenshots_dir.mkdir(parents=True, exist_ok=True)
        try:
            if not target.exists() or source.resolve() != target.resolve():
                shutil.copy2(source, target)
            with Image.open(target) as image:
                size = [int(image.width), int(image.height)]
            return f"screenshots/{target_name}", size
        except Exception as e:
            logger.debug("复制训练截图失败：%s", e)
            return None, None

    def _build_training_steps(
        self,
        screenshots_dir: Path,
    ) -> list[dict[str, Any]]:
        training_steps: list[dict[str, Any]] = []
        action_history: list[dict[str, Any]] = []
        for record in self._step_records:
            actions = [
                self._normalized_action(action)
                for action in record.get("动作", [])
                if isinstance(action, dict)
            ]
            image_path, image_size = self._copy_training_screenshot(record, screenshots_dir)
            observation: dict[str, Any] = {"screenshot": image_path}
            if image_size:
                observation["screenshot_size"] = image_size

            item = {
                "task_id": self.agent_id,
                "step": record.get("步骤序号"),
                "timestamp": record.get("时间戳"),
                "instruction": self.original_task,
                "subgoal": record.get("目标", ""),
                "observation": observation,
                "history": list(action_history),
                "target_action": actions[0] if len(actions) == 1 else None,
                "target_actions": actions,
                "result": record.get("状态", ""),
                "step_evaluate": record.get("步骤评估", ""),
                "thought_or_state": record.get("思考", ""),
                "timing": record.get("计时", {}),
                "source_record": record,
            }
            if record.get("错误"):
                item["error"] = record.get("错误")
            training_steps.append(item)
            action_history.extend(actions)
        return training_steps

    def _write_training_dataset(
        self,
        *,
        status: str,
        reason: str = "",
        total_run_time: Optional[float] = None,
        max_steps: Optional[int] = None,
    ) -> None:
        """Export CUA training data grouped by task under .kylin_tmp/datasets."""
        dataset_dir = getattr(self, "_training_dataset_dir", None)
        if not dataset_dir:
            return
        try:
            screenshots_dir = dataset_dir / "screenshots"
            steps = self._build_training_steps(screenshots_dir)
            total_actions = sum(len(item.get("target_actions", [])) for item in steps)
            trajectory = {
                "schema_version": "openkylin-cua-trajectory-v1",
                "task_id": self.agent_id,
                "os": "openKylin",
                "platform": platform.platform(),
                "instruction": self.original_task,
                "status": status,
                "reason": reason,
                "start_time": self.current_time.isoformat(),
                "updated_time": datetime.now().isoformat(),
                "total_steps": len(steps),
                "total_actions": total_actions,
                "total_run_time": round(total_run_time, 3) if total_run_time is not None else None,
                "max_steps": max_steps,
                "source": {
                    "step_records": (
                        str(self._step_record_json_path)
                        if self._step_record_json_path
                        else None
                    ),
                    "images_dir": str(getattr(self, "images_dir", "") or ""),
                },
                "steps": steps,
            }

            dataset_dir.mkdir(parents=True, exist_ok=True)
            trajectory_tmp = dataset_dir / "trajectory.json.tmp"
            with open(trajectory_tmp, "w", encoding="utf-8") as f:
                json.dump(trajectory, f, indent=2, ensure_ascii=False)
            os.replace(trajectory_tmp, dataset_dir / "trajectory.json")

            jsonl_tmp = dataset_dir / "steps.jsonl.tmp"
            with open(jsonl_tmp, "w", encoding="utf-8") as f:
                for item in steps:
                    f.write(json.dumps(item, ensure_ascii=False) + "\n")
            os.replace(jsonl_tmp, dataset_dir / "steps.jsonl")
        except Exception as e:
            logger.debug("写入 CUA 训练数据失败：%s", e)

    def _generate_step_record_summary(self) -> str:
        if not self._step_records:
            return ""
        last = self._step_records[-1]
        error_count = sum(1 for item in self._step_records if item.get("状态") == "error")
        return (
            f"已记录 {len(self._step_records)} 个步骤，"
            f"错误 {error_count} 个，"
            f"最新目标：{str(last.get('目标', ''))[:120]}"
        )

    def _write_step_to_jsonl(self, record: dict) -> None:
        """将步骤记录写入 JSONL 文件（使用清晰易读的格式）"""
        if not self._step_log_path:
            return

        try:
            with open(self._step_log_path, 'a', encoding='utf-8') as f:
                # 写入分割线
                f.write("=" * 80 + "\n")
                
                # 写入步骤序号
                f.write(f"步骤 {record.get('步骤序号', '?')}:\n")
                
                # 写入目标
                f.write(f"  目标：{record.get('目标', '')}\n")
                
                # 写入思考
                thinking = record.get('思考', '')
                if thinking:
                    f.write(f"  思考：{thinking}\n")
                
                # 写入步骤评估
                step_eval = record.get('步骤评估', '')
                if step_eval:
                    f.write(f"  评估：{step_eval}\n")
                error = record.get('错误', '')
                if error:
                    f.write(f"  错误：{error}\n")
                
                # 写入动作详情
                actions = record.get('动作', [])
                if actions:
                    f.write("  动作详情:\n")
                    for i, action in enumerate(actions, 1):
                        action_name = action.get('动作', 'unknown')
                        params = action.get('参数', {})
                        f.write(f"    {i}. {action_name}: {params}\n")
                
                # 写入总耗时
                timing = record.get('计时', {})
                step_total = timing.get('步骤总计', 0)
                f.write(f"  总耗时：{step_total:.3f}秒\n")

                # 写入耗时详情
                f.write("  耗时详情:\n")
                if "截图区域检测" in timing:
                    f.write(f"    截图区域检测：{timing.get('截图区域检测', 0):.3f}秒\n")
                    if timing.get("截图后端"):
                        f.write(f"    截图后端：{timing.get('截图后端')}\n")
                    f.write(f"    截图抓取：{timing.get('截图抓取', 0):.3f}秒\n")
                    f.write(f"    截图缩放：{timing.get('截图缩放', 0):.3f}秒\n")
                    f.write(f"    截图保存：{timing.get('截图保存', 0):.3f}秒\n")
                    f.write(f"    截图调度开销：{timing.get('截图调度开销', 0):.3f}秒\n")
                else:
                    f.write(f"    截图：{timing.get('截图', 0):.3f}秒\n")
                if "结果解析" in timing:
                    f.write(f"    结果解析：{timing.get('结果解析', 0):.3f}秒\n")
                else:
                    f.write(f"    Brain 推理：{timing.get('Brain 推理', 0):.3f}秒\n")
                    f.write(f"    Brain 其他：{timing.get('Brain 其他', 0):.3f}秒\n")
                f.write(f"    Actor 推理：{timing.get('Actor 推理', 0):.3f}秒\n")
                f.write(f"    动作执行：{timing.get('动作执行', 0):.3f}秒\n")
                f.write(f"    内存更新：{timing.get('内存更新', 0):.3f}秒\n")
                f.write(f"    历史记录创建：{timing.get('历史记录创建', 0):.3f}秒\n")
                f.write(f"    代码执行开销：{timing.get('代码执行开销', 0):.3f}秒\n")
                
                # 系统开销（支持嵌套结构）
                system_overhead = timing.get('系统开销', 0)
                if isinstance(system_overhead, dict):
                    f.write("    系统开销:\n")
                    f.write(f"      内存更新：{system_overhead.get('内存更新', 0):.3f}秒\n")
                    f.write(f"      历史记录创建：{system_overhead.get('历史记录创建', 0):.3f}秒\n")
                    f.write(f"      代码执行开销：{system_overhead.get('代码执行开销', 0):.3f}秒\n")
                else:
                    f.write(f"    系统开销：{system_overhead:.3f}秒\n")

                # 写入状态和时间戳
                f.write(f"  状态：{record.get('状态', '?')}\n")
                f.write(f"  时间：{record.get('时间戳', '')}\n")
                
                # 写入空行分隔
                f.write("\n")
                
            logger.debug(f"步骤 {record.get('步骤序号')} 已写入 {self._step_log_path}")
        except Exception as e:
            logger.debug(f"写入步骤日志失败：{e}")

    def _finalize_session(self, reason: str, total_run_time: float) -> None:
        """完成会话并保存最终日志"""
        if self._status_window:
            try:
                from src.utils.status_window import finalize_session
                finalize_session(reason)
                logger.info(f"Session finalized: {reason}, total time: {total_run_time:.2f}s")
            except Exception as e:
                logger.debug(f"Failed to finalize session: {e}")

    @staticmethod
    def _status_auto_close_on_success_enabled() -> bool:
        raw = os.getenv("KYLIN_STATUS_AUTO_CLOSE_ON_SUCCESS", "1").strip().lower()
        return raw not in {"0", "false", "no", "off"}

    def _close_status_window_on_success(self, final_status: str, reason: str) -> None:
        """任务成功完成后关闭监视窗口，错误/停止时保留窗口方便排查。"""
        if final_status != "success" or reason != "task_completed":
            return
        if not self._status_auto_close_on_success_enabled():
            return
        try:
            from src.utils.status_window import close_status_window

            close_status_window()
            self._status_window = None
            logger.info("Status window closed after successful task completion")
        except Exception as e:
            logger.debug("Failed to close status window after success: %s", e)

    def initiate_messages(self):
        self.brain_message_manager = MessageManager(
            llm=self.brain_llm,
            task=self.task,
            action_descriptions=self.controller.registry.get_prompt_description(),
            system_prompt_class=BrainPrompt_kylin,
            max_input_tokens=self.max_input_tokens,
            include_attributes=self.include_attributes,
            max_error_length=self.max_error_length,
            max_actions_per_step=self.max_actions_per_step,
            give_task=True,
        )
        self.actor_message_manager = MessageManager(
            llm=self.actor_llm,
            task=self.task,
            action_descriptions=self.controller.registry.get_prompt_description(),
            system_prompt_class=ActorPrompt_kylin,
            max_input_tokens=self.max_input_tokens,
            include_attributes=self.include_attributes,
            max_error_length=self.max_error_length,
            max_actions_per_step=self.max_actions_per_step,
            give_task=False,
        )
        if self.combined_llm:
            self.combined_message_manager = MessageManager(
                llm=self.combined_llm,
                task=self.task,
                action_descriptions=self.controller.registry.get_prompt_description(),
                system_prompt_class=CombinedPrompt_kylin,
                max_input_tokens=self.max_input_tokens,
                include_attributes=self.include_attributes,
                max_error_length=self.max_error_length,
                max_actions_per_step=self.max_actions_per_step,
                give_task=True,
            )
        else:
            self.combined_message_manager = None
        self.memory_message_manager = MessageManager(
            llm=self.memory_llm,
            task=self.task,
            action_descriptions=self.controller.registry.get_prompt_description(),
            system_prompt_class=MemoryPrompt,
            max_input_tokens=self.max_input_tokens,
            include_attributes=self.include_attributes,
            max_error_length=self.max_error_length,
            max_actions_per_step=self.max_actions_per_step,
            give_task=True,
        )
