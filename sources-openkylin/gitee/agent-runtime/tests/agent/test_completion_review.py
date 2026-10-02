import json
import threading
from types import SimpleNamespace
from unittest.mock import MagicMock

import pytest

from agent.completion_review import (
    CompletionReviewer, ReviewSettings, ReviewResponseError, _model_call, parse_review,
    persist_review_outcome, visible_review_context,
)
from agent.iteration_budget import IterationBudget
from kylin_agent_runtime_state import SessionDB


def verdict(decision="continue", **updates):
    result = {
        "analysis": "还有商品条件未核验。", "score": 35,
        "decision": decision, "reason": "回复只说明准备获取快照。",
        "suggestions": ["获取页面快照并核验配送条件。"],
        "evidence": [{"source": "candidate", "finding": "承诺下一步行动"}],
    }
    result.update(updates)
    return json.dumps(result, ensure_ascii=False), {"model": "judge", "usage": {}}


@pytest.fixture
def runner():
    return SimpleNamespace(
        _interrupt_requested=False, iteration_budget=IterationBudget(30),
        _api_call_count=1, max_iterations=30, session_id="review-test",
        _cached_system_prompt="Only authorized actions.", _emit_status=MagicMock(),
        provider="custom", model="test", api_key="fake", base_url="http://localhost/v1", api_mode="chat_completions",
    )


def review(reviewer, runner, messages=None, todo=None):
    return reviewer.review(runner, request="找满足要求的商品", messages=messages or [],
                           candidate="页面可能还没加载完。让我等一等然后获取快照。",
                           todo=todo or {"has_plan": False})


@pytest.mark.parametrize("decision,completed,stop", [
    ("completed", True, True), ("continue", False, False),
    ("await_user", False, True), ("await_approval", False, True), ("blocked", False, True),
])
def test_decision_drives_flags(decision, completed, stop):
    result = parse_review(verdict(decision)[0], {"candidate"})
    assert result.task_completed is completed
    assert result.should_stop is stop


@pytest.mark.parametrize("update", [
    {"score": True}, {"score": 101}, {"score": float("nan")},
    {"decision": "done"}, {"decision": []}, {"suggestions": []}, {"task_completed": True},
    {"should_stop": "false"}, {"analysis": ""},
    {"evidence": [{"source": "message:99", "finding": "invented"}]},
    {"evidence": [{"source": [], "finding": "invalid source type"}]},
])
def test_invalid_or_inconsistent_verdict_is_rejected(update):
    with pytest.raises(ValueError):
        parse_review(verdict(**update)[0], {"candidate"})


def test_single_json_fence_is_accepted_without_accepting_prose_or_reasoning():
    raw = verdict("completed")[0]
    assert parse_review("```json\n" + raw + "\n```", {"candidate"}).task_completed
    with pytest.raises(ValueError):
        parse_review("<think>" + raw + "</think>", {"candidate"})
    with pytest.raises(ValueError):
        parse_review("unrelated prose " + raw, {"candidate"})


def mock_review_client(monkeypatch, responses, base_url="https://api.deepseek.com/v1"):
    client = MagicMock()
    client.base_url = base_url
    client.with_options.return_value = client
    client.chat.completions.create.side_effect = responses
    monkeypatch.setattr("agent.auxiliary_client.get_text_auxiliary_client", lambda *a, **k: (client, "judge"))
    return client


def model_response(content, finish="stop", reasoning="", completion_tokens=10):
    return SimpleNamespace(
        choices=[SimpleNamespace(message=SimpleNamespace(content=content, reasoning_content=reasoning, tool_calls=None), finish_reason=finish)],
        usage={"prompt_tokens": 100, "completion_tokens": completion_tokens,
               "completion_tokens_details": {"reasoning_tokens": completion_tokens if reasoning else 0}},
    )


def test_reasoning_exhaustion_retries_with_more_room_and_accounts_failed_usage(monkeypatch, runner):
    client = mock_review_client(monkeypatch, [
        model_response("", "length", "private reasoning", 4096),
        model_response(verdict("completed")[0], completion_tokens=20),
    ])
    events = []
    monkeypatch.setattr("agent.session_detail_logger.write_session_detail", lambda sid, event, data: events.append((event, data)))
    result = review(CompletionReviewer(ReviewSettings(max_tokens=4096, max_retry_tokens=8192)), runner)
    assert result.task_completed and result.attempts == 2
    requests = [c.kwargs for c in client.chat.completions.create.call_args_list]
    assert [r["max_tokens"] for r in requests] == [4096, 8192]
    assert requests[0]["messages"][1] == requests[1]["messages"][1]  # full evidence unchanged
    assert "output_truncated" in requests[1]["messages"][0]["content"]
    assert all("tools" not in r for r in requests)
    assert runner.iteration_budget.used == runner.session_api_calls == 2
    assert runner.session_completion_tokens == 4116
    assert runner.session_prompt_tokens == 200
    assert runner.session_reasoning_tokens == 4096
    retry = next(data for event, data in events if event == "task.review.retry")
    assert retry["stage"] == "response" and retry["error_code"] == "output_truncated"
    assert "private reasoning" not in json.dumps(events)


