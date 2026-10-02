---
name: kylin-agent
description: Configure, extend, or troubleshoot Kylin Agent Runtime.
version: 3.0.0
author: Teknium + Hermes Agent
license: MIT
platforms: [linux, macos, windows]
metadata:
  hermes:
    tags: [kylin, setup, configuration, multi-agent, memory, cli, gateway, development]
    category: autonomous-ai-agents
    homepage: https://gitee.com/openkylin/agent-runtime
---

# Kylin Agent Skill

Kylin Agent Runtime is a self-improving agent runtime built on Hermes, with improved trunk and branch collaboration, stronger memory, self-evolution constraints, and detailed process visibility. This skill covers installation, configuration, operation, troubleshooting, and extension of that runtime; it does not apply to unrelated agent products.

## When to Use

- Set up or update Kylin Agent Runtime and configure models, tools, skills, memory, or messaging platforms.
- Diagnose CLI, TUI, dashboard, gateway, session, or profile problems.
- Choose between a session branch, delegated work, an independent process, and a durable job.
- Extend the runtime through plugins or contribute changes to this repository.

## Prerequisites

- Use `terminal` for runtime commands, `read_file` for local documentation, `search_files` to locate implementations, and `patch` for targeted edits.
- Installation needs Git, `uv`, and Python 3.11 or later. Model execution also needs a configured provider and credentials.
- Run source-related commands from the actual checkout. The README clones into `agent-runtime/`; the checkout is not necessarily under the runtime's home directory.
- Shell examples below use Linux/macOS/WSL syntax. On native Windows, activate `.venv\Scripts\Activate.ps1` in PowerShell and use `kylin-agent-runtime gateway --help` for available service controls. The `systemctl` examples apply only to Linux with systemd.

## How to Run

Load this skill with `/skill kylin-agent`, or preload it with `kylin-agent-runtime -s kylin-agent`.
The skill name is `kylin-agent`; the installed CLI executable is `kylin-agent-runtime`.

Use the current checkout's `README.md`, `README.zh-CN.md`, and applicable `AGENTS.md` as the source of truth. Consult `kylin-agent-runtime --help`, subcommand help, and in-session `/help` for options before relying on older Hermes documentation.

For a new installation, use `terminal` to follow the README's editable-install workflow:

```bash
git clone https://gitee.com/openkylin/agent-runtime.git
cd agent-runtime
uv venv .venv --python 3.11
source .venv/bin/activate
uv pip install -e ".[all]"
kylin-agent-runtime setup
kylin-agent-runtime
```

If `uv` is missing, use the installation instructions in the README appropriate to the host. Reuse an existing checkout and virtual environment when present.

## Quick Reference

| Task | Command or source |
|------|-------------------|
| Interactive CLI | `kylin-agent-runtime` |
| Modern TUI | `kylin-agent-runtime --tui` |
| One task, final result only | `kylin-agent-runtime -z "Analyze the current project"` |
| Web dashboard | `kylin-agent-runtime dashboard` (default port 9119) |
| Setup / model selection | `kylin-agent-runtime setup` / `kylin-agent-runtime model` |
| Health / component status | `kylin-agent-runtime doctor` / `kylin-agent-runtime status` |
| Config / secrets file locations | `kylin-agent-runtime config path` / `kylin-agent-runtime config env-path` |
| Inspect / edit settings | `kylin-agent-runtime config` / `kylin-agent-runtime config edit` |
| Change a setting | `kylin-agent-runtime config set section.key value` |
| Tool availability | `kylin-agent-runtime tools list` / `kylin-agent-runtime tools` |
| Skill availability | `kylin-agent-runtime skills list` / `kylin-agent-runtime skills config` |
| Memory provider | `kylin-agent-runtime memory status` / `kylin-agent-runtime memory setup` |
| Profiles | `kylin-agent-runtime profile list` / `kylin-agent-runtime -p NAME` |
| Sessions | `kylin-agent-runtime sessions list` / `kylin-agent-runtime --resume SESSION` |
| MCP / plugins | `kylin-agent-runtime mcp list` / `kylin-agent-runtime plugins list` |
| Messaging gateway | `kylin-agent-runtime gateway setup` / `kylin-agent-runtime gateway run` |
| Logs | `kylin-agent-runtime logs --level WARNING` |
| In-session help | `/help`; registry: `kylin_agent_runtime_cli/commands.py` |

## Procedure

### 1. Identify the installation and active profile

Establish the checkout, virtual environment, runtime version, execution surface, and profile before diagnosing or changing anything. Use `kylin-agent-runtime --version` and the config path commands above.

The default state directory is `~/.kylin-agent-runtime/`; named profiles normally use `~/.kylin-agent-runtime/profiles/<name>/`. `HERMES_HOME` remains the supported override. Resolve the active profile instead of treating the default path as universal.

| Relative to the active runtime home | Purpose |
|------------------------------------|---------|
| `config.yaml` | Non-secret settings |
| `.env` | API keys and other secrets |
| `skills/`, `plugins/` | Installed extensions |
| `state.db` | Canonical SQLite session store with FTS search |
| `sessions/` | Gateway routing data and transcript artifacts |
| `logs/` | Agent, error, and gateway logs |
| `auth.json` | Stored authentication material |

For code, use `get_hermes_home()` and `display_hermes_home()` from `kylin_agent_runtime_constants`. `HERMES_HOME`, other supported `HERMES_*` variables, and `metadata.hermes` are compatibility identifiers; do not invent renamed alternatives.

### 2. Configure the requested feature

Use `setup`, `model`, `tools`, `skills config`, or the relevant feature's CLI. Check settings in `kylin_agent_runtime_cli/config.py` and implementation-specific loaders before changing them. Non-secret settings belong in `config.yaml`; credentials belong in `.env` or the provider's authentication flow.

