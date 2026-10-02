import json
import os
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import MagicMock

import kylin_agent_runtime_cli.plugins as plugins_mod
import tools.terminal_tool as terminal_tool_module


_UNSET = object()


def _make_env_config(tmp_path, **overrides):
    config = {
        "env_type": "local",
        "timeout": 30,
        "cwd": str(tmp_path),
        "host_cwd": None,
        "modal_mode": "auto",
        "docker_image": "",
        "singularity_image": "",
        "modal_image": "",
        "daytona_image": "",
    }
    config.update(overrides)
    return config


def _run_terminal(
    monkeypatch,
    tmp_path,
    *,
    output,
    returncode=0,
    invoke_hook=_UNSET,
    approval=None,
    command="echo hello",
):
    mock_env = MagicMock()
    mock_env.execute.return_value = {"output": output, "returncode": returncode}

    monkeypatch.setattr(
        terminal_tool_module, "_get_env_config", lambda: _make_env_config(tmp_path)
    )
    monkeypatch.setattr(terminal_tool_module, "_start_cleanup_thread", lambda: None)
    monkeypatch.setattr(
        terminal_tool_module,
        "_check_all_guards",
        lambda *_args, **_kwargs: approval or {"approved": True},
    )
    monkeypatch.setitem(terminal_tool_module._active_environments, "default", mock_env)
    monkeypatch.setitem(terminal_tool_module._last_activity, "default", 0.0)

    if invoke_hook is not _UNSET:
        monkeypatch.setattr("kylin_agent_runtime_cli.plugins.invoke_hook", invoke_hook)

    result = json.loads(terminal_tool_module.terminal_tool(command=command))
    return result, mock_env


def test_terminal_output_unchanged_when_transform_hook_not_registered(monkeypatch, tmp_path):
    result, _mock_env = _run_terminal(monkeypatch, tmp_path, output="plain output")

    assert result["output"] == "plain output"
    assert result["exit_code"] == 0
    assert result["error"] is None


def test_terminal_output_unchanged_for_none_hook_result(monkeypatch, tmp_path):
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="plain output",
        invoke_hook=lambda hook_name, **kwargs: [None],
    )

    assert result["output"] == "plain output"


def test_terminal_output_ignores_invalid_hook_results(monkeypatch, tmp_path):
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="plain output",
        invoke_hook=lambda hook_name, **kwargs: [{"bad": True}, 123, ["nope"]],
    )

    assert result["output"] == "plain output"


def test_terminal_output_uses_first_valid_string_from_hooks(monkeypatch, tmp_path):
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="plain output",
        invoke_hook=lambda hook_name, **kwargs: [None, {"bad": True}, "first", "second"],
    )

    assert result["output"] == "first"


def test_terminal_output_transform_still_truncates_long_replacement(monkeypatch, tmp_path):
    transformed_output = "PLUGIN-HEAD\n" + ("A" * 60000) + "\nPLUGIN-TAIL"
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="short output",
        invoke_hook=lambda hook_name, **kwargs: [transformed_output],
    )

    assert "PLUGIN-HEAD" in result["output"]
    assert "PLUGIN-TAIL" in result["output"]
    assert "[OUTPUT TRUNCATED" in result["output"]
    assert transformed_output != result["output"]


def test_terminal_output_transform_still_runs_strip_and_redact(monkeypatch, tmp_path):
    # Ensure redaction is active regardless of host HERMES_REDACT_SECRETS state
    # or collection-time import order (the module snapshots env at import).
    monkeypatch.setattr("agent.redact._REDACT_ENABLED", True)

    secret = "sk-proj-abc123def456ghi789jkl012mno345"
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="plain output",
        invoke_hook=lambda hook_name, **kwargs: [f" \x1b[31mOPENAI_API_KEY={secret}\x1b[0m "],
    )

    assert "\x1b" not in result["output"]
    assert secret not in result["output"]
    assert "OPENAI_API_KEY=" in result["output"]
    assert "***" in result["output"]


def test_terminal_output_transform_hook_exception_falls_back(monkeypatch, tmp_path):
    def _raise(*_args, **_kwargs):
        raise RuntimeError("boom")

    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="plain output",
        invoke_hook=_raise,
    )

    assert result["output"] == "plain output"
    assert result["exit_code"] == 0
    assert result["error"] is None


