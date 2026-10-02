// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashMap;

use crate::core::catalog::Catalog;
use crate::core::global as core_global;
use crate::core::kuit::{
    emit_kuit_markup_xml_parse_warning,
    kuit_set_markup_warning_full_message,
    parse as kuit_parse,
    parse_wrapped_xml_markup,
    resolve_visual_format_from_context,
    salvage_markup as kuit_salvage_markup,
    to_format_with_langs_for_domain as kuit_to_format_with_langs_for_domain,
    VisualFormat,
};
use crate::core::transcript::apply_transcript_with_fallback;

#[derive(Clone, Debug)]
pub struct FieldFormat {
    pub field_width: i32,
    pub base: u32,
    pub fill_char: char,
    pub float_format: char,
    pub precision: i32,
}

impl Default for FieldFormat {
    fn default() -> Self {
        FieldFormat {
            field_width: 0,
            base: 10,
            fill_char: ' ',
            float_format: 'g',
            precision: -1,
        }
    }
}

#[derive(Clone, Debug)]
pub enum SubsArg {
    I64(i64, FieldFormat),
    U64(u64, FieldFormat),
    F64(f64, FieldFormat),
    String(String, FieldFormat),
    Char(char, FieldFormat),
    /// 嵌套消息，用于 KLocalizedString::subs(const KLocalizedString &)
    Message(Box<Message>, FieldFormat),
}

#[derive(Clone, Debug)]
pub struct Message {
    pub text: String,
    pub plural: Option<String>,
    pub context: Option<String>,
    pub domain: Option<String>,
    pub languages: Vec<String>,
    pub markup_aware: bool,
    pub subs: Vec<SubsArg>,
    pub format: VisualFormat,
    pub in_context: HashMap<String, String>,
    pub relax_subs: bool,
    pub ignore_markup: bool,
}

#[derive(Debug, thiserror::Error)]
pub enum MessageError {
    #[error("Catalog lookup failed")]
    CatalogLookup,
}

impl Message {
    pub fn new(text: &str, plural: Option<&str>, context: Option<&str>, markup: bool) -> Self {
        Message {
            text: text.to_string(),
            plural: plural.map(|s| s.to_string()),
            context: context.map(|s| s.to_string()),
            domain: None,
            languages: Vec::new(),
            markup_aware: markup,
            subs: Vec::new(),
            format: VisualFormat::UndefinedFormat,
            in_context: HashMap::new(),
            relax_subs: false,
            ignore_markup: false,
        }
    }

    pub fn select_plural_text(&self) -> String {
        if let Some(plural) = &self.plural {
            let n = self.plural_count();
            if n == 1 {
                self.text.clone()
            } else {
                plural.clone()
            }
        } else {
            self.text.clone()
        }
    }

    fn plural_count(&self) -> u64 {
        self.subs
            .iter()
            .find_map(|s| match s {
                SubsArg::I64(v, _) => Some(*v as u64),
                SubsArg::U64(v, _) => Some(*v),
                SubsArg::F64(v, _) => Some(*v as u64),
                _ => None,
            })
            .unwrap_or(0)
    }

    /// 与 `finalize` 中 catalog 查表键一致。
    pub fn catalog_key_and_plain(&self) -> (String, String) {
        match self.context.as_deref() {
            Some(ctx) if !ctx.is_empty() => {
                if let Some(pos) = self.text.find('\u{4}').or_else(|| self.text.find('\0')) {
                    if pos == ctx.len() && self.text[..pos] == *ctx {
                        (self.text.clone(), self.text[pos + 1..].to_string())
                    } else {
                        (format!("{}\u{4}{}", ctx, self.text), self.text.clone())
                    }
                } else {
                    (format!("{}\u{4}{}", ctx, self.text), self.text.clone())
                }
            }
            _ => (self.text.clone(), self.text.clone()),
        }
    }

    /// 给定 catalog 中是否存在非空译文（与 ki18n `translateRaw` 命中条件一致）。
    pub fn has_translation_in_catalog(&self, catalog: &Catalog) -> bool {
        let (catalog_key, _) = self.catalog_key_and_plain();
        if self.plural.is_some() {
            let idx = catalog.get_plural_form(self.plural_count()) as usize;
            catalog
                .get_translated_message(&catalog_key, idx)
                .is_some_and(|t| !t.is_empty())
        } else {
            catalog
                .get_translated_message(&catalog_key, 0)
                .is_some_and(|t| !t.is_empty())
        }
    }

