from __future__ import annotations

import json
from types import SimpleNamespace
from unittest.mock import MagicMock

import pytest

from agent.user_profile.models import ProfileEntry, ProfileUpdate, UserProfile
from agent.user_profile import InteractionMessage, OpenAICompatibleProfileExtractor
from agent.user_profile_runtime import (
    RuntimeUserProfile,
    _error_summary,
    _extractor_for_agent,
    _interaction_messages,
    commit_user_profile_session,
    initialize_user_profile,
)
from agent.user_profile.extractors import _parse_precheck_answer


def test_initialize_profile_uses_hermes_home_and_pseudonymous_identity(tmp_path, monkeypatch):
    monkeypatch.setenv("HERMES_HOME", str(tmp_path))
    agent = SimpleNamespace(platform="telegram", _user_id="raw-platform-user", client=None)

    runtime = initialize_user_profile(agent, {})

    assert runtime is not None
    assert runtime.user_id.startswith("v1_")
    assert "raw-platform-user" not in runtime.user_id
    assert (tmp_path / "user_profile" / "profile.key").stat().st_mode & 0o777 == 0o600
    prompt = runtime.profile_prompt()
    assert "<user_profile_data>" in prompt
    assert list((tmp_path / "user_profile" / "profiles").glob("*.profile.enc"))


def test_local_frontends_share_one_profile_identity(tmp_path, monkeypatch):
    monkeypatch.setenv("HERMES_HOME", str(tmp_path))
    cli = initialize_user_profile(SimpleNamespace(platform="cli", _user_id=None, client=None), {})
    tui = initialize_user_profile(SimpleNamespace(platform="tui", _user_id=None, client=None), {})

    assert cli is not None and tui is not None
    assert cli.user_id == tui.user_id


def test_extractor_reuses_active_agent_model_and_client():
    client = SimpleNamespace(
        chat=SimpleNamespace(completions=MagicMock()),
        responses=SimpleNamespace(),
    )
    agent = SimpleNamespace(
        client=client,
        model="runtime-model",
        api_mode="chat_completions",
    )

    extractor = _extractor_for_agent(agent)

    assert extractor is not None
    assert extractor.client is client
    assert extractor.model == "runtime-model"
    assert extractor.api_mode == "chat_completions"


def test_commit_extracts_each_session_once():
    runtime = MagicMock()
    runtime.observe.return_value = True
    agent = SimpleNamespace(
        session_id="session-1",
        _user_profile_runtime=runtime,
        _user_profile_committed_sessions={},
    )
    messages = [{"role": "user", "content": "hello"}]

    commit_user_profile_session(agent, messages)
    commit_user_profile_session(agent, messages)

    runtime.observe.assert_called_once_with(messages)


def test_resumed_session_with_new_messages_is_extracted_again():
    runtime = MagicMock()
    runtime.observe.return_value = True
    agent = SimpleNamespace(
        session_id="session-1",
        _user_profile_runtime=runtime,
        _user_profile_committed_sessions={},
    )

    commit_user_profile_session(agent, [{"role": "user", "content": "first"}])
    resumed = [
        {"role": "user", "content": "first"},
        {"role": "assistant", "content": "reply"},
        {"role": "user", "content": "new fact"},
    ]
    commit_user_profile_session(agent, resumed)

    assert runtime.observe.call_count == 2
    runtime.observe.assert_called_with(resumed)


def test_message_conversion_keeps_only_conversation_text():
    messages = [
        {"role": "system", "content": "secret system prompt"},
        {"role": "user", "content": [{"type": "text", "text": "hello"}, {"type": "image_url"}]},
        {"role": "assistant", "content": "hi"},
        {"role": "tool", "content": "private tool output"},
    ]

    converted = _interaction_messages(messages)

    assert [(item.role, item.content) for item in converted] == [
        ("user", "hello"),
        ("assistant", "hi"),
    ]


def test_runtime_observe_skips_cleanly_without_extractor():
    service = MagicMock(extractor=None)
    runtime = RuntimeUserProfile(service, "user")

    assert runtime.observe([{"role": "user", "content": "hello"}]) is False
    service.observe.assert_not_called()


def test_runtime_observe_skips_structured_extraction_when_precheck_says_no():
    extractor = SimpleNamespace(
        model="profile-test-model", should_extract=MagicMock(return_value=False)
    )
    service = MagicMock(extractor=extractor)
    current_profile = UserProfile(user_id="user")
    service.get_or_create.return_value = current_profile
    runtime = RuntimeUserProfile(service, "user")

    assert runtime.observe([{"role": "user", "content": "谢谢"}]) is True

    assert extractor.should_extract.call_args.args[2] is current_profile
    service.observe.assert_not_called()