Inspect the relevant local documentation with `read_file` when more detail is needed:

| Area | Path relative to the checkout |
|------|------------------------------|
| Configuration and profiles | `website/docs/user-guide/configuration.md`, `website/docs/user-guide/profiles.md` |
| CLI and tools | `website/docs/reference/cli-commands.md`, `website/docs/reference/tools-reference.md` |
| Provider availability | `plugins/model-providers/`, `providers/__init__.py` |
| Memory and learning | `website/docs/user-guide/features/memory.md`, `website/docs/user-guide/features/curator.md` |
| Messaging platforms | `website/docs/user-guide/messaging/`, `gateway/platforms/` |
| Voice | `stt` and `tts` settings in `kylin_agent_runtime_cli/config.py` |
| MCP and plugins | `website/docs/user-guide/features/mcp.md`, `kylin_agent_runtime_cli/plugins.py` |

Some inherited documentation still describes Hermes. Confirm commands and defaults against the local implementation. After tool or skill changes, use a new session (`/new` or `/reset`) when required; `/reload-skills` refreshes skill discovery. Restart the affected CLI or gateway process for settings captured at startup.

### 3. Select the appropriate collaboration or background mechanism

| Need | Mechanism |
|------|-----------|
| Explore another direction from the current conversation | `/branch` or `/fork`; inspect `/help` for local behavior |
| A bounded subtask within the current run | `delegate_task`, with a concrete goal and relevant context |
| An independent agent session | A separate `kylin-agent-runtime` process; select a profile for state isolation |
| Parallel edits to one repository | `kylin-agent-runtime -w` for a separate worktree |
| Scheduled work that survives the current turn | `cronjob` or `kylin-agent-runtime cron` |
| Durable work shared by profiles and workers | `kylin-agent-runtime kanban` |
| Maintenance of agent-created skills | `kylin-agent-runtime curator` |

Check `delegation.*` settings for budgets and available roles. Delegation waits for child results and is not a durable queue. Background processes use `terminal` with `background=true` and, when needed, `notify_on_complete=true`; interactive CLI processes require a terminal. Choose the mechanism required by the user's task rather than starting additional agents simply because the skill is loaded.

Curator manages agent-created skills and archives them; bundled and hub-installed skills are outside its maintenance scope. Use memory, session search, and skill updates to persist useful knowledge while respecting the user's requested scope and existing evolution constraints.

### 4. Manage or troubleshoot the gateway

On Linux, the README specifies the `kylin-agent-runtime-gateway.service` systemd user unit:

```bash
systemctl --user status kylin-agent-runtime-gateway.service
journalctl --user -u kylin-agent-runtime-gateway.service -f
```

Use `start`, `stop`, or `restart` with that unit when the task calls for service control. For a system-level unit, omit `--user` and use the corresponding privileged command. Check the actual unit and active profile before restarting a service.

For failures, inspect relevant logs using `kylin-agent-runtime logs` or `search_files` and `read_file`. Distinguish provider/authentication failures, missing tool prerequisites, platform configuration, and process/service failures. A configured gateway working directory comes from `terminal.cwd`; the CLI uses its process working directory.

### 5. Update or extend the runtime

For an existing source installation, follow the README from the checkout:

```bash
git pull
source .venv/bin/activate
uv pip install -e ".[all]"
```

Inspect local changes before updating. If the dependency lock changed, use `uv sync --all-extras` as documented. Prefer this fork's README workflow over inherited upstream update or installation instructions.

For development, install `.[all,dev]` and read the applicable `AGENTS.md` before editing:

- Prefer plugins for local tools. Only add a core tool when explicitly contributing to the base runtime; register it in `tools/` and expose it through `toolsets.py`. Tool handlers return JSON strings.
- Add slash commands through `kylin_agent_runtime_cli/commands.py` and the appropriate CLI/gateway handlers.
- Preserve prompt caching and profile isolation. Keep configuration in the correct loader; CLI and gateway loaders differ.
- Extend `ui-tui/` and `tui_gateway/` for the primary chat experience. The dashboard embeds that TUI through a PTY; supporting React widgets may complement it.
- Use the repository's dependency pinning policy and run appropriate checks through `scripts/run_tests.sh`.

## Pitfalls

- Renaming this skill does not rename internal APIs such as `HermesCLI`, `get_hermes_home()`, or `_HERMES_CORE_TOOLS`.
- The source directory and runtime state directory serve different purposes; avoid editing an inactive checkout or another profile's state.
- Changing enabled tools does not necessarily change the running conversation. Preserve caching and start a new session when needed.
- Missing credentials, disabled toolsets, and unavailable plugins can hide functionality. Inspect availability before assuming a command or tool is broken.
- Native Windows activation and service management differ from POSIX. The dashboard's PTY chat requires a POSIX environment such as Linux/WSL; check feature support before choosing a workaround.
- For tests, prefer `scripts/run_tests.sh` and follow `AGENTS.md` for platform-specific exceptions. Tests must not write to the user's real runtime home.

## Verification

- Confirm the CLI resolves from the intended environment and reports its version; use `doctor` for installation/configuration checks.
- Verify the requested setting or feature on the selected profile and execution surface. For gateways, check both service status and relevant logs.
- After a skill rename or edit, confirm it appears as `kylin-agent` in skill discovery and can be loaded with `/skill kylin-agent` or `-s kylin-agent` in a new session.
- For source changes, run focused checks with `scripts/run_tests.sh`; run the full suite before pushing as required by `AGENTS.md`.
- Report what changed, what was verified, and any remaining limitation without exposing authentication material.
