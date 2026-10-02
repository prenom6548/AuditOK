---
sidebar_position: 3
title: "Persistent Memory"
description: "How Hermes Agent remembers across sessions using context files and conversation memory"
---

# Persistent Memory

Hermes Agent has bounded, curated memory plus a schema-driven user profile that persists across sessions. The profile is encrypted on disk and rendered as plaintext Markdown only when constructing the model prompt.

## How It Works

Two files make up the agent's memory:

| File | Purpose | Char Limit |
|------|---------|------------|
| **MEMORY.md** | Agent's personal notes — environment facts, conventions, things learned | 2,200 chars (~800 tokens) |
| **USER.md** | User profile — your preferences, communication style, expectations | 1,375 chars (~500 tokens) |

Both legacy files remain stored in `~/.kylin-agent-runtime/memories/` and can still be managed via the `memory` tool, but neither is injected into the system prompt. Their prompt context is replaced by the automatic profile described below.

### Automatic user profile

At explicit session boundaries, Hermes sends the complete user/assistant transcript to the bundled schema-driven profile extractor. The resulting profile is encrypted under `~/.kylin-agent-runtime/user_profile/`; platform identities are pseudonymized before storage, and system/tool messages are excluded.

At the next session start, Hermes decrypts the profile locally, applies confidence filtering and decay, escapes user-controlled values, and renders a plaintext Markdown `<user_profile_data>` block for the model prompt. The prompt block itself is not encrypted because the model must read it; encryption protects only persisted profile data at rest.

Developers can inspect that encrypted profile directly without exposing an
agent tool:

```bash
python -m agent.user_profile.cli show                 # prompt Markdown
python -m agent.user_profile.cli show --format json   # full decrypted document
```

The command uses the active `HERMES_HOME` and the local-user identity by
default. For a messaging identity, pass both `--platform` and
`--platform-user-id`. Inspection is read-only and does not create a missing
key or profile.

The profile extractor always reuses the active agent's model and client, so it needs no separate model, endpoint, or API key configuration. Profile extraction is best-effort and never prevents a session from closing.

:::info
Character limits keep memory focused. When memory is full, the agent consolidates or replaces entries to make room for new information.
:::

## How Memory Appears in the System Prompt

At the start of every session, memory entries are loaded from disk and rendered into the system prompt as a frozen block:

```
══════════════════════════════════════════════
MEMORY (your personal notes) [67% — 1,474/2,200 chars]
══════════════════════════════════════════════
User's project is a Rust web service at ~/code/myapi using Axum + SQLx
§
This machine runs Ubuntu 22.04, has Docker and Podman installed
§
User prefers concise responses, dislikes verbose explanations
```

The format includes:
- A header showing which store (MEMORY or USER PROFILE)
- Usage percentage and character counts so the agent knows capacity
- Individual entries separated by `§` (section sign) delimiters
- Entries can be multiline

**Frozen snapshot pattern:** The system prompt injection is captured once at session start and never changes mid-session. This is intentional — it preserves the LLM's prefix cache for performance. When the agent adds/removes memory entries during a session, the changes are persisted to disk immediately but won't appear in the system prompt until the next session starts. Tool responses always show the live state.

## Memory Tool Actions

The agent uses the `memory` tool with these actions:

- **add** — Add a new memory entry
- **replace** — Replace an existing entry with updated content (uses substring matching via `old_text`)
- **remove** — Remove an entry that's no longer relevant (uses substring matching via `old_text`)

There is no `read` action — memory content is automatically injected into the system prompt at session start. The agent sees its memories as part of its conversation context.

### Substring Matching

The `replace` and `remove` actions use short unique substring matching — you don't need the full entry text. The `old_text` parameter just needs to be a unique substring that identifies exactly one entry:

```python
# If memory contains "User prefers dark mode in all editors"
memory(action="replace", target="memory",
       old_text="dark mode",
       content="User prefers light mode in VS Code, dark mode in terminal")
```

If the substring matches multiple entries, an error is returned asking for a more specific match.

## Two Targets Explained

### `memory` — Agent's Personal Notes

For information the agent needs to remember about the environment, workflows, and lessons learned:

- Environment facts (OS, tools, project structure)
- Project conventions and configuration
- Tool quirks and workarounds discovered
- Completed task diary entries
- Skills and techniques that worked

### `user` — User Profile

For information about the user's identity, preferences, and communication style:

- Name, role, timezone
- Communication preferences (concise vs detailed, format preferences)
- Pet peeves and things to avoid
- Workflow habits
- Technical skill level

## What to Save vs Skip

### Save These (Proactively)

The agent saves automatically — you don't need to ask. It saves when it learns:

- **User preferences:** "I prefer TypeScript over JavaScript" → save to `user`
- **Environment facts:** "This server runs Debian 12 with PostgreSQL 16" → save to `memory`
- **Corrections:** "Don't use `sudo` for Docker commands, user is in docker group" → save to `memory`
- **Conventions:** "Project uses tabs, 120-char line width, Google-style docstrings" → save to `memory`
- **Completed work:** "Migrated database from MySQL to PostgreSQL on 2026-01-15" → save to `memory`
- **Explicit requests:** "Remember that my API key rotation happens monthly" → save to `memory`

### Skip These

