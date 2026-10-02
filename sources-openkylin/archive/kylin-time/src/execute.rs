//! 命令执行模块
//!
//! 对应 C 源码中的 run_command() 函数
//! 负责：
//! - fork 创建子进程
//! - 子进程 execvp 执行用户命令
//! - 父进程忽略 SIGINT/SIGQUIT，等待子进程结束
//! - 收集 rusage 数据

use crate::resuse::{resuse_end, resuse_start, ResUse};
use nix::sys::signal::{self, SigHandler, Signal};
use nix::unistd::{fork, ForkResult};

/// 退出码常量（与 C 源码一致）
pub const EXIT_CANCELED: i32 = 125;
pub const EXIT_CANNOT_INVOKE: i32 = 126;
pub const EXIT_ENOENT: i32 = 127;
pub const SIGNALLED_OFFSET: i32 = 128;

/// 执行命令并收集资源使用数据
///
/// 对应 C 源码中的 run_command()
pub fn run_command(cmd: &[String]) -> Result<ResUse, i32> {
    // 记录开始时间
    let start = resuse_start();

    // fork 子进程
    match unsafe { fork() } {
        Err(_) => {
            eprintln!("time: cannot fork: {}", std::io::Error::last_os_error());
            return Err(EXIT_CANCELED);
        }
        Ok(ForkResult::Child) => {
            // 子进程：执行用户命令
            let c_args: Vec<std::ffi::CString> = cmd
                .iter()
                .map(|s| {
                    std::ffi::CString::new(s.as_str())
                        .unwrap_or_else(|_| std::ffi::CString::new("?").unwrap())
                })
                .collect();
            let c_argv: Vec<*const std::ffi::c_char> = c_args
                .iter()
                .map(|s| s.as_ptr())
                .chain(std::iter::once(std::ptr::null()))
                .collect();

            let result = unsafe { libc::execvp(c_argv[0], c_argv.as_ptr()) };
            if result < 0 {
                let err = std::io::Error::last_os_error();
                eprintln!("time: cannot run {}: {}", cmd[0], err);
                let exit_code = if err.raw_os_error() == Some(libc::ENOENT) {
                    EXIT_ENOENT
                } else {
                    EXIT_CANNOT_INVOKE
                };
                std::process::exit(exit_code);
            }
            // execvp 成功不会返回
            unreachable!()
        }
        Ok(ForkResult::Parent { child }) => {
            // 父进程：忽略 SIGINT/SIGQUIT，防止自己被杀死
            let old_int = unsafe {
                signal::sigaction(
                    Signal::SIGINT,
                    &nix::sys::signal::SigAction::new(
                        SigHandler::SigIgn,
                        nix::sys::signal::SaFlags::empty(),
                        nix::sys::signal::SigSet::empty(),
                    ),
                )
            }
            .ok();

            let old_quit = unsafe {
                signal::sigaction(
                    Signal::SIGQUIT,
                    &nix::sys::signal::SigAction::new(
                        SigHandler::SigIgn,
                        nix::sys::signal::SaFlags::empty(),
                        nix::sys::signal::SigSet::empty(),
                    ),
                )
            }
            .ok();

            // 等待子进程并收集 rusage
            let result = resuse_end(child, start);

            // 恢复信号处理
            if let Some(old) = old_int {
                let _ = unsafe { signal::sigaction(Signal::SIGINT, &old) };
            }
            if let Some(old) = old_quit {
                let _ = unsafe { signal::sigaction(Signal::SIGQUIT, &old) };
            }

            match result {
                Ok(resuse) => Ok(resuse),
                Err(msg) => {
                    eprintln!("time: {}", msg);
                    Err(EXIT_CANCELED)
                }
            }
        }
    }
}

/// 根据子进程的 wait 状态计算 time 的退出码
///
/// 对应 C 源码 main() 末尾的退出状态计算
pub fn compute_exit_status(resp: &ResUse) -> i32 {
    match &resp.wait_status {
        nix::sys::wait::WaitStatus::Stopped(_, sig) => {
            (*sig as i32) + SIGNALLED_OFFSET
        }
        nix::sys::wait::WaitStatus::Signaled(_, sig, _) => {
            (*sig as i32) + SIGNALLED_OFFSET
        }
        nix::sys::wait::WaitStatus::Exited(_, code) => *code,
        nix::sys::wait::WaitStatus::Continued(_) => {
            eprintln!("time: unknown status from command (continued)");
            1
        }
        nix::sys::wait::WaitStatus::StillAlive => {
            eprintln!("time: unknown status from command (still alive)");
            1
        }
        // PtraceEvent 和 PtraceSyscall 在普通程序执行中不会出现
        _ => {
            eprintln!("time: unknown status from command");
            1
        }
    }
}