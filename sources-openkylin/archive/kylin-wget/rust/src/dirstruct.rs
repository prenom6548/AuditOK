use crate::argv_skip;
use crate::cli::Cli;

/// GNU wget 的 `dirstruct`（目录树）开关：
/// - `-x/--force-directories` => dirstruct=1
/// - `-nd/--no-directories` 以及兼容的 `-n d` / `-nd` => dirstruct=0
/// - 命令行顺序 last-wins（后出现者覆盖）
///
/// 注意：wgetrc/配置层会影响默认值；这里做的是“命令行层”的最终覆盖，用于对齐 GNU 的 last-wins。
pub fn scan_ordered_dirstruct_state() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }

        if a == "--force-directories" || a == "-x" {
            last = Some(true);
            i += 1;
            continue;
        }
        if a == "--no-directories" {
            last = Some(false);
            i += 1;
            continue;
        }

        // GNU：`-nd` 是 `-n d` 的短写（getopt 会把它解析为 -n 的 optarg="d"）
        if a == "-n" && i + 1 < args.len() {
            let flags = &args[i + 1];
            if flags.starts_with("http://") || flags.starts_with("https://") {
                i += 1;
                continue;
            }
            if flags == "-" || !flags.starts_with('-') {
                if flags.contains('d') {
                    last = Some(false);
                }
                i += 2;
                continue;
            }
            i += 1;
            continue;
        }
        if a.len() >= 3 && a.starts_with("-n") && !a.starts_with("--") {
            let rest = &a[2..];
            if !rest.is_empty()
                && rest
                    .chars()
                    .all(|ch| matches!(ch, 'v' | 'H' | 'd' | 'c' | 'p'))
            {
                if rest.contains('d') {
                    last = Some(false);
                }
                i += 1;
                continue;
            }
        }

        i += argv_skip::skip_option_slots(&args, i);
    }
    last
}

/// 是否使用目录树（dirstruct）。`default_dirstruct` 由调用方提供：
/// - 非递归单 URL：通常为 false（除非 -x 显式打开）
/// - 递归/`-p`：通常为 true（除非 -nd/-n d 显式关闭）
pub fn effective_dirstruct(cli: &Cli, default_dirstruct: bool) -> bool {
    if let Some(v) = scan_ordered_dirstruct_state() {
        return v;
    }
    if cli.force_directories {
        return true;
    }
    if cli.no_directories || cli.no_flags.as_ref().is_some_and(|s| s.contains('d')) {
        return false;
    }
    default_dirstruct
}
