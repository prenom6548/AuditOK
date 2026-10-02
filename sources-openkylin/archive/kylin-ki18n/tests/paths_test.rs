// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::paths::ki18n_autotests_dir;
use std::fs;
use std::path::PathBuf;

#[test]
fn test_ki18n_autotests_dir_env_override() {
    let _env_guard = common::env_test_lock();

    let dir = tempfile::tempdir().expect("tempdir");
    fs::write(dir.path().join("test.js"), "// stub").expect("write stub");

    std::env::set_var("KI18N_AUTOTESTS_DIR", dir.path());
    let resolved = ki18n_autotests_dir();
    std::env::remove_var("KI18N_AUTOTESTS_DIR");

    assert_eq!(resolved, PathBuf::from(dir.path()));
    assert!(resolved.join("test.js").is_file());
}

#[test]
fn test_ki18n_autotests_dir_bundled_default() {
    let _env_guard = common::env_test_lock();

    std::env::remove_var("KI18N_AUTOTESTS_DIR");
    let resolved = ki18n_autotests_dir();
    let bundled = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("testdata/ki18n-autotests");
    if bundled.join("test.js").is_file() {
        assert_eq!(resolved, bundled);
    } else {
        assert!(resolved.ends_with("ki18n/autotests"));
    }
}