def test_truncated_json_is_never_accepted_even_when_it_looks_complete(monkeypatch, runner):
    client = mock_review_client(monkeypatch, [model_response(verdict("completed")[0], "length")])
    result = review(CompletionReviewer(ReviewSettings(max_tokens=128, max_retry_tokens=128)), runner)
    assert not result.task_completed and result.decision == "review_failed"
    assert result.failure["finish_reason"] == "length"
    assert result.failure["error_code"] == "output_truncated"
    assert client.chat.completions.create.call_count == 1


def test_schema_retry_explains_error_and_does_not_change_evidence(monkeypatch, runner):
    client = mock_review_client(monkeypatch, [
        model_response(verdict("completed", evidence=[{"source": "message:999", "finding": "invented"}])[0]),
        model_response(verdict("completed")[0]),
    ])
    result = review(CompletionReviewer(), runner)
    assert result.task_completed and result.attempts == 2
    requests = [c.kwargs for c in client.chat.completions.create.call_args_list]
    assert requests[0]["messages"][1] == requests[1]["messages"][1]
    assert "source ID present in context" in requests[1]["messages"][0]["content"]


@pytest.mark.parametrize("base_url,json_mode,limit_key", [
    ("https://api.deepseek.com/v1", True, "max_tokens"),
    ("http://localhost:8111/v1", False, "max_tokens"),
    ("https://api.openai.com/v1", False, "max_completion_tokens"),
])
def test_output_options_follow_actual_endpoint(monkeypatch, base_url, json_mode, limit_key):
    client = mock_review_client(monkeypatch, [model_response(verdict()[0])], base_url)
    _model_call("{}", {}, ReviewSettings())
    kwargs = client.chat.completions.create.call_args.kwargs
    assert (kwargs.get("response_format") == {"type": "json_object"}) is json_mode
    assert kwargs[limit_key] == ReviewSettings().max_tokens


def test_cancel_after_truncated_response_prevents_retry(monkeypatch, runner):
    def cancelled(**kwargs):
        runner._interrupt_requested = True
        return model_response("", "length", "thinking", 4096)
    client = mock_review_client(monkeypatch, cancelled)
    assert review(CompletionReviewer(), runner).decision == "cancelled"
    assert client.chat.completions.create.call_count == 1


def test_empty_reasoning_only_response_is_not_a_verdict(monkeypatch):
    mock_review_client(monkeypatch, [model_response("", "stop", "not a verdict")])
    with pytest.raises(ReviewResponseError) as exc:
        _model_call("{}", {}, ReviewSettings())
    assert exc.value.code == "empty_response"


def test_invalid_credentials_pause_without_identical_retries(monkeypatch, runner):
    import httpx
    from openai import AuthenticationError
    response = httpx.Response(401, request=httpx.Request("POST", "https://review.invalid"))
    client = mock_review_client(monkeypatch, [AuthenticationError("private response body", response=response, body=None)])
    result = review(CompletionReviewer(), runner)
    assert result.decision == "review_failed" and result.attempts == 1
    assert result.failure["error_code"] == "authentication_failed"
    assert client.chat.completions.create.call_count == 1
    assert "private response body" not in json.dumps(result.to_dict())


