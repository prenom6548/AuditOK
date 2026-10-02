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
