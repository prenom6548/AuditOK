use crate::argv_skip;
use crate::i18n;
use clap::{ArgAction, CommandFactory, FromArgMatches, Parser, ValueEnum};
use std::path::PathBuf;

#[derive(Copy, Clone, Debug, ValueEnum)]
pub enum ReportSpeedType {
    Bits,
}

#[derive(Debug, Parser, Clone)]
#[command(name = "wget")]
#[command(
    about = "Kylin Wget",
    long_about = None,
    before_help = "Usage: wget [OPTION]... [URL]...\n",
    after_help = "Try `wget --help' for more options.\n",
    version = crate::version::LONG_VERSION,
    help_template = "\
{before-help}\
{about-with-newline}\n\
\n\
{all-args}{after-help}\
",
    next_help_heading = "Options",
    disable_help_flag = true,
    disable_version_flag = true,
)]
pub struct Cli {
    #[arg(short = 'h', long = "help", action = ArgAction::Help)]
    pub help: Option<bool>,

    #[arg(short = 'V', long = "version", action = ArgAction::Version)]
    pub version: Option<bool>,

    #[arg(short = 'q', long = "quiet", default_value_t = false)]
    pub quiet: bool,

    #[arg(short = 'v', long = "verbose", default_value_t = false)]
    pub verbose: bool,

    #[arg(long = "no-verbose", default_value_t = false)]
    pub no_verbose: bool,

    #[arg(short = 'n', value_name = "FLAGS", num_args = 1)]
    pub no_flags: Option<String>,

    #[arg(long = "show-progress", default_value_t = false)]
    pub show_progress: bool,

    #[arg(long = "progress", value_name = "TYPE")]
    pub progress: Option<String>,

    #[arg(short = 'b', long = "background", default_value_t = false)]
    pub background: bool,

    #[arg(short = 'O', long = "output-document", action = ArgAction::Append)]
    pub output_document: Vec<String>,

    #[arg(long = "config", value_name = "FILE")]
    pub config: Option<PathBuf>,

    #[arg(long = "no-config", default_value_t = false)]
    pub no_config: bool,

    #[arg(short = 'e', long = "execute", action = ArgAction::Append, value_name = "COMMAND")]
    pub execute: Vec<String>,

    #[arg(short = 'c', long = "continue", default_value_t = false)]
    pub resume: bool,

    #[arg(short = 'N', long = "timestamping", default_value_t = false)]
    pub timestamping: bool,

    #[arg(long = "use-server-timestamps", default_value_t = false)]
    pub use_server_timestamps: bool,

    #[arg(long = "no-use-server-timestamps", default_value_t = false)]
    pub no_use_server_timestamps: bool,

    #[arg(long = "start-pos")]
    pub start_pos: Option<u64>,

    #[arg(long = "max-redirect")]
    pub max_redirect: Option<u32>,

    #[arg(short = 't', long = "tries")]
    pub tries: Option<i32>,

    #[arg(long = "retry-on-http-error")]
    pub retry_on_http_error: Option<String>,

    #[arg(long = "retry-connrefused", default_value_t = false)]
    pub retry_connrefused: bool,

    #[arg(long = "retry-on-host-error", default_value_t = false)]
    pub retry_on_host_error: bool,

    #[arg(short = 'T', long = "timeout")]
    pub timeout_seconds: Option<f64>,

    #[arg(long = "connect-timeout")]
    pub connect_timeout_seconds: Option<f64>,

    #[arg(long = "read-timeout")]
    pub read_timeout_seconds: Option<f64>,

    #[arg(long = "limit-rate")]
    pub limit_rate: Option<String>,

    #[arg(short = 'F', long = "force-html", default_value_t = false)]
    pub force_html: bool,

    #[arg(long = "spider", default_value_t = false)]
    pub spider: bool,

    #[arg(short = 'i', long = "input-file")]
    pub input_file: Option<PathBuf>,

    #[arg(short = 'B', long = "base")]
    pub base_url: Option<String>,

    #[arg(short = 'o', long = "output-file")]
    pub log_file: Option<PathBuf>,

    #[arg(short = 'a', long = "append-output")]
    pub append_log_file: Option<PathBuf>,

