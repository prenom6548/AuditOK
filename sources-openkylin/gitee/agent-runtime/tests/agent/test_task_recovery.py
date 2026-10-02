import json

from agent.task_recovery import (
    prepare_recovery_history,
    record_task_checkpoint,
    recovery_policy_for_tool,
    task_checkpoint_context,
)
from kylin_agent_runtime_state import SessionDB
from tools.code_execution_tool import execute_code


def test_recovery_policy_distinguishes_read_child_and_external_effects():
    assert recovery_policy_for_tool("read_file") == "safe_retry"
    assert recovery_policy_for_tool("delegate_task") == "resume_children"
    assert recovery_policy_for_tool("write_file") == "verify_effects"
    assert recovery_policy_for_tool("send_email") == "confirm_before_retry"


def test_prepare_recovery_history_keeps_context_and_closes_dangling_calls():
    history = [
        {"role": "user", "content": "生成文件"},
        {
            "role": "assistant",
            "content": "",
            "tool_calls": [
                {
                    "id": "done-call",
                    "function": {"name": "read_file", "arguments": "{}"},
                },
                {
                    "id": "lost-call",
                    "function": {"name": "write_file", "arguments": "{}"},
                },
            ],
        },
        {
            "role": "tool",
            "tool_call_id": "done-call",
            "content": '{"content":"ok"}',
        },
    ]

    repaired, synthesized = prepare_recovery_history(history)

    assert repaired[:3] == history
    assert repaired[-1]["tool_call_id"] == "lost-call"
    assert json.loads(repaired[-1]["content"])["result_available"] is False
    assert synthesized == [{"tool_call_id": "lost-call", "tool": "write_file"}]


def test_thread_task_id_registry_records_subagent_checkpoint(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-1", "desktop")
    db.start_task_run("task-1", "desktop-1", "执行子任务")
    db.start_task_subrun("sub-1", "task-1", "写文件")

    with task_checkpoint_context(
        db,
        "task-1",
        "sub-1",
        task_id="child-task-1",
    ):
        assert record_task_checkpoint(
            "execute_code",
            "after_rpc:write_file",
            task_id="child-task-1",
            payload={"path": "/tmp/a.txt"},
        )

    assert not record_task_checkpoint(
        "execute_code",
        "after_rpc:write_file",
        task_id="child-task-1",
    )
    checkpoint = db.list_task_checkpoints("task-1")[0]
    assert checkpoint["subrun_id"] == "sub-1"
    assert checkpoint["payload"]["path"] == "/tmp/a.txt"


def test_execute_code_records_script_safe_points(tmp_path, monkeypatch):
    monkeypatch.setenv("TERMINAL_ENV", "local")
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-1", "desktop")
    db.start_task_run("task-1", "desktop-1", "执行一段代码")

    with task_checkpoint_context(
        db,
        "task-1",
        task_id="desktop-1",
    ):
        execute_code("print('done')", task_id="desktop-1", enabled_tools=[])

    checkpoints = db.list_task_checkpoints("task-1")
    locations = [item["location"] for item in checkpoints]
    assert locations[0] == "script_started"
    assert locations[-1] == "script_finished"
    assert checkpoints[-1]["payload"]["backend"] == "local"
