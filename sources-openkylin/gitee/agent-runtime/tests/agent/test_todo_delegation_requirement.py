import json
import sys
import types
from types import SimpleNamespace

from agent.tool_executor import (
    _append_todo_delegation_requirement,
    _mainline_branch_enabled,
    _record_delegate_completion_status,
)


def _agent(valid_tool_names=None, prompt_profile="full"):
    return SimpleNamespace(
        valid_tool_names=set(valid_tool_names or {"todo", "delegate_task"}),
        prompt_profile=prompt_profile,
        session_id="test-session",
    )


def test_delegate_completion_status_tracks_each_expected_subtask(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    agent = _agent()
    agent._todo_update_expected_ids = ["build", "verify"]

    updated = json.loads(
        _record_delegate_completion_status(
            agent,
            json.dumps(
                {
                    "results": [
                        {"status": "completed"},
                        {"status": "partial"},
                    ]
                }
            ),
        )
    )

    status = updated["subtask_execution_status"]
    assert status["complete"] is False
    assert status["statuses"] == ["completed", "partial"]
    assert status["incomplete_todo_ids"] == ["verify"]
    assert agent._todo_delegation_outcome == status


def test_delegate_completion_status_requires_every_expected_result(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    agent = _agent()
    agent._todo_update_expected_ids = ["one"]

    updated = json.loads(
        _record_delegate_completion_status(agent, '{"results": []}')
    )

    status = updated["subtask_execution_status"]
    assert status["complete"] is False
    assert status["incomplete_todo_ids"] == ["one"]


def test_sequential_branch_todo_requires_single_delegate(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    monkeypatch.setattr(
        "agent.tool_executor._mainline_branch_enabled",
        lambda: True,
    )
    monkeypatch.setattr(
        "agent.tool_executor._todo_fanout_enabled",
        lambda: True,
    )
    result = json.dumps(
        {
            "todos": [
                {
                    "id": "1",
                    "content": "Inspect code path",
                    "status": "in_progress",
                    "executor": "branch",
                },
                {"id": "2", "content": "Run focused tests", "status": "pending"},
            ],
            "summary": {"total": 2, "pending": 1, "in_progress": 1},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    requirement = updated["delegation_required"]
    assert requirement["mode"] == "single"
    assert requirement["todo_ids"] == ["1"]
    assert requirement["suggested_delegate_task_args"]["goal"] == "Inspect code path"
    assert updated["execution_policy"]["mode"] == "plan_driven"
    assert "Complete and verify" in updated["execution_policy"]["instruction"]


def test_explicit_parallel_group_requires_delegate_task(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    monkeypatch.setattr(
        "agent.tool_executor._mainline_branch_enabled",
        lambda: True,
    )
    monkeypatch.setattr(
        "agent.tool_executor._todo_fanout_enabled",
        lambda: True,
    )
    result = json.dumps(
        {
            "todos": [
                    {
                        "id": "1",
                        "content": "Inspect backend",
                        "status": "in_progress",
                        "parallel_group": "review",
                        "executor": "branch",
                    },
                {
                        "id": "2",
                        "content": "Inspect frontend",
                        "status": "in_progress",
                        "parallel_group": "review",
                        "executor": "branch",
                },
                {"id": "3", "content": "Integrate findings", "status": "pending"},
            ],
            "summary": {"total": 3, "pending": 1, "in_progress": 2},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}, {"id": "3"}]},
            result,
        )
    )

    requirement = updated["delegation_required"]
    assert requirement["required"] is True
    assert requirement["parallel_group"] == "review"
    assert requirement["active_todo_count"] == 2
    assert len(requirement["suggested_delegate_task_args"]["tasks"]) == 2


def test_single_active_todo_does_not_require_delegation(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    result = json.dumps(
        {
            "todos": [
                {"id": "1", "content": "Inspect code path", "status": "pending"},
                {"id": "2", "content": "Already done", "status": "completed"},
            ],
            "summary": {"total": 2, "pending": 1, "completed": 1},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    assert "delegation_required" not in updated


def test_lean_agent_does_not_get_todo_delegation_requirement(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    monkeypatch.setattr("agent.tool_executor._todo_fanout_enabled", lambda: True)
    result = json.dumps(
        {
            "todos": [
                {"id": "1", "content": "A", "status": "pending"},
                {"id": "2", "content": "B", "status": "pending"},
            ],
            "summary": {"total": 2, "pending": 2},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(prompt_profile="lean"),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    assert "delegation_required" not in updated


def test_mainline_branch_disabled_does_not_require_delegation(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    monkeypatch.setattr("agent.tool_executor._todo_fanout_enabled", lambda: True)
    monkeypatch.setattr(
        "agent.tool_executor._mainline_branch_enabled",
        lambda: False,
    )
    result = json.dumps(
        {
            "todos": [
                {"id": "1", "content": "A", "status": "pending"},
                {"id": "2", "content": "B", "status": "pending"},
            ],
            "summary": {"total": 2, "pending": 2},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    assert "delegation_required" not in updated


def test_sequential_branch_delegation_is_independent_of_parallel_fanout_toggle(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    monkeypatch.setattr(
        "agent.tool_executor._mainline_branch_enabled",
        lambda: True,
    )
    monkeypatch.setattr("agent.tool_executor._todo_fanout_enabled", lambda: False)
    result = json.dumps(
        {
            "todos": [
                {
                    "id": "1",
                    "content": "Inspect code path",
                    "status": "in_progress",
                    "executor": "branch",
                },
                {"id": "2", "content": "Run focused tests", "status": "pending"},
            ],
            "summary": {"total": 2, "pending": 1, "in_progress": 1},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    assert updated["delegation_required"]["mode"] == "single"


def test_mainline_owned_todo_does_not_delegate(monkeypatch):
    monkeypatch.setattr("agent.tool_executor.write_session_detail", lambda *a, **k: None)
    result = json.dumps(
        {
            "todos": [
                {
                    "id": "1",
                    "content": "Send the approved message",
                    "status": "in_progress",
                    "executor": "mainline",
                },
                {"id": "2", "content": "Verify delivery", "status": "pending"},
            ],
            "summary": {"total": 2, "pending": 1, "in_progress": 1},
        }
    )

    updated = json.loads(
        _append_todo_delegation_requirement(
            _agent(),
            {"todos": [{"id": "1"}, {"id": "2"}]},
            result,
        )
    )

    assert "delegation_required" not in updated


def test_mainline_branch_prefers_current_config_over_stale_cli_config(monkeypatch):
    fake_config = types.ModuleType("kylin_agent_runtime_cli.config")
    fake_config.load_config_readonly = lambda: {
        "delegation": {"enabled": True, "mainline_branch_enabled": False}
    }
    fake_cli = types.ModuleType("cli")
    fake_cli.CLI_CONFIG = {"delegation": {"enabled": True, "mainline_branch_enabled": True}}

    monkeypatch.setitem(sys.modules, "kylin_agent_runtime_cli.config", fake_config)
    monkeypatch.setitem(sys.modules, "cli", fake_cli)

    assert _mainline_branch_enabled() is False
