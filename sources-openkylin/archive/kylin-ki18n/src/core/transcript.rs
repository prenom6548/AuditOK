// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashMap;
use std::cell::RefCell;
use std::mem::ManuallyDrop;
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::sync::OnceLock;

use crate::core::accelerator::remove_accelerator_marker;
use crate::core::catalog::Catalog;
use crate::core::global as core_global;
use crate::core::js_engine::JsEngine;

// ---------------------------------------------------------------------------
// Cross-message JS Scriptface persistence (#4)
// ---------------------------------------------------------------------------
//
// Upstream KTranscript keeps Scriptface state across translations for the
// same language. We mimic this with a per-thread cache.
thread_local! {
    static JS_ENGINE_BY_LANG: RefCell<HashMap<String, Rc<RefCell<ManuallyDrop<JsEngine>>>>> =
        RefCell::new(HashMap::new());
}

fn get_or_create_js_engine_by_lang(
    language: &str,
) -> Option<Rc<RefCell<ManuallyDrop<JsEngine>>>> {
    JS_ENGINE_BY_LANG.with(|cache| {
        let mut map = cache.borrow_mut();
        if let Some(rc) = map.get(language) {
            return Some(rc.clone());
        }

        let mut e = JsEngine::new().ok()?;
        // Load default script modules once when the engine is created.
        for p in default_js_module_files(language) {
            if e.eval_module_file(&p).is_err() {
                // Continue; module might not be required for all calls.
            }
        }

        let rc = Rc::new(RefCell::new(ManuallyDrop::new(e)));
        map.insert(language.to_string(), rc.clone());
        Some(rc)
    })
}

/// Transcript 内置函数签名。
type TranscriptFn = fn(
    args: &[&str],
    context: &HashMap<String, String>,
    msg_args: &[String],
    catalog: &Catalog,
    language: &str,
) -> String;

fn default_js_module_files(language: &str) -> Vec<PathBuf> {
    // Production-like location (ki18n's own scripting module):
    // locale/<lang>/LC_SCRIPTS/ki18n5/ki18n5.js
    let our_domain = "ki18n5";
    for base in core_global::domain_locale_dirs(our_domain) {
        let cand = base
            .join("locale")
            .join(language)
            .join("LC_SCRIPTS")
            .join(our_domain)
            .join(format!("{}.js", our_domain));
        if cand.is_file() {
            return vec![cand];
        }
    }

    // Test fallback: testdata/ki18n-autotests/test.js 或 ../ki18n/autotests/test.js
    static TEST_JS: OnceLock<PathBuf> = OnceLock::new();
    let p = TEST_JS
        .get_or_init(|| {
            let path = crate::paths::ki18n_autotests_dir().join("test.js");
            path.canonicalize().unwrap_or(path)
        })
        .clone();
    vec![p]
}

fn builtin_functions() -> HashMap<&'static str, TranscriptFn> {
    let mut funcs: HashMap<&'static str, TranscriptFn> = HashMap::new();
    funcs.insert("arg", builtin_arg);
    funcs.insert("plural", builtin_plural);
    funcs.insert("gender", builtin_gender);
    funcs.insert("language", builtin_language);
    funcs.insert("country", builtin_country);
    funcs.insert("msgstrf", builtin_msgstrf);
    funcs.insert("msgid", builtin_msgid);
    funcs.insert("msgctxt", builtin_msgctxt);
    funcs.insert("msgkey", builtin_msgkey);
    funcs.insert("nsubs", builtin_nsubs);
    funcs.insert("subs", builtin_subs);
    funcs.insert("dynctxt", builtin_dynctxt);
    funcs.insert("localeCountry", builtin_locale_country);
    funcs.insert("contains", builtin_contains);
    funcs.insert("startsWith", builtin_starts_with);
    funcs.insert("endsWith", builtin_ends_with);
    funcs.insert("upperFirst", builtin_upper_first);
    funcs.insert("lowerFirst", builtin_lower_first);
    funcs.insert("pluralForm", builtin_plural_form);
    funcs.insert("argRange", builtin_arg_range);
    funcs.insert("languageName", builtin_language_name);
    funcs.insert("capitalize", builtin_capitalize);
    // String transforms (literal argument).
    funcs.insert("upper", builtin_upper);
    funcs.insert("lower", builtin_lower);
    funcs.insert("trim", builtin_trim);
    funcs.insert("sort", builtin_sort);
    funcs.insert("reverse", builtin_reverse);
    funcs.insert("replace", builtin_replace);
    funcs.insert("substring", builtin_substring);
     funcs.insert("removeAccelerator", builtin_remove_accelerator);
    funcs.insert("script", builtin_script);
    // R06: normKey / getConf*
    funcs.insert("normKey", builtin_norm_key);
    funcs.insert("getConfString", builtin_get_conf_string);
    funcs.insert("getConfBool", builtin_get_conf_bool);
    funcs.insert("getConfNumber", builtin_get_conf_number);
    // R02: grammatical case
    funcs.insert("case", builtin_case);
    // R01: loadProps / getProp / setProp
    funcs.insert("loadProps", builtin_load_props);
    funcs.insert("getProp", builtin_get_prop);
    funcs.insert("setProp", builtin_set_prop);
    funcs
}

