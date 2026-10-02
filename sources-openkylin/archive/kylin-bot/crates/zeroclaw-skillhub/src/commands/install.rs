use anyhow::{Context, Result};
use colored::Colorize;

use crate::client::ApiClient;
use crate::config::{
    SKILL_CONFIG_NAME, SKILL_META_NAME, set_owner_only_permissions, skills_install_dir,
};
use crate::extract;
use crate::lockfile;

pub async fn run(
    api: &ApiClient,
    workspace_dir: &std::path::Path,
    source: &str,
    force: bool,
    _timeout_secs: u64,
) -> Result<()> {
    let skills_dir = skills_install_dir(workspace_dir);

    std::fs::create_dir_all(&skills_dir)?;

    let slug = extract_slug(source)?;
    let target_dir = skills_dir.join(&slug);

    println!("Installing from: {}", source);
    println!("Target: {} ({})", slug, target_dir.display());

    let (tmp_zip, remote_sha) = api.download(&slug).await.context("download failed")?;
    println!(
        "Downloaded: {} bytes, sha256={}",
        tmp_zip.path().metadata()?.len(),
        &remote_sha[..16]
    );

    let detail = api.detail(&slug).await.ok();

    extract::extract_skill_from_zip(tmp_zip.path(), &slug, &target_dir, force, Some(&remote_sha))
        .context("install failed")?;

    let (skill_name, skill_version, update_url) = read_skill_meta(&target_dir);

    let meta = serde_json::json!({
        "version": skill_version,
        "slug": slug,
    });
    let meta_path = target_dir.join(SKILL_META_NAME);
    let meta_content = serde_json::to_string_pretty(&meta).unwrap_or_default();
    let _ = std::fs::write(&meta_path, &meta_content)
        .and_then(|_| set_owner_only_permissions(&meta_path));

    let mut lock = lockfile::load_lockfile(&skills_dir);
    let entry = lock.skills.entry(slug.clone()).or_default();
    entry.name = detail
        .as_ref()
        .map(|d| d.skill.displayName.trim())
        .filter(|name| !name.is_empty())
        .map(String::from)
        .or_else(|| (!skill_name.is_empty()).then_some(skill_name))
        .unwrap_or_else(|| slug.clone());
    entry.slug = slug.clone();
    entry.version = skill_version;
    entry.source = source.to_string();
    entry.zip_url = format!("https://api.skillhub.cn/api/v1/download?slug={}", slug);
    entry.update_url = update_url;
    entry.sha256 = remote_sha;
    entry.installed_at = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)?
        .as_secs() as i64;
    entry.detail = detail;

    lockfile::save_lockfile(&skills_dir, &lock).context("save lockfile failed")?;

    println!(
        "\n  {} Skill installed successfully: {}",
        "✓".green().bold(),
        slug
    );
    Ok(())
}

fn read_skill_meta(target_dir: &std::path::Path) -> (String, String, String) {
    let config_path = target_dir.join(SKILL_CONFIG_NAME);
    if !config_path.exists() {
        return (String::new(), String::new(), String::new());
    }

    let text = match std::fs::read_to_string(&config_path) {
        Ok(t) => t,
        Err(_) => return (String::new(), String::new(), String::new()),
    };

    let config: toml::Value = match toml::from_str(&text) {
        Ok(v) => v,
        Err(_) => return (String::new(), String::new(), String::new()),
    };

    let name = config
        .get("name")
        .and_then(|v| v.as_str())
        .unwrap_or("")
        .to_string();
    let version = config
        .get("version")
        .and_then(|v| v.as_str())
        .unwrap_or("")
        .to_string();

    let mut update_url = String::new();
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
                update_url = url.trim().to_string();
                break;
            }
        }
    }

    (name, version, update_url)
}

