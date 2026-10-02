"""Per-session detail logging for gateway/API conversations.

The normal Hermes logs are component-wide.  This module appends a rich,
session-scoped audit trail under ``~/.kylin-agent-runtime/sessions`` so a single desktop
conversation can be inspected without filtering the global logs.
"""

from __future__ import annotations

import json
import re
import threading
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path
from types import SimpleNamespace
from typing import Any, Callable, Dict, Iterator, Optional

from kylin_agent_runtime_constants import get_hermes_home

_LOCK = threading.RLock()
_SUBSCRIBER_LOCK = threading.RLock()
_SUBSCRIBERS: Dict[str, Dict[int, Callable[[Dict[str, Any]], None]]] = {}
_NEXT_SUBSCRIBER_ID = 0
_DETAIL_CONTEXT = threading.local()
_SAFE_SESSION_CHARS = re.compile(r"[^A-Za-z0-9_.{}-]+")
_SENSITIVE_KEY_RE = re.compile(r"(api[_-]?key|authorization|token|password|secret|credential)", re.I)


@contextmanager
def session_detail_context(**metadata: Any) -> Iterator[None]:
    """Attach branch identity metadata to detail events in this thread.

    Delegated agents run in worker threads, so a thread-local context keeps
    concurrent branches isolated while allowing the existing detail writes to
    remain unchanged. Contexts are stackable for nested delegation.
    """
    previous = getattr(_DETAIL_CONTEXT, "metadata", None)
    merged = dict(previous or {})
    merged.update({key: value for key, value in metadata.items() if value is not None})
    _DETAIL_CONTEXT.metadata = merged
    try:
        yield
    finally:
        if previous is None:
            try:
                delattr(_DETAIL_CONTEXT, "metadata")
            except AttributeError:
                pass
        else:
            _DETAIL_CONTEXT.metadata = previous


def subscribe_session_details(
    session_id: Optional[str], callback: Callable[[Dict[str, Any]], None]
) -> Callable[[], None]:
    """Subscribe to newly written, already-redacted detail events.

    Returns an idempotent unsubscribe function. Subscribers are notified after
    the durable append and never run while the file lock is held.
    """
    if not session_id or not callable(callback):
        return lambda: None
    safe_id = _safe_session_id(session_id)
    global _NEXT_SUBSCRIBER_ID
    with _SUBSCRIBER_LOCK:
        _NEXT_SUBSCRIBER_ID += 1
        subscriber_id = _NEXT_SUBSCRIBER_ID
        _SUBSCRIBERS.setdefault(safe_id, {})[subscriber_id] = callback

    def _unsubscribe() -> None:
        with _SUBSCRIBER_LOCK:
            callbacks = _SUBSCRIBERS.get(safe_id)
            if callbacks is None:
                return
            callbacks.pop(subscriber_id, None)
            if not callbacks:
                _SUBSCRIBERS.pop(safe_id, None)

    return _unsubscribe


def _safe_session_id(session_id: Optional[str]) -> str:
    raw = str(session_id or "unknown").strip() or "unknown"
    return _SAFE_SESSION_CHARS.sub("_", raw)[:180]


def session_detail_log_path(session_id: Optional[str]) -> Path:
    sessions_dir = get_hermes_home() / "sessions"
    return sessions_dir / f"{_safe_session_id(session_id)}-details.log"


def session_usage_log_path(session_id: Optional[str]) -> Path:
    detail_path = session_detail_log_path(session_id)
    return detail_path.with_name(detail_path.name.replace("details", "usage"))


def legacy_session_detail_log_path(session_id: Optional[str]) -> Path:
    sessions_dir = get_hermes_home() / "sessions"
    return sessions_dir / f"{_safe_session_id(session_id)}-detials.log"


