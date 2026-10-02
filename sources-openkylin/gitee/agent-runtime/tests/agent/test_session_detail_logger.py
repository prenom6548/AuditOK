from agent import session_detail_logger as sdl


def test_session_usage_log_path_replaces_details(tmp_path, monkeypatch):
    monkeypatch.setattr(sdl, "get_hermes_home", lambda: tmp_path)

    path = sdl.session_usage_log_path("abc")

    assert path == tmp_path / "sessions" / "abc-usage.log"


def test_write_session_usage_appends_plain_text(tmp_path, monkeypatch):
    monkeypatch.setattr(sdl, "get_hermes_home", lambda: tmp_path)

    sdl.write_session_usage("abc", "Token usage [trunk] total=123")

    text = (tmp_path / "sessions" / "abc-usage.log").read_text(encoding="utf-8")
    assert "Token usage [trunk] total=123" in text


def test_detail_context_enriches_persisted_and_subscribed_events(tmp_path, monkeypatch):
    monkeypatch.setattr(sdl, "get_hermes_home", lambda: tmp_path)
    received = []
    unsubscribe = sdl.subscribe_session_details("parent", received.append)

    with sdl.session_detail_context(
        scope="branch", subagent_id="sa-1", task_index=0
    ):
        sdl.write_session_detail("parent", "model.request", {"model": "test"})
    unsubscribe()
    sdl.write_session_detail("parent", "model.response", {"model": "ignored"})

    assert len(received) == 1
    assert received[0]["event"] == "model.request"
    assert received[0]["data"] == {
        "model": "test",
        "scope": "branch",
        "subagent_id": "sa-1",
        "task_index": 0,
    }


def test_detail_subscriber_receives_redacted_payload(tmp_path, monkeypatch):
    monkeypatch.setattr(sdl, "get_hermes_home", lambda: tmp_path)
    received = []
    unsubscribe = sdl.subscribe_session_details("parent", received.append)

    sdl.write_session_detail("parent", "tool.started", {"api_key": "secret"})
    unsubscribe()

    assert received[0]["data"]["api_key"] == "***"
