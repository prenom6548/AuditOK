"""Tool-call execution — sequential and concurrent dispatch.

Both AIAgent methods (``_execute_tool_calls_sequential`` and
``_execute_tool_calls_concurrent``) live here as module-level
functions that take the parent ``AIAgent`` as their first argument.

``run_agent`` keeps thin wrappers so existing call sites work; tests
that patch ``run_agent._set_interrupt`` are honored because the
extracted functions reach back through the ``run_agent`` module via
``_ra()`` for that symbol.
"""

from __future__ import annotations

import concurrent.futures
import contextvars
import json
import logging
import os
import random
import threading
import time
from typing import Any, Optional

from agent.display import (
    KawaiiSpinner,
    build_tool_preview as _build_tool_preview,
    get_cute_tool_message as _get_cute_tool_message_impl,
    get_tool_emoji as _get_tool_emoji,
    _detect_tool_failure,
)
from agent.tool_guardrails import ToolGuardrailDecision
from agent.tool_dispatch_helpers import (
    _is_destructive_command,
    _is_multimodal_tool_result,
    _multimodal_text_summary,
    _append_subdir_hint_to_multimodal,
    make_tool_result_message,
)
from agent.session_detail_logger import write_session_detail
from tools.terminal_tool import (
    _get_approval_callback,
    _get_sudo_password_callback,
    set_approval_callback as _set_approval_callback,
    set_sudo_password_callback as _set_sudo_password_callback,
    get_active_env,
)
from tools.tool_result_storage import (
    maybe_persist_tool_result,
    enforce_turn_budget,
)
from utils import is_truthy_value

logger = logging.getLogger(__name__)

# Maximum number of concurrent worker threads for parallel tool execution.
# Mirrors the constant in ``run_agent`` for tests/imports that look here.
_MAX_TOOL_WORKERS = 8


def _ra():
    """Lazy reference to ``run_agent`` so patches like ``run_agent._set_interrupt`` work."""
    import run_agent
    return run_agent


def _detail_session_id(agent) -> Optional[str]:
    subagent_id = getattr(agent, "_subagent_id", None)
    delegate_depth = getattr(agent, "_delegate_depth", 0) or 0
    parent_session_id = getattr(agent, "_parent_session_id", None)
    if (
        (subagent_id or delegate_depth > 0)
        and isinstance(parent_session_id, str)
        and parent_session_id
    ):
        return parent_session_id
    return getattr(agent, "session_id", None)


def _write_detail(agent, event: str, data: dict) -> None:
    payload = dict(data or {})
    subagent_id = getattr(agent, "_subagent_id", None)
    delegate_depth = getattr(agent, "_delegate_depth", 0) or 0
    if subagent_id or delegate_depth > 0:
        payload.setdefault("scope", "branch")
        payload.setdefault("subagent_id", subagent_id)
        payload.setdefault("parent_subagent_id", getattr(agent, "_parent_subagent_id", None))
        payload.setdefault("delegate_depth", delegate_depth)
        payload.setdefault("child_session_id", getattr(agent, "session_id", None))
        payload.setdefault("routed_to_parent_session", _detail_session_id(agent) != getattr(agent, "session_id", None))
    else:
        payload.setdefault("scope", "trunk")
    write_session_detail(_detail_session_id(agent), event, payload)


def _mainline_branch_enabled() -> bool:
    cfg = {}
    try:
        from kylin_agent_runtime_cli.config import load_config_readonly

        cfg = (load_config_readonly().get("delegation") or {})
    except Exception:
        cfg = {}
    if not cfg:
        try:
            from cli import CLI_CONFIG

            cfg = CLI_CONFIG.get("delegation") or {}
        except Exception:
            cfg = {}
    if not isinstance(cfg, dict):
        return False
    if not is_truthy_value(cfg.get("enabled", True), default=True):
        return False
    return is_truthy_value(
        cfg.get("mainline_branch_enabled", False),
        default=False,
    )


def _todo_fanout_enabled() -> bool:
    """Return whether explicit parallel TODO groups may fan out automatically."""
    cfg = {}
    try:
        from kylin_agent_runtime_cli.config import load_config_readonly

        cfg = (load_config_readonly().get("delegation") or {})
    except Exception:
        cfg = {}
    if not cfg:
        try:
            from cli import CLI_CONFIG

            cfg = CLI_CONFIG.get("delegation") or {}
        except Exception:
            cfg = {}
    if not isinstance(cfg, dict):
        return False
    return is_truthy_value(cfg.get("todo_fanout_enabled", False), default=False)


