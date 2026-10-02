use anyhow::{Context, Result};
use sha2::{Digest, Sha256};
use std::fs;
use std::path::Path;

pub fn safe_extract_zip(zip_path: &Path, target_dir: &Path) -> Result<()> {
    let file = fs::File::open(zip_path).context("open zip file")?;
    let mut archive = zip::ZipArchive::new(file).context("read zip archive")?;

    for i in 0..archive.len() {
        let entry = archive.by_index(i)?;
        let name = entry.name();
        let path = Path::new(name);
        if path.is_absolute() || path.components().any(|c| c.as_os_str() == "..") {
            anyhow::bail!("Unsafe zip path entry detected: {}", name);
        }
    }

    for i in 0..archive.len() {
        let mut entry = archive.by_index(i)?;
        let outpath = target_dir.join(entry.name());

        if entry.is_dir() {
            fs::create_dir_all(&outpath)?;
        } else {
            if let Some(p) = outpath.parent() {
                if !p.exists() {
                    fs::create_dir_all(p)?;
                }
            }
            let mut outfile = fs::File::create(&outpath)?;
            std::io::copy(&mut entry, &mut outfile)?;
        }
    }
    Ok(())
}

pub fn sha256_file(path: &Path) -> Result<String> {
    let mut hasher = Sha256::new();
    let mut file = fs::File::open(path)?;
    std::io::copy(&mut file, &mut hasher)?;
    Ok(hex::encode(hasher.finalize()))
}

pub fn sha256_bytes(data: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(data);
    hex::encode(hasher.finalize())
}

pub fn extract_skill_from_zip(
    zip_path: &Path,
    slug: &str,
    target_dir: &Path,
    force: bool,
    expected_sha256: Option<&str>,
) -> Result<()> {
    if let Some(expected) = expected_sha256 {
        let actual = sha256_file(zip_path)?;
        if !expected.is_empty() && actual != expected {
            anyhow::bail!(
                "SHA256 mismatch for {}: expected {}, got {}",
                slug,
                expected,
                actual
            );
        }
    }

    if target_dir.exists() && !force {
        anyhow::bail!(
            "{} already exists at {:?}. Use --force to overwrite.",
            slug,
            target_dir
        );
    }

    if target_dir.exists() {
        fs::remove_dir_all(target_dir)?;
    }
    fs::create_dir_all(target_dir)?;

    safe_extract_zip(zip_path, target_dir)?;

    flatten_if_needed(target_dir)?;

    Ok(())
}

