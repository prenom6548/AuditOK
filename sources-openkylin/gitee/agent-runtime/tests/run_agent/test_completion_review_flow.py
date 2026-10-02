"""Exercise the real conversation loop with an independent mocked reviewer."""
import json
from unittest.mock import patch

import pytest

from agent.completion_review import ReviewSettings
from tests.run_agent.test_run_agent import agent, _mock_response, _mock_tool_call
from tests.agent.test_completion_review import verdict


@pytest.fixture
def reviewed_agent(agent, monkeypatch):
    monkeypatch.setattr("agent.completion_review.load_review_settings", lambda: ReviewSettings())
    agent._cached_system_prompt = "Complete the user's task."
    agent._use_prompt_caching = False
    agent.tool_delay = 0
    agent.compression_enabled = False
    agent.save_trajectories = False
    with (patch.object(agent, "_persist_session"), patch.object(agent, "_save_trajectory"),
          patch.object(agent, "_cleanup_task_resources")):
        yield agent


def test_qwen_mid_task_stop_continues_with_concrete_feedback(reviewed_agent):
    agent = reviewed_agent
    agent.model = "qwen3.6-27b"
    tool = _mock_tool_call(name="web_search", arguments='{"q":"product"}', call_id="c1")
    agent.client.chat.completions.create.side_effect = [
        _mock_response(content="页面可能还没加载完。让我等一等然后获取快照。"),
        _mock_response(content="", tool_calls=[tool], finish_reason="tool_calls"),
        _mock_response(content="已核验商品条件，以下是结果。"),
    ]
    with (patch("agent.completion_review._model_call", side_effect=[verdict(), verdict("completed")]) as judge,
          patch("run_agent.handle_function_call", return_value='{"success":true,"delivery_days":1}')):
        result = agent.run_conversation("查找商品并核验配送时间")
    assert result["completed"] is True
    assert result["api_calls"] == 3
    assert result["task_outcome"] == "completed"
    assert judge.call_count == 2
    assert any(m.get("_completion_review_feedback") for m in result["messages"])
    payload = json.loads(judge.call_args.args[0])
    assert "delivery_days" not in judge.call_args.args[0]
    assert "Internal task review" not in judge.call_args.args[0]
    assert payload["request"] == "查找商品并核验配送时间"
    assert payload["candidate"] == "已核验商品条件，以下是结果。"
    assert any("获取快照" in m["content"] for m in payload["context"])
    assert any("delivery_days" in str(m.get("content")) for m in result["messages"])


@pytest.mark.parametrize("decision", ["await_user", "await_approval", "blocked"])
def test_unfinished_todo_can_pause_without_forced_execution(reviewed_agent, decision):
    agent = reviewed_agent
    agent._todo_store.write([{"id": "1", "content": "用户选择后购买", "status": "in_progress", "executor": "mainline"}])
    agent.client.chat.completions.create.return_value = _mock_response(content="需要你选择颜色。")
    with patch("agent.completion_review._model_call", return_value=verdict(decision)):
        result = agent.run_conversation("继续")
    assert result["completed"] is False
    assert result["task_outcome"] == decision
    assert result["api_calls"] == 1
    assert agent._todo_store.read()[0]["status"] == "in_progress"
    agent._cleanup_task_resources.assert_not_called()


def test_stale_todo_does_not_override_evidence_of_completion(reviewed_agent):
    agent = reviewed_agent
    agent._todo_store.write([{"id": "1", "content": "回答问题", "status": "in_progress", "executor": "mainline"}])
    agent.client.chat.completions.create.return_value = _mock_response(content="答案是42。")
    with patch("agent.completion_review._model_call", return_value=verdict("completed")):
        result = agent.run_conversation("给我答案")
    assert result["completed"] is True
    assert result["task_status"]["complete"] is False
    agent._cleanup_task_resources.assert_called_once()


def test_checked_todo_does_not_bypass_reviewer(reviewed_agent):
    agent = reviewed_agent
    agent._todo_store.write([{"id": "1", "content": "找商品", "status": "completed"}])
    agent.client.chat.completions.create.return_value = _mock_response(content="完成了")
    with patch("agent.completion_review._model_call", return_value=verdict("blocked")):
        result = agent.run_conversation("继续")
    assert result["completed"] is False
    assert result["task_status"]["complete"] is True


def test_review_failure_is_not_success(reviewed_agent):
    reviewed_agent.client.chat.completions.create.return_value = _mock_response(content="已经完成。")
    with patch("agent.completion_review._model_call", return_value=("bad json", {})):
        result = reviewed_agent.run_conversation("核验商品")
    assert result["completed"] is False
    assert result["task_outcome"] == "review_failed"
    assert "评审" in result["final_response"] and "暂停" in result["final_response"]


def test_cancel_during_judge_cannot_be_overridden(reviewed_agent):
    reviewed_agent.client.chat.completions.create.return_value = _mock_response(content="完成。")
    def stop(*args):
        reviewed_agent.interrupt("user stopped")
        return verdict("completed")
    with patch("agent.completion_review._model_call", side_effect=stop):
        result = reviewed_agent.run_conversation("查商品")
    assert result["interrupted"] is True
    assert result["completed"] is False
    assert result["task_outcome"] == "cancelled"


def test_review_respects_shared_budget(reviewed_agent):
    reviewed_agent.max_iterations = 2
    reviewed_agent.client.chat.completions.create.return_value = _mock_response(content="接着查")
    with patch("agent.completion_review._model_call", return_value=verdict()) as judge:
        result = reviewed_agent.run_conversation("查商品")
    assert judge.call_count == 1
    assert result["api_calls"] == 1
    assert not result["completed"]
    assert result["task_outcome"] == "budget_exhausted"