def _append_todo_delegation_requirement(agent, function_args: dict, function_result: Any) -> Any:
    """Attach plan-execution policy and parallel-only delegation guidance.

    Sequential TODOs must run one by one. Only in-progress items that explicitly
    share a non-empty ``parallel_group`` may fan out as a batch.
    """
    if not isinstance(function_result, str):
        return function_result
    setattr(agent, "_todo_delegation_requirement", None)

    try:
        payload = json.loads(function_result)
    except (TypeError, ValueError):
        return function_result
    if not isinstance(payload, dict):
        return function_result

    task_status = payload.get("execution_status")
    if isinstance(task_status, dict):
        try:
            _write_detail(agent, "task.status.updated", {
                "task_status": task_status,
                "source": "todo_tool",
            })
        except Exception:
            pass

    if not _mainline_branch_enabled():
        return function_result
    if str(getattr(agent, "prompt_profile", "full") or "full").strip().lower() == "lean":
        return function_result
    if not isinstance(function_args.get("todos"), list):
        return function_result

    todos = payload.get("todos")
    if not isinstance(todos, list):
        return function_result

    payload["execution_policy"] = {
        "mode": "plan_driven",
        "instruction": (
            "Follow TODO order and dependencies. Complete and verify the current "
            "sequential in_progress item, update it to completed, then promote "
            "the next pending item. Only items sharing the same non-empty "
            "parallel_group may run concurrently. Revise the plan when needed; "
            "if clarify is required, preserve the plan and resume or update it "
            "after the user's answer. Continue until no required active items remain."
        ),
    }

    running = [
        item
        for item in todos
        if isinstance(item, dict)
        and str(item.get("status", "")).strip().lower() == "in_progress"
    ]
    branch_running = [
        item
        for item in running
        if str(item.get("executor", "mainline")).strip().lower() == "branch"
    ]
    delegate_available = "delegate_task" in set(
        getattr(agent, "valid_tool_names", set()) or set()
    )
    requirement = None
    if len(running) == 1 and len(branch_running) == 1 and delegate_available:
        item = branch_running[0]
        requirement = {
            "required": True,
            "mode": "single",
            "reason": "todo_current_item_uses_branch",
            "todo_ids": [item.get("id")],
            "instruction": (
                "The current sequential TODO is assigned to a branch. Call "
                "delegate_task with the suggested single goal, wait for the "
                "result, verify it, then update TODO before any next task."
            ),
            "suggested_delegate_task_args": {
                "goal": str(item.get("content", "")).strip()
                or f"Complete TODO {item.get('id', 'current')}",
                "context": (
                    "This is the current sequential item from the mainline TODO "
                    "plan. Return concrete evidence and anything the parent must verify."
                ),
            },
        }
    elif (
        len(running) >= 2
        and len(branch_running) == len(running)
        and _todo_fanout_enabled()
        and delegate_available
    ):
        groups = {str(item.get("parallel_group", "")).strip() for item in running}
        if len(groups) == 1 and "" not in groups:
            parallel_group = next(iter(groups))
            tasks = [
                {
                    "goal": str(item.get("content", "")).strip()
                    or f"Complete TODO {item.get('id', idx + 1)}",
                    "context": (
                        f"This task is in parallel_group={parallel_group!r} from "
                        "the mainline TODO plan. Return a concise summary with "
                        "concrete evidence, files, commands, URLs, or risks the "
                        "mainline should verify."
                    ),
                }
                for idx, item in enumerate(running)
            ]
            requirement = {
                "required": True,
                "mode": "parallel",
                "reason": "todo_plan_has_explicit_parallel_group",
                "parallel_group": parallel_group,
                "todo_ids": [item.get("id") for item in running],
                "active_todo_count": len(running),
                "instruction": (
                    "These in-progress TODOs explicitly share a parallel_group. "
                    "Call delegate_task with the suggested tasks array, wait for "
                    "every branch, verify results, then update their TODO statuses "
                    "before continuing."
                ),
                "suggested_delegate_task_args": {"tasks": tasks},
            }

    setattr(agent, "_todo_delegation_requirement", requirement)
    if requirement is None:
        return json.dumps(payload, ensure_ascii=False)
    payload["delegation_required"] = requirement

    try:
        _write_detail(agent, "delegation.todo.required", {
            "mode": requirement["mode"],
            "active_todo_count": len(requirement["todo_ids"]),
            "todo_ids": requirement["todo_ids"],
            "parallel_group": requirement.get("parallel_group"),
        })
    except Exception:
        pass
    try:
        print(
            "====delegation todo.required "
            f"mode={requirement['mode']} active_todos={len(requirement['todo_ids'])}"
        )
    except Exception:
        pass
    return json.dumps(payload, ensure_ascii=False)


def _record_delegate_completion_status(agent, function_result: Any) -> Any:
    """Track child completion and expose a normalized delegation snapshot."""
    if not isinstance(function_result, str):
        return function_result
    try:
        payload = json.loads(function_result)
    except (TypeError, ValueError):
        return function_result
    if not isinstance(payload, dict) or not isinstance(payload.get("results"), list):
        return function_result

    results = payload["results"]
    expected_ids = list(getattr(agent, "_todo_update_expected_ids", []) or [])
    if not expected_ids:
        try:
            expected_ids = [
                item["id"]
                for item in agent._todo_store.read()
                if item.get("status") == "in_progress"
                and item.get("executor", "mainline") == "branch"
            ]
        except Exception:
            expected_ids = []
    statuses = [
        str(item.get("status", "unknown")).strip().lower()
        if isinstance(item, dict)
        else "unknown"
        for item in results
    ]
    incomplete_indexes = [
        index for index, status in enumerate(statuses) if status != "completed"
    ]
    if len(statuses) < len(expected_ids):
        incomplete_indexes.extend(range(len(statuses), len(expected_ids)))
    incomplete_indexes = sorted(set(incomplete_indexes))
    incomplete_todo_ids = [
        expected_ids[index]
        for index in incomplete_indexes
        if index < len(expected_ids)
    ]
    snapshot = {
        "state": "completed" if results and not incomplete_indexes else "incomplete",
        "complete": bool(results) and not incomplete_indexes,
        "total": max(len(results), len(expected_ids)),
        "completed": sum(1 for status in statuses if status == "completed"),
        "incomplete": len(incomplete_indexes),
        "statuses": statuses,
        "expected_todo_ids": expected_ids,
        "incomplete_todo_ids": incomplete_todo_ids,
    }
    if expected_ids:
        agent._todo_update_expected_ids = expected_ids
        agent._todo_update_required_after_delegation = True
    agent._todo_delegation_outcome = snapshot
    payload["subtask_execution_status"] = snapshot
    try:
        _write_detail(agent, "task.subtasks.updated", {
            "subtask_status": snapshot,
            "source": "delegate_task",
        })
    except Exception:
        pass
    return json.dumps(payload, ensure_ascii=False)


