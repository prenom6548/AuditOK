#!/usr/bin/env python3
"""Memory Tool - long-term conversation recall from the memory store."""

from tools.registry import registry
from tools.memory_retrieval import retrieve_memory


def memory(
    query: str = "",
    role_filter: str = None,
    limit: int = 3,
    db=None,
    current_session_id: str = None,
    session_id: str = None,
    around_message_id: int = None,
    window: int = 5,
    sort: str = None,
) -> str:
    """Search or browse the dedicated conversation memory database."""
    if db is None:
        try:
            from kylin_agent_runtime_state import MemoryDB

            db = MemoryDB()
        except Exception:
            from tools.registry import tool_error

            return tool_error("Memory database is not available.", success=False)

    return retrieve_memory(
        query=query,
        role_filter=role_filter,
        limit=limit,
        db=db,
        current_session_id=current_session_id,
        session_id=session_id,
        around_message_id=around_message_id,
        window=window,
        sort=sort,
    )


def check_memory_requirements() -> bool:
    """Memory recall only requires the profile directory."""
    try:
        from kylin_agent_runtime_state import DEFAULT_MEMORY_DB_PATH

        return DEFAULT_MEMORY_DB_PATH.parent.exists()
    except ImportError:
        return False


MEMORY_SCHEMA = {
    "name": "memory",
    "description": (
        "Search conversations stored in the dedicated long-term memory database, "
        "or scroll inside one. FTS5-backed retrieval over actual messages. No LLM "
        "calls or summaries are used.\n\n"
        "THREE CALLING SHAPES\n\n"
        "  1) DISCOVERY - pass `query`:\n"
        "     memory(query=\"auth refactor\", limit=3)\n"
        "     Runs FTS5, dedupes hits by conversation lineage, and returns the top N "
        "memories. Each result includes its id, title, time, source, highlighted "
        "snippet, first and last messages, and a message window around the match. "
        "Together these reconstruct goal, match, and resolution without loading the "
        "whole transcript.\n\n"
        "  2) SCROLL - pass `session_id` + `around_message_id`:\n"
        "     memory(session_id=\"...\", around_message_id=12345, window=10)\n"
        "     Returns a window around an anchor from a known memory. To scroll "
        "FORWARD, pass messages[-1].id; to scroll BACKWARD, pass messages[0].id. "
        "When messages_before or messages_after is less than window, that edge of "
        "the memory has been reached.\n\n"
        "  3) BROWSE - no args:\n"
        "     memory()\n"
        "     Lists recent conversation memories with titles, previews, and times. "
        "Each result includes first_message_id and last_message_id; use one of "
        "those exact ids when scrolling, and never invent an anchor id. Use this "
        "for broad recency questions such as what the user worked on yesterday.\n\n"
        "FTS5 SYNTAX\n\n"
        "  AND is the default, so multi-word queries require every term. Use OR for "
        "broader recall, quoted phrases for exact matches, NOT for exclusions, or a "
        "trailing * for prefix matching.\n\n"
        "WHEN TO USE\n\n"
        "  Use memory whenever the user refers to a previous conversation, asks "
        "what was decided or done earlier, or when prior context may prevent them "
        "from having to repeat information. For broad time-based questions, browse "
        "first and scroll into relevant results."
    ),
    "parameters": {
        "type": "object",
        "properties": {
            "query": {
                "type": "string",
                "description": "Text or FTS5 query for discovering conversation memories. Omit to browse recent memories.",
            },
            "limit": {
                "type": "integer",
                "description": "Maximum discovery or browse results (default 3, clamped to 1-10).",
                "default": 3,
            },
            "sort": {
                "type": "string",
                "enum": ["newest", "oldest"],
                "description": "Optional temporal ordering for discovery results.",
            },
            "session_id": {
                "type": "string",
                "description": "Memory session to scroll, taken from a discovery or browse result.",
            },
            "around_message_id": {
                "type": "integer",
                "description": "Exact SQLite message id from a discovery result, browse first_message_id/last_message_id, or a previous scroll window. Never guess this value.",
            },
            "window": {
                "type": "integer",
                "description": "Messages on each side of the scroll anchor (default 5, clamped to 1-20).",
                "default": 5,
            },
            "role_filter": {
                "type": "string",
                "description": "Comma-separated roles to search; discovery defaults to user,assistant.",
            },
        },
        "required": [],
    },
}


registry.register(
    name="memory",
    toolset="memory",
    schema=MEMORY_SCHEMA,
    handler=lambda args, **kw: memory(
        query=args.get("query") or "",
        role_filter=args.get("role_filter"),
        limit=args.get("limit", 3),
        session_id=args.get("session_id"),
        around_message_id=args.get("around_message_id"),
        window=args.get("window", 5),
        sort=args.get("sort"),
        db=kw.get("db"),
        current_session_id=kw.get("current_session_id"),
    ),
    check_fn=check_memory_requirements,
    emoji="🧠",
)
