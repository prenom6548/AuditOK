//! 命令行帮助：源码 msgid 为英文，译文在 `po/*.po` 维护。

use clap::Command;
use crate::i18n;

fn help_for_id(id: &str) -> &'static str {
    match id {
        "accept" => i18n::gettext("Comma-separated accepted extensions; append when repeated"),
        "accept_regex" => i18n::gettext("Regex of accepted URLs"),
        "adjust_extension" => i18n::gettext("Save HTML/CSS with proper extensions"),
        "append_output" => i18n::gettext("Append messages to FILE"),
        "auth_no_challenge" => i18n::gettext("Send Basic HTTP authentication without waiting for challenge"),
        "background" => i18n::gettext("Go to background after startup"),
        "backup_converted" => i18n::gettext("Back up as X.orig before converting X"),
        "backups" => i18n::gettext("Rotate up to N backup files before writing"),
        "base_url" => i18n::gettext("Resolve -i/-F HTML links relative to URL"),
        "body_data" => i18n::gettext("Request body string (requires --method; not with --post-*)"),
        "body_file" => i18n::gettext("Request body from file (requires --method)"),
        "ca_certificate" => i18n::gettext("Trusted CA certificates (PEM file)"),
        "ca_directory" => i18n::gettext("Directory of trusted CA PEM files"),
        "certificate" => i18n::gettext("Check server TLS certificate (default on; =off or =quiet)"),
        "clobber" => i18n::gettext("Allow overwriting files (last wins vs --no-clobber; =off to disable)"),
        "config" => i18n::gettext("Use CONFIG file"),
        "connect_timeout_seconds" => i18n::gettext("Set connect timeout to SECONDS"),
        "content_disposition" => i18n::gettext("Use last path segment of redirect URL as local name"),
        "content_disposition_on" => i18n::gettext("Use Content-Disposition for local name (without -O and not -c)"),
        "content_on_error" => i18n::gettext("Write response body even on HTTP error status"),
        "convert_file_only" => i18n::gettext("Convert file part of URLs only"),
        "convert_links" => i18n::gettext("Convert links in downloaded HTML for local viewing"),
        "cut_dirs" => i18n::gettext("Ignore NUMBER remote directory components"),
        "default_page" => i18n::gettext("Default page name for directory URLs (default index.html); last wins"),
        "delete_after" => i18n::gettext("Delete local files after downloading"),
        "directory_prefix" => i18n::gettext("Save files under PREFIX/"),
        "domains" => i18n::gettext("Comma-separated accepted domains"),
        "exclude_directories" => i18n::gettext("Comma-separated excluded directories"),
        "exclude_domains" => i18n::gettext("Comma-separated rejected domains"),
        "execute" => i18n::gettext("Execute a .wgetrc-style command (last wins)"),
        "force_directories" => i18n::gettext("Force creation of directories"),
        "force_html" => i18n::gettext("Treat input file as HTML (with -i)"),
        "ftp_password" => i18n::gettext("Set FTP password"),
        "ftp_user" => i18n::gettext("Set FTP user"),
        "header" => i18n::gettext("Insert STRING among the request headers"),
        "help" => i18n::gettext("Print this help"),
        "host_directories" => i18n::gettext("Create host directories"),
        "hsts" => i18n::gettext("Enable HSTS (default on)"),
        "hsts_file" => i18n::gettext("Path to HSTS database"),
        "http_password" => i18n::gettext("Set HTTP password"),
        "http_user" => i18n::gettext("Set HTTP user to USER"),
        "https_only" => i18n::gettext("Only follow HTTPS links (mainly when recursing)"),
        "include_directories" => i18n::gettext("Comma-separated allowed directories"),
        "inet4_only" => i18n::gettext("Connect using IPv4 only"),
        "inet6_only" => i18n::gettext("Connect using IPv6 only"),
        "input_file" => i18n::gettext("Download URLs found in FILE"),
        "iri" => i18n::gettext("Enable IRI support"),
        "keep_session_cookies" => i18n::gettext("Load and save session cookies"),
        "level" => i18n::gettext("Maximum recursion depth (0 is infinite)"),
        "limit_rate" => i18n::gettext("Limit download rate (e.g. 50k, 2m)"),
        "load_cookies" => i18n::gettext("Load cookies from FILE before session"),
        "max_redirect" => i18n::gettext("Maximum redirects allowed per page"),
        "method" => i18n::gettext("HTTP method (required with --body-data/--body-file)"),
        "mirror" => i18n::gettext("Mirror website (implies -r -N -l 0)"),
        "netrc" => i18n::gettext("Use .netrc for credentials (default on)"),
        "no_check_certificate" => i18n::gettext("Do not check TLS certificate (HTTPS; testing/intranet only)"),
        "no_clobber" => i18n::gettext("Do not overwrite existing files"),
        "no_config" => i18n::gettext("Do not read any config file"),
        "no_cookies" => i18n::gettext("Disable cookies"),
        "no_directories" => i18n::gettext("Do not create directory hierarchy when recursing"),
        "no_flags" => i18n::gettext("Accept -nv style flags after -n (e.g. v turns off verbose)"),
        "no_host_directories" => i18n::gettext("Do not create host directories"),
        "no_hsts" => i18n::gettext("Disable HSTS"),
        "no_iri" => i18n::gettext("Disable IRI support"),
        "no_netrc" => i18n::gettext("Do not read credentials from .netrc"),
        "no_parent" => i18n::gettext("Do not ascend to parent directory when recursing"),
        "no_proxy" => i18n::gettext("Do not use any proxy (ignore http_proxy, etc.)"),
        "no_remove_listing" => i18n::gettext("Do not remove FTP .listing files"),
        "no_use_server_timestamps" => i18n::gettext("Do not set local timestamps from the server"),
        "no_verbose" => i18n::gettext("Turn off verbose output (same as -nv / -n v)"),
        "output_document" => i18n::gettext(
            "Write documents to FILE (- is stdout); if multiple -O are given, the last one is used",
        ),
        "output_file" => i18n::gettext("Log messages to FILE"),
        "page_requisites" => i18n::gettext("Download assets needed to display HTML pages"),
        "password" => i18n::gettext("Set password"),
        "post_data" => i18n::gettext("POST body string (not with --method)"),
        "post_file" => i18n::gettext("POST body from file (not with --method)"),
        "progress" => i18n::gettext("Progress indicator type (e.g. bar, dot, dot:mega)"),
        "protocol_directories" => i18n::gettext("Use protocol name in directory paths"),
        "proxy" => i18n::gettext("Use proxy URL (default scheme http:// if missing)"),
        "proxy_password" => i18n::gettext("Proxy password (with --proxy-user)"),
        "proxy_user" => i18n::gettext("Proxy authentication user"),
        "quiet" => i18n::gettext("Quiet mode (no output)"),
        "read_timeout_seconds" => i18n::gettext("Set read timeout to SECONDS"),
        "recursive" => i18n::gettext("Recursive download"),
        "referer" => i18n::gettext("Include Referer: URL header"),
        "reject" => i18n::gettext("Comma-separated rejected extensions; append when repeated"),
        "reject_regex" => i18n::gettext("Regex of rejected URLs"),
        "relative_only" => i18n::gettext("Follow relative links only"),
        "remove_listing" => i18n::gettext("Remove FTP .listing files"),
        "report_speed" => i18n::gettext("Report bandwidth as TYPE (e.g. bits)"),
        "restrict_file_names" => i18n::gettext("Restrict file name characters (unix/vms/windows, comma-separated flags)"),
        "resume" => i18n::gettext("Resume downloading a partially-downloaded file"),
        "retry_connrefused" => i18n::gettext("Retry even if connection is refused"),
        "retry_on_host_error" => i18n::gettext("Retry on host errors"),
        "retry_on_http_error" => i18n::gettext("Retry on listed HTTP errors (comma-separated, e.g. 503,404)"),
        "robots" => i18n::gettext("Obey robots.txt (default on; =off to disable)"),
        "save_cookies" => i18n::gettext("Save cookies to FILE after session"),
        "save_headers" => i18n::gettext("Save response headers in the output file"),
        "server_response" => i18n::gettext("Print server response headers to stderr"),
        "show_progress" => i18n::gettext("Force progress display (even with --no-verbose; still off when -q)"),
        "span_hosts" => i18n::gettext("Span hosts when recursing"),
        "spider" => i18n::gettext("Spider mode: check availability without saving body"),
        "start_pos" => i18n::gettext("Start at byte OFFSET (HTTP Range; disables --continue)"),
        "timeout_seconds" => i18n::gettext("Set all timeout values to SECONDS"),
        "timestamping" => i18n::gettext("Only download newer files than local copies"),
        "tries" => i18n::gettext("Set number of retries (0 means unbounded)"),
        "urls" => i18n::gettext("URLs to download"),
        "use_server_timestamps" => i18n::gettext("Set local file timestamps from the server"),
        "user" => i18n::gettext("Set user (fallback for HTTP and other protocols)"),
        "user_agent" => i18n::gettext("Identify as AGENT instead of default User-Agent"),
        "verbose" => i18n::gettext("Verbose output (default unless -q or --no-verbose)"),
        "version" => i18n::gettext("Print version and exit"),
        _ => "",
    }
}

/// 按当前 locale 为 clap Command 设置译文帮助。
pub fn apply_locale(cmd: Command) -> Command {
    let options_heading = i18n::gettext("Options");
    let args_heading = i18n::gettext("Arguments");
    cmd.before_help(i18n::gettext("Usage: wget [OPTION]... [URL]...\n"))
        .about(i18n::gettext(
            "Kylin Wget non-interactive network file retriever",
        ))
        .after_help(i18n::gettext("Try `wget --help' for more options.\n"))
        .next_help_heading(options_heading)
        .mut_args(move |arg| {
            let id = arg.get_id().to_string();
            if id.starts_with("ignore") {
                return arg;
            }
            let mut a = arg;
            let h = help_for_id(&id);
            if !h.is_empty() {
                a = a.help(h);
            }
            if id == "urls" {
                a.help_heading(args_heading)
            } else {
                a.help_heading(options_heading)
            }
        })
}
