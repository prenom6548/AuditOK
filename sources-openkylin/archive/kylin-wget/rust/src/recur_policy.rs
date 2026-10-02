use crate::cli::Cli;
use globset::{Glob, GlobBuilder, GlobMatcher};
use regex::Regex;
use url::Url;

fn sufmatch(host_low: &str, pat: &str) -> bool {
    // 对齐 GNU host.c:sufmatch()：
    // - 大小写不敏感
    // - 精确匹配
    // - 子域名匹配：what 以 ".pat" 结尾
    // - 若 pat 以 '.' 开头，则允许更宽松的匹配（GNU 用于 ".example.com" 这种写法）
    let raw = pat.trim();
    if raw.is_empty() {
        return false;
    }
    let host = host_low;
    let pat_low = raw.to_ascii_lowercase();

    if host == pat_low {
        return true;
    }
    if host.ends_with(&format!(".{pat_low}")) {
        return true;
    }
    if pat_low.starts_with('.') {
        let without_dot = pat_low.trim_start_matches('.');
        if without_dot.is_empty() {
            return false;
        }
        if host == without_dot {
            return true;
        }
        if host.ends_with(&format!(".{without_dot}")) {
            return true;
        }
    }
    false
}

fn percent_decode_token(s: &str) -> String {
    // 仅用于测试/过滤逻辑的稳定对齐：把 %XX 解码为字节，再按 UTF-8 lossily 还原字符串。
    let b = s.as_bytes();
    let mut out: Vec<u8> = Vec::with_capacity(b.len());
    let mut i = 0usize;
    while i < b.len() {
        if b[i] == b'%' && i + 2 < b.len() {
            let hi = b[i + 1];
            let lo = b[i + 2];
            let hex = |c: u8| -> Option<u8> {
                match c {
                    b'0'..=b'9' => Some(c - b'0'),
                    b'a'..=b'f' => Some(c - b'a' + 10),
                    b'A'..=b'F' => Some(c - b'A' + 10),
                    _ => None,
                }
            };
            if let (Some(h), Some(l)) = (hex(hi), hex(lo)) {
                out.push((h << 4) | l);
                i += 3;
                continue;
            }
        }
        out.push(b[i]);
        i += 1;
    }
    String::from_utf8_lossy(&out).to_string()
}

fn dir_of(url: &Url) -> String {
    // GNU accdir ignores leading '/' and compares directory strings.
    // GNU 的 URL 结构会对远端路径做一定程度的解码；这里按最小对齐把 %XX 解码后再做目录匹配。
    let p_raw = url.path();
    let p = percent_decode_token(p_raw);
    // GNU 传入 accdir 的是“目录路径”（不含文件名），且用于 fnmatch(FNM_PATHNAME)；
    // 这里返回不带首尾 '/' 的目录字符串（根目录为 ""）。
    let dir = if p.is_empty() || p == "/" {
        "".to_string()
    } else if p.ends_with('/') {
        p.trim_matches('/').to_string()
    } else {
        match p.rfind('/') {
            None | Some(0) => "".to_string(),
            Some(i) => p[1..i].to_string(),
        }
    };
    dir
}

fn file_of_default_page(default_page: &str, url: &Url) -> String {
    let p_raw = url.path();
    if p_raw.is_empty() || p_raw.ends_with('/') {
        // 目录 URL：对齐 GNU 递归落盘的默认页概念，让 -A/-R 能作用到它。
        return default_page.to_string();
    }
    let seg = p_raw.rsplit('/').next().unwrap_or("");
    percent_decode_token(seg)
}

fn has_html_suffix(file: &str) -> bool {
    let f = file.to_ascii_lowercase();
    f.ends_with(".html") || f.ends_with(".htm") || f.ends_with(".xhtml")
}

fn has_wildcards(p: &str) -> bool {
    p.contains('*') || p.contains('?') || p.contains('[')
}

fn subdir_p(prefix: &str, dir: &str) -> bool {
    // 对齐 GNU utils.c:subdir_p（默认大小写敏感）。
    if prefix.is_empty() {
        return true;
    }
    if dir == prefix {
        return true;
    }
    if dir.starts_with(prefix) {
        let rest = &dir[prefix.len()..];
        return rest.is_empty() || rest.starts_with('/');
    }
    false
}

fn compile_fnmatch(pat: &str) -> Option<GlobMatcher> {
    // 对齐 GNU fnmatch(..., FNM_PATHNAME)：'/' 不能被 '*' 匹配。
    GlobBuilder::new(pat)
        .literal_separator(true)
        .build()
        .ok()
        .map(|g| g.compile_matcher())
}

