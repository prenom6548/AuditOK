import json
from unittest.mock import MagicMock, patch

import pytest
from aiohttp.test_utils import TestClient, TestServer

from gateway.session_models import find_model_entry, public_selection, resolve_entry_runtime, switch_at_boundary
from kylin_agent_runtime_state import SessionDB
from tests.gateway.test_api_server import _make_adapter, _create_app


MODELS = [
    {"model": "shared-model", "provider": "custom", "baseUrl": "http://localhost:8111/v1", "apiKey": "key-a", "name": "A"},
    {"model": "shared-model", "provider": "custom", "baseUrl": "http://localhost:8222/v1", "apiKey": "key-b", "name": "B"},
]


@pytest.fixture
def adapter(tmp_path):
    instance = _make_adapter()
    # SessionDB's default path is captured during module import, before the
    # per-test HERMES_HOME fixture. Always provide an explicit isolated path.
    instance._session_db = SessionDB(tmp_path / "api-state.db")
    yield instance
    instance._session_db.close()


def test_same_model_id_requires_endpoint_and_never_uses_the_first_match():
    with pytest.raises(ValueError, match="ambiguous"):
        find_model_entry(MODELS, {"model": "shared-model"})
    assert find_model_entry(MODELS, public_selection(MODELS[1])) is MODELS[1]
    with pytest.raises(ValueError, match="no longer"):
        find_model_entry(MODELS, {"model": "shared-model", "base_url": "https://unconfigured.invalid"})


@pytest.mark.parametrize("saved_provider", ["custom", "kylin-local-test"])
def test_local_model_config_and_desktop_rows_share_identity(tmp_path, saved_provider):
    from gateway.platforms.api_server import APIServerAdapter
    config = tmp_path / "config.yaml"
    models = tmp_path / "models.json"
    config.write_text('custom_providers:\n  - name: kylin-local-test\n'
                      '    model: local-model\n    base_url: http://localhost:8111/v1\n')
    models.write_text(json.dumps([{"id": "desktop-row", "model": "local-model",
                                  "provider": "custom", "baseUrl": "http://localhost:8111/v1",
                                  "apiKey": "local-test-key", "name": "Local"}]))
    with patch.object(APIServerAdapter, "_config_file", return_value=config), \
         patch.object(APIServerAdapter, "_models_file", return_value=models):
        entries = APIServerAdapter._combined_model_entries()
    selection = {"model": "local-model", "provider": saved_provider,
                 "base_url": "http://localhost:8111/v1/"}
    entry = find_model_entry(entries, selection)
    assert public_selection(entry)["provider"] == "custom"
    runtime = resolve_entry_runtime(entry)
    assert runtime["base_url"] == "http://localhost:8111/v1"
    assert runtime["provider"] == "custom"
    with pytest.raises(ValueError, match="no longer"):
        find_model_entry(entries, {**selection, "base_url": "http://localhost:8222/v1"})
    with pytest.raises(ValueError, match="no longer"):
        find_model_entry(entries, {**selection, "provider": "different-cloud-provider"})


def test_session_selections_survive_restart_without_credentials(tmp_path):
    path = tmp_path / "state.db"
    db = SessionDB(path)
    for sid, entry in zip(["a", "b"], MODELS):
        db.create_session(sid, "desktop", model_config={"unrelated": True})
        db.set_session_model_selection(sid, {**public_selection(entry), "api_key": "must-not-save"})
    db.close()
    db = SessionDB(path)
    assert db.get_session_model_selection("a")["base_url"].endswith("8111/v1")
    assert db.get_session_model_selection("b")["base_url"].endswith("8222/v1")
    assert "must-not-save" not in db.get_session("a")["model_config"]
    assert json.loads(db.get_session("a")["model_config"])["unrelated"] is True
    db.close()


def test_runtime_resolution_uses_selected_model_protocol_and_credentials():
    with patch("kylin_agent_runtime_cli.runtime_provider.resolve_runtime_provider", return_value={}) as resolve:
        resolve_entry_runtime(MODELS[1])
    assert resolve.call_args.kwargs == {
        "requested": "custom", "explicit_base_url": MODELS[1]["baseUrl"],
        "explicit_api_key": "key-b", "target_model": "shared-model",
    }


def test_desktop_provider_alias_uses_its_configured_endpoint():
    runtime = resolve_entry_runtime({"model": "qwen-plus", "provider": "desktop-aliyun-alias",
                                    "baseUrl": "http://localhost:8222/v1", "apiKey": "alias-only-key"})
    assert runtime["provider"] == "custom"
    assert runtime["base_url"] == "http://localhost:8222/v1"
    assert runtime["api_key"] == "alias-only-key"


