# 模型管理工具

这是 `add-model-management-tool` OpenSpec 变更的首阶段实现代码，位于独立目录 `model-management-tool/`。

首阶段范围：

- Tauri 桌面端应用骨架。
- 本地模型文件/目录导入，包括拖拽路径处理。
- 本地模型包目录生成：`<model_name>/1`、`config.pbtxt`、`model_description.yaml`。
- 允许编辑的模型元数据字段校验和模板渲染。
- 云端仓库接入真实接口：从 `default.json` 拉取模型清单并断点续传下载，下载产物可直接打包。

运行依赖：

- Node.js（建议 18 及以上）及其包管理器（npm 等）
- Rust 工具链（`cargo`、`rustc`，建议通过 rustup 安装，需 1.77.2 及以上；openKylin apt 源中的 1.75.0 版本低于 Tauri 2 的 MSRV，请勿使用）
- Linux 系统依赖按系统版本区分（openKylin/Ubuntu/Debian）：
  - **V11（Tauri 2）**：`libwebkit2gtk-4.1-dev`、`libjavascriptcoregtk-4.1-dev`、`libsoup-3.0-dev`、`libgtk-3-dev`（提供 `gdk-3.0`）、`libayatana-appindicator3-dev`、`librsvg2-dev`、`libxdo-dev`、`libssl-dev`、`pkg-config`、`build-essential`、`curl`、`wget`、`file`
  - **V10（Tauri 1）**：`libwebkit2gtk-4.0-dev`、`libjavascriptcoregtk-4.0-dev`、`libsoup2.4-dev`、`libgtk-3-dev`、`libssl-dev`、`pkg-config`、`build-essential`、`curl`、`wget`、`file`

<details>
<summary>手动安装步骤（供参考）</summary>

```bash
# 1. 安装 Linux 系统依赖（按系统版本二选一）
sudo apt update
# V11（Tauri 2）：
sudo apt install -y libwebkit2gtk-4.1-dev libjavascriptcoregtk-4.1-dev libsoup-3.0-dev \
  libgtk-3-dev libayatana-appindicator3-dev \
  librsvg2-dev libxdo-dev libssl-dev pkg-config build-essential curl wget file
# V10（Tauri 1）：
sudo apt install -y libwebkit2gtk-4.0-dev libjavascriptcoregtk-4.0-dev libsoup2.4-dev \
  libgtk-3-dev libssl-dev pkg-config build-essential curl wget file

# 2. 安装 Rust 工具链（rustup，使用 rsproxy 镜像加速）
export RUSTUP_DIST_SERVER=https://rsproxy.cn
export RUSTUP_UPDATE_ROOT=https://rsproxy.cn/rustup
curl --proto '=https' --tlsv1.2 -sSf https://rsproxy.cn/rustup-init.sh | sh -s -- -y

# 3. 在当前 shell 加载 cargo 环境变量（新开终端会自动生效）
source "$HOME/.cargo/env"

# 4. 配置 cargo crates 镜像，加速后续依赖下载（写入 ~/.cargo/config.toml）
cat > ~/.cargo/config.toml <<'EOF'
[source.crates-io]
replace-with = 'rsproxy-sparse'

[source.rsproxy-sparse]
registry = "sparse+https://rsproxy.cn/index/"

[net]
git-fetch-with-cli = true
EOF

# 5. 校验安装
cargo --version
rustc --version

# 6. 安装前端依赖
npm install
```

</details>

常用命令：

```bash
npm install
npm run dev
npm run tauri dev      # 自动按系统版本选择 Tauri（V10 → Tauri 1，V11 → Tauri 2）
npm run tauri build    # 同上
npm test
cd src-tauri && cargo test       # Tauri 2 crate（仅 V11 可编译）
cd src-tauri-v1 && cargo test    # Tauri 1 crate（V10/V11 均可编译）
```

系统版本适配（V10 / V11）：

`npm run tauri <子命令>`（等价于 `npm run tauri:dev` / `npm run tauri:build`）会读取 `/etc/os-release` 的 `VERSION_ID` 自动选择构建链（也可用环境变量 `KYLIN_VERSION=v10|v11` 强制指定）：

- **麒麟 V10**：glib 2.64 低于 Tauri 2 要求的 2.70，且缺少 webkit2gtk-4.1，因此使用 **Tauri 1**。构建入口为独立 crate `src-tauri-v1/`（与 `src-tauri` 共享同一份 Rust 源码，通过 `tauri-v1` feature 选择对话框/事件 API），Tauri CLI 通过 `npx @tauri-apps/cli@1` 临时引入；前端 `@tauri-apps/api` v2 导入由 `src/lib/tauri-v1/` 下的 shim 经 vite alias 映射到 `@tauri-apps/api@1`。
- **麒麟 V11**：使用 **Tauri 2**，构建入口为 `src-tauri/`，行为与原来一致。

构建时会自动在 deb 版本号上追加系统版本后缀以区分产物（如 `model-management-tool_0.1.0-v10_amd64.deb` / `model-management-tool_0.1.0-v11_amd64.deb`），通过 `tauri build --config` 覆盖 `version` 实现，不改动 `tauri.conf.json`。

两个版本的系统依赖不同，见上文“运行依赖”