    // 整数族 -------------------------------------------------------------------

    pub fn subs_i64(&mut self, value: i64, field_width: i32, base: u32, fill_char: char) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.base = if base == 0 { 10 } else { base };
        fmt.fill_char = fill_char;
        self.subs.push(SubsArg::I64(value, fmt));
    }

    pub fn subs_i32(&mut self, value: i32, field_width: i32, base: u32, fill_char: char) {
        self.subs_i64(value as i64, field_width, base, fill_char);
    }

    pub fn subs_i16(&mut self, value: i16, field_width: i32, base: u32, fill_char: char) {
        self.subs_i64(value as i64, field_width, base, fill_char);
    }

    pub fn subs_i8(&mut self, value: i8, field_width: i32, base: u32, fill_char: char) {
        self.subs_i64(value as i64, field_width, base, fill_char);
    }

    pub fn subs_u64(&mut self, value: u64, field_width: i32, base: u32, fill_char: char) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.base = if base == 0 { 10 } else { base };
        fmt.fill_char = fill_char;
        self.subs.push(SubsArg::U64(value, fmt));
    }

    pub fn subs_u32(&mut self, value: u32, field_width: i32, base: u32, fill_char: char) {
        self.subs_u64(value as u64, field_width, base, fill_char);
    }

    pub fn subs_u16(&mut self, value: u16, field_width: i32, base: u32, fill_char: char) {
        self.subs_u64(value as u64, field_width, base, fill_char);
    }

    pub fn subs_u8(&mut self, value: u8, field_width: i32, base: u32, fill_char: char) {
        self.subs_u64(value as u64, field_width, base, fill_char);
    }

    // 浮点族 -------------------------------------------------------------------

    pub fn subs_f64(
        &mut self,
        value: f64,
        field_width: i32,
        float_format: char,
        precision: i32,
        fill_char: char,
    ) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.float_format = if float_format == '\0' { 'g' } else { float_format };
        fmt.precision = precision;
        fmt.fill_char = fill_char;
        self.subs.push(SubsArg::F64(value, fmt));
    }

    pub fn subs_f32(
        &mut self,
        value: f32,
        field_width: i32,
        float_format: char,
        precision: i32,
        fill_char: char,
    ) {
        self.subs_f64(value as f64, field_width, float_format, precision, fill_char);
    }

    // 字符串 / 字符 -----------------------------------------------------------

    pub fn subs_string(&mut self, value: &str, field_width: i32, fill_char: char) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.fill_char = fill_char;
        self.subs
            .push(SubsArg::String(value.to_string(), fmt));
    }

    pub fn subs_char(&mut self, value: char, field_width: i32, fill_char: char) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.fill_char = fill_char;
        self.subs.push(SubsArg::Char(value, fmt));
    }

    // 嵌套消息 -----------------------------------------------------------------

    pub fn subs_message(&mut self, other: Message, field_width: i32, fill_char: char) {
        let mut fmt = FieldFormat::default();
        fmt.field_width = field_width;
        fmt.fill_char = fill_char;
        self.subs.push(SubsArg::Message(Box::new(other), fmt));
    }

    /// 使用给定 Catalog 和语言列表完成翻译和占位符替换。
    pub fn finalize(&self, catalog: &Catalog, languages: &[String]) -> Result<String, MessageError> {
        let fence = "|/|";

        // Catalog stores msgctxt by composing keys as "<ctx>\x04<msgid>".
        // Transcript, however, must receive the original msgid without msgctxt.
        let (catalog_key, msgid_plain) = self.catalog_key_and_plain();

        // 先选取 raw msgstr（包含可能的脚本 fence），稍后再拆 ordinary/scripted。
        let (raw_msgstr, plural_idx) = if let Some(plural_text) = self.plural.as_ref() {
            let n = self.plural_count();
            let idx = catalog.get_plural_form(n) as usize;
            let msgstr = catalog
                .get_translated_message(&catalog_key, idx)
                .unwrap_or_else(|| {
                    if idx == 0 {
                        msgid_plain.as_str()
                    } else {
                        plural_text.as_str()
                    }
                })
                .to_string()
                ;
            (msgstr, idx)
        } else {
            let msgstr = catalog
                .get_translated_message(&catalog_key, 0)
                .unwrap_or_else(|| msgid_plain.as_str())
                .to_string();
            (msgstr, 0usize)
        };

        // 根据 ki18n 的 script fence 拆分普通翻译 / scripted 翻译。
        // - fencePos > 0: ordinary = left(fencePos), scripted = mid(...)
        // - fencePos < 0: 只有 ordinary
        // - fencePos == 0: scripted 但缺 ordinary，在 ki18n 中会丢弃 scripted，ordinary 退回到源文本
        let (ordinary_raw, scripted_raw) = if let Some(pos) = raw_msgstr.find(fence) {
            if pos > 0 {
                (raw_msgstr[..pos].to_string(), raw_msgstr[pos + fence.len()..].to_string())
            } else {
                let fallback_src = if self.plural.is_none() || plural_idx == 0 {
                    msgid_plain.clone()
                } else {
                    self.plural.clone().unwrap_or_default()
                };
                (fallback_src, String::new())
            }
        } else {
            (raw_msgstr, String::new())
        };

        // 先解析嵌套消息，再应用占位符替换
        let resolved_subs = self.resolve_nested_subs(catalog, languages)?;
        let ordinary_text = self.apply_subs_with(&ordinary_raw, &resolved_subs);

        // 为 transcript 准备参数列表：所有占位符展开后的字符串
        let msg_args: Vec<String> = resolved_subs.iter().map(format_arg).collect();
        let lang_code_owned = languages
            .get(0)
            .cloned()
            .unwrap_or_else(|| "en".to_string());

        // transcript 内置函数（如 msgid/msgstrf/msgctxt/dynctxt 等）依赖 context 中的保留键。
        let mut transcript_ctx = self.in_context.clone();
        transcript_ctx.insert("msgid".to_string(), msgid_plain.clone());
        transcript_ctx.insert(
            "msgctxt".to_string(),
            self.context.clone().unwrap_or_default(),
        );
        // msgstrf 对齐 ki18n：scripted 求值的普通翻译输入。
        transcript_ctx.insert("msgstrf".to_string(), ordinary_text.clone());
        let msgkey = format!(
            "{}|{}",
            transcript_ctx.get("msgctxt").unwrap_or(&String::new()),
            transcript_ctx.get("msgid").unwrap_or(&String::new())
        );
        transcript_ctx.insert("msgkey".to_string(), msgkey);

        // ki18n：只有 scriptedTranslation 非空才会求值并可能覆盖 finalTranslation。
        let mut base_text = ordinary_text;
        if !scripted_raw.is_empty() {
            let scripted_text = self.apply_subs_with(&scripted_raw, &resolved_subs);
            let (tr_out, tr_fallback) = apply_transcript_with_fallback(
                &scripted_text,
                &transcript_ctx,
                &msg_args,
                catalog,
                &lang_code_owned,
            );
            if !tr_out.is_empty() && !tr_fallback {
                base_text = tr_out;
            }
        }

        // KUIT 解析（按消息 domain + 应用 domain 选用 `Kuit::setupForDomain` 对应配置）
        if self.markup_aware && !self.ignore_markup {
            let kuit_domain = self
                .domain
                .as_deref()
                .map(str::trim)
                .filter(|s| !s.is_empty())
                .map(|s| s.to_string())
                .or_else(core_global::get_application_domain)
                .unwrap_or_default();
            let fmt = resolve_visual_format_from_context(
                self.context.as_deref(),
                self.format,
                &kuit_domain,
            );
            let trimmed = base_text.trim_start();
            kuit_set_markup_warning_full_message(Some(base_text.as_str()));
            // 与上游 `toVisualText`：含 `<` 时先整体包在内部 `<root>` 下解析；失败则警告并 `salvageMarkup`。
            let out = if !trimmed.contains('<') {
                let node = kuit_parse(&base_text);
                kuit_to_format_with_langs_for_domain(&node, fmt, languages, &kuit_domain)
            } else {
                match parse_wrapped_xml_markup(trimmed) {
                    Ok(node) => {
                        kuit_to_format_with_langs_for_domain(&node, fmt, languages, &kuit_domain)
                    }
                    Err(e) => {
                        emit_kuit_markup_xml_parse_warning(trimmed, &e);
                        kuit_salvage_markup(&base_text, fmt, languages, &kuit_domain)
                    }
                }
            };
            kuit_set_markup_warning_full_message(None);
            Ok(out)
        } else {
            Ok(base_text)
        }
    }

    /// 占位符替换逻辑。
    ///
    /// 支持：
    /// - %1, %2, ...   对应 subs 中的第 i 个参数
    /// - %L1, %L2, ... Locale-aware 目前按普通格式处理
    pub fn apply_subs(&self, text: &str) -> String {
        self.apply_subs_with(text, &self.subs)
    }

    fn apply_subs_with(&self, text: &str, subs: &[SubsArg]) -> String {
        use std::collections::HashSet;

        let mut result = String::new();
        let mut chars = text.chars().peekable();
        let mut used_indices: HashSet<usize> = HashSet::new();
        let mut max_index: usize = 0;
        let mut missing_argument = false;

        while let Some(ch) = chars.next() {
            if ch == '%' {
                let mut is_localized = false;
                if let Some('L') = chars.peek().copied() {
                    is_localized = true;
                    chars.next();
                }

                // 读取占位符数字
                let mut digits = String::new();
                while let Some(c2) = chars.peek().copied() {
                    if c2.is_ascii_digit() {
                        digits.push(c2);
                        chars.next();
                    } else {
                        break;
                    }
                }

                if digits.is_empty() {
                    // 单独的 '%' 或 '%L'，按字面量处理
                    result.push('%');
                    if is_localized {
                        result.push('L');
                    }
                    continue;
                }

                let index: usize = digits.parse().unwrap_or(0);

                // C++ 语义：%0 不作为占位符处理，保留原样
                if index == 0 {
                    result.push('%');
                    if is_localized {
                        result.push('L');
                    }
                    result.push_str(&digits);
                    continue;
                }

                max_index = max_index.max(index);
                if let Some(arg) = subs.get(index - 1) {
                    let formatted = format_arg(arg);
                    result.push_str(&formatted);
                    used_indices.insert(index);
                } else {
                    // 对应参数不存在：保留原始占位符，并记录缺失标记
                    result.push('%');
                    if is_localized {
                        result.push('L');
                    }
                    result.push_str(&digits);
                    missing_argument = true;
                }
            } else {
                result.push(ch);
            }
        }

        // 调试标记（与 C++ 行为对齐）
        let mut markers = String::new();

        if missing_argument && !self.relax_subs {
            markers.push_str("(I18N_ARGUMENT_MISSING)");
        }

        if max_index > 0 {
            // C++ `substituteSimple`：复数消息里第一个数值型 subs 的下标记为 `numberOrdinal`，
            // 即使 singular/plural 模板里不出现对应 `%n`，也视为已使用，避免误报 GAPS
            //（如 `i18np("A pod left on %2", "%1 pods left on %2", 1, "Discovery")`）。
            let plural_numeric_1based = if self.plural.is_some() {
                subs
                    .iter()
                    .position(|s| {
                        matches!(
                            s,
                            SubsArg::I64(..) | SubsArg::U64(..) | SubsArg::F64(..)
                        )
                    })
                    .map(|i| i + 1)
            } else {
                None
            };

            // C++ 语义：占位符空洞或参数数量与模板不匹配 → GAPS 标记。
            let mut has_gap = false;
            if max_index > subs.len() {
                has_gap = true;
            } else {
                for k in 1..=max_index {
                    let filled_by_plural = plural_numeric_1based == Some(k);
                    if !used_indices.contains(&k) && !filled_by_plural {
                        has_gap = true;
                        break;
                    }
                }
            }
            if has_gap {
                if !self.relax_subs {
                    markers.push_str("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)");
                }
            }
        }

        if self.plural.is_some() && !self.relax_subs {
            let has_numeric = subs.iter().any(|s| {
                matches!(
                    s,
                    SubsArg::I64(..) | SubsArg::U64(..) | SubsArg::F64(..)
                )
            });
            if !has_numeric {
                markers.push_str("(I18N_PLURAL_ARGUMENT_MISSING)");
            }
        }

        result.push_str(&markers);
        result
    }

    fn resolve_nested_subs(
        &self,
        catalog: &Catalog,
        languages: &[String],
    ) -> Result<Vec<SubsArg>, MessageError> {
        let mut resolved = Vec::with_capacity(self.subs.len());
        for arg in &self.subs {
            match arg {
                SubsArg::Message(msg, fmt) => {
                    let s = msg.finalize(catalog, languages)?;
                    resolved.push(SubsArg::String(s, fmt.clone()));
                }
                other => resolved.push(other.clone()),
            }
        }
        Ok(resolved)
    }
}

