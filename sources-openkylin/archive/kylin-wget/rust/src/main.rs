mod argv_skip;
mod auth;
mod background;
mod backups;
mod cert_check;
mod cli;
mod cli_help;
mod i18n;
mod locale;
mod client;
mod content_disp;
mod cookies;
mod css_urls;
mod dirstruct;
mod dns_family;
mod exit;
mod ftp;
mod headers;
mod hostdir;
mod hsts;
mod html_urls;
mod http;
mod idn;
mod log;
mod netrc;
mod noclobber;
mod output;
mod pathgen;
mod progress;
mod recur_policy;
mod recursive;
mod request;
mod restrict_names;
mod resume;
mod robots;
mod tls_peer;
mod tls_ca;
mod urls;
mod version;
mod wgetrc;

use cli::Cli;
use exit::ExitCode;

#[tokio::main]
async fn main() {
    i18n::textdomain("kylin-wget");
    // 对齐 GNU main.c：在解析其它参数前，先处理 --config/--no-config 并注入 wgetrc 默认值。
    let argv = std::env::args().collect::<Vec<_>>();
    let injected = wgetrc::inject_wgetrc_args(&argv).unwrap_or_else(|ec| {
        std::process::exit(ec as i32);
    });
    // 对齐 GNU：-e/--execute 在命令行解析阶段按出现顺序生效（last-wins）。
    let injected = wgetrc::inject_execute_args(&injected).unwrap_or_else(|ec| {
        std::process::exit(ec as i32);
    });
    let injected_for_child = injected.clone();
    let cli = Cli::parse_from(&injected).unwrap_or_else(|e| {
        e.print().ok();
        std::process::exit(ExitCode::GenericError as i32);
    });
    if let Err(err) = cli::validate(&cli) {
        cli::report_validate_error(err);
        // 对齐 GNU wget：互斥/缺 URL 等多为 WGET_EXIT_GENERIC_ERROR (1)
        std::process::exit(ExitCode::GenericError as i32);
    }

    // 对齐 GNU：-b/--background 在启动期进入后台；父进程提示 pid 并退出 0；子进程继续执行。
    // Rust 侧不在 tokio runtime 内 fork（不安全），改为“spawn 自身进程”保持可控行为一致。
    if let Err(ec) = background::maybe_spawn_background_and_exit(&cli, &injected_for_child) {
        eprintln!("{}", i18n::gettext("wget: failed to background"));
        std::process::exit(ec as i32);
    }

    // 初始化 -o/--output-file / -a/--append-output 的落盘日志
    if let Err(ec) = log::init_output_file(&cli) {
        eprintln!("{}", i18n::gettext("wget: failed to initialize output log"));
        std::process::exit(ec as i32);
    }

    // Cookie jar：加载（若指定）并在进程结束时保存（若指定）。
    // 与 GNU wget 一致：--no-cookies 直接禁用（即使传了 load/save 也不生效）。
    if let Err(ec) = cookies::init_cookie_jar(&cli) {
        eprintln!("{}", i18n::gettext("wget: failed to initialize cookies"));
        std::process::exit(ec as i32);
    }

    if let Err(ec) = hsts::init_hsts(&cli) {
        eprintln!("{}", i18n::gettext("wget: failed to initialize hsts"));
        std::process::exit(ec as i32);
    }

    let urls = match urls::expand_url_list(&cli) {
        Ok(v) => v,
        Err(e) => {
            if !e.is_empty() {
                eprintln!("wget: {e}");
            }
            std::process::exit(ExitCode::GenericError as i32);
        }
    };
    let url_outputs = urls::output_documents_for_all_urls(&cli, urls.len());

    if cli.effective_recursive() || cli.effective_page_requisites() {
        let code = recursive::fetch_recursive(&cli, urls).await;
        std::process::exit(code);
    }

    let url_count = urls.len();
    let mut existed_at_start: std::collections::HashSet<String> =
        std::collections::HashSet::new();
    for o in &url_outputs {
        if let Some(p) = o {
            if p != "-" && std::path::Path::new(p).exists() {
                existed_at_start.insert(p.clone());
            }
        }
    }
    let mut outcomes: Vec<(i32, bool)> = Vec::with_capacity(url_count);
    for (i, u) in urls.iter().enumerate() {
        let url_o = url_outputs[i].as_deref();
        match http::fetch_one(&cli, u, url_o, &existed_at_start).await {
            Ok(pair) => outcomes.push(pair),
            Err(ec) => outcomes.push((ec as i32, false)),
        }
    }
    let mut final_code = crate::exit::aggregate_batch_exit(&outcomes);

    // GNU：HTTP 4xx/5xx 与网络失败已打印 `ERROR` / `Giving up.`；
    // 单 URL `--no-clobber` 跳过已存在文件 → 1，但不打 `wget: failed`。
    if final_code != 0
        && !cli.quiet
        && final_code != exit::ExitCode::ServerError as i32
        && final_code != exit::ExitCode::ServerAuthFail as i32
        && final_code != exit::ExitCode::SslAuthFail as i32
        && final_code != exit::ExitCode::NetworkFail as i32
        && final_code != exit::ExitCode::GenericError as i32
    {
        eprintln!("{}", i18n::gettext("wget: failed"));
    }

    if let Err(ec) = cookies::save_cookies_at_exit(&cli) {
        // GNU wget：保存 cookie 失败也会导致非 0；这里尽量不覆盖更严重的退出码。
        if final_code == 0 {
            final_code = ec as i32;
        }
    }

    if let Err(ec) = hsts::save_hsts_at_exit(&cli) {
        if final_code == 0 {
            final_code = ec as i32;
        }
    }
    std::process::exit(final_code);
}
