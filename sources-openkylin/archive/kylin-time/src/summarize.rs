//! 格式化输出模块
//!
//! 对应 C 源码中的 summarize() 函数
//! 实现自定义格式引擎，支持 20+ 种 %X 占位符
//! 以及 \t \n \\ 转义序列

use crate::rusage_kb;
use crate::resuse::ResUse;
use std::io::Write;
use std::time::Duration;

/// 默认输出格式
pub const DEFAULT_FORMAT: &str =
    "%Uuser %Ssystem %Eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k\n\
     %Iinputs+%Ooutputs (%Fmajor+%Rminor)pagefaults %Wswaps";

/// POSIX 格式 (-p 选项)
pub const POSIX_FORMAT: &str = "real %e\nuser %U\nsys %S";

/// 详细模式格式（-v 选项）
pub const VERBOSE_FORMAT_LINES: &[&str] = &[
    "\tCommand being timed: \"%C\"",
    "\tUser time (seconds): %U",
    "\tSystem time (seconds): %S",
    "\tPercent of CPU this job got: %P",
    "\tElapsed (wall clock) time (h:mm:ss or m:ss): %E",
    "\tAverage shared text size (kbytes): %X",
    "\tAverage unshared data size (kbytes): %D",
    "\tAverage stack size (kbytes): %p",
    "\tAverage total size (kbytes): %K",
    "\tMaximum resident set size (kbytes): %M",
    "\tAverage resident set size (kbytes): %t",
    "\tMajor (requiring I/O) page faults: %F",
    "\tMinor (reclaiming a frame) page faults: %R",
    "\tVoluntary context switches: %w",
    "\tInvoluntary context switches: %c",
    "\tSwaps: %W",
    "\tFile system inputs: %I",
    "\tFile system outputs: %O",
    "\tSocket messages sent: %s",
    "\tSocket messages received: %r",
    "\tSignals delivered: %k",
    "\tPage size (bytes): %Z",
    "\tExit status: %x",
];

/// 生成详细模式的格式字符串
pub fn verbose_format() -> String {
    VERBOSE_FORMAT_LINES
        .iter()
        .map(|s| *s)
        .collect::<Vec<&str>>()
        .join("\n")
}

/// 将 Duration 格式化为 [h:]mm:ss 或 mm:ss.ss 格式
/// 对应 C 源码中的 %E 占位符
fn format_elapsed_hms(elapsed: Duration) -> String {
    let total_secs = elapsed.as_secs();
    let subsec_millis = elapsed.subsec_millis();

    if total_secs >= 3600 {
        // h:mm:ss 格式
        let h = total_secs / 3600;
        let m = (total_secs % 3600) / 60;
        let s = total_secs % 60;
        format!("{}:{:02}:{:02}", h, m, s)
    } else {
        // m:ss.ss 格式 (ss 是百分之一秒)
        let m = total_secs / 60;
        let s = total_secs % 60;
        let cs = subsec_millis / 10; // 百分之一秒
        format!("{}:{:02}.{:02}", m, s, cs)
    }
}

/// 将 timeval (秒+微秒) 格式化为秒.ss 格式 (ss 是百分之一秒)
fn format_timeval_secs(secs: i64, usecs: i64) -> String {
    let whole_secs = secs;
    let centisecs = usecs / 10000; // 微秒 -> 百分之一秒
    format!("{}.{:02}", whole_secs, centisecs)
}