def test_only_visible_dialogue_is_sent_and_original_history_is_unchanged(monkeypatch, runner):
    calls = []
    def model(payload, runtime, settings):
        calls.append(json.loads(payload))
        assert runtime["model"] == "test"
        return verdict()
    monkeypatch.setattr("agent.completion_review._model_call", model)
    reviewer = CompletionReviewer(ReviewSettings())
    messages = [{"role": "system", "content": "INTERNAL_SYSTEM"},
                {"role": "user", "content": "预算3785"},
                {"role": "assistant", "content": "正在核验配送条件。", "reasoning": "INTERNAL_REASONING",
                 "tool_calls": [{"id": "c1", "function": {"name": "browser_snapshot", "arguments": "INTERNAL_ARGUMENTS"}}]},
                {"role": "tool", "name": "browser_snapshot", "content": "INTERNAL_TOOL_RESULT"},
                {"role": "assistant", "content": "<think>INTERNAL_THINK</think>已找到两件符合条件的商品。"},
                {"role": "user", "content": "INTERNAL_REVIEW", "_completion_review_feedback": True},
                {"role": "user", "content": "INTERNAL_RECOVERY", "_empty_recovery_synthetic": True},
                {"role": "assistant", "content": "", "reasoning_content": "INTERNAL_REASONING_ONLY"}]
    original = json.dumps(messages)
    runner._cached_system_prompt = "INTERNAL_EXECUTION_POLICY"
    runner._turn_failed_file_mutations = {"INTERNAL_FAILURE": "details"}
    review(reviewer, runner, messages, todo={"INTERNAL_TODO": True})
    review(reviewer, runner, messages)
    assert calls[0]["context"] == [
        {"id": "message:1", "role": "user", "content": "预算3785"},
        {"id": "message:2", "role": "assistant", "content": "正在核验配送条件。"},
        {"id": "message:4", "role": "assistant", "content": "已找到两件符合条件的商品。"},
    ]
    assert set(calls[0]) == {"request", "context", "candidate"}
    assert "INTERNAL_" not in json.dumps(calls)
    assert "previous_reviews" not in calls[1]
    assert json.dumps(messages) == original
    assert runner.iteration_budget.used == 2


def test_multimodal_input_keeps_text_without_images_or_reasoning_blocks():
    messages = [{"role": "user", "content": [
        {"type": "text", "text": "优先蓝色"},
        {"type": "image_url", "image_url": {"url": "data:image/png;base64,INTERNAL_IMAGE"}},
    ]}, {"role": "assistant", "content": [
        {"type": "thinking", "thinking": "INTERNAL_THINKING"},
        {"type": "tool_use", "name": "terminal", "input": {"command": "INTERNAL_COMMAND"}},
        {"type": "text", "text": "已筛选蓝色款。"},
    ]}]
    assert [m["content"] for m in visible_review_context(messages)] == ["优先蓝色", "已筛选蓝色款。"]


def test_recovery_and_compression_wrappers_do_not_leak_runtime_context():
    messages = [
        {"role": "user", "content": "Continue the interrupted task from its durable checkpoint.\n"
         "Original request: 查找蓝色上衣\n预算300\nInterrupted location: INTERNAL_CHECKPOINT\n"
         "Interrupted tool arguments: INTERNAL_ARGUMENTS"},
        {"role": "assistant", "content": "[CONTEXT SUMMARY]: INTERNAL_SUMMARY"},
        {"role": "user", "content": "[CONTEXT COMPACTION — REFERENCE ONLY] INTERNAL_SUMMARY\n\n"
         "--- END OF CONTEXT SUMMARY — respond to the message below, not the summary above ---\n\n改成灰色"},
        {"role": "user", "content": "[Internal task review; this is not a new user request or authorization.] INTERNAL_REVIEW"},
        {"role": "user", "content": "[System: Continue now. INTERNAL_CONTROL]"},
        {"role": "assistant", "content": "<memory-context>INTERNAL_MEMORY</memory-context>已改成灰色。"},
    ]
    context = visible_review_context(messages)
    assert [m["content"] for m in context] == ["继续上次中断的任务。\n原始请求：查找蓝色上衣\n预算300", "改成灰色", "已改成灰色。"]
    assert "INTERNAL_" not in json.dumps(context)


def test_large_tool_results_do_not_exhaust_visible_review_input(monkeypatch, runner):
    call = MagicMock(return_value=verdict("completed"))
    monkeypatch.setattr("agent.completion_review._model_call", call)
    messages = [{"role": "user", "content": "给出结果"}, {"role": "tool", "content": "x" * 200000}]
    result = review(CompletionReviewer(ReviewSettings(max_context_chars=1000)), runner, messages)
    assert result.task_completed
    assert len(call.call_args.args[0]) < 1000


def test_todo_is_advisory_not_a_completion_gate(monkeypatch, runner):
    monkeypatch.setattr("agent.completion_review._model_call", lambda *a: verdict("completed"))
    result = review(CompletionReviewer(), runner, todo={"has_plan": True, "complete": False})
    assert result.task_completed


def test_invalid_output_retries_then_pauses(monkeypatch, runner):
    call = MagicMock(return_value=("not JSON", {}))
    monkeypatch.setattr("agent.completion_review._model_call", call)
    result = review(CompletionReviewer(ReviewSettings(max_attempts=2)), runner)
    assert call.call_count == 2
    assert result.decision == "review_failed"
    assert not result.task_completed and result.should_stop
    assert result.failure["error_code"] == "invalid_json"
    assert result.failure["stage"] == "validation"
    assert result.attempts == 2


