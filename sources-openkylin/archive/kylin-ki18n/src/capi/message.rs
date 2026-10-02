// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashMap;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;

use crate::capi::Ki18nRsError;
use crate::core::global as core_global;
use crate::core::kuit::VisualFormat;
use crate::core::message::Message;

#[repr(C)]
pub struct Ki18nRsMsgHandle {
    message: Message,
}

fn c_str_opt(ptr: *const c_char) -> Result<Option<String>, Ki18nRsError> {
    if ptr.is_null() {
        return Ok(None);
    }
    unsafe {
        let s = CStr::from_ptr(ptr)
            .to_str()
            .map_err(|_| Ki18nRsError::INVALID_ARGUMENT)?;
        Ok(Some(s.to_string()))
    }
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

// ----------------------------------------------------------------------------
// 消息创建与销毁
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_create(
    context: *const c_char,
    text: *const c_char,
    plural: *const c_char,
) -> *mut Ki18nRsMsgHandle {
    let text_s = match c_str_required(text) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };
    let context_s = match c_str_opt(context) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };
    let plural_s = match c_str_opt(plural) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    let msg = Message::new(&text_s, plural_s.as_deref(), context_s.as_deref(), false);
    Box::into_raw(Box::new(Ki18nRsMsgHandle { message: msg }))
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_create_markup(
    context: *const c_char,
    text: *const c_char,
    plural: *const c_char,
) -> *mut Ki18nRsMsgHandle {
    let text_s = match c_str_required(text) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };
    let context_s = match c_str_opt(context) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };
    let plural_s = match c_str_opt(plural) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    let mut msg = Message::new(&text_s, plural_s.as_deref(), context_s.as_deref(), true);
    msg.markup_aware = true;
    Box::into_raw(Box::new(Ki18nRsMsgHandle { message: msg }))
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_clone(handle: *const Ki18nRsMsgHandle) -> *mut Ki18nRsMsgHandle {
    if handle.is_null() {
        return ptr::null_mut();
    }
    unsafe {
        let msg = &*handle;
        Box::into_raw(Box::new(Ki18nRsMsgHandle {
            message: msg.message.clone(),
        }))
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_destroy(handle: *mut Ki18nRsMsgHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(handle);
    }
}

// ----------------------------------------------------------------------------
// 消息配置
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_set_domain(
    handle: *mut Ki18nRsMsgHandle,
    domain: *const c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let domain_s = match c_str_required(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        (*handle).message.domain = Some(domain_s);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_set_languages(
    handle: *mut Ki18nRsMsgHandle,
    langs: *const *const c_char,
    count: usize,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    if langs.is_null() && count > 0 {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let mut list = Vec::with_capacity(count);
    for i in 0..count {
        let ptr = unsafe { *langs.add(i) };
        match c_str_required(ptr) {
            Ok(s) => list.push(s),
            Err(e) => return e.into(),
        }
    }
    unsafe {
        (*handle).message.languages = list;
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_set_format(
    handle: *mut Ki18nRsMsgHandle,
    visual_format: c_int,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let vf = match visual_format {
        10 => VisualFormat::PlainText,
        20 => VisualFormat::RichText,
        30 => VisualFormat::TermText,
        0 | _ => VisualFormat::UndefinedFormat,
    };
    unsafe {
        (*handle).message.format = vf;
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_add_context(
    handle: *mut Ki18nRsMsgHandle,
    key: *const c_char,
    value: *const c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let key_s = match c_str_required(key) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let value_s = match c_str_required(value) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        let map: &mut HashMap<String, String> = &mut (*handle).message.in_context;
        map.insert(key_s, value_s);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_set_relax_subs(
    handle: *mut Ki18nRsMsgHandle,
    enabled: c_int,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        (*handle).message.relax_subs = enabled != 0;
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_set_ignore_markup(
    handle: *mut Ki18nRsMsgHandle,
    enabled: c_int,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        (*handle).message.ignore_markup = enabled != 0;
    }
    Ki18nRsError::OK.into()
}

// ----------------------------------------------------------------------------
// 占位符替换
// ----------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_i64(
    handle: *mut Ki18nRsMsgHandle,
    value: i64,
    field_width: c_int,
    base: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ch = std::char::from_u32(fill_char).unwrap_or(' ');
    let base_u32 = if base <= 0 { 10 } else { base as u32 };
    unsafe {
        (*handle)
            .message
            .subs_i64(value, field_width as i32, base_u32, ch);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_i32(
    handle: *mut Ki18nRsMsgHandle,
    value: i32,
    field_width: c_int,
    base: c_int,
    fill_char: u32,
) -> c_int {
    ki18n_rs_msg_subs_i64(handle, value as i64, field_width, base, fill_char)
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_u64(
    handle: *mut Ki18nRsMsgHandle,
    value: u64,
    field_width: c_int,
    base: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ch = std::char::from_u32(fill_char).unwrap_or(' ');
    let base_u32 = if base <= 0 { 10 } else { base as u32 };
    unsafe {
        (*handle)
            .message
            .subs_u64(value, field_width as i32, base_u32, ch);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_u32(
    handle: *mut Ki18nRsMsgHandle,
    value: u32,
    field_width: c_int,
    base: c_int,
    fill_char: u32,
) -> c_int {
    ki18n_rs_msg_subs_u64(handle, value as u64, field_width, base, fill_char)
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_f64(
    handle: *mut Ki18nRsMsgHandle,
    value: f64,
    field_width: c_int,
    format: c_char,
    precision: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ch = std::char::from_u32(fill_char).unwrap_or(' ');
    let fmt_char = if format == 0 { 'g' } else { format as u8 as char };
    unsafe {
        (*handle).message.subs_f64(
            value,
            field_width as i32,
            fmt_char,
            precision as i32,
            ch,
        );
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_f32(
    handle: *mut Ki18nRsMsgHandle,
    value: f32,
    field_width: c_int,
    format: c_char,
    precision: c_int,
    fill_char: u32,
) -> c_int {
    ki18n_rs_msg_subs_f64(
        handle,
        value as f64,
        field_width,
        format,
        precision,
        fill_char,
    )
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_qstring(
    handle: *mut Ki18nRsMsgHandle,
    utf8: *const c_char,
    field_width: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let s = match c_str_required(utf8) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    let ch = std::char::from_u32(fill_char).unwrap_or(' ');
    unsafe {
        (*handle)
            .message
            .subs_string(&s, field_width as i32, ch);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_char(
    handle: *mut Ki18nRsMsgHandle,
    char_utf32: u32,
    field_width: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let ch = std::char::from_u32(char_utf32).unwrap_or('\u{FFFD}');
    let fill = std::char::from_u32(fill_char).unwrap_or(' ');
    unsafe {
        (*handle)
            .message
            .subs_char(ch, field_width as i32, fill);
    }
    Ki18nRsError::OK.into()
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_subs_message(
    handle: *mut Ki18nRsMsgHandle,
    other: *const Ki18nRsMsgHandle,
    field_width: c_int,
    fill_char: u32,
) -> c_int {
    if handle.is_null() || other.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let fill = std::char::from_u32(fill_char).unwrap_or(' ');
    unsafe {
        let other_msg = (*other).message.clone();
        (*handle)
            .message
            .subs_message(other_msg, field_width as i32, fill);
    }
    Ki18nRsError::OK.into()
}

// ----------------------------------------------------------------------------
// 最终转换
// ----------------------------------------------------------------------------

fn resolve_domain_and_langs(
    msg: &Message,
    override_domain: Option<&str>,
    override_langs: Option<&[String]>,
) -> (Option<String>, Vec<String>) {
    let domain = override_domain
        .map(|s| s.to_string())
        .or_else(|| msg.domain.clone())
        .or_else(core_global::get_application_domain);

    let langs = if let Some(l) = override_langs {
        l.to_vec()
    } else if !msg.languages.is_empty() {
        msg.languages.clone()
    } else {
        core_global::effective_languages()
    };

    (domain, langs)
}

/// `ki18n_rs_msg_to_string*` 与 `ki18n_rs_context_translate` 共用。
pub(crate) fn translate_with_domain_and_langs(
    msg: &Message,
    override_domain: Option<&str>,
    override_langs: Option<&[String]>,
) -> Result<String, Ki18nRsError> {
    let (domain_opt, langs) = resolve_domain_and_langs(msg, override_domain, override_langs);
    let domain = domain_opt.unwrap_or_default();

    if langs.is_empty() {
        let text = msg.select_plural_text();
        return Ok(msg.apply_subs(&text));
    }

    let code = core_global::code_language();
    for lang in &langs {
        if lang == &code {
            break;
        }
        if let Some(catalog) = core_global::get_or_load_catalog(&domain, lang) {
            if msg.has_translation_in_catalog(&catalog) {
                match msg.finalize(&catalog, &langs) {
                    Ok(s) => return Ok(s),
                    Err(_) => continue,
                }
            }
        }
    }

    let empty = crate::core::catalog::Catalog::empty_fallback();
    match msg.finalize(&empty, &langs) {
        Ok(s) => Ok(s),
        Err(_) => {
            let text = msg.select_plural_text();
            Ok(msg.apply_subs(&text))
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_to_string(
    handle: *const Ki18nRsMsgHandle,
    out_utf8: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = ptr::null_mut();
        let msg = &(*handle).message;
        match translate_with_domain_and_langs(msg, None, None) {
            Ok(s) => match alloc_c_string(&s, out_utf8) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_get_untranslated_text(
    handle: *const Ki18nRsMsgHandle,
    out_utf8: *mut *mut c_char,
) -> c_int {
    if handle.is_null() || out_utf8.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        *out_utf8 = ptr::null_mut();
        let msg = &(*handle).message;
        let text = msg.text.clone();
        match alloc_c_string(&text, out_utf8) {
            Ok(()) => Ki18nRsError::OK.into(),
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_to_string_with_langs(
    handle: *const Ki18nRsMsgHandle,
    langs: *const *const c_char,
    count: usize,
    out_utf8: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    if langs.is_null() && count > 0 {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = ptr::null_mut();
        let mut lang_vec = Vec::with_capacity(count);
        for i in 0..count {
            let ptr = *langs.add(i);
            match c_str_required(ptr) {
                Ok(s) => lang_vec.push(s),
                Err(e) => return e.into(),
            }
        }
        let msg = &(*handle).message;
        match translate_with_domain_and_langs(msg, None, Some(&lang_vec)) {
            Ok(s) => match alloc_c_string(&s, out_utf8) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            Err(e) => e.into(),
        }
    }
}

#[no_mangle]
pub extern "C" fn ki18n_rs_msg_to_string_with_domain(
    handle: *const Ki18nRsMsgHandle,
    domain: *const c_char,
    out_utf8: *mut *mut c_char,
) -> c_int {
    if handle.is_null() {
        return Ki18nRsError::INVALID_ARGUMENT.into();
    }
    let domain_s = match c_str_required(domain) {
        Ok(s) => s,
        Err(e) => return e.into(),
    };
    unsafe {
        if out_utf8.is_null() {
            return Ki18nRsError::INVALID_ARGUMENT.into();
        }
        *out_utf8 = ptr::null_mut();
        let msg = &(*handle).message;
        match translate_with_domain_and_langs(msg, Some(&domain_s), None) {
            Ok(s) => match alloc_c_string(&s, out_utf8) {
                Ok(()) => Ki18nRsError::OK.into(),
                Err(e) => e.into(),
            },
            Err(e) => e.into(),
        }
    }
}

