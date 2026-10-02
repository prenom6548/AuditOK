// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::{HashMap, HashSet};
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::{Mutex, OnceLock};
use std::sync::atomic::{AtomicUsize, Ordering};

use boa_engine::{Context, JsResult, JsString, JsValue};

use crate::core::transcript_builtins;

// ---------------------------------------------------------------------------
// Global stores (native callbacks are closures and cannot easily capture &mut state)
// ---------------------------------------------------------------------------

static PROPS_STORE: OnceLock<
    Mutex<HashMap<String, HashMap<String, HashMap<String, String>>>>,
> = OnceLock::new();
static CONF_STORE: OnceLock<Mutex<HashMap<String, HashMap<String, String>>>> = OnceLock::new();

/// 与上游 `Scriptface::loadedPmapPaths` 一致：同一 pmap 路径只解析合并一次（按引擎实例区分）。
static LOADED_PROPS_PATHS: OnceLock<Mutex<HashSet<String>>> = OnceLock::new();

static ENGINE_ID_COUNTER: AtomicUsize = AtomicUsize::new(1);

fn props_store() -> &'static Mutex<HashMap<String, HashMap<String, HashMap<String, String>>>> {
    PROPS_STORE.get_or_init(|| Mutex::new(HashMap::new()))
}

fn conf_store() -> &'static Mutex<HashMap<String, HashMap<String, String>>> {
    CONF_STORE.get_or_init(|| Mutex::new(HashMap::new()))
}

fn props_loaded_paths() -> &'static Mutex<HashSet<String>> {
    LOADED_PROPS_PATHS.get_or_init(|| Mutex::new(HashSet::new()))
}

fn props_load_cache_key(engine_id: &str, path: &Path) -> String {
    let p = path
        .canonicalize()
        .unwrap_or_else(|_| path.to_path_buf())
        .display()
        .to_string();
    format!("{engine_id}\x1c{p}")
}

fn autotests_dir() -> PathBuf {
    let p = crate::paths::ki18n_autotests_dir();
    p.canonicalize().unwrap_or(p)
}

fn ensure_conf_loaded() {
    let mut conf = conf_store().lock().unwrap();
    if !conf.is_empty() {
        return;
    }

    // Upstream ki18n/ktranscript loads:
    // - config location "ktranscript.ini"
    // - fallback: ~/.transcriptrc
    //
    // For deterministic behavior in our test suite, we prefer the repo-local
    // `ktranscript-test.ini` if it exists. In production installs this file
    // should typically not be present, and we fall back to upstream paths.
    let mut candidates = vec![autotests_dir().join("ktranscript-test.ini")];

    let home = std::env::var_os("HOME")
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();
    if !home.is_empty() {
        // ~/.transcriptrc
        candidates.push(PathBuf::from(&home).join(".transcriptrc"));
        // $XDG_CONFIG_HOME/ktranscript.ini or ~/.config/ktranscript.ini
        if let Some(xdg) = std::env::var_os("XDG_CONFIG_HOME") {
            candidates.push(PathBuf::from(xdg).join("ktranscript.ini"));
        } else {
            candidates.push(PathBuf::from(&home).join(".config").join("ktranscript.ini"));
        }
    }

    let content = candidates
        .iter()
        .find_map(|p| fs::read_to_string(p).ok())
        .unwrap_or_default();

    if content.is_empty() {
        return;
    }

    let mut current_group = String::new();
    for line in content.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with(';') {
            continue;
        }
        if line.starts_with('[') && line.ends_with(']') {
            current_group = line.trim_matches(&['[', ']'][..]).to_string();
            conf.entry(current_group.clone()).or_default();
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            if !current_group.is_empty() {
                conf.get_mut(&current_group)
                    .unwrap()
                    .insert(k.trim().to_string(), v.trim().to_string());
            }
        }
    }
}

fn js_escape_string(s: &str) -> String {
    // Minimal JS string escaping suitable for test inputs.
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\r', "\\r")
}

fn to_upper_first(s: &str) -> String {
    let mut out = String::new();
    let mut replaced = false;
    for ch in s.chars() {
        if !replaced && ch.is_alphabetic() {
            for up in ch.to_uppercase() {
                out.push(up);
            }
            replaced = true;
        } else {
            out.push(ch);
        }
    }
    out
}

fn to_lower_first(s: &str) -> String {
    let mut out = String::new();
    let mut replaced = false;
    for ch in s.chars() {
        if !replaced && ch.is_alphabetic() {
            for low in ch.to_lowercase() {
                out.push(low);
            }
            replaced = true;
        } else {
            out.push(ch);
        }
    }
    out
}

/// 对齐上游 `Scriptface::toUpperFirst` / `toLowerFirst` 中的 `toCaseFirst`（`~@` 多候选分隔）。
fn to_case_first(s: &str, nalt: i32, upper: bool) -> String {
    if nalt <= 0 {
        return if upper {
            to_upper_first(s)
        } else {
            to_lower_first(s)
        };
    }
    let mut qstrcc: Vec<char> = s.chars().collect();
    let len = qstrcc.len();
    let mut alt_sep: Option<char> = None;
    let mut remaining_alts = 0i32;
    let mut check_case = true;
    let mut num_cased = 0i32;
    let mut i = 0usize;
    while i < len {
        let c = qstrcc[i];
        if nalt > 0
            && remaining_alts == 0
            && i + 2 <= len
            && qstrcc[i] == '~'
            && qstrcc[i + 1] == '@'
        {
            i += 2;
            if i >= len {
                break;
            }
            alt_sep = Some(qstrcc[i]);
            remaining_alts = nalt;
            check_case = true;
        } else if remaining_alts > 0 && alt_sep == Some(c) {
            remaining_alts -= 1;
            check_case = true;
        } else if check_case && c.is_alphabetic() {
            let rep = if upper {
                c.to_uppercase().next().unwrap_or(c)
            } else {
                c.to_lowercase().next().unwrap_or(c)
            };
            qstrcc[i] = rep;
            num_cased += 1;
            check_case = false;
        }
        if num_cased > 0 && remaining_alts == 0 {
            break;
        }
        i += 1;
    }
    qstrcc.into_iter().collect()
}