def execute_tool_calls_concurrent(agent, assistant_message, messages: list, effective_task_id: str, api_call_count: int = 0) -> None:
    """Execute multiple tool calls concurrently using a thread pool.

    Results are collected in the original tool-call order and appended to
    messages so the API sees them in the expected sequence.
    """
    tool_calls = assistant_message.tool_calls
    num_tools = len(tool_calls)

    # ── Pre-flight: interrupt check ──────────────────────────────────
    if agent._interrupt_requested:
        print(f"{agent.log_prefix}⚡ Interrupt: skipping {num_tools} tool call(s)")
        for tc in tool_calls:
            messages.append(make_tool_result_message(
                tc.function.name,
                f"[Tool execution cancelled — {tc.function.name} was skipped due to user interrupt]",
                tc.id,
            ))
        return

    # ── Parse args + pre-execution bookkeeping ───────────────────────
    parsed_calls = []  # list of (tool_call, function_name, function_args)
    for tool_call in tool_calls:
        function_name = tool_call.function.name

        # Reset nudge counters
        if function_name == "memory":
            agent._turns_since_memory = 0
        elif function_name == "skill_manage":
            agent._iters_since_skill = 0

        try:
            function_args = json.loads(tool_call.function.arguments)
        except json.JSONDecodeError:
            function_args = {}
        if not isinstance(function_args, dict):
            function_args = {}

        # Checkpoint for file-mutating tools
        if function_name in {"write_file", "patch"} and agent._checkpoint_mgr.enabled:
            try:
                file_path = function_args.get("path", "")
                if file_path:
                    work_dir = agent._checkpoint_mgr.get_working_dir_for_path(file_path)
                    agent._checkpoint_mgr.ensure_checkpoint(work_dir, f"before {function_name}")
            except Exception:
                pass

        # Checkpoint before destructive terminal commands
        if function_name == "terminal" and agent._checkpoint_mgr.enabled:
            try:
                cmd = function_args.get("command", "")
                if _is_destructive_command(cmd):
                    from gateway.session_context import get_session_env
                    cwd = function_args.get("workdir") or get_session_env("TERMINAL_CWD", os.getcwd())
                    agent._checkpoint_mgr.ensure_checkpoint(
                        cwd, f"before terminal: {cmd[:60]}"
                    )
            except Exception:
                pass

        # execute_code can mutate the project directly in project mode and can
        # invoke write/terminal tools through its RPC bridge. Snapshot the
        # working tree before it starts so an interrupted, partially-applied
        # script can be inspected or rolled back without blind re-execution.
        if function_name == "execute_code" and agent._checkpoint_mgr.enabled:
            try:
                from gateway.session_context import get_session_env
                cwd = get_session_env("TERMINAL_CWD", os.getcwd())
                agent._checkpoint_mgr.ensure_checkpoint(cwd, "before execute_code")
            except Exception:
                pass

        block_result = None
        blocked_by_guardrail = False
        try:
            from kylin_agent_runtime_cli.plugins import get_pre_tool_call_block_message
            block_message = get_pre_tool_call_block_message(
                function_name, function_args, task_id=effective_task_id or "",
            )
        except Exception:
            block_message = None

        if block_message is not None:
            block_result = json.dumps({"error": block_message}, ensure_ascii=False)
        else:
            guardrail_decision = agent._tool_guardrails.before_call(function_name, function_args)
            if not guardrail_decision.allows_execution:
                block_result = agent._guardrail_block_result(guardrail_decision)
                blocked_by_guardrail = True

        parsed_calls.append((tool_call, function_name, function_args, block_result, blocked_by_guardrail))

    # ── Logging / callbacks ──────────────────────────────────────────
    tool_names_str = ", ".join(name for _, name, _, _, _ in parsed_calls)
    _write_detail(agent, "tools.concurrent.batch.started", {
        "task_id": effective_task_id,
        "api_call_count": api_call_count,
        "tool_count": num_tools,
        "tools": [
            {
                "tool_call_id": getattr(tc, "id", None),
                "name": name,
                "arguments": args,
                "blocked": block_result is not None,
                "guardrail_blocked": blocked_by_guardrail,
            }
            for tc, name, args, block_result, blocked_by_guardrail in parsed_calls
        ],
    })
    if not agent.quiet_mode:
        print(f"  ⚡ Concurrent: {num_tools} tool calls — {tool_names_str}")
        for i, (tc, name, args, block_result, blocked_by_guardrail) in enumerate(parsed_calls, 1):
            args_str = json.dumps(args, ensure_ascii=False)
            if agent.verbose_logging:
                print(f"  📞 Tool {i}: {name}({list(args.keys())})")
                print(agent._wrap_verbose("Args: ", json.dumps(args, indent=2, ensure_ascii=False)))
            else:
                args_preview = args_str[:agent.log_prefix_chars] + "..." if len(args_str) > agent.log_prefix_chars else args_str
                print(f"  📞 Tool {i}: {name}({list(args.keys())}) - {args_preview}")

    for tc, name, args, block_result, blocked_by_guardrail in parsed_calls:
        if block_result is not None:
            continue
        if agent.tool_progress_callback:
            try:
                preview = _build_tool_preview(name, args)
                agent.tool_progress_callback("tool.started", name, preview, args)
            except Exception as cb_err:
                logging.debug(f"Tool progress callback error: {cb_err}")

    for tc, name, args, block_result, blocked_by_guardrail in parsed_calls:
        if block_result is not None:
            continue
        if agent.tool_start_callback:
            try:
                agent.tool_start_callback(tc.id, name, args)
            except Exception as cb_err:
                logging.debug(f"Tool start callback error: {cb_err}")

    # ── Concurrent execution ─────────────────────────────────────────
    # Each slot holds (function_name, function_args, function_result, duration, error_flag, blocked_flag)
    results = [None] * num_tools
    for i, (tc, name, args, block_result, blocked_by_guardrail) in enumerate(parsed_calls):
        if block_result is not None:
            results[i] = (name, args, block_result, 0.0, True, True)

    # Touch activity before launching workers so the gateway knows
    # we're executing tools (not stuck).
    agent._current_tool = tool_names_str
    agent._touch_activity(f"executing {num_tools} tools concurrently: {tool_names_str}")

    # Capture CLI callbacks from the agent thread so worker threads can
    # register them locally.  Without this, _get_approval_callback() in
    # terminal_tool returns None in ThreadPoolExecutor workers, causing
    # the dangerous-command prompt to fall back to input() — which
    # deadlocks against prompt_toolkit's raw terminal mode (#13617).
    _parent_approval_cb = _get_approval_callback()
    _parent_sudo_cb = _get_sudo_password_callback()

    def _run_tool(index, tool_call, function_name, function_args):
        """Worker function executed in a thread."""
        # Register this worker tid so the agent can fan out an interrupt
        # to it — see AIAgent.interrupt().  Must happen first thing, and
        # must be paired with discard + clear in the finally block.
        _worker_tid = threading.current_thread().ident
        with agent._tool_worker_threads_lock:
            agent._tool_worker_threads.add(_worker_tid)
        # Race: if the agent was interrupted between fan-out (which
        # snapshotted an empty/earlier set) and our registration, apply
        # the interrupt to our own tid now so is_interrupted() inside
        # the tool returns True on the next poll.
        if agent._interrupt_requested:
            try:
                _ra()._set_interrupt(True, _worker_tid)
            except Exception:
                pass
        # Set the activity callback on THIS worker thread so
        # _wait_for_process (terminal commands) can fire heartbeats.
        # The callback is thread-local; the main thread's callback
        # is invisible to worker threads.
        try:
            from tools.environments.base import set_activity_callback
            set_activity_callback(agent._touch_activity)
        except Exception:
            pass
        # Propagate approval/sudo callbacks to this worker thread.
        # Mirrors cli.py run_agent() pattern (GHSA-qg5c-hvr5-hjgr).
        if _parent_approval_cb is not None:
            try:
                _set_approval_callback(_parent_approval_cb)
            except Exception:
                pass
        if _parent_sudo_cb is not None:
            try:
                _set_sudo_password_callback(_parent_sudo_cb)
            except Exception:
                pass
        start = time.time()
        _write_detail(agent, "tool.execution.started", {
            "task_id": effective_task_id,
            "api_call_count": api_call_count,
            "mode": "concurrent",
            "tool_call_id": getattr(tool_call, "id", None),
            "name": function_name,
            "arguments": function_args,
        })
        try:
            result = agent._invoke_tool(
                function_name,
                function_args,
                effective_task_id,
                tool_call.id,
                messages=messages,
                pre_tool_block_checked=True,
            )
        except Exception as tool_error:
            result = f"Error executing tool '{function_name}': {tool_error}"
            logger.error("_invoke_tool raised for %s: %s", function_name, tool_error, exc_info=True)
        duration = time.time() - start
        is_error, _ = _detect_tool_failure(function_name, result)
        _write_detail(agent, "tool.execution.completed", {
            "task_id": effective_task_id,
            "api_call_count": api_call_count,
            "mode": "concurrent",
            "tool_call_id": getattr(tool_call, "id", None),
            "name": function_name,
            "arguments": function_args,
            "duration_seconds": duration,
            "is_error": is_error,
            "result": result,
            "is_skill_tool": function_name in {"skill_manage", "skill_view", "skills_list"},
        })
        if is_error:
            logger.info("tool %s failed (%.2fs): %s", function_name, duration, result[:200])
        else:
            logger.info("tool %s completed (%.2fs, %d chars)", function_name, duration, len(result))
        results[index] = (function_name, function_args, result, duration, is_error, False)
        # Tear down worker-tid tracking.  Clear any interrupt bit we may
        # have set so the next task scheduled onto this recycled tid
        # starts with a clean slate.
        with agent._tool_worker_threads_lock:
            agent._tool_worker_threads.discard(_worker_tid)
        try:
            _ra()._set_interrupt(False, _worker_tid)
        except Exception:
            pass
        # Clear thread-local callbacks so a recycled worker thread
        # doesn't hold stale references to a disposed CLI instance.
        try:
            _set_approval_callback(None)
            _set_sudo_password_callback(None)
        except Exception:
            pass

    # Start spinner for CLI mode (skip when TUI handles tool progress)
    spinner = None
    if agent._should_emit_quiet_tool_messages() and agent._should_start_quiet_spinner():
        face = random.choice(KawaiiSpinner.get_waiting_faces())
        spinner = KawaiiSpinner(f"{face} ⚡ running {num_tools} tools concurrently", spinner_type='dots', print_fn=agent._print_fn)
        spinner.start()

    try:
        runnable_calls = [
            (i, tc, name, args)
            for i, (tc, name, args, block_result, blocked_by_guardrail) in enumerate(parsed_calls)
            if block_result is None
        ]
        futures = []
        if runnable_calls:
            max_workers = min(len(runnable_calls), _MAX_TOOL_WORKERS)
            with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
                for i, tc, name, args in runnable_calls:
                    # Propagate ContextVars (e.g. _approval_session_key); mirrors asyncio.to_thread.
                    ctx = contextvars.copy_context()
                    f = executor.submit(ctx.run, _run_tool, i, tc, name, args)
                    futures.append(f)

                # Wait for all to complete with periodic heartbeats so the
                # gateway's inactivity monitor doesn't kill us during long
                # concurrent tool batches. Also check for user interrupts
                # so we don't block indefinitely when the user sends /stop
                # or a new message during concurrent tool execution.
                _conc_start = time.time()
                _interrupt_logged = False
                while True:
                    done, not_done = concurrent.futures.wait(
                        futures, timeout=5.0,
                    )
                    if not not_done:
                        break

                    # Check for interrupt — the per-thread interrupt signal
                    # already causes individual tools (terminal, execute_code)
                    # to abort, but tools without interrupt checks (web_search,
                    # read_file) will run to completion. Cancel any futures
                    # that haven't started yet so we don't block on them.
                    if agent._interrupt_requested:
                        if not _interrupt_logged:
                            _interrupt_logged = True
                            agent._vprint(
                                f"{agent.log_prefix}⚡ Interrupt: cancelling "
                                f"{len(not_done)} pending concurrent tool(s)",
                                force=True,
                            )
                        for f in not_done:
                            f.cancel()
                        # Give already-running tools a moment to notice the
                        # per-thread interrupt signal and exit gracefully.
                        concurrent.futures.wait(not_done, timeout=3.0)
                        break

                    _conc_elapsed = int(time.time() - _conc_start)
                    # Heartbeat every ~30s (6 × 5s poll intervals)
                    if _conc_elapsed > 0 and _conc_elapsed % 30 < 6:
                        _still_running = [
                            parsed_calls[futures.index(f)][1]
                            for f in not_done
                            if f in futures
                        ]
                        agent._touch_activity(
                            f"concurrent tools running ({_conc_elapsed}s, "
                            f"{len(not_done)} remaining: {', '.join(_still_running[:3])})"
                        )
    finally:
        if spinner:
            # Build a summary message for the spinner stop
            completed = sum(1 for r in results if r is not None)
            total_dur = sum(r[3] for r in results if r is not None)
            spinner.stop(f"⚡ {completed}/{num_tools} tools completed in {total_dur:.1f}s total")

    # ── Post-execution: display per-tool results ─────────────────────
    for i, (tc, name, args, block_result, blocked_by_guardrail) in enumerate(parsed_calls):
        r = results[i]
        blocked = False
        if r is None:
            # Tool was cancelled (interrupt) or thread didn't return
            if agent._interrupt_requested:
                function_result = f"[Tool execution cancelled — {name} was skipped due to user interrupt]"
            else:
                function_result = f"Error executing tool '{name}': thread did not return a result"
            tool_duration = 0.0
            _write_detail(agent, "tool.execution.missing_result", {
                "task_id": effective_task_id,
                "api_call_count": api_call_count,
                "mode": "concurrent",
                "tool_call_id": getattr(tc, "id", None),
                "name": name,
                "arguments": args,
                "result": function_result,
            })
        else:
            function_name, function_args, function_result, tool_duration, is_error, blocked = r

            if not blocked:
                function_result = agent._append_guardrail_observation(
                    function_name,
                    function_args,
                    function_result,
                    failed=is_error,
                )
                if function_name == "todo":
                    function_result = _append_todo_delegation_requirement(
                        agent, function_args, function_result
                    )

            if is_error:
                _err_text = _multimodal_text_summary(function_result)
                result_preview = _err_text[:200] if len(_err_text) > 200 else _err_text
                logger.warning("Tool %s returned error (%.2fs): %s", function_name, tool_duration, result_preview)

            # Track file-mutation outcome for the turn-end verifier.
            # `blocked` calls never actually ran — don't let a guardrail
            # block count as either a failure or a success.
            if not blocked:
                try:
                    agent._record_file_mutation_result(
                        function_name, function_args, function_result, is_error,
                    )
                except Exception as _ver_err:
                    logging.debug("file-mutation verifier record failed: %s", _ver_err)

            if not blocked and agent.tool_progress_callback:
                try:
                    agent.tool_progress_callback(
                        "tool.completed", function_name, None, None,
                        duration=tool_duration, is_error=is_error,
                    )
                except Exception as cb_err:
                    logging.debug(f"Tool progress callback error: {cb_err}")

            if agent.verbose_logging:
                logging.debug(f"Tool {function_name} completed in {tool_duration:.2f}s")
                logging.debug(f"Tool result ({len(function_result)} chars): {function_result}")

        # Print cute message per tool
        if agent._should_emit_quiet_tool_messages():
            cute_msg = _get_cute_tool_message_impl(name, args, tool_duration, result=function_result)
            agent._safe_print(f"  {cute_msg}")
        elif not agent.quiet_mode:
            _preview_str = _multimodal_text_summary(function_result)
            if agent.verbose_logging:
                print(f"  ✅ Tool {i+1} completed in {tool_duration:.2f}s")
                print(agent._wrap_verbose("Result: ", _preview_str))
            else:
                response_preview = _preview_str[:agent.log_prefix_chars] + "..." if len(_preview_str) > agent.log_prefix_chars else _preview_str
                print(f"  ✅ Tool {i+1} completed in {tool_duration:.2f}s - {response_preview}")

        agent._current_tool = None
        agent._touch_activity(f"tool completed: {name} ({tool_duration:.1f}s)")

        if not blocked and agent.tool_complete_callback:
            try:
                agent.tool_complete_callback(tc.id, name, args, function_result)
            except Exception as cb_err:
                logging.debug(f"Tool complete callback error: {cb_err}")

        function_result = maybe_persist_tool_result(
            content=function_result,
            tool_name=name,
            tool_use_id=tc.id,
            env=get_active_env(effective_task_id),
        ) if not _is_multimodal_tool_result(function_result) else function_result

        subdir_hints = agent._subdirectory_hints.check_tool_call(name, args)
        if subdir_hints:
            if _is_multimodal_tool_result(function_result):
                # Append the hint to the text summary part so the model
                # still sees it; don't touch the image blocks.
                _append_subdir_hint_to_multimodal(function_result, subdir_hints)
            else:
                function_result += subdir_hints

        # Unwrap _multimodal dicts to an OpenAI-style content list so any
        # vision-capable provider receives [{type:text},{type:image_url}]
        # rather than a raw Python dict.  The Anthropic adapter already
        # accepts content lists; vision-capable OpenAI-compatible servers
        # (mlx-vlm, GPT-4o, …) accept image_url in tool messages natively.
        # Text-only servers get a string-safe fallback here so a rejected
        # image tool result never poisons canonical session history.
        # String results pass through unchanged.
        _tool_content = agent._tool_result_content_for_active_model(name, function_result)
        messages.append(make_tool_result_message(name, _tool_content, tc.id))

        # ── Per-tool /steer drain ───────────────────────────────────
        # Same as the sequential path: drain between each collected
        # result so the steer lands as early as possible.
        agent._apply_pending_steer_to_tool_results(messages, 1)

    # ── Per-turn aggregate budget enforcement ─────────────────────────
    num_tools = len(parsed_calls)
    if num_tools > 0:
        turn_tool_msgs = messages[-num_tools:]
        enforce_turn_budget(turn_tool_msgs, env=get_active_env(effective_task_id))

    # ── /steer injection ──────────────────────────────────────────────
    # Append any pending user steer text to the last tool result so the
    # agent sees it on its next iteration. Runs AFTER budget enforcement
    # so the steer marker is never truncated. See steer() for details.
    if num_tools > 0:
        agent._apply_pending_steer_to_tool_results(messages, num_tools)