    #[arg(long = "report-speed")]
    pub report_speed: Option<ReportSpeedType>,

    #[arg(short = 'E', long = "adjust-extension", default_value_t = false)]
    pub adjust_extension: bool,

    #[arg(long = "header")]
    pub header: Vec<String>,

    #[arg(long = "trust-server-names", default_value_t = false)]
    pub trust_server_names: bool,

    #[arg(short = 'U', long = "user-agent")]
    pub user_agent: Option<String>,

    #[arg(long = "referer")]
    pub referer: Option<String>,

    #[arg(long = "save-headers", default_value_t = false)]
    pub save_headers: bool,

    #[arg(long = "method")]
    pub method: Option<String>,

    #[arg(long = "body-data")]
    pub body_data: Option<String>,

    #[arg(long = "body-file")]
    pub body_file: Option<PathBuf>,

    #[arg(long = "post-data")]
    pub post_data: Option<String>,

    #[arg(long = "post-file")]
    pub post_file: Option<PathBuf>,

    #[arg(short = 'r', long = "recursive", default_value_t = false)]
    pub recursive: bool,

    #[arg(long = "delete-after", default_value_t = false)]
    pub delete_after: bool,

    #[arg(short = 'm', long = "mirror", default_value_t = false)]
    pub mirror: bool,

    #[arg(short = 'k', long = "convert-links", default_value_t = false)]
    pub convert_links: bool,

    #[arg(long = "convert-file-only", default_value_t = false)]
    pub convert_file_only: bool,

    #[arg(short = 'K', long = "backup-converted", default_value_t = false)]
    pub backup_converted: bool,

    #[arg(long = "backups")]
    pub backups: Option<u32>,

    #[arg(short = 'p', long = "page-requisites", default_value_t = false)]
    pub page_requisites: bool,

    #[arg(short = 'l', long = "level", value_name = "NUMBER")]
    pub level: Option<u32>,

    #[arg(long = "cut-dirs")]
    pub cut_dirs: Option<u32>,

    #[arg(long = "no-host-directories", default_value_t = false)]
    pub no_host_directories: bool,

    #[arg(long = "host-directories", default_value_t = false)]
    pub host_directories: bool,

    #[arg(short = 'P', long = "directory-prefix")]
    pub directory_prefix: Option<String>,

    #[arg(long = "dont-remove-listing", default_value_t = false)]
    pub dont_remove_listing: bool,

    #[arg(long = "remove-listing", default_value_t = false)]
    pub remove_listing: bool,

    #[arg(long = "no-parent", default_value_t = false)]
    pub no_parent: bool,

    #[arg(long = "no-directories", default_value_t = false)]
    pub no_directories: bool,

    #[arg(short = 'x', long = "force-directories", default_value_t = false)]
    pub force_directories: bool,

    #[arg(long = "protocol-directories", default_value_t = false)]
    pub protocol_directories: bool,

    #[arg(long = "default-page", value_name = "NAME", action = ArgAction::Append)]
    pub default_page: Vec<String>,

    #[arg(short = 'A', long = "accept", value_name = "LIST", action = ArgAction::Append)]
    pub accept: Vec<String>,

    #[arg(short = 'R', long = "reject", value_name = "LIST", action = ArgAction::Append)]
    pub reject: Vec<String>,

    #[arg(short = 'D', long = "domains")]
    pub domains: Option<String>,

    #[arg(long = "exclude-domains")]
    pub exclude_domains: Option<String>,

    #[arg(short = 'H', long = "span-hosts", default_value_t = false)]
    pub span_hosts: bool,

    #[arg(short = 'L', long = "relative", default_value_t = false)]
    pub relative_only: bool,

    #[arg(short = 'I', long = "include-directories")]
    pub include_directories: Option<String>,

    #[arg(short = 'X', long = "exclude-directories")]
    pub exclude_directories: Option<String>,

    #[arg(long = "accept-regex")]
    pub accept_regex: Option<String>,

    #[arg(long = "reject-regex")]
    pub reject_regex: Option<String>,

    #[arg(long = "no-proxy", default_value_t = false)]
    pub disable_proxy: bool,