/// 与上游 `countLines(s, p)` 一致：1-based 行号，统计位置 `p` 之前的换行数。
fn pmap_count_lines(chars: &[char], p: usize) -> usize {
    let mut n = 1usize;
    for i in 0..p.min(chars.len()) {
        if chars[i] == '\n' {
            n += 1;
        }
    }
    n
}

/// 对齐 `normKeystr(raw, mayHaveAcc)`（无正则，性能路径）。
fn norm_keystr_pmap(raw: &str, may_have_acc: bool) -> String {
    let mut nkey = String::new();
    for ch in raw.chars() {
        if !ch.is_whitespace() {
            nkey.push(ch);
        }
    }
    let key = if may_have_acc {
        crate::core::accelerator::remove_accelerator_marker(&nkey)
    } else {
        nkey
    };
    key.to_lowercase()
}

/// 对齐 `trimSmart`。
fn trim_smart_pmap(raw: &str) -> String {
    let chars: Vec<char> = raw.chars().collect();
    let len = chars.len();
    let mut is = 0usize;
    while is < len && chars[is].is_whitespace() && chars[is] != '\n' {
        is += 1;
    }
    let start = if is >= len || chars[is] != '\n' {
        0usize
    } else {
        is + 1
    };

    let mut ie = len as isize - 1;
    while ie >= 0 {
        let ui = ie as usize;
        if !chars[ui].is_whitespace() || chars[ui] == '\n' {
            break;
        }
        ie -= 1;
    }
    let end = if ie < 0 {
        len
    } else {
        let ui = ie as usize;
        if chars[ui] != '\n' {
            len
        } else {
            ui
        }
    };

    chars[start..end].iter().collect()
}

/// 对齐 `Scriptface::loadProps_text` 流式解析（多 entry key、注释 `#`、错误文案与行号）。
fn parse_pmap_text(content: &str, fpath: &str) -> Result<HashMap<String, HashMap<String, String>>, String> {
    #[derive(Clone, Copy, PartialEq, Eq)]
    enum State {
        NextEntry,
        NextKey,
        NextValue,
    }

    let s: Vec<char> = content.chars().collect();
    let slen = s.len();
    let mut global: HashMap<String, HashMap<String, String>> = HashMap::new();
    let mut ekeys: Vec<String> = Vec::new();
    let mut props: HashMap<String, String> = HashMap::new();
    let mut state = State::NextEntry;
    let mut pkey = String::new();
    let mut key_sep = '\0';
    let mut prop_sep = '\0';
    let mut i = 0usize;

    let mut eof = false;
    while !eof {
        if i >= slen {
            break;
        }
        let i_checkpoint = i;

        match state {
            State::NextEntry => {
                while i < slen && s[i].is_whitespace() {
                    i += 1;
                    if i >= slen {
                        eof = true;
                        break;
                    }
                }
                if eof || i >= slen {
                    break;
                }
                if i + 1 >= slen {
                    return Err(format!("Ts.loadProps_text: unexpected end of file in {fpath}"));
                }
                if s[i] != '#' {
                    key_sep = s[i];
                    prop_sep = s[i + 1];
                    if key_sep.is_alphabetic() || prop_sep.is_alphabetic() {
                        let line = pmap_count_lines(&s, i);
                        return Err(format!(
                            "Ts.loadProps_text: separator characters must not be letters at {fpath}:{line}"
                        ));
                    }
                    ekeys.clear();
                    props.clear();
                    pkey.clear();
                    i += 2;
                    state = State::NextKey;
                } else {
                    while i < slen && s[i] != '\n' {
                        i += 1;
                        if i >= slen {
                            eof = true;
                            break;
                        }
                    }
                }
            }
            State::NextKey => {
                let ip = i;
                while i < slen && s[i] != key_sep && s[i] != prop_sep {
                    i += 1;
                    if i >= slen {
                        eof = true;
                        break;
                    }
                }
                if eof || i >= slen {
                    break;
                }
                if s[i] == key_sep {
                    let mid: String = s[ip..i].iter().collect();
                    pkey = norm_keystr_pmap(&mid, false);
                    i += 1;
                    state = State::NextValue;
                } else {
                    let mid: String = s[ip..i].iter().collect();
                    let ekey = norm_keystr_pmap(&mid, false);
                    if !ekey.is_empty() {
                        ekeys.push(ekey);
                        i += 1;
                    } else {
                        if ekeys.len() < 1 {
                            let line = pmap_count_lines(&s, i);
                            return Err(format!(
                                "Ts.loadProps_text: no entry key for entry ending at {fpath}:{line}"
                            ));
                        }
                        for ek in &ekeys {
                            global.insert(ek.clone(), props.clone());
                        }
                        i += 1;
                        state = State::NextEntry;
                    }
                }
            }
            State::NextValue => {
                let ip = i;
                while i < slen && s[i] != prop_sep {
                    if s[i] == key_sep {
                        let line = pmap_count_lines(&s, i);
                        return Err(format!(
                            "Ts.loadProps_text: property separator inside property value at {fpath}:{line}"
                        ));
                    }
                    i += 1;
                    if i >= slen {
                        eof = true;
                        break;
                    }
                }
                if eof || i >= slen {
                    break;
                }
                let mid: String = s[ip..i].iter().collect();
                let pval = trim_smart_pmap(&mid);
                props.insert(pkey.clone(), pval);
                i += 1;
                state = State::NextKey;
            }
        }

        if eof {
            break;
        }
        if i >= slen {
            break;
        }
        if i == i_checkpoint {
            let line = pmap_count_lines(&s, i);
            return Err(format!("Ts.loadProps: internal error 20 at {fpath}:{line}"));
        }
    }

    if state != State::NextEntry {
        return Err(format!("Ts.loadProps: unexpected end of file in {fpath}"));
    }

    Ok(global)
}

