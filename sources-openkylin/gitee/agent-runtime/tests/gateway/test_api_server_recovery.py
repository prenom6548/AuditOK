import json
from unittest.mock import AsyncMock, MagicMock

import pytest

from gateway.config import PlatformConfig
from gateway.platforms.api_server import APIServerAdapter
from kylin_agent_runtime_state import SessionDB


def _adapter_with_interrupted_task(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-1", "desktop", model="test-model")
    db.update_system_prompt("desktop-1", "persisted system context")
    db.append_message("desktop-1", "user", "完成原始任务")
    db.append_message(
        "desktop-1",
        "assistant",
        "",
        tool_calls=[
            {
                "id": "dangling-call",
                "type": "function",
                "function": {
                    "name": "write_file",
                    "arguments": '{"path":"/tmp/result.txt"}',
                },
            }
        ],
    )
    db.start_task_run(
        "task-1",
        "desktop-1",
        "完成原始任务",
        model="test-model",
        priority=1,
        execution_context={
            "active_session_id": "desktop-1",
            "working_directory": "/workspace/project",
            "gateway_session_key": "desktop-memory-scope",
        },
    )
    db.update_task_checkpoint(
        "task-1",
        current_location="executing tool terminal",
        current_tool="terminal",
        current_tool_call_id="call-1",
        current_tool_args={"command": "make test"},
        todo_snapshot=[
            {"id": "1", "content": "读取配置", "status": "completed"},
            {"id": "2", "content": "运行测试", "status": "in_progress"},
        ],
        completed_step={"tool": "read_file", "summary": "已读取配置"},
    )
    db.add_task_checkpoint(
        "task-1",
        kind="execute_code",
        location="after_rpc:read_file",
        payload={"call_index": 1},
    )
    db.start_task_subrun(
        "sub-1",
        "task-1",
        "运行专项测试",
        child_session_id="child-1",
    )
    db.finish_task_run("task-1", "interrupted", reason="power loss")
    adapter = APIServerAdapter(PlatformConfig(enabled=True))
    adapter._session_db = db
    return adapter, db


@pytest.mark.asyncio
async def test_recovery_metadata_endpoint_reports_precise_checkpoint(tmp_path):
    adapter, _db = _adapter_with_interrupted_task(tmp_path)
    request = MagicMock()
    request.headers = {}
    request.match_info = {"session_id": "desktop-1"}
    request.json = AsyncMock(return_value={})

    response = await adapter._handle_get_session_recovery(request)
    payload = json.loads(response.text)

    assert response.status == 200
    assert payload["recoverable"] is True
    assert payload["task_id"] == "task-1"
    assert payload["current_tool"] == "terminal"
    assert json.loads(payload["current_tool_args"]) == {"command": "make test"}
    assert payload["completed_step_count"] == 1
    assert payload["unfinished_subtasks"][0]["content"] == "运行测试"
    assert payload["completed_subtasks"][0]["content"] == "读取配置"
    assert payload["last_checkpoint"]["location"] == "after_rpc:read_file"
    assert payload["subagent_runs"][0]["goal"] == "运行专项测试"


@pytest.mark.asyncio
async def test_chat_recovery_claims_same_run_and_uses_checkpoint_prompt(tmp_path):
    adapter, db = _adapter_with_interrupted_task(tmp_path)
    adapter._run_agent = AsyncMock(
        return_value=(
            {"final_response": "任务已继续完成", "completed": True, "messages": []},
            {"input_tokens": 1, "output_tokens": 1, "total_tokens": 2},
        )
    )
    request = MagicMock()
    request.headers = {
        "X-Hermes-Session-Id": "desktop-1",
        "X-Hermes-Recover-Task": "task-1",
    }
    request.json = AsyncMock(
        return_value={
            "model": "test-model",
            "stream": False,
            "messages": [{"role": "user", "content": "继续上次中断的任务"}],
        }
    )

    response = await adapter._handle_chat_completions(request)
    payload = json.loads(response.text)
    call = adapter._run_agent.await_args.kwargs

    assert response.status == 200
    assert payload["choices"][0]["message"]["content"] == "任务已继续完成"
    assert call["user_message"].startswith("Continue the interrupted task")
    assert "executing tool terminal" in call["user_message"]
    assert "make test" in call["user_message"]
    assert "after_rpc:read_file" in call["user_message"]
    assert "运行专项测试" in call["user_message"]
    assert "/workspace/project" in call["user_message"]
    assert call["persist_user_message"] == "[[kylin-agent-task-recovery]]继续上次中断的任务"
    assert call["task_run_id"] == "task-1"
    assert call["gateway_session_key"] == "desktop-memory-scope"
    assert call["recovery_state"]["cached_system_prompt"] == "persisted system context"
    assert call["recovery_state"]["todo_snapshot"][1]["status"] == "in_progress"
    synthetic = [
        item
        for item in call["conversation_history"]
        if item.get("role") == "tool"
        and item.get("tool_call_id") == "dangling-call"
    ]
    assert len(synthetic) == 1
    assert json.loads(synthetic[0]["content"])["status"] == "interrupted"
    assert db.get_task_run("task-1")["status"] == "completed"


@pytest.mark.asyncio
async def test_explicit_session_interrupt_immediately_creates_recovery(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-1", "desktop")
    db.start_task_run("task-running", "desktop-1", "尚未完成的任务")
    adapter = APIServerAdapter(PlatformConfig(enabled=True))
    adapter._session_db = db
    active_agent = MagicMock()
    adapter._active_chat_agents["desktop-1"] = active_agent

    request = MagicMock()
    request.headers = {}
    request.match_info = {"session_id": "desktop-1"}

    response = await adapter._handle_interrupt_session_task(request)
    payload = json.loads(response.text)

    assert response.status == 200
    assert payload == {
        "session_id": "desktop-1",
        "interrupted": True,
        "recoverable": True,
        "task_id": "task-running",
    }
    active_agent.interrupt.assert_called_once_with("Desktop stop button requested")
    assert db.get_task_run("task-running")["status"] == "interrupted"


@pytest.mark.asyncio
async def test_queued_send_interrupt_cancels_without_recovery(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-1", "desktop")
    db.start_task_run("task-running", "desktop-1", "将被排队消息替换")
    adapter = APIServerAdapter(PlatformConfig(enabled=True))
    adapter._session_db = db
    active_agent = MagicMock()
    adapter._active_chat_agents["desktop-1"] = active_agent

    request = MagicMock()
    request.headers = {}
    request.match_info = {"session_id": "desktop-1"}
    request.json = AsyncMock(return_value={
        "recoverable": False,
        "reason": "queued_message_superseded",
    })

    response = await adapter._handle_interrupt_session_task(request)
    payload = json.loads(response.text)

    assert response.status == 200
    assert payload["interrupted"] is True
    assert payload["recoverable"] is False
    assert payload["task_id"] is None
    active_agent.interrupt.assert_called_once_with(
        "Queued message superseded the active task"
    )
    task = db.get_task_run("task-running")
    assert task["status"] == "cancelled"
    assert task["interruption_reason"] == "queued_message_superseded"
