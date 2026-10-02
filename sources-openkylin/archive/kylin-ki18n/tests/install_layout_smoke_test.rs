// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use ki18n_rs::capi::{global::ki18n_rs_string_free, locale::Ki18nRsCountryHandle, Ki18nRsError};
use std::fs;
use std::path::{Path, PathBuf};

fn copy_dir_recursive(from: &Path, to: &Path) {
    fs::create_dir_all(to).expect("create dir");
    for entry in fs::read_dir(from).expect("read_dir") {
        let entry = entry.expect("entry");
        let path = entry.path();
        if path.is_file() {
            let file_name = entry.file_name();
            fs::copy(&path, to.join(file_name)).expect("copy file");
        }
    }
}

#[test]
fn install_layout_smoke_without_env_var() {
    // This test simulates an installed layout by placing JSON files into
    // <current_exe_parent>/share/kylin-ki18n/data/ and verifying Rust locale can find them
    // when KI18N_RS_DATA_DIR is not set.
    std::env::remove_var("KI18N_RS_DATA_DIR");

    let manifest_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    let repo_data_dir = manifest_dir.join("data");
    let exe = std::env::current_exe().expect("current_exe");
    let exe_dir = exe.parent().expect("exe parent");
    let prefix = exe_dir.parent().expect("prefix parent");

    let installed_data_dir = prefix.join("share").join("kylin-ki18n").join("data");
    copy_dir_recursive(&repo_data_dir, &installed_data_dir);

    // Call C ABI, which internally resolves data_dir().
    let mut handle: *mut Ki18nRsCountryHandle = std::ptr::null_mut();
    let code = std::ffi::CString::new("DE").unwrap();
    let res = ki18n_rs::capi::locale::ki18n_rs_country_from_alpha2(code.as_ptr(), &mut handle);
    assert_eq!(res, Ki18nRsError::OK as i32);
    assert!(!handle.is_null());

    ki18n_rs::capi::locale::ki18n_rs_country_destroy(handle);

    // Also verify timezone lookup works without KI18N_RS_DATA_DIR set.
    // ki18n's KTimeZoneTest expects VN (21,106) to map to Asia/Bangkok.
    let mut tz_id_ptr: *mut std::os::raw::c_char = std::ptr::null_mut();
    let res = ki18n_rs::capi::locale::ki18n_rs_timezone_from_location(21.0, 106.0, &mut tz_id_ptr);
    assert_eq!(res, Ki18nRsError::OK as i32);
    let tz_id = unsafe {
        let s = std::ffi::CStr::from_ptr(tz_id_ptr)
            .to_str()
            .unwrap()
            .to_string();
        ki18n_rs_string_free(tz_id_ptr);
        s
    };
    assert_eq!(tz_id, "Asia/Bangkok");
}