fn parse_pmapc(content: &[u8]) -> Result<HashMap<String, HashMap<String, String>>, String> {
    fn read_u32_be(b: &[u8], pos: &mut usize) -> Result<u32, String> {
        if *pos + 4 > b.len() {
            return Err("u32 out of bounds".to_string());
        }
        let mut arr = [0u8; 4];
        arr.copy_from_slice(&b[*pos..*pos + 4]);
        *pos += 4;
        Ok(u32::from_be_bytes(arr))
    }
    fn read_u64_be(b: &[u8], pos: &mut usize) -> Result<u64, String> {
        if *pos + 8 > b.len() {
            return Err("u64 out of bounds".to_string());
        }
        let mut arr = [0u8; 8];
        arr.copy_from_slice(&b[*pos..*pos + 8]);
        *pos += 8;
        Ok(u64::from_be_bytes(arr))
    }
    fn read_string_be(b: &[u8], pos: &mut usize) -> Result<String, String> {
        let nbytes = read_u32_be(b, pos)? as usize;
        if *pos + nbytes > b.len() {
            return Err("string out of bounds".to_string());
        }
        let s = std::str::from_utf8(&b[*pos..*pos + nbytes])
            .map_err(|_| "invalid utf8 in string".to_string())?;
        *pos += nbytes;
        Ok(s.to_string())
    }

    fn norm_keystr_compile_like(s: &str) -> String {
        let mut out = String::new();
        for ch in s.chars() {
            if ch.is_whitespace() || ch == '&' {
                continue;
            }
            out.push(ch);
        }
        out.to_lowercase()
    }

    if content.len() < 8 {
        return Err("pmapc too small".to_string());
    }
    let magic = &content[..8];
    if magic == b"TSPMAP00" {
        let mut pos = 8usize;
        let nentries = read_u32_be(content, &mut pos)? as usize;
        let mut out: HashMap<String, HashMap<String, String>> = HashMap::new();
        for _ in 0..nentries {
            let nekeys = read_u32_be(content, &mut pos)? as usize;
            let mut ekeys = Vec::with_capacity(nekeys);
            for _ in 0..nekeys {
                let ekey = read_string_be(content, &mut pos)?;
                ekeys.push(norm_keystr_compile_like(&ekey));
            }

            let nprops = read_u32_be(content, &mut pos)? as usize;
            let mut props: HashMap<String, String> = HashMap::new();
            for _ in 0..nprops {
                let pkey = read_string_be(content, &mut pos)?;
                let pval = read_string_be(content, &mut pos)?;
                props.insert(norm_keystr_compile_like(&pkey), pval);
            }

            for ekey in ekeys {
                out.insert(ekey, props.clone());
            }
        }
        Ok(out)
    } else if magic == b"TSPMAP01" {
        let mut pos = 8usize;
        let numekeys = read_u32_be(content, &mut pos)? as usize;
        let lenekeys = read_u64_be(content, &mut pos)? as usize;
        if pos + lenekeys > content.len() {
            return Err("pmapc01 binekeys out of bounds".to_string());
        }
        let binekeys_start = pos;
        let binekeys_end = pos + lenekeys;
        let mut local_pos = binekeys_start;

        // ekey -> offset into file of property blob.
        let mut offsets: HashMap<String, u64> = HashMap::new();
        for _ in 0..numekeys {
            // read_string_be but constrained within binekeys blob
            let nbytes = {
                if local_pos + 4 > binekeys_end {
                    return Err("pmapc01 string length out of bounds".to_string());
                }
                let mut arr = [0u8; 4];
                arr.copy_from_slice(&content[local_pos..local_pos + 4]);
                local_pos += 4;
                u32::from_be_bytes(arr) as usize
            };
            if local_pos + nbytes > binekeys_end {
                return Err("pmapc01 string bytes out of bounds".to_string());
            }
            let s = std::str::from_utf8(&content[local_pos..local_pos + nbytes])
                .map_err(|_| "invalid utf8 in ekey".to_string())?
                .to_string();
            local_pos += nbytes;

            // offset is u64 big-endian.
            if local_pos + 8 > binekeys_end {
                return Err("pmapc01 offset out of bounds".to_string());
            }
            let mut arr = [0u8; 8];
            arr.copy_from_slice(&content[local_pos..local_pos + 8]);
            local_pos += 8;
            let offset = u64::from_be_bytes(arr);

            offsets.insert(norm_keystr_compile_like(&s), offset);
        }

        // Parse each property blob at its absolute offset.
        let mut out: HashMap<String, HashMap<String, String>> = HashMap::new();
        for (phrase, offset) in offsets {
            let off = offset as usize;
            if off + 8 > content.len() {
                return Err("pmapc01 blob out of bounds".to_string());
            }
            let mut blob_pos = off;
            let numpkeys = read_u32_be(content, &mut blob_pos)? as usize;
            let lenpkeys = read_u32_be(content, &mut blob_pos)? as usize;
            if blob_pos + lenpkeys > content.len() {
                return Err("pmapc01 blob content out of bounds".to_string());
            }
            let blob_end = blob_pos + lenpkeys;
            let mut local_blob_pos = blob_pos;

            let mut props: HashMap<String, String> = HashMap::new();
            for _ in 0..numpkeys {
                if local_blob_pos >= blob_end {
                    return Err("pmapc01 unexpected end in properties".to_string());
                }
                // Read pkey string (u32 length + bytes)
                let pkey_len = {
                    if local_blob_pos + 4 > blob_end {
                        return Err("pmapc01 pkey len out of bounds".to_string());
                    }
                    let mut arr = [0u8; 4];
                    arr.copy_from_slice(&content[local_blob_pos..local_blob_pos + 4]);
                    local_blob_pos += 4;
                    u32::from_be_bytes(arr) as usize
                };
                if local_blob_pos + pkey_len > blob_end {
                    return Err("pmapc01 pkey bytes out of bounds".to_string());
                }
                let pkey = std::str::from_utf8(&content[local_blob_pos..local_blob_pos + pkey_len])
                    .map_err(|_| "invalid utf8 in pkey".to_string())?
                    .to_string();
                local_blob_pos += pkey_len;

                // Read pval string
                let pval_len = {
                    if local_blob_pos + 4 > blob_end {
                        return Err("pmapc01 pval len out of bounds".to_string());
                    }
                    let mut arr = [0u8; 4];
                    arr.copy_from_slice(&content[local_blob_pos..local_blob_pos + 4]);
                    local_blob_pos += 4;
                    u32::from_be_bytes(arr) as usize
                };
                if local_blob_pos + pval_len > blob_end {
                    return Err("pmapc01 pval bytes out of bounds".to_string());
                }
                let pval = std::str::from_utf8(
                    &content[local_blob_pos..local_blob_pos + pval_len],
                )
                .map_err(|_| "invalid utf8 in pval".to_string())?
                .to_string();
                local_blob_pos += pval_len;

                props.insert(norm_keystr_compile_like(&pkey), pval);
            }

            out.insert(phrase, props);
        }

        Ok(out)
    } else {
        Err("unknown pmapc magic".to_string())
    }
}

