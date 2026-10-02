//! `--version` / `-V` 输出：Kylin Wget 品牌信息。

/// 与默认 User-Agent、命令行兼容目标一致的主版本号。
pub const WGET_COMPAT_VERSION: &str = "1.20.3";

/// clap 的 `version` / `long_version`：`wget -V` 多行输出。
pub const LONG_VERSION: &str = concat!(
    "Kylin Wget ",
    "1.20.3",
    " (rust ",
    env!("CARGO_PKG_VERSION"),
    ")\n",
    "\n",
    "+https +rustls (wget ",
    "1.20.3",
    " command-line compatible)\n",
    "\n",
    "Copyright (C) Kylin OS.\n",
);