fn format_arg(arg: &SubsArg) -> String {
    match arg {
        SubsArg::I64(v, f) => format_integer(*v, f),
        SubsArg::U64(v, f) => format_unsigned(*v, f),
        SubsArg::F64(v, f) => format_float(*v, f),
        SubsArg::String(s, f) => format_with_width(s, f),
        SubsArg::Char(c, f) => format_with_width(&c.to_string(), f),
        SubsArg::Message(_, _) => String::new(), // 实际不会到这里，已在 resolve_nested_subs 中展开
    }
}

fn format_integer(v: i64, f: &FieldFormat) -> String {
    let base = f.base.max(2).min(36);
    let mut s = if base == 10 {
        v.to_string()
    } else {
        // 手动实现进制转换，保证与 base 一致
        let mut n = if v < 0 { -v as u64 } else { v as u64 };
        let mut digits = Vec::new();
        if n == 0 {
            digits.push('0');
        } else {
            while n > 0 {
                let d = (n % base as u64) as u8;
                let ch = match d {
                    0..=9 => (b'0' + d) as char,
                    10..=35 => (b'a' + (d - 10)) as char,
                    _ => '?',
                };
                digits.push(ch);
                n /= base as u64;
            }
        }
        if v < 0 {
            digits.push('-');
        }
        digits.iter().rev().collect()
    };
    apply_field_width(&mut s, f);
    s
}

