use std::path::Path;

use crate::config::{LOCKFILE_NAME, set_owner_only_permissions};
use crate::models::Lockfile;

pub fn load_lockfile(install_root: &Path) -> Lockfile {
    let path = install_root.join(LOCKFILE_NAME);
    if path.exists() {
        if let Ok(text) = std::fs::read_to_string(&path) {
            if let Ok(lock) = serde_json::from_str(&text) {
                return lock;
            }
        }
    }
    Lockfile::default()
}

pub fn save_lockfile(install_root: &Path, lock: &Lockfile) -> anyhow::Result<()> {
    let path = install_root.join(LOCKFILE_NAME);
    let text = serde_json::to_string_pretty(lock)?;
    let tmp_path = path.with_extension("json.tmp");
    std::fs::write(&tmp_path, &text)?;
    set_owner_only_permissions(&tmp_path)?;
    std::fs::rename(&tmp_path, &path)?;
    set_owner_only_permissions(&path)?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::models::SkillLockEntry;
    use tempfile;

    #[test]
    fn test_load_lockfile_missing() {
        let tmp = tempfile::tempdir().unwrap();
        let lock = load_lockfile(tmp.path());
        assert!(lock.version.is_empty());
        assert!(lock.skills.is_empty());
    }

    #[test]
    fn test_save_and_load_lockfile() {
        let tmp = tempfile::tempdir().unwrap();
        let mut lock = Lockfile {
            version: "1".to_string(),
            ..Default::default()
        };

        let entry = SkillLockEntry {
            name: "test-skill".to_string(),
            slug: "test-skill".to_string(),
            version: "1.0.0".to_string(),
            source: "clawhub:test-skill".to_string(),
            zip_url: "https://api.skillhub.cn/api/v1/download?slug=test-skill".to_string(),
            sha256: "abc123".to_string(),
            installed_at: 1000,
            ..Default::default()
        };
        lock.skills.insert("test-skill".to_string(), entry);

        save_lockfile(tmp.path(), &lock).unwrap();
        let loaded = load_lockfile(tmp.path());

        assert_eq!(loaded.version, "1");
        assert_eq!(loaded.skills.len(), 1);
        let e = loaded.skills.get("test-skill").unwrap();
        assert_eq!(e.slug, "test-skill");
        assert_eq!(e.version, "1.0.0");
        assert_eq!(e.sha256, "abc123");
    }

    #[test]
    fn test_load_lockfile_corrupt() {
        let tmp = tempfile::tempdir().unwrap();
        let path = tmp.path().join(LOCKFILE_NAME);
        std::fs::write(&path, "{invalid json").unwrap();
        let lock = load_lockfile(tmp.path());
        assert!(lock.version.is_empty());
        assert!(lock.skills.is_empty());
    }

    #[test]
    fn test_lockfile_roundtrip_multiple_skills() {
        let tmp = tempfile::tempdir().unwrap();
        let mut lock = Lockfile {
            version: "2".to_string(),
            ..Default::default()
        };

        for i in 0..3 {
            let slug = format!("skill-{}", i);
            let entry = SkillLockEntry {
                name: slug.clone(),
                slug: slug.clone(),
                version: format!("{}.0.0", i + 1),
                installed_at: 1000 + i as i64,
                ..Default::default()
            };
            lock.skills.insert(slug, entry);
        }

        save_lockfile(tmp.path(), &lock).unwrap();
        let loaded = load_lockfile(tmp.path());
        assert_eq!(loaded.skills.len(), 3);
        assert!(loaded.skills.contains_key("skill-0"));
        assert!(loaded.skills.contains_key("skill-1"));
        assert!(loaded.skills.contains_key("skill-2"));
    }

    #[test]
    fn test_save_lockfile_no_tmp_file_left() {
        let tmp = tempfile::tempdir().unwrap();
        let lock = Lockfile::default();
        save_lockfile(tmp.path(), &lock).unwrap();

        let lockfile_path = tmp.path().join(LOCKFILE_NAME);
        assert!(lockfile_path.exists());

        let tmp_path = tmp.path().join("skillhub-lock.json.tmp");
        assert!(
            !tmp_path.exists(),
            "temp file should be cleaned up after rename"
        );
    }

    #[test]
    fn test_save_lockfile_overwrites_existing() {
        let tmp = tempfile::tempdir().unwrap();

        let mut lock1 = Lockfile {
            version: "1".to_string(),
            ..Default::default()
        };
        lock1.skills.insert(
            "old-skill".to_string(),
            SkillLockEntry {
                name: "old-skill".to_string(),
                slug: "old-skill".to_string(),
                version: "1.0.0".to_string(),
                ..Default::default()
            },
        );
        save_lockfile(tmp.path(), &lock1).unwrap();

        let mut lock2 = Lockfile {
            version: "2".to_string(),
            ..Default::default()
        };
        lock2.skills.insert(
            "new-skill".to_string(),
            SkillLockEntry {
                name: "new-skill".to_string(),
                slug: "new-skill".to_string(),
                version: "2.0.0".to_string(),
                ..Default::default()
            },
        );
        save_lockfile(tmp.path(), &lock2).unwrap();

        let loaded = load_lockfile(tmp.path());
        assert_eq!(loaded.version, "2");
        assert!(!loaded.skills.contains_key("old-skill"));
        assert!(loaded.skills.contains_key("new-skill"));
    }

    #[cfg(unix)]
    #[test]
    fn test_save_lockfile_permissions() {
        use std::os::unix::fs::PermissionsExt;
        let tmp = tempfile::tempdir().unwrap();
        let lock = Lockfile {
            version: "1".to_string(),
            ..Default::default()
        };
        save_lockfile(tmp.path(), &lock).unwrap();

        let lockfile_path = tmp.path().join(LOCKFILE_NAME);
        let mode = std::fs::metadata(&lockfile_path)
            .unwrap()
            .permissions()
            .mode()
            & 0o777;
        assert_eq!(mode, 0o600);
    }
}