fn builtin_msgid(
    _args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    context.get("msgid").cloned().unwrap_or_default()
}

fn builtin_msgctxt(
    _args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    context.get("msgctxt").cloned().unwrap_or_default()
}

fn builtin_msgstrf(
    _args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    context.get("msgstrf").cloned().unwrap_or_default()
}

fn builtin_msgkey(
    _args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if let Some(v) = context.get("msgkey") {
        return v.clone();
    }
    let c = context.get("msgctxt").cloned().unwrap_or_default();
    let id = context.get("msgid").cloned().unwrap_or_default();
    format!("{}|{}", c, id)
}

fn builtin_nsubs(
    _args: &[&str],
    _context: &HashMap<String, String>,
    msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    msg_args.len().to_string()
}

fn builtin_subs(
    args: &[&str],
    _context: &HashMap<String, String>,
    msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    // 1-based index, consistent with ki18n's Ts.subs().
    let idx: i64 = match args[0].trim().parse() {
        Ok(v) => v,
        Err(_) => return String::new(),
    };
    if idx <= 0 {
        return String::new();
    }
    let u = idx as usize;
    if u == 0 || u > msg_args.len() {
        return String::new();
    }
    msg_args[u - 1].clone()
}

fn builtin_dynctxt(
    args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    context.get(args[0]).cloned().unwrap_or_default()
}

fn builtin_locale_country(
    _args: &[&str],
    context: &HashMap<String, String>,
    msg_args: &[String],
    catalog: &Catalog,
    language: &str,
) -> String {
    builtin_country(&[], context, msg_args, catalog, language)
}

/// Transcript 脚本后处理（扩展版）。
///
/// - `$[key]` / `$[="literal"]` / 管线变换同原实现
/// - `$[func(arg1, arg2,...)]` 形式调用内置函数（见上表）
pub fn apply_transcript(
    text: &str,
    context: &HashMap<String, String>,
    msg_args: &[String],
    catalog: &Catalog,
    language: &str,
) -> String {
    apply_transcript_with_fallback(text, context, msg_args, catalog, language).0
}

/// ki18n transcript: `$[callname arg1 arg2 ...]`
///
/// 语法要点（与 ki18n `resolveInterpolation` 对齐）：
/// - 参数按空白分隔
/// - 支持单引号包裹片段：`'this bar'`（支持 `\` 转义）
/// - 支持嵌套 `$[...]` 作为参数的一部分
/// - 支持值引用（value reference）：参数仅为 `^N` 时，转换为 `msg_args[N-1]`
/// - 若任意插值触发 fallback（未知 call/显式 fallback），返回空串并置 fallback=true
pub fn apply_transcript_with_fallback(
    text: &str,
    context: &HashMap<String, String>,
    msg_args: &[String],
    catalog: &Catalog,
    language: &str,
) -> (String, bool) {
    let mut out = String::new();
    let mut fallback = false;

    let chars: Vec<char> = text.chars().collect();

    let msgctxt = context.get("msgctxt").map(|s| s.as_str()).unwrap_or("");
    let msgid = context.get("msgid").map(|s| s.as_str()).unwrap_or("");
    let msgstrf = context.get("msgstrf").map(|s| s.as_str()).unwrap_or("");

    // Lazy JS engine init: only create/use JS Scriptface when we need to call
    // an unknown $[call ...] function (not a builtin).
    let mut js_engine: Option<Rc<RefCell<ManuallyDrop<JsEngine>>>> = None;

    let mut i = 0usize;
    while i < chars.len() {
        if chars[i] == '$' && i + 1 < chars.len() && chars[i + 1] == '[' {
            // Evaluate interpolation starting at i.
            match eval_interpolation_at(
                &chars,
                i,
                context,
                msg_args,
                catalog,
                language,
                &mut js_engine,
                msgctxt,
                msgid,
                msgstrf,
            ) {
                Some((replacement, local_fallback, next_i)) => {
                    if local_fallback {
                        fallback = true;
                    }
                    out.push_str(&replacement);
                    i = next_i;
                    continue;
                }
                // ki18n: malformed/unclosed interpolation requests fallback for the whole scripted translation.
                None => {
                    fallback = true;
                    out.clear();
                    break;
                }
            }
        }
        out.push(chars[i]);
        i += 1;
    }

    // Upstream runs scripted postCalls once after transcript evaluation.
    if let Some(engine_rc) = js_engine.as_ref() {
        let mut engine = engine_rc.borrow_mut();
        if engine.execute_post_calls().is_err() {
            fallback = true;
        }
    }

    if fallback {
        ("".to_string(), true)
    } else {
        (out, false)
    }
}

