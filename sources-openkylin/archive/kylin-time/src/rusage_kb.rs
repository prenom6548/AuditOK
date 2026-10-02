//! 内存单位转换模块
//!
//! 不同 OS 的 getrusage() 返回的内存值单位不同：
//! - Linux/BSD: KB
//! - 某些系统: 字节
//! - 某些系统: 页(pages)
//!
//! 本模块统一将 rusage 的内存字段转换为 KB。

use libc::rusage;

/// 获取系统页大小（字节）
pub fn page_size() -> i64 {
    unsafe { libc::sysconf(libc::_SC_PAGESIZE) as i64 }
}

/// 将 rusage 中的内存相关字段统一转换为 KB
///
/// Linux 上 ru_maxrss 等字段已经以 KB 为单位返回，
/// 所以直接使用即可。
pub fn maxrss_kb(ru: &rusage) -> u64 {
    // Linux: ru_maxrss 已经是 KB
    ru.ru_maxrss as u64
}

pub fn ixrss_kb(ru: &rusage) -> u64 {
    // Linux: ru_ixrss 以 KB*ticks 为单位
    ru.ru_ixrss as u64
}

pub fn idrss_kb(ru: &rusage) -> u64 {
    // Linux: ru_idrss 以 KB*ticks 为单位
    ru.ru_idrss as u64
}

pub fn isrss_kb(ru: &rusage) -> u64 {
    // Linux: ru_isrss 以 KB*ticks 为单位
    ru.ru_isrss as u64
}