def test_unavailable_model_pauses_with_specific_diagnostics_without_retry(monkeypatch, runner):
    resolver = MagicMock(return_value=(None, None))
    monkeypatch.setattr("agent.auxiliary_client.get_text_auxiliary_client", resolver)
    events = []
    monkeypatch.setattr("agent.session_detail_logger.write_session_detail",
                        lambda sid, event, data: events.append((event, data)))
    result = review(CompletionReviewer(), runner)
    assert resolver.call_count == 1
    assert result.decision == "review_failed" and not result.task_completed
    assert result.attempts == 1
    assert result.failure["error_code"] == "model_unavailable"
    assert result.failure["stage"] == "client_resolution"
    assert "未能创建评审模型客户端" in result.reason
    assert "超时" not in result.reason
    assert next(data for event, data in events if event == "task.review.retry")["will_retry"] is False


def test_failure_diagnostics_never_include_provider_exception_body(monkeypatch, runner):
    error = RuntimeError("Authorization: Bearer private-test-key; private request body")
    monkeypatch.setattr("agent.completion_review._model_call", MagicMock(side_effect=error))
    events = []
    monkeypatch.setattr("agent.session_detail_logger.write_session_detail",
                        lambda sid, event, data: events.append((event, data)))
    result = review(CompletionReviewer(), runner)
    assert result.failure["stage"] == "model_call"
    assert result.failure["error_type"] == "RuntimeError"
    assert "private-test-key" not in json.dumps(events) + json.dumps(result.to_dict())
    assert "private request body" not in json.dumps(events) + json.dumps(result.to_dict())


def test_transient_model_error_can_recover(monkeypatch, runner):
    call = MagicMock(side_effect=[TimeoutError(), verdict("completed")])
    monkeypatch.setattr("agent.completion_review._model_call", call)
    assert review(CompletionReviewer(), runner).task_completed
    assert call.call_count == 2


def test_hard_timeout_does_not_launch_overlapping_retry(monkeypatch, runner):
    release = threading.Event()
    def slow(*args):
        release.wait(2)
        return verdict("completed")
    call = MagicMock(side_effect=slow)
    monkeypatch.setattr("agent.completion_review._model_call", call)
    try:
        result = review(CompletionReviewer(ReviewSettings(timeout=0.03)), runner)
        assert result.decision == "review_failed"
        assert call.call_count == 1
    finally:
        release.set()


def test_cancellation_during_review_discards_late_completion(monkeypatch, runner):
    def cancel(*args):
        runner._interrupt_requested = True
        return verdict("completed")
    monkeypatch.setattr("agent.completion_review._model_call", cancel)
    result = review(CompletionReviewer(), runner)
    assert result.decision == "cancelled" and not result.task_completed


@pytest.mark.parametrize("boundary", ["cancel", "budget"])
def test_stop_and_budget_prevent_review_calls(monkeypatch, runner, boundary):
    call = MagicMock()
    monkeypatch.setattr("agent.completion_review._model_call", call)
    if boundary == "cancel":
        runner._interrupt_requested = True
    else:
        runner.iteration_budget = IterationBudget(0)
    assert review(CompletionReviewer(), runner).should_stop
    call.assert_not_called()


def test_last_review_cannot_restart_exhausted_execution(monkeypatch, runner):
    monkeypatch.setattr("agent.completion_review._model_call", lambda *a: verdict())
    runner.iteration_budget = IterationBudget(1)
    assert review(CompletionReviewer(), runner).decision == "budget_exhausted"


def test_repeated_evidence_cannot_keep_loop_alive(monkeypatch, runner):
    monkeypatch.setattr("agent.completion_review._model_call", lambda *a: verdict())
    reviewer = CompletionReviewer(ReviewSettings(max_no_progress=2))
    for count in range(3):
        messages = [{"role": "tool", "name": "browser_snapshot", "tool_call_id": f"new-{count}", "content": "same failure"}]
        result = review(reviewer, runner, messages)
    assert result.decision == "no_progress"


def test_new_evidence_resets_no_progress_but_total_reviews_are_bounded(monkeypatch, runner):
    monkeypatch.setattr("agent.completion_review._model_call", lambda *a: verdict())
    reviewer = CompletionReviewer(ReviewSettings(max_reviews=3, max_no_progress=1))
    for count in range(3):
        result = review(reviewer, runner, [{"role": "tool", "name": "browser_snapshot", "content": f"new product {count}"}])
        assert result.decision == "continue"
    assert review(reviewer, runner).decision == "review_limit"