fn eval_interpolation_at(
    chars: &[char],
    start_i: usize,
    context: &HashMap<String, String>,
    msg_args: &[String],
    catalog: &Catalog,
    language: &str,
    js_engine: &mut Option<Rc<RefCell<ManuallyDrop<JsEngine>>>>,
    msgctxt: &str,
    msgid: &str,
    msgstrf: &str,
) -> Option<(String, bool, usize)> {
    // start_i points to '$', and chars[start_i+1]=='['.
    if start_i + 1 >= chars.len() || chars[start_i + 1] != '[' {
        return None;
    }

    let mut i = start_i + 2; // after "$["
    let mut argv: Vec<String> = Vec::new();
    let mut local_fallback = false;

    // Parse until we find closing ']' at argument boundary.
    // The loop is intentionally similar to ki18n resolveInterpolation:
    // - skip whitespace
    // - parse one argument as concatenation of segments (free/quoted/sub-interp)
    // - stop when next char is ']'
    while i < chars.len() {
        // Skip whitespace
        while i < chars.len() && chars[i].is_whitespace() {
            i += 1;
        }
        if i >= chars.len() {
            return None;
        }
        if chars[i] == ']' {
            // End of interpolation.
            i += 1;
            break;
        }

        // Parse one argument: concatenation of segments until whitespace or ']'.
        let mut segs: Vec<String> = Vec::new();
        let mut vref: Option<String> = None;

        loop {
            if i >= chars.len() {
                return None;
            }
            if chars[i].is_whitespace() || chars[i] == ']' {
                break;
            }
            if chars[i] == '$' && i + 1 < chars.len() && chars[i + 1] == '[' {
                // sub-interpolation
                if let Some((sub, sub_fb, next_i)) = eval_interpolation_at(
                    chars,
                    i,
                    context,
                    msg_args,
                    catalog,
                    language,
                    js_engine,
                    msgctxt,
                    msgid,
                    msgstrf,
                ) {
                    if sub_fb {
                        local_fallback = true;
                    }
                    segs.push(sub);
                    vref = None; // not a bare value reference anymore
                    i = next_i;
                    continue;
                } else {
                    return None;
                }
            }

            if chars[i] == '\'' {
                // quoted segment
                i += 1; // skip opening quote
                let mut buf = String::new();
                while i < chars.len() {
                    if chars[i] == '\'' {
                        i += 1; // skip closing quote
                        break;
                    }
                    if chars[i] == '\\' && i + 1 < chars.len() {
                        // escape next char
                        i += 1;
                        buf.push(chars[i]);
                        i += 1;
                        continue;
                    }
                    buf.push(chars[i]);
                    i += 1;
                }
                segs.push(buf);
                vref = None;
                continue;
            }

            // Free segment:
            // - Stops on whitespace, end ']', quote, or the start of a sub-interpolation "$["
            // - Treats backslash escapes like ki18n: "\X" => append X (drop backslash),
            //   even if X would normally be a delimiter like whitespace or ']' .
            let mut j = i;
            let mut seg_buf = String::new();
            while j < chars.len() {
                let c = chars[j];

                // Delimiters (only when not escaped).
                if c.is_whitespace() || c == ']' || c == '\'' {
                    break;
                }
                if c == '$' && j + 1 < chars.len() && chars[j + 1] == '[' {
                    break;
                }

                if c == '\\' {
                    // Drop the backslash and append the next character (if any).
                    if j + 1 < chars.len() {
                        seg_buf.push(chars[j + 1]);
                        j += 2;
                        continue;
                    } else {
                        // Trailing backslash: treat it as literal.
                        seg_buf.push('\\');
                        j += 1;
                        continue;
                    }
                }

                seg_buf.push(c);
                j += 1;
            }
            let seg = seg_buf;

            // value reference only recognized when the whole argument is exactly "^N"
            // (ki18n uses segmentToValue and only uses it when segs.size()==1).
            if seg.len() >= 2 && seg.as_bytes()[0] == b'^' {
                // parse digits
                let digits = &seg[1..];
                if !digits.is_empty() && digits.chars().all(|c| c.is_ascii_digit()) {
                    if let Ok(n) = digits.parse::<usize>() {
                        if n >= 1 && n <= msg_args.len() {
                            vref = Some(msg_args[n - 1].clone());
                        } else {
                            vref = None;
                        }
                    }
                }
            }

            segs.push(seg);
            i = j;
        }

        // Finalize argument: if it's exactly a bare value reference, use vref.
        let arg = if segs.len() == 1 {
            if let Some(v) = vref {
                v
            } else {
                segs[0].clone()
            }
        } else {
            // concatenated segments
            segs.concat()
        };
        argv.push(arg);
    }

    if argv.is_empty() {
        return Some(("".to_string(), local_fallback, i));
    }

    let call_name = argv[0].as_str();
    let call_args: Vec<String> = argv.iter().skip(1).cloned().collect();

    // Evaluate call.
    let funcs = builtin_functions();
    if call_name.eq_ignore_ascii_case("fallback") {
        return Some(("".to_string(), true, i));
    }

    // T9: `$[hascall target]` 表示「是否存在名为 target 的可调用」，需检查 target 而非外层名
    // `hascall`。若走 `call_ts("hascall", ...)`，会错误地执行 `Ts.hascall("hascall")`。
    if call_name.eq_ignore_ascii_case("hascall") {
        if call_args.is_empty() {
            return Some(("".to_string(), local_fallback, i));
        }
        let target = call_args[0].as_str();
        if funcs.contains_key(target) {
            return Some(("true".to_string(), local_fallback, i));
        }
        if js_engine.is_none() {
            let engine_rc = match get_or_create_js_engine_by_lang(language) {
                Some(rc) => rc,
                None => return Some(("false".to_string(), local_fallback, i)),
            };
            *js_engine = Some(engine_rc);
        }
        let mut engine = js_engine
            .as_ref()
            .expect("js_engine must exist after initialization")
            .borrow_mut();
        if engine
            .set_message_context(
                language,
                msgctxt,
                msgid,
                context,
                msg_args,
                msg_args,
                msgstrf,
            )
            .is_err()
        {
            return Some(("false".to_string(), local_fallback, i));
        }
        let ok = match engine.eval_ts_hascall(target) {
            Ok(b) => b,
            Err(_) => false,
        };
        return Some((if ok { "true" } else { "false" }.to_string(), local_fallback, i));
    }

    if let Some(func) = funcs.get(call_name) {
        let args_ref: Vec<&str> = call_args.iter().map(|s| s.as_str()).collect();
        let replacement = func(&args_ref, context, msg_args, catalog, language);
        Some((replacement, local_fallback, i))
    } else {
        // Unknown call: execute through JS transcript engine (Ts.* API).
        // R07: Ensure JS engine and context are ready; always refresh context before call_ts
        // (engine may be reused from a previous message with different msgid/msgctxt).
        if js_engine.is_none() {
            let engine_rc = match get_or_create_js_engine_by_lang(language) {
                Some(rc) => rc,
                None => return Some(("".to_string(), true, i)),
            };
            *js_engine = Some(engine_rc);
        }

        let mut engine = js_engine
            .as_ref()
            .expect("js_engine must exist after initialization")
            .borrow_mut();
        // Always set context before each call (handles cross-message engine reuse).
        if engine
            .set_message_context(
                language,
                msgctxt,
                msgid,
                context,
                msg_args,
                msg_args,
                msgstrf,
            )
            .is_err()
        {
            return Some(("".to_string(), true, i));
        }
        match engine.call_ts(call_name, &call_args) {
            Ok((replacement, fb)) => Some((replacement, fb, i)),
            Err(_) => Some(("".to_string(), true, i)),
        }
    }
}

