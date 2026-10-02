# SkillHub Uninstall Feature Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add skill uninstall support to the SkillHub module and auto-clean stale lockfile entries when skill directories are missing.

**Architecture:** Full-stack implementation across 4 layers: (1) Rust backend `zeroclaw-skillhub` crate adds `uninstall` command that deletes skill directory and removes lockfile entry; (2) CLI adds `SkillhubCommands::Uninstall` variant; (3) Gateway adds `POST /api/skillhub/uninstall` handler; (4) Frontend adds `uninstallSkill` API function, `UninstallResult` type, and UI buttons. Additionally, the `list` command and Gateway list handler will check if skill directories exist and clean stale entries from lockfile.

**Tech Stack:** Rust (axum, serde, anyhow), TypeScript/React (lucide-react, apiFetch)

---

## File Structure

| Action | File | Responsibility |
|--------|------|----------------|
| Create | `crates/zeroclaw-skillhub/src/commands/uninstall.rs` | Uninstall command implementation |
| Modify | `crates/zeroclaw-skillhub/src/commands/mod.rs` | Export uninstall module |
| Modify | `crates/zeroclaw-skillhub/src/commands/list.rs` | Add directory existence check + lockfile cleanup |
| Modify | `crates/kylinbot-gateway/src/api_skillhub.rs` | Add uninstall handler + list cleanup logic |
| Modify | `crates/kylinbot-gateway/src/lib.rs` | Register uninstall route |
| Modify | `src/lib.rs` | Add `SkillhubCommands::Uninstall` variant |
| Modify | `src/skillhub/mod.rs` | Handle Uninstall command dispatch |
| Modify | `web_kbot/src/lib/skillhubApi.ts` | Add `uninstallSkill` function |
| Modify | `web_kbot/src/pages/skillhub/types.ts` | Add `UninstallResult` type |
| Modify | `web_kbot/src/pages/skillhub/InstalledTab.tsx` | Add uninstall button |
| Modify | `web_kbot/src/pages/skillhub/SkillRow.tsx` | Add uninstall button support |

---

### Task 1: Backend - Create uninstall command

**Files:**
- Create: `crates/zeroclaw-skillhub/src/commands/uninstall.rs`

- [ ] **Step 1: Write uninstall.rs**

```rust
use anyhow::{bail, Result};
use colored::Colorize;

use crate::config::skills_install_dir;
use crate::lockfile;

pub fn run(workspace_dir: &std::path::Path, slug: &str) -> Result<()> {
    let slug = slug.trim();
    if slug.is_empty() {
        bail!("empty skill slug");
    }
    if slug.contains("..") || slug.contains('/') || slug.contains('\\') {
        bail!("invalid skill slug: {}", slug);
    }

    let skills_dir = skills_install_dir(workspace_dir);
    let target_dir = skills_dir.join(slug);

    if let Ok(canonical_skills) = skills_dir.canonicalize() {
        if let Ok(canonical_target) = target_dir.canonicalize() {
            if !canonical_target.starts_with(&canonical_skills) {
                bail!("skill path escapes skills directory: {}", slug);
            }
        }
    }

    let mut lock = lockfile::load_lockfile(&skills_dir);

    if !lock.skills.contains_key(slug) && !target_dir.exists() {
        bail!("skill '{}' not found", slug);
    }

    if target_dir.exists() {
        std::fs::remove_dir_all(&target_dir)?;
    }

    let removed = lock.skills.remove(slug).is_some();
    if removed {
        lockfile::save_lockfile(&skills_dir, &lock)?;
        println!(
            "\n  {} Skill uninstalled: {}",
            "✓".green().bold(),
            slug
        );
    } else {
        lockfile::save_lockfile(&skills_dir, &lock)?;
        println!(
            "\n  {} Lockfile entry removed for: {}",
            "✓".green().bold(),
            slug
        );
    }

    Ok(())
}
```

---

### Task 2: Backend - Export uninstall module

**Files:**
- Modify: `crates/zeroclaw-skillhub/src/commands/mod.rs`

- [ ] **Step 1: Add `pub mod uninstall;` to mod.rs**

---

### Task 3: Backend - Add directory existence check to list command

**Files:**
- Modify: `crates/zeroclaw-skillhub/src/commands/list.rs`

- [ ] **Step 1: Modify list.rs to check directory existence and clean stale entries**

After loading lockfile, iterate entries and check if each skill directory exists. Remove entries whose directories are missing, save updated lockfile if any were removed.

---

### Task 4: CLI - Add Uninstall variant

**Files:**
- Modify: `src/lib.rs`

- [ ] **Step 1: Add `Uninstall` variant to `SkillhubCommands` enum**

---

### Task 5: CLI - Handle Uninstall command

**Files:**
- Modify: `src/skillhub/mod.rs`

- [ ] **Step 1: Add `SkillhubCommands::Uninstall` match arm**

---

### Task 6: Gateway - Add uninstall handler

**Files:**
- Modify: `crates/kylinbot-gateway/src/api_skillhub.rs`

- [ ] **Step 1: Add `UninstallBody` struct and `handle_skillhub_uninstall` handler**

- [ ] **Step 2: Modify `handle_skillhub_list` to check directory existence and clean stale lockfile entries**

---

### Task 7: Gateway - Register route

**Files:**
- Modify: `crates/kylinbot-gateway/src/lib.rs`

- [ ] **Step 1: Add `.route("/api/skillhub/uninstall", post(api_skillhub::handle_skillhub_uninstall))`**

---

### Task 8: Frontend - Add uninstallSkill API

**Files:**
- Modify: `web_kbot/src/lib/skillhubApi.ts`
- Modify: `web_kbot/src/pages/skillhub/types.ts`

- [ ] **Step 1: Add `UninstallResult` type to types.ts**

- [ ] **Step 2: Add `uninstallSkill` function to skillhubApi.ts**

---

### Task 9: Frontend - Add uninstall UI

**Files:**
- Modify: `web_kbot/src/pages/skillhub/InstalledTab.tsx`
- Modify: `web_kbot/src/pages/skillhub/SkillRow.tsx`

- [ ] **Step 1: Add uninstall button and handler to InstalledTab.tsx**

- [ ] **Step 2: Add uninstall button and onUninstall callback to SkillRow.tsx**

---

### Task 10: Verify

- [ ] **Step 1: Run `cargo clippy --all-targets -- -D warnings`**
- [ ] **Step 2: Run `cargo test`**
