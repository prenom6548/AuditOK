pub fn version_is_newer(a: &str, b: &str) -> bool {
    let va: Vec<u32> = a.split('.').filter_map(|s| s.parse().ok()).collect();
    let vb: Vec<u32> = b.split('.').filter_map(|s| s.parse().ok()).collect();
    va > vb
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version_is_newer_major() {
        assert!(version_is_newer("2.0.0", "1.9.9"));
        assert!(!version_is_newer("1.0.0", "2.0.0"));
    }

    #[test]
    fn test_version_is_newer_minor() {
        assert!(version_is_newer("1.2.0", "1.1.9"));
        assert!(!version_is_newer("1.1.0", "1.2.0"));
    }

    #[test]
    fn test_version_is_newer_patch() {
        assert!(version_is_newer("1.0.2", "1.0.1"));
        assert!(!version_is_newer("1.0.1", "1.0.2"));
    }

    #[test]
    fn test_version_is_newer_equal() {
        assert!(!version_is_newer("1.0.0", "1.0.0"));
    }

    #[test]
    fn test_version_is_newer_different_lengths() {
        assert!(version_is_newer("1.0.1", "1.0"));
        assert!(!version_is_newer("1.0", "1.0.1"));
    }

    #[test]
    fn test_version_is_newer_non_numeric() {
        assert!(!version_is_newer("abc", "1.0.0"));
        assert!(version_is_newer("1.0.0", "abc"));
    }
}