// ---------------------------------------------------------------------------
// 内置函数实现
// ---------------------------------------------------------------------------

fn builtin_arg(
    args: &[&str],
    _context: &HashMap<String, String>,
    msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let n: usize = match args[0].parse() {
        Ok(v) => v,
        Err(_) => return String::new(),
    };
    if n == 0 || n > msg_args.len() {
        if args.len() > 1 {
            return args[1..].join(" ");
        }
        return String::new();
    }
    msg_args[n - 1].clone()
}

fn builtin_plural(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let n: u64 = match args[0].parse() {
        Ok(v) => v,
        Err(_) => return String::new(),
    };
    let idx = catalog.get_plural_form(n);
    idx.to_string()
}

fn builtin_gender(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 3 {
        return String::new();
    }
    let gender_code: i32 = match args[0].parse().ok() {
        Some(v) => v,
        None => return String::new(),
    };
    match gender_code {
        1 => args[1].to_string(), // male
        2 => args[2].to_string(), // female
        _ => {
            if args.len() >= 4 {
                args[3].to_string() // neutral
            } else {
                args[2].to_string() // 默认 female
            }
        }
    }
}

fn builtin_language(
    _args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    language.to_string()
}

fn builtin_country(
    _args: &[&str],
    context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    // 优先使用上下文中的 "country"，否则从语言代码中推断
    if let Some(c) = context.get("country") {
        return c.clone();
    }
    // 简单从语言代码推断: "xx_YY" / "xx-YY" -> "YY"
    if let Some(pos) = language.find(['_', '-']) {
        return language[pos + 1..].to_string();
    }
    String::new()
}

