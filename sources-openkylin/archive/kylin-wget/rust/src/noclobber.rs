//! GNU wget `noclobber`：**`--no-clobber` / `-nc` / `-n c` / `--clobber`** 等与「后者生效」规则。
//!
//! clap 无法完整表达「命令行中出现顺序」，因此对覆盖顺序单独扫描 `std::env::args()`。

use crate::argv_skip;
use crate::cli::Cli;

/// `true`：文件已存在时应跳过（GNU `noclobber`）。
pub fn effective_noclobber(cli: &Cli) -> bool {
    match scan_ordered_noclobber_state() {
        Some(v) => v,
        None => cli.no_clobber || cli.no_flags.as_ref().is_some_and(|s| s.contains('c')),
    }
}

/// 扫描 argv，返回**最后一次**显式影响 noclobber 的状态；若无此类参数则 `None`。
fn scan_ordered_noclobber_state() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }

        if a == "--no-clobber" {
            last = Some(true);
            i += 1;
            continue;
        }

        if a == "--clobber" || a.starts_with("--clobber=") {
            let allow_overwrite = if let Some(v) = a.strip_prefix("--clobber=") {
                gnu_clobber_value_allows_overwrite(v)
            } else {
                true
            };
            last = Some(!allow_overwrite);
            i += 1;
            continue;
        }

        if a == "-nc" {
            last = Some(true);
            i += 1;
            continue;
        }

        // GNU：`wget -nvc` / `-nc` 等短选项拼接（`-n` 后仅允许已知字母）
        if a.len() >= 3 && a.starts_with("-n") && !a.starts_with("--") {
            let rest = &a[2..];
            if !rest.is_empty()
                && rest
                    .chars()
                    .all(|ch| matches!(ch, 'v' | 'H' | 'd' | 'c' | 'p'))
            {
                if rest.contains('c') {
                    last = Some(true);
                }
                i += 1;
                continue;
            }
        }

        // GNU：`wget -n c`（两个参数）
        if a == "-n" && i + 1 < args.len() {
            let flags = &args[i + 1];
            if flags.starts_with("http://") || flags.starts_with("https://") {
                i += 1;
                continue;
            }
            if flags == "-" || !flags.starts_with('-') {
                if flags.contains('c') {
                    last = Some(true);
                }
                i += 2;
                continue;
            }
            i += 1;
            continue;
        }

        i += argv_skip::skip_option_slots(&args, i);
    }
    last
}

/// `--clobber[=WHEN]`：`WHEN` 为「允许覆盖」时为真（GNU `OPT__CLOBBER` 语义）。
fn gnu_clobber_value_allows_overwrite(v: &str) -> bool {
    let v = v.trim();
    if v.is_empty() {
        return true;
    }
    if v == "1" || v.eq_ignore_ascii_case("y") {
        return true;
    }
    if v.len() >= 2 {
        let b = v.as_bytes();
        if b[0].eq_ignore_ascii_case(&b'o') && b[1].eq_ignore_ascii_case(&b'n') {
            return true;
        }
    }
    false
}