pub struct JsEngine {
    ctx: Context,
}

impl JsEngine {
    pub fn new() -> JsResult<Self> {
        let mut ctx = Context::default();
        Self::register_native(&mut ctx)?;
        Self::ensure_prelude(&mut ctx)?;
        let engine_id = ENGINE_ID_COUNTER.fetch_add(1, Ordering::Relaxed);
        let set_id = format!("Ts._engineId = \"{}\";", engine_id);
        let _ = ctx.eval(set_id.as_bytes())?;
        Ok(Self { ctx })
    }

    pub fn eval_module_code(&mut self, code: &str) -> JsResult<()> {
        let _ = self.ctx.eval(code.as_bytes())?;
        Ok(())
    }

    pub fn eval_module_file(&mut self, path: &Path) -> JsResult<()> {
        // ki18n：currentModulePath 影响 loadProps/子模块加载的相对路径。
        if let Some(parent) = path.parent() {
            // Even if canonicalize() fails (e.g. transient filesystem changes in tests),
            // we must still set a non-empty module path so that setcall/acall and
            // relative loadProps behave deterministically.
            let pbuf = parent.canonicalize().unwrap_or_else(|_| parent.to_path_buf());
            let p = pbuf.to_string_lossy();
            let code = format!(
                "Ts._currentModulePath = \"{}\";",
                js_escape_string(&p)
            );
            let _ = self.ctx.eval(code.as_bytes())?;
        }

        let code = fs::read_to_string(path).map_err(|e| JsValue::from(e.to_string()))?;
        self.eval_module_code(&code)
            .and_then(|_| {
                // Align with upstream KTranscriptImp::loadModules():
                // it unsets currentModulePath after module loading.
                let _ = self
                    .ctx
                    .eval("Ts._currentModulePath = '';".as_bytes())?;
                Ok(())
            })
    }

    fn register_native(ctx: &mut Context) -> JsResult<()> {
        // Debug output
        ctx.register_global_closure(
            "__native_dbgputs",
            1,
            |_this, args, _context| {
                let s = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(_context)?;
                eprintln!("[KTranscript JS dbg] {}", s);
                Ok(JsValue::undefined())
            },
        )?;

        ctx.register_global_closure(
            "__native_toUpperFirst",
            2,
            |_this, args, context| {
                let s = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(context)?;
                let s = s.to_string();
                let nalt_val = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined());
                let nalt = if nalt_val.is_undefined() || nalt_val.is_null() {
                    0
                } else {
                    nalt_val.as_number().map(|n| n as i32).unwrap_or(0)
                };
                Ok(JsString::from(to_case_first(&s, nalt, true)).into())
            },
        )?;