def _to_plain(value: Any, depth: int = 0) -> Any:
    if depth > 12:
        return repr(value)
    if value is None or isinstance(value, (bool, int, float, str)):
        return value
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, bytes):
        return f"<bytes:{len(value)}>"
    if isinstance(value, dict):
        result = {}
        for key, item in value.items():
            text_key = str(key)
            if _SENSITIVE_KEY_RE.search(text_key):
                result[text_key] = "***REDACTED***"
            else:
                result[text_key] = _to_plain(item, depth + 1)
        return result
    if isinstance(value, (list, tuple, set)):
        return [_to_plain(item, depth + 1) for item in value]
    if isinstance(value, SimpleNamespace):
        return _to_plain(vars(value), depth + 1)
    if hasattr(value, "model_dump"):
        try:
            return _to_plain(value.model_dump(), depth + 1)
        except Exception:
            pass
    if hasattr(value, "dict"):
        try:
            return _to_plain(value.dict(), depth + 1)
        except Exception:
            pass
    if hasattr(value, "__dict__"):
        try:
            return _to_plain(
                {k: v for k, v in vars(value).items() if not k.startswith("_")},
                depth + 1,
            )
        except Exception:
            pass
    return repr(value)


def _redact_text(text: str) -> str:
    try:
        from agent.redact import redact_sensitive_text
        return redact_sensitive_text(text)
    except Exception:
        return text


def write_session_detail(
    session_id: Optional[str],
    event: str,
    data: Any = None,
    *,
    text: Optional[str] = None,
) -> None:
    """Append one event to the session detail log.

    This function is intentionally best-effort: logging must never break agent
    execution.  It flushes every write so ``tail -f`` shows events in real time.
    """
    if not session_id:
        return
    try:
        path = session_detail_log_path(session_id)
        path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "ts": datetime.now(timezone.utc).astimezone().isoformat(timespec="milliseconds"),
            "event": str(event),
        }
        context_metadata = getattr(_DETAIL_CONTEXT, "metadata", None)
        if data is not None or context_metadata:
            plain_data = _to_plain(data) if data is not None else {}
            if context_metadata and isinstance(plain_data, dict):
                for key, value in context_metadata.items():
                    plain_data.setdefault(key, _to_plain(value))
            payload["data"] = plain_data
        if text is not None:
            payload["text"] = text

        # Newline-delimited JSON: one complete event per line.  This is valid
        # for real-time appends and straightforward for mind-map/tree viewers
        # to parse without needing to repair a partially written JSON array.
        body = json.dumps(payload, ensure_ascii=False, separators=(",", ":"), default=repr)
        body = _redact_text(body)
        with _LOCK:
            with path.open("a", encoding="utf-8") as fh:
                fh.write(body)
                fh.write("\n")
                fh.flush()
        # Notify with the redacted representation that was actually persisted,
        # never with the pre-redaction payload.
        emitted_payload = json.loads(body)
        with _SUBSCRIBER_LOCK:
            callbacks = list(_SUBSCRIBERS.get(_safe_session_id(session_id), {}).values())
        for callback in callbacks:
            try:
                callback(emitted_payload)
            except Exception:
                # Observability must never affect agent execution.
                pass
    except Exception:
        return


def write_session_usage(session_id: Optional[str], text: str) -> None:
    """Append one human-readable model-usage line to the session usage log."""
    if not session_id or not text:
        return
    try:
        path = session_usage_log_path(session_id)
        path.parent.mkdir(parents=True, exist_ok=True)
        ts = datetime.now(timezone.utc).astimezone().isoformat(timespec="milliseconds")
        line = _redact_text(f"{ts} {text}")
        with _LOCK:
            with path.open("a", encoding="utf-8") as fh:
                fh.write(line)
                fh.write("\n")
                fh.flush()
    except Exception:
        return


def describe_tools(tools: Any) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for tool in tools or []:
        plain = _to_plain(tool)
        if isinstance(plain, dict):
            fn = plain.get("function") if isinstance(plain.get("function"), dict) else {}
            result.append({
                "type": plain.get("type"),
                "name": fn.get("name") or plain.get("name"),
                "description": fn.get("description") or plain.get("description"),
                "schema": fn.get("parameters") or plain.get("parameters"),
            })
        else:
            result.append({"value": plain})
    return result