    #[arg(long = "proxy")]
    pub proxy_url: Option<String>,

    #[arg(long = "proxy-user")]
    pub proxy_user: Option<String>,

    #[arg(long = "proxy-password", alias = "proxy-passwd")]
    pub proxy_password: Option<String>,

    #[arg(long = "http-user")]
    pub http_user: Option<String>,

    #[arg(long = "http-password")]
    pub http_password: Option<String>,

    #[arg(long = "user")]
    pub user: Option<String>,

    #[arg(long = "password")]
    pub password: Option<String>,

    #[arg(long = "auth-no-challenge", default_value_t = false)]
    pub auth_no_challenge: bool,

    #[arg(long = "https-only", default_value_t = false)]
    pub https_only: bool,

    #[arg(long = "content-disposition", default_value_t = false)]
    pub content_disposition: bool,

    #[arg(long = "content-on-error", default_value_t = false)]
    pub content_on_error: bool,

    #[arg(long = "no-clobber", default_value_t = false)]
    pub no_clobber: bool,

    #[arg(long = "clobber", num_args = 0..=1, default_missing_value = "on", value_name = "WHEN")]
    pub clobber: Option<String>,

    #[arg(short = 'S', long = "server-response", default_value_t = false)]
    pub server_response: bool,

    #[arg(long = "robots", num_args = 0..=1, default_missing_value = "on", value_name = "WHEN")]
    pub robots: Option<String>,

    #[arg(long = "no-check-certificate", default_value_t = false)]
    pub no_check_certificate: bool,

    #[arg(
        long = "check-certificate",
        num_args = 0..=1,
        default_missing_value = "on",
        value_name = "WHEN"
    )]
    pub check_certificate: Option<String>,

    #[arg(long = "ca-certificate", value_name = "FILE")]
    pub ca_certificate: Option<PathBuf>,

    #[arg(long = "ca-directory", value_name = "DIR")]
    pub ca_directory: Option<PathBuf>,

    #[arg(short = '4', long = "inet4-only", default_value_t = false)]
    pub inet4_only: bool,

    #[arg(short = '6', long = "inet6-only", default_value_t = false)]
    pub inet6_only: bool,

    #[arg(long = "no-cookies", default_value_t = false)]
    pub no_cookies: bool,

    #[arg(long = "load-cookies", value_name = "FILE")]
    pub load_cookies: Option<PathBuf>,

    #[arg(long = "save-cookies", value_name = "FILE")]
    pub save_cookies: Option<PathBuf>,

    #[arg(long = "keep-session-cookies", default_value_t = false)]
    pub keep_session_cookies: bool,

    #[arg(long = "hsts", default_value_t = true)]
    pub hsts: bool,

    #[arg(long = "no-hsts", default_value_t = false)]
    pub no_hsts: bool,

    #[arg(long = "hsts-file", value_name = "FILE")]
    pub hsts_file: Option<PathBuf>,

    #[arg(long = "ftp-user", value_name = "USER")]
    pub ftp_user: Option<String>,

    #[arg(long = "ftp-password", value_name = "PASS")]
    pub ftp_password: Option<String>,

    #[arg(long = "netrc", default_value_t = true)]
    pub netrc: bool,

    #[arg(long = "no-netrc", default_value_t = false)]
    pub no_netrc: bool,

    #[arg(long = "iri", default_value_t = false)]
    pub iri: bool,

    #[arg(long = "no-iri", default_value_t = false)]
    pub no_iri: bool,

    #[arg(long = "restrict-file-names", value_name = "OS")]
    pub restrict_file_names: Option<String>,

    #[arg(value_name = "URL")]
    pub urls: Vec<String>,
}

impl Cli {
    /// 解析命令行：按 `LANG` 等环境变量为 `-h`/`--help` 选择中英文。
    pub fn parse_from<I, T>(itr: I) -> Result<Self, clap::Error>
    where
        I: IntoIterator<Item = T>,
        T: Into<std::ffi::OsString> + Clone,
    {
        let cmd = crate::cli_help::apply_locale(Self::command());
        let matches = cmd.try_get_matches_from(itr)?;
        Ok(Self::from_arg_matches(&matches)?)
    }