- **Trivial/obvious info:** "User asked about Python" — too vague to be useful
- **Easily re-discovered facts:** "Python 3.12 supports f-string nesting" — can web search this
- **Raw data dumps:** Large code blocks, log files, data tables — too big for memory
- **Session-specific ephemera:** Temporary file paths, one-off debugging context
- **Information already in context files:** SOUL.md and AGENTS.md content

## Capacity Management

Memory has strict character limits to keep system prompts bounded:

| Store | Limit | Typical entries |
|-------|-------|----------------|
| memory | 2,200 chars | 8-15 entries |
| user | 1,375 chars | 5-10 entries |

### What Happens When Memory is Full

When you try to add an entry that would exceed the limit, the tool returns an error:

```json
{
  "success": false,
  "error": "Memory at 2,100/2,200 chars. Adding this entry (250 chars) would exceed the limit. Replace or remove existing entries first.",
  "current_entries": ["..."],
  "usage": "2,100/2,200"
}
```

The agent should then:
1. Read the current entries (shown in the error response)
2. Identify entries that can be removed or consolidated
3. Use `replace` to merge related entries into shorter versions
4. Then `add` the new entry

**Best practice:** When memory is above 80% capacity (visible in the system prompt header), consolidate entries before adding new ones. For example, merge three separate "project uses X" entries into one comprehensive project description entry.

### Practical Examples of Good Memory Entries

**Compact, information-dense entries work best:**

```
# Good: Packs multiple related facts
User runs macOS 14 Sonoma, uses Homebrew, has Docker Desktop and Podman. Shell: zsh with oh-my-zsh. Editor: VS Code with Vim keybindings.

# Good: Specific, actionable convention
Project ~/code/api uses Go 1.22, sqlc for DB queries, chi router. Run tests with 'make test'. CI via GitHub Actions.

# Good: Lesson learned with context
The staging server (10.0.1.50) needs SSH port 2222, not 22. Key is at ~/.ssh/staging_ed25519.

# Bad: Too vague
User has a project.

# Bad: Too verbose
On January 5th, 2026, the user asked me to look at their project which is
located at ~/code/api. I discovered it uses Go version 1.22 and...
```

## Duplicate Prevention

The memory system automatically rejects exact duplicate entries. If you try to add content that already exists, it returns success with a "no duplicate added" message.

## Security Scanning

Memory entries are scanned for injection and exfiltration patterns before being accepted, since they're injected into the system prompt. Content matching threat patterns (prompt injection, credential exfiltration, SSH backdoors) or containing invisible Unicode characters is blocked.

## Conversation Memory Retrieval

Beyond MEMORY.md and USER.md context, the agent can search past conversations using the `memory` tool:

- Conversations are mirrored into a dedicated SQLite archive (`~/.kylin-agent-runtime/memory.db`) with FTS5 full-text search
- Search queries return actual messages from the DB — no LLM summarization, no truncation
- The agent can find things it discussed weeks ago, even if they're not in its active memory
- The agent can also scroll forward/backward inside any session it finds

```bash
kylin-agent-runtime sessions list    # Browse past sessions
```

See [Memory Tool](/docs/user-guide/sessions#memory-tool) for the three calling shapes (discovery / scroll / browse) and the response format.

### Context files vs conversation memory

| Feature | MEMORY.md / USER.md context | `memory` tool |
|---------|------------------|----------------|
| **Capacity** | ~1,300 tokens total | Unlimited (all sessions) |
| **Speed** | Instant (in system prompt) | ~20ms FTS5 query, ~1ms scroll |
| **Cost** | Token cost in every prompt | Free — no LLM calls |
| **Use case** | Key facts always available | Finding specific past conversations |
| **Management** | Curated context files/providers | Automatic conversation mirroring |
| **Token cost** | Fixed per session (~1,300 tokens) | On-demand (searched when needed) |

Context files are for critical facts that should always be in context. The `memory` tool is for "did we discuss X last week?" queries where the agent needs to recall specifics from past conversations.

## Configuration

```yaml
# In ~/.kylin-agent-runtime/config.yaml
memory:
  memory_enabled: true
  user_profile_enabled: true
  memory_char_limit: 2200   # ~800 tokens
  user_char_limit: 1375     # ~500 tokens

user_profile:
  enabled: true
  prompt_max_chars: 4000
  min_confidence: 0.5
  precheck_enabled: true  # skip extraction for sessions with no profile value
  max_attempts: 3         # total structured extraction attempts
  retry_base_delay_seconds: 1
```

The precheck sends the complete existing profile and session to the active
model in a short yes/no request. It skips structured extraction when the
session only repeats existing information and the model clearly answers
`否`/`no`; errors and unrecognized answers continue with extraction.
Structured extraction failures retry with exponential backoff. The classic CLI
and Ink TUI report each retry reason and the final success or failure; messaging
frontends remain silent.

## External Memory Providers

For deeper, persistent memory that goes beyond MEMORY.md and USER.md, Hermes ships with 8 external memory provider plugins — including Honcho, OpenViking, Mem0, Hindsight, Holographic, RetainDB, ByteRover, and Supermemory.

External providers run **alongside** built-in memory (never replacing it) and add capabilities like knowledge graphs, semantic search, automatic fact extraction, and cross-session user modeling.

```bash
kylin-agent-runtime memory setup      # pick a provider and configure it
kylin-agent-runtime memory status     # check what's active
```

See the [Memory Providers](./memory-providers.md) guide for full details on each provider, setup instructions, and comparison.
