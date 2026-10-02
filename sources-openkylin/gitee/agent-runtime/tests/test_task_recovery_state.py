import json

from kylin_agent_runtime_state import SessionDB


def _db(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("desktop-session", "desktop")
    return db


def test_task_checkpoint_survives_reopen_and_preserves_interrupted_tool(tmp_path):
    db = _db(tmp_path)
    db.start_task_run(
        "task-1",
        "desktop-session",
        "完成三个子任务",
        model="test-model",
        priority=2,
    )
    db.update_task_checkpoint(
        "task-1",
        current_location="executing tool terminal",
        current_tool="terminal",
        current_tool_call_id="call-2",
        current_tool_args={"command": "make test"},
        todo_snapshot=[
            {"id": "1", "content": "读取配置", "status": "completed"},
            {"id": "2", "content": "运行测试", "status": "in_progress"},
        ],
        completed_step={
            "tool": "read_file",
            "summary": "configuration loaded",
            "completed_at": 10,
        },
    )
    db.close()

    reopened = SessionDB(tmp_path / "state.db")
    assert reopened.mark_running_tasks_interrupted("runtime_restarted") == 1
    task = reopened.get_recoverable_task("desktop-session")
    assert task["id"] == "task-1"
    assert task["status"] == "interrupted"
    assert task["current_location"] == "executing tool terminal"
    assert task["current_tool"] == "terminal"
    assert json.loads(task["current_tool_args"]) == {"command": "make test"}
    assert task["completed_steps"][0]["tool"] == "read_file"
    assert task["todo_snapshot"][1]["status"] == "in_progress"


def test_recovery_claim_is_atomic_and_completion_removes_recovery_offer(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("task-1", "desktop-session", "继续执行")
    assert db.finish_task_run("task-1", "interrupted", reason="client stopped")

    assert db.resume_task_run("task-1")
    assert not db.resume_task_run("task-1")
    assert db.get_recoverable_task("desktop-session") is None

    assert db.finish_task_run("task-1", "completed")
    task = db.get_task_run("task-1")
    assert task["status"] == "completed"
    assert task["completed_at"] is not None
    assert task["recovery_attempts"] == 1


def test_running_task_lookup_only_returns_active_lease(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("task-1", "desktop-session", "执行中")
    assert db.get_running_task("desktop-session")["id"] == "task-1"
    db.finish_task_run("task-1", "interrupted", reason="user_stopped")
    assert db.get_running_task("desktop-session") is None


def test_completed_task_is_not_marked_interrupted_on_next_start(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("done", "desktop-session", "已完成")
    assert db.finish_task_run("done", "completed")
    assert db.mark_running_tasks_interrupted() == 0
    assert db.get_recoverable_task("desktop-session") is None


def test_cancelled_latest_task_suppresses_obsolete_recovery(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("old", "desktop-session", "旧的中断任务")
    assert db.finish_task_run("old", "interrupted", reason="user_stopped")
    db.start_task_run("latest", "desktop-session", "立即发送的新任务")
    assert db.finish_task_run(
        "latest",
        "cancelled",
        reason="queued_message_superseded",
    )

    assert db.get_recoverable_task("desktop-session") is None


def test_later_completed_task_does_not_discard_user_stopped_recovery(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("paused", "desktop-session", "用户主动停止的任务")
    assert db.finish_task_run("paused", "interrupted", reason="user_stopped")
    db.start_task_run("queued", "desktop-session", "正常执行的排队任务")
    assert db.finish_task_run("queued", "completed")

    assert db.get_recoverable_task("desktop-session")["id"] == "paused"


def test_cancelled_status_wins_disconnect_interrupt_race(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("task-1", "desktop-session", "被立即发送替换")
    assert db.finish_task_run(
        "task-1",
        "interrupted",
        reason="SSE client disconnected",
    )
    assert db.finish_task_run(
        "task-1",
        "cancelled",
        reason="queued_message_superseded",
    )
    task = db.get_task_run("task-1")
    assert task["status"] == "cancelled"
    assert db.get_recoverable_task("desktop-session") is None


def test_semantic_checkpoints_are_append_only_and_survive_reopen(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("task-1", "desktop-session", "执行代码并写入文件")
    first = db.add_task_checkpoint(
        "task-1",
        kind="execute_code",
        location="script_started",
        status="started",
        payload={"code_sha256": "abc"},
    )
    second = db.add_task_checkpoint(
        "task-1",
        kind="execute_code",
        location="after_rpc:write_file",
        payload={"call_index": 1, "coordinates": {"path": "/tmp/result.txt"}},
    )
    assert second > first
    db.close()

    reopened = SessionDB(tmp_path / "state.db")
    checkpoints = reopened.list_task_checkpoints("task-1")
    assert [item["location"] for item in checkpoints] == [
        "script_started",
        "after_rpc:write_file",
    ]
    assert checkpoints[-1]["payload"]["coordinates"]["path"] == "/tmp/result.txt"


def test_execution_context_is_merged_and_decoded_after_reopen(tmp_path):
    db = _db(tmp_path)
    db.start_task_run(
        "task-1",
        "desktop-session",
        "继续完整上下文",
        execution_context={
            "active_session_id": "desktop-session",
            "working_directory": "/workspace/old",
        },
    )
    assert db.update_task_execution_context(
        "task-1",
        {
            "working_directory": "/workspace/current",
            "message_count": 9,
        },
    )
    db.close()

    reopened = SessionDB(tmp_path / "state.db")
    context = reopened.get_task_run("task-1")["execution_context"]
    assert context == {
        "active_session_id": "desktop-session",
        "working_directory": "/workspace/current",
        "message_count": 9,
    }


def test_running_subagent_is_interrupted_and_can_resume_from_same_record(tmp_path):
    db = _db(tmp_path)
    db.start_task_run("task-1", "desktop-session", "并行完成两个子任务")
    db.start_task_subrun(
        "sub-1",
        "task-1",
        "生成报告",
        child_session_id="child-session-1",
    )
    db.update_task_subrun_checkpoint(
        "sub-1",
        current_tool="write_file",
        current_tool_call_id="call-3",
        current_tool_args={"path": "/tmp/report.md"},
        current_recovery_policy="verify_effects",
    )

    assert db.finish_task_run("task-1", "interrupted", reason="user_stopped")
    subrun = db.get_latest_task_subrun("task-1", "生成报告")
    assert subrun["status"] == "interrupted"
    assert subrun["child_session_id"] == "child-session-1"
    assert subrun["current_tool"] == "write_file"
    assert subrun["current_recovery_policy"] == "verify_effects"

    assert db.resume_task_run("task-1")
    assert db.resume_task_subrun("sub-1")
    assert not db.resume_task_subrun("sub-1")
    resumed = db.get_latest_task_subrun("task-1", "生成报告")
    assert resumed["status"] == "running"
    assert resumed["recovery_attempts"] == 1
