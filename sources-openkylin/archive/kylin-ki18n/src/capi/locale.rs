// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::path::PathBuf;
use std::ptr;

use crate::capi::Ki18nRsError;
use crate::locale::country::Country as CoreCountry;
use crate::locale::data;
use crate::locale::subdivision::Subdivision as CoreSubdivision;
use crate::locale::timezone as core_tz;

/// 沿可执行文件路径向上搜索常见布局，解决上游 ctest 在 `…/build/autotests/` 下运行时 cwd 无 `data/`、
/// 且 `exe/../share/kylin-ki18n/data` 也不存在的情况（数据实际在源码树 `…/kylin-ki18n/data`）。
const MAX_EXE_ANCESTOR_HOPS: usize = 32;

fn data_dir_candidates_along_exe() -> Vec<PathBuf> {
    let mut out = Vec::new();
    let Ok(exe) = std::env::current_exe() else {
        return out;
    };
    let exe_resolved = std::fs::canonicalize(&exe).unwrap_or(exe);
    for anc in exe_resolved.ancestors().take(MAX_EXE_ANCESTOR_HOPS) {
        out.push(anc.join("share").join("kylin-ki18n").join("data"));
        out.push(anc.join("kylin-ki18n").join("data"));
        out.push(anc.join("data"));
    }
    out
}

fn data_dir() -> PathBuf {
    if let Ok(p) = std::env::var("KI18N_RS_DATA_DIR") {
        return PathBuf::from(p);
    }

    // 编译进库的源码树 data/（cargo build 的 crate 根目录）
    let manifest_data = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("data");
    if has_required_files(&manifest_data) {
        return manifest_data;
    }

    let cwd_candidate = PathBuf::from("data");
    if has_required_files(&cwd_candidate) {
        return cwd_candidate;
    }

    for cand in data_dir_candidates_along_exe() {
        if has_required_files(&cand) {
            return cand;
        }
    }

    let system = PathBuf::from("/usr/share/kylin-ki18n/data");
    if has_required_files(&system) {
        return system;
    }

    PathBuf::from("data")
}

fn has_required_files(dir: &PathBuf) -> bool {
    dir.join("iso_3166_1.json").exists()
        && dir.join("iso_3166_2.json").exists()
        && dir.join("timezone_coords.json").exists()
}

fn c_str_required(ptr: *const c_char) -> Result<String, Ki18nRsError> {
    if ptr.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    unsafe {
        CStr::from_ptr(ptr)
            .to_str()
            .map(|s| s.to_string())
            .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)
    }
}

unsafe fn alloc_c_string(s: &str, out: *mut *mut c_char) -> Result<(), Ki18nRsError> {
    if out.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let cstr = CString::new(s).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
    *out = cstr.into_raw();
    Ok(())
}

