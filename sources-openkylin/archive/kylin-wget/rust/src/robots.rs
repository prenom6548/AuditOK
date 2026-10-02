use crate::cli::Cli;
use crate::client;
use crate::exit::ExitCode;
use std::collections::HashMap;
use url::Url;

#[derive(Clone, Debug)]
struct PathRule {
    path: String, // GNU: stored without leading '/'
    allowed: bool,
    user_agent_exact: bool,
}

#[derive(Clone, Debug, Default)]
struct Rules {
    paths: Vec<PathRule>, // GNU: first match wins
}

fn match_user_agent(agent: &str) -> (bool, bool) {
    // 对齐 GNU `res.c:match_user_agent()`：仅匹配 "*" 或 "wget"（大小写不敏感）。
    let a = agent.trim();
    if a == "*" {
        return (true, false);
    }
    if a.eq_ignore_ascii_case("wget") {
        return (true, true);
    }
    (false, false)
}

fn add_path(rules: &mut Rules, value: &str, allowed: bool, exact: bool) {
    let mut v = value.trim();
    if v.starts_with('/') {
        v = &v[1..];
    }
    rules.paths.push(PathRule {
        path: v.to_string(),
        allowed,
        user_agent_exact: exact,
    });
}

fn parse_rules_for_wget(text: &str) -> Rules {
    // 对齐 GNU `res_parse()` 的“record”与剪枝规则：
    // - record = 1+ User-agent 行 + 1+ Allow/Disallow 行
    // - 仅当 UA 记录匹配 "wget" 或 "*" 时收集规则
    // - 若出现过 "wget" 的 exact record，则剪掉 "*" record 的规则
    let mut rules = Rules::default();

    let mut user_agent_applies = false;
    let mut user_agent_exact = false;
    let mut found_exact = false;
    let mut record_count: i32 = 0;

    for raw in text.lines() {
        // strip leading space, ignore comment-only and end-of-line comments preceded by space
        let mut line = raw;
        // trim leading spaces
        line = line.trim_start();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        // emulate GNU: end-of-line comment begins at '#' if preceded by whitespace or start-of-line
        let mut cut = line.len();
        for (i, ch) in line.char_indices() {
            if ch == '#' {
                if i == 0 || line[..i].ends_with(char::is_whitespace) {
                    cut = i;
                    break;
                }
            }
        }
        let line = line[..cut].trim_end();
        if line.is_empty() {
            continue;
        }
        let Some((k, v)) = line.split_once(':') else {
            continue;
        };
        let key = k.trim().to_ascii_lowercase();
        let val = v.trim();

        if key == "user-agent" {
            // GNU: if record already has rules, new UA starts new record;
            // else keep trying until UA applies, then stop updating until next record.
            if record_count != 0 || !user_agent_applies {
                let (m, ex) = match_user_agent(val);
                user_agent_applies = m;
                user_agent_exact = ex;
            }
            if user_agent_exact {
                found_exact = true;
            }
            record_count = 0;
            continue;
        }

        if key == "allow" {
            if user_agent_applies {
                add_path(&mut rules, val, true, user_agent_exact);
            }
            record_count += 1;
            continue;
        }

        if key == "disallow" {
            if user_agent_applies {
                // GNU: empty disallow means allow all
                let allowed = val.is_empty();
                add_path(&mut rules, val, allowed, user_agent_exact);
            }
            record_count += 1;
            continue;
        }
    }

    if found_exact {
        rules.paths.retain(|p| p.user_agent_exact);
    }

    rules
}

fn decode_maybe_pct(s: &str) -> Vec<u8> {
    // 对齐 GNU `DECODE_MAYBE`：解码 %XX，但若解码结果为 '/' 则保持为 "%2F" 三字节的逐字符比较效果。
    let b = s.as_bytes();
    let mut out: Vec<u8> = Vec::with_capacity(b.len());
    let mut i = 0usize;
    let hex = |c: u8| -> Option<u8> {
        match c {
            b'0'..=b'9' => Some(c - b'0'),
            b'a'..=b'f' => Some(c - b'a' + 10),
            b'A'..=b'F' => Some(c - b'A' + 10),
            _ => None,
        }
    };
    while i < b.len() {
        if b[i] == b'%' && i + 2 < b.len() {
            if let (Some(h), Some(l)) = (hex(b[i + 1]), hex(b[i + 2])) {
                let decoded = (h << 4) | l;
                if decoded != b'/' {
                    out.push(decoded);
                    i += 3;
                    continue;
                }
            }
        }
        out.push(b[i]);
        i += 1;
    }
    out
}

fn matches(rule_path: &str, url_path: &str) -> bool {
    // 对齐 GNU `matches()`：rule_path 是 url_path 的“前缀”（在 DECODE_MAYBE 语义下）。
    let rp = decode_maybe_pct(rule_path);
    let up = decode_maybe_pct(url_path);
    if rp.is_empty() {
        return true;
    }
    if up.len() < rp.len() {
        return false;
    }
    up[..rp.len()] == rp[..]
}

fn path_allowed(r: &Rules, path: &str) -> bool {
    let mut p = path;
    if p.starts_with('/') {
        p = &p[1..];
    }
    for rule in &r.paths {
        if matches(&rule.path, p) {
            return rule.allowed;
        }
    }
    true
}

#[derive(Default)]
pub struct RobotsCache {
    map: HashMap<(String, u16), Rules>, // (host_low, port) -- align GNU host:port cache
}

impl RobotsCache {
    pub async fn allowed(&mut self, cli: &Cli, url: &Url) -> Result<bool, ExitCode> {
        if !cli.effective_use_robots() {
            return Ok(true);
        }
        let Some(host) = url.host_str() else {
            return Ok(true);
        };
        let host_low = host.to_ascii_lowercase();
        let port = url.port_or_known_default().unwrap_or(80);
        let key = (host_low.clone(), port);
        if !self.map.contains_key(&key) {
            let scheme = url.scheme();
            let rules = fetch_robots(cli, scheme, &host_low, port)
                .await
                .unwrap_or_default();
            self.map.insert(key.clone(), rules);
        }
        let rules = self.map.get(&key).cloned().unwrap_or_default();
        Ok(path_allowed(&rules, url.path()))
    }
}

async fn fetch_robots(
    cli: &Cli,
    scheme: &str,
    host_low: &str,
    port: u16,
) -> Result<Rules, ExitCode> {
    // GNU logs "Loading robots.txt; please ignore errors." and ignores failures.
    let robots_url = format!("{scheme}://{host_low}:{port}/robots.txt");
    let url = Url::parse(&robots_url).map_err(|_| ExitCode::ParseError)?;
    let client = client::build_client(cli, &url, 5)?;
    let resp = client
        .get(url)
        .send()
        .await
        .map_err(|e| client::classify_reqwest_error(&e))?;
    if !resp.status().is_success() {
        return Ok(Rules::default());
    }
    let bytes = resp
        .bytes()
        .await
        .map_err(|e| client::classify_reqwest_error(&e))?;
    let text = String::from_utf8_lossy(&bytes);
    Ok(parse_rules_for_wget(&text))
}