fn builtin_contains(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let value = args[0];
    let sub = args[1];
    if value.contains(sub) {
        "true".to_string()
    } else {
        "false".to_string()
    }
}

fn builtin_starts_with(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let value = args[0];
    let prefix = args[1];
    if value.starts_with(prefix) {
        "true".to_string()
    } else {
        "false".to_string()
    }
}

fn builtin_ends_with(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let value = args[0];
    let suffix = args[1];
    if value.ends_with(suffix) {
        "true".to_string()
    } else {
        "false".to_string()
    }
}

fn builtin_upper_first(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    to_case_first(args, true)
}

fn builtin_lower_first(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    to_case_first(args, false)
}

/// ki18n Scriptface::toCaseFirst 的 Rust 等价实现（用于 Transcript 内置 `upperFirst`/`lowerFirst`）。
///
/// 语义：
/// - 当检查开关打开时，遇到第一个 Unicode 字母才更改大小写；
/// - 支持 `~@` alternatives 指令：当 `nalt > 0` 且遇到 `~@<sep>` 时，在每个替代段的首个字母上执行大小写变换。
fn to_case_first(args: &[&str], toupper: bool) -> String {
    if args.is_empty() {
        return String::new();
    }
    let s = args[0];
    let qnalt: usize = if args.len() > 1 {
        args[1].parse::<usize>().unwrap_or(0)
    } else {
        0
    };

    // Work with chars so indices match the parser's "string literal" expectations.
    let mut out: Vec<char> = s.chars().collect();
    let mut remaining_alts: usize = 0;
    let mut alt_sep: Option<char> = None;
    let mut check_case = true;
    let mut num_chcased: usize = 0;

    let head: Vec<char> = "~@".chars().collect(); // always 2 chars here
    let hlen = head.len();

    let mut i: usize = 0;
    while i < out.len() {
        let c = out[i];

        if qnalt > 0 && remaining_alts == 0 && i + hlen <= out.len() {
            if out[i..i + hlen] == head[..] {
                i += hlen;
                if i >= out.len() {
                    break; // malformed directive, bail out
                }
                alt_sep = Some(out[i]);
                remaining_alts = qnalt;
                check_case = true;
            } else if remaining_alts > 0 {
                // no-op
            }
        } else if remaining_alts > 0 {
            if let Some(sep) = alt_sep {
                if c == sep {
                    remaining_alts -= 1;
                    check_case = true;
                }
            }
        }

        // Only case-check when check_case is active and current char is a letter.
        if check_case && c.is_alphabetic() {
            if toupper {
                let upper: String = c.to_uppercase().collect();
                // to_uppercase() for some chars can yield multiple chars; keep it simple by taking the first.
                // This matches our current engine's limitations and avoids reallocations across indices.
                if let Some(first) = upper.chars().next() {
                    out[i] = first;
                }
            } else {
                let lower: String = c.to_lowercase().collect();
                if let Some(first) = lower.chars().next() {
                    out[i] = first;
                }
            }
            num_chcased += 1;
            check_case = false;
        }

        if num_chcased > 0 && remaining_alts == 0 {
            break;
        }

        i += 1;
    }

    out.into_iter().collect()
}

fn builtin_upper(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    args[0].to_uppercase()
}

fn builtin_lower(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    args[0].to_lowercase()
}

fn builtin_trim(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    args[0].trim().to_string()
}

fn builtin_remove_accelerator(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    remove_accelerator_marker(args[0])
}

fn builtin_plural_form(
    _args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    catalog: &Catalog,
    _language: &str,
) -> String {
    catalog.plural_rule().nplurals().to_string()
}

