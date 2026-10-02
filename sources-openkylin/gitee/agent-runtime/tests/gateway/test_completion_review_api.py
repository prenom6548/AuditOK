import json
from unittest.mock import MagicMock, patch

import pytest
from aiohttp.test_utils import TestClient, TestServer

from agent.session_detail_logger import write_session_detail
from tests.gateway.test_api_server import adapter, _create_app


@pytest.mark.asyncio
@pytest.mark.parametrize("stream", [False, True])
async def test_review_pause_is_exposed_and_recoverable(adapter, stream):
    review = {"decision": "await_user", "analysis": "需要选择颜色", "score": 40,
              "task_completed": False, "should_stop": True, "reason": "请选择颜色",
              "suggestions": ["请选择灰色或黑色"], "evidence": []}
    async def run(**kwargs):
        write_session_detail("review-session", "task.review.started", {"review_number": 1})
        write_session_detail("review-session", "task.review.completed", review)
        if kwargs.get("stream_delta_callback"):
            kwargs["stream_delta_callback"]("请选择颜色")
        return ({"final_response": "请选择颜色", "completed": False, "messages": [],
                 "completion_review": review, "task_outcome": "await_user"}, {})
    adapter._ensure_session_db().create_session("review-session", "desktop")
    app = _create_app(adapter)
    app.router.add_get("/api/sessions/{session_id}/recovery", adapter._handle_get_session_recovery)
    async with TestClient(TestServer(app)) as client:
        with patch.object(adapter, "_run_agent", side_effect=run):
            response = await client.post("/v1/chat/completions",
                headers={"X-Hermes-Session-Id": "review-session"},
                json={"model": "test", "messages": [{"role": "user", "content": "find goods"}], "stream": stream})
            assert response.status == 200
            if stream:
                text = await response.text()
                assert text.index("event: task.review.started") < text.index("event: task.review.completed") < text.index("[DONE]")
                chunks = [json.loads(line[6:]) for line in text.splitlines() if line.startswith("data: {")]
                payload = next(chunk for chunk in chunks if "hermes" in chunk)
            else:
                payload = await response.json()
            assert payload["hermes"]["completed"] is False
            assert payload["hermes"]["task_outcome"] == "await_user"
            assert payload["hermes"]["completion_review"]["score"] == 40
        response = await client.get("/api/sessions/review-session/recovery")
        recovery = await response.json()
        assert recovery["recoverable"] is True
        assert recovery["execution_context"]["completion_review"]["decision"] == "await_user"


@pytest.mark.asyncio
@pytest.mark.parametrize("stream", [False, True])
async def test_responses_api_does_not_label_review_pause_completed(adapter, stream):
    result = {"final_response": "等待用户输入", "completed": False, "messages": [],
              "task_outcome": "await_user", "completion_review": {"decision": "await_user"}}
    async def run(**kwargs):
        return result, {}
    async with TestClient(TestServer(_create_app(adapter))) as client:
        with patch.object(adapter, "_run_agent", side_effect=run):
            response = await client.post("/v1/responses", json={"input": "task", "stream": stream})
            assert response.status == 200
            if stream:
                text = await response.text()
                assert "event: response.incomplete" in text
                assert "event: response.completed" not in text
            else:
                payload = await response.json()
                assert payload["status"] == "incomplete"
                assert payload["hermes"]["task_outcome"] == "await_user"


@pytest.mark.asyncio
async def test_structured_run_keeps_review_pause_status(adapter):
    from tests.gateway.test_api_server_runs import _create_runs_app
    agent = MagicMock()
    agent.run_conversation.return_value = {
        "final_response": "评审失败，暂停", "completed": False,
        "completion_review": {"decision": "review_failed"}, "task_outcome": "review_failed",
    }
    agent.session_prompt_tokens = agent.session_completion_tokens = agent.session_total_tokens = 0
    async with TestClient(TestServer(_create_runs_app(adapter))) as client:
        with patch.object(adapter, "_create_agent", return_value=agent):
            response = await client.post("/v1/runs", json={"input": "task"})
            run_id = (await response.json())["run_id"]
            events = await client.get(f"/v1/runs/{run_id}/events")
            text = await events.text()
            assert "run.paused" in text and "run.completed" not in text
            response = await client.get(f"/v1/runs/{run_id}")
            state = await response.json()
            assert state["status"] == "paused"
            assert state["task_outcome"] == "review_failed"
