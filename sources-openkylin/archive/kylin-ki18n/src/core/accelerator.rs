// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

//! Qt/KDE 风格加速器标记移除，与 `ki18n/src/i18n/common_helpers.cpp` 中
//! `removeAcceleratorMarker` / `removeReducedCJKAccMark` 行为对齐。

/// 与 `QChar::isLetterOrNumber` 足够接近：字母与数字可作加速器目标。
#[inline]
fn is_letter_or_number(c: char) -> bool {
    c.is_alphanumeric()
}

/// 若 `pos` 指向 `(X)` 中的 `X`，且前后仅被非字母数字隔开，则在位于整段开头或结尾时去掉 `(X)`。
fn remove_reduced_cjk_acc_mark(chars: &[char], pos: usize) -> Option<Vec<char>> {
    let len = chars.len();
    if pos == 0 || pos + 1 >= len {
        return None;
    }
    if chars[pos - 1] != '('
        || chars[pos + 1] != ')'
        || !is_letter_or_number(chars[pos])
    {
        return None;
    }

    let mut p1: i32 = pos as i32 - 2;
    while p1 >= 0 && !is_letter_or_number(chars[p1 as usize]) {
        p1 -= 1;
    }
    p1 += 1;
    let p1 = p1 as usize;

    let mut p2 = pos + 2;
    while p2 < len && !is_letter_or_number(chars[p2]) {
        p2 += 1;
    }
    p2 = p2.saturating_sub(1);

    let mut out = Vec::with_capacity(len);
    if p1 == 0 {
        out.extend_from_slice(&chars[..pos - 1]);
        out.extend_from_slice(&chars[p2 + 1..]);
        Some(out)
    } else if p2 + 1 == len {
        out.extend_from_slice(&chars[..p1]);
        out.extend_from_slice(&chars[pos + 2..]);
        Some(out)
    } else {
        None
    }
}

/// 从文本中移除 Qt 风格的加速器标记（`&`），规则与 KDE `removeAcceleratorMarker` 一致。
pub fn remove_accelerator_marker(text: &str) -> String {
    let mut label: Vec<char> = text.chars().collect();
    let mut p = 0usize;
    let mut accmark_removed = false;

    loop {
        let ip = label[p..]
            .iter()
            .position(|&c| c == '&')
            .map(|i| p + i);
        let Some(ip) = ip else {
            break;
        };
        p = ip;
        if p + 1 >= label.len() {
            break;
        }
        let marker = label[p + 1];
        if is_letter_or_number(marker) {
            label.remove(p);
            if let Some(new_label) = remove_reduced_cjk_acc_mark(&label, p) {
                label = new_label;
            }
            accmark_removed = true;
        } else if marker == '&' {
            label.remove(p);
        }
        // 与上游一致：每次处理过 `&` 后前进一格（含非法 `&` 情况）。
        p += 1;
        if p > label.len() {
            break;
        }
    }

    if !accmark_removed {
        let has_cjk = label.iter().any(|c| (*c as u32) >= 0x2e00);
        if has_cjk {
            p = 0;
            while p < label.len() {
                if let Some(rel) = label[p..].iter().position(|&c| c == '(') {
                    let open = p + rel;
                    if let Some(new_label) = remove_reduced_cjk_acc_mark(&label, open + 1) {
                        label = new_label;
                    }
                    p = open + 1;
                } else {
                    break;
                }
            }
        }
    }

    label.into_iter().collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn upstream_remove_accelerator_cases() {
        assert_eq!(remove_accelerator_marker(""), "");
        assert_eq!(remove_accelerator_marker("Foo bar"), "Foo bar");
        assert_eq!(remove_accelerator_marker("&Foo bar"), "Foo bar");
        assert_eq!(remove_accelerator_marker("Foo &bar"), "Foo bar");
        assert_eq!(remove_accelerator_marker("Foo b&ar"), "Foo bar");
        assert_eq!(remove_accelerator_marker("Foo && Bar"), "Foo & Bar");
        assert_eq!(remove_accelerator_marker("Foo && &Bar"), "Foo & Bar");
        assert_eq!(remove_accelerator_marker("&Foo && Bar"), "Foo & Bar");
        assert_eq!(remove_accelerator_marker("Foo bar (&F)"), "Foo bar");
        assert_eq!(remove_accelerator_marker("(&F) Foo bar"), "Foo bar");
        assert_eq!(remove_accelerator_marker("Foo bar (&F):"), "Foo bar:");
        assert_eq!(remove_accelerator_marker("Foo bar (&F)..."), "Foo bar...");
        assert_eq!(remove_accelerator_marker("...(&F) foo bar"), "...foo bar");
        assert_eq!(remove_accelerator_marker("Foo (&F) bar"), "Foo (F) bar");
        assert_eq!(remove_accelerator_marker("Foo bar (F)"), "Foo bar (F)");
        assert_eq!(remove_accelerator_marker("&"), "&");
        assert_eq!(remove_accelerator_marker("Foo bar &"), "Foo bar &");
        assert_eq!(remove_accelerator_marker("Foo & Bar"), "Foo & Bar");
    }

    #[test]
    fn upstream_cjk_printing_mark() {
        assert_eq!(
            remove_accelerator_marker("印刷(P)..."),
            "印刷..."
        );
    }
}