    pub fn effective_output_document(&self) -> Option<&str> {
        self.output_document.last().map(|s| s.as_str())
    }

    #[inline]
    pub fn effective_default_page(&self) -> &str {
        self.default_page
            .last()
            .map(|s| s.as_str())
            .unwrap_or("index.html")
    }
    #[inline]
    pub(crate) fn legacy_no_verbose_v(&self) -> bool {
        self.no_flags.as_ref().is_some_and(|s| s.contains('v'))
    }

    #[inline]
    pub(crate) fn legacy_no_parent_p(&self) -> bool {
        self.no_flags.as_ref().is_some_and(|s| s.contains('p'))
    }

    #[inline]
    pub(crate) fn legacy_no_directories_d(&self) -> bool {
        self.no_flags.as_ref().is_some_and(|s| s.contains('d'))
    }

    #[inline]
    pub fn effective_no_parent(&self) -> bool {
        self.no_parent || self.legacy_no_parent_p()
    }

    #[inline]
    pub fn effective_no_directories(&self) -> bool {
        // 仅表示“用户显式要求不建目录树”；是否真的落到 no-directories
        // 还要考虑 `-x/--force-directories` 的 last-wins，见 dirstruct::effective_dirstruct。
        self.no_directories || self.legacy_no_directories_d()
    }

    #[inline]
    pub fn effective_recursive(&self) -> bool {
        self.recursive || self.mirror
    }

    #[inline]
    pub fn effective_timestamping(&self) -> bool {
        self.timestamping || self.mirror
    }

    #[inline]
    pub fn effective_use_server_timestamps(&self) -> bool {
        // GNU 默认开启 useservertimestamps；这里用显式 no-* 关闭。
        if self.no_use_server_timestamps {
            return false;
        }
        if self.use_server_timestamps {
            return true;
        }
        true
    }

    #[inline]
    pub fn effective_level(&self) -> Option<u32> {
        if self.mirror {
            return Some(0);
        }
        self.level
    }

    #[inline]
    pub fn effective_dont_remove_listing(&self) -> bool {
        // GNU: --mirror implies --dont-remove-listing (i.e. keep FTP listings).
        // 在 HTTP-only 测试中它没有可见效果，但为了与 GNU 语义对齐仍需保留开关。
        if self.mirror {
            return true;
        }
        if self.remove_listing {
            return false;
        }
        self.dont_remove_listing
    }

    #[inline]
    pub fn effective_convert_links(&self) -> bool {
        self.convert_links
    }

    #[inline]
    pub fn effective_convert_file_only(&self) -> bool {
        self.convert_file_only
    }

    #[inline]
    pub fn effective_page_requisites(&self) -> bool {
        self.page_requisites
    }

    /// 对齐 GNU wget `main.c`：默认 `verbose = !quiet`；`-v` / `--no-verbose` / `-nv` 显式覆盖。
    #[inline]
    pub fn effective_verbose(&self) -> bool {
        if let Some(v) = scan_ordered_verbose_state() {
            return v;
        }
        // GNU: if opt.verbose == -1, opt.verbose = !opt.quiet
        !self.quiet
    }

    /// GNU：`show_progress == -1` 时在 C 中为真；即默认与 `effective_verbose` 一致，除非显式关闭。
    /// `--show-progress` 可强制打开（仍服从 `-q`）。
    #[inline]
    pub fn effective_show_progress(&self) -> bool {
        if self.quiet {
            return false;
        }
        if self.show_progress {
            return true;
        }
        self.effective_verbose()
    }

    #[inline]
    pub(crate) fn accept_list(&self) -> Vec<String> {
        split_csv_many(&self.accept)
    }

    #[inline]
    pub(crate) fn reject_list(&self) -> Vec<String> {
        split_csv_many(&self.reject)
    }

    #[inline]
    pub(crate) fn domains_list(&self) -> Vec<String> {
        split_csv(self.domains.as_deref())
    }

    #[inline]
    pub(crate) fn exclude_domains_list(&self) -> Vec<String> {
        split_csv(self.exclude_domains.as_deref())
    }