        ctx.register_global_closure(
            "__native_toLowerFirst",
            2,
            |_this, args, context| {
                let s = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(context)?;
                let s = s.to_string();
                let nalt_val = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined());
                let nalt = if nalt_val.is_undefined() || nalt_val.is_null() {
                    0
                } else {
                    nalt_val.as_number().map(|n| n as i32).unwrap_or(0)
                };
                Ok(JsString::from(to_case_first(&s, nalt, false)).into())
            },
        )?;

        ctx.register_global_closure(
            "__native_warnputs",
            1,
            |_this, args, _context| {
                let s = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(_context)?;
                eprintln!("[KTranscript JS warn] {}", s);
                Ok(JsValue::undefined())
            },
        )?;

        // Module loading by name — 对齐 `Scriptface::load`：仅 `currentModulePath` 下相对路径，失败即抛错。
        ctx.register_global_closure(
            "__native_load_module",
            2,
            |_this, args, context| {
                let name_js = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined());
                if !name_js.is_string() {
                    return Err(JsValue::from("Ts.load: expected string as file name"));
                }
                let module_name = name_js.to_string(context)?.to_string();
                let _lang = args
                    .get(1)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(context)?;
                // `lang` 保留与 prelude 签名一致；上游 `load` 不按 locale 回退路径。

                let cur_mod = context
                    .eval("Ts._currentModulePath".as_bytes())
                    .ok()
                    .and_then(|v| v.to_string(context).ok())
                    .map(|s| s.to_string())
                    .unwrap_or_default();
                if cur_mod.trim().is_empty() {
                    return Err(JsValue::from("Ts.load: no current module path, aiiie..."));
                }

                let mut mod_no_ext = module_name;
                if mod_no_ext.ends_with(".js") {
                    mod_no_ext.truncate(mod_no_ext.len() - 3);
                }
                let cand = PathBuf::from(&cur_mod).join(format!("{mod_no_ext}.js"));
                let cand_display = cand.display().to_string();

                let code = fs::read_to_string(&cand).map_err(|_| {
                    JsValue::from(format!("Ts.load: cannot read file '{cand_display}'"))
                })?;

                if let Some(parent) = cand.parent() {
                    let parent_canon = parent.canonicalize().unwrap_or_else(|_| parent.to_path_buf());
                    let p = parent_canon.to_string_lossy();
                    let set_code = format!(
                        "Ts._currentModulePath = \"{}\";",
                        js_escape_string(&p)
                    );
                    context.eval(set_code.as_bytes())?;
                }
                context
                    .eval(code.as_bytes())
                    .map_err(|e| JsValue::from(format!("at {cand_display}: {e:?}")))?;
                Ok(JsValue::undefined())
            },
        )?;

        // Props loading/parsing — 对齐 `Scriptface::loadProps`（路径、报错、去重）。
        ctx.register_global_closure(
            "__native_load_props",
            2,
            |_this, args, _context| {
                let rel_js = args
                    .get(0)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined());
                if !rel_js.is_string() {
                    return Err(JsValue::from("Ts.loadProps: expected string as file name"));
                }
                let relpath = rel_js.to_string(_context)?;
                let lang = args
                    .get(1)
                    .cloned()
                    .unwrap_or_else(|| JsValue::undefined())
                    .to_string(_context)?;
                let lang = lang.to_string();
                let rel = relpath.to_string();

                let cur_mod = _context
                    .eval("Ts._currentModulePath".as_bytes())
                    .ok()
                    .and_then(|v| v.to_string(_context).ok())
                    .map(|s| s.to_string())
                    .unwrap_or_default();

                if cur_mod.trim().is_empty() {
                    return Err(JsValue::from("Ts.loadProps: no current module path, aiiie..."));
                }

                let base_dir = PathBuf::from(cur_mod);

                let engine_id = _context
                    .eval("Ts._engineId".as_bytes())
                    .ok()
                    .and_then(|v| v.to_string(_context).ok())
                    .map(|s| s.to_string())
                    .unwrap_or_default();
                let merged_key = format!("__merged__:{engine_id}:{lang}");

                let pmapc_path = base_dir.join(format!("{}.pmapc", rel));
                let pmap_path = base_dir.join(format!("{}.pmap", rel));

                let pmap_err_path = pmap_path.display().to_string();
                let (load_path, bytes_opt, text_opt) =
                    if let Ok(b) = fs::read(&pmapc_path) {
                        (pmapc_path, Some(b), None)
                    } else if let Ok(t) = fs::read_to_string(&pmap_path) {
                        (pmap_path, None, Some(t))
                    } else {
                        return Err(JsValue::from(format!(
                            "Ts.loadProps: cannot read map '{pmap_err_path}'"
                        )));
                    };

                let cache_key = props_load_cache_key(&engine_id, &load_path);
                {
                    let seen = props_loaded_paths().lock().unwrap();
                    if seen.contains(&cache_key) {
                        return Ok(JsString::from(merged_key.clone()).into());
                    }
                }

                if let Some(bytes) = bytes_opt {
                    let ld = load_path.display().to_string();
                    let parsed =
                        parse_pmapc(&bytes).map_err(|e| JsValue::from(format!("Ts.loadProps: {e} ({ld})")))?;
                    let mut store = props_store().lock().unwrap();
                    let merged_store = store.entry(merged_key.clone()).or_default();
                    for (phrase, kv) in parsed {
                        let dest = merged_store.entry(phrase).or_default();
                        for (prop, val) in kv {
                            dest.insert(prop, val);
                        }
                    }
                } else if let Some(content) = text_opt {
                    let ld = load_path.display().to_string();
                    let parsed = parse_pmap_text(&content, &ld).map_err(JsValue::from)?;
                    let mut store = props_store().lock().unwrap();
                    let merged_store = store.entry(merged_key.clone()).or_default();
                    for (phrase, kv) in parsed {
                        let dest = merged_store.entry(phrase).or_default();
                        for (prop, val) in kv {
                            dest.insert(prop, val);
                        }
                    }
                }

                props_loaded_paths().lock().unwrap().insert(cache_key);

                Ok(JsString::from(merged_key).into())
            },
        )?;

        ctx.register_global_closure(
            "__native_get_prop",
            3,
            |_this, args, _context| {
                fn norm_keystr_compile_like(s: &str) -> String {
                    let mut out = String::new();
                    for ch in s.chars() {
                        if ch.is_whitespace() || ch == '&' {
                            continue;
                        }
                        out.push(ch);
                    }
                    out.to_lowercase()
                }

                let props_name = args.get(0).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let phrase = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let prop = args.get(2).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let props_name = props_name.to_string();
                let phrase = phrase.to_string();
                let prop = prop.to_string();
                let phrase = norm_keystr_compile_like(&phrase);
                let prop = norm_keystr_compile_like(&prop);

                let store = props_store().lock().unwrap();
                let out = store
                    .get(&props_name)
                    .and_then(|m| m.get(&phrase))
                    .and_then(|kv| kv.get(&prop))
                    .cloned()
                    .unwrap_or_default();
                Ok(JsString::from(out).into())
            },
        )?;

        ctx.register_global_closure(
            "__native_set_prop",
            4,
            |_this, args, _context| {
                fn norm_keystr_compile_like(s: &str) -> String {
                    let mut out = String::new();
                    for ch in s.chars() {
                        if ch.is_whitespace() || ch == '&' {
                            continue;
                        }
                        out.push(ch);
                    }
                    out.to_lowercase()
                }

                let props_name = args.get(0).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let phrase = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let prop = args.get(2).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let value = args.get(3).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let props_name = props_name.to_string();
                let phrase = phrase.to_string();
                let prop = prop.to_string();
                let phrase = norm_keystr_compile_like(&phrase);
                let prop = norm_keystr_compile_like(&prop);
                let value = value.to_string();

                let mut store = props_store().lock().unwrap();
                let file = store.entry(props_name).or_default();
                let kv = file.entry(phrase).or_default();
                kv.insert(prop, value);

                Ok(JsValue::undefined())
            },
        )?;

        // Config access.
        ctx.register_global_closure(
            "__native_get_conf_string",
            3,
            |_this, args, _context| {
                ensure_conf_loaded();
                let lang = args.get(0).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let key = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let default_val = args.get(2).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;

                let lang = lang.to_string();
                let key = key.to_string();
                let default_val = default_val.to_string();

                let store = conf_store().lock().unwrap();
                let out = store
                    .get(&lang)
                    .and_then(|m| m.get(&key))
                    .cloned()
                    .unwrap_or(default_val);
                Ok(JsString::from(out).into())
            },
        )?;

        ctx.register_global_closure(
            "__native_get_conf_bool",
            3,
            |_this, args, _context| {
                ensure_conf_loaded();
                let lang = args.get(0).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let key = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let default_val = args.get(2).cloned().unwrap_or_else(|| JsValue::undefined());

                let lang = lang.to_string();
                let key = key.to_string();
                let default_bool = default_val
                    .as_boolean()
                    .unwrap_or_else(|| {
                        // JS call arguments may come in as strings in our compatibility layer.
                        let s = default_val
                            .to_string(_context)
                            .ok()
                            .map(|v| v.to_string())
                            .unwrap_or_default();
                        s == "true"
                    });

                let store = conf_store().lock().unwrap();
                let out = store
                    .get(&lang)
                    .and_then(|m| m.get(&key))
                    .map(|v| v == "true")
                    .unwrap_or(default_bool);
                Ok(JsValue::from(out))
            },
        )?;

        ctx.register_global_closure(
            "__native_get_conf_number",
            3,
            |_this, args, _context| {
                ensure_conf_loaded();
                let lang = args.get(0).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let key = args.get(1).cloned().unwrap_or_else(|| JsValue::undefined()).to_string(_context)?;
                let default_val = args.get(2).cloned().unwrap_or_else(|| JsValue::undefined());

                let lang = lang.to_string();
                let key = key.to_string();
                let default_num = default_val
                    .as_number()
                    .unwrap_or_else(|| {
                        let s = default_val
                            .to_string(_context)
                            .ok()
                            .map(|v| v.to_string())
                            .unwrap_or_default();
                        s.parse::<f64>().unwrap_or(0.0)
                    });

                let store = conf_store().lock().unwrap();
                let out = store
                    .get(&lang)
                    .and_then(|m| m.get(&key))
                    .and_then(|v| v.parse::<f64>().ok())
                    .unwrap_or(default_num);
                Ok(JsValue::from(out))
            },
        )?;

        Ok(())
    }

    fn ensure_prelude(ctx: &mut Context) -> JsResult<()> {
        // 对齐上游 `KTranscript` / `Scriptface` 暴露给 PO 脚本的 `Ts` 接口（见 ki18n/src/i18n/ktranscript.cpp）。
        let builtin_names_js = transcript_builtins::BUILTIN_NAMES
            .iter()
            .map(|s| format!("\"{}\"", js_escape_string(s)))
            .collect::<Vec<_>>()
            .join(",");
        let prelude = [
            r#"
            var Ts = {};
            Ts._calls = {};
            Ts._fvals = {};
            Ts._fallback = false;
            Ts._propsName = null;
            Ts._callsAll = {};
            Ts._callsAllOrder = [];
            Ts._callModulePath = {};
            Ts._currentModulePath = '';
            Ts._builtinNames = ["#,
            &builtin_names_js,
            r#"];

            var ScriptfaceInternal = {};
            Ts.put = function(propertyName, value) {
                ScriptfaceInternal[propertyName] = value;
            };

            function firstAlphaIndex(s) {
                for (var i = 0; i < s.length; ++i) {
                    var c = s.charAt(i);
                    if (c.toUpperCase() !== c.toLowerCase()) return i;
                }
                return -1;
            }

            Ts.toUpperFirst = function(str, nalt) {
                if (str === null || str === undefined) return (nalt !== null && nalt !== undefined) ? nalt : "";
                str = "" + str;
                var na = (nalt === null || nalt === undefined) ? 0 : (nalt|0);
                return __native_toUpperFirst(str, na);
            };
            Ts.toLowerFirst = function(str, nalt) {
                if (str === null || str === undefined) return (nalt !== null && nalt !== undefined) ? nalt : "";
                str = "" + str;
                var na = (nalt === null || nalt === undefined) ? 0 : (nalt|0);
                return __native_toLowerFirst(str, na);
            };

            Ts.normKey = function(phrase) {
                phrase = (phrase === null || phrase === undefined) ? "" : ("" + phrase);
                phrase = phrase.toLowerCase();
                var out = "";
                for (var i = 0; i < phrase.length; ++i) {
                    var c = phrase.charAt(i);
                    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
                        out += c;
                    }
                }
                return out;
            };

            Ts.dbgputs = function(str) { __native_dbgputs(str); };
            Ts.warnputs = function(str) { __native_warnputs(str); };

            Ts.setcall = function(name, func, fval) {
                if (typeof name !== "string") throw new Error("Ts.setcall: expected string as first argument");
                if (typeof func !== "function") throw new Error("Ts.setcall: expected function as second argument");
                if (fval !== undefined && fval !== null && typeof fval !== "object")
                    throw new Error("Ts.setcall: expected object or null as third argument");
                Ts._calls[name] = func;
                Ts._fvals[name] = (fval === undefined || fval === null) ? null : fval;
                Ts._callModulePath[name] = Ts._currentModulePath;
                Ts.put('#:f<' + name + '>', func);
                Ts.put('#:o<' + name + '>', (fval === undefined || fval === null) ? null : fval);
            };
            Ts.setcallForall = function(name, func, fval) {
                Ts.setcall(name, func, fval);
                Ts.put('#:fall<' + name + '>', func);
                Ts.put('#:oall<' + name + '>', (fval === undefined || fval === null) ? null : fval);
                Ts._callsAll[name] = func;
                Ts._callsAllOrder.push(name);
            };
            Ts.hascall = function(name) {
                if (Ts._calls[name] !== undefined) return true;
                return Ts._builtinNames.indexOf(name) >= 0;
            };
            Ts.acallInternal = function(args) {
                if (!args || args.length < 1) throw new Error("Ts.acall: expected at least one argument (call name)");
                var name = args[0];
                if (typeof name !== "string") throw new Error("Ts.acall: expected string as first argument (call name)");
                var fn = Ts._calls[name];
                if (fn === undefined) throw new Error(`Ts.acall: unregistered call to '${name}'`);
                var fval = Ts._fvals[name];
                var prevModulePath = Ts._currentModulePath;
                if (Ts._callModulePath && Ts._callModulePath[name]) {
                    Ts._currentModulePath = Ts._callModulePath[name];
                }
                var callArgs = [];
                for (var i = 1; i < args.length; ++i) callArgs.push(args[i]);
                var thisObj = (fval !== null && fval !== undefined) ? fval : null;
                var r;
                try {
                    r = fn.apply(thisObj, callArgs);
                } finally {
                    Ts._currentModulePath = prevModulePath;
                }
                if (r === undefined || r === null) return "";
                return r;
            };
            Ts.acall = function() {
                var a = [];
                for (var i = 0; i < arguments.length; ++i) a.push(arguments[i]);
                return Ts.acallInternal(a);
            };
            Ts.fallback = function() { Ts._fallback = true; };

            // Message context
            Ts.msgid = function() { return _msgid; };
            Ts.msgstrf = function() { return _msgstrf; };
            Ts.msgctxt = function() { return _msgctxt; };
            Ts.msgkey = function() { return _msgctxt + '|' + _msgid; };

            Ts.nsubs = function() { return _subs.length; };
            Ts.subs = function(ind) { return _subs[ind]; };
            Ts.vals = function(ind) { return _vals[ind]; };

            Ts.dynctxt = function(key) { return (_dynctxt && _dynctxt[key]) ? _dynctxt[key] : ''; };

            Ts.localeCountry = function() { return _cur_lang; };

            Ts.load = function() {
                var cur = Ts._currentModulePath;
                if (cur === undefined || cur === null || ("" + cur).trim() === "") {
                    throw new Error("Ts.load: no current module path, aiiie...");
                }
                for (var i = 0; i < arguments.length; ++i) {
                    if (typeof arguments[i] !== "string") {
                        throw new Error("Ts.load: expected string as file name");
                    }
                    __native_load_module(arguments[i], _cur_lang);
                }
            };

            // Properties
            Ts.loadProps = function() {
                var curp = Ts._currentModulePath;
                if (curp === undefined || curp === null || ("" + curp).trim() === "") {
                    throw new Error("Ts.loadProps: no current module path, aiiie...");
                }
                var flat = [];
                for (var i = 0; i < arguments.length; ++i) {
                    var a = arguments[i];
                    if (Array.isArray(a)) {
                        for (var j = 0; j < a.length; ++j) { flat.push(a[j]); }
                    } else {
                        flat.push(a);
                    }
                }
                var propsName = null;
                for (var k = 0; k < flat.length; ++k) {
                    if (typeof flat[k] !== "string") {
                        throw new Error("Ts.loadProps: expected string as file name");
                    }
                    propsName = __native_load_props(flat[k], _cur_lang);
                }
                Ts._propsName = propsName;
            };
            Ts.getProp = function(phrase, prop) { return __native_get_prop(Ts._propsName, phrase, prop); };
            Ts.setProp = function(phrase, prop, value) { __native_set_prop(Ts._propsName, phrase, prop, value); };

            // Config
            Ts.getConfString = function(key, defValue) { return __native_get_conf_string(_cur_lang, key, defValue); };
            Ts.getConfBool = function(key, defValue) { return __native_get_conf_bool(_cur_lang, key, defValue); };
            Ts.getConfNumber = function(key, defValue) { return __native_get_conf_number(_cur_lang, key, defValue); };
        "#,
        ];
        let prelude = prelude.join("");
        let _ = ctx.eval(prelude.as_bytes())?;
        Ok(())
    }

    pub fn set_message_context(
        &mut self,
        language: &str,
        msgctxt: &str,
        msgid: &str,
        dynamic_context: &HashMap<String, String>,
        subs: &[String],
        vals: &[String],
        msgstrf: &str,
    ) -> JsResult<()> {
        let dyn_js = {
            let mut parts = Vec::new();
            for (k, v) in dynamic_context {
                parts.push(format!(
                    "\"{}\":\"{}\"",
                    js_escape_string(k),
                    js_escape_string(v)
                ));
            }
            format!("{{{}}}", parts.join(","))
        };
        let subs_js = format!(
            "[{}]",
            subs.iter()
                .map(|s| format!("\"{}\"", js_escape_string(s)))
                .collect::<Vec<_>>()
                .join(",")
        );
        let vals_js = format!(
            "[{}]",
            vals.iter()
                .map(|s| format!("\"{}\"", js_escape_string(s)))
                .collect::<Vec<_>>()
                .join(",")
        );

        let code = format!(
            r#"
            var _cur_lang = "{language}";
            var _msgctxt = "{msgctxt}";
            var _msgid = "{msgid}";
            var _msgstrf = "{msgstrf}";
            var _dynctxt = {dyn_js};
            var _subs = {subs_js};
            var _vals = {vals_js};
            Ts._fallback = false;
            "#,
            language = js_escape_string(language),
            msgctxt = js_escape_string(msgctxt),
            msgid = js_escape_string(msgid),
            msgstrf = js_escape_string(msgstrf),
            dyn_js = dyn_js,
            subs_js = subs_js,
            vals_js = vals_js
        );

        let _ = self.ctx.eval(code.as_bytes())?;
        Ok(())
    }

    /// `Ts.hascall(name)` 与 ki18n Scriptface 一致（内置名 + `Ts.setcall` 注册名）。
    pub fn eval_ts_hascall(&mut self, name: &str) -> JsResult<bool> {
        let code = format!("Ts.hascall(\"{}\")", js_escape_string(name));
        let v = self.ctx.eval(code.as_bytes())?;
        Ok(v.as_boolean().unwrap_or(false))
    }

    pub fn call_ts(&mut self, call_name: &str, args: &[String]) -> JsResult<(String, bool)> {
        // ki18n 行为：调用未注册的 Ts.* 时，会触发 fallback。
        // 这里用 Ts.hascall 来判断函数是否存在，确保后续替换逻辑与原版一致。
        let hascall_code = format!("Ts.hascall(\"{}\")", js_escape_string(call_name));
        let hascall_val = self
            .ctx
            .eval(hascall_code.as_bytes())?;
        let hascall = hascall_val.as_boolean().unwrap_or(false);
        if !hascall {
            return Ok(("".to_string(), true));
        }

        let mut js = format!("Ts._fallback = false; Ts.acall(\"{}\"", js_escape_string(call_name));
        for a in args {
            js.push_str(&format!(",\"{}\"", js_escape_string(a)));
        }
        js.push_str(")");

        let v = match self.ctx.eval(js.as_bytes()) {
            Ok(v) => v,
            Err(e) => {
                // Try to enrich the error with a quick type check.
                let typeof_expr = format!(
                    "typeof Ts._calls[\"{}\"]",
                    js_escape_string(call_name)
                );
                let typeof_to_upper = self
                    .ctx
                    .eval("typeof Ts.toUpperFirst".as_bytes())
                    .ok()
                    .and_then(|v| v.as_string().map(|s| s.to_string()));
                let typeof_first_alpha = self
                    .ctx
                    .eval("typeof firstAlphaIndex".as_bytes())
                    .ok()
                    .and_then(|v| v.as_string().map(|s| s.to_string()));
                let fn_type = self
                    .ctx
                    .eval(typeof_expr.as_bytes())
                    .ok()
                    .and_then(|v| v.as_string().map(|s| s.to_string()));

                let msg = format!(
                    "Ts.acall(\"{}\", ...) failed: {e:?} (typeof firstAlphaIndex= {:?}, typeof Ts.toUpperFirst= {:?}, typeof Ts._calls[call]= {:?})",
                    call_name, typeof_first_alpha, typeof_to_upper, fn_type
                );
                return Err(JsValue::from(msg));
            }
        };
        let fb_val = self.ctx.eval("Ts._fallback".as_bytes())?;
        let fallback = fb_val.as_boolean().unwrap_or(false);
        if fallback {
            Ok(("".to_string(), true))
        } else {
            Ok((v.to_string(&mut self.ctx)?.to_string(), false))
        }
    }

    /// Execute queued `setcallForall` calls.
    ///
    /// Upstream ki18n runs `postCalls(language)` after scripted translation
    /// evaluation, and each queued call is invoked as `Ts.acall(pcall)` with
    /// no arguments.
    pub fn execute_post_calls(&mut self) -> JsResult<()> {
        // Prefer deterministic registration order.
        let js = r#"
            if (Ts._callsAllOrder && Ts._callsAllOrder.length) {
                for (var i = 0; i < Ts._callsAllOrder.length; ++i) {
                    Ts._fallback = false;
                    Ts.acall(Ts._callsAllOrder[i]);
                }
            } else {
                for (var k in Ts._callsAll) {
                    Ts._fallback = false;
                    Ts.acall(k);
                }
            }
        "#;
        let _ = self.ctx.eval(js.as_bytes())?;
        Ok(())
    }
}

