"""Schema-shape tests for database-backed memory recall."""

import json

from tools.memory_tool import MEMORY_SCHEMA


_FORBIDDEN_TOP_LEVEL_KEYS = ("allOf", "anyOf", "oneOf", "enum", "not")


def test_memory_schema_has_no_forbidden_top_level_combinators():
    """OpenAI's Codex backend rejects these at the top level of parameters."""
    params = MEMORY_SCHEMA["parameters"]
    for key in _FORBIDDEN_TOP_LEVEL_KEYS:
        assert key not in params, (
            f"top-level {key!r} in memory tool parameters will break the "
            "Codex backend (chatgpt.com/backend-api/codex). Per-action "
            "required-field checks belong in the runtime handler, not the schema."
        )


def test_memory_schema_is_well_formed():
    params = MEMORY_SCHEMA["parameters"]
    assert params["type"] == "object"
    assert params["required"] == []
    assert params["properties"]["sort"]["enum"] == ["newest", "oldest"]


def test_memory_schema_is_json_serializable():
    json.dumps(MEMORY_SCHEMA)