    #[inline]
    pub(crate) fn include_dirs_list(&self) -> Vec<String> {
        split_csv(self.include_directories.as_deref())
    }

    #[inline]
    pub(crate) fn exclude_dirs_list(&self) -> Vec<String> {
        split_csv(self.exclude_directories.as_deref())
    }

    #[inline]
    pub fn effective_use_robots(&self) -> bool {
        // GNU 默认开启；支持 `--robots=off` 关闭
        match self.robots.as_deref() {
            None => true,
            Some(v) => {
                let low = v.to_ascii_lowercase();
                !(low == "0" || low == "off" || low == "no" || low == "false")
            }
        }
    }

    #[inline]
    pub fn effective_netrc(&self) -> bool {
        // 对齐 GNU：默认开启；`--netrc` / `--no-netrc` 按 argv 顺序 last-wins。
        let args: Vec<String> = std::env::args().collect();
        let mut last: Option<bool> = None;
        let mut i = 1usize;
        while i < args.len() {
            let a = &args[i];
            if a == "--" {
                break;
            }
            if a == "--netrc" {
                last = Some(true);
                i += 1;
                continue;
            }
            if a == "--no-netrc" {
                last = Some(false);
                i += 1;
                continue;
            }
            i += argv_skip::skip_option_slots(&args, i);
        }
        if let Some(v) = last {
            return v;
        }
        if self.no_netrc {
            return false;
        }
        self.netrc
    }

    #[inline]
    pub fn effective_enable_iri(&self) -> bool {
        // GNU: default depends on ENABLE_IRI at build time; we ship with IRI enabled by default.
        // Also allow explicit --iri / --no-iri with last-wins.
        let args: Vec<String> = std::env::args().collect();
        let mut last: Option<bool> = None;
        let mut i = 1usize;
        while i < args.len() {
            let a = &args[i];
            if a == "--" {
                break;
            }
            if a == "--iri" {
                last = Some(true);
                i += 1;
                continue;
            }
            if a == "--no-iri" {
                last = Some(false);
                i += 1;
                continue;
            }
            i += argv_skip::skip_option_slots(&args, i);
        }
        if let Some(v) = last {
            return v;
        }
        if self.iri {
            return true;
        }
        if self.no_iri {
            return false;
        }
        true
    }
}

/// 扫描 argv，返回**最后一次**显式影响 verbose 的状态；若无此类参数则 `None`。
fn scan_ordered_verbose_state() -> Option<bool> {
    let args: Vec<String> = std::env::args().collect();
    let mut last: Option<bool> = None;
    let mut i = 1usize;
    while i < args.len() {
        let a = &args[i];
        if a == "--" {
            break;
        }

        if a == "-v" || a == "--verbose" {
            last = Some(true);
            i += 1;
            continue;
        }
        if a == "--no-verbose" {
            last = Some(false);
            i += 1;
            continue;
        }

        // GNU：`wget -nv` / `-nvc` 等短选项拼接（`-n` 后仅允许已知字母）
        if a.len() >= 3 && a.starts_with("-n") && !a.starts_with("--") {
            let rest = &a[2..];
            if !rest.is_empty()
                && rest
                    .chars()
                    .all(|ch| matches!(ch, 'v' | 'H' | 'd' | 'c' | 'p'))
            {
                if rest.contains('v') {
                    last = Some(false);
                }
                i += 1;
                continue;
            }
        }

        // GNU：`wget -n v`（两个参数）
        if a == "-n" && i + 1 < args.len() {
            let flags = &args[i + 1];
            if flags.starts_with("http://") || flags.starts_with("https://") {
                i += 1;
                continue;
            }
            if flags == "-" || !flags.starts_with('-') {
                if flags.contains('v') {
                    last = Some(false);
                }
                i += 2;
                continue;
            }
            i += 1;
            continue;
        }

        i += argv_skip::skip_option_slots(&args, i);
    }
    last
}

fn split_csv(s: Option<&str>) -> Vec<String> {
    let Some(s) = s else {
        return Vec::new();
    };
    split_csv_one(s)
}

