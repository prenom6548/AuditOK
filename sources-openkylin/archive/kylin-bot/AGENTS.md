# AGENTS.md — KylinBot

Cross-tool agent instructions for any AI coding assistant working on this repository.

## Commands

```bash
cargo fmt --all -- --check
cargo clippy --all-targets -- -D warnings
cargo test
```

Full pre-PR validation (recommended):

```bash
./dev/ci.sh all
```

Docs-only changes: run markdown lint and link-integrity checks. If touching bootstrap scripts: `bash -n install.sh`.

## Project Snapshot

KylinBot is a Rust-first autonomous agent runtime optimized for performance, efficiency, stability, extensibility, sustainability, and security.

Core architecture is trait-driven and modular. Extend by implementing traits and registering in factory modules.

Key extension points:

- `crates/kylinbot-api/src/provider.rs` (`Provider`)
- `crates/kylinbot-api/src/channel.rs` (`Channel`)
- `crates/kylinbot-api/src/tool.rs` (`Tool`)
- `crates/kylinbot-api/src/memory_traits.rs` (`Memory`)
- `crates/kylinbot-api/src/observability_traits.rs` (`Observer`)
- `crates/kylinbot-api/src/runtime_traits.rs` (`RuntimeAdapter`)
- `crates/kylinbot-api/src/peripherals_traits.rs` (`Peripheral`) — hardware boards (STM32, RPi GPIO)

## Stability Tiers

Every workspace crate carries a stability tier per the Microkernel Architecture RFC.

| Crate | Tier | Notes |
|-------|------|-------|
| `kylinbot-api` | Experimental | Stable at v1.0.0 (formal milestone) |
| `kylinbot-config` | Beta | Stable at v0.8.0 |
| `kylinbot-providers` | Beta | — |
| `kylinbot-memory` | Beta | — |
| `kylinbot-infra` | Beta | — |
| `kylinbot-tool-call-parser` | Beta | Stable at v0.8.0 |
| `kylinbot-channels` | Experimental | Plugin migration at v1.0.0 |
| `kylinbot-tools` | Experimental | Plugin migration at v1.0.0 |
| `kylinbot-runtime` | Experimental | Agent runtime (agent loop, security, cron, SOP, skills, observability) |
| `kylinbot-gateway` | Experimental | Separate binary at v0.9.0 |
| `kylinbot-tui` | Experimental | TUI onboarding wizard |
| `kylinbot-plugins` | Experimental | WASM plugin system — foundation for v1.0.0 plugin ecosystem |
| `kylinbot-hardware` | Experimental | USB discovery, peripherals, serial |
| `kylinbot-macros` | Beta | Tightly coupled to config schema |

**Tiers**: Stable = covered by breaking-change policy. Beta = breaking changes permitted in MINOR with changelog notes. Experimental = no stability guarantee.

Tiers are promoted, never demoted, through deliberate team decision.

## Repository Map

- `src/main.rs` — CLI entrypoint and command routing
- `src/lib.rs` — module re-exports and CLI command enum definitions
- `crates/kylinbot-api/` — public trait definitions (Provider, Channel, Tool, Memory, Observer, Peripheral)
- `crates/kylinbot-config/` — schema, config loading/merging
- `crates/kylinbot-macros/` — Configurable derive macro
- `crates/kylinbot-providers/` — model providers and resilient wrapper
- `crates/kylinbot-channels/` — messaging platform integrations (30+ channels)
- `crates/kylinbot-channels/src/orchestrator/` — channel lifecycle, routing, media pipeline
- `crates/kylinbot-tools/` — tool execution surface (shell, file, memory, browser)
- `crates/kylinbot-runtime/` — agent loop, security, cron, SOP, skills, onboarding wizard, observability
- `crates/kylinbot-memory/` — memory backends (markdown, sqlite, embeddings, vector merge)
- `crates/kylinbot-infra/` — shared infrastructure (debounce, session, stall watchdog)
- `crates/kylinbot-gateway/` — webhook/gateway server (separate binary)
- `crates/kylinbot-hardware/` — USB discovery, peripherals, serial, GPIO
- `crates/kylinbot-tui/` — TUI onboarding wizard
- `crates/kylinbot-plugins/` — WASM plugin system
- `crates/kylinbot-tool-call-parser/` — tool call parsing
- `docs/` — topic-based documentation (setup-guides, reference, ops, security, hardware, contributing, maintainers)
- `.github/` — CI, templates, automation workflows

## Risk Tiers

- **Low risk**: docs/chore/tests-only changes
- **Medium risk**: most `crates/*/src/**` behavior changes without boundary/security impact
- **High risk**: `crates/kylinbot-runtime/src/**` (especially `src/security/`), `crates/kylinbot-gateway/src/**`, `crates/kylinbot-tools/src/**`, `.github/workflows/**`, access-control boundaries

When uncertain, classify as higher risk.

## Workflow