def test_runtime_observe_extracts_when_precheck_is_unknown_or_fails():
    result = SimpleNamespace(applied=[], deleted=[], conflicts=[], rejected=[])
    for precheck in (MagicMock(return_value=None), MagicMock(side_effect=RuntimeError("down"))):
        extractor = SimpleNamespace(model="profile-test-model", should_extract=precheck)
        service = MagicMock(extractor=extractor)
        service.observe.return_value = result
        runtime = RuntimeUserProfile(service, "user")

        assert runtime.observe([{"role": "user", "content": "请帮我查一下天气"}]) is True
        service.observe.assert_called_once()


@pytest.mark.parametrize(
    ("answer", "expected"),
    [("是", True), ("YES\n", True), ("否。", False), ("NO", False), ("可能", None)],
)
def test_parse_precheck_answer_is_strict(answer, expected):
    assert _parse_precheck_answer(answer) is expected


def test_extractor_precheck_uses_one_output_token():
    create = MagicMock(
        return_value=SimpleNamespace(
            choices=[SimpleNamespace(message=SimpleNamespace(content="否"))]
        )
    )
    client = SimpleNamespace(chat=SimpleNamespace(completions=SimpleNamespace(create=create)))
    extractor = OpenAICompatibleProfileExtractor(
        model="profile-test-model", client=client, api_mode="chat_completions"
    )
    profile = UserProfile(
        user_id="user",
        entries={
            "basic.preferred_name": ProfileEntry(
                value="小王",
                confidence=1.0,
                source="explicit",
                lifecycle="static",
            )
        },
    )

    assert extractor.should_extract(
        "user", [InteractionMessage(role="user", content="我还是叫小王")], profile
    ) is False
    assert create.call_args.kwargs["max_tokens"] == 1
    payload = json.loads(create.call_args.kwargs["messages"][1]["content"])
    assert payload["current_profile"]["basic.preferred_name"] == {
        "value": "小王",
        "source": "explicit",
        "confidence": 1.0,
    }


def test_extractor_renders_json_examples_without_treating_braces_as_fields():
    create = MagicMock(
        return_value=SimpleNamespace(
            choices=[SimpleNamespace(message=SimpleNamespace(content='{"updates": []}'))]
        )
    )
    client = SimpleNamespace(chat=SimpleNamespace(completions=SimpleNamespace(create=create)))
    extractor = OpenAICompatibleProfileExtractor(
        model="profile-test-model", client=client, api_mode="chat_completions"
    )

    result = extractor.extract(
        "user",
        [InteractionMessage(role="user", content="删除这个")],
        UserProfile(user_id="user"),
    )

    assert result.updates == []
    instructions = create.call_args.kwargs["messages"][0]["content"]
    assert '{\n action:"delete",\n explicit:true\n}' in instructions
    assert '"action":"upsert|delete"' in instructions
    assert "{schema}" not in instructions


def test_legacy_profile_fields_remain_readable_but_cannot_be_new_updates(
    tmp_path, monkeypatch
):
    monkeypatch.setenv("HERMES_HOME", str(tmp_path))
    runtime = initialize_user_profile(
        SimpleNamespace(platform="cli", _user_id=None, client=None), {}
    )
    profile = runtime.service.get_or_create(runtime.user_id)
    profile.entries["context.current_task"] = ProfileEntry(
        value=["调研智能体记忆"],
        confidence=1.0,
        source="explicit",
        lifecycle="dynamic",
    )
    runtime.service.store.save(profile)

    assert "调研智能体记忆" in runtime.profile_prompt()
    with pytest.raises(ValueError, match="unsupported profile path"):
        ProfileUpdate(
            path="context.current_task",
            value=["新任务"],
            confidence=1.0,
            explicit=True,
            evidence_quote="新任务",
        )


