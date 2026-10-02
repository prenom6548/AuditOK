---
sidebar_position: 7
---

# Profile Commands Reference

This page covers all commands related to [Hermes profiles](../user-guide/profiles.md). For general CLI commands, see [CLI Commands Reference](./cli-commands.md).

## `kylin-agent-runtime profile`

```bash
kylin-agent-runtime profile <subcommand>
```

Top-level command for managing profiles. Running `kylin-agent-runtime profile` without a subcommand shows help.

| Subcommand | Description |
|------------|-------------|
| `list` | List all profiles. |
| `use` | Set the active (default) profile. |
| `create` | Create a new profile. |
| `delete` | Delete a profile. |
| `show` | Show details about a profile. |
| `alias` | Regenerate the shell alias for a profile. |
| `rename` | Rename a profile. |
| `export` | Export a profile to a tar.gz archive. |
| `import` | Import a profile from a tar.gz archive. |
| `install` | Install a profile distribution from a git URL or local directory. See [Profile Distributions](../user-guide/profile-distributions.md). |
| `update` | Re-pull a distribution-managed profile and re-apply its bundle. |
| `info` | Show distribution metadata for a profile (origin URL, commit, last update). |

## `kylin-agent-runtime profile list`

```bash
kylin-agent-runtime profile list
```

Lists all profiles. The currently active profile is marked with `*`.

**Example:**

```bash
$ kylin-agent-runtime profile list
  default
* work
  dev
  personal
```

No options.

## `kylin-agent-runtime profile use`

```bash
kylin-agent-runtime profile use <name>
```

Sets `<name>` as the active profile. All subsequent `kylin-agent-runtime` commands (without `-p`) will use this profile.

| Argument | Description |
|----------|-------------|
| `<name>` | Profile name to activate. Use `default` to return to the base profile. |

**Example:**

```bash
kylin-agent-runtime profile use work
kylin-agent-runtime profile use default
```

## `kylin-agent-runtime profile create`

```bash
kylin-agent-runtime profile create <name> [options]
```

Creates a new profile.

| Argument / Option | Description |
|-------------------|-------------|
| `<name>` | Name for the new profile. Must be a valid directory name (alphanumeric, hyphens, underscores). |
| `--clone` | Copy `config.yaml`, `.env`, and `SOUL.md` from the current profile. |
| `--clone-all` | Copy everything (config, memories, skills, sessions, state) from the current profile. |
| `--clone-from <profile>` | Clone from a specific profile instead of the current one. Used with `--clone` or `--clone-all`. |
| `--no-alias` | Skip wrapper script creation. |
| `--description "<text>"` | One- or two-sentence description of what this profile is good at. Used by the kanban orchestrator to route tasks based on role instead of profile name alone. Skip and add later via `kylin-agent-runtime profile describe`. Persisted in `<profile_dir>/profile.yaml`. |
| `--no-skills` | Create an **empty** profile with zero bundled skills enabled. Writes a `.no-skills` marker into the profile so future `kylin-agent-runtime update` runs won't re-seed the bundled set, and refuses to combine with `--clone` / `--clone-all` (which would copy skills in anyway). Useful for narrow orchestrator profiles or sandbox profiles that should not inherit the full skill catalog. |

Creating a profile does **not** make that profile directory the default project/workspace directory for terminal commands. If you want a profile to start in a specific project, set `terminal.cwd` in that profile's `config.yaml`.

**Examples:**

```bash
# Blank profile — needs full setup
kylin-agent-runtime profile create mybot

# Clone config only from current profile
kylin-agent-runtime profile create work --clone

# Clone everything from current profile
kylin-agent-runtime profile create backup --clone-all

# Clone config from a specific profile
kylin-agent-runtime profile create work2 --clone --clone-from work
```

## `kylin-agent-runtime profile describe`

```bash
kylin-agent-runtime profile describe [<name>] [options]
```

Read or set a profile's description. The description is consumed by the kanban orchestrator to route tasks based on what each profile is good at, rather than guessing from the profile name alone. Persisted in `<profile_dir>/profile.yaml` so it survives reboots and is shared with the gateway.

