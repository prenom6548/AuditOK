from unittest.mock import MagicMock, patch

from gateway.session_models import SessionModelController, public_selection
from kylin_agent_runtime_state import SessionDB
from tests.run_agent.test_run_agent import agent, _mock_response, _mock_tool_call


def test_cancel_prevents_pending_model_change(agent, tmp_path):
    db = SessionDB(tmp_path / "cancel.db")
    db.create_session("a", "desktop")
    db.set_session_model_selection("a", {"model": "new-model"})
    control = SessionModelController(db, "a", lambda: [], {})
    agent._interrupt_requested = True
    with patch("gateway.session_models.switch_at_boundary") as switch:
        control(agent)
    switch.assert_not_called()
    db.close()


def test_running_task_changes_client_after_tool_and_preserves_context(agent, tmp_path):
    db = SessionDB(tmp_path / "switch.db")
    db.create_session("a", "desktop")
    old = {"model": agent.model, "provider": "custom", "baseUrl": "http://localhost:8111/v1"}
    new = {"model": "next-model", "provider": "custom", "baseUrl": "http://localhost:8222/v1"}
    initial = db.set_session_model_selection("a", public_selection(old))
    agent._model_selection_checkpoint = SessionModelController(db, "a", lambda: [old, new], initial)
    agent._cached_system_prompt = "stable prefix"
    agent._use_prompt_caching = False
    agent.compression_enabled = False
    agent.tool_delay = 0
    old_client = agent.client
    old_client.chat.completions.create.return_value = _mock_response(
        content="", tool_calls=[_mock_tool_call(name="web_search", arguments='{"q":"fixture"}', call_id="tool-1")], finish_reason="tool_calls")
    new_client = MagicMock()
    new_client.chat.completions.create.return_value = _mock_response(content="done")
    def complete_tool(*args, **kwargs):
        db.set_session_model_selection("a", public_selection(new))
        assert agent.client is old_client
        return '{"evidence":"tool completed exactly once"}'
    runtime = {"provider": "custom", "api_key": "key-b", "base_url": new["baseUrl"], "api_mode": "chat_completions", "credential_pool": None}
    with patch("run_agent.handle_function_call", side_effect=complete_tool) as tool, \
         patch("gateway.session_models.resolve_entry_runtime", return_value=runtime), \
         patch("run_agent.AIAgent._create_openai_client", return_value=new_client), \
         patch("agent.model_metadata.get_model_context_length", return_value=128000), \
         patch.object(agent, "_persist_session"), patch.object(agent, "_save_trajectory"), \
         patch.object(agent, "_cleanup_task_resources"):
        result = agent.run_conversation("lookup")
    assert result["completed"] is True
    assert old_client.chat.completions.create.call_count == 1
    assert new_client.chat.completions.create.call_count == 1
    assert new_client.chat.completions.create.call_args.kwargs["model"] == "next-model"
    assert tool.call_count == 1
    assert any("tool completed exactly once" in str(m.get("content")) for m in result["messages"])
    assert agent._cached_system_prompt == "stable prefix"
    assert agent._credential_pool is None
    db.close()
