use crate::exit::ExitCode;

fn scan_config_controls(argv: &[String]) -> (bool, Option<String>) {
    let mut no_config = false;
    let mut config: Option<String> = None;
    let mut i = 1usize;
    while i < argv.len() {
        let a = &argv[i];
        if a == "--" {
            break;
        }
        if a == "--no-config" {
            no_config = true;
            break;
        }
        if a == "--config" {
            if i + 1 < argv.len() {
                config = Some(argv[i + 1].clone());
                break;
            }
        }
        if let Some(v) = a.strip_prefix("--config=") {
            config = Some(v.to_string());
            break;
        }
        i += 1;
    }
    (no_config, config)
}

fn existing_file(path: &str) -> Option<String> {
    let p = std::path::Path::new(path);
    if p.exists() && p.is_file() {
        Some(path.to_string())
    } else {
        None
    }
}

fn wgetrc_env_file() -> Result<Option<String>, ExitCode> {
    let Ok(v) = std::env::var("WGETRC") else {
        return Ok(None);
    };
    let t = v.trim();
    if t.is_empty() {
        return Ok(None);
    }
    if let Some(p) = existing_file(t) {
        return Ok(Some(p));
    }
    // 对齐 GNU：WGETRC 指向不可访问文件时直接退出（GNU 为 GenericError）
    Err(ExitCode::GenericError)
}

fn wgetrc_user_file() -> Option<String> {
    let home = std::env::var("HOME").ok()?;
    let p = std::path::Path::new(&home).join(".wgetrc");
    p.to_str().and_then(|s| existing_file(s))
}

fn read_wgetrc_lines(path: &str) -> Result<Vec<String>, ExitCode> {
    let text = std::fs::read_to_string(path).map_err(|_| ExitCode::IoFail)?;
    Ok(text.lines().map(|s| s.to_string()).collect())
}

fn dehyphen(s: &str) -> String {
    s.chars()
        .filter(|&c| c != '_' && c != '-')
        .collect::<String>()
        .to_ascii_lowercase()
}

fn parse_bool(v: &str) -> Option<bool> {
    match v.trim().to_ascii_lowercase().as_str() {
        "1" | "on" | "yes" | "true" => Some(true),
        "0" | "off" | "no" | "false" => Some(false),
        _ => None,
    }
}

fn parse_line(line: &str) -> Option<(String, String)> {
    let mut t = line.trim();
    if t.is_empty() || t.starts_with('#') {
        return None;
    }
    // strip inline comments (GNU supports '#', but only when not in quotes; keep simple)
    if let Some(idx) = t.find('#') {
        t = t[..idx].trim();
    }
    if t.is_empty() {
        return None;
    }
    // command [=] value
    let (k, v) = if let Some(eq) = t.find('=') {
        (t[..eq].trim().to_string(), t[eq + 1..].trim().to_string())
    } else {
        let mut it = t.split_whitespace();
        let k = it.next()?.to_string();
        let v = it.collect::<Vec<_>>().join(" ");
        (k, v.trim().to_string())
    };
    if k.trim().is_empty() {
        return None;
    }
    Some((dehyphen(&k), v))
}