fn flatten_if_needed(target_dir: &Path) -> Result<()> {
    let entries: Vec<_> = fs::read_dir(target_dir)?.filter_map(|e| e.ok()).collect();

    if entries.len() == 1 && entries[0].path().is_dir() {
        let inner = entries[0].path();
        for entry in fs::read_dir(&inner)? {
            let entry = entry?;
            let dest = target_dir.join(entry.file_name());
            fs::rename(entry.path(), dest)?;
        }
        fs::remove_dir(&inner)?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write as IoWrite;
    use std::path::PathBuf;

    fn create_test_zip(dir: &Path, files: &[(&str, &[u8])]) -> PathBuf {
        let zip_path = dir.join("test.zip");
        let zip_file = fs::File::create(&zip_path).unwrap();
        let mut writer = zip::ZipWriter::new(zip_file);
        let options = zip::write::SimpleFileOptions::default()
            .compression_method(zip::CompressionMethod::Stored);

        for (name, content) in files {
            writer.start_file(name, options).unwrap();
            writer.write_all(content).unwrap();
        }
        writer.finish().unwrap();
        zip_path
    }

    #[test]
    fn test_sha256_file() {
        let tmp = tempfile::tempdir().unwrap();
        let file_path = tmp.path().join("test.txt");
        fs::write(&file_path, b"hello world").unwrap();
        let hash = sha256_file(&file_path).unwrap();
        assert_eq!(hash.len(), 64);
        assert_ne!(hash, "");
    }

    #[test]
    fn test_sha256_file_deterministic() {
        let tmp = tempfile::tempdir().unwrap();
        let file_path = tmp.path().join("test.txt");
        fs::write(&file_path, b"deterministic content").unwrap();
        let hash1 = sha256_file(&file_path).unwrap();
        let hash2 = sha256_file(&file_path).unwrap();
        assert_eq!(hash1, hash2);
    }

    #[test]
    fn test_safe_extract_zip_simple() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(
            tmp.path(),
            &[
                ("SKILL.toml", b"[skill]\nname = \"test\""),
                ("README.md", b"# Test Skill"),
            ],
        );
        let target = tmp.path().join("extracted");
        fs::create_dir_all(&target).unwrap();

        safe_extract_zip(&zip_path, &target).unwrap();

        assert!(target.join("SKILL.toml").exists());
        assert!(target.join("README.md").exists());
        let content = fs::read_to_string(target.join("SKILL.toml")).unwrap();
        assert!(content.contains("test"));
    }

    #[test]
    fn test_safe_extract_zip_with_subdir() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("subdir/nested.txt", b"nested content")]);
        let target = tmp.path().join("extracted");
        fs::create_dir_all(&target).unwrap();

        safe_extract_zip(&zip_path, &target).unwrap();
        assert!(target.join("subdir/nested.txt").exists());
    }

    #[test]
    fn test_safe_extract_zip_path_traversal() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("../etc/passwd", b"malicious")]);
        let target = tmp.path().join("extracted");
        fs::create_dir_all(&target).unwrap();

        let result = safe_extract_zip(&zip_path, &target);
        assert!(result.is_err());
        assert!(result.unwrap_err().to_string().contains("Unsafe"));
    }

    #[test]
    fn test_extract_skill_from_zip_no_force() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("SKILL.toml", b"[skill]\nname = \"test\"")]);
        let target = tmp.path().join("existing-skill");
        fs::create_dir_all(&target).unwrap();

        let result = extract_skill_from_zip(&zip_path, "test", &target, false, None);
        assert!(result.is_err());
        assert!(result.unwrap_err().to_string().contains("already exists"));
    }

    #[test]
    fn test_extract_skill_from_zip_force() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("SKILL.toml", b"[skill]\nname = \"test\"")]);
        let target = tmp.path().join("existing-skill");
        fs::create_dir_all(&target).unwrap();
        fs::write(target.join("old.txt"), "old").unwrap();

        extract_skill_from_zip(&zip_path, "test", &target, true, None).unwrap();
        assert!(target.join("SKILL.toml").exists());
        assert!(!target.join("old.txt").exists());
    }

    #[test]
    fn test_extract_skill_sha256_mismatch() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("SKILL.toml", b"[skill]\nname = \"test\"")]);
        let target = tmp.path().join("new-skill");

        let result =
            extract_skill_from_zip(&zip_path, "test", &target, false, Some("0000deadbeef"));
        assert!(result.is_err());
        assert!(result.unwrap_err().to_string().contains("SHA256 mismatch"));
    }

    #[test]
    fn test_extract_skill_sha256_match() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("SKILL.toml", b"[skill]\nname = \"test\"")]);
        let target = tmp.path().join("new-skill");
        let expected_hash = sha256_file(&zip_path).unwrap();

        extract_skill_from_zip(&zip_path, "test", &target, false, Some(&expected_hash)).unwrap();
        assert!(target.join("SKILL.toml").exists());
    }

    #[test]
    fn test_extract_skill_sha256_empty_string_skips_check() {
        let tmp = tempfile::tempdir().unwrap();
        let zip_path = create_test_zip(tmp.path(), &[("SKILL.toml", b"[skill]\nname = \"test\"")]);
        let target = tmp.path().join("new-skill");

        extract_skill_from_zip(&zip_path, "test", &target, false, Some("")).unwrap();
        assert!(target.join("SKILL.toml").exists());
    }

    #[test]
    fn test_flatten_single_subdir() {
        let tmp = tempfile::tempdir().unwrap();
        let outer = tmp.path().join("skill");
        let inner = outer.join("inner-dir");
        fs::create_dir_all(&inner).unwrap();
        fs::write(inner.join("SKILL.toml"), "test").unwrap();
        fs::write(inner.join("README.md"), "readme").unwrap();

        flatten_if_needed(&outer).unwrap();

        assert!(outer.join("SKILL.toml").exists());
        assert!(outer.join("README.md").exists());
        assert!(!outer.join("inner-dir").exists());
    }

    #[test]
    fn test_no_flatten_multiple_entries() {
        let tmp = tempfile::tempdir().unwrap();
        let outer = tmp.path().join("skill");
        fs::create_dir_all(&outer).unwrap();
        fs::write(outer.join("SKILL.toml"), "test").unwrap();
        fs::create_dir_all(outer.join("subdir")).unwrap();

        flatten_if_needed(&outer).unwrap();

        assert!(outer.join("SKILL.toml").exists());
        assert!(outer.join("subdir").exists());
    }

    #[test]
    fn test_sha256_bytes_empty() {
        let hash = sha256_bytes(b"");
        assert_eq!(
            hash,
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
        );
    }

    #[test]
    fn test_sha256_bytes_hello() {
        let hash = sha256_bytes(b"hello");
        assert_eq!(
            hash,
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"
        );
    }

    #[test]
    fn test_sha256_bytes_consistent_with_sha256_file() {
        let tmp = tempfile::tempdir().unwrap();
        let data = b"test data for consistency check";
        let file_path = tmp.path().join("consistency-test");
        fs::write(&file_path, data).unwrap();

        let from_bytes = sha256_bytes(data);
        let from_file = sha256_file(&file_path).unwrap();
        assert_eq!(from_bytes, from_file);
    }
}
