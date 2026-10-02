use std::path::Path;

use crate::cli::Cli;

pub struct ResumePlan {
    pub start_from: u64,
    pub want_range: bool,
}

/// GNU wget：`--start-pos` 与 `-c` 同时给出时会关闭 continue（以 start-pos 为准）；
/// 此处不打印告警，仅对齐「Range 起点 = start-pos」。
pub fn plan_resume(cli: &Cli, out: &str) -> ResumePlan {
    if let Some(sp) = cli.start_pos {
        return ResumePlan {
            start_from: sp,
            want_range: sp > 0,
        };
    }

    if !cli.resume || out == "-" {
        return ResumePlan {
            start_from: 0,
            want_range: false,
        };
    }

    let mut start_from = 0;
    if let Ok(md) = std::fs::metadata(out) {
        if md.is_file() {
            start_from = md.len();
        }
    }

    if start_from > 0 && Path::new(out).exists() {
        ResumePlan {
            start_from,
            want_range: true,
        }
    } else {
        ResumePlan {
            start_from: 0,
            want_range: false,
        }
    }
}