def test_runtime_observe_logs_real_extractor_call(caplog):
    result = SimpleNamespace(
        applied=["basic.preferred_name"],
        deleted=[],
        conflicts=[],
        rejected=[],
    )
    extractor = SimpleNamespace(model="profile-test-model")
    service = MagicMock(extractor=extractor)
    service.observe.return_value = result
    runtime = RuntimeUserProfile(service, "user")

    with caplog.at_level("INFO", logger="agent.user_profile_runtime"):
        assert runtime.observe([{"role": "user", "content": "叫我小王"}]) is True

    service.observe.assert_called_once()
    assert "extraction LLM call starting" in caplog.text
    assert "model=profile-test-model" in caplog.text
    assert "extraction LLM call completed: applied=1" in caplog.text


def test_runtime_observe_retries_then_reports_success():
    result = SimpleNamespace(
        applied=["basic.preferred_name"],
        deleted=[],
        conflicts=[],
        rejected=[],
    )
    extractor = SimpleNamespace(model="profile-test-model")
    service = MagicMock(extractor=extractor)
    service.observe.side_effect = [
        RuntimeError("provider temporarily unavailable"),
        RuntimeError("invalid profile JSON"),
        result,
    ]
    events = []
    runtime = RuntimeUserProfile(
        service,
        "user",
        max_attempts=3,
        retry_base_delay_seconds=0,
    )

    assert runtime.observe(
        [{"role": "user", "content": "叫我小王"}],
        status_callback=events.append,
    ) is True

    assert service.observe.call_count == 3
    assert [event["status"] for event in events] == [
        "retrying",
        "retrying",
        "success",
    ]
    assert events[-1] == {
        "status": "success",
        "attempts": 3,
        "applied": 1,
        "deleted": 0,
        "conflicts": 0,
        "rejected": 0,
    }


def test_failed_extraction_is_not_marked_committed_and_reports_cli_reason():
    extractor = SimpleNamespace(model="profile-test-model")
    service = MagicMock(extractor=extractor)
    service.observe.side_effect = RuntimeError("upstream 503")
    runtime = RuntimeUserProfile(
        service,
        "user",
        max_attempts=2,
        retry_base_delay_seconds=0,
    )
    emitted = []
    agent = SimpleNamespace(
        session_id="session-1",
        platform="cli",
        _emit_status=emitted.append,
        _user_profile_runtime=runtime,
        _user_profile_committed_sessions={},
    )
    messages = [{"role": "user", "content": "我喜欢简洁回答"}]

    outcome = commit_user_profile_session(agent, messages)

    assert outcome == {
        "status": "failed",
        "error": "upstream 503",
        "attempts": 2,
    }
    assert service.observe.call_count == 2
    assert agent._user_profile_committed_sessions == {}
    assert emitted == [
        "User profile extraction failed (attempt 1/2): upstream 503. "
        "Retrying in 0.0s...",
        "User profile extraction failed after 2 attempts: upstream 503",
    ]


def test_successful_commit_reports_update_summary_only_to_terminal_frontends():
    result = SimpleNamespace(
        applied=["basic.preferred_name", "preferences.answer_style"],
        deleted=["interests.short_term"],
        conflicts=[],
        rejected=[],
    )
    messages = [{"role": "user", "content": "叫我小王，回答简洁一些"}]

    def make_agent(platform):
        runtime = RuntimeUserProfile(
            MagicMock(
                extractor=SimpleNamespace(model="profile-test-model"),
                observe=MagicMock(return_value=result),
            ),
            "user",
            retry_base_delay_seconds=0,
        )
        return SimpleNamespace(
            session_id=f"{platform}-session",
            platform=platform,
            _emit_status=MagicMock(),
            status_callback=MagicMock(),
            _user_profile_runtime=runtime,
            _user_profile_committed_sessions={},
        )

    cli_agent = make_agent("cli")
    tui_agent = make_agent("tui")
    gateway_agent = make_agent("telegram")
    commit_user_profile_session(cli_agent, messages)
    commit_user_profile_session(tui_agent, messages)
    commit_user_profile_session(gateway_agent, messages)

    cli_agent._emit_status.assert_called_once_with(
        "User profile updated: 2 added or changed, 1 deleted."
    )
    tui_agent.status_callback.assert_called_once_with(
        "lifecycle", "User profile updated: 2 added or changed, 1 deleted."
    )
    tui_agent._emit_status.assert_not_called()
    gateway_agent._emit_status.assert_not_called()
    gateway_agent.status_callback.assert_not_called()


def test_profile_error_summary_redacts_credentials():
    summary = _error_summary(
        RuntimeError(
            "request failed Authorization: Bearer sk-test-1234567890abcdef"
        )
    )

    assert summary == "request failed Authorization: Bearer ***"
    assert "sk-test" not in summary