fn split_csv_one(s: &str) -> Vec<String> {
    s.split(',')
        .map(|x| x.trim())
        .filter(|x| !x.is_empty())
        .map(|x| x.to_string())
        .collect()
}

fn split_csv_many(v: &[String]) -> Vec<String> {
    let mut out: Vec<String> = Vec::new();
    for s in v {
        out.extend(split_csv_one(s));
    }
    out
}

/// CLI 校验错误（对齐 GNU wget 的 stderr 格式）。
pub enum ValidateError {
    /// 普通错误，由 `report_validate_error` 加 `wget:` 前缀输出。
    Msg(String),
    /// 未指定 URL：输出 GNU 风格的多行用法提示。
    MissingUrl,
}

impl ValidateError {
    fn msg(s: impl Into<String>) -> Self {
        Self::Msg(s.into())
    }
}

/// 无 URL 时的用法提示（译文见 `po/*.po`）。
pub fn print_missing_url_usage() {
    use crate::i18n;
    eprintln!("{}", i18n::gettext("wget: missing URL"));
    eprintln!("{}", i18n::gettext("Usage: wget [OPTION]... [URL]..."));
    eprintln!();
    eprintln!(
        "{}",
        i18n::gettext("Try `wget --help' for more options.")
    );
}

pub fn report_validate_error(err: ValidateError) {
    match err {
        ValidateError::MissingUrl => print_missing_url_usage(),
        ValidateError::Msg(m) => {
            if m.starts_with("wget:") || m.starts_with("wget：") {
                eprintln!("{m}");
            } else {
                eprintln!("wget: {m}");
            }
        }
    }
}

