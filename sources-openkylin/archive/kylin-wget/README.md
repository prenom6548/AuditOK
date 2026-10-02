# wget-rust

这是一个在 `~/桌面/wget-rust/` 下进行的 **wget 纯 Rust 重写** 项目。

**长期目标**：与 **GNU Wget** 对外行为一致（CLI、退出码、协议与文件语义），便于脚本迁移；定义与验收见 **`docs/功能对等目标.md`**，选项差距见 **`docs/gnu_wget_options_inventory.md`**。

- 计划与进展：见 `重写计划书.md`
- 兼容矩阵：`docs/兼容矩阵.md`
- Rust 代码：见 `rust/`

## LaunchPad / 内网：Cargo 镜像

配置在 **仓库根** 的 **`.cargo/config.toml`**（与 `rust/` 同级），需 **随 Git / `*.orig.tar.gz` 一起发布**。在 `rust/` 下执行 `cargo build` 时，Cargo 会向上读取该文件。

修改镜像时只编辑 **`.cargo/config.toml`**。详见 `debian/README.source`。

## 快速开始

```bash
cd rust
cargo build
./target/debug/wget --help
```

## 发布/打包（可选）

```bash
# 在仓库根目录打包 release 二进制到 dist/
./scripts/build_dist.sh

# 在仓库根目录生成最小 .deb（release 产物为 `rust/target/release/wget`，安装为 `/usr/bin/wget`）
./scripts/build_deb.sh
```

安装说明（替换系统 wget）

这个包会通过 `dpkg-divert` 把系统的 `/usr/bin/wget` 挪到 diversion 路径，再由本包把 Rust 版本安装为新的 `/usr/bin/wget`。
因此安装后你直接调用 `wget ...` 即为本 Rust 实现（二进制本体亦为 `wget`）。

产物在 `dist/` 下：
- `wget-rust-<version>.tar.gz`（包内可执行文件名为 **`wget`**）
- （如环境中有）`wget-rust-<version>.tar.gz.sha256`
- `wget_<version>_<arch>.deb`（替换系统的 `/usr/bin/wget`）

