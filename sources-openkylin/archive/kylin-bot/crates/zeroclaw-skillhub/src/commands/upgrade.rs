use anyhow::Result;

use crate::client::ApiClient;
use crate::config::{self, SKILL_CONFIG_NAME, SKILL_META_NAME, set_owner_only_permissions};
use crate::extract;
use crate::lockfile;
use crate::version;

pub async fn run(
    api: &ApiClient,
    workspace_dir: &std::path::Path,
    slug: Option<&str>,
    check_only: bool,
    timeout_secs: u64,
) -> Result<()> {
    let skills_dir = config::skills_install_dir(workspace_dir);

    let mut lock = lockfile::load_lockfile(&skills_dir);

    let targets: Vec<String> = if let Some(s) = slug {
        vec![s.to_string()]
    } else {
        lock.skills.keys().cloned().collect()
    };

    if targets.is_empty() {
        println!("No installed skills to upgrade.");
        return Ok(());
    }

    let mut checked = 0u32;
    let mut upgraded = 0u32;
    let mut skipped = 0u32;
    let mut failed = 0u32;

    for target_slug in &targets {
        checked += 1;
        let target_dir = skills_dir.join(target_slug);

        if !target_dir.exists() {
            println!("[{}] skip: directory not found", target_slug);
            skipped += 1;
            continue;
        }

        let current_version = lock
            .skills
            .get(target_slug)
            .and_then(|e| {
                if e.version.is_empty() {
                    None
                } else {
                    Some(e.version.clone())
                }
            })
            .unwrap_or_default();

        let config_path = target_dir.join(SKILL_CONFIG_NAME);
        if !config_path.exists() {
            println!("[{}] skip: {} not found", target_slug, SKILL_CONFIG_NAME);
            skipped += 1;
            continue;
        }

        let config_text = match std::fs::read_to_string(&config_path) {
            Ok(t) => t,
            Err(e) => {
                println!("[{}] fail: cannot read config: {}", target_slug, e);
                failed += 1;
                continue;
            }
        };

        let skill_config: serde_json::Value = match serde_json::from_str(&config_text) {
            Ok(v) => v,
            Err(e) => {
                println!("[{}] fail: invalid config JSON: {}", target_slug, e);
                failed += 1;
                continue;
            }
        };

        let update_url = extract_update_url(&skill_config);
        if update_url.is_empty() {
            println!("[{}] skip: no update URL in config", target_slug);
            skipped += 1;
            continue;
        }

        let manifest = match api.fetch_json(&update_url, timeout_secs).await {
            Ok(m) => m,
            Err(e) => {
                println!("[{}] fail: cannot fetch manifest: {}", target_slug, e);
                failed += 1;
                continue;
            }
        };

        let latest_version = manifest
            .get("version")
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string();
        let package_url = manifest
            .get("url")
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string();
        let expected_sha = manifest
            .get("sha256")
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string();

        if latest_version.is_empty() || package_url.is_empty() {
            println!("[{}] fail: manifest missing version or url", target_slug);
            failed += 1;
            continue;
        }

        if !version::version_is_newer(&latest_version, &current_version) {
            println!(
                "[{}] up-to-date: current={} latest={}",
                target_slug, current_version, latest_version
            );
            skipped += 1;
            continue;
        }

        if check_only {
            println!(
                "[{}] upgrade available: current={} latest={}",
                target_slug, current_version, latest_version
            );
            continue;
        }

        println!(
            "[{}] upgrading: {} -> {} ...",
            target_slug, current_version, latest_version
        );

        match api.try_download(&package_url).await {
            Ok((tmp_zip, _sha)) => {
                let preserved_config = config_text.clone();

                match extract::extract_skill_from_zip(
                    tmp_zip.path(),
                    target_slug,
                    &target_dir,
                    true,
                    if expected_sha.is_empty() {
                        None
                    } else {
                        Some(&expected_sha)
                    },
                ) {
                    Ok(_) => {
                        let restored_config_path = target_dir.join(SKILL_CONFIG_NAME);
                        if !restored_config_path.exists() {
                            let _ = std::fs::write(&restored_config_path, &preserved_config)
                                .and_then(|_| set_owner_only_permissions(&restored_config_path));
                        }

                        let meta = serde_json::json!({
                            "version": latest_version,
                            "slug": target_slug,
                        });
                        let meta_path = target_dir.join(SKILL_META_NAME);
                        let meta_content = serde_json::to_string_pretty(&meta).unwrap_or_default();
                        let _ = std::fs::write(&meta_path, &meta_content)
                            .and_then(|_| set_owner_only_permissions(&meta_path));

                        let entry = lock.skills.entry(target_slug.clone()).or_default();
                        entry.slug = target_slug.clone();
                        entry.version = latest_version.clone();
                        entry.zip_url = package_url.clone();
                        entry.update_url = update_url.clone();
                        if entry.name.is_empty() {
                            entry.name = target_slug.clone();
                        }

                        upgraded += 1;
                        println!(
                            "[{}] upgraded: {} -> {}",
                            target_slug, current_version, latest_version
                        );
                    }
                    Err(e) => {
                        println!("[{}] fail: install error: {}", target_slug, e);
                        failed += 1;
                    }
                }
            }
            Err(e) => {
                println!("[{}] fail: download error: {}", target_slug, e);
                failed += 1;
            }
        }
    }

    lockfile::save_lockfile(&skills_dir, &lock)?;

    println!(
        "\nupgrade done: checked={} upgraded={} skipped={} failed={}",
        checked, upgraded, skipped, failed
    );

    if failed > 0 {
        std::process::exit(2);
    }
    Ok(())
}

fn extract_update_url(config: &serde_json::Value) -> String {
    for key in &[
        "update_url",
        "updateUrl",
        "upgrade_url",
        "upgradeUrl",
        "manifest_url",
        "manifestUrl",
    ] {
        if let Some(url) = config.get(key).and_then(|v| v.as_str()) {
            if !url.trim().is_empty() {
                return url.trim().to_string();
            }
        }
    }
    for container in &["update", "upgrade", "autoupdate"] {
        if let Some(nested) = config.get(container) {
            for key in &["url", "uri", "manifest", "manifest_url"] {
                if let Some(url) = nested.get(key).and_then(|v| v.as_str()) {
                    if !url.trim().is_empty() {
                        return url.trim().to_string();
                    }
                }
            }
        }
    }
    String::new()
}
