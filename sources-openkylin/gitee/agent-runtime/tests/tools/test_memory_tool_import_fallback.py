"""Regression tests for memory-tool import fallbacks."""

import builtins
import importlib
import sys

from tools.registry import registry


def test_memory_tool_imports_without_fcntl(monkeypatch):
    original_import = builtins.__import__

    def fake_import(name, globals=None, locals=None, fromlist=(), level=0):
        if name == "fcntl":
            raise ImportError("simulated missing fcntl")
        return original_import(name, globals, locals, fromlist, level)

    registry.deregister("memory")
    monkeypatch.delitem(sys.modules, "tools.memory_tool", raising=False)
    monkeypatch.setattr(builtins, "__import__", fake_import)

    memory_tool = importlib.import_module("tools.memory_tool")

    assert registry.get_entry("memory") is not None
    assert memory_tool.MEMORY_SCHEMA["name"] == "memory"
