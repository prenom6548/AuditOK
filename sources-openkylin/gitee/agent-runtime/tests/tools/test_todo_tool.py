"""Tests for the todo tool module."""

import json

from tools.todo_tool import TODO_SCHEMA, TodoStore, todo_tool


def test_schema_defaults_execution_to_mainline():
    assert "mainline (the default when omitted)" in TODO_SCHEMA["description"]
    executor = (
        TODO_SCHEMA["parameters"]["properties"]["todos"]["items"]
        ["properties"]["executor"]["description"]
    )
    assert "mainline is the default" in executor


class TestWriteAndRead:
    def test_write_replaces_list(self):
        store = TodoStore()
        items = [
            {"id": "1", "content": "First task", "status": "pending"},
            {"id": "2", "content": "Second task", "status": "in_progress"},
        ]
        result = store.write(items)
        assert len(result) == 2
        assert result[0]["id"] == "1"
        assert result[1]["status"] == "in_progress"

    def test_read_returns_copy(self):
        store = TodoStore()
        store.write([{"id": "1", "content": "Task", "status": "pending"}])
        items = store.read()
        items[0]["content"] = "MUTATED"
        assert store.read()[0]["content"] == "Task"

    def test_write_deduplicates_duplicate_ids(self):
        store = TodoStore()
        result = store.write([
            {"id": "1", "content": "First version", "status": "pending"},
            {"id": "2", "content": "Other task", "status": "pending"},
            {"id": "1", "content": "Latest version", "status": "in_progress"},
        ])
        assert result == [
            {"id": "2", "content": "Other task", "status": "pending"},
            {"id": "1", "content": "Latest version", "status": "in_progress"},
        ]

    def test_multiple_sequential_in_progress_items_are_normalized(self):
        store = TodoStore()
        result = store.write([
            {"id": "1", "content": "First", "status": "in_progress"},
            {"id": "2", "content": "Second", "status": "in_progress"},
        ])
        assert [item["status"] for item in result] == ["in_progress", "pending"]

    def test_shared_parallel_group_allows_concurrent_items(self):
        store = TodoStore()
        result = store.write([
            {
                "id": "1",
                "content": "Backend review",
                "status": "in_progress",
                "parallel_group": "review",
            },
            {
                "id": "2",
                "content": "Frontend review",
                "status": "in_progress",
                "parallel_group": "review",
            },
        ])
        assert [item["status"] for item in result] == ["in_progress", "in_progress"]
        assert all(item["parallel_group"] == "review" for item in result)

    def test_executor_is_preserved(self):
        store = TodoStore()
        result = store.write([
            {
                "id": "1",
                "content": "Send message",
                "status": "pending",
                "executor": "mainline",
            }
        ])
        assert result[0]["executor"] == "mainline"
        assert result[0]["status"] == "in_progress"


class TestHasItems:
    def test_empty_store(self):
        store = TodoStore()
        assert store.has_items() is False

    def test_non_empty_store(self):
        store = TodoStore()
        store.write([{"id": "1", "content": "x", "status": "pending"}])
        assert store.has_items() is True


class TestExecutionStatus:
    def test_empty_store_is_not_planned(self):
        status = TodoStore().execution_status()
        assert status["state"] == "not_planned"
        assert status["has_plan"] is False
        assert status["complete"] is False

    def test_active_subtasks_are_reported_by_id(self):
        store = TodoStore()
        store.write([
            {"id": "build", "content": "Build", "status": "in_progress"},
            {"id": "verify", "content": "Verify", "status": "pending"},
        ])
        status = store.execution_status()
        assert status["state"] == "in_progress"
        assert status["complete"] is False
        assert status["active_ids"] == ["build", "verify"]
        assert status["pending_ids"] == ["verify"]
        assert status["in_progress_ids"] == ["build"]

    def test_all_terminal_subtasks_complete_the_plan(self):
        store = TodoStore()
        store.write([
            {"id": "done", "content": "Done", "status": "completed"},
            {"id": "obsolete", "content": "Obsolete", "status": "cancelled"},
        ])
        status = store.execution_status()
        assert status["state"] == "completed_with_cancellations"
        assert status["complete"] is True
        assert store.is_execution_complete() is True


class TestFormatForInjection:
    def test_empty_returns_none(self):
        store = TodoStore()
        assert store.format_for_injection() is None

    def test_non_empty_has_markers(self):
        store = TodoStore()
        store.write([
            {"id": "1", "content": "Do thing", "status": "completed"},
            {"id": "2", "content": "Next", "status": "pending"},
            {"id": "3", "content": "Working", "status": "in_progress"},
        ])
        text = store.format_for_injection()
        # Completed items are filtered out of injection
        assert "[x]" not in text
        assert "Do thing" not in text
        # Active items are included
        assert "[ ]" in text
        assert "[>]" in text
        assert "Next" in text
        assert "Working" in text
        assert "context compression" in text.lower()


class TestMergeMode:
    def test_update_existing_by_id(self):
        store = TodoStore()
        store.write([
            {"id": "1", "content": "Original", "status": "pending"},
        ])
        store.write(
            [{"id": "1", "status": "completed"}],
            merge=True,
        )
        items = store.read()
        assert len(items) == 1
        assert items[0]["status"] == "completed"
        assert items[0]["content"] == "Original"

    def test_merge_appends_new(self):
        store = TodoStore()
        store.write([{"id": "1", "content": "First", "status": "pending"}])
        store.write(
            [{"id": "2", "content": "Second", "status": "pending"}],
            merge=True,
        )
        items = store.read()
        assert len(items) == 2


class TestTodoToolFunction:
    def test_read_mode(self):
        store = TodoStore()
        store.write([{"id": "1", "content": "Task", "status": "pending"}])
        result = json.loads(todo_tool(store=store))
        assert result["summary"]["total"] == 1
        assert result["summary"]["in_progress"] == 1

    def test_write_mode(self):
        store = TodoStore()
        result = json.loads(todo_tool(
            todos=[{"id": "1", "content": "New", "status": "in_progress"}],
            store=store,
        ))
        assert result["summary"]["in_progress"] == 1
        assert result["execution_status"]["complete"] is False
        assert result["execution_status"]["active_ids"] == ["1"]

    def test_no_store_returns_error(self):
        result = json.loads(todo_tool())
        assert "error" in result