fn format_unsigned(v: u64, f: &FieldFormat) -> String {
    let base = f.base.max(2).min(36);
    let mut n = v;
    let mut digits = Vec::new();
    if n == 0 {
        digits.push('0');
    } else {
        while n > 0 {
            let d = (n % base as u64) as u8;
            let ch = match d {
                0..=9 => (b'0' + d) as char,
                10..=35 => (b'a' + (d - 10)) as char,
                _ => '?',
            };
            digits.push(ch);
            n /= base as u64;
        }
    }
    let mut s: String = digits.iter().rev().collect();
    apply_field_width(&mut s, f);
    s
}

fn format_float(v: f64, f: &FieldFormat) -> String {
    let decimal_sep = get_locale_decimal_separator();
    let decimal_sep_str = decimal_sep.to_string();
    
    let mut s = if f.precision >= 0 {
        match f.float_format {
            'e' | 'E' => format!("{:.*e}", f.precision as usize, v),
            'f' | 'F' => {
                let formatted = format!("{:.*}", f.precision as usize, v);
                formatted.replace('.', &decimal_sep_str)
            }
            'g' | 'G' | _ => {
                let formatted = format!("{:.*}", f.precision as usize, v);
                formatted.replace('.', &decimal_sep_str)
            }
        }
    } else {
        match f.float_format {
            'e' | 'E' => format!("{:e}", v),
            'f' | 'F' => format!("{}", v).replace('.', &decimal_sep_str),
            'g' | 'G' | _ => format!("{}", v).replace('.', &decimal_sep_str),
        }
    };
    apply_field_width(&mut s, f);
    s
}

