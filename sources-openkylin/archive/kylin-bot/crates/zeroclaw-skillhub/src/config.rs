use std::path::{Path, PathBuf};

pub const LOCKFILE_NAME: &str = "skillhub-lock.json";
pub const SKILL_CONFIG_NAME: &str = "SKILL.toml";
pub const SKILL_META_NAME: &str = ".skill-meta.json";

#[cfg(unix)]
pub fn set_owner_only_permissions(path: &Path) -> std::io::Result<()> {
    use std::os::unix::fs::PermissionsExt;
    std::fs::set_permissions(path, std::fs::Permissions::from_mode(0o600))
}

#[cfg(not(unix))]
pub fn set_owner_only_permissions(_path: &Path) -> std::io::Result<()> {
    Ok(())
}

pub fn skills_install_dir(workspace_dir: &Path) -> PathBuf {
    workspace_dir.join("skills")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_skills_install_dir() {
        let workspace = PathBuf::from("/tmp/test-workspace");
        let skills = skills_install_dir(&workspace);
        assert_eq!(skills, PathBuf::from("/tmp/test-workspace/skills"));
    }

    #[test]
    fn test_set_owner_only_permissions_on_existing_file() {
        let tmp = tempfile::tempdir().unwrap();
        let file_path = tmp.path().join("test-file");
        std::fs::write(&file_path, b"test").unwrap();
        assert!(set_owner_only_permissions(&file_path).is_ok());
        assert!(file_path.exists());
    }

    #[test]
    fn test_set_owner_only_permissions_on_nonexistent_path() {
        let path = PathBuf::from("/tmp/nonexistent_file_for_permission_test_12345");
        assert!(set_owner_only_permissions(&path).is_err());
    }

    #[cfg(unix)]
    #[test]
    fn test_set_owner_only_permissions_unix_mode() {
        use std::os::unix::fs::PermissionsExt;
        let tmp = tempfile::tempdir().unwrap();
        let file_path = tmp.path().join("mode-test");
        std::fs::write(&file_path, b"test").unwrap();
        set_owner_only_permissions(&file_path).unwrap();
        let mode = std::fs::metadata(&file_path).unwrap().permissions().mode() & 0o777;
        assert_eq!(mode, 0o600);
    }
}
