//! HSTS：`--hsts/--no-hsts` 与 `--hsts-file`（GNU `hsts.c` 语义子集）。
//!
//! - 文件格式对齐 GNU：`# HSTS 1.0 Known Hosts database for GNU Wget.` + 行：
//!   `<hostname>\t<port>\t<incl_subdomains>\t<created>\t<max_age>`
//! - 仅对 **HTTPS 响应**的 `Strict-Transport-Security` 生效；host 为 IP 时忽略（GNU 同样不记录 IP）。
//! - match：当 URL 为 HTTP 且命中未过期条目时，rewrite 为 HTTPS；若原端口为 80 则改 443。

use crate::argv_skip;
use crate::cli::Cli;
use crate::exit::ExitCode;
use reqwest::header::STRICT_TRANSPORT_SECURITY;
use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader, BufWriter, Write};
use std::net::IpAddr;
use std::path::{Path, PathBuf};
use std::sync::{OnceLock, RwLock};
use url::Url;

static STORE: OnceLock<HstsStore> = OnceLock::new();

#[derive(Debug, Clone, Copy)]
struct Entry {
    created: i64,
    max_age: i64,
    include_subdomains: bool,
}

#[derive(Debug, Default)]
struct HstsStore {
    // key: (host_lower, explicit_port)
    map: RwLock<HashMap<(String, u16), Entry>>,
    changed: RwLock<bool>,
}

fn store() -> &'static HstsStore {
    STORE.get_or_init(HstsStore::default)
}

pub fn enabled(cli: &Cli) -> bool {
    match scan_ordered_hsts() {
        Some(v) => v,
        None => cli.hsts && !cli.no_hsts,
    }
}

fn scan_ordered_hsts() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }
        if a == "--hsts" {
            last = Some(true);
            i += 1;
            continue;
        }
        if a == "--no-hsts" {
            last = Some(false);
            i += 1;
            continue;
        }
        i += argv_skip::skip_option_slots(&args, i);
    }
    last
}

fn default_db_path() -> Option<PathBuf> {
    let home = std::env::var("HOME").ok()?;
    Some(PathBuf::from(home).join(".wget-hsts"))
}

fn db_path(cli: &Cli) -> Option<PathBuf> {
    cli.hsts_file.clone().or_else(default_db_path)
}

pub fn init_hsts(cli: &Cli) -> Result<(), ExitCode> {
    if !enabled(cli) {
        return Ok(());
    }
    let Some(p) = db_path(cli) else {
        return Ok(());
    };
    if !p.exists() {
        return Ok(());
    }
    load_file(&p)?;
    Ok(())
}

pub fn save_hsts_at_exit(cli: &Cli) -> Result<(), ExitCode> {
    if !enabled(cli) {
        return Ok(());
    }
    let Some(p) = db_path(cli) else {
        return Ok(());
    };
    let changed = *store().changed.read().map_err(|_| ExitCode::IoFail)?;
    if !changed {
        return Ok(());
    }
    save_file(&p)?;
    Ok(())
}

pub fn maybe_rewrite_url(cli: &Cli, url: &Url) -> Url {
    if !enabled(cli) {
        return url.clone();
    }
    if url.scheme() == "https" {
        return url.clone();
    }
    if url.scheme() != "http" {
        return url.clone();
    }

    let Some(host) = url.host_str() else {
        return url.clone();
    };
    if host.parse::<IpAddr>().is_ok() {
        return url.clone();
    }

    let port = url.port().unwrap_or(80);
    let key_port = make_explicit_port_http(port);
    let host_low = host.to_ascii_lowercase();

    // congruent match
    if let Some(e) = get_entry(&host_low, key_port) {
        if !is_expired(&e) {
            return rewrite(url);
        }
        remove_entry(&host_low, key_port);
        return url.clone();
    }

    // superdomain match：foo.bar.example.com -> bar.example.com -> example.com
    // 仅当条目 include_subdomains 才允许 rewrite。
    let mut cur = host_low.as_str();
    while let Some(pos) = cur.find('.') {
        let rest = &cur[pos + 1..];
        if rest.find('.').is_none() {
            break;
        }
        cur = rest;
        if let Some(e) = get_entry(cur, key_port) {
            if is_expired(&e) {
                remove_entry(cur, key_port);
                continue;
            }
            if e.include_subdomains {
                return rewrite(url);
            }
        }
    }
    url.clone()
}

fn rewrite(url: &Url) -> Url {
    let mut u = url.clone();
    let _ = u.set_scheme("https");
    if u.port().unwrap_or(80) == 80 {
        let _ = u.set_port(Some(443));
    }
    if let Ok(mut c) = store().changed.write() {
        *c = true;
    }
    u
}

fn is_expired(e: &Entry) -> bool {
    if e.max_age <= 0 {
        return true;
    }
    let now = chrono::Utc::now().timestamp();
    e.created.saturating_add(e.max_age) < now
}