@pytest.mark.asyncio
async def test_chat_request_keeps_qualified_model_identity(adapter):
    db = adapter._ensure_session_db()
    db.create_session("a", "desktop")
    db.create_session("b", "desktop")
    db.set_session_model_selection("b", public_selection(MODELS[0]))
    async def run(**kwargs):
        assert kwargs["model_name"] == "shared-model"
        assert db.get_session_model_selection("a")["base_url"].endswith("8222/v1")
        return {"final_response": "done", "completed": True, "messages": []}, {}
    with patch.object(adapter, "_combined_model_entries", return_value=MODELS), \
         patch.object(adapter, "_run_agent", side_effect=run):
        async with TestClient(TestServer(_create_app(adapter))) as client:
            response = await client.post("/v1/chat/completions", headers={"X-Hermes-Session-Id": "a"}, json={
                "model": "shared-model", "model_selection": public_selection(MODELS[1]),
                "messages": [{"role": "user", "content": "fixture"}],
            })
            assert response.status == 200
    assert db.get_session_model_selection("b")["base_url"].endswith("8111/v1")


@pytest.mark.asyncio
async def test_switch_is_session_scoped_and_does_not_mutate_global_config(adapter):
    db = adapter._ensure_session_db()
    for sid in ["a", "b"]:
        db.create_session(sid, "desktop")
        db.set_session_model_selection(sid, public_selection(MODELS[0]))
    agent = MagicMock()
    adapter._active_chat_agents["a"] = agent
    app = _create_app(adapter)
    app.router.add_patch("/api/sessions/{session_id}", adapter._handle_update_session)
    app.router.add_get("/api/sessions/{session_id}", adapter._handle_get_session)
    with patch.object(adapter, "_combined_model_entries", return_value=MODELS), \
         patch("gateway.session_models.resolve_entry_runtime", return_value={}), \
         patch.object(adapter, "_sync_models_to_config") as global_write:
        async with TestClient(TestServer(app)) as client:
            response = await client.patch("/api/sessions/a", json={"model_selection": public_selection(MODELS[1])})
            assert response.status == 200
            assert (await response.json())["model_switch_status"] == "pending"
            row = await (await client.get("/api/sessions/b")).json()
            assert row["model_selection"]["base_url"].endswith("8111/v1")
            assert "key-a" not in json.dumps(row)
            assert (await client.patch("/api/sessions/a", json={"model_selection": {"model": "missing"}})).status == 400
            assert db.get_session_model_selection("a")["base_url"].endswith("8222/v1")
        global_write.assert_not_called()
        agent.switch_model.assert_not_called()  # the HTTP thread must not swap in-flight clients


def test_create_agent_resolves_saved_endpoint_without_global_provider(adapter):
    db = adapter._ensure_session_db()
    db.create_session("a", "desktop")
    db.set_session_model_selection("a", public_selection(MODELS[1]))
    runtime = {"provider": "custom", "api_key": "key-b", "base_url": MODELS[1]["baseUrl"],
               "api_mode": "chat_completions", "credential_pool": None}
    with patch.object(adapter, "_combined_model_entries", return_value=MODELS), \
         patch("gateway.session_models.resolve_entry_runtime", return_value=runtime) as resolve, \
         patch("gateway.run._resolve_runtime_agent_kwargs", side_effect=AssertionError("global runtime used")), \
         patch("run_agent.AIAgent") as constructor:
        adapter._create_agent(session_id="a", model_name="shared-model")
    assert resolve.call_args.args[0] is MODELS[1]
    assert constructor.call_args.kwargs["api_mode"] == "chat_completions"
    assert constructor.call_args.kwargs["api_key"] == "key-b"
    assert constructor.call_args.kwargs["credential_pool"] is None


def test_failed_or_cancelled_switch_does_not_mutate_runtime_or_prompt():
    from tests.run_agent.test_switch_model_context import _make_agent_with_compressor
    agent = _make_agent_with_compressor(32768)
    agent._interrupt_requested = False
    agent._cached_system_prompt = "stable prefix"
    original_client, original_compressor = agent.client, agent.context_compressor
    runtime = {"provider": "custom", "api_key": "new-key", "base_url": "http://localhost:8222/v1", "api_mode": "chat_completions"}
    with patch("run_agent.AIAgent._create_openai_client", side_effect=RuntimeError("unavailable")):
        with pytest.raises(RuntimeError):
            switch_at_boundary(agent, runtime, "next-model")
    assert agent.model == "primary-model" and agent.api_key == "sk-primary"
    assert agent.client is original_client and agent.context_compressor is original_compressor
    assert agent._cached_system_prompt == "stable prefix"
    def cancel(*args, **kwargs):
        agent._interrupt_requested = True
        return MagicMock()
    with patch("run_agent.AIAgent._create_openai_client", side_effect=cancel), \
         patch("agent.model_metadata.get_model_context_length", return_value=64000):
        switch_at_boundary(agent, runtime, "next-model")
    assert agent.model == "primary-model" and agent._interrupt_requested
