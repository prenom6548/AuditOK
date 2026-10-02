"""Tests for full/branch/lean system prompt profiles."""

from types import SimpleNamespace
from unittest.mock import patch

from agent.system_prompt import LEAN_AGENT_IDENTITY, build_system_prompt


class _MemoryStore:
    def format_for_system_prompt(self, kind):
        return f"{kind.upper()} MEMORY BLOCK"


class _MemoryManager:
    def build_system_prompt(self):
        return "EXTERNAL MEMORY BLOCK"


class _UserProfileRuntime:
    def profile_prompt(self):
        return "<user_profile_data>PROFILE BLOCK</user_profile_data>"


def _make_agent(profile="lean"):
    return SimpleNamespace(
        prompt_profile=profile,
        pass_session_id=True,
        session_id="session-123",
        model="test-model",
        provider="test-provider",
        _kanban_worker_guidance=None,
        skip_context_files=False,
        load_soul_identity=True,
        valid_tool_names={
            "memory",
            "skills_list",
            "skill_view",
            "skill_manage",
            "todo",
        },
        _tool_use_enforcement=True,
        platform="cli",
        _memory_store=None,
        _memory_enabled=True,
        _user_profile_enabled=True,
        _user_profile_runtime=_UserProfileRuntime(),
        _memory_manager=_MemoryManager(),
    )


def test_lean_prompt_keeps_only_minimal_branch_context():
    prompt = build_system_prompt(
        _make_agent("lean"),
        system_message="Caller scoped instruction.",
    )

    assert LEAN_AGENT_IDENTITY in prompt
    assert "Caller scoped instruction." in prompt
    assert "Session ID: session-123" in prompt
    assert "Model: test-model" in prompt
    assert "Provider: test-provider" in prompt
    assert "Never use Google Search" in prompt
    assert "https://cn.bing.com/" in prompt

    assert "MEMORY MEMORY BLOCK" not in prompt
    assert "USER MEMORY BLOCK" not in prompt
    assert "EXTERNAL MEMORY BLOCK" not in prompt
    assert "available_skills" not in prompt
    assert "SOUL.md" not in prompt
    assert "<task_execution>" not in prompt


def test_full_prompt_replaces_memory_md_with_encrypted_profile():
    prompt = build_system_prompt(_make_agent("full"))

    assert "Never use Google Search" in prompt
    assert prompt.count("Never use Google Search") == 1
    assert "<user_profile_data>PROFILE BLOCK</user_profile_data>" in prompt
    assert "MEMORY MEMORY BLOCK" not in prompt
    assert "USER MEMORY BLOCK" not in prompt
    assert "EXTERNAL MEMORY BLOCK" in prompt
    assert "<task_execution>" in prompt
    assert "do not proactively create subagents" in prompt


def test_branch_prompt_replaces_identity_but_keeps_full_guidance_and_context():
    with (
        patch("run_agent.load_soul_md", return_value="SOUL IDENTITY") as load_soul,
        patch(
            "run_agent.build_context_files_prompt",
            return_value="PROJECT CONTEXT BLOCK",
        ) as build_context,
    ):
        prompt = build_system_prompt(_make_agent("branch"))

    load_soul.assert_not_called()
    assert build_context.call_args.kwargs["skip_soul"] is True
    assert LEAN_AGENT_IDENTITY in prompt
    assert "SOUL IDENTITY" not in prompt
    assert "PROJECT CONTEXT BLOCK" in prompt
    assert "configuring, setting up, or using Kylin Agent" in prompt
    assert "Never use Google Search" in prompt
    assert "<user_profile_data>PROFILE BLOCK</user_profile_data>" in prompt
    assert "<task_execution>" in prompt
    assert "only when the user explicitly requests delegation" in prompt
