// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

//! C ABI for the `ktranscript` Qt plugin (`load_transcript` / `KTranscript::eval`).

use std::collections::HashMap;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::path::{Path, PathBuf};

use crate::capi::Ki18nRsError;
use crate::core::transcript::eval_transcript_js_call_ex;

unsafe fn ptr_to_string(p: *const c_char, allow_empty: bool) -> Result<String, Ki18nRsError> {
    if p.is_null() {
        if allow_empty {
            return Ok(String::new());
        }
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    CStr::from_ptr(p)
        .to_str()
        .map(|s| s.to_string())
        .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)
}

unsafe fn c_str_array_to_vec(ptr: *const *const c_char, count: usize) -> Result<Vec<String>, Ki18nRsError> {
    if count == 0 {
        return Ok(Vec::new());
    }
    if ptr.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let mut out = Vec::with_capacity(count);
    for i in 0..count {
        let p = *ptr.add(i);
        out.push(ptr_to_string(p, false)?);
    }
    Ok(out)
}

/// Evaluate one transcript interpolation (same role as `KTranscriptImp::eval`).
///
/// - `argv`: call name at index 0, arguments follow (UTF-8, pre-formatted like `QVariant` → string).
/// - `mod_paths` / `mod_langs`: parallel arrays of length `mod_count` (module file path + language).
/// - On success: `*out_result` is allocated; free with `ki18n_rs_string_free`.
/// - On script/parse failure: `*out_error` is allocated; free with `ki18n_rs_string_free`.
/// - `*out_fallback` is 1 if the script requested gettext fallback.
#[no_mangle]
pub unsafe extern "C" fn ki18n_rs_ktranscript_eval(
    argv: *const *const c_char,
    argc: usize,
    lang: *const c_char,
    _ctry: *const c_char,
    msgctxt: *const c_char,
    dyn_keys: *const *const c_char,
    dyn_vals: *const *const c_char,
    dyn_count: usize,
    msgid: *const c_char,
    subs: *const *const c_char,
    subs_count: usize,
    vals: *const *const c_char,
    vals_count: usize,
    msgstrf: *const c_char,
    mod_paths: *const *const c_char,
    mod_langs: *const *const c_char,
    mod_count: usize,
    out_result: *mut *mut c_char,
    out_error: *mut *mut c_char,
    out_fallback: *mut c_int,
) -> c_int {
    if out_result.is_null() || out_error.is_null() || out_fallback.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    *out_result = std::ptr::null_mut();
    *out_error = std::ptr::null_mut();
    *out_fallback = 0;

    let run = || -> Result<(), Ki18nRsError> {
        let argv_s = c_str_array_to_vec(argv, argc)?;
        if argv_s.is_empty() {
            let c = CString::new("").map_err(|_| Ki18nRsError::OUT_OF_MEMORY)?;
            *out_result = c.into_raw();
            return Ok(());
        }

        let language = ptr_to_string(lang, false)?;
        let msgctxt_s = ptr_to_string(msgctxt, true)?;
        let msgid_s = ptr_to_string(msgid, true)?;
        let msgstrf_s = ptr_to_string(msgstrf, true)?;

        let keys = c_str_array_to_vec(dyn_keys, dyn_count)?;
        let dvals = c_str_array_to_vec(dyn_vals, dyn_count)?;
        if keys.len() != dvals.len() {
            return Err(Ki18nRsError::INVALID_ARGUMENT);
        }
        let mut dynmap = HashMap::new();
        for (k, v) in keys.into_iter().zip(dvals.into_iter()) {
            dynmap.insert(k, v);
        }

        let subs_s = c_str_array_to_vec(subs, subs_count)?;
        let vals_s = c_str_array_to_vec(vals, vals_count)?;

        let mut paths_owned: Vec<PathBuf> = Vec::new();
        if mod_count > 0 {
            if mod_paths.is_null() || mod_langs.is_null() {
                return Err(Ki18nRsError::INVALID_ARGUMENT);
            }
            for i in 0..mod_count {
                let _mlang = ptr_to_string(*mod_langs.add(i), false)?;
                let p = *mod_paths.add(i);
                let path = ptr_to_string(p, false)?;
                paths_owned.push(PathBuf::from(path));
            }
        }
        let path_refs: Vec<&Path> = paths_owned.iter().map(|p| p.as_path()).collect();

        match eval_transcript_js_call_ex(
            &argv_s,
            &language,
            &msgctxt_s,
            &dynmap,
            &msgid_s,
            &subs_s,
            &vals_s,
            &msgstrf_s,
            &path_refs,
            false,
        ) {
            Ok((s, fb)) => {
                *out_fallback = if fb { 1 } else { 0 };
                let c = CString::new(s).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
                *out_result = c.into_raw();
            }
            Err(e) => {
                let c = CString::new(e).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
                *out_error = c.into_raw();
            }
        }
        Ok(())
    };

    match run() {
        Ok(()) => Ki18nRsError::OK.into(),
        Err(e) => e.into(),
    }
}