unsafe fn alloc_c_string_array(
    list: &[String],
    out_arr: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> Result<(), Ki18nRsError> {
    if out_arr.is_null() || out_count.is_null() {
        return Err(Ki18nRsError::INVALID_ARGUMENT);
    }
    let count = list.len();
    let mut vec_ptrs: Vec<*mut c_char> = Vec::with_capacity(count);
    for s in list {
        let cstr = CString::new(s.as_str()).map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
        vec_ptrs.push(cstr.into_raw());
    }
    let arr = libc::calloc(count, std::mem::size_of::<*mut c_char>()) as *mut *mut c_char;
    if arr.is_null() {
        for p in vec_ptrs {
            if !p.is_null() {
                let _ = CString::from_raw(p);
            }
        }
        return Err(Ki18nRsError::OUT_OF_MEMORY);
    }
    for (i, p) in vec_ptrs.into_iter().enumerate() {
        *arr.add(i) = p;
    }
    *out_arr = arr;
    *out_count = count;
    Ok(())
}

// ----------------------------------------------------------------------------
// 句柄类型
// ----------------------------------------------------------------------------

#[repr(C)]
pub struct Ki18nRsCountryHandle {
    country: CoreCountry,
}

#[repr(C)]
pub struct Ki18nRsSubdivisionHandle {
    subdivision: CoreSubdivision,
}

// ----------------------------------------------------------------------------
// Country API
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_country_from_alpha2(
    alpha2: *const c_char,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let code = match c_str_required(alpha2) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let base = data_dir();
    match CoreCountry::from_alpha2(&base, &code) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_from_alpha3(
    alpha3: *const c_char,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let code = match c_str_required(alpha3) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let base = data_dir();
    match CoreCountry::from_alpha3(&base, &code) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_from_name(
    name: *const c_char,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let name_s = match c_str_required(name) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let base = data_dir();
    match CoreCountry::from_name(&base, &name_s) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_from_location(
    lat: f32,
    lon: f32,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let base = data_dir();
    match CoreCountry::from_location(&base, lat, lon) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

/// 按 **ISO 3166-1 numeric**（整数）查国家。`QLocale::Country` 是 Qt 枚举，请勿直接传入；
/// C++ 侧 `KCountry::fromQLocale` 已改为 `territoryToCode` / `QLocalePrivate::countryToCode` 再走 alpha2。
#[no_mangle]
pub extern "C" fn ki18n_rs_country_from_qlocale(
    country: c_int,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let base = data_dir();
    match CoreCountry::from_qlocale(&base, country as i32) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_all(
    out_list: *mut *mut *mut Ki18nRsCountryHandle,
    out_count: *mut usize,
) -> c_int {
    unsafe {
        if out_list.is_null() || out_count.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        let base = data_dir();
        let list = CoreCountry::all(&base);
        let count = list.len();
        let arr = libc::calloc(count, std::mem::size_of::<*mut Ki18nRsCountryHandle>())
            as *mut *mut Ki18nRsCountryHandle;
        if arr.is_null() {
            return Ki18nRsError::OUT_OF_MEMORY.into();
        }
        for (i, c) in list.into_iter().enumerate() {
            *arr.add(i) = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
        }
        *out_list = arr;
        *out_count = count;
        Ki18nRsError::OK.into()
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_index(handle: *const Ki18nRsCountryHandle) -> c_int {
    if handle.is_null() {
        return -1;
    }
    unsafe { (*handle).country.index() as c_int }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_handle_from_index(
    index: c_int,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() || index < 0 {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let base = data_dir();
    let iso = match data::load_iso_data_cached(&base) {
        Some(d) => d,
        None => {
            unsafe { *out_handle = ptr::null_mut() };
            return Ki18nRsError::NOT_FOUND.into();
        }
    };
    let idx = index as usize;
    if idx >= iso.countries.len() {
        unsafe { *out_handle = ptr::null_mut() };
        return Ki18nRsError::NOT_FOUND.into();
    }
    let c = match CoreCountry::from_data_index(idx) {
        Some(x) => x,
        None => {
            unsafe { *out_handle = ptr::null_mut() };
            return Ki18nRsError::NOT_FOUND.into();
        }
    };
    unsafe {
        *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_destroy(handle: *mut Ki18nRsCountryHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_list_destroy(
    list: *mut *mut Ki18nRsCountryHandle,
    count: usize,
) {
    if list.is_null() {
        return;
    }
    unsafe {
        for i in 0..count {
            let elem = *list.add(i);
            if !elem.is_null() {
                let _ = Box::from_raw(elem);
            }
        }
        libc::free(list as *mut libc::c_void);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_alpha2(
    handle: *const Ki18nRsCountryHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let s = (*handle).country.alpha2(&base);
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_alpha3(
    handle: *const Ki18nRsCountryHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let s = (*handle).country.alpha3(&base);
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_name(
    handle: *const Ki18nRsCountryHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let s = (*handle).country.name(&base);
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_emoji_flag(
    handle: *const Ki18nRsCountryHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let s = (*handle).country.emoji_flag(&base);
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_currency_code(
    handle: *const Ki18nRsCountryHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        match (*handle).country.currency_code(&base) {
            Some(code) => match alloc_c_string(&code, out) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            None => {
                if !out.is_null() {
                    *out = ptr::null_mut();
                }
                Ki18nRsError::NOT_FOUND.into()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_timezones(
    handle: *const Ki18nRsCountryHandle,
    out_ids: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let list = (*handle).country.timezones(&base);
        match alloc_c_string_array(&list, out_ids, out_count) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_country_subdivisions(
    handle: *const Ki18nRsCountryHandle,
    out_list: *mut *mut *mut Ki18nRsSubdivisionHandle,
    out_count: *mut usize,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        if out_list.is_null() || out_count.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        let base = data_dir();
        let list = (*handle).country.subdivisions(&base);
        let count = list.len();
        let arr = libc::calloc(count, std::mem::size_of::<*mut Ki18nRsSubdivisionHandle>())
            as *mut *mut Ki18nRsSubdivisionHandle;
        if arr.is_null() {
            return Ki18nRsError::OUT_OF_MEMORY.into();
        }
        for (i, s) in list.into_iter().enumerate() {
            *arr.add(i) =
                Box::into_raw(Box::new(Ki18nRsSubdivisionHandle { subdivision: s }));
        }
        *out_list = arr;
        *out_count = count;
        Ki18nRsError::OK.into()
    }
}

// ----------------------------------------------------------------------------
// Subdivision API
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_from_code(
    code: *const c_char,
    out_handle: *mut *mut Ki18nRsSubdivisionHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let code_s = match c_str_required(code) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let base = data_dir();
    match CoreSubdivision::from_code(&base, &code_s) {
        Some(s) => unsafe {
            *out_handle =
                Box::into_raw(Box::new(Ki18nRsSubdivisionHandle { subdivision: s }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_from_location(
    lat: f32,
    lon: f32,
    out_handle: *mut *mut Ki18nRsSubdivisionHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let base = data_dir();
    match CoreSubdivision::from_location(&base, lat, lon) {
        Some(s) => unsafe {
            *out_handle =
                Box::into_raw(Box::new(Ki18nRsSubdivisionHandle { subdivision: s }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_destroy(handle: *mut Ki18nRsSubdivisionHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_index(handle: *const Ki18nRsSubdivisionHandle) -> c_int {
    if handle.is_null() {
        return -1;
    }
    unsafe {
        let idx = (*handle).subdivision.index();
        if idx > i32::MAX as u32 {
            -1
        } else {
            idx as c_int
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_handle_from_index(
    index: c_int,
    out_handle: *mut *mut Ki18nRsSubdivisionHandle,
) -> c_int {
    if out_handle.is_null() || index < 0 {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let base = data_dir();
    match CoreSubdivision::from_data_index(&base, index as usize) {
        Some(s) => unsafe {
            *out_handle =
                Box::into_raw(Box::new(Ki18nRsSubdivisionHandle { subdivision: s }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_list_destroy(
    list: *mut *mut Ki18nRsSubdivisionHandle,
    count: usize,
) {
    if list.is_null() {
        return;
    }
    unsafe {
        for i in 0..count {
            let elem = *list.add(i);
            if !elem.is_null() {
                let _ = Box::from_raw(elem);
            }
        }
        libc::free(list as *mut libc::c_void);
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_code(
    handle: *const Ki18nRsSubdivisionHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let s = (*handle).subdivision.code().to_string();
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_name(
    handle: *const Ki18nRsSubdivisionHandle,
    out: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let s = (*handle).subdivision.name();
        match alloc_c_string(&s, out) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_country(
    handle: *const Ki18nRsSubdivisionHandle,
    out: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if handle.is_null() || out.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let c = (*handle).subdivision.country(&base);
        *out = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
        Ki18nRsError::OK.into()
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_parent(
    handle: *const Ki18nRsSubdivisionHandle,
    out: *mut *mut Ki18nRsSubdivisionHandle,
) -> c_int {
    if handle.is_null() || out.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        match (*handle).subdivision.parent(&base) {
            Some(parent) => {
                *out = Box::into_raw(Box::new(Ki18nRsSubdivisionHandle {
                    subdivision: parent,
                }));
                Ki18nRsError::OK.into()
            }
            None => {
                *out = ptr::null_mut();
                Ki18nRsError::NOT_FOUND.into()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_subdivisions(
    handle: *const Ki18nRsSubdivisionHandle,
    out_list: *mut *mut *mut Ki18nRsSubdivisionHandle,
    out_count: *mut usize,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        if out_list.is_null() || out_count.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        let base = data_dir();
        let list = (*handle).subdivision.subdivisions(&base);
        let count = list.len();
        let arr = libc::calloc(count, std::mem::size_of::<*mut Ki18nRsSubdivisionHandle>())
            as *mut *mut Ki18nRsSubdivisionHandle;
        if arr.is_null() {
            return Ki18nRsError::OUT_OF_MEMORY.into();
        }
        for (i, s) in list.into_iter().enumerate() {
            *arr.add(i) =
                Box::into_raw(Box::new(Ki18nRsSubdivisionHandle { subdivision: s }));
        }
        *out_list = arr;
        *out_count = count;
        Ki18nRsError::OK.into()
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_subdivision_timezones(
    handle: *const Ki18nRsSubdivisionHandle,
    out_ids: *mut *mut *mut c_char,
    out_count: *mut usize,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        let base = data_dir();
        let tzs = (*handle).subdivision.timezones(&base);
        match alloc_c_string_array(&tzs, out_ids, out_count) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

// ----------------------------------------------------------------------------
// TimeZone API
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_timezone_from_location(
    lat: f32,
    lon: f32,
    out_iana_id: *mut *mut c_char,
) -> c_int {
    unsafe {
        if out_iana_id.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        let base = data_dir();
        match core_tz::from_location(&base, lat, lon) {
            Some(id) => match alloc_c_string(&id, out_iana_id) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            None => {
                *out_iana_id = ptr::null_mut();
                Ki18nRsError::NOT_FOUND.into()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_timezone_country(
    iana_id: *const c_char,
    out_handle: *mut *mut Ki18nRsCountryHandle,
) -> c_int {
    if out_handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let id = match c_str_required(iana_id) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let base = data_dir();
    match core_tz::country(&base, &id) {
        Some(c) => unsafe {
            *out_handle = Box::into_raw(Box::new(Ki18nRsCountryHandle { country: c }));
            Ki18nRsError::OK.into()
        },
        None => unsafe {
            *out_handle = ptr::null_mut();
            Ki18nRsError::NOT_FOUND.into()
        },
    }
}

