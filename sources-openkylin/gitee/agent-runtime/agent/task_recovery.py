"""Recovery policy helpers for interrupted tool and subagent execution."""

from __future__ import annotations

from contextlib import contextmanager
from contextvars import ContextVar
import threading
import json
from typing import Any, Dict, List, Tuple


_SAFE_RETRY_TOOLS = frozenset(
    {
        "read_file",
        "search_files",
        "web_search",
        "web_extract",
        "skills_list",
        "skill_view",
        "memory",
        "todo",
    }
)

_VERIFY_EFFECTS_TOOLS = frozenset(
    {
        "execute_code",
        "terminal",
        "write_file",
        "patch",
        "browser",
        "computer",
    }
)

_NEVER_BLIND_RETRY_MARKERS = (
    "send",
    "email",
    "message",
    "post",
    "publish",
    "create_event",
    "payment",
    "purchase",
    "delete",
)

_checkpoint_context: ContextVar[Dict[str, Any] | None] = ContextVar(
    "task_checkpoint_context",
    default=None,
)
_task_contexts: Dict[str, Dict[str, Any]] = {}
_task_contexts_lock = threading.Lock()


@contextmanager
def task_checkpoint_context(
    db,
    task_run_id: str | None,
    subrun_id: str | None = None,
    task_id: str | None = None,
):
    """Bind durable checkpoint identity to the current execution thread."""
    value = (
        {
            "db": db,
            "task_run_id": task_run_id,
            "subrun_id": subrun_id,
        }
        if db is not None and task_run_id
        else None
    )
    token = _checkpoint_context.set(value)
    if task_id and value:
        with _task_contexts_lock:
            _task_contexts[task_id] = value
    try:
        yield
    finally:
        if task_id and value:
            with _task_contexts_lock:
                if _task_contexts.get(task_id) is value:
                    _task_contexts.pop(task_id, None)
        _checkpoint_context.reset(token)


def record_task_checkpoint(
    kind: str,
    location: str,
    *,
    status: str = "completed",
    payload: Any = None,
    task_id: str | None = None,
) -> bool:
    """Best-effort append at a semantic safe point in the active task."""
    context = _checkpoint_context.get()
    if context is None and task_id:
        with _task_contexts_lock:
            context = _task_contexts.get(task_id)
    if not context:
        return False
    try:
        context["db"].add_task_checkpoint(
            context["task_run_id"],
            subrun_id=context.get("subrun_id"),
            kind=kind,
            location=location,
            status=status,
            payload=payload,
        )
        return True
    except Exception:
        return False


def recovery_policy_for_tool(tool_name: str, args: Any = None) -> str:
    """Classify how an interrupted call may be recovered.

    Policies:
    - ``safe_retry``: observational/idempotent reads may be repeated.
    - ``resume_children``: inspect durable branch records and resume only
      incomplete children.
    - ``verify_effects``: inspect filesystem/process state before deciding
      whether any part needs to be repeated.
    - ``confirm_before_retry``: external or destructive effects must not be
      repeated without proving they did not happen (or obtaining confirmation).
    """
    normalized = str(tool_name or "").strip().lower()
    if normalized == "delegate_task":
        return "resume_children"
    if normalized in _SAFE_RETRY_TOOLS:
        return "safe_retry"
    if normalized in _VERIFY_EFFECTS_TOOLS:
        return "verify_effects"
    if any(marker in normalized for marker in _NEVER_BLIND_RETRY_MARKERS):
        return "confirm_before_retry"
    return "verify_effects"


def recovery_instruction(policy: str, tool_name: str, args: Any = None) -> str:
    normalized = str(policy or "verify_effects")
    tool = str(tool_name or "unknown")
    if normalized == "safe_retry":
        return f"The interrupted {tool} call is read-only and may be retried if its result is absent."
    if normalized == "resume_children":
        return (
            "Use durable child-run records: reuse completed subagent results and "
            "resume only interrupted/unfinished branches."
        )
    if normalized == "confirm_before_retry":
        return (
            f"Do not repeat the interrupted {tool} call until its external effect "
            "has been checked; ask the user if the outcome cannot be verified."
        )
    return (
        f"Do not blindly rerun interrupted {tool}. First inspect files, processes, "
        "and recorded outputs to determine which effects already happened, then "
        "continue only the missing portion."
    )


def summarize_subrun_for_recovery(subrun: Dict[str, Any]) -> Dict[str, Any]:
    """Return a bounded, prompt/API-safe child checkpoint."""
    result = subrun.get("result_json")
    if isinstance(result, str) and len(result) > 4000:
        result = result[:4000] + "...[truncated]"
    return {
        "id": subrun.get("id"),
        "parent_subrun_id": subrun.get("parent_subrun_id"),
        "child_session_id": subrun.get("child_session_id"),
        "goal": subrun.get("goal"),
        "status": subrun.get("status"),
        "current_tool": subrun.get("current_tool"),
        "current_tool_args": subrun.get("current_tool_args"),
        "recovery_policy": subrun.get("current_recovery_policy"),
        "result": result,
        "interruption_reason": subrun.get("interruption_reason"),
        "recovery_attempts": subrun.get("recovery_attempts", 0),
    }


def prepare_recovery_history(
    history: List[Dict[str, Any]],
) -> Tuple[List[Dict[str, Any]], List[Dict[str, Any]]]:
    """Make a crashed transcript protocol-valid without discarding context.

    A hard exit can occur after an assistant tool-call message is committed
    but before every matching tool result is stored. Providers reject a new
    user message after such a dangling call. Insert explicit synthetic
    interruption results for only the missing calls; completed results and all
    surrounding conversation remain authoritative.
    """
    repaired: List[Dict[str, Any]] = []
    synthesized: List[Dict[str, Any]] = []
    index = 0
    while index < len(history):
        message = history[index]
        repaired.append(message)
        index += 1
        if not isinstance(message, dict) or message.get("role") != "assistant":
            continue
        tool_calls = message.get("tool_calls")
        if not isinstance(tool_calls, list) or not tool_calls:
            continue

        answered = set()
        while index < len(history):
            candidate = history[index]
            if not isinstance(candidate, dict) or candidate.get("role") != "tool":
                break
            repaired.append(candidate)
            if candidate.get("tool_call_id"):
                answered.add(str(candidate["tool_call_id"]))
            index += 1

        for call in tool_calls:
            if not isinstance(call, dict):
                continue
            call_id = str(call.get("id") or call.get("tool_call_id") or "")
            if not call_id or call_id in answered:
                continue
            function = call.get("function")
            name = (
                function.get("name")
                if isinstance(function, dict)
                else call.get("name")
            ) or "unknown"
            synthetic = {
                "role": "tool",
                "tool_call_id": call_id,
                "tool_name": str(name),
                "content": json.dumps(
                    {
                        "status": "interrupted",
                        "result_available": False,
                        "message": (
                            "Execution stopped before a durable tool result was "
                            "recorded. Verify possible side effects before retrying."
                        ),
                    },
                    ensure_ascii=False,
                ),
            }
            repaired.append(synthetic)
            synthesized.append(
                {
                    "tool_call_id": call_id,
                    "tool": str(name),
                }
            )
    return repaired, synthesized
