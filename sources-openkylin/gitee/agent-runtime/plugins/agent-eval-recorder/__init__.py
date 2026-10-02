"""Opt-in event recorder for agent behavior evals.

The plugin is intentionally inert unless both conditions are true:

1. the plugin is enabled in ``plugins.enabled``;
2. ``HERMES_AGENT_EVAL_RECORD`` is set to a truthy value.

It writes JSONL events that ``agent_eval_runner.py --trace-events`` can read.
Hook failures are swallowed so recording never changes task behavior.
"""

from __future__ import annotations

import json
import os
import threading
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from kylin_agent_runtime_constants import get_hermes_home
from utils import env_var_enabled

_LOCK = threading.Lock()


def _enabled() -> bool:
    return env_var_enabled("HERMES_AGENT_EVAL_RECORD")


def _trace_file() -> Path:
    override = os.getenv("HERMES_AGENT_EVAL_TRACE_FILE")
    if override:
        return Path(override).expanduser()
    return get_hermes_home() / "eval_traces" / "agent_events.jsonl"


def _safe_json(value: Any) -> Any:
    if isinstance(value, str):
        try:
            parsed = json.loads(value)
        except json.JSONDecodeError:
            return value
        return parsed
    if isinstance(value, (dict, list, int, float, bool)) or value is None:
        return value
    return repr(value)


def _append_event(event: dict[str, Any]) -> None:
    if not _enabled():
        return
    try:
        event.setdefault("timestamp", datetime.now(timezone.utc).isoformat())
        path = _trace_file()
        path.parent.mkdir(parents=True, exist_ok=True)
        line = json.dumps(event, ensure_ascii=False, default=repr)
        with _LOCK:
            with path.open("a", encoding="utf-8") as fh:
                fh.write(line + "\n")
    except Exception:
        return


def _on_post_tool_call(
    *,
    tool_name: str = "",
    args: Any = None,
    result: Any = None,
    task_id: str = "",
    session_id: str = "",
    tool_call_id: str = "",
    duration_ms: int = 0,
    **_: Any,
) -> None:
    _append_event(
        {
            "type": "tool_call",
            "name": tool_name,
            "args": _safe_json(args),
            "result": _safe_json(result),
            "task_id": task_id,
            "session_id": session_id,
            "tool_call_id": tool_call_id,
            "duration_ms": duration_ms,
        }
    )


def _on_pre_gateway_dispatch(*, event: Any = None, **_: Any) -> None:
    payload = {
        "type": "gateway.event",
        "name": "message.received",
        "result": {},
    }
    for attr in (
        "source",
        "session_key",
        "chat_id",
        "thread_id",
        "user_id",
        "platform",
    ):
        value = getattr(event, attr, None)
        if value is not None:
            payload["result"][attr] = value
    text = getattr(event, "text", None)
    if text:
        payload["result"]["text_length"] = len(str(text))
    _append_event(payload)


def register(ctx) -> None:
    ctx.register_hook("post_tool_call", _on_post_tool_call)
    ctx.register_hook("pre_gateway_dispatch", _on_pre_gateway_dispatch)