def execute_tool_calls_sequential(agent, assistant_message, messages: list, effective_task_id: str, api_call_count: int = 0) -> None:
    """Execute tool calls sequentially (original behavior). Used for single calls or interactive tools."""
    for i, tool_call in enumerate(assistant_message.tool_calls, 1):
        # SAFETY: check interrupt BEFORE starting each tool.
        # If the user sent "stop" during a previous tool's execution,
        # do NOT start any more tools -- skip them all immediately.
        if agent._interrupt_requested:
            remaining_calls = assistant_message.tool_calls[i-1:]
            if remaining_calls:
                agent._vprint(f"{agent.log_prefix}⚡ Interrupt: skipping {len(remaining_calls)} tool call(s)", force=True)
            for skipped_tc in remaining_calls:
                skipped_name = skipped_tc.function.name
                skip_msg = {
                    "role": "tool",
                    "name": skipped_name,
                    "content": f"[Tool execution cancelled — {skipped_name} was skipped due to user interrupt]",
                    "tool_call_id": skipped_tc.id,
                }
                messages.append(skip_msg)
            break

        function_name = tool_call.function.name

        try:
            function_args = json.loads(tool_call.function.arguments)
        except json.JSONDecodeError as e:
            logging.warning(f"Unexpected JSON error after validation: {e}")
            function_args = {}
        if not isinstance(function_args, dict):
            function_args = {}

        # Check plugin hooks for a block directive before executing.
        _block_msg: Optional[str] = None
        try:
            from kylin_agent_runtime_cli.plugins import get_pre_tool_call_block_message
            _block_msg = get_pre_tool_call_block_message(
                function_name, function_args, task_id=effective_task_id or "",
            )
        except Exception:
            pass

        _guardrail_block_decision: ToolGuardrailDecision | None = None
        if _block_msg is None:
            guardrail_decision = agent._tool_guardrails.before_call(function_name, function_args)
            if not guardrail_decision.allows_execution:
                _guardrail_block_decision = guardrail_decision

        _execution_blocked = _block_msg is not None or _guardrail_block_decision is not None

        if _execution_blocked:
            # Tool blocked by plugin or guardrail policy — skip counters,
            # callbacks, checkpointing, activity mutation, and real execution.
            pass
        # Reset nudge counters when the relevant tool is actually used
        elif function_name == "memory":
            agent._turns_since_memory = 0
        elif function_name == "skill_manage":
            agent._iters_since_skill = 0

        if not agent.quiet_mode:
            args_str = json.dumps(function_args, ensure_ascii=False)
            if agent.verbose_logging:
                print(f"  📞 Tool {i}: {function_name}({list(function_args.keys())})")
                print(agent._wrap_verbose("Args: ", json.dumps(function_args, indent=2, ensure_ascii=False)))
            else:
                args_preview = args_str[:agent.log_prefix_chars] + "..." if len(args_str) > agent.log_prefix_chars else args_str
                print(f"  📞 Tool {i}: {function_name}({list(function_args.keys())}) - {args_preview}")

        if not _execution_blocked:
            agent._current_tool = function_name
            agent._touch_activity(f"executing tool: {function_name}")

        # Set activity callback for long-running tool execution (terminal
        # commands, etc.) so the gateway's inactivity monitor doesn't kill
        # the agent while a command is running.
        if not _execution_blocked:
            try:
                from tools.environments.base import set_activity_callback
                set_activity_callback(agent._touch_activity)
            except Exception:
                pass

        if not _execution_blocked and agent.tool_progress_callback:
            try:
                preview = _build_tool_preview(function_name, function_args)
                agent.tool_progress_callback("tool.started", function_name, preview, function_args)
            except Exception as cb_err:
                logging.debug(f"Tool progress callback error: {cb_err}")

        if not _execution_blocked and agent.tool_start_callback:
            try:
                agent.tool_start_callback(tool_call.id, function_name, function_args)
            except Exception as cb_err:
                logging.debug(f"Tool start callback error: {cb_err}")
        _write_detail(agent, "tool.execution.started", {
            "task_id": effective_task_id,
            "api_call_count": api_call_count,
            "mode": "sequential",
            "tool_call_id": getattr(tool_call, "id", None),
            "name": function_name,
            "arguments": function_args,
            "blocked": _execution_blocked,
            "block_message": _block_msg,
            "guardrail": _guardrail_block_decision,
            "is_skill_tool": function_name in {"skill_manage", "skill_view", "skills_list"},
        })

        # Checkpoint: snapshot working dir before file-mutating tools
        if not _execution_blocked and function_name in {"write_file", "patch"} and agent._checkpoint_mgr.enabled:
            try:
                file_path = function_args.get("path", "")
                if file_path:
                    work_dir = agent._checkpoint_mgr.get_working_dir_for_path(file_path)
                    agent._checkpoint_mgr.ensure_checkpoint(
                        work_dir, f"before {function_name}"
                    )
            except Exception:
                pass  # never block tool execution

        # Checkpoint before destructive terminal commands
        if not _execution_blocked and function_name == "terminal" and agent._checkpoint_mgr.enabled:
            try:
                cmd = function_args.get("command", "")
                if _is_destructive_command(cmd):
                    from gateway.session_context import get_session_env
                    cwd = function_args.get("workdir") or get_session_env("TERMINAL_CWD", os.getcwd())
                    agent._checkpoint_mgr.ensure_checkpoint(
                        cwd, f"before terminal: {cmd[:60]}"
                    )
            except Exception:
                pass  # never block tool execution

        if not _execution_blocked and function_name == "execute_code" and agent._checkpoint_mgr.enabled:
            try:
                from gateway.session_context import get_session_env
                cwd = get_session_env("TERMINAL_CWD", os.getcwd())
                agent._checkpoint_mgr.ensure_checkpoint(cwd, "before execute_code")
            except Exception:
                pass  # never block tool execution

        tool_start_time = time.time()

        if _block_msg is not None:
            # Tool blocked by plugin policy — return error without executing.
            function_result = json.dumps({"error": _block_msg}, ensure_ascii=False)
            tool_duration = 0.0
        elif _guardrail_block_decision is not None:
            # Tool blocked by tool-loop guardrail — synthesize exactly one
            # tool result for the original tool_call_id without executing.
            function_result = agent._guardrail_block_result(_guardrail_block_decision)
            tool_duration = 0.0
        elif function_name == "todo":
            from tools.todo_tool import todo_tool as _todo_tool
            function_result = _todo_tool(
                todos=function_args.get("todos"),
                merge=function_args.get("merge", False),
                store=agent._todo_store,
            )
            tool_duration = time.time() - tool_start_time
            if agent._should_emit_quiet_tool_messages():
                agent._vprint(f"  {_get_cute_tool_message_impl('todo', function_args, tool_duration, result=function_result)}")
        elif function_name == "memory":
            memory_db = agent._get_memory_db_for_recall()
            if not memory_db:
                function_result = json.dumps({"success": False, "error": "Memory database is not available."})
            else:
                from tools.memory_tool import memory as _memory
                function_result = _memory(
                    query=function_args.get("query", ""),
                    role_filter=function_args.get("role_filter"),
                    limit=function_args.get("limit", 3),
                    session_id=function_args.get("session_id"),
                    around_message_id=function_args.get("around_message_id"),
                    window=function_args.get("window", 5),
                    sort=function_args.get("sort"),
                    db=memory_db,
                    current_session_id=agent.session_id,
                )
            tool_duration = time.time() - tool_start_time
            if agent._should_emit_quiet_tool_messages():
                agent._vprint(f"  {_get_cute_tool_message_impl('memory', function_args, tool_duration, result=function_result)}")
        elif function_name == "clarify":
            from tools.clarify_tool import clarify_tool as _clarify_tool
            function_result = _clarify_tool(
                question=function_args.get("question", ""),
                choices=function_args.get("choices"),
                callback=agent.clarify_callback,
            )
            tool_duration = time.time() - tool_start_time
            if agent._should_emit_quiet_tool_messages():
                agent._vprint(f"  {_get_cute_tool_message_impl('clarify', function_args, tool_duration, result=function_result)}")
        elif function_name == "delegate_task":
            tasks_arg = function_args.get("tasks")
            if tasks_arg and isinstance(tasks_arg, list):
                spinner_label = f"🔀 delegating {len(tasks_arg)} tasks"
            else:
                goal_preview = (function_args.get("goal") or "")[:30]
                spinner_label = f"🔀 {goal_preview}" if goal_preview else "🔀 delegating"
            spinner = None
            if agent._should_emit_quiet_tool_messages() and agent._should_start_quiet_spinner():
                face = random.choice(KawaiiSpinner.get_waiting_faces())
                spinner = KawaiiSpinner(f"{face} {spinner_label}", spinner_type='dots', print_fn=agent._print_fn)
                spinner.start()
            agent._delegate_spinner = spinner
            _delegate_result = None
            _delegate_dispatch_start = time.monotonic()
            _delegate_task_count = len(tasks_arg) if isinstance(tasks_arg, list) else 1
            _delegate_goal_chars = len(function_args.get("goal") or "")
            _delegate_context_chars = len(function_args.get("context") or "")
            _delegate_session_id = getattr(agent, "session_id", None)
            _delegate_diag_payload = {
                "task_count": _delegate_task_count,
                "goal_chars": _delegate_goal_chars,
                "context_chars": _delegate_context_chars,
                "has_tasks_arg": isinstance(tasks_arg, list),
                "requested_toolsets": function_args.get("toolsets"),
                "requested_role": function_args.get("role"),
            }
            try:
                if isinstance(_delegate_session_id, str) and _delegate_session_id:
                    write_session_detail(
                        _delegate_session_id,
                        "delegate.tool_dispatch.start",
                        _delegate_diag_payload,
                    )
            except Exception:
                pass
            try:
                print(
                    "====delegate tool_dispatch.start "
                    f"tasks={_delegate_task_count} goal_chars={_delegate_goal_chars}"
                )
            except Exception:
                pass
            try:
                function_result = agent._dispatch_delegate_task(function_args)
                _delegate_result = function_result
            finally:
                agent._delegate_spinner = None
                tool_duration = time.time() - tool_start_time
                _delegate_dispatch_ms = int((time.monotonic() - _delegate_dispatch_start) * 1000)
                _complete_payload = {
                    **_delegate_diag_payload,
                    "dispatch_ms": _delegate_dispatch_ms,
                    "tool_duration_seconds": round(tool_duration, 3),
                    "result_chars": len(_delegate_result or ""),
                }
                try:
                    if isinstance(_delegate_session_id, str) and _delegate_session_id:
                        write_session_detail(
                            _delegate_session_id,
                            "delegate.tool_dispatch.complete",
                            _complete_payload,
                        )
                except Exception:
                    pass
                try:
                    print(
                        "====delegate tool_dispatch.complete "
                        f"dispatch_ms={_delegate_dispatch_ms} result_chars={len(_delegate_result or '')}"
                    )
                except Exception:
                    pass
                cute_msg = _get_cute_tool_message_impl('delegate_task', function_args, tool_duration, result=_delegate_result)
                if spinner:
                    spinner.stop(cute_msg)
                elif agent._should_emit_quiet_tool_messages():
                    agent._vprint(f"  {cute_msg}")
        elif agent._context_engine_tool_names and function_name in agent._context_engine_tool_names:
            # Context engine tools (lcm_grep, lcm_describe, lcm_expand, etc.)
            spinner = None
            if agent._should_emit_quiet_tool_messages():
                face = random.choice(KawaiiSpinner.get_waiting_faces())
                emoji = _get_tool_emoji(function_name)
                preview = _build_tool_preview(function_name, function_args) or function_name
                spinner = KawaiiSpinner(f"{face} {emoji} {preview}", spinner_type='dots', print_fn=agent._print_fn)
                spinner.start()
            _ce_result = None
            try:
                function_result = agent.context_compressor.handle_tool_call(function_name, function_args, messages=messages)
                _ce_result = function_result
            except Exception as tool_error:
                function_result = json.dumps({"error": f"Context engine tool '{function_name}' failed: {tool_error}"})
                logger.error("context_engine.handle_tool_call raised for %s: %s", function_name, tool_error, exc_info=True)
            finally:
                tool_duration = time.time() - tool_start_time
                cute_msg = _get_cute_tool_message_impl(function_name, function_args, tool_duration, result=_ce_result)
                if spinner:
                    spinner.stop(cute_msg)
                elif agent._should_emit_quiet_tool_messages():
                    agent._vprint(f"  {cute_msg}")
        elif agent._memory_manager and agent._memory_manager.has_tool(function_name):
            # Memory provider tools (hindsight_retain, honcho_search, etc.)
            # These are not in the tool registry — route through MemoryManager.
            spinner = None
            if agent._should_emit_quiet_tool_messages() and agent._should_start_quiet_spinner():
                face = random.choice(KawaiiSpinner.get_waiting_faces())
                emoji = _get_tool_emoji(function_name)
                preview = _build_tool_preview(function_name, function_args) or function_name
                spinner = KawaiiSpinner(f"{face} {emoji} {preview}", spinner_type='dots', print_fn=agent._print_fn)
                spinner.start()
            _mem_result = None
            try:
                function_result = agent._memory_manager.handle_tool_call(function_name, function_args)
                _mem_result = function_result
            except Exception as tool_error:
                function_result = json.dumps({"error": f"Memory tool '{function_name}' failed: {tool_error}"})
                logger.error("memory_manager.handle_tool_call raised for %s: %s", function_name, tool_error, exc_info=True)
            finally:
                tool_duration = time.time() - tool_start_time
                cute_msg = _get_cute_tool_message_impl(function_name, function_args, tool_duration, result=_mem_result)
                if spinner:
                    spinner.stop(cute_msg)
                elif agent._should_emit_quiet_tool_messages():
                    agent._vprint(f"  {cute_msg}")
        elif agent.quiet_mode:
            spinner = None
            if agent._should_emit_quiet_tool_messages() and agent._should_start_quiet_spinner():
                face = random.choice(KawaiiSpinner.get_waiting_faces())
                emoji = _get_tool_emoji(function_name)
                preview = _build_tool_preview(function_name, function_args) or function_name
                spinner = KawaiiSpinner(f"{face} {emoji} {preview}", spinner_type='dots', print_fn=agent._print_fn)
                spinner.start()
            _spinner_result = None
            try:
                function_result = _ra().handle_function_call(
                    function_name, function_args, effective_task_id,
                    tool_call_id=tool_call.id,
                    session_id=agent.session_id or "",
                    enabled_tools=list(agent.valid_tool_names) if agent.valid_tool_names else None,
                    skip_pre_tool_call_hook=True,
                )
                _spinner_result = function_result
            except Exception as tool_error:
                function_result = f"Error executing tool '{function_name}': {tool_error}"
                logger.error("handle_function_call raised for %s: %s", function_name, tool_error, exc_info=True)
            finally:
                tool_duration = time.time() - tool_start_time
                cute_msg = _get_cute_tool_message_impl(function_name, function_args, tool_duration, result=_spinner_result)
                if spinner:
                    spinner.stop(cute_msg)
                elif agent._should_emit_quiet_tool_messages():
                    agent._vprint(f"  {cute_msg}")
        else:
            try:
                function_result = _ra().handle_function_call(
                    function_name, function_args, effective_task_id,
                    tool_call_id=tool_call.id,
                    session_id=agent.session_id or "",
                    enabled_tools=list(agent.valid_tool_names) if agent.valid_tool_names else None,
                    skip_pre_tool_call_hook=True,
                )
            except Exception as tool_error:
                function_result = f"Error executing tool '{function_name}': {tool_error}"
                logger.error("handle_function_call raised for %s: %s", function_name, tool_error, exc_info=True)
            tool_duration = time.time() - tool_start_time

        if isinstance(function_result, str):
            if function_name == "delegate_task":
                function_result = _record_delegate_completion_status(
                    agent, function_result
                )
            result_preview = function_result if agent.verbose_logging else (
                function_result[:200] if len(function_result) > 200 else function_result
            )
            _result_len = len(function_result)
        else:
            # Multimodal dict result (_multimodal=True) — not sliceable as string
            result_preview = function_result
            _result_len = len(str(function_result))

        # Log tool errors to the persistent error log so [error] tags
        # in the UI always have a corresponding detailed entry on disk.
        _is_error_result, _ = _detect_tool_failure(function_name, function_result)
        if not _execution_blocked:
            function_result = agent._append_guardrail_observation(
                function_name,
                function_args,
                function_result,
                failed=_is_error_result,
            )
            if function_name == "todo":
                function_result = _append_todo_delegation_requirement(
                    agent, function_args, function_result
                )
            result_preview = function_result if agent.verbose_logging else (
                function_result[:200] if len(function_result) > 200 else function_result
            )
        if _is_error_result:
            logger.warning("Tool %s returned error (%.2fs): %s", function_name, tool_duration, result_preview)
        else:
            logger.info("tool %s completed (%.2fs, %d chars)", function_name, tool_duration, _result_len)
        _write_detail(agent, "tool.execution.completed", {
            "task_id": effective_task_id,
            "api_call_count": api_call_count,
            "mode": "sequential",
            "tool_call_id": getattr(tool_call, "id", None),
            "name": function_name,
            "arguments": function_args,
            "duration_seconds": tool_duration,
            "is_error": _is_error_result,
            "blocked": _execution_blocked,
            "result": function_result,
            "is_skill_tool": function_name in {"skill_manage", "skill_view", "skills_list"},
        })

        # Track file-mutation outcome for the turn-end verifier.  See
        # the concurrent path for the rationale; both paths must feed
        # the same state so the footer reflects every tool call in the
        # turn, not just the parallel ones.
        if not _execution_blocked:
            try:
                agent._record_file_mutation_result(
                    function_name, function_args, function_result, _is_error_result,
                )
            except Exception as _ver_err:
                logging.debug("file-mutation verifier record failed: %s", _ver_err)

        if not _execution_blocked and agent.tool_progress_callback:
            try:
                agent.tool_progress_callback(
                    "tool.completed", function_name, None, None,
                    duration=tool_duration, is_error=_is_error_result,
                )
            except Exception as cb_err:
                logging.debug(f"Tool progress callback error: {cb_err}")

        agent._current_tool = None
        agent._touch_activity(f"tool completed: {function_name} ({tool_duration:.1f}s)")

        if agent.verbose_logging:
            logging.debug(f"Tool {function_name} completed in {tool_duration:.2f}s")
            _log_result = _multimodal_text_summary(function_result)
            logging.debug(f"Tool result ({len(_log_result)} chars): {_log_result}")

        if not _execution_blocked and agent.tool_complete_callback:
            try:
                agent.tool_complete_callback(tool_call.id, function_name, function_args, function_result)
            except Exception as cb_err:
                logging.debug(f"Tool complete callback error: {cb_err}")

        function_result = maybe_persist_tool_result(
            content=function_result,
            tool_name=function_name,
            tool_use_id=tool_call.id,
            env=get_active_env(effective_task_id),
        ) if not _is_multimodal_tool_result(function_result) else function_result

        # Discover subdirectory context files from tool arguments
        subdir_hints = agent._subdirectory_hints.check_tool_call(function_name, function_args)
        if subdir_hints:
            if _is_multimodal_tool_result(function_result):
                _append_subdir_hint_to_multimodal(function_result, subdir_hints)
            else:
                function_result += subdir_hints

        # Unwrap _multimodal dicts to an OpenAI-style content list
        # (see parallel path for rationale). String results pass through.
        _tool_content = agent._tool_result_content_for_active_model(function_name, function_result)
        messages.append(make_tool_result_message(function_name, _tool_content, tool_call.id))

        # ── Per-tool /steer drain ───────────────────────────────────
        # Drain pending steer BETWEEN individual tool calls so the
        # injection lands as soon as a tool finishes — not after the
        # entire batch.  The model sees it on the next API iteration.
        agent._apply_pending_steer_to_tool_results(messages, 1)

        if not agent.quiet_mode:
            if agent.verbose_logging:
                print(f"  ✅ Tool {i} completed in {tool_duration:.2f}s")
                print(agent._wrap_verbose("Result: ", function_result))
            else:
                _fr_str = function_result if isinstance(function_result, str) else str(function_result)
                response_preview = _fr_str[:agent.log_prefix_chars] + "..." if len(_fr_str) > agent.log_prefix_chars else _fr_str
                print(f"  ✅ Tool {i} completed in {tool_duration:.2f}s - {response_preview}")

        if agent._interrupt_requested and i < len(assistant_message.tool_calls):
            remaining = len(assistant_message.tool_calls) - i
            agent._vprint(f"{agent.log_prefix}⚡ Interrupt: skipping {remaining} remaining tool call(s)", force=True)
            for skipped_tc in assistant_message.tool_calls[i:]:
                skipped_name = skipped_tc.function.name
                messages.append(make_tool_result_message(
                    skipped_name,
                    f"[Tool execution skipped — {skipped_name} was not started. User sent a new message]",
                    skipped_tc.id,
                ))
            break

        if agent.tool_delay > 0 and i < len(assistant_message.tool_calls):
            time.sleep(agent.tool_delay)

    # ── Per-turn aggregate budget enforcement ─────────────────────────
    num_tools_seq = len(assistant_message.tool_calls)
    if num_tools_seq > 0:
        enforce_turn_budget(messages[-num_tools_seq:], env=get_active_env(effective_task_id))

    # ── /steer injection ──────────────────────────────────────────────
    # See _execute_tool_calls_parallel for the rationale. Same hook,
    # applied to sequential execution as well.
    if num_tools_seq > 0:
        agent._apply_pending_steer_to_tool_results(messages, num_tools_seq)




__all__ = [
    "execute_tool_calls_concurrent",
    "execute_tool_calls_sequential",
]
