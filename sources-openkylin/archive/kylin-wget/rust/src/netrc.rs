//! `.netrc` 解析与查找（对齐 GNU `src/netrc.c` 的可见语义子集）。
//!
//! 对齐点：
//! - 读取 `$HOME/.netrc`
//! - token 以空白分隔；支持双引号包裹；支持 `\` 逃逸（移除反斜杠）
//! - 关键字：`machine` / `default` / `login` / `password` / `macdef`
//! - `macdef`：进入宏定义模式，直到出现空行才退出（期间不解析 token）
//! - 查找：host 大小写不敏感；HTTP 不使用 default（slack_default=0），FTP 可使用 default（slack_default=1）

use std::sync::OnceLock;

#[derive(Clone, Debug)]
struct Entry {
    host: Option<String>, // None => default
    login: Option<String>,
    password: Option<String>,
}

static PARSED: OnceLock<Option<Vec<Entry>>> = OnceLock::new();

fn netrc_path() -> Option<std::path::PathBuf> {
    let home = std::env::var("HOME").ok()?;
    Some(std::path::PathBuf::from(home).join(".netrc"))
}

fn unescape_token(s: &str) -> String {
    // GNU: `\` 会“吞掉”自身（把下一个字符原样保留）；若 `\` 结尾则丢弃它。
    let mut out = String::with_capacity(s.len());
    let mut it = s.chars().peekable();
    while let Some(ch) = it.next() {
        if ch == '\\' {
            if let Some(n) = it.next() {
                out.push(n);
            } else {
                break;
            }
        } else {
            out.push(ch);
        }
    }
    out
}

fn tokenize_line(line: &str) -> Vec<String> {
    let mut out: Vec<String> = Vec::new();
    let mut i = 0usize;
    let b = line.as_bytes();
    while i < b.len() {
        // skip whitespace
        while i < b.len() && b[i].is_ascii_whitespace() {
            i += 1;
        }
        if i >= b.len() {
            break;
        }
        if b[i] == b'#' {
            break;
        }

        let mut tok = String::new();
        if b[i] == b'"' {
            i += 1;
            while i < b.len() {
                let ch = b[i] as char;
                if ch == '"' {
                    i += 1;
                    break;
                }
                if ch == '\\' {
                    i += 1;
                    if i < b.len() {
                        tok.push(b[i] as char);
                        i += 1;
                    }
                    continue;
                }
                tok.push(ch);
                i += 1;
            }
            out.push(tok);
            continue;
        }

        while i < b.len() && !b[i].is_ascii_whitespace() {
            let ch = b[i] as char;
            if ch == '\\' {
                i += 1;
                if i < b.len() {
                    tok.push(b[i] as char);
                    i += 1;
                }
                continue;
            }
            tok.push(ch);
            i += 1;
        }
        out.push(tok);
    }
    out
}

fn parse_netrc_text(text: &str) -> Vec<Entry> {
    #[derive(Copy, Clone, Debug, PartialEq, Eq)]
    enum Expect {
        Nothing,
        Machine,
        Login,
        Password,
        Macdef,
    }

    let mut entries: Vec<Entry> = Vec::new();
    let mut cur: Entry = Entry {
        host: None,
        login: None,
        password: None,
    };
    let mut expect = Expect::Nothing;
    let mut in_macdef = false;

    let flush_cur = |entries: &mut Vec<Entry>, cur: &mut Entry| {
        // GNU 会把条目加入链表，即使字段不全；这里保持宽松。
        if cur.host.is_some() || cur.login.is_some() || cur.password.is_some() {
            entries.push(cur.clone());
        }
        cur.host = None;
        cur.login = None;
        cur.password = None;
    };

    for raw_line in text.lines() {
        let line = raw_line.trim_start_matches(['\r', '\n']);
        let trimmed = line.trim();

        if in_macdef {
            if trimmed.is_empty() {
                in_macdef = false;
                expect = Expect::Nothing;
            }
            continue;
        }

        if trimmed.is_empty() {
            continue;
        }

        let toks = tokenize_line(trimmed);
        for tok in toks {
            match expect {
                Expect::Machine => {
                    flush_cur(&mut entries, &mut cur);
                    cur.host = Some(tok);
                    expect = Expect::Nothing;
                    continue;
                }
                Expect::Login => {
                    cur.login = Some(tok);
                    expect = Expect::Nothing;
                    continue;
                }
                Expect::Password => {
                    cur.password = Some(tok);
                    expect = Expect::Nothing;
                    continue;
                }
                Expect::Macdef => {
                    // macdef NAME：进入宏定义模式；直到空行退出。我们不存储宏内容。
                    let _name = tok;
                    in_macdef = true;
                    expect = Expect::Nothing;
                    continue;
                }
                Expect::Nothing => {}
            }

            match tok.as_str() {
                "machine" => expect = Expect::Machine,
                "default" => {
                    flush_cur(&mut entries, &mut cur);
                    cur.host = None;
                }
                "login" => expect = Expect::Login,
                "password" => expect = Expect::Password,
                "macdef" => expect = Expect::Macdef,
                // GNU 还识别 account/port/force，但不影响我们的行为；直接忽略其值。
                "account" | "port" | "force" => expect = Expect::Nothing,
                _ => {
                    // 未知 token：GNU 会打印警告；这里静默忽略以避免引入日志差异。
                }
            }
        }
    }

    flush_cur(&mut entries, &mut cur);

    // GNU 会把链表翻转以保持“文件顺序”。我们从上到下 append，已经是文件顺序。
    entries
}

fn load_entries() -> Option<Vec<Entry>> {
    let p = netrc_path()?;
    let text = std::fs::read_to_string(p).ok()?;
    Some(parse_netrc_text(&text))
}

fn entries() -> &'static Option<Vec<Entry>> {
    PARSED.get_or_init(load_entries)
}

fn find_host<'a>(list: &'a [Entry], host: &str) -> Option<&'a Entry> {
    let host_low = host.to_ascii_lowercase();
    for e in list {
        if let Some(h) = &e.host {
            if h.eq_ignore_ascii_case(&host_low) {
                return Some(e);
            }
        }
    }
    None
}

fn find_default<'a>(list: &'a [Entry]) -> Option<&'a Entry> {
    list.iter().find(|e| e.host.is_none())
}

/// 对齐 GNU `search_netrc`：
/// - 若 user+pass 都已有，则不做任何事
/// - 若匹配 host 且 user 为空：填充 login/password
/// - 若匹配 host 且 user 非空：只有当 login 相同才填 password，否则清空 password
/// - 若 host 无匹配且 slack_default=1：在 user 为空时尝试 default 记录
pub fn search_and_apply(
    host: &str,
    user: &mut Option<String>,
    pass: &mut Option<String>,
    slack_default: bool,
) {
    let Some(list) = entries().as_ref() else {
        return;
    };
    if user.is_some() && pass.is_some() {
        return;
    }

    if let Some(e) = find_host(list, host) {
        if let Some(u) = user.as_deref() {
            if e.login.as_deref() == Some(u) {
                if pass.is_none() {
                    *pass = e.password.clone();
                }
            } else {
                *pass = None;
            }
            return;
        }
        *user = e.login.clone().map(|s| unescape_token(&s));
        if pass.is_none() {
            *pass = e.password.clone().map(|s| unescape_token(&s));
        }
        return;
    }

    if !slack_default {
        return;
    }
    if user.is_some() {
        return;
    }
    if let Some(e) = find_default(list) {
        *user = e.login.clone().map(|s| unescape_token(&s));
        if pass.is_none() {
            *pass = e.password.clone().map(|s| unescape_token(&s));
        }
    }
}
