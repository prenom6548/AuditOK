use crate::cli::Cli;

pub fn rotate_backups(cli: &Cli, path: &str) {
    let Some(n) = cli.backups else { return };
    if n == 0 {
        return;
    }

    // GNU: X -> X.1, X.1 -> X.2, ... up to N; ENOENT is not an error.
    for i in (2..=n).rev() {
        let from = format!("{path}.{}", i - 1);
        let to = format!("{path}.{i}");
        if let Err(e) = std::fs::rename(&from, &to) {
            if e.kind() != std::io::ErrorKind::NotFound {
                // best-effort; wget prints warning, we keep quiet unless verbose logging is added
            }
        }
    }

    let to1 = format!("{path}.1");
    let _ = std::fs::rename(path, &to1);
}

pub fn backup_converted_once(
    cli: &Cli,
    backed_up: &mut std::collections::HashSet<String>,
    path: &str,
) {
    if !cli.backup_converted {
        return;
    }
    if !backed_up.insert(path.to_string()) {
        return;
    }
    // 对齐 GNU convert.c：若 -E 导致写成 X.html，则备份为 X.orig（而不是 X.html.orig）
    let orig = if cli.adjust_extension && path.to_ascii_lowercase().ends_with(".html") {
        let base = &path[..path.len().saturating_sub(5)];
        format!("{base}.orig")
    } else {
        format!("{path}.orig")
    };
    let _ = std::fs::rename(path, orig);
}
