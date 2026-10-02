use crate::backups;
use crate::cli::Cli;
use crate::css_urls;
use crate::dirstruct;
use crate::exit::ExitCode;
use crate::ftp;
use crate::html_urls;
use crate::http;
use crate::log;
use crate::pathgen;
use crate::recur_policy::RecurPolicy;
use crate::robots::RobotsCache;
use std::collections::{HashSet, VecDeque};
use std::path::Path;
use url::Url;

#[derive(Clone, Debug)]
struct RootPolicy {
    host_low: String,
    port: u16,
    scheme: String,
    // e.g. "/recur/" (always ends with '/')
    no_parent_prefix: String,
}

#[derive(Clone, Debug)]
struct QueueItem {
    url: Url,
    depth: u32,
    root: RootPolicy,
    inline_only: bool,
}

fn is_same_host_port(a: &Url, host_low: &str, port: u16) -> bool {
    let Some(h) = a.host_str() else {
        return false;
    };
    if h.to_ascii_lowercase() != host_low {
        return false;
    }
    a.port_or_known_default().unwrap_or(port) == port
}

fn dir_prefix_for(url: &Url) -> String {
    let p = url.path();
    if p.is_empty() || p == "/" {
        return "/".to_string();
    }
    if p.ends_with('/') {
        return p.to_string();
    }
    match p.rfind('/') {
        Some(0) => "/".to_string(),
        Some(i) => format!("{}/", &p[..i]),
        None => "/".to_string(),
    }
}

fn local_name_last_segment(url: &Url) -> String {
    let path = url.path();
    let last = path.rsplit('/').find(|s| !s.is_empty()).unwrap_or("");
    if last.is_empty() {
        "index.html".to_string()
    } else {
        last.to_string()
    }
}

fn local_path_for_recursive(cli: &Cli, url: &Url) -> String {
    let dirstruct_on = dirstruct::effective_dirstruct(cli, true);
    pathgen::output_path_from_url(cli, url, dirstruct_on)
}

fn relative_href(from_file: &str, to_file: &str) -> String {
    let from_parent = Path::new(from_file)
        .parent()
        .unwrap_or_else(|| Path::new(""));
    let to_path = Path::new(to_file);
    let mut from_parts: Vec<&str> = from_parent
        .components()
        .filter_map(|c| c.as_os_str().to_str())
        .collect();
    let mut to_parts: Vec<&str> = to_path
        .components()
        .filter_map(|c| c.as_os_str().to_str())
        .collect();

    // find common prefix
    while !from_parts.is_empty() && !to_parts.is_empty() && from_parts[0] == to_parts[0] {
        from_parts.remove(0);
        to_parts.remove(0);
    }
    let mut rel: Vec<String> = Vec::new();
    for _ in 0..from_parts.len() {
        rel.push("..".to_string());
    }
    for p in to_parts {
        if !p.is_empty() {
            rel.push(p.to_string());
        }
    }
    if rel.is_empty() {
        ".".to_string()
    } else {
        rel.join("/")
    }
}

fn local_href_for(cli: &Cli, from_file: &str, url: &Url) -> String {
    // -nd/-x 冲突需按 argv 顺序 last-wins；这里以最终 dirstruct 是否开启为准。
    let dirstruct_on = dirstruct::effective_dirstruct(cli, true);
    if !dirstruct_on {
        return local_name_last_segment(url);
    }
    let to_file = local_path_for_recursive(cli, url);
    relative_href(from_file, &to_file)
}

fn convert_basename_only(raw_attr: &str, local_name: &str) -> String {
    // 对齐 GNU convert.c: convert_basename()
    // 仅替换原 URL 的 basename 为 local_name 的 basename，保留 dirname（含 query 等不在此实现中拆分）。
    let raw = raw_attr.trim().trim_matches(|c| c == '"' || c == '\'');
    let org_base = raw.rsplit('/').next().unwrap_or(raw);
    let local_base = local_name.rsplit('/').next().unwrap_or(local_name);
    if org_base == local_base {
        return raw.to_string();
    }
    // graft: replace text after last '/' (or whole string if no '/')
    if let Some(i) = raw.rfind('/') {
        format!("{}{}", &raw[..=i], local_base)
    } else {
        local_base.to_string()
    }
}