With no flags, prints the current description (or `(no description set for '<name>')` if empty).

| Argument / Option | Description |
|-------------------|-------------|
| `<name>` | Profile to describe. Required unless `--all --auto` is used. |
| `--text "<text>"` | Set the description to this exact text (user-authored). Overwrites any existing description. |
| `--auto` | Auto-generate a 1-2 sentence description via the auxiliary LLM, based on the profile's installed skills, configured model, and name. Configure the model under `auxiliary.profile_describer` in `config.yaml`. Auto-generated descriptions are marked `description_auto: true` so the dashboard can flag them for review. |
| `--overwrite` | With `--auto`, replace user-authored descriptions too (default: skip profiles whose description was set explicitly). |
| `--all` | With `--auto`, sweep every profile missing a description. |

**Examples:**

```bash
# Read the current description
kylin-agent-runtime profile describe researcher

# Set it explicitly
kylin-agent-runtime profile describe researcher --text "Reads source code and writes findings."

# Let the LLM generate one
kylin-agent-runtime profile describe researcher --auto

# Fill in descriptions for every profile that doesn't have one
kylin-agent-runtime profile describe --all --auto
```

## `kylin-agent-runtime profile delete`

```bash
kylin-agent-runtime profile delete <name> [options]
```

Deletes a profile and removes its shell alias.

| Argument / Option | Description |
|-------------------|-------------|
| `<name>` | Profile to delete. |
| `--yes`, `-y` | Skip confirmation prompt. |

**Example:**

```bash
kylin-agent-runtime profile delete mybot
kylin-agent-runtime profile delete mybot --yes
```

:::warning
This permanently deletes the profile's entire directory including all config, memories, sessions, and skills. Cannot delete the currently active profile.
:::

## `kylin-agent-runtime profile show`

```bash
kylin-agent-runtime profile show <name>
```

Displays details about a profile including its home directory, configured model, gateway status, skills count, and configuration file status.

This shows the profile's Hermes home directory, not the terminal working directory. Terminal commands start from `terminal.cwd` (or the launch directory on the local backend when `cwd: "."`).

| Argument | Description |
|----------|-------------|
| `<name>` | Profile to inspect. |

**Example:**

```bash
$ kylin-agent-runtime profile show work
Profile: work
Path:    ~/.kylin-agent-runtime/profiles/work
Model:   anthropic/claude-sonnet-4 (anthropic)
Gateway: stopped
Skills:  12
.env:    exists
SOUL.md: exists
Alias:   ~/.local/bin/work
```

## `kylin-agent-runtime profile alias`

```bash
kylin-agent-runtime profile alias <name> [options]
```

Regenerates the shell alias script at `~/.local/bin/<name>`. Useful if the alias was accidentally deleted or if you need to update it after moving your Hermes installation.

| Argument / Option | Description |
|-------------------|-------------|
| `<name>` | Profile to create/update the alias for. |
| `--remove` | Remove the wrapper script instead of creating it. |
| `--name <alias>` | Custom alias name (default: profile name). |

**Example:**

```bash
kylin-agent-runtime profile alias work
# Creates/updates ~/.local/bin/work

kylin-agent-runtime profile alias work --name mywork
# Creates ~/.local/bin/mywork

kylin-agent-runtime profile alias work --remove
# Removes the wrapper script
```

## `kylin-agent-runtime profile rename`

```bash
kylin-agent-runtime profile rename <old-name> <new-name>
```

Renames a profile. Updates the directory and shell alias.

| Argument | Description |
|----------|-------------|
| `<old-name>` | Current profile name. |
| `<new-name>` | New profile name. |

**Example:**

```bash
kylin-agent-runtime profile rename mybot assistant
# ~/.kylin-agent-runtime/profiles/mybot → ~/.kylin-agent-runtime/profiles/assistant
# ~/.local/bin/mybot → ~/.local/bin/assistant
```

## `kylin-agent-runtime profile export`

```bash
kylin-agent-runtime profile export <name> [options]
```

Exports a profile as a compressed tar.gz archive.