fn extract_slug(source: &str) -> Result<String> {
    if source.starts_with("clawhub:") {
        let slug = source
            .strip_prefix("clawhub:")
            .unwrap()
            .trim()
            .trim_end_matches('/');
        if slug.is_empty() || slug.contains('/') {
            anyhow::bail!(
                "invalid clawhub source '{}': expected 'clawhub:<slug>'",
                source
            );
        }
        return Ok(slug.to_string());
    }

    if let Ok(parsed) = url::Url::parse(source) {
        match parsed.scheme() {
            "https" | "http" => {}
            _ => anyhow::bail!("invalid URL scheme in source: {}", source),
        }
        let host = parsed.host_str().unwrap_or("");
        if !host.contains("skillhub") && !host.contains("skillhub.cn") && !host.contains("clawhub") && !host.contains("clawhub.ai") {
            anyhow::bail!("not a ClawHub URL: {}", source);
        }
        let path = parsed
            .path_segments()
            .into_iter()
            .flatten()
            .collect::<Vec<_>>();
        if path.is_empty() {
            anyhow::bail!("could not extract slug from URL: {}", source);
        }
        let slug = path.last().unwrap_or(&"");
        if slug.is_empty() {
            anyhow::bail!("could not extract slug from URL: {}", source);
        }
        return Ok(slug.to_string());
    }

    if source.contains('/') || source.contains('\\') || source.contains("..") {
        anyhow::bail!("invalid skill name: {}", source);
    }

    if source.is_empty() {
        anyhow::bail!("empty skill name");
    }

    Ok(source.trim().to_string())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_extract_slug_clawhub_prefix() {
        assert_eq!(extract_slug("clawhub:auto-coder").unwrap(), "auto-coder");
        assert_eq!(extract_slug("clawhub:my-skill").unwrap(), "my-skill");
    }

    #[test]
    fn test_extract_slug_clawhub_prefix_trailing_slash() {
        assert_eq!(extract_slug("clawhub:my-skill/").unwrap(), "my-skill");
    }

    #[test]
    fn test_extract_slug_clawhub_prefix_empty() {
        assert!(extract_slug("clawhub:").is_err());
        assert!(extract_slug("clawhub: ").is_err());
    }

    #[test]
    fn test_extract_slug_clawhub_prefix_with_slash() {
        assert!(extract_slug("clawhub:foo/bar").is_err());
    }

    #[test]
    fn test_extract_slug_skillhub_url() {
        assert_eq!(
            extract_slug("https://www.skillhub.cn/skills/auto-coder").unwrap(),
            "auto-coder"
        );
        assert_eq!(
            extract_slug("https://skillhub.cn/owner/my-skill").unwrap(),
            "my-skill"
        );
    }

    #[test]
    fn test_extract_slug_clawhub_url() {
        assert_eq!(
            extract_slug("https://clawhub.ai/owner/auto-coder").unwrap(),
            "auto-coder"
        );
        assert_eq!(
            extract_slug("https://www.clawhub.ai/owner/my-skill").unwrap(),
            "my-skill"
        );
    }

    #[test]
    fn test_extract_slug_non_clawhub_url() {
        assert!(extract_slug("https://github.com/foo/bar").is_err());
    }

    #[test]
    fn test_extract_slug_plain_name() {
        assert_eq!(extract_slug("auto-coder").unwrap(), "auto-coder");
        assert_eq!(extract_slug("my_skill").unwrap(), "my_skill");
    }

    #[test]
    fn test_extract_slug_empty() {
        assert!(extract_slug("").is_err());
    }

    #[test]
    fn test_extract_slug_with_path_separator() {
        assert!(extract_slug("foo/bar").is_err());
        assert!(extract_slug("foo\\bar").is_err());
        assert!(extract_slug("foo..bar").is_err());
    }

    #[test]
    fn test_extract_slug_trimmed() {
        assert_eq!(extract_slug("  auto-coder  ").unwrap(), "auto-coder");
    }

    #[test]
    fn test_read_skill_meta_with_config() {
        let tmp = tempfile::tempdir().unwrap();
        std::fs::write(
            tmp.path().join("SKILL.toml"),
            r#"
name = "Test Skill"
version = "1.2.3"
update_url = "https://example.com/manifest.json"
"#,
        )
        .unwrap();

        let (name, version, update_url) = read_skill_meta(tmp.path());
        assert_eq!(name, "Test Skill");
        assert_eq!(version, "1.2.3");
        assert_eq!(update_url, "https://example.com/manifest.json");
    }

    #[test]
    fn test_read_skill_meta_no_config() {
        let tmp = tempfile::tempdir().unwrap();
        let (name, version, update_url) = read_skill_meta(tmp.path());
        assert!(name.is_empty());
        assert!(version.is_empty());
        assert!(update_url.is_empty());
    }
}
