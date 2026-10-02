//! 资源测量模块
//!
//! 对应 C 源码中的 resuse.h / resuse.c
//! 负责：
//! - 记录命令开始时间 (resuse_start)
//! - 等待子进程结束并收集 rusage 数据 (resuse_end)
//! - 计算墙钟耗时 (elapsed = end - start)

use libc::rusage;
use nix::sys::wait::WaitStatus;
use nix::unistd::Pid;
use std::time::{Duration, Instant};

/// 资源使用数据结构
///
/// 对应 C 源码中的 RESUSE 结构体
pub struct ResUse {
    /// 子进程的 wait 状态
    pub wait_status: WaitStatus,
    /// 内核返回的 rusage 数据
    pub ru: rusage,
    /// 墙钟耗时
    pub elapsed: Duration,
}

/// 记录命令开始时间
///
/// 使用 Instant 获取高精度时间戳
pub fn resuse_start() -> Instant {
    Instant::now()
}

/// 等待子进程结束并收集 rusage 数据
///
/// 对应 C 源码中的 resuse_end()
/// 使用 wait4() 系统调用同时等待子进程和收集资源使用数据
/// (wait4 是 wait3 的现代替代，支持指定 pid)
pub fn resuse_end(pid: Pid, start: Instant) -> Result<ResUse, String> {
    // 使用 wait4 等待指定子进程并收集 rusage
    let mut ru: rusage = unsafe { std::mem::zeroed() };
    let mut status: i32 = 0;

    let result = unsafe {
        libc::wait4(
            pid.as_raw(),
            &mut status,
            0,
            &mut ru,
        )
    };

    if result < 0 {
        return Err(format!(
            "error waiting for child process: {}",
            std::io::Error::last_os_error()
        ));
    }

    // 计算墙钟耗时
    let elapsed = start.elapsed();

    // 解析 wait 状态
    let wait_status = WaitStatus::from_raw(pid, status)
        .map_err(|e| format!("unknown wait status {}: {:?}", status, e))?;

    Ok(ResUse {
        wait_status,
        ru,
        elapsed,
    })
}