| Argument / Option | Description |
|-------------------|-------------|
| `<name>` | Profile to export. |
| `-o`, `--output <path>` | Output file path (default: `<name>.tar.gz`). |

**Example:**

```bash
kylin-agent-runtime profile export work
# Creates work.tar.gz in the current directory

kylin-agent-runtime profile export work -o ./work-2026-03-29.tar.gz
```

## `kylin-agent-runtime profile import`

```bash
kylin-agent-runtime profile import <archive> [options]
```

Imports a profile from a tar.gz archive.

| Argument / Option | Description |
|-------------------|-------------|
| `<archive>` | Path to the tar.gz archive to import. |
| `--name <name>` | Name for the imported profile (default: inferred from archive). |

**Example:**

```bash
kylin-agent-runtime profile import ./work-2026-03-29.tar.gz
# Infers profile name from the archive

kylin-agent-runtime profile import ./work-2026-03-29.tar.gz --name work-restored
```

## Distribution commands

:::tip
**New to distributions?** Start with the [Profile Distributions user guide](../user-guide/profile-distributions.md) — it covers the why, when, and how with full examples. The sections below are a dry CLI reference for when you know what you want.
:::

Distributions turn a profile into a shareable, versioned artifact published
as a **git repository**. A recipient installs the distribution with a single
command and can update it in place later without touching their local
memories, sessions, or credentials.

`auth.json` and `.env` are never part of a distribution — they stay on the
installing user's machine.

The recipient's user data (memories, sessions, auth, their own edits to
`.env`) is always preserved across the initial install and subsequent
updates.

:::info
`kylin-agent-runtime profile export` / `import` are still the right commands for
**local backup and restore** of a profile on your own machine. Distribution
(`install` / `update` / `info`) is a separate concept: ship a profile via
git so someone else can install it.
:::

### `kylin-agent-runtime profile install`

```bash
kylin-agent-runtime profile install <source> [--name <name>] [--alias] [--force] [--yes]
```

Installs a profile distribution from a git URL or a local directory.

| Option | Description |
|--------|-------------|
| `<source>` | Git URL (`github.com/user/repo`, `https://...`, `git@...`, `ssh://`, `git://`) or a local directory containing `distribution.yaml` at its root. |
| `--name NAME` | Override the profile name from the manifest. |
| `--alias` | Also create a shell wrapper (e.g. `telemetry` → `kylin-agent-runtime -p telemetry`). |
| `--force` | Overwrite an existing profile of the same name. User data is still preserved. |
| `-y`, `--yes` | Skip the manifest-preview confirmation prompt. |

The installer shows the manifest, lists required env vars, and warns about
cron jobs before asking for confirmation. Required env vars go into a
`.env.EXAMPLE` file you copy to `.env` and fill in.

**Examples:**

```bash
# Install from a GitHub repo (shorthand)
kylin-agent-runtime profile install github.com/kyle/telemetry-distribution --alias

# Install from a full HTTPS git URL
kylin-agent-runtime profile install https://github.com/kyle/telemetry-distribution.git

# Install from SSH
kylin-agent-runtime profile install git@github.com:kyle/telemetry-distribution.git

# Install from a local directory during development
kylin-agent-runtime profile install ./telemetry/
```

### `kylin-agent-runtime profile update`

```bash
kylin-agent-runtime profile update <name> [--force-config] [--yes]
```

Re-clones the distribution from its recorded source and applies updates.
Distribution-owned files (SOUL.md, skills/, cron/, mcp.json) are
overwritten; user data (memories, sessions, auth, .env) is never touched.

`config.yaml` is preserved by default to keep your local overrides.
Pass `--force-config` to reset it to the distribution's shipped config.

### `kylin-agent-runtime profile info`

```bash
kylin-agent-runtime profile info <name>
```

Prints the profile's distribution manifest — name, version, required
Hermes version, author, env var requirements, the source URL/path, and
the `Installed:` timestamp recorded when the distribution was last
`install`-ed or `update`-d. Useful for checking what a shared profile
needs before installing it, and for spotting "this profile was installed
6 months ago and hasn't been updated."