fn to_args(k: &str, v: &str) -> Vec<String> {
    // 仅对当前 wget-rust 已支持的选项做映射；未知项忽略（GNU 会报 unknown command）。
    // 这里以“注入默认值”方式实现：命令行参数会覆盖它们。
    let mut out: Vec<String> = Vec::new();
    match k {
        "quiet" => {
            if parse_bool(v) == Some(true) {
                out.push("--quiet".to_string());
            }
        }
        "verbose" => {
            if parse_bool(v) == Some(true) {
                out.push("--verbose".to_string());
            } else if parse_bool(v) == Some(false) {
                out.push("--no-verbose".to_string());
            }
        }
        "outputdocument" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push("--output-document".to_string());
                out.push(tv.to_string());
            }
        }
        "trustservernames" => {
            if parse_bool(v) == Some(true) {
                out.push("--trust-server-names".to_string());
            }
        }
        "contentdisposition" => {
            if parse_bool(v) == Some(true) {
                out.push("--content-disposition".to_string());
            }
        }
        "directoryprefix" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push("--directory-prefix".to_string());
                out.push(tv.to_string());
            }
        }
        "cutdirs" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push("--cut-dirs".to_string());
                out.push(tv.to_string());
            }
        }
        "useragent" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push("--user-agent".to_string());
                out.push(tv.to_string());
            }
        }
        "header" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push("--header".to_string());
                out.push(tv.to_string());
            }
        }
        "progress" => {
            let tv = v.trim();
            if !tv.is_empty() {
                out.push(format!("--progress={tv}"));
            }
        }
        "showprogress" => {
            if parse_bool(v) == Some(true) {
                out.push("--show-progress".to_string());
            }
        }
        "spider" => {
            if parse_bool(v) == Some(true) {
                out.push("--spider".to_string());
            }
        }
        "deleteafter" => {
            if parse_bool(v) == Some(true) {
                out.push("--delete-after".to_string());
            }
        }
        _ => {}
    }
    out
}

fn to_args_strict(k: &str, v: &str) -> Result<Vec<String>, ExitCode> {
    let out = to_args(k, v);
    if out.is_empty() {
        return Err(ExitCode::ParseError);
    }
    Ok(out)
}

pub fn inject_execute_args(argv: &[String]) -> Result<Vec<String>, ExitCode> {
    let mut out: Vec<String> = Vec::with_capacity(argv.len());
    out.push(argv.get(0).cloned().unwrap_or_else(|| "wget".to_string()));

    let mut i = 1usize;
    while i < argv.len() {
        let a = &argv[i];
        if a == "--" {
            out.extend(argv.iter().skip(i).cloned());
            break;
        }

        let mut cmd: Option<String> = None;
        if a == "-e" || a == "--execute" {
            if i + 1 >= argv.len() {
                return Err(ExitCode::ParseError);
            }
            cmd = Some(argv[i + 1].clone());
            i += 2;
        } else if let Some(v) = a.strip_prefix("--execute=") {
            cmd = Some(v.to_string());
            i += 1;
        } else {
            out.push(a.clone());
            i += 1;
        }

        if let Some(cmdopt) = cmd {
            let Some((k, v)) = parse_line(&cmdopt) else {
                return Err(ExitCode::ParseError);
            };
            let expanded = to_args_strict(&k, &v)?;
            out.extend(expanded);
        }
    }

    Ok(out)
}

fn args_from_files(files: &[String]) -> Result<Vec<String>, ExitCode> {
    let mut injected: Vec<String> = Vec::new();
    for f in files {
        for line in read_wgetrc_lines(f)? {
            if let Some((k, v)) = parse_line(&line) {
                injected.extend(to_args(&k, &v));
            }
        }
    }
    Ok(injected)
}

pub fn inject_wgetrc_args(argv: &[String]) -> Result<Vec<String>, ExitCode> {
    let (no_config, config) = scan_config_controls(argv);
    if no_config {
        return Ok(argv.to_vec());
    }

    let mut files: Vec<String> = Vec::new();
    if let Some(f) = config {
        // GNU：--config 失败即退出 parse error；这里用 GenericError 简化为非 0
        if existing_file(&f).is_none() {
            return Err(ExitCode::ParseError);
        }
        files.push(f);
    } else {
        if let Some(f) = wgetrc_env_file()? {
            files.push(f);
        } else if let Some(f) = wgetrc_user_file() {
            files.push(f);
        }
    }

    if files.is_empty() {
        return Ok(argv.to_vec());
    }
    let injected_args = args_from_files(&files)?;
    if injected_args.is_empty() {
        return Ok(argv.to_vec());
    }

    // 注入顺序：program + (wgetrc-derived args) + original args[1..]
    let mut merged: Vec<String> = Vec::new();
    merged.push(argv.get(0).cloned().unwrap_or_else(|| "wget".to_string()));
    merged.extend(injected_args);
    merged.extend(argv.iter().skip(1).cloned());
    Ok(merged)
}