// ---------------------------------------------------------------------------
// Public API for transcript builtins (pure Rust path without JS)
// ---------------------------------------------------------------------------

/// Normalize phrase key for hash lookups. Matches ki18n's normKeystr():
/// strip whitespace, strip accelerator marker (&), convert to lowercase.
pub fn transcript_norm_keystr(s: &str) -> String {
    let without_acc = crate::core::accelerator::remove_accelerator_marker(s);
    let mut out = String::new();
    for ch in without_acc.chars() {
        if !ch.is_whitespace() {
            out.push(ch);
        }
    }
    out.to_lowercase()
}

/// Get config string for transcript builtins. Uses language as group key.
pub fn transcript_get_conf_string(language: &str, key: &str, default: &str) -> String {
    ensure_conf_loaded();
    let store = conf_store().lock().unwrap();
    store
        .get(language)
        .and_then(|m| m.get(key))
        .cloned()
        .unwrap_or_else(|| default.to_string())
}

/// Get config bool. Falsities: "0", "no", "false" (case-insensitive).
pub fn transcript_get_conf_bool(language: &str, key: &str, default: bool) -> bool {
    ensure_conf_loaded();
    const FALSITIES: &[&str] = &["0", "no", "false"];
    let store = conf_store().lock().unwrap();
    if let Some(m) = store.get(language) {
        if let Some(v) = m.get(key) {
            return !FALSITIES.contains(&v.to_lowercase().as_str());
        }
    }
    default
}