`kylin-agent-runtime profile list` also shows the distribution name and version in a
`Distribution` column, and `kylin-agent-runtime profile show <name>` / `delete <name>`
surface the source URL so you can tell at a glance which profiles came
from a git repo vs. were created locally.

### Private distributions

A private git repository works as a distribution source with no extra
configuration — the install shells out to your normal `git` binary, so
whatever authentication your shell is already set up for (SSH key,
`git credential` helper, GitHub CLI's stored HTTPS credentials) applies
transparently.

```bash
# Uses your SSH key, the same as any other `git clone`
kylin-agent-runtime profile install git@github.com:your-org/internal-assistant.git

# Uses your git credential helper
kylin-agent-runtime profile install https://github.com/your-org/internal-assistant.git
```

If a clone prompts for credentials interactively in your terminal during
install, that prompt flows through. Set up your auth the way you'd
normally use `git clone` against the same repo first, then install.

### Distribution manifest (`distribution.yaml`)

Every distribution has a `distribution.yaml` at the root of its repository:

```yaml
name: telemetry
version: 0.1.0
description: "Compliance monitoring harness"
hermes_requires: ">=0.12.0"
author: "Your Name"
license: "MIT"
env_requires:
  - name: OPENAI_API_KEY
    description: "OpenAI API key"
    required: true
  - name: GRAPHITI_MCP_URL
    description: "Memory graph URL"
    required: false
    default: "http://127.0.0.1:8000/sse"
distribution_owned:   # optional; defaults to SOUL.md, config.yaml,
                      #   mcp.json, skills/, cron/, distribution.yaml
  - SOUL.md
  - skills/compliance/
  - cron/
```

`hermes_requires` supports `>=`, `<=`, `==`, `!=`, `>`, `<`, or a bare
version (treated as `>=`). Install fails with a clear error if the current
Hermes version doesn't satisfy the spec.

`distribution_owned` is optional. If set, only those paths are replaced on
update; anything else in the profile stays user-owned. If omitted, the
defaults above apply.

### Publishing a distribution

Authoring a distribution is just a git push:

1. In your profile directory, create `distribution.yaml` with at least `name`
   and `version`.
2. Initialize a git repo (or use an existing one) and push to GitHub /
   GitLab / any host Hermes can clone from.
3. Tell recipients to run `kylin-agent-runtime profile install <your-repo-url>`.

Use git tags for versioned releases — recipients who clone `HEAD` get your
latest state, and you can always bump `version:` in the manifest.

## `kylin-agent-runtime -p` / `kylin-agent-runtime --profile`

```bash
kylin-agent-runtime -p <name> <command> [options]
kylin-agent-runtime --profile <name> <command> [options]
```

Global flag to run any Hermes command under a specific profile without changing the sticky default. This overrides the active profile for the duration of the command.

| Option | Description |
|--------|-------------|
| `-p <name>`, `--profile <name>` | Profile to use for this command. |

**Examples:**

```bash
kylin-agent-runtime -p work chat -q "Check the server status"
kylin-agent-runtime --profile dev gateway start
kylin-agent-runtime -p personal skills list
kylin-agent-runtime -p work config edit
```

## `kylin-agent-runtime completion`

```bash
kylin-agent-runtime completion <shell>
```

Generates shell completion scripts. Includes completions for profile names and profile subcommands.

| Argument | Description |
|----------|-------------|
| `<shell>` | Shell to generate completions for: `bash`, `zsh`, or `fish`. |

**Examples:**

```bash
# Install completions
kylin-agent-runtime completion bash >> ~/.bashrc
kylin-agent-runtime completion zsh >> ~/.zshrc
kylin-agent-runtime completion fish > ~/.config/fish/completions/hermes.fish

# Reload shell
source ~/.bashrc
```

After installation, tab completion works for:
- `kylin-agent-runtime profile <TAB>` — subcommands (list, use, create, etc.)
- `kylin-agent-runtime profile use <TAB>` — profile names
- `kylin-agent-runtime -p <TAB>` — profile names

## See also

- [Profiles User Guide](../user-guide/profiles.md)
- [CLI Commands Reference](./cli-commands.md)
- [FAQ — Profiles section](./faq.md#profiles)