fn rewrite_html_links(
    cli: &Cli,
    html: &str,
    page_url: &Url,
    item: &QueueItem,
    will_download: &HashSet<String>,
    out_file: &str,
) -> String {
    if !cli.effective_convert_links() && !cli.effective_convert_file_only() {
        return html.to_string();
    }

    let links = html_urls::extract_links_from_html(html, Some(page_url), Path::new("<downloaded>"));
    if links.is_empty() {
        return html.to_string();
    }

    let mut out = html.to_string();
    for l in links {
        let Ok(u) = Url::parse(&l.resolved) else {
            continue;
        };
        if !should_follow(cli, &u, page_url, item) {
            continue;
        }
        let local_path = local_href_for(cli, out_file, &u);
        if local_path.is_empty() {
            continue;
        }
        // GNU：若该 URL 被下载（或将会被下载）则指向本地；否则指向绝对 URL。
        let replacement = if will_download.contains(&l.resolved) {
            if cli.effective_convert_file_only() {
                convert_basename_only(&l.raw, &local_path)
            } else {
                local_path
            }
        } else {
            l.resolved
        };
        // 极简替换：只替换形如 "raw" 或 'raw' 的属性值。
        // 这样能在测试页稳定对齐 GNU 的 -k 行为。
        out = out.replace(&format!("\"{}\"", l.raw), &format!("\"{}\"", replacement));
        out = out.replace(&format!("'{}'", l.raw), &format!("'{}'", replacement));
    }
    out
}

fn ensure_parent_dir(path: &str) -> Result<(), ExitCode> {
    let p = Path::new(path);
    let Some(parent) = p.parent() else {
        return Ok(());
    };
    if parent.as_os_str().is_empty() {
        return Ok(());
    }
    std::fs::create_dir_all(parent).map_err(|_| ExitCode::IoFail)?;
    Ok(())
}

fn is_html_content_type(headers: &reqwest::header::HeaderMap) -> bool {
    let Some(v) = headers.get(reqwest::header::CONTENT_TYPE) else {
        return false;
    };
    let Ok(s) = v.to_str() else {
        return false;
    };
    let low = s.to_ascii_lowercase();
    low.starts_with("text/html") || low.starts_with("application/xhtml+xml")
}

fn schemes_are_similar(a: &str, b: &str) -> bool {
    if a == b {
        return true;
    }
    let a_http = a == "http" || a == "https";
    let b_http = b == "http" || b == "https";
    a_http && b_http
}

