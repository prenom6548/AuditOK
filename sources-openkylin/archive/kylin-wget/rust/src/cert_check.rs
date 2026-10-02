//! TLS 服务端证书校验：**`--check-certificate`** / **`--no-check-certificate`** 及 argv 顺序（后者覆盖）。

use crate::argv_skip;
use crate::cli::Cli;

/// `true`：校验服务端证书（默认；HTTP URL 忽略此标志）。
pub fn verify_server_certificate(cli: &Cli) -> bool {
    match scan_ordered_verify() {
        Some(v) => v,
        None => cli_fallback(cli),
    }
}

fn cli_fallback(cli: &Cli) -> bool {
    if let Some(ref s) = cli.check_certificate {
        return parse_check_certificate_value(s);
    }
    !cli.no_check_certificate
}

/// GNU `checkcertificate`：支持 `on`/`off`/`quiet`（`quiet` 仍校验证书；仅弱警告语义与 C 版不同）。
fn parse_check_certificate_value(raw: &str) -> bool {
    let t = raw.trim();
    if t.is_empty() {
        return true;
    }
    if t.eq_ignore_ascii_case("off")
        || t == "0"
        || t.eq_ignore_ascii_case("false")
        || t.eq_ignore_ascii_case("n")
        || t.eq_ignore_ascii_case("no")
    {
        return false;
    }
    if t.eq_ignore_ascii_case("quiet") {
        return true;
    }
    true
}

fn scan_ordered_verify() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }
        if a == "--no-check-certificate" {
            last = Some(false);
            i += 1;
            continue;
        }
        if a == "--check-certificate" || a.starts_with("--check-certificate=") {
            let verify = if let Some(v) = a.strip_prefix("--check-certificate=") {
                parse_check_certificate_value(v)
            } else {
                true
            };
            last = Some(verify);
            i += 1;
            continue;
        }
        i += argv_skip::skip_option_slots(&args, i);
    }
    last
}
