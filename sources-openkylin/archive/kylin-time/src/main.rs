//! GNU time 的 Rust 重写版
//!
//! 用法: time [OPTIONS] COMMAND [ARG]...
//! 运行 COMMAND，然后打印系统资源使用情况。

mod execute;
mod resuse;
mod rusage_kb;
mod summarize;

use std::fs::File;
use std::io::stderr;

use execute::{compute_exit_status, run_command, EXIT_CANCELED};
use summarize::{summarize, DEFAULT_FORMAT, POSIX_FORMAT, verbose_format};

/// 程序选项
struct Options {
    append: bool,
    format: Option<String>,
    output_file: Option<String>,
    portability: bool,
    quiet: bool,
    verbose: bool,
}

/// 打印帮助信息
fn print_help() {
    println!("Usage: time [OPTIONS] COMMAND [ARG]...");
    println!("Run COMMAND, then print system resource usage.");
    println!();
    println!("  -a, --append              with -o FILE, append instead of overwriting");
    println!("  -f, --format=FORMAT       use the specified FORMAT instead of the default");
    println!("  -o, --output=FILE         write to FILE instead of STDERR");
    println!("  -p, --portability         print POSIX standard 1003.2 conformant string:");
    println!("                              real %e");
    println!("                              user %U");
    println!("                              sys %S");
    println!("  -q, --quiet               do not print information about abnormal program");
    println!("                            termination (non-zero exit codes or signals)");
    println!("  -v, --verbose             print all resource usage information instead of");
    println!("                            the default format");
    println!("  -h,  --help               display this help and exit");
    println!("  -V,  --version            output version information and exit");
    println!();
    println!("Commonly used format sequences for -f/--format:");
    println!("(see documentation for full list)");
    println!("  %%   a literal '%'");
    println!("  %C   command line and arguments");
    println!("  %c   involuntary context switches");
    println!("  %E   elapsed real time (wall clock) in [hour:]min:sec");
    println!("  %e   elapsed real time (wall clock) in seconds");
    println!("  %F   major page faults");
    println!("  %M   maximum resident set size in KB");
    println!("  %P   percent of CPU this job got");
    println!("  %R   minor page faults");
    println!("  %S   system (kernel) time in seconds");
    println!("  %U   user time in seconds");
    println!("  %w   voluntary context switches");
    println!("  %x   exit status of command");
    println!();
    println!("Default output format:");
    println!("{}", DEFAULT_FORMAT);
    println!();
    println!("NOTE: your shell may have its own version of time, which usually supersedes");
    println!("the version described here.  Please refer to your shell's documentation");
    println!("for details about the options it supports.");
}

/// 打印版本信息
fn print_version() {
    println!("kylin-time (Rust rewrite) 1.9.0");
    println!("Originally written by David Keppel, David MacKenzie, Assaf Gordon.");
    println!("Rewritten in Rust.");
}

/// 解析命令行参数
///
/// 对应 C 源码中的 getargs() 函数
/// 支持短选项和长选项，使用手工解析
fn parse_args(args: &[String]) -> Result<(Options, Vec<String>), i32> {
    let mut opts = Options {
        append: false,
        format: None,
        output_file: None,
        portability: false,
        quiet: false,
        verbose: false,
    };

    let mut i = 1; // 跳过程序名
    while i < args.len() {
        let arg = &args[i];
        if arg == "--" {
            // "--" 之后的所有参数都是命令
            i += 1;
            break;
        }
        if arg == "-a" || arg == "--append" {
            opts.append = true;
            i += 1;
        } else if arg == "-f" || arg == "--format" {
            if i + 1 >= args.len() {
                eprintln!("time: option '{}' requires an argument", arg);
                return Err(EXIT_CANCELED);
            }
            opts.format = Some(args[i + 1].clone());
            i += 2;
        } else if arg.starts_with("-f") || arg.starts_with("--format=") {
            // -fFORMAT 或 --format=FORMAT
            if arg.starts_with("-f") {
                opts.format = Some(arg[2..].to_string());
            } else {
                opts.format = Some(arg["--format=".len()..].to_string());
            }
            i += 1;
        } else if arg == "-o" || arg == "--output" || arg == "--output-file" {
            if i + 1 >= args.len() {
                eprintln!("time: option '{}' requires an argument", arg);
                return Err(EXIT_CANCELED);
            }
            opts.output_file = Some(args[i + 1].clone());
            i += 2;
        } else if arg.starts_with("-o") || arg.starts_with("--output=") || arg.starts_with("--output-file=") {
            if arg.starts_with("-o") {
                opts.output_file = Some(arg[2..].to_string());
            } else if arg.starts_with("--output=") {
                opts.output_file = Some(arg["--output=".len()..].to_string());
            } else {
                opts.output_file = Some(arg["--output-file=".len()..].to_string());
            }
            i += 1;
        } else if arg == "-p" || arg == "--portability" {
            opts.portability = true;
            i += 1;
        } else if arg == "-q" || arg == "--quiet" {
            opts.quiet = true;
            i += 1;
        } else if arg == "-v" || arg == "--verbose" {
            opts.verbose = true;
            i += 1;
        } else if arg == "-h" || arg == "--help" {
            print_help();
            std::process::exit(0);
        } else if arg == "-V" || arg == "--version" {
            print_version();
            std::process::exit(0);
        } else if arg.starts_with('-') {
            eprintln!("time: unrecognized option '{}'", arg);
            eprintln!("Try 'time --help' for more information.");
            return Err(EXIT_CANCELED);
        } else {
            // 非选项参数，是命令的开始
            break;
        }
    }

    let command: Vec<String> = args[i..].to_vec();
    if command.is_empty() {
        eprintln!("time: missing program to run");
        eprintln!("Try 'time --help' for more information.");
        return Err(EXIT_CANCELED);
    }

    Ok((opts, command))
}

fn main() {
    let args: Vec<String> = std::env::args().collect();

    let (opts, command) = match parse_args(&args) {
        Ok(result) => result,
        Err(code) => std::process::exit(code),
    };

    // 确定输出格式
    let output_format = if opts.verbose {
        verbose_format()
    } else if opts.portability {
        POSIX_FORMAT.to_string()
    } else if let Some(fmt) = &opts.format {
        fmt.clone()
    } else {
        // 检查环境变量 TIME
        std::env::var("TIME").unwrap_or_else(|_| DEFAULT_FORMAT.to_string())
    };

    // 确定输出目标
    let mut out_file: Option<File> = None;
    if let Some(outfile) = &opts.output_file {
        let file = if opts.append {
            File::options().append(true).create(true).open(outfile)
        } else {
            File::create(outfile)
        };
        match file {
            Ok(f) => out_file = Some(f),
            Err(e) => {
                eprintln!("time: {}: {}", outfile, e);
                std::process::exit(EXIT_CANCELED);
            }
        }
    }

    // 执行命令
    let result = run_command(&command);
    match result {
        Ok(resuse) => {
            // 格式化输出
            if let Some(ref mut file) = out_file {
                let _ = summarize(
                    file,
                    &output_format,
                    &command,
                    &resuse,
                    opts.quiet,
                    opts.portability,
                );
            } else {
                let mut err = stderr();
                let _ = summarize(
                    &mut err,
                    &output_format,
                    &command,
                    &resuse,
                    opts.quiet,
                    opts.portability,
                );
            }

            // 计算退出状态
            let status = compute_exit_status(&resuse);
            std::process::exit(status);
        }
        Err(code) => {
            std::process::exit(code);
        }
    }
}