/// 格式化输出统计信息
///
/// 对应 C 源码中的 summarize() 函数
/// 遍历格式字符串，替换 %X 占位符和 \\ 转义序列
pub fn summarize(
    fp: &mut dyn Write,
    fmt: &str,
    command: &[String],
    resp: &ResUse,
    quiet: bool,
    is_posix: bool,
) -> std::io::Result<()> {
    let ru = &resp.ru;
    let elapsed = resp.elapsed;

    // 如果不是 quiet 模式且不是 POSIX 格式，打印异常终止信息
    if !quiet && !is_posix {
        match &resp.wait_status {
            nix::sys::wait::WaitStatus::Stopped(_, sig) => {
                writeln!(fp, "Command stopped by signal {}", sig)?;
            }
            nix::sys::wait::WaitStatus::Signaled(_, sig, _) => {
                writeln!(fp, "Command terminated by signal {}", sig)?;
            }
            nix::sys::wait::WaitStatus::Exited(_, code) if *code != 0 => {
                writeln!(fp, "Command exited with non-zero status {}", code)?;
            }
            _ => {}
        }
    }

    // 计算毫秒级时间值
    let r_ms: u64 = elapsed.as_secs() * 1000 + elapsed.subsec_millis() as u64; // 墙钟毫秒
    let v_ms: u64 = (ru.ru_utime.tv_sec as u64 * 1000 + ru.ru_utime.tv_usec as u64 / 1000)
        + (ru.ru_stime.tv_sec as u64 * 1000 + ru.ru_stime.tv_usec as u64 / 1000); // CPU毫秒

    // 微秒级时间值
    let us_r: u64 = elapsed.subsec_micros() as u64;
    let us_v: u64 = ru.ru_utime.tv_usec as u64 + ru.ru_stime.tv_usec as u64;

    // ticks per second: Linux 默认 100
    let ticks_per_sec: u64 = 100;
    let msec_per_tick: u64 = 1000 / ticks_per_sec;
    let v_ticks: u64 = if msec_per_tick > 0 { v_ms / msec_per_tick } else { 0 };

    // 遍历格式字符串
    let mut chars = fmt.chars().peekable();
    while let Some(ch) = chars.next() {
        match ch {
            '%' => {
                let next = chars.next();
                match next {
                    Some('%') => write!(fp, "%")?,
                    Some('C') => {
                        // 命令名和参数
                        let cmd_str = command.join(" ");
                        write!(fp, "{}", cmd_str)?;
                    }
                    Some('D') => {
                        // 平均未共享数据大小 (KB)
                        let val = if v_ticks == 0 {
                            0u64
                        } else {
                            rusage_kb::idrss_kb(ru) / v_ticks + rusage_kb::isrss_kb(ru) / v_ticks
                        };
                        write!(fp, "{}", val)?;
                    }
                    Some('E') => {
                        // 墙钟时间 [h:]m:s
                        write!(fp, "{}", format_elapsed_hms(elapsed))?;
                    }
                    Some('e') => {
                        // 墙钟时间（秒）
                        let secs = elapsed.as_secs();
                        let cs = elapsed.subsec_millis() / 10;
                        write!(fp, "{}.{:02}", secs, cs)?;
                    }
                    Some('F') => {
                        // 主要页错误
                        write!(fp, "{}", ru.ru_majflt)?;
                    }
                    Some('I') => {
                        // 文件系统输入
                        write!(fp, "{}", ru.ru_inblock)?;
                    }
                    Some('K') => {
                        // 平均总内存使用 (KB)
                        let val = if v_ticks == 0 {
                            0u64
                        } else {
                            rusage_kb::idrss_kb(ru) / v_ticks
                                + rusage_kb::isrss_kb(ru) / v_ticks
                                + rusage_kb::ixrss_kb(ru) / v_ticks
                        };
                        write!(fp, "{}", val)?;
                    }
                    Some('M') => {
                        // 最大驻留集大小 (KB)
                        write!(fp, "{}", rusage_kb::maxrss_kb(ru))?;
                    }
                    Some('O') => {
                        // 文件系统输出
                        write!(fp, "{}", ru.ru_oublock)?;
                    }
                    Some('P') => {
                        // CPU 占用百分比
                        if r_ms > 0 {
                            write!(fp, "{}%", v_ms * 100 / r_ms)?;
                        } else if us_r > 0 {
                            write!(fp, "{}%", us_v * 100 / us_r)?;
                        } else {
                            write!(fp, "?%")?;
                        }
                    }
                    Some('R') => {
                        // 次要页错误
                        write!(fp, "{}", ru.ru_minflt)?;
                    }
                    Some('S') => {
                        // 系统时间（秒）
                        write!(
                            fp,
                            "{}",
                            format_timeval_secs(ru.ru_stime.tv_sec, ru.ru_stime.tv_usec)
                        )?;
                    }
                    Some('U') => {
                        // 用户时间（秒）
                        write!(
                            fp,
                            "{}",
                            format_timeval_secs(ru.ru_utime.tv_sec, ru.ru_utime.tv_usec)
                        )?;
                    }
                    Some('W') => {
                        // 交换次数
                        write!(fp, "{}", ru.ru_nswap)?;
                    }
                    Some('X') => {
                        // 平均共享文本大小 (KB)
                        let val = if v_ticks == 0 {
                            0u64
                        } else {
                            rusage_kb::ixrss_kb(ru) / v_ticks
                        };
                        write!(fp, "{}", val)?;
                    }
                    Some('Z') => {
                        // 页大小
                        write!(fp, "{}", rusage_kb::page_size())?;
                    }
                    Some('c') => {
                        // 非自愿上下文切换
                        write!(fp, "{}", ru.ru_nivcsw)?;
                    }
                    Some('k') => {
                        // 传递的信号
                        write!(fp, "{}", ru.ru_nsignals)?;
                    }
                    Some('p') => {
                        // 平均栈大小 (KB)
                        let val = if v_ticks == 0 {
                            0u64
                        } else {
                            rusage_kb::isrss_kb(ru) / v_ticks
                        };
                        write!(fp, "{}", val)?;
                    }
                    Some('r') => {
                        // 接收的 socket 消息
                        write!(fp, "{}", ru.ru_msgrcv)?;
                    }
                    Some('s') => {
                        // 发送的 socket 消息
                        write!(fp, "{}", ru.ru_msgsnd)?;
                    }
                    Some('t') => {
                        // 平均驻留集大小 (KB)
                        let val = if v_ticks == 0 {
                            0u64
                        } else {
                            rusage_kb::idrss_kb(ru) / v_ticks
                        };
                        write!(fp, "{}", val)?;
                    }
                    Some('w') => {
                        // 自愿上下文切换
                        write!(fp, "{}", ru.ru_nvcsw)?;
                    }
                    Some('x') => {
                        // 退出状态
                        match &resp.wait_status {
                            nix::sys::wait::WaitStatus::Exited(_, code) => {
                                write!(fp, "{}", code)?;
                            }
                            _ => write!(fp, "?")?,
                        }
                    }
                    None => write!(fp, "?")?,
                    Some(other) => write!(fp, "?{}", other)?,
                }
            }
            '\\' => {
                let next = chars.next();
                match next {
                    Some('t') => write!(fp, "\t")?,
                    Some('n') => write!(fp, "\n")?,
                    Some('\\') => write!(fp, "\\")?,
                    None => write!(fp, "?\\")?,
                    Some(other) => write!(fp, "?\\{}", other)?,
                }
            }
            _ => write!(fp, "{}", ch)?,
        }
    }

    writeln!(fp)?;
    fp.flush()?;
    Ok(())
}