fn dir_matches_p(dirlist: &[String], dir: &str) -> bool {
    for raw in dirlist {
        // GNU: pattern 可带 leading '/'，要忽略
        let p = raw.trim().trim_start_matches('/');
        if p.is_empty() {
            continue;
        }
        if has_wildcards(p) {
            if let Some(m) = compile_fnmatch(p) {
                if m.is_match(dir) {
                    return true;
                }
            }
        } else if subdir_p(p, dir) {
            return true;
        }
    }
    false
}

fn in_acclist(patterns: &[String], s: &str) -> bool {
    for pat in patterns {
        let p = pat.trim();
        if p.is_empty() {
            continue;
        }
        let has_wc = p.contains('*') || p.contains('?') || p.contains('[');
        if has_wc {
            if let Ok(glob) = Glob::new(p) {
                let m = glob.compile_matcher();
                if m.is_match(s) {
                    return true;
                }
            }
        } else {
            // GNU acceptable() uses match_tail in backward mode.
            if s.ends_with(p) {
                return true;
            }
        }
    }
    false
}

pub struct RecurPolicy {
    include_dirs: Vec<String>,
    exclude_dirs: Vec<String>,
    accepts: Vec<String>,
    rejects: Vec<String>,
    domains: Vec<String>,
    exclude_domains: Vec<String>,
    default_page: String,
    accept_re: Option<Regex>,
    reject_re: Option<Regex>,
}

impl RecurPolicy {
    pub fn from_cli(cli: &Cli) -> Self {
        let include_dirs = cli.include_dirs_list();
        let exclude_dirs = cli.exclude_dirs_list();
        let accepts = cli.accept_list();
        let rejects = cli.reject_list();
        let domains = cli.domains_list();
        let exclude_domains = cli.exclude_domains_list();
        let accept_re = cli.accept_regex.as_deref().and_then(|s| Regex::new(s).ok());
        let reject_re = cli.reject_regex.as_deref().and_then(|s| Regex::new(s).ok());
        Self {
            include_dirs,
            exclude_dirs,
            accepts,
            rejects,
            domains,
            exclude_domains,
            default_page: cli.effective_default_page().to_string(),
            accept_re,
            reject_re,
        }
    }

    #[inline]
    pub fn file_for_filter(&self, url: &Url) -> String {
        file_of_default_page(&self.default_page, url)
    }

    pub fn accept_domain(&self, url: &Url) -> bool {
        let Some(host) = url.host_str() else {
            return false;
        };
        let host_low = host.to_ascii_lowercase();
        if !self.domains.is_empty() {
            if !self.domains.iter().any(|d| sufmatch(&host_low, d)) {
                return false;
            }
        }
        if !self.exclude_domains.is_empty() {
            if self.exclude_domains.iter().any(|d| sufmatch(&host_low, d)) {
                return false;
            }
        }
        true
    }

    pub fn accept_url_regex(&self, url_str: &str) -> bool {
        if let Some(re) = &self.accept_re {
            if !re.is_match(url_str) {
                return false;
            }
        }
        if let Some(re) = &self.reject_re {
            if re.is_match(url_str) {
                return false;
            }
        }
        true
    }

    pub fn accdir(&self, url: &Url) -> bool {
        let dir = dir_of(url);
        if !self.include_dirs.is_empty() {
            if !dir_matches_p(&self.include_dirs, &dir) {
                return false;
            }
        }
        if !self.exclude_dirs.is_empty() {
            if dir_matches_p(&self.exclude_dirs, &dir) {
                return false;
            }
        }
        true
    }

    pub fn acceptable_file(&self, file: &str) -> bool {
        if !self.accepts.is_empty() {
            if !in_acclist(&self.accepts, file) {
                return false;
            }
            if !self.rejects.is_empty() && in_acclist(&self.rejects, file) {
                return false;
            }
            return true;
        }
        if !self.rejects.is_empty() {
            return !in_acclist(&self.rejects, file);
        }
        true
    }

    pub fn should_apply_rules_to_child(
        &self,
        child: &Url,
        parent_depth: u32,
        max_depth: Option<u32>,
        page_requisites: bool,
    ) -> bool {
        let file = self.file_for_filter(child);
        if file.is_empty() {
            return false;
        }
        if !has_html_suffix(&file) {
            return true;
        }
        // GNU exception: non-leaf HTMLs are allowed regardless of suffix rules,
        // except when depth equals maximum depth. We approximate the same condition.
        match max_depth {
            None => false,
            Some(l) => {
                if parent_depth < l.saturating_sub(1) {
                    return false;
                }
                if page_requisites {
                    return false;
                }
                true
            }
        }
    }
}
