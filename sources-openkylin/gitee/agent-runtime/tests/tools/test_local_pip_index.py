"""Tests for the default Python package index in terminal subprocesses."""

from unittest.mock import patch

from tools.environments.base import BaseEnvironment, DEFAULT_PIP_INDEX_URL
from tools.environments.local import (
    _make_run_env,
    _sanitize_subprocess_env,
)


class _Environment(BaseEnvironment):
    def _run_bash(self, *args, **kwargs):  # pragma: no cover - not used
        raise NotImplementedError

    def cleanup(self):
        pass


def test_make_run_env_defaults_to_tsinghua_index():
    with patch.dict("os.environ", {}, clear=True):
        result = _make_run_env({})

    assert result["PIP_INDEX_URL"] == DEFAULT_PIP_INDEX_URL


def test_make_run_env_preserves_explicit_index():
    custom = "https://packages.example.test/simple"
    with patch.dict("os.environ", {"PIP_INDEX_URL": custom}, clear=True):
        result = _make_run_env({})

    assert result["PIP_INDEX_URL"] == custom


def test_backend_env_overrides_process_index():
    custom = "https://packages.example.test/backend/simple"
    with patch.dict(
        "os.environ",
        {"PIP_INDEX_URL": "https://process.example.test/simple"},
        clear=True,
    ):
        result = _make_run_env({"PIP_INDEX_URL": custom})

    assert result["PIP_INDEX_URL"] == custom


def test_background_subprocess_env_gets_tsinghua_index():
    result = _sanitize_subprocess_env({}, {})

    assert result["PIP_INDEX_URL"] == DEFAULT_PIP_INDEX_URL


def test_background_subprocess_env_preserves_explicit_index():
    custom = "https://packages.example.test/background/simple"
    result = _sanitize_subprocess_env({"PIP_INDEX_URL": custom}, {})

    assert result["PIP_INDEX_URL"] == custom


def test_wrapped_command_sets_index_for_every_backend():
    env = _Environment(cwd="/workspace", timeout=10)

    wrapped = env._wrap_command("python -m pip install demo", "/workspace")

    export = (
        f'export PIP_INDEX_URL="${{PIP_INDEX_URL:-{DEFAULT_PIP_INDEX_URL}}}"'
    )
    assert export in wrapped
    assert wrapped.index(export) < wrapped.index("pip install demo")
