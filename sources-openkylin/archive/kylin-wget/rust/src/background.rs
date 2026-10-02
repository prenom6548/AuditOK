use crate::cli::Cli;
use crate::exit::ExitCode;
use std::process::Stdio;

const DEFAULT_LOGFILE: &str = "wget-log";
const BG_CHILD_ENV: &str = "WGET_RUST_BG_CHILD";

fn unique_create_logfile(base: &str) -> Result<Option<String>, ExitCode> {
    // 对齐 GNU `unique_create(DEFAULT_LOGFILE, ...)`：在“后台子进程启动前”创建文件并确定最终名字。
    for i in 0u32.. {
        let name = if i == 0 {
            base.to_string()
        } else {
            format!("{base}.{i}")
        };
        let r = std::fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&name);
        match r {
            Ok(f) => {
                drop(f);
                return Ok(Some(name));
            }
            Err(e) => {
                if e.kind() == std::io::ErrorKind::AlreadyExists {
                    continue;
                }
                return Err(ExitCode::IoFail);
            }
        }
    }
    Ok(None)
}

fn strip_background_flag(argv: &[String]) -> Vec<String> {
    let mut out: Vec<String> = Vec::with_capacity(argv.len());
    out.push(argv.get(0).cloned().unwrap_or_else(|| "wget".to_string()));
    let mut i = 1usize;
    while i < argv.len() {
        let a = &argv[i];
        if a == "--" {
            out.extend(argv.iter().skip(i).cloned());
            break;
        }
        if a == "-b" || a == "--background" || a.starts_with("--background=") {
            i += 1;
            continue;
        }
        out.push(a.clone());
        i += 1;
    }
    out
}

pub fn maybe_spawn_background_and_exit(
    cli: &Cli,
    argv_for_child: &[String],
) -> Result<(), ExitCode> {
    if !cli.background {
        return Ok(());
    }
    // 防止递归自我 spawn
    if std::env::var_os(BG_CHILD_ENV).is_some() {
        return Ok(());
    }

    // 对齐 GNU：仅当未指定 -o/-a 且 (非 quiet 或 server-response) 时创建默认日志文件
    let mut logfile_changed = false;
    let mut logfile_path: Option<String> = None;
    if cli.log_file.is_none()
        && cli.append_log_file.is_none()
        && (!cli.quiet || cli.server_response)
    {
        if let Some(p) = unique_create_logfile(DEFAULT_LOGFILE)? {
            logfile_changed = true;
            logfile_path = Some(p);
        }
    }

    let child_argv = strip_background_flag(argv_for_child);
    let exe = child_argv
        .get(0)
        .cloned()
        .unwrap_or_else(|| "wget".to_string());
    let mut cmd = std::process::Command::new(exe);

    // 如果我们创建了默认日志文件，则显式给子进程注入 `-o <file>`，以走既有 log 模块落盘逻辑
    if let Some(p) = &logfile_path {
        cmd.arg("-o").arg(p);
    }
    cmd.args(child_argv.iter().skip(1));
    cmd.env(BG_CHILD_ENV, "1");
    cmd.stdin(Stdio::null());
    cmd.stdout(Stdio::null());
    cmd.stderr(Stdio::null());

    let child = cmd.spawn().map_err(|_| ExitCode::GenericError)?;
    let pid = child.id();

    // parent: 对齐 GNU 的提示语（写到 stdout）
    println!(
        "{}",
        crate::i18n::tr_named(
            "Continuing in background, pid {pid}.",
            &[("pid", &pid.to_string())],
        )
    );
    if logfile_changed {
        if let Some(p) = logfile_path {
            println!(
                "{}",
                crate::i18n::tr_named(
                    "Output will be written to {path}.",
                    &[("path", &p)],
                )
            );
        }
    }
    std::process::exit(ExitCode::Success as i32);
}