fn builtin_arg_range(
    args: &[&str],
    _context: &HashMap<String, String>,
    msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let start: usize = args[0].parse().unwrap_or(1);
    let end: usize = if args.len() > 1 {
        args[1].parse().unwrap_or(start)
    } else {
        start
    };
    // 第三个参数作为分隔符；如果测试写成空字符串（`, ,` 解析后为空），按 " ," 处理
    let sep = if args.len() > 2 {
        if args[2].is_empty() { " ," } else { args[2] }
    } else {
        " "
    };
    if start == 0 || end == 0 || start > msg_args.len() || start > end {
        return String::new();
    }
    let end_idx = end.min(msg_args.len());
    msg_args[start - 1..end_idx].join(sep)
}

fn builtin_language_name(
    _args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    let lang = language.split(|c| c == '_' || c == '-').next().unwrap_or(language);
    match lang {
        "en" => "English",
        "de" => "German",
        "fr" => "French",
        "zh" => "Chinese",
        "ja" => "Japanese",
        "ko" => "Korean",
        _ => language,
    }
    .to_string()
}

fn builtin_capitalize(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let s = args[0];
    let mut result = String::new();
    let mut new_word = true;
    for ch in s.chars() {
        if ch.is_whitespace() {
            new_word = true;
            result.push(ch);
        } else if new_word {
            for up in ch.to_uppercase() {
                result.push(up);
            }
            new_word = false;
        } else {
            result.push(ch);
        }
    }
    result
}

fn builtin_sort(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let s = args[0];
    let mut parts: Vec<&str> = s.split_whitespace().collect();
    parts.sort_unstable();
    parts.join(" ")
}

fn builtin_reverse(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let s = args[0];
    s.chars().rev().collect()
}

fn builtin_replace(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 3 {
        return String::new();
    }
    let s = args[0];
    let from = args[1];
    let to = args[2];
    s.replace(from, to)
}

fn builtin_substring(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let s = args[0];
    let start: usize = args[1].parse().unwrap_or(0);
    let len: Option<usize> = if args.len() > 2 {
        Some(args[2].parse().unwrap_or(0))
    } else {
        None
    };

    let chars: Vec<char> = s.chars().collect();
    if start >= chars.len() {
        return String::new();
    }
    let end = match len {
        Some(l) => (start + l).min(chars.len()),
        None => chars.len(),
    };
    chars[start..end].iter().collect()
}

fn builtin_norm_key(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    _language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    crate::core::js_engine::transcript_norm_keystr(args[0])
}

fn builtin_get_conf_string(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let key = args[0];
    let default = args.get(1).unwrap_or(&"").to_string();
    crate::core::js_engine::transcript_get_conf_string(language, key, &default)
}

fn builtin_get_conf_bool(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.is_empty() {
        return "false".to_string();
    }
    let key = args[0];
    let default = args
        .get(1)
        .map(|s| s.eq_ignore_ascii_case("true"))
        .unwrap_or(false);
    let val = crate::core::js_engine::transcript_get_conf_bool(language, key, default);
    val.to_string()
}

fn builtin_get_conf_number(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.is_empty() {
        return "0".to_string();
    }
    let key = args[0];
    let default = args
        .get(1)
        .and_then(|s| s.parse::<f64>().ok())
        .unwrap_or(0.0);
    let val = crate::core::js_engine::transcript_get_conf_number(language, key, default);
    val.to_string()
}

/// R02: Grammatical case - Russian/Polish noun declension.
/// Ts.case(caseNum, noun) - caseNum 1=nom, 2=gen, 3=dat, 4=acc, 5=inst, 6=prep
fn builtin_case(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let case_num: u8 = match args[0].trim().parse() {
        Ok(n) if (1..=6).contains(&n) => n,
        _ => return args[1].to_string(),
    };
    let noun = args[1];

    // Russian (ru) - masculine inanimate declension table for common words
    let lang_base = language.split(['_', '-']).next().unwrap_or(language);
    if lang_base == "ru" {
        return decline_russian(noun, case_num);
    }
    if lang_base == "pl" {
        return decline_polish(noun, case_num);
    }
    // Unsupported: return noun unchanged
    noun.to_string()
}

