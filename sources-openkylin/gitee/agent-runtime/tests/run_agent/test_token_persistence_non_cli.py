from types import ModuleType, SimpleNamespace
from unittest.mock import MagicMock, patch
import json
import sys

from run_agent import AIAgent


def _mock_response(*, usage: dict, content: str = "done"):
    msg = SimpleNamespace(content=content, tool_calls=None)
    choice = SimpleNamespace(message=msg, finish_reason="stop")
    return SimpleNamespace(
        choices=[choice],
        model="test/model",
        usage=SimpleNamespace(**usage),
    )


def _make_agent(session_db, *, platform: str):
    with (
        patch("run_agent.get_tool_definitions", return_value=[]),
        patch("run_agent.check_toolset_requirements", return_value={}),
        patch("run_agent.OpenAI"),
    ):
        agent = AIAgent(
            api_key="test-key",
            base_url="https://openrouter.ai/api/v1",
            quiet_mode=True,
            skip_context_files=True,
            skip_memory=True,
            session_db=session_db,
            session_id=f"{platform}-session",
            platform=platform,
        )
    agent.client = MagicMock()
    agent.client.chat.completions.create.return_value = _mock_response(
        usage={
            "prompt_tokens": 11,
            "completion_tokens": 7,
            "total_tokens": 18,
        }
    )
    return agent


def test_run_conversation_persists_tokens_for_telegram_sessions():
    session_db = MagicMock()
    agent = _make_agent(session_db, platform="telegram")

    result = agent.run_conversation("hello")

    assert result["final_response"] == "done"
    session_db.update_token_counts.assert_called_once()
    assert session_db.update_token_counts.call_args.args[0] == "telegram-session"


def test_run_conversation_persists_tokens_for_cron_sessions():
    session_db = MagicMock()
    agent = _make_agent(session_db, platform="cron")

    result = agent.run_conversation("hello")

    assert result["final_response"] == "done"
    session_db.update_token_counts.assert_called_once()
    assert session_db.update_token_counts.call_args.args[0] == "cron-session"


def test_memory_lazily_opens_dedicated_db(monkeypatch):
    sentinel_db = object()
    captured = {}

    class FakeMemoryDB:
        def __new__(cls):
            return sentinel_db

    kylin_agent_runtime_state = ModuleType("kylin_agent_runtime_state")
    kylin_agent_runtime_state.MemoryDB = FakeMemoryDB
    monkeypatch.setitem(sys.modules, "kylin_agent_runtime_state", kylin_agent_runtime_state)

    memory_mod = ModuleType("tools.memory_tool")

    def fake_memory(**kwargs):
        captured.update(kwargs)
        return json.dumps({"success": True, "results": []})

    memory_mod.memory = fake_memory
    monkeypatch.setitem(sys.modules, "tools.memory_tool", memory_mod)

    agent = _make_agent(None, platform="acp")
    agent._memory_db = None
    assert agent._memory_db_enabled is True
    result = json.loads(agent._invoke_tool("memory", {"query": "Hermes"}, "task-id"))

    assert result["success"] is True
    assert captured["db"] is sentinel_db
    assert captured["query"] == "Hermes"
    assert agent._memory_db is sentinel_db


def test_agent_close_releases_owned_memory_db():
    agent = _make_agent(None, platform="acp")
    memory_db = MagicMock()
    agent._memory_db = memory_db

    agent.close()

    memory_db.close.assert_called_once_with()
    assert agent._memory_db is None


def test_normal_conversation_persists_to_state_and_memory_db(tmp_path, monkeypatch):
    from kylin_agent_runtime_state import MemoryDB, SessionDB
    import kylin_agent_runtime_state

    state_path = tmp_path / "state.db"
    memory_path = tmp_path / "memory.db"
    monkeypatch.setattr(kylin_agent_runtime_state, "DEFAULT_DB_PATH", state_path)
    monkeypatch.setattr(kylin_agent_runtime_state, "DEFAULT_MEMORY_DB_PATH", memory_path)

    session_db = SessionDB(state_path)
    agent = _make_agent(session_db, platform="cli")
    try:
        result = agent.run_conversation("hello")
        assert result["final_response"] == "done"
        assert memory_path.exists()

        memory_db = MemoryDB(memory_path)
        try:
            state_messages = session_db.get_messages("cli-session")
            memory_messages = memory_db.get_messages("cli-session")
            assert [m["role"] for m in state_messages] == ["user", "assistant"]
            assert [m["role"] for m in memory_messages] == ["user", "assistant"]
            assert [m["content"] for m in memory_messages] == ["hello", "done"]
        finally:
            memory_db.close()
    finally:
        agent.close()
        session_db.close()


def test_memory_failure_does_not_duplicate_state_writes():
    session_db = MagicMock()
    memory_db = MagicMock()
    memory_db.append_message.side_effect = RuntimeError("memory unavailable")
    agent = _make_agent(session_db, platform="cli")
    agent._memory_db = memory_db

    messages = [
        {"role": "user", "content": "hello"},
        {"role": "assistant", "content": "done"},
    ]
    agent._flush_messages_to_session_memory_db(messages)
    agent._flush_messages_to_session_memory_db(messages)

    assert session_db.append_message.call_count == 2
    assert agent._last_flushed_db_idx == 2
    assert agent._last_flushed_memory_db_idx == 0
