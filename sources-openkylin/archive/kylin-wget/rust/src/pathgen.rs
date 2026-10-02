use crate::cli::Cli;
use crate::restrict_names;
use url::Url;

fn default_page_name(cli: &Cli) -> &str {
    cli.effective_default_page()
}

fn local_name_last_segment(cli: &Cli, url: &Url) -> String {
    let path = url.path();
    if path.is_empty() || path.ends_with('/') {
        // GNU：目录 URL 用 default-page；若有 query，则也会拼到文件名上。
        let base = default_page_name(cli).to_string();
        if let Some(q) = url.query() {
            let sep = restrict_names::query_separator(cli).unwrap_or('?');
            let combo = format!("{base}{sep}{q}");
            return restrict_names::restrict_path_segment(cli, &combo, true).unwrap_or(combo);
        }
        return base;
    }
    let last = path.rsplit('/').find(|s| !s.is_empty()).unwrap_or("");
    if last.is_empty() {
        let base = default_page_name(cli).to_string();
        if let Some(q) = url.query() {
            let sep = restrict_names::query_separator(cli).unwrap_or('?');
            let combo = format!("{base}{sep}{q}");
            return restrict_names::restrict_path_segment(cli, &combo, true).unwrap_or(combo);
        }
        base
    } else {
        // GNU url.c:url_file_name：文件名在 append_uri_pathel 前会拼 query（若存在），并受 FN_QUERY_SEP 影响。
        let mut name = last.to_string();
        if let Some(q) = url.query() {
            let sep = restrict_names::query_separator(cli).unwrap_or('?');
            name = format!("{name}{sep}{q}");
        }
        // GNU url.c:append_uri_pathel unescapes and then escapes unsafe chars
        restrict_names::restrict_path_segment(cli, &name, true).unwrap_or(name)
    }
}

fn host_dir_with_port(cli: &Cli, url: &Url) -> String {
    let host = url.host_str().unwrap_or("unknown-host");
    let mut host_dir = host.to_string();
    if let Some(p) = url.port() {
        if let Some(def) = url.port_or_known_default() {
            if p != def {
                let sep = restrict_names::port_separator(cli).unwrap_or(':');
                host_dir = format!("{host}{sep}{p}");
            }
        } else {
            let sep = restrict_names::port_separator(cli).unwrap_or(':');
            host_dir = format!("{host}{sep}{p}");
        }
    }
    host_dir
}

/// 展开 `-P` 前缀中的 `~` / `~/`（对齐 GNU；命令行若已由 shell 展开则不变）。
fn expand_tilde_prefix(prefix: &str) -> String {
    let p = prefix.trim_end_matches('/');
    if p == "~" {
        return std::env::var("HOME").unwrap_or_else(|_| p.to_string());
    }
    if let Some(rest) = p.strip_prefix("~/") {
        if let Ok(home) = std::env::var("HOME") {
            return format!("{home}/{rest}");
        }
    }
    p.to_string()
}

fn apply_directory_prefix(cli: &Cli, path: &str) -> String {
    if let Some(prefix) = cli.directory_prefix.as_deref() {
        let p = expand_tilde_prefix(prefix);
        if !p.is_empty() {
            return format!("{p}/{path}");
        }
    }
    path.to_string()
}

/// 生成“目录树模式”的 base（不含文件名）以及 file（含 index.html 兜底）。
/// cut-dirs 只裁剪目录段，文件名段保留，行为与 GNU append_dir_structure/url_file_name 对齐。
pub fn dirstruct_base_and_file(cli: &Cli, url: &Url) -> (String, String) {
    let add_hostdir = crate::hostdir::effective_add_hostdir(cli);
    let host_dir = host_dir_with_port(cli, url);

    let p = url.path();
    let ends_with_slash = p.ends_with('/');
    let mut comps: Vec<&str> = p.split('/').filter(|s| !s.is_empty()).collect();
    if comps.is_empty() {
        comps.push(default_page_name(cli));
    } else if ends_with_slash {
        comps.push(default_page_name(cli));
    }

    let mut file = comps.pop().unwrap_or(default_page_name(cli)).to_string();
    if let Some(q) = url.query() {
        let sep = restrict_names::query_separator(cli).unwrap_or('?');
        file = format!("{file}{sep}{q}");
    }
    let mut dirs = comps;
    let cut = cli.cut_dirs.unwrap_or(0) as usize;
    if cut > 0 {
        if cut >= dirs.len() {
            dirs.clear();
        } else {
            dirs.drain(0..cut);
        }
    }

    let mut parts: Vec<String> = Vec::new();
    if cli.protocol_directories {
        parts.push(url.scheme().to_string());
    }
    if add_hostdir {
        parts.push(host_dir);
    }
    for d in dirs {
        let seg =
            restrict_names::restrict_path_segment(cli, d, true).unwrap_or_else(|_| d.to_string());
        parts.push(seg);
    }
    let base = parts.join("/");
    (base, file)
}

/// 计算最终输出路径（只负责“从 URL 推导文件名/目录树”的那条路径）。
/// - `dirstruct=false`：保存为 basename（并应用 -P 前缀）
/// - `dirstruct=true` ：保存为 [protocol/][host/][path...]（并应用 -P 前缀）
pub fn output_path_from_url(cli: &Cli, url: &Url, dirstruct: bool) -> String {
    if !dirstruct {
        let name = local_name_last_segment(cli, url);
        return apply_directory_prefix(cli, &name);
    }
    let (base, file) = dirstruct_base_and_file(cli, url);
    let path = if base.is_empty() {
        restrict_names::restrict_path_segment(cli, &file, true).unwrap_or(file)
    } else {
        let f = restrict_names::restrict_path_segment(cli, &file, true).unwrap_or(file);
        format!("{base}/{f}")
    };
    apply_directory_prefix(cli, &path)
}

/// 当最终文件名来自 Content-Disposition/--trust-server-names 等“外部 filename”时，
/// 仍需把它放进目录树 base 下（若 dirstruct=true）。
pub fn output_path_with_filename(
    cli: &Cli,
    url_for_dirs: &Url,
    filename: &str,
    dirstruct: bool,
) -> String {
    let fname = restrict_names::restrict_path_segment(cli, filename, false)
        .unwrap_or_else(|_| filename.to_string());
    if !dirstruct {
        return apply_directory_prefix(cli, &fname);
    }
    let (base, _file) = dirstruct_base_and_file(cli, url_for_dirs);
    let path = if base.is_empty() {
        fname
    } else {
        format!("{base}/{fname}")
    };
    apply_directory_prefix(cli, &path)
}