fn get_entry(host: &str, port: u16) -> Option<Entry> {
    let g = store().map.read().ok()?;
    g.get(&(host.to_string(), port)).copied()
}

fn remove_entry(host: &str, port: u16) {
    if let Ok(mut g) = store().map.write() {
        g.remove(&(host.to_string(), port));
    }
    if let Ok(mut c) = store().changed.write() {
        *c = true;
    }
}

fn make_explicit_port_https(port: u16) -> u16 {
    if port == 443 { 0 } else { port }
}
fn make_explicit_port_http(port: u16) -> u16 {
    if port == 80 { 0 } else { port }
}

pub fn observe_sts_header(cli: &Cli, url: &Url, headers: &reqwest::header::HeaderMap) {
    if !enabled(cli) {
        return;
    }
    if url.scheme() != "https" {
        return;
    }
    let Some(host) = url.host_str() else {
        return;
    };
    if host.parse::<IpAddr>().is_ok() {
        return;
    }
    let Some(v) = headers.get(STRICT_TRANSPORT_SECURITY) else {
        return;
    };
    let Ok(s) = v.to_str() else {
        return;
    };
    let (max_age, include_subdomains) = match parse_sts(s) {
        Some(x) => x,
        None => return,
    };
    if max_age < 0 {
        return;
    }
    let created = chrono::Utc::now().timestamp();
    let port = url.port().unwrap_or(443);
    let explicit_port = make_explicit_port_https(port);
    let host_low = host.to_ascii_lowercase();

    if let Ok(mut g) = store().map.write() {
        if max_age == 0 {
            g.remove(&(host_low, explicit_port));
        } else {
            g.insert(
                (host_low, explicit_port),
                Entry {
                    created,
                    max_age,
                    include_subdomains,
                },
            );
        }
    }
    if let Ok(mut c) = store().changed.write() {
        *c = true;
    }
}

fn parse_sts(raw: &str) -> Option<(i64, bool)> {
    // RFC 6797: "max-age=SECONDS" + optional "includeSubDomains"
    let mut max_age: Option<i64> = None;
    let mut include = false;
    for part in raw.split(';') {
        let p = part.trim();
        if p.is_empty() {
            continue;
        }
        if p.eq_ignore_ascii_case("includesubdomains") {
            include = true;
            continue;
        }
        let Some((k, v)) = p.split_once('=') else {
            continue;
        };
        if k.trim().eq_ignore_ascii_case("max-age") {
            let vv = v.trim().trim_matches('"');
            if let Ok(n) = vv.parse::<i64>() {
                max_age = Some(n);
            }
        }
    }
    max_age.map(|n| (n, include))
}

fn load_file(path: &Path) -> Result<(), ExitCode> {
    let f = File::open(path).map_err(|_| ExitCode::IoFail)?;
    let r = BufReader::new(f);
    for line in r.lines() {
        let line = line.map_err(|_| ExitCode::IoFail)?;
        let t = line.trim();
        if t.is_empty() || t.starts_with('#') {
            continue;
        }
        // host port include created max_age (whitespace or tab; GNU 用 sscanf)
        let parts: Vec<&str> = t.split_whitespace().collect();
        if parts.len() != 5 {
            continue;
        }
        let host = parts[0].to_ascii_lowercase();
        if host.parse::<IpAddr>().is_ok() {
            continue;
        }
        let port: u16 = parts[1].parse().unwrap_or(0);
        let include: i64 = parts[2].parse().unwrap_or(0);
        let created: i64 = parts[3].parse().unwrap_or(0);
        let max_age: i64 = parts[4].parse().unwrap_or(0);
        if max_age <= 0 {
            continue;
        }
        if let Ok(mut g) = store().map.write() {
            g.insert(
                (host, port),
                Entry {
                    created,
                    max_age,
                    include_subdomains: include != 0,
                },
            );
        }
    }
    Ok(())
}

fn save_file(path: &Path) -> Result<(), ExitCode> {
    let f = File::create(path).map_err(|_| ExitCode::IoFail)?;
    let mut w = BufWriter::new(f);
    let _ = writeln!(w, "# HSTS 1.0 Known Hosts database for GNU Wget.");
    let _ = writeln!(w, "# Edit at your own risk.");
    let _ = writeln!(
        w,
        "# <hostname>\t<port>\t<incl. subdomains>\t<created>\t<max-age>"
    );

    let g = store().map.read().map_err(|_| ExitCode::IoFail)?;
    let mut items: Vec<_> = g.iter().collect();
    items.sort_by_key(|((h, p), _)| (h.clone(), *p));
    for ((h, p), e) in items {
        writeln!(
            w,
            "{h}\t{p}\t{}\t{}\t{}",
            if e.include_subdomains { 1 } else { 0 },
            e.created,
            e.max_age
        )
        .map_err(|_| ExitCode::IoFail)?;
    }
    w.flush().map_err(|_| ExitCode::IoFail)?;
    Ok(())
}