def test_terminal_output_transform_does_not_change_approval_or_exit_code_meaning(monkeypatch, tmp_path):
    approval = {
        "approved": True,
        "user_approved": True,
        "description": "dangerous command",
    }
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="original output",
        returncode=1,
        approval=approval,
        command="grep foo bar",
        invoke_hook=lambda hook_name, **kwargs: ["replaced output"],
    )

    assert result["output"] == "replaced output"
    assert result["approval"] == (
        "Command required approval (dangerous command) and was approved by the user."
    )
    assert result["exit_code_meaning"] == "No matches found (not an error)"


def test_terminal_output_transform_integration_with_real_plugin(monkeypatch, tmp_path):
    import yaml

    hermes_home = Path(os.environ["HERMES_HOME"])
    plugins_dir = hermes_home / "plugins"
    plugin_dir = plugins_dir / "terminal_transform"
    plugin_dir.mkdir(parents=True)
    (plugin_dir / "plugin.yaml").write_text("name: terminal_transform\n", encoding="utf-8")
    (plugin_dir / "__init__.py").write_text(
        "def register(ctx):\n"
        '    ctx.register_hook("transform_terminal_output", '
        'lambda **kw: "PLUGIN-HEAD\\n" + kw["output"] + "\\nPLUGIN-TAIL")\n',
        encoding="utf-8",
    )
    # Plugins are opt-in — must be listed in plugins.enabled to load.
    cfg_path = hermes_home / "config.yaml"
    cfg_path.write_text(
        yaml.safe_dump({"plugins": {"enabled": ["terminal_transform"]}}),
        encoding="utf-8",
    )

    # Force a fresh plugin manager so the new config is picked up.
    plugins_mod._plugin_manager = plugins_mod.PluginManager()
    plugins_mod.discover_plugins()

    long_output = "X" * 60000
    result, _mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output=long_output,
    )

    assert "PLUGIN-HEAD" in result["output"]
    assert "PLUGIN-TAIL" in result["output"]
    assert "[OUTPUT TRUNCATED" in result["output"]


def test_openkylin_normal_mode_switches_to_maintenance_before_sudo_install(
    monkeypatch, tmp_path
):
    monkeypatch.setattr(
        terminal_tool_module,
        "_openkylin_install_needs_maintenance",
        lambda command, env_type: True,
    )
    result, mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="maintenance mode scheduled",
        command="sudo apt install -y ffmpeg",
    )

    executed_command = mock_env.execute.call_args.args[0]
    assert executed_command == "sudo mm-cli -o"
    assert result["status"] == "reboot_required"
    assert result["reboot_required"] is True
    assert result["original_install_skipped"] is True
    assert "请立即重启系统" in result["output"]


def test_openkylin_maintenance_mode_keeps_original_install(monkeypatch, tmp_path):
    monkeypatch.setattr(
        terminal_tool_module,
        "_openkylin_install_needs_maintenance",
        lambda command, env_type: False,
    )
    result, mock_env = _run_terminal(
        monkeypatch,
        tmp_path,
        output="installed",
        command="sudo apt install -y ffmpeg",
    )

    assert mock_env.execute.call_args.args[0] == "sudo apt install -y ffmpeg"
    assert result["output"] == "installed"
    assert "reboot_required" not in result


def test_openkylin_mode_probe_only_blocks_normal_mode(monkeypatch):
    monkeypatch.setattr(terminal_tool_module, "_is_openkylin_host", lambda: True)
    monkeypatch.setattr(terminal_tool_module.shutil, "which", lambda name: "/usr/bin/mm-cli")

    monkeypatch.setattr(
        terminal_tool_module.subprocess,
        "run",
        lambda *_args, **_kwargs: SimpleNamespace(
            returncode=0, stdout="Normal Mode\n", stderr=""
        ),
    )
    assert terminal_tool_module._openkylin_install_needs_maintenance(
        "sudo apt install ffmpeg", "local"
    ) is True

    monkeypatch.setattr(
        terminal_tool_module.subprocess,
        "run",
        lambda *_args, **_kwargs: SimpleNamespace(
            returncode=0, stdout="Maintain Mode\n", stderr=""
        ),
    )
    assert terminal_tool_module._openkylin_install_needs_maintenance(
        "sudo apt install ffmpeg", "local"
    ) is False