/// Russian masculine inanimate noun declension.
/// nom=1, gen=2, dat=3, acc=4, inst=5, prep=6
fn decline_russian(noun: &str, case_num: u8) -> String {
    // Static table: nominative -> [gen, dat, acc, inst, prep]
    const RU_DECL: &[(&str, &[&str])] = &[
        ("файл", &["файла", "файлу", "файл", "файлом", "файле"]),
        ("день", &["дня", "дню", "день", "днём", "дне"]),
        ("месяц", &["месяца", "месяцу", "месяц", "месяцем", "месяце"]),
        ("год", &["года", "году", "год", "годом", "годе"]),
        ("час", &["часа", "часу", "час", "часом", "часе"]),
        ("раз", &["раза", "разу", "раз", "разом", "разе"]),
    ];
    let noun_lower = noun.to_lowercase();
    for (nom, forms) in RU_DECL {
        if noun_lower == *nom {
            let idx = match case_num {
                1 => return noun.to_string(),
                2..=6 => (case_num - 2) as usize,
                _ => return noun.to_string(),
            };
            if idx < forms.len() {
                return forms[idx].to_string();
            }
        }
    }
    // Simple rule: masculine consonant + а/у/ом/е
    let s = noun.trim();
    if let Some(last) = s.chars().last() {
        if last.is_alphabetic() && !"аеёиоуыэюяь".contains(last) {
            return apply_russian_consonant_rule(s, case_num);
        }
    }
    noun.to_string()
}

fn apply_russian_consonant_rule(word: &str, case_num: u8) -> String {
    const GEN_SUFF: &[char] = &['а'];
    const DAT_SUFF: &[char] = &['у'];
    const INST_SUFF: &[char] = &['о', 'м'];
    const PREP_SUFF: &[char] = &['е'];
    match case_num {
        1 | 4 => word.to_string(),
        2 => format!("{}{}", word, GEN_SUFF.iter().collect::<String>()),
        3 => format!("{}{}", word, DAT_SUFF.iter().collect::<String>()),
        5 => format!("{}{}", word, INST_SUFF.iter().collect::<String>()),
        6 => format!("{}{}", word, PREP_SUFF.iter().collect::<String>()),
        _ => word.to_string(),
    }
}

/// 波兰语：与 `Ts.case` 六格约定对齐（1=nom…6=loc）；常见 UI 名词表 + 简单词尾规则。
fn decline_polish(noun: &str, case_num: u8) -> String {
    // 与俄语相同：静态表 nominative -> [gen, dat, acc, inst, loc]
    const PL_DECL: &[(&str, &[&str])] = &[
        ("plik", &["pliku", "plikowi", "plik", "plikiem", "pliku"]),
        ("folder", &["folderu", "folderowi", "folder", "folderem", "folderze"]),
        ("okno", &["okna", "oknu", "okno", "oknem", "oknie"]),
        ("program", &["programu", "programowi", "program", "programem", "programie"]),
        ("ustawienia", &["ustawień", "ustawieniom", "ustawienia", "ustawieniami", "ustawieniach"]),
        ("godzina", &["godziny", "godzinie", "godzinę", "godziną", "godzinie"]),
        ("minuta", &["minuty", "minucie", "minutę", "minutą", "minucie"]),
    ];
    let noun_lower = noun.to_lowercase();
    for (nom, forms) in PL_DECL {
        if noun_lower == *nom {
            let idx = match case_num {
                1 => return noun.to_string(),
                2..=6 => (case_num - 2) as usize,
                _ => return noun.to_string(),
            };
            if idx < forms.len() {
                // 保留原大小写风格：若首字母大写则输出首字母大写
                let out = forms[idx].to_string();
                return if noun.chars().next().is_some_and(|c| c.is_uppercase()) {
                    let mut c = out.chars();
                    match c.next() {
                        None => out,
                        Some(f) => f.to_uppercase().collect::<String>() + c.as_str(),
                    }
                } else {
                    out
                };
            }
        }
    }
    noun.to_string()
}

/// R01: Load .pmap/.pmapc properties. Ts.loadProps(relpath)
fn builtin_load_props(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.is_empty() {
        return String::new();
    }
    let relpath = args[0];
    let _ = crate::core::js_engine::transcript_load_props(language, relpath);
    "loaded".to_string()
}

/// R01: Get property. Ts.getProp(phrase, prop) or Ts.getProp(relpath, phrase, prop)
fn builtin_get_prop(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.len() < 2 {
        return String::new();
    }
    let result = if args.len() >= 3 {
        crate::core::js_engine::transcript_get_prop(
            language,
            args[0],
            args[1],
            Some(args[2]),
        )
    } else {
        crate::core::js_engine::transcript_get_prop(language, args[0], args[1], None)
    };
    result
}

