// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashMap;
use std::fs;
use std::io::{self, Read};
use std::path::Path;

use thiserror::Error;

use crate::core::plural::{evaluate_plural, get_plural_forms_expression, parse_plural_forms, PluralRule};

const MO_MAGIC_LE: u32 = 0x9504_12de;
const MO_MAGIC_BE: u32 = 0xde12_0495;

#[derive(Debug, Error)]
pub enum CatalogError {
    #[error("I/O error: {0}")]
    Io(#[from] io::Error),
    #[error("Invalid MO file magic number")]
    InvalidMagic,
    #[error("Unsupported MO file format")]
    UnsupportedFormat,
    #[error("Invalid MO header")]
    InvalidHeader,
    #[error("Plural forms error: {0}")]
    Plural(#[from] crate::core::plural::PluralError),
    #[error("Invalid PO file format")]
    InvalidPo,
}

#[derive(Debug, Clone)]
pub struct MoEntry {
    pub msgid: String,
    pub msgid_plural: Option<String>,
    pub msgstr: Vec<String>,
}

#[derive(Debug, Clone)]
pub struct Catalog {
    entries: Vec<MoEntry>,
    index: HashMap<String, usize>,
    plural_rule: PluralRule,
}

impl Catalog {
    /// 无翻译条目，仅用于复数下标；与 ki18n 在「未加载到 .mo」时仍要对 msgid 做 KUIT 格式化的路径一致。
    pub fn empty_fallback() -> Self {
        let plural_rule = parse_plural_forms("nplurals=2; plural=(n != 1);")
            .expect("builtin english plural");
        Self {
            entries: Vec::new(),
            index: HashMap::new(),
            plural_rule,
        }
    }

    /// 从磁盘加载 .mo 文件并解析为 Catalog。
    pub fn load_mo(path: &Path, lang_hint: Option<&str>) -> Result<Self, CatalogError> {
        let mut file = fs::File::open(path)?;
        let mut data = Vec::new();
        file.read_to_end(&mut data)?;
        parse_mo(&data, lang_hint)
    }

    /// 从磁盘加载 .po 文件并解析为 Catalog。
    ///
    /// 仅支持常见 gettext PO 语法：
    /// - msgid/msgid_plural/msgstr/msgstr[n]
    /// - 可选 msgctxt
    /// - 多行字符串（多个连续的引号行）
    pub fn load_po(path: &Path, lang_hint: Option<&str>) -> Result<Self, CatalogError> {
        let content = fs::read_to_string(path)?;
        parse_po(&content, lang_hint)
    }

    /// 根据 msgid 查找消息（包含上下文复合 key 时，要求调用方自行组合）。
    pub fn get_message(&self, msgid: &str) -> Option<&MoEntry> {
        if let Some(&idx) = self.index.get(msgid) {
            return self.entries.get(idx);
        }
        // Backward compatibility: tolerate either gettext EOT('\x04') or legacy NUL('\0')
        // as context separators in lookup keys.
        if msgid.contains('\u{4}') {
            let alt = msgid.replace('\u{4}', "\u{0}");
            if let Some(&idx) = self.index.get(&alt) {
                return self.entries.get(idx);
            }
        } else if msgid.contains('\u{0}') {
            let alt = msgid.replace('\u{0}', "\u{4}");
            if let Some(&idx) = self.index.get(&alt) {
                return self.entries.get(idx);
            }
        }
        None
    }

    /// 计算给定 n 应该使用的复数下标。
    pub fn get_plural_form(&self, n: u64) -> usize {
        let idx = evaluate_plural(&self.plural_rule, n);
        let max = (self.plural_rule.nplurals().saturating_sub(1)) as usize;
        if idx > max {
            max
        } else {
            idx
        }
    }

    /// 获取翻译文本
    pub fn get_translated_message(&self, msgid: &str, plural_idx: usize) -> Option<&str> {
        let entry = self.get_message(msgid)?;
        if entry.msgstr.is_empty() {
            return None;
        }
        let idx = plural_idx.min(entry.msgstr.len().saturating_sub(1));
        entry.msgstr.get(idx).map(|s| s.as_str())
    }

    pub fn plural_rule(&self) -> &PluralRule {
        &self.plural_rule
    }
}

fn parse_mo(data: &[u8], lang_hint: Option<&str>) -> Result<Catalog, CatalogError> {
    if data.len() < 28 {
        return Err(CatalogError::InvalidHeader);
    }

    let magic = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
    let (is_le, magic_ok) = if magic == MO_MAGIC_LE {
        (true, true)
    } else if magic == MO_MAGIC_BE {
        (false, true)
    } else {
        (true, false)
    };

    if !magic_ok {
        return Err(CatalogError::InvalidMagic);
    }

    let read_u32 = |offset: usize, le: bool| -> u32 {
        let bytes = [data[offset], data[offset + 1], data[offset + 2], data[offset + 3]];
        if le {
            u32::from_le_bytes(bytes)
        } else {
            u32::from_be_bytes(bytes)
        }
    };

    let revision = read_u32(4, is_le);
    if revision != 0 {
        // 兼容性考虑，仅支持 revision 0
        return Err(CatalogError::UnsupportedFormat);
    }

    let nstrings = read_u32(8, is_le) as usize;
    let orig_tab_offset = read_u32(12, is_le) as usize;
    let trans_tab_offset = read_u32(16, is_le) as usize;
    let _hash_size = read_u32(20, is_le) as usize;
    let _hash_offset = read_u32(24, is_le) as usize;

    let mut entries: Vec<MoEntry> = Vec::with_capacity(nstrings);
    let mut index = HashMap::new();

    for i in 0..nstrings {
        let o_offset = orig_tab_offset + i * 8;
        let o_len = read_u32(o_offset, is_le) as usize;
        let o_off = read_u32(o_offset + 4, is_le) as usize;

        let t_offset = trans_tab_offset + i * 8;
        let t_len = read_u32(t_offset, is_le) as usize;
        let t_off = read_u32(t_offset + 4, is_le) as usize;

        if o_off + o_len > data.len() || t_off + t_len > data.len() {
            return Err(CatalogError::InvalidHeader);
        }

        let orig_bytes = &data[o_off..o_off + o_len];
        let trans_bytes = &data[t_off..t_off + t_len];

        let orig = decode_mo_string(orig_bytes);
        let trans = decode_mo_string(trans_bytes);

        // msgid 为空表示 header
        if orig.is_empty() {
            // header entry
            entries.push(MoEntry {
                msgid: String::new(),
                msgid_plural: None,
                msgstr: vec![trans.clone()],
            });
            index.insert(String::new(), entries.len() - 1);
        } else {
            // gettext .mo 中 msgctxt 采用 EOT('\x04') 连接，形式为 "ctx\x04msgid"。
            // 注意：'\0' 主要用于 plural 原文串（"msgid\0msgid_plural"）。
            let (_, _context) = if let Some(pos) = orig.find('\u{4}') {
                (orig[pos + 1..].to_string(), Some(orig[..pos].to_string()))
            } else {
                (orig.clone(), None)
            };

            let msgstr_parts: Vec<String> = trans.split('\u{0}').map(|s| s.to_string()).collect();
            let entry = MoEntry {
                msgid: orig.clone(),
                msgid_plural: None,
                msgstr: msgstr_parts,
            };

            let idx = entries.len();
            entries.push(entry);
            index.insert(orig, idx);
        }
    }

    // 解析 Plural-Forms
    let mut plural_rule_opt: Option<PluralRule> = None;
    if let Some(header_entry) = index
        .get("")
        .and_then(|&idx| entries.get(idx))
        .and_then(|e| e.msgstr.get(0))
    {
        for line in header_entry.lines() {
            let line = line.trim();
            if line.to_ascii_lowercase().starts_with("plural-forms:") {
                let header = line["Plural-Forms:".len()..].trim();
                plural_rule_opt = Some(parse_plural_forms(header)?);
                break;
            }
        }
    }

    // 如果 header 未提供，则根据语言代码推断
    if plural_rule_opt.is_none() {
        if let Some(lang) = lang_hint {
            if let Some(rule) = get_plural_forms_expression(lang) {
                plural_rule_opt = Some(rule);
            }
        }
    }

    // 仍然未确定则退化为英文规则
    let plural_rule = plural_rule_opt.unwrap_or_else(|| {
        get_plural_forms_expression("en").expect("English plural rule must be available")
    });

    Ok(Catalog {
        entries,
        index,
        plural_rule,
    })
}

fn decode_mo_string(bytes: &[u8]) -> String {
    // 移除末尾的 null 字符
    let bytes = if let Some(last) = bytes.last() { 
        if *last == 0 {
            &bytes[..bytes.len()-1]
        } else {
            bytes
        }
    } else {
        bytes
    };
    match String::from_utf8(bytes.to_vec()) {
        Ok(s) => s,
        Err(_) => bytes.iter().map(|&b| b as char).collect(), // 退化为 Latin-1
    }
}

fn parse_po(content: &str, lang_hint: Option<&str>) -> Result<Catalog, CatalogError> {
    #[derive(Default)]
    struct PoEntryBuilder {
        ctx: Option<String>,
        id: Option<String>,
        id_plural: Option<String>,
        msgstr: HashMap<usize, String>,
    }

    let mut entries: Vec<MoEntry> = Vec::new();
    let mut current = PoEntryBuilder::default();

    enum Field {
        None,
        Ctx,
        Id,
        IdPlural,
        Str(usize),
    }

    let mut current_field = Field::None;

    fn push_entry(builder: &mut PoEntryBuilder, out: &mut Vec<MoEntry>) {
        if let Some(id) = builder.id.take() {
            let mut msgstr_vec: Vec<String> = Vec::new();
            if builder.msgstr.is_empty() {
                msgstr_vec.push(String::new());
            } else {
                let max_idx = builder.msgstr.keys().copied().max().unwrap_or(0);
                for i in 0..=max_idx {
                    msgstr_vec.push(builder.msgstr.remove(&i).unwrap_or_default());
                }
            }
            out.push(MoEntry {
                msgid: id,
                msgid_plural: builder.id_plural.take(),
                msgstr: msgstr_vec,
            });
            builder.ctx = None;
        }
        builder.msgstr.clear();
    }

    fn parse_po_string(line: &str) -> Option<String> {
        let start = line.find('"')?;
        let mut s = String::new();
        let mut escaped = false;
        for ch in line[start + 1..].chars() {
            if escaped {
                s.push(match ch {
                    'n' => '\n',
                    't' => '\t',
                    '"' => '"',
                    '\\' => '\\',
                    other => other,
                });
                escaped = false;
            } else if ch == '\\' {
                escaped = true;
            } else if ch == '"' {
                break;
            } else {
                s.push(ch);
            }
        }
        Some(s)
    }

    for raw_line in content.lines() {
        let line = raw_line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }

        if line.starts_with("msgctxt") {
            push_entry(&mut current, &mut entries);
            current_field = Field::Ctx;
            current.ctx = parse_po_string(line);
        } else if line.starts_with("msgid_plural") {
            current_field = Field::IdPlural;
            current.id_plural = parse_po_string(line);
        } else if line.starts_with("msgid") {
            push_entry(&mut current, &mut entries);
            current_field = Field::Id;
            current.id = parse_po_string(line);
        } else if line.starts_with("msgstr[") {
            let idx_end = line.find(']').unwrap_or(6);
            let idx_str = &line[7..idx_end];
            let idx: usize = idx_str.parse().unwrap_or(0);
            current_field = Field::Str(idx);
            if let Some(s) = parse_po_string(line) {
                current.msgstr.insert(idx, s);
            }
        } else if line.starts_with("msgstr") {
            current_field = Field::Str(0);
            if let Some(s) = parse_po_string(line) {
                current.msgstr.insert(0, s);
            }
        } else if line.starts_with('"') {
            // 多行字符串追加
            if let Some(s) = parse_po_string(line) {
                match current_field {
                    Field::Ctx => {
                        if let Some(ref mut ctx) = current.ctx {
                            ctx.push_str(&s);
                        }
                    }
                    Field::Id => {
                        if let Some(ref mut id) = current.id {
                            id.push_str(&s);
                        }
                    }
                    Field::IdPlural => {
                        if let Some(ref mut idp) = current.id_plural {
                            idp.push_str(&s);
                        }
                    }
                    Field::Str(idx) => {
                        current
                            .msgstr
                            .entry(idx)
                            .and_modify(|v| v.push_str(&s))
                            .or_insert(s);
                    }
                    Field::None => {}
                }
            }
        } else {
            // 未知行，忽略
        }
    }

    // 推入最后一个 entry
    push_entry(&mut current, &mut entries);

    // 构建索引和复数规则
    let mut index = HashMap::new();
    for (i, e) in entries.iter().enumerate() {
        index.insert(e.msgid.clone(), i);
    }

    // 查找 header (msgid == "")
    let mut plural_rule_opt: Option<PluralRule> = None;
    if let Some(header_entry) = entries
        .iter()
        .find(|e| e.msgid.is_empty())
        .and_then(|e| e.msgstr.get(0))
    {
        for line in header_entry.lines() {
            let line = line.trim();
            if line.to_ascii_lowercase().starts_with("plural-forms:") {
                let header = line["Plural-Forms:".len()..].trim();
                plural_rule_opt = Some(parse_plural_forms(header)?);
                break;
            }
        }
    }

    if plural_rule_opt.is_none() {
        if let Some(lang) = lang_hint {
            if let Some(rule) = get_plural_forms_expression(lang) {
                plural_rule_opt = Some(rule);
            }
        }
    }

    let plural_rule = plural_rule_opt.unwrap_or_else(|| {
        get_plural_forms_expression("en").expect("English plural rule must be available")
    });

    if entries.is_empty() {
        return Err(CatalogError::InvalidPo);
    }

    Ok(Catalog {
        entries,
        index,
        plural_rule,
    })
}