fn get_locale_decimal_separator() -> char {
    if let Some(locale) = sys_locale::get_locale() {
        let locale_lower = locale.to_lowercase();
        let lang = locale_lower.split('_').next().unwrap_or(&locale_lower);
        match lang {
            "ca" | "es" | "de" | "fr" | "it" | "pt" | "nl" | "pl" | "ru" | "uk" | "cs" | "sk" | "hu" | "ro" | "bg" | "hr" | "sr" | "sl" | "et" | "lv" | "lt" | "fi" | "sv" | "no" | "da" | "el" | "tr" | "ar" | "he" | "fa" | "vi" | "th" | "id" | "ms" | "fil" => ',',
            _ => '.',
        }
    } else {
        '.'
    }
}

fn format_with_width(s: &str, f: &FieldFormat) -> String {
    let mut owned = s.to_string();
    apply_field_width(&mut owned, f);
    owned
}

fn apply_field_width(s: &mut String, f: &FieldFormat) {
            let width = f.field_width;
            if width == 0 {
                return;
            }
            let abs_width = width.abs();
            let len = s.chars().count() as i32;
            if len >= abs_width {
                return;
            }
            let pad_len = (abs_width - len) as usize;
            let pad: String = std::iter::repeat(f.fill_char).take(pad_len).collect();
            if width > 0 {
                // 正的 field宽度：右对齐（左侧填充)
                s.insert_str(0, &pad);
            } else {
                // 负的 field宽度：左对齐 (右侧填充)
                s.push_str(&pad);
            }
}

