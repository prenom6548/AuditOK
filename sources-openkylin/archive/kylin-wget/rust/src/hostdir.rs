//! GNU wget `-nH/--no-host-directories` 与 `--host-directories` 的「后者生效」规则。
//!
//! 说明：GNU 的默认值为 `opt.add_hostdir = true`（见 `src/init.c:defaults()`）。
//! clap 无法完整表达「命令行中出现顺序」，因此对覆盖顺序单独扫描 `std::env::args()`。

use crate::argv_skip;
use crate::cli::Cli;

/// `true`：递归目录树落盘时应包含 host 目录；`false`：不包含。
pub fn effective_add_hostdir(cli: &Cli) -> bool {
    match scan_ordered_add_hostdir_state() {
        Some(v) => v,
        None => {
            if cli.host_directories {
                return true;
            }
            if cli.no_host_directories {
                return false;
            }
            // GNU：`-nH` 会被解析为 `-n` 的 FLAGS 含 'H'（本仓库沿用该兼容路径）。
            if cli.no_flags.as_ref().is_some_and(|s| s.contains('H')) {
                return false;
            }
            true
        }
    }
}

/// 扫描 argv，返回**最后一次**显式影响 hostdir 的状态；若无此类参数则 `None`。
fn scan_ordered_add_hostdir_state() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }

        if a == "--host-directories" {
            last = Some(true);
            i += 1;
            continue;
        }
        if a == "--no-host-directories" {
            last = Some(false);
            i += 1;
            continue;
        }

        // GNU：`wget -nH` / `-nvcH` 等短选项拼接（`-n` 后允许已知字母）
        if a.len() >= 3 && a.starts_with("-n") && !a.starts_with("--") {
            let rest = &a[2..];
            if !rest.is_empty()
                && rest
                    .chars()
                    .all(|ch| matches!(ch, 'v' | 'H' | 'd' | 'c' | 'p'))
            {
                if rest.contains('H') {
                    last = Some(false);
                }
                i += 1;
                continue;
            }
        }

        // GNU：`wget -n H`（两个参数）
        if a == "-n" && i + 1 < args.len() {
            let flags = &args[i + 1];
            if flags.starts_with("http://") || flags.starts_with("https://") {
                i += 1;
                continue;
            }
            if flags == "-" || !flags.starts_with('-') {
                if flags.contains('H') {
                    last = Some(false);
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
