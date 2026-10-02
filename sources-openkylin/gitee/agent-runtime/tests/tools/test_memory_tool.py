"""Tests for database-backed long-term conversation memory."""

import json

from kylin_agent_runtime_state import MemoryDB, SessionDB
from tools.memory_tool import MEMORY_SCHEMA, memory


def _seed(db):
    db.create_session("old-memory", source="cli")
    db.set_session_title("old-memory", "Authentication refactor")
    db.append_message("old-memory", "user", "How should we refactor authentication?")
    db.append_message("old-memory", "assistant", "Use short-lived access tokens.")


def test_schema_exposes_discovery_scroll_and_browse_shapes():
    params = MEMORY_SCHEMA["parameters"]["properties"]
    assert MEMORY_SCHEMA["name"] == "memory"
    assert set(params) == {
        "query", "limit", "sort", "session_id", "around_message_id",
        "window", "role_filter",
    }
    assert MEMORY_SCHEMA["parameters"]["required"] == []
    assert "dedicated long-term memory database" in MEMORY_SCHEMA["description"]
    assert "DISCOVERY" in MEMORY_SCHEMA["description"]
    assert "SCROLL" in MEMORY_SCHEMA["description"]
    assert "BROWSE" in MEMORY_SCHEMA["description"]


def test_memory_db_defaults_to_separate_file(tmp_path, monkeypatch):
    memory_path = tmp_path / "memory.db"
    monkeypatch.setattr("kylin_agent_runtime_state.DEFAULT_MEMORY_DB_PATH", memory_path)
    db = MemoryDB()
    try:
        assert db.db_path == memory_path
    finally:
        db.close()


def test_default_memory_db_does_not_import_existing_session_history(tmp_path, monkeypatch):
    state_path = tmp_path / "state.db"
    memory_path = tmp_path / "memory.db"
    session_db = SessionDB(state_path)
    _seed(session_db)
    session_db.close()
    monkeypatch.setattr("kylin_agent_runtime_state.DEFAULT_DB_PATH", state_path)
    monkeypatch.setattr("kylin_agent_runtime_state.DEFAULT_MEMORY_DB_PATH", memory_path)

    db = MemoryDB()
    try:
        result = json.loads(memory(query="authentication", db=db))
        assert result["count"] == 0
        assert result["results"] == []
    finally:
        db.close()


def test_memory_search_reads_only_memory_store(tmp_path):
    session_db = SessionDB(tmp_path / "state.db")
    memory_db = MemoryDB(tmp_path / "memory.db")
    try:
        _seed(session_db)
        session_db.append_message("old-memory", "user", "session-only-secret")
        _seed(memory_db)
        memory_db.append_message("old-memory", "user", "memory-only-secret")

        found = json.loads(memory(query="memory-only-secret", db=memory_db))
        missing = json.loads(memory(query="session-only-secret", db=memory_db))
        assert found["success"] is True
        assert found["count"] == 1
        assert missing["success"] is True
        assert missing["results"] == []
    finally:
        session_db.close()
        memory_db.close()


def test_memory_supports_browse_and_scroll(tmp_path):
    db = MemoryDB(tmp_path / "memory.db")
    try:
        _seed(db)
        browse = json.loads(memory(db=db))
        assert browse["mode"] == "browse"
        assert browse["results"][0]["session_id"] == "old-memory"

        discovered = json.loads(memory(query="authentication", db=db))
        hit = discovered["results"][0]
        scrolled = json.loads(memory(
            session_id=hit["session_id"],
            around_message_id=hit["match_message_id"],
            db=db,
        ))
        assert scrolled["mode"] == "scroll"
        assert any(msg.get("anchor") for msg in scrolled["messages"])
    finally:
        db.close()