fn should_follow(cli: &Cli, child: &Url, parent: &Url, item: &QueueItem) -> bool {
    if cli.https_only && child.scheme() != "https" {
        return false;
    }
    if !cli.span_hosts {
        // GNU recur.c: 仅当 schemes 相似时做“同 host”约束；并且以“父页面 host”为准（不是 root）。
        if schemes_are_similar(parent.scheme(), child.scheme()) {
            let Some(ph) = parent.host_str() else {
                return false;
            };
            let Some(ch) = child.host_str() else {
                return false;
            };
            if ph.to_ascii_lowercase() != ch.to_ascii_lowercase() {
                return false;
            }
        }
    }
    if cli.effective_no_parent() {
        // 对齐 GNU `recur.c`：
        // opt.no_parent 生效条件：
        // - schemes 相似（HTTP/HTTPS 视为相似）
        // - host 相同（大小写不敏感）
        // - 且 (scheme 变了) 或 (port 相同)
        // 满足以上条件时，要求 child.path() 仍位于 root.no_parent_prefix 之下。
        let same_host = child
            .host_str()
            .is_some_and(|h| h.to_ascii_lowercase() == item.root.host_low);
        let scheme_similar = schemes_are_similar(child.scheme(), &item.root.scheme);
        let same_port = child
            .port_or_known_default()
            .is_some_and(|p| p == item.root.port);
        let scheme_changed = child.scheme() != item.root.scheme;
        if same_host && scheme_similar && (scheme_changed || same_port) {
            if !child.path().starts_with(&item.root.no_parent_prefix) {
                return false;
            }
        }
    }
    true
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum RedirectRejectReason {
    Success,
    NotHttps,
    NonHttp,
    Absolute,
    Domain,
    Parent,
    List,
    Regex,
    Rules,
    SpannedHost,
    Robots,
}

async fn redirect_target_reason(
    cli: &Cli,
    policy: &RecurPolicy,
    robots: &mut RobotsCache,
    parent: &Url,
    item: &QueueItem,
    effective_url: &Url,
    max_depth: Option<u32>,
) -> RedirectRejectReason {
    // Mirrors GNU recur.c:descend_redirect() -> download_child(), for deciding whether
    // to descend into children after a redirect.
    if cli.https_only && effective_url.scheme() != "https" {
        return RedirectRejectReason::NotHttps;
    }
    let scheme = effective_url.scheme();
    let scheme_like_http = scheme == "http" || scheme == "https";
    if !scheme_like_http {
        return RedirectRejectReason::NonHttp;
    }
    if cli.relative_only {
        // Redirect targets are considered absolute for -L/--relative.
        return RedirectRejectReason::Absolute;
    }
    if !policy.accept_domain(effective_url) {
        return RedirectRejectReason::Domain;
    }
    if cli.effective_no_parent() {
        let same_host = effective_url
            .host_str()
            .is_some_and(|h| h.to_ascii_lowercase() == item.root.host_low);
        let scheme_similar = schemes_are_similar(effective_url.scheme(), &item.root.scheme);
        let same_port = effective_url
            .port_or_known_default()
            .is_some_and(|p| p == item.root.port);
        let scheme_changed = effective_url.scheme() != item.root.scheme;
        if same_host && scheme_similar && (scheme_changed || same_port) {
            if !effective_url
                .path()
                .starts_with(&item.root.no_parent_prefix)
            {
                return RedirectRejectReason::Parent;
            }
        }
    }
    if !cli.span_hosts && schemes_are_similar(effective_url.scheme(), parent.scheme()) {
        let Some(ph) = parent.host_str() else {
            return RedirectRejectReason::SpannedHost;
        };
        let Some(ch) = effective_url.host_str() else {
            return RedirectRejectReason::SpannedHost;
        };
        if ph.to_ascii_lowercase() != ch.to_ascii_lowercase() {
            return RedirectRejectReason::SpannedHost;
        }
    }
    if cli.effective_use_robots() && scheme_like_http {
        match robots.allowed(cli, effective_url).await {
            Ok(true) => {}
            Ok(false) => return RedirectRejectReason::Robots,
            Err(_) => {} // best-effort
        }
    }
    if !policy.accdir(effective_url) {
        return RedirectRejectReason::List;
    }
    if !policy.accept_url_regex(effective_url.as_str()) {
        return RedirectRejectReason::Regex;
    }
    if policy.should_apply_rules_to_child(
        effective_url,
        item.depth,
        max_depth,
        cli.effective_page_requisites(),
    ) {
        let file = policy.file_for_filter(effective_url);
        if !file.is_empty() && !policy.acceptable_file(&file) {
            return RedirectRejectReason::Rules;
        }
    }
    RedirectRejectReason::Success
}

fn max_depth(cli: &Cli) -> Option<u32> {
    // GNU: -l 0 means infinite.
    let d = cli.effective_level().unwrap_or(5);
    if d == 0 { None } else { Some(d) }
}

pub async fn fetch_recursive(cli: &Cli, raw_urls: Vec<String>) -> i32 {
    let mut final_code: i32 = ExitCode::Success as i32;
    let mut q: VecDeque<QueueItem> = VecDeque::new();
    let mut seen: HashSet<String> = HashSet::new();
    let limit = max_depth(cli);
    let mut converted_backups: HashSet<String> = HashSet::new();
    let mut robots: RobotsCache = RobotsCache::default();

    let policy = RecurPolicy::from_cli(cli);

    for raw in raw_urls {
        let Ok(url) = crate::urls::parse_url_with_iri_policy(cli, &raw) else {
            final_code = match (final_code, ExitCode::ParseError as i32) {
                (0, x) => x,
                (cur, x) => cur.max(x),
            };
            continue;
        };
        // GNU：起始 URL 不走 download_child()，因此不在入队前应用 domains/dirs/regex 等过滤；
        // 这些过滤用于递归“子项”入队阶段。
        let host_low = url.host_str().unwrap_or("").to_ascii_lowercase();
        let port = url.port_or_known_default().unwrap_or(80);
        let root = RootPolicy {
            host_low,
            port,
            scheme: url.scheme().to_string(),
            no_parent_prefix: dir_prefix_for(&url),
        };
        let k = url.to_string();
        if seen.insert(k) {
            q.push_back(QueueItem {
                url,
                depth: 0,
                root,
                inline_only: !cli.effective_recursive() && cli.effective_page_requisites(),
            });
        }
    }

    while let Some(item) = q.pop_front() {
        // FTP 目录 URL：落盘为 ".listing"（对齐 GNU 的 listing 文件概念）。
        let index_out = local_path_for_recursive(cli, &item.url);
        let mut planned_out = index_out.clone();
        let is_ftp = item.url.scheme() == "ftp";
        let want_listing = is_ftp && (item.url.path().is_empty() || item.url.path().ends_with('/'));
        if want_listing {
            planned_out = ftp::ftp_listing_path_for_index(&index_out);
        }
        let ftp_index_arg = if want_listing {
            if index_out != "-" {
                let _ = ensure_parent_dir(&index_out);
            }
            Some(index_out.as_str())
        } else {
            None
        };
        let ftp_quick_retr = is_ftp && !want_listing && item.depth > 0;
        if let Err(ec) = ensure_parent_dir(&planned_out) {
            final_code = match (final_code, ec as i32) {
                (0, x) => x,
                (cur, x) => cur.max(x),
            };
            continue;
        }

        let (code, mut out, effective_url, body_opt, hdrs_opt, _retrieved) = if is_ftp {
            match ftp::fetch_ftp(
                cli,
                &item.url,
                &planned_out,
                want_listing,
                ftp_index_arg,
                ftp_quick_retr,
            )
            .await
            {
                Ok((code, listing_opt)) => {
                    let out_path = if want_listing {
                        index_out.clone()
                    } else {
                        planned_out.clone()
                    };
                    (
                        code,
                        out_path,
                        item.url.clone(),
                        listing_opt,
                        None,
                        code == crate::exit::ExitCode::Success as i32,
                    )
                }
                Err(ec) => {
                    final_code = match (final_code, ec as i32) {
                        (0, x) => x,
                        (cur, x) => cur.max(x),
                    };
                    continue;
                }
            }
        } else {
            match http::fetch_one_with_body(
                cli,
                item.url.as_str(),
                Some(&planned_out),
                true,
                None,
                &std::collections::HashSet::new(),
            )
            .await
            {
                Ok(v) => v,
                Err(ec) => {
                    final_code = match (final_code, ec as i32) {
                        (0, x) => x,
                        (cur, x) => cur.max(x),
                    };
                    continue;
                }
            }
        };
        // 对齐 GNU `descend_redirect`：是否将最终 URL 加入 blacklist/seen，
        // 取决于它是否通过过滤，或仅因 LIST/REGEX 被拒绝（这两类对重定向目标会被忽略）。
        let mut descend_allowed_by_redirect: bool = true;
        if effective_url != item.url {
            let r = redirect_target_reason(
                cli,
                &policy,
                &mut robots,
                &item.url,
                &item,
                &effective_url,
                limit,
            )
            .await;
            match r {
                RedirectRejectReason::Success
                | RedirectRejectReason::List
                | RedirectRejectReason::Regex => {
                    let _ = seen.insert(effective_url.to_string());
                }
                _ => {
                    descend_allowed_by_redirect = false;
                }
            }
        }
        // 递归场景：为了对齐 GNU `--trust-server-names`，
        // 若发生重定向，则以最终 URL 计算本地落盘路径（同一套递归目录规则）。
        if cli.trust_server_names && effective_url != item.url && out != "-" {
            let desired = local_path_for_recursive(cli, &effective_url);
            if desired != out {
                let _ = ensure_parent_dir(&desired);
                // best-effort: if rename fails, keep old name.
                if std::fs::rename(&out, &desired).is_ok() {
                    out = desired;
                }
            }
        }
        if code != 0 && final_code == 0 {
            final_code = code;
        }

        // FTP 目录列表：解析 listing 并最小递归闭环（入队子项）。
        if is_ftp && want_listing {
            let listing_bytes = body_opt.unwrap_or_default();
            let entries = ftp::parse_unix_listing(&listing_bytes);

            let Some(next_depth) = item.depth.checked_add(1) else {
                continue;
            };
            if let Some(maxd) = limit {
                if item.depth >= maxd {
                    continue;
                }
            }

            // GNU invalid entry: if there are duplicates later, reject earlier one.
            let mut last_idx: std::collections::HashMap<String, usize> =
                std::collections::HashMap::new();
            for (i, (name, _is_dir)) in entries.iter().enumerate() {
                last_idx.insert(name.clone(), i);
            }

            let mut accepted_any: bool = false;
            for (i, (name, is_dir)) in entries.into_iter().enumerate() {
                if last_idx.get(&name).copied().unwrap_or(i) != i {
                    continue;
                }
                // GNU has_insecure_name_p: leading '/' or contains '../'
                if name.starts_with('/') || name.contains("../") {
                    continue;
                }
                let child_url = if is_dir {
                    item.url.join(&(name.clone() + "/"))
                } else {
                    item.url.join(&name)
                };
                let Ok(child) = child_url else { continue };

                // Apply filters in GNU order:
                // 1) acceptable (-A/-R) for non-dirs
                if !is_dir {
                    if !policy.acceptable_file(&name) {
                        continue;
                    }
                }
                // 2) accept_url (regex) on full URL
                if !policy.accept_url_regex(child.as_str()) {
                    continue;
                }
                // 3) domain
                if !policy.accept_domain(&child) {
                    continue;
                }
                // 4) generic follow checks (span-hosts/no-parent/https-only etc.)
                if !should_follow(cli, &child, &item.url, &item) {
                    continue;
                }

                let key = child.to_string();
                if !seen.insert(key) {
                    continue;
                }
                q.push_back(QueueItem {
                    url: child,
                    depth: next_depth,
                    root: item.root.clone(),
                    inline_only: false,
                });
                accepted_any = true;
            }

            // GNU：当目录 listing 经过滤后为空，会继续尝试把“目录 URL”当作普通文件去取一次，
            // 这会触发 550 并以退出码 8 失败（见 TC177）。
            if !accepted_any {
                let out2 = local_path_for_recursive(cli, &item.url);
                if out2 != out {
                    if let Err(ec) =
                        ftp::fetch_ftp(cli, &item.url, &out2, false, None, false).await
                    {
                        final_code = match (final_code, ec as i32) {
                            (0, x) => x,
                            (cur, x) => cur.max(x),
                        };
                    }
                }
            }
            continue;
        }

        let Some(body) = body_opt else { continue };
        let Some(hdrs) = hdrs_opt else { continue };
        let ct_is_html = is_html_content_type(&hdrs);
        let ct_is_css = hdrs
            .get(reqwest::header::CONTENT_TYPE)
            .and_then(|v| v.to_str().ok())
            .is_some_and(|s| s.to_ascii_lowercase().starts_with("text/css"));
        if !ct_is_html && !ct_is_css {
            continue;
        }
        if !descend_allowed_by_redirect {
            // GNU：重定向目标若因 DOMAIN/SPANNEDHOST/ROBOTS/... 等拒绝，则不再抽链/入队其子链接。
            continue;
        }
        let text = String::from_utf8_lossy(&body);
        let base_url_for_parse = &effective_url;

        let Some(next_depth) = item.depth.checked_add(1) else {
            continue;
        };
        let infinite = limit.is_none();

        // emulate GNU dash_p_leaf_HTML (allow inline-only when exceeding depth; up to +2 levels)
        let mut dash_p_leaf_html = false;
        if !infinite {
            let l = limit.unwrap();
            if item.depth >= l {
                if cli.effective_page_requisites() && (item.depth == l || item.depth == l + 1) {
                    dash_p_leaf_html = true;
                } else {
                    // no further descent
                }
            }
        }

        let mut will_download: HashSet<String> = HashSet::new();
        let mut to_enqueue: Vec<(Url, u32, bool)> = Vec::new();
        fn stage_url(
            will_download: &mut HashSet<String>,
            to_enqueue: &mut Vec<(Url, u32, bool)>,
            cli: &Cli,
            item: &QueueItem,
            parent: &Url,
            child: Url,
            depth: u32,
            inline_only: bool,
        ) {
            if !should_follow(cli, &child, parent, item) {
                return;
            }
            will_download.insert(child.to_string());
            to_enqueue.push((child, depth, inline_only));
        }

        if ct_is_html {
            let links = html_urls::extract_recursive_links_from_html(
                &text,
                Some(base_url_for_parse),
                Path::new("<downloaded>"),
            );

            if cli.effective_use_robots() && html_urls::meta_robots_nofollow(&text) {
                // GNU：nofollow attribute found -> will not follow any links on this page
                continue;
            }

            for lnk in links {
                let is_inline = lnk.kind == html_urls::LinkKind::Inline;
                // without -r, never follow "Follow" links; only inline requisites
                if item.inline_only && !is_inline {
                    continue;
                }
                if cli.relative_only && !lnk.link_relative_p {
                    continue;
                }
                // depth limiting: follow links require depth<limit; inline links allowed in dash_p_leaf_html window
                if let Some(maxd) = limit {
                    if item.depth >= maxd && !is_inline {
                        continue;
                    }
                    if item.depth >= maxd
                        && is_inline
                        && !(cli.effective_page_requisites() && dash_p_leaf_html)
                    {
                        continue;
                    }
                }
                if let Ok(child) = Url::parse(&lnk.resolved) {
                    // robots.txt (best-effort): disallow => skip enqueue
                    if child.scheme() == "http" || child.scheme() == "https" {
                        match robots.allowed(cli, &child).await {
                            Ok(true) => {}
                            Ok(false) => continue,
                            Err(_) => {} // ignore robots fetch errors
                        }
                    }
                    if !policy.accept_domain(&child) {
                        continue;
                    }
                    if !policy.accdir(&child) {
                        continue;
                    }
                    if !policy.accept_url_regex(child.as_str()) {
                        continue;
                    }
                    if policy.should_apply_rules_to_child(
                        &child,
                        item.depth,
                        limit,
                        cli.effective_page_requisites(),
                    ) {
                        let file = policy.file_for_filter(&child);
                        if !file.is_empty() && !policy.acceptable_file(&file) {
                            continue;
                        }
                    }
                    // inline links are considered "do not further follow" when we are in leaf mode
                    let child_inline_only = item.inline_only || (dash_p_leaf_html && !is_inline);
                    stage_url(
                        &mut will_download,
                        &mut to_enqueue,
                        cli,
                        &item,
                        base_url_for_parse,
                        child,
                        next_depth,
                        child_inline_only,
                    );
                }
            }

            if !cli.delete_after
                && (cli.effective_convert_links() || cli.effective_convert_file_only())
            {
                let rewritten =
                    rewrite_html_links(cli, &text, base_url_for_parse, &item, &will_download, &out);
                if rewritten != text {
                    backups::backup_converted_once(cli, &mut converted_backups, &out);
                    let _ = std::fs::write(&out, rewritten.as_bytes());
                }
            }
        }

        if ct_is_css && cli.effective_page_requisites() {
            for u in css_urls::extract_urls_from_css(&text, base_url_for_parse) {
                if let Ok(child) = Url::parse(&u) {
                    stage_url(
                        &mut will_download,
                        &mut to_enqueue,
                        cli,
                        &item,
                        base_url_for_parse,
                        child,
                        next_depth,
                        true,
                    );
                }
            }
        }

        for (child, depth, inline_only) in to_enqueue {
            let key = child.to_string();
            if !seen.insert(key) {
                continue;
            }
            q.push_back(QueueItem {
                url: child,
                depth,
                root: item.root.clone(),
                inline_only,
            });
        }

        if cli.delete_after && out != "-" {
            if std::fs::remove_file(&out).is_ok() {
                if cli.effective_verbose() {
                    log::log_status_line(
                        cli,
                        &crate::i18n::tr_named("Removing {out}.", &[("out", &out)]),
                    );
                }
            }
        }
    }

    final_code
}
