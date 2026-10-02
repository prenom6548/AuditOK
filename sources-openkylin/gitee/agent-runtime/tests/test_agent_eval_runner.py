import json
import importlib.util
from pathlib import Path

import agent_eval_runner as runner


def test_default_eval_cases_pass():
    cases = runner._load_jsonl(runner.DEFAULT_CASES)

    report = runner.evaluate_cases(cases)

    assert report["summary"]["failed"] == 0
    assert report["summary"]["total"] >= 6
    assert report["summary"]["success_rate"] == 1.0


def test_pip_install_without_tsinghua_index_fails():
    case = {
        "id": "bad-pip",
        "name": "bad pip",
        "trace": [
            {
                "type": "tool_call",
                "name": "terminal",
                "args": {
                    "command": "python -m pip install rich",
                    "env": {"PIP_INDEX_URL": "https://pypi.org/simple"},
                },
            }
        ],
        "criteria": {"require_pip_tsinghua_index": True},
    }

    result = runner.evaluate_case(case)

    assert result.passed is False
    assert "pip install did not use the Tsinghua index" in result.failures


def test_branch_todo_must_delegate_before_substantive_tool():
    case = {
        "id": "bad-branch",
        "name": "bad branch",
        "trace": [
            {
                "type": "tool_call",
                "name": "todo",
                "result": {
                    "todos": [
                        {
                            "id": "inspect",
                            "content": "Inspect",
                            "status": "in_progress",
                            "executor": "branch",
                        }
                    ]
                },
            },
            {
                "type": "tool_call",
                "name": "terminal",
                "args": {"command": "pytest tests"},
            },
        ],
        "criteria": {"require_delegate_after_branch_todo": True},
    }

    result = runner.evaluate_case(case)

    assert result.passed is False
    assert any("next substantive tool" in failure for failure in result.failures)


def test_branch_todo_fails_when_no_delegate_follows():
    case = {
        "id": "stalled-branch",
        "name": "stalled branch",
        "trace": [
            {
                "type": "tool_call",
                "name": "todo",
                "result": {
                    "todos": [
                        {
                            "id": "inspect",
                            "content": "Inspect",
                            "status": "in_progress",
                            "executor": "branch",
                        }
                    ]
                },
            }
        ],
        "criteria": {"require_delegate_after_branch_todo": True},
    }

    result = runner.evaluate_case(case)

    assert result.passed is False
    assert result.failures == [
        "branch TODO entered in_progress but no delegate_task followed"
    ]


def test_incomplete_todo_plan_fails_completion_gate():
    case = {
        "id": "bad-todo",
        "name": "bad todo",
        "trace": [
            {
                "type": "tool_call",
                "name": "todo",
                "result": {
                    "todos": [
                        {"id": "one", "content": "Do work", "status": "in_progress"}
                    ]
                },
            },
            {"type": "final_response", "content": "done"},
        ],
        "criteria": {"require_todo_complete": True},
    }

    result = runner.evaluate_case(case)

    assert result.passed is False
    assert result.failures == ["TODO plan still has active items: one"]


def test_json_report_is_serializable():
    cases = [
        {
            "id": "ok",
            "name": "ok",
            "trace": [{"type": "tool_call", "name": "read_file"}],
            "criteria": {"required_tools": ["read_file"]},
        }
    ]

    report = runner.evaluate_cases(cases)

    assert json.loads(json.dumps(report))["summary"]["passed"] == 1


def test_load_real_trajectory_cases_extracts_tool_calls(tmp_path):
    trajectory_file = tmp_path / "trajectory_samples.jsonl"
    trajectory_file.write_text(
        json.dumps(
            {
                "timestamp": "2026-08-25T00:00:00",
                "model": "test-model",
                "completed": True,
                "conversations": [
                    {"from": "system", "value": "tools"},
                    {"from": "human", "value": "安装 rich"},
                    {
                        "from": "gpt",
                        "value": (
                            "<think>\n</think>\n"
                            "<tool_call>\n"
                            '{"name":"terminal","arguments":'
                            '{"command":"python -m pip install rich",'
                            '"env":{"PIP_INDEX_URL":"https://pypi.tuna.tsinghua.edu.cn/simple"}}}'
                            "\n</tool_call>"
                        ),
                    },
                    {
                        "from": "tool",
                        "value": (
                            "<tool_response>\n"
                            '{"tool_call_id":"call_1","name":"terminal",'
                            '"content":{"ok":true,"exit_code":0}}'
                            "\n</tool_response>"
                        ),
                    },
                    {"from": "gpt", "value": "<think>\n</think>\n安装完成"},
                ],
            },
            ensure_ascii=False,
        )
        + "\n",
        encoding="utf-8",
    )

    cases = runner.load_trajectory_cases(
        trajectory_file,
        criteria={"require_pip_tsinghua_index_if_used": True},
    )
    report = runner.evaluate_cases(cases)

    assert len(cases) == 1
    assert cases[0]["task"] == "安装 rich"
    assert cases[0]["trace"][0]["name"] == "terminal"
    assert cases[0]["trace"][0]["result"] == {"ok": True, "exit_code": 0}
    assert report["summary"]["failed"] == 0


def test_raw_event_trace_case_can_be_evaluated(tmp_path):
    event_file = tmp_path / "agent_events.jsonl"
    event_file.write_text(
        json.dumps(
            {
                "type": "tool_call",
                "name": "terminal",
                "args": {
                    "command": "python -m pip install rich",
                    "env": {"PIP_INDEX_URL": "https://pypi.org/simple"},
                },
            }
        )
        + "\n",
        encoding="utf-8",
    )

    case = runner.load_event_trace_case(
        event_file,
        criteria={"require_pip_tsinghua_index_if_used": True},
    )
    result = runner.evaluate_case(case)

    assert result.passed is False
    assert result.failures == ["pip install did not use the Tsinghua index"]


def test_eval_recorder_plugin_is_opt_in(tmp_path, monkeypatch):
    plugin_path = (
        Path(__file__).parents[1] / "plugins" / "agent-eval-recorder" / "__init__.py"
    )
    spec = importlib.util.spec_from_file_location("agent_eval_recorder_test", plugin_path)
    module = importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    spec.loader.exec_module(module)

    trace_file = tmp_path / "agent_events.jsonl"
    monkeypatch.setenv("HERMES_AGENT_EVAL_TRACE_FILE", str(trace_file))
    monkeypatch.delenv("HERMES_AGENT_EVAL_RECORD", raising=False)

    module._on_post_tool_call(tool_name="terminal", args={"command": "echo hi"})
    assert not trace_file.exists()

    monkeypatch.setenv("HERMES_AGENT_EVAL_RECORD", "1")
    module._on_post_tool_call(
        tool_name="terminal",
        args={"command": "echo hi"},
        result='{"ok": true}',
        task_id="task-1",
        duration_ms=7,
    )

    payload = json.loads(trace_file.read_text(encoding="utf-8"))
    assert payload["type"] == "tool_call"
    assert payload["name"] == "terminal"
    assert payload["result"] == {"ok": True}
    assert payload["duration_ms"] == 7
