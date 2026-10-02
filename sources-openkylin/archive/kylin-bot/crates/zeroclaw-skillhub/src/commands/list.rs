use anyhow::Result;
use colored::Colorize;

use crate::config::skills_install_dir;

pub fn run(workspace_dir: &std::path::Path, json_output: bool) -> Result<()> {
    let skills_dir = skills_install_dir(workspace_dir);

    let mut lock = crate::lockfile::load_lockfile(&skills_dir);

    if lock.skills.is_empty() {
        println!("No skills installed via ClawHub.");
        return Ok(());
    }

    let stale: Vec<String> = lock
        .skills
        .keys()
        .filter(|slug| !skills_dir.join(slug).exists())
        .cloned()
        .collect();

    if !stale.is_empty() {
        for slug in &stale {
            lock.skills.remove(slug);
        }
        crate::lockfile::save_lockfile(&skills_dir, &lock)?;
    }

    if json_output {
        println!("{}", serde_json::to_string_pretty(&lock)?);
        return Ok(());
    }

    println!("Installed skills ({}):\n", lock.skills.len());

    let mut slugs: Vec<_> = lock.skills.keys().collect();
    slugs.sort();

    for slug in slugs {
        let entry = &lock.skills[slug];
        let version_str = if entry.version.is_empty() {
            String::new()
        } else {
            format!(" v{}", entry.version)
        };
        let name = if entry.name.is_empty() || entry.name == *slug {
            slug.bold().to_string()
        } else {
            format!("{} ({})", entry.name, slug.bold())
        };
        println!("  {}{}", name, version_str.dimmed());
        if !entry.source.is_empty() {
            println!("    source: {}", entry.source);
        }
    }

    Ok(())
}