/// Get config number. Returns default if not found or parse fails.
pub fn transcript_get_conf_number(language: &str, key: &str, default: f64) -> f64 {
    ensure_conf_loaded();
    let store = conf_store().lock().unwrap();
    if let Some(m) = store.get(language) {
        if let Some(v) = m.get(key) {
            if let Ok(n) = v.parse::<f64>() {
                return n;
            }
        }
    }
    default
}

// ---------------------------------------------------------------------------
// R01: loadProps / getProp / setProp for transcript builtins
// ---------------------------------------------------------------------------

fn norm_keystr_for_props(s: &str) -> String {
    let mut out = String::new();
    for ch in s.chars() {
        if ch.is_whitespace() || ch == '&' {
            continue;
        }
        out.push(ch);
    }
    out.to_lowercase()
}

/// Load .pmap/.pmapc into props store. Returns merged key for get_prop/set_prop.
/// Base dir defaults to autotests_dir() when used from builtins.
pub fn transcript_load_props(language: &str, relpath: &str) -> String {
    let base = autotests_dir();
    let merged_key = format!("__builtin__:{}", language);

    let pmapc = base.join(format!("{}.pmapc", relpath));
    if let Ok(bytes) = fs::read(&pmapc) {
        if let Ok(parsed) = parse_pmapc(&bytes) {
            let mut store = props_store().lock().unwrap();
            let merged_store = store.entry(merged_key.clone()).or_default();
            for (phrase, kv) in parsed {
                let dest = merged_store.entry(phrase).or_default();
                for (prop, val) in kv {
                    dest.insert(prop, val);
                }
            }
            return merged_key;
        }
    }

    let pmap = base.join(format!("{}.pmap", relpath));
    if let Ok(content) = fs::read_to_string(&pmap) {
        if let Ok(parsed) = parse_pmap_text(&content, &pmap.display().to_string()) {
            let mut store = props_store().lock().unwrap();
            let merged_store = store.entry(merged_key.clone()).or_default();
            for (phrase, kv) in parsed {
                let dest = merged_store.entry(phrase).or_default();
                for (prop, val) in kv {
                    dest.insert(prop, val);
                }
            }
        }
    }
    merged_key
}