def test_context_is_never_silently_truncated(monkeypatch, runner):
    call = MagicMock()
    monkeypatch.setattr("agent.completion_review._model_call", call)
    result = review(CompletionReviewer(ReviewSettings(max_context_chars=20)), runner)
    assert result.decision == "review_failed"
    call.assert_not_called()


def test_model_call_has_no_tools_or_agent_recursion(monkeypatch):
    client = MagicMock()
    client.with_options.return_value = client
    client.chat.completions.create.return_value = SimpleNamespace(
        choices=[SimpleNamespace(message=SimpleNamespace(content=verdict()[0], tool_calls=None), finish_reason="stop")], usage=None)
    monkeypatch.setattr("agent.auxiliary_client.get_text_auxiliary_client", lambda *a, **k: (client, "judge"))
    _model_call("{}", {}, ReviewSettings())
    kwargs = client.chat.completions.create.call_args.kwargs
    assert "tools" not in kwargs
    assert [m["role"] for m in kwargs["messages"]] == ["system", "user"]
    assert client.with_options.call_args.kwargs["max_retries"] == 0


def test_explicit_review_provider_does_not_inherit_another_providers_credentials(monkeypatch):
    from agent.auxiliary_client import get_text_auxiliary_client
    monkeypatch.setattr("agent.auxiliary_client._get_auxiliary_task_config", lambda task: {
        "provider": "deepseek", "model": "deepseek-v4-flash",
    })
    monkeypatch.setattr("kylin_agent_runtime_cli.auth.resolve_api_key_provider_credentials", lambda provider: {
        "api_key": "review-only-key", "base_url": "http://127.0.0.1:9999/v1",
    })
    client, model = get_text_auxiliary_client("completion_review", main_runtime={
        "provider": "custom", "model": "main-model", "api_key": "main-only-key",
        "base_url": "http://127.0.0.1:8888/v1",
    })
    try:
        assert model == "deepseek-v4-flash"
        assert client.api_key == "review-only-key"
        assert str(client.base_url) == "http://127.0.0.1:9999/v1/"
    finally:
        client.close()


def test_paused_review_survives_recovery_and_cancel_race(tmp_path):
    db = SessionDB(tmp_path / "state.db")
    db.create_session("s", "desktop")
    db.start_task_run("t", "s", "find goods")
    result = {"completed": False, "completion_review": {"decision": "await_user", "reason": "请选择颜色"}}
    persist_review_outcome(db, "t", result)
    task = db.get_recoverable_task("s")
    assert task["execution_context"]["completion_review"]["decision"] == "await_user"
    assert task["interruption_reason"] == "请选择颜色"
    assert db.finish_task_run("t", "cancelled")
    persist_review_outcome(db, "t", {"completed": True})
    assert db.get_task_run("t")["status"] == "cancelled"
    db.close()


@pytest.mark.parametrize("provider", ["custom", "deepseek", "custom:deepseek"])
def test_real_auxiliary_resolution_uses_local_main_runtime(monkeypatch, provider):
    """Exercise real client resolution and HTTP serialization, without live services."""
    from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
    # Desktop settings provide credentials on the live agent, with no matching
    # provider environment variable. Exercise that path without any fallback.
    monkeypatch.setattr("agent.auxiliary_client._get_auxiliary_task_config", lambda task: {})
    monkeypatch.setattr("agent.auxiliary_client._get_provider_chain", lambda: [])
    monkeypatch.setattr("kylin_agent_runtime_cli.auth.resolve_api_key_provider_credentials", lambda provider: {})
    captured = []
    authorization = []
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass
        def do_POST(self):
            authorization.append(self.headers.get("Authorization"))
            captured.append(json.loads(self.rfile.read(int(self.headers["Content-Length"]))))
            body = json.dumps({"id": "review-1", "object": "chat.completion", "created": 1,
                               "model": "deepseek-v4-flash", "choices": [{"index": 0, "finish_reason": "stop",
                               "message": {"role": "assistant", "content": verdict("completed")[0]}}]}).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        raw, _ = _model_call("{\"request\":\"hello\"}", {
            "provider": provider, "model": "deepseek-v4-flash", "api_key": "local-test",
            "api_mode": "chat_completions", "base_url": f"http://127.0.0.1:{server.server_port}/v1",
        }, ReviewSettings(timeout=3))
        assert parse_review(raw, {"candidate"}).task_completed
        assert captured[0]["model"] == "deepseek-v4-flash"
        assert "tools" not in captured[0]
        assert authorization == ["Bearer local-test"]
    finally:
        server.shutdown()
        server.server_close()
        thread.join(2)
