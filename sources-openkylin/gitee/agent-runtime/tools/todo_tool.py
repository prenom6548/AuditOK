#!/usr/bin/env python3
"""
Todo Tool Module - Planning & Task Management

Provides an in-memory task list the agent uses to decompose complex tasks,
track progress, and maintain focus across long conversations. The state
lives on the AIAgent instance (one per session) and is re-injected into
the conversation after context compression events.

Design:
- Single `todo` tool: provide `todos` param to write, omit to read
- Every call returns the full current list
- No system prompt mutation, no tool response modification
- Behavioral guidance lives entirely in the tool schema description
"""

import json
from typing import Dict, Any, List, Optional


# Valid status values for todo items
VALID_STATUSES = {"pending", "in_progress", "completed", "cancelled"}
VALID_EXECUTORS = {"branch", "mainline"}


class TodoStore:
    """
    In-memory todo list. One instance per AIAgent (one per session).

    Items are ordered -- list position is priority. Each item has:
      - id: unique string identifier (agent-chosen)
      - content: task description
      - status: pending | in_progress | completed | cancelled
      - parallel_group: optional group shared by independent items that may run
        concurrently
      - executor: optional branch | mainline execution owner
    """

    def __init__(self):
        self._items: List[Dict[str, str]] = []

    def write(self, todos: List[Dict[str, Any]], merge: bool = False) -> List[Dict[str, str]]:
        """
        Write todos. Returns the full current list after writing.

        Args:
            todos: list of {id, content, status} dicts
            merge: if False, replace the entire list. If True, update
                   existing items by id and append new ones.
        """
        if not merge:
            # Replace mode: new list entirely
            self._items = [self._validate(t) for t in self._dedupe_by_id(todos)]
        else:
            # Merge mode: update existing items by id, append new ones
            existing = {item["id"]: item for item in self._items}
            for t in self._dedupe_by_id(todos):
                item_id = str(t.get("id", "")).strip()
                if not item_id:
                    continue  # Can't merge without an id

                if item_id in existing:
                    # Update only the fields the LLM actually provided
                    if "content" in t and t["content"]:
                        existing[item_id]["content"] = str(t["content"]).strip()
                    if "status" in t and t["status"]:
                        status = str(t["status"]).strip().lower()
                        if status in VALID_STATUSES:
                            existing[item_id]["status"] = status
                    if "parallel_group" in t:
                        group = str(t.get("parallel_group", "")).strip()
                        if group:
                            existing[item_id]["parallel_group"] = group
                        else:
                            existing[item_id].pop("parallel_group", None)
                    if "executor" in t:
                        executor = str(t.get("executor", "")).strip().lower()
                        if executor in VALID_EXECUTORS:
                            existing[item_id]["executor"] = executor
                        else:
                            existing[item_id].pop("executor", None)
                else:
                    # New item -- validate fully and append to end
                    validated = self._validate(t)
                    existing[validated["id"]] = validated
                    self._items.append(validated)
            # Rebuild _items preserving order for existing items
            seen = set()
            rebuilt = []
            for item in self._items:
                current = existing.get(item["id"], item)
                if current["id"] not in seen:
                    rebuilt.append(current)
                    seen.add(current["id"])
            self._items = rebuilt
        self._normalize_in_progress()
        return self.read()

    def read(self) -> List[Dict[str, str]]:
        """Return a copy of the current list."""
        return [item.copy() for item in self._items]

    def has_items(self) -> bool:
        """Check if there are any items in the list."""
        return bool(self._items)

    def execution_status(self) -> Dict[str, Any]:
        """Return a stable task/subtask completion snapshot.

        ``complete`` means every required subtask reached a terminal state.
        Cancelled items are terminal because they are no longer part of the
        executable plan, but they remain visible in the snapshot.
        """
        items = self.read()
        by_status = {
            status: [item["id"] for item in items if item["status"] == status]
            for status in VALID_STATUSES
        }
        active_ids = [
            item["id"]
            for item in items
            if item["status"] in {"pending", "in_progress"}
        ]
        has_plan = bool(items)
        complete = has_plan and not active_ids
        if not has_plan:
            state = "not_planned"
        elif active_ids:
            state = "in_progress"
        elif by_status["cancelled"]:
            state = "completed_with_cancellations"
        else:
            state = "completed"

        return {
            "state": state,
            "has_plan": has_plan,
            "complete": complete,
            "total": len(items),
            "pending": len(by_status["pending"]),
            "in_progress": len(by_status["in_progress"]),
            "completed": len(by_status["completed"]),
            "cancelled": len(by_status["cancelled"]),
            "active_ids": active_ids,
            "pending_ids": by_status["pending"],
            "in_progress_ids": by_status["in_progress"],
            "completed_ids": by_status["completed"],
            "cancelled_ids": by_status["cancelled"],
        }

    def is_execution_complete(self) -> bool:
        """Return whether a non-empty plan has no unfinished subtasks."""
        return bool(self.execution_status()["complete"])

    def format_for_injection(self) -> Optional[str]:
        """
        Render the todo list for post-compression injection.

        Returns a human-readable string to append to the compressed
        message history, or None if the list is empty.
        """
        if not self._items:
            return None

        # Status markers for compact display
        markers = {
            "completed": "[x]",
            "in_progress": "[>]",
            "pending": "[ ]",
            "cancelled": "[~]",
        }

        # Only inject pending/in_progress items — completed/cancelled ones
        # cause the model to re-do finished work after compression.
        active_items = [
            item for item in self._items
            if item["status"] in {"pending", "in_progress"}
        ]
        if not active_items:
            return None

        lines = ["[Your active task list was preserved across context compression]"]
        for item in active_items:
            marker = markers.get(item["status"], "[?]")
            group = (
                f", parallel_group={item['parallel_group']}"
                if item.get("parallel_group")
                else ""
            )
            executor = f", executor={item.get('executor', 'mainline')}"
            lines.append(
                f"- {marker} {item['id']}. {item['content']} "
                f"({item['status']}{group}{executor})"
            )

        return "\n".join(lines)

    @staticmethod
    def _validate(item: Dict[str, Any]) -> Dict[str, str]:
        """
        Validate and normalize a todo item.

        Ensures required fields exist and status is valid.
        Returns a clean dict with only {id, content, status}.
        """
        item_id = str(item.get("id", "")).strip()
        if not item_id:
            item_id = "?"

        content = str(item.get("content", "")).strip()
        if not content:
            content = "(no description)"

        status = str(item.get("status", "pending")).strip().lower()
        if status not in VALID_STATUSES:
            status = "pending"

        validated = {"id": item_id, "content": content, "status": status}
        parallel_group = str(item.get("parallel_group", "")).strip()
        if parallel_group:
            validated["parallel_group"] = parallel_group
        executor = str(item.get("executor", "")).strip().lower()
        if executor in VALID_EXECUTORS:
            validated["executor"] = executor
        return validated

    def _normalize_in_progress(self) -> None:
        """Keep execution sequential unless all running items share a group."""
        running = [
            item for item in self._items if item["status"] == "in_progress"
        ]
        if not running:
            next_pending = next(
                (
                    item
                    for item in self._items
                    if item["status"] == "pending"
                ),
                None,
            )
            if next_pending is not None:
                next_pending["status"] = "in_progress"
            return
        if len(running) <= 1:
            return
        groups = {item.get("parallel_group", "") for item in running}
        if len(groups) == 1 and "" not in groups:
            return
        for item in running[1:]:
            item["status"] = "pending"

    @staticmethod
    def _dedupe_by_id(todos: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Collapse duplicate ids, keeping the last occurrence in its position."""
        last_index: Dict[str, int] = {}
        for i, item in enumerate(todos):
            item_id = str(item.get("id", "")).strip() or "?"
            last_index[item_id] = i
        return [todos[i] for i in sorted(last_index.values())]


def todo_tool(
    todos: Optional[List[Dict[str, Any]]] = None,
    merge: bool = False,
    store: Optional[TodoStore] = None,
) -> str:
    """
    Single entry point for the todo tool. Reads or writes depending on params.

    Args:
        todos: if provided, write these items. If None, read current list.
        merge: if True, update by id. If False (default), replace entire list.
        store: the TodoStore instance from the AIAgent.

    Returns:
        JSON string with the full current list and summary metadata.
    """
    if store is None:
        return tool_error("TodoStore not initialized")

    if todos is not None:
        items = store.write(todos, merge)
    else:
        items = store.read()

    execution_status = store.execution_status()

    return json.dumps({
        "todos": items,
        "summary": {
            key: execution_status[key]
            for key in ("total", "pending", "in_progress", "completed", "cancelled")
        },
        "execution_status": execution_status,
    }, ensure_ascii=False)


def check_todo_requirements() -> bool:
    """Todo tool has no external requirements -- always available."""
    return True


# =============================================================================
# OpenAI Function-Calling Schema
# =============================================================================
# Behavioral guidance is baked into the description so it's part of the
# static tool schema (cached, never changes mid-conversation).

TODO_SCHEMA = {
    "name": "todo",
    "description": (
        "Manage your task list for the current session. Use for complex tasks "
        "with 3+ steps or when the user provides multiple tasks. "
        "Call with no parameters to read the current list.\n\n"
        "Writing:\n"
        "- Provide 'todos' array to create/update items\n"
        "- merge=false (default): replace the entire list with a fresh plan\n"
        "- merge=true: update existing items by id, add any new ones\n\n"
        "Each item: {id: string, content: string, "
        "status: pending|in_progress|completed|cancelled}\n"
        "List order is priority. Only ONE item may be in_progress at a time "
        "unless 2+ independent items share the same non-empty parallel_group. "
        "Items without a shared parallel_group are sequential and must be "
        "completed one by one in list/dependency order.\n"
        "Use executor=mainline (the default when omitted). Set executor=branch "
        "only when the user explicitly requested delegation, subagents, or a "
        "mainline/branch workflow.\n"
        "Mark items completed immediately when done. If something fails, "
        "cancel it and add a revised item. Adjust the list when discoveries "
        "change the plan. If user confirmation is required, call clarify, "
        "leave the affected item in_progress, then resume or revise the list "
        "after the answer.\n\n"
        "Always returns the full current list."
    ),
    "parameters": {
        "type": "object",
        "properties": {
            "todos": {
                "type": "array",
                "description": "Task items to write. Omit to read current list.",
                "items": {
                    "type": "object",
                    "properties": {
                        "id": {
                            "type": "string",
                            "description": "Unique item identifier"
                        },
                        "content": {
                            "type": "string",
                            "description": "Task description"
                        },
                        "status": {
                            "type": "string",
                            "enum": ["pending", "in_progress", "completed", "cancelled"],
                            "description": "Current status"
                        },
                        "parallel_group": {
                            "type": "string",
                            "description": (
                                "Optional group id. Only independent items with "
                                "the same non-empty group may run concurrently."
                            )
                        },
                        "executor": {
                            "type": "string",
                            "enum": ["branch", "mainline"],
                            "description": (
                                "Execution owner. mainline is the default. Use "
                                "branch only for explicit user-requested delegation."
                            )
                        }
                    },
                    "required": ["id", "content", "status"]
                }
            },
            "merge": {
                "type": "boolean",
                "description": (
                    "true: update existing items by id, add new ones. "
                    "false (default): replace the entire list."
                ),
                "default": False
            }
        },
        "required": []
    }
}


# --- Registry ---
from tools.registry import registry, tool_error

registry.register(
    name="todo",
    toolset="todo",
    schema=TODO_SCHEMA,
    handler=lambda args, **kw: todo_tool(
        todos=args.get("todos"), merge=args.get("merge", False), store=kw.get("store")),
    check_fn=check_todo_requirements,
    emoji="📋",
)