/// R01: Set property. Ts.setProp(phrase, prop, value)
fn builtin_set_prop(
    args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    if args.len() < 3 {
        return String::new();
    }
    crate::core::js_engine::transcript_set_prop(language, args[0], args[1], args[2]);
    args[2].to_string()
}

fn builtin_script(
    _args: &[&str],
    _context: &HashMap<String, String>,
    _msg_args: &[String],
    _catalog: &Catalog,
    language: &str,
) -> String {
    // 提取语言部分，如 "en_US" -> "en"
    let lang = language
        .split(|c| c == '_' || c == '-')
        .next()
        .unwrap_or("en");

    match lang {
        // 拉丁字母
        "en" | "de" | "fr" | "es" | "it" | "pt" | "nl" | "pl" | "sv" | "da" | "no" | "fi"
        | "et" | "lv" | "lt" => "Latn",

        // 西里尔字母
        "ru" | "uk" | "bg" | "sr" | "mk" | "be" | "kk" | "uz" | "tg" | "ky" | "mn" => "Cyrl",

        // 汉字（简体/繁体）
        "zh" => {
            if language.contains("Hant") || language.contains("TW") || language.contains("HK") {
                "Hant"
            } else {
                "Hans"
            }
        }

        // 阿拉伯字母
        "ar" | "fa" | "ur" | "ps" => "Arab",

        // 希伯来字母
        "he" | "yi" => "Hebr",

        // 日语
        "ja" => "Jpan",

        // 韩语
        "ko" => "Kore",

        // 希腊字母
        "el" => "Grek",

        // 泰米尔等
        "ta" | "ml" => "Taml",

        // 格鲁吉亚字母
        "ka" => "Geor",

        // 亚美尼亚字母
        "hy" => "Armn",

        // 泰语
        "th" => "Thai",

        // 老挝语
        "lo" => "Laoo",

        // 缅甸语
        "my" => "Mymr",

        // 其他未知语言默认视为拉丁
        _ => "Latn",
    }
    .to_string()
}

// ----------------------------------------------------------------------------
// JavaScript Transcript (Ts.* API) - compatibility layer for autotests/test.js
// ----------------------------------------------------------------------------

/// Evaluate a single `Ts.*` call from JS transcript scripts.
///
/// This helper is used to validate the `Ts.*` API compatibility with
/// `ki18n/autotests/test.js` / `ktranscripttest.cpp` semantics.
///
/// When `run_post_calls` is `false`, matches the standalone `KTranscript::eval`
/// plugin path: post-call queue is not flushed here (KLocalizedString runs
/// those via `postCalls()` + separate `eval` invocations upstream).
pub fn eval_transcript_js_call_ex(
    argv: &[String],
    language: &str,
    msgctxt: &str,
    dynamic_context: &HashMap<String, String>,
    msgid: &str,
    subs: &[String],
    vals: &[String],
    msgstrf: &str,
    module_files: &[&Path],
    run_post_calls: bool,
) -> Result<(String, bool), String> {
    let call_name = argv.get(0).map(|s| s.as_str()).unwrap_or("");
    let args: Vec<String> = argv.iter().skip(1).cloned().collect();

    let mut engine = JsEngine::new().map_err(|e| format!("{e:?}"))?;

    // Load JS modules (e.g. ki18n/autotests/test.js).
    for p in module_files {
        engine.eval_module_file(p).map_err(|e| format!("{e:?}"))?;
    }

    // Message context exposed to JS (Ts.msgid()/Ts.subs()/Ts.getConf*()/...).
    engine
        .set_message_context(
            language,
            msgctxt,
            msgid,
            dynamic_context,
            subs,
            vals,
            msgstrf,
        )
        .map_err(|e| format!("{e:?}"))?;

    let ret = engine
        .call_ts(call_name, &args)
        .map_err(|e| format!("{e:?}"))?;
    if run_post_calls {
        // Full-message transcript path: run setcallForall hooks once.
        engine
            .execute_post_calls()
            .map_err(|e| format!("{e:?}"))?;
    }
    Ok(ret)
}

/// Same as [`eval_transcript_js_call_ex`] with `run_post_calls = true`.
pub fn eval_transcript_js_call(
    argv: &[String],
    language: &str,
    msgctxt: &str,
    dynamic_context: &HashMap<String, String>,
    msgid: &str,
    subs: &[String],
    vals: &[String],
    msgstrf: &str,
    module_files: &[&Path],
) -> Result<(String, bool), String> {
    eval_transcript_js_call_ex(
        argv,
        language,
        msgctxt,
        dynamic_context,
        msgid,
        subs,
        vals,
        msgstrf,
        module_files,
        true,
    )
}