/// Get property. Use props_key from transcript_load_props (or __builtin__:{lang}).
pub fn transcript_get_prop(language: &str, relpath_or_phrase: &str, phrase_or_prop: &str, prop_opt: Option<&str>) -> String {
    let (props_key, phrase, prop) = if let Some(p) = prop_opt {
        // 3 args: (relpath, phrase, prop) - load relpath first
        let key = transcript_load_props(language, relpath_or_phrase);
        (key, phrase_or_prop, p)
    } else {
        // 2 args: (phrase, prop) - use __builtin__:{lang}
        let key = format!("__builtin__:{}", language);
        (key, relpath_or_phrase, phrase_or_prop)
    };

    let phrase_n = norm_keystr_for_props(phrase);
    let prop_n = norm_keystr_for_props(prop);

    let store = props_store().lock().unwrap();
    store
        .get(&props_key)
        .and_then(|m| m.get(&phrase_n))
        .and_then(|kv| kv.get(&prop_n))
        .cloned()
        .unwrap_or_default()
}

/// Set property in the builtin props store.
pub fn transcript_set_prop(language: &str, phrase: &str, prop: &str, value: &str) {
    let key = format!("__builtin__:{}", language);
    let phrase_n = norm_keystr_for_props(phrase);
    let prop_n = norm_keystr_for_props(prop);

    let mut store = props_store().lock().unwrap();
    let merged = store.entry(key).or_default();
    let dest = merged.entry(phrase_n).or_default();
    dest.insert(prop_n, value.to_string());
}