1. **Start with architecture** — before starting a new task, read `docs/book/src/architecture/detailed.md`, then inspect the relevant focused docs for the area being changed.
2. **Read before write** — inspect existing module, factory wiring, and adjacent tests before editing.
3. **One concern per PR** — avoid mixed feature+refactor+infra patches.
4. **Implement minimal patch** — no speculative abstractions, no config keys without a concrete use case.
5. **Validate by risk tier** — docs-only: lightweight checks. Code changes: full relevant checks.
6. **Document impact** — update PR notes for behavior, risk, side effects, and rollback.
7. **Queue hygiene** — stacked PR: declare `Depends on #...`. Replacing old PR: declare `Supersedes #...`.

Branch/commit/PR rules:
- Work from a non-`master` branch. Open a PR to `master`; do not push directly.
- Use conventional commit titles. Prefer small PRs (`size: XS/S/M`).
- Follow `.github/pull_request_template.md` fully.
- Never commit secrets, personal data, or real identity information (see `@docs/book/src/contributing/privacy.md`).

## Anti-Patterns

- Do not add heavy dependencies for minor convenience.
- Do not silently weaken security policy or access constraints.
- Do not add speculative config/feature flags "just in case".
- Do not mix massive formatting-only changes with functional changes.
- Do not modify unrelated modules "while here".
- Do not bypass failing checks without explicit explanation.
- Do not hide behavior-changing side effects in refactor commits.
- Do not suppress unused production code with underscore prefixes or `#[allow(dead_code)]`; delete it, wire it into behavior, or track a follow-up issue. Reserve underscore names for required but intentionally unused API, trait, or callback parameters.
- Do not leave `unwrap()` / `expect()` in production paths; propagate errors or document the invariant that makes panic impossible.
- Do not include personal identity or sensitive information in test data, examples, docs, or commits.

## Skills

AI coding assistant skills live in `.claude/skills/`. Use the right one for the job:

- `.claude/skills/github-pr-review-session/SKILL.md` — PR review co-pilot; assists **you** as the human reviewer. Posts reviews as WareWolf-MoonWall using the RFC feedback taxonomy (🔴/🟡/✅/🔵/🟢). Trigger: `review 1234`, `re-review 1234`, `go through the queue`.
- `.claude/skills/changelog-generation/SKILL.md` — generates `CHANGELOG-next.md` between stable tags, resolves contributors via GraphQL, feeds the release workflow. Trigger: `generate changelog`, `release notes for v0.7.x`.
- `.claude/skills/github-issue-triage/SKILL.md` — Issue triage and lifecycle management; manages the backlog, labels, and stale policies. Trigger: `triage issues`, `sweep issues`, `handle issue #N`.
- `.claude/skills/github-issue/SKILL.md` — Interactively files structured GitHub issues (bug reports or feature requests) using repo templates. Trigger: `file issue`, `report bug`, `feature request`.
- `.claude/skills/github-pr/SKILL.md` — Opens or updates GitHub PRs, handles validation evidence, and manages PR descriptions. Trigger: `open PR`, `update PR`, `submit for review`.
- `.claude/skills/skill-creator/SKILL.md` — Framework for creating, testing, evaluating, and optimizing new AI skills. Trigger: `create skill`, `improve skill`, `run skill evals`.
- `.claude/skills/squash-merge/SKILL.md` — Performs conventional squash-merges into master with preserved commit history. Trigger: `squash-merge #123`, `land #789`.
- `.claude/skills/kylinbot/SKILL.md` — Operational guide for interacting with a KylinBot agent instance via CLI or API. Trigger: `check agent status`, `manage memory`, `kylinbot config`.

## Localization

- All user-facing output (CLI messages, tool descriptions, onboarding prompts) must use `fl!()` / Fluent strings — never bare string literals.
- Log messages, `tracing::` spans/events, and panic messages stay in English with stable `error_key` fields (RFC #5653 §4.6).
- Panics and `tracing::` lines are never translated.
- The Wiki and internal developer docs are English only.

Dev-operational contracts — files consumed by AI coding skills and development tooling. Do not move or delete without updating all consuming skills and AGENTS.md:

| Protected file | Consuming skill / tool |
|---|---|
| `docs/book/src/contributing/pr-review-protocol.md` | `github-pr-review-session` — review protocol |
| `docs/book/src/maintainers/changelog-generation.md` | `changelog-generation` — release procedure |
| `docs/book/src/maintainers/reviewer-playbook.md` | `github-issue-triage` — triage governance |
| `docs/book/src/maintainers/pr-workflow.md` | `github-issue-triage` — triage discipline |
| `docs/book/src/contributing/privacy.md` | `github-issue-triage`, PR template — privacy rules |
| `docs/book/src/foundations/fnd-00*.md` | `github-pr-review-session` — RFC reference data; public transparency documents |

## Linked References

- `@docs/book/src/architecture/detailed.md` — required start-of-task architecture map for agents and contributors
- `@docs/book/src/developing/extension-examples.md` — adding providers, channels, tools, peripherals; tool shared-state contract; architecture boundary rules
- `@docs/book/src/contributing/privacy.md` — privacy rules and neutral-placeholder palette
- `@docs/book/src/maintainers/superseding.md` — superseded-PR attribution, PR/commit templates, handoff template