pub fn validate(cli: &Cli) -> Result<(), ValidateError> {
    if let Some(v) = cli.restrict_file_names.as_deref() {
        crate::restrict_names::parse_restrict_file_names(v).map_err(ValidateError::msg)?;
    }
    if cli.effective_output_document().is_some() && cli.effective_recursive() {
        return Err(ValidateError::msg(i18n::gettext(
            "Cannot specify both -r and -O. See the manual for details.",
        )));
    }
    if cli.effective_output_document() == Some("-") && cli.effective_timestamping() {
        return Err(ValidateError::msg(i18n::gettext(
            "Timestamping does not work when saving to stdout (-O -).",
        )));
    }
    if cli.use_server_timestamps && cli.no_use_server_timestamps {
        return Err(ValidateError::msg(i18n::gettext(
            "Cannot specify both --use-server-timestamps and --no-use-server-timestamps.",
        )));
    }

    if cli.effective_output_document() == Some("-") && cli.effective_convert_links() {
        return Err(ValidateError::msg(i18n::gettext(
            "Link conversion does not work when saving to stdout (-O -).",
        )));
    }
    if cli.effective_output_document() == Some("-") && cli.convert_file_only {
        return Err(ValidateError::msg(i18n::gettext(
            "Link conversion does not work when saving to stdout (-O -).",
        )));
    }

    // `--trust-server-names` 仅影响“按 URL 推导默认输出文件名”的场景；
    // 若用户显式指定 `-O/--output-document` 或启用 `--content-disposition`，则该开关不会改变最终文件名。

    if cli.dont_remove_listing && cli.remove_listing {
        return Err(ValidateError::msg(i18n::gettext(
            "Cannot specify both --dont-remove-listing and --remove-listing.",
        )));
    }

    if cli.log_file.is_some() && cli.append_log_file.is_some() {
        return Err(ValidateError::msg(i18n::gettext(
            "You cannot specify both --output-file (-o) and --append-output (-a).",
        )));
    }

    if cli.quiet && cli.effective_verbose() {
        return Err(ValidateError::msg(i18n::gettext(
            "Can't be verbose and quiet at the same time.",
        )));
    }

    if let Some(ref p) = cli.progress {
        crate::progress::validate_progress_type(p).map_err(ValidateError::msg)?;
    }

    if cli.accept_regex.is_some() {
        if let Some(ref r) = cli.accept_regex {
            if regex::Regex::new(r).is_err() {
                return Err(ValidateError::msg(i18n::tr_named(
                    "wget: invalid regular expression '{regex}'",
                    &[("regex", r)],
                )));
            }
        }
    }
    if cli.reject_regex.is_some() {
        if let Some(ref r) = cli.reject_regex {
            if regex::Regex::new(r).is_err() {
                return Err(ValidateError::msg(i18n::tr_named(
                    "wget: invalid regular expression '{regex}'",
                    &[("regex", r)],
                )));
            }
        }
    }

    if let Some(ref s) = cli.no_flags {
        for c in s.chars() {
            if !matches!(c, 'v' | 'c' | 'p' | 'd' | 'H') {
                return Err(ValidateError::msg(i18n::tr_named(
                    "illegal option -- `-n{flag}`",
                    &[("flag", &c.to_string())],
                )));
            }
        }
    }

    // `--cut-dirs` 仅在“递归 + 目录树落盘”场景有意义；在 `--no-directories` 下应被忽略。
    // clap 已保证其为非负整数，这里不额外报错。

    // 递归层级：GNU `-l 0` 表示无限；本实现保持该语义。
    // `-l` 在非递归模式下允许（GNU 也允许，但通常无效）。

    if cli.post_data.is_some() && cli.post_file.is_some() {
        return Err(ValidateError::msg(i18n::gettext(
            "You cannot specify both --post-data and --post-file.",
        )));
    }
    if (cli.post_data.is_some() || cli.post_file.is_some()) && cli.method.is_some() {
        return Err(ValidateError::msg(i18n::gettext(
            "You cannot use --post-data or --post-file along with --method. --method expects data through --body-data and --body-file options",
        )));
    }
    if cli.body_data.is_some() || cli.body_file.is_some() {
        if cli.method.is_none() {
            return Err(ValidateError::msg(i18n::gettext(
                "You must specify a method through --method=HTTPMethod to use with --body-data or --body-file.",
            )));
        }
        if cli.body_data.is_some() && cli.body_file.is_some() {
            return Err(ValidateError::msg(i18n::gettext(
                "You cannot specify both --body-data and --body-file.",
            )));
        }
    }

    if cli.input_file.is_none() && cli.urls.is_empty() {
        return Err(ValidateError::MissingUrl);
    }

    if cli.inet4_only && cli.inet6_only {
        return Err(ValidateError::msg(i18n::gettext(
            "Cannot specify both --inet4-only and --inet6-only.",
        )));
    }

    if let Some(ref p) = cli.load_cookies {
        let ps = p.display().to_string();
        if !p.exists() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: cannot stat '{path}': No such file or directory",
                &[("path", &ps)],
            )));
        }
        if !p.is_file() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: '{path}' is not a regular file",
                &[("path", &ps)],
            )));
        }
    }
    // save-cookies：允许文件不存在（会创建）；但若给的是目录则报错。
    if let Some(ref p) = cli.save_cookies {
        if p.exists() && !p.is_file() {
            let ps = p.display().to_string();
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: '{path}' is not a regular file",
                &[("path", &ps)],
            )));
        }
    }

    if cli.no_hsts && cli.hsts {
        return Err(ValidateError::msg(i18n::gettext(
            "Cannot specify both --hsts and --no-hsts.",
        )));
    }
    if let Some(ref p) = cli.hsts_file {
        if p.exists() && !p.is_file() {
            let ps = p.display().to_string();
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: '{path}' is not a regular file",
                &[("path", &ps)],
            )));
        }
    }

    if let Some(ref p) = cli.ca_certificate {
        let ps = p.display().to_string();
        if !p.exists() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: cannot stat '{path}': No such file or directory",
                &[("path", &ps)],
            )));
        }
        if !p.is_file() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: '{path}' is not a regular file",
                &[("path", &ps)],
            )));
        }
    }
    if let Some(ref p) = cli.ca_directory {
        let ps = p.display().to_string();
        if !p.exists() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: cannot stat '{path}': No such file or directory",
                &[("path", &ps)],
            )));
        }
        if !p.is_dir() {
            return Err(ValidateError::msg(i18n::tr_named(
                "wget: '{path}' is not a directory",
                &[("path", &ps)],
            )));
        }
    }

    Ok(())
}
