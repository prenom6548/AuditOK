#!/usr/bin/env bash
# 一键安装本项目全部运行/构建依赖：
#   1) Linux 系统依赖（按麒麟 V10/V11 自动选择，可用 KYLIN_VERSION=v10|v11 强制指定）
#   2) Node.js / npm（缺失或低于 18 时从 npmmirror 安装 LTS，并升级 npm 到最新）
#   3) Rust 工具链（rustup + rsproxy 国内镜像）
#   4) cargo crates 镜像加速配置
#   5) npm install
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

info() { printf '\033[1;32m[install-deps]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[install-deps]\033[0m %s\n' "$*"; }
die()  { printf '\033[1;31m[install-deps]\033[0m %s\n' "$*" >&2; exit 1; }

SUDO=""
if [ "$(id -u)" -ne 0 ]; then
  if command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
  else
    die "需要 root 权限安装系统依赖，且当前环境无 sudo"
  fi
fi

detect_kylin_version() {
  local forced="${KYLIN_VERSION:-}"
  case "$forced" in
    v10|v11) printf '%s' "$forced"; return ;;
    "") ;;
    *) warn "无效的 KYLIN_VERSION=$forced，忽略并按自动检测处理" ;;
  esac
  if [ -r /etc/os-release ] && grep -qi '^VERSION_ID="\?v10' /etc/os-release; then
    printf 'v10'
  else
    printf 'v11'
  fi
}

KYLIN_VERSION="$(detect_kylin_version)"
info "系统版本: $KYLIN_VERSION"

if [ "$KYLIN_VERSION" = "v10" ]; then
  PKGS=(libwebkit2gtk-4.0-dev libjavascriptcoregtk-4.0-dev libsoup2.4-dev
        libgtk-3-dev libssl-dev pkg-config build-essential curl wget file)
else
  PKGS=(libwebkit2gtk-4.1-dev libjavascriptcoregtk-4.1-dev libsoup-3.0-dev
        libgtk-3-dev libayatana-appindicator3-dev librsvg2-dev libxdo-dev
        libssl-dev pkg-config build-essential curl wget file)
fi
info "安装系统依赖 (apt): ${PKGS[*]}"
$SUDO apt-get update
$SUDO apt-get install -y "${PKGS[@]}"

NODE_LTS="v20.18.1"
NODE_MIRROR="https://cdn.npmmirror.com/binaries/node"
NODE_MIN_MAJOR=18

node_major() { node -p 'process.versions.node.split(".")[0]' 2>/dev/null || echo 0; }

install_node_from_mirror() {
  local arch
  case "$(uname -m)" in
    x86_64)          arch="x64" ;;
    aarch64|arm64)   arch="arm64" ;;
    *) die "不支持的 CPU 架构: $(uname -m)，请手动安装 Node.js $NODE_LTS" ;;
  esac
  local pkg="node-$NODE_LTS-linux-$arch"
  local tarball="$pkg.tar.xz"
  info "从 npmmirror 下载 Node.js $NODE_LTS ($arch)..."
  local tmpdir
  tmpdir="$(mktemp -d)"
  trap 'rm -rf "$tmpdir"' RETURN
  curl -fSL --retry 3 -o "$tmpdir/$tarball" "$NODE_MIRROR/$NODE_LTS/$tarball"
  info "解压并安装到 /usr/local ..."
  tar -xJf "$tmpdir/$tarball" -C "$tmpdir"
  $SUDO cp -rf "$tmpdir/$pkg"/{bin,include,lib,share} /usr/local/
}

if ! command -v node >/dev/null 2>&1; then
  info "未检测到 Node.js，开始安装..."
  install_node_from_mirror
elif [ "$(node_major)" -lt "$NODE_MIN_MAJOR" ]; then
  warn "当前 Node.js $(node -v) 低于 $NODE_MIN_MAJOR，升级到 $NODE_LTS..."
  install_node_from_mirror
else
  info "Node.js $(node -v)"
fi
command -v node >/dev/null 2>&1 || die "Node.js 安装失败"
hash -r
info "Node.js $(node -v)"

info "升级 npm 到 10.x 最新版本..."
$SUDO env "PATH=$PATH" npm install -g npm@10 --registry=https://registry.npmmirror.com
info "npm $(npm -v)"

if [ ! -f "$ROOT_DIR/.npmrc" ]; then
  info "配置 npm registry 镜像 → $ROOT_DIR/.npmrc"
  echo "registry=https://registry.npmmirror.com" > "$ROOT_DIR/.npmrc"
else
  info "已存在 .npmrc，跳过 registry 镜像配置"
fi

RUST_MIN="1.77.2"

# 若以 sudo（root）执行本脚本，rustup 必须装到原始用户下，
# 否则普通用户构建时找不到 cargo（报 failed to get cargo metadata）
RUST_INSTALL_USER=""
RUST_USER_HOME="$HOME"
if [ "$(id -u)" -eq 0 ] && [ -n "${SUDO_USER:-}" ]; then
  RUST_INSTALL_USER="$SUDO_USER"
  RUST_USER_HOME="$(getent passwd "$SUDO_USER" | cut -d: -f6)"
  info "检测到 sudo 运行，Rust 工具链将安装到原始用户 $RUST_INSTALL_USER ($RUST_USER_HOME)"
elif [ "$(id -u)" -eq 0 ]; then
  warn "当前直接以 root 运行，Rust 将装到 /root/.cargo，普通用户构建时可能找不到 cargo；建议以普通用户执行本脚本"
fi
CARGO_DIR="$RUST_USER_HOME/.cargo"

cargo_version_of() { "$1" --version 2>/dev/null | awk '{print $2}'; }
CUR=""
if command -v cargo >/dev/null 2>&1; then
  CUR="$(cargo_version_of cargo)"
elif [ -x "$CARGO_DIR/bin/cargo" ]; then
  CUR="$(cargo_version_of "$CARGO_DIR/bin/cargo")"
fi

need_rust=1
if [ -n "$CUR" ] && printf '%s\n%s\n' "$RUST_MIN" "$CUR" | sort -VC; then
  need_rust=0
  info "已安装 Rust 工具链 $CUR，跳过安装"
elif [ -n "$CUR" ]; then
  warn "现有 cargo $CUR 低于 $RUST_MIN（Tauri 2 的 MSRV；openKylin apt 源中的 1.75 不可用），将用 rustup 安装"
fi
if [ "$need_rust" -eq 1 ]; then
  info "安装 Rust 工具链（rustup + rsproxy 镜像）..."
  RUSTUP_ENV='RUSTUP_DIST_SERVER=https://rsproxy.cn RUSTUP_UPDATE_ROOT=https://rsproxy.cn/rustup'
  RUSTUP_CMD='curl --proto =https --tlsv1.2 -sSf https://rsproxy.cn/rustup-init.sh | sh -s -- -y --profile minimal'
  if [ -n "$RUST_INSTALL_USER" ]; then
    sudo -u "$RUST_INSTALL_USER" -H env $RUSTUP_ENV sh -c "$RUSTUP_CMD"
  else
    env $RUSTUP_ENV sh -c "$RUSTUP_CMD"
  fi
fi
[ -f "$CARGO_DIR/env" ] && . "$CARGO_DIR/env"
export PATH="$CARGO_DIR/bin:$PATH"
if command -v cargo >/dev/null 2>&1; then
  info "$(cargo --version)"
else
  die "Rust 工具链安装失败：未找到 cargo，请检查网络后重试"
fi

CARGO_CONFIG="$CARGO_DIR/config.toml"
if [ ! -f "$CARGO_CONFIG" ] && [ ! -f "$CARGO_DIR/config" ]; then
  info "配置 cargo crates 镜像加速 → $CARGO_CONFIG"
  mkdir -p "$CARGO_DIR"
  cat > "$CARGO_CONFIG" <<'EOF'
[source.crates-io]
replace-with = 'rsproxy-sparse'

[source.rsproxy-sparse]
registry = "sparse+https://rsproxy.cn/index/"

[net]
git-fetch-with-cli = true
EOF
  if [ -n "$RUST_INSTALL_USER" ]; then
    chown -R "$RUST_INSTALL_USER:$RUST_INSTALL_USER" "$CARGO_DIR"
  fi
else
  info "已存在 cargo 配置文件，跳过镜像配置"
fi

info "安装前端依赖 (npm install)..."
(cd "$ROOT_DIR" && npm install)

if [ "$KYLIN_VERSION" = "v10" ]; then
  CLI_PKG="@tauri-apps/cli-v1"
else
  CLI_PKG="@tauri-apps/cli"
fi
if [ ! -f "$ROOT_DIR/node_modules/$CLI_PKG/tauri.js" ]; then
  die "依赖安装不全：缺少 $CLI_PKG（Tauri CLI），请检查网络/registry 后重新执行本脚本"
fi
info "已验证 Tauri CLI: $CLI_PKG ($(node "$ROOT_DIR/node_modules/$CLI_PKG/tauri.js" --version))"

info "全部依赖安装完成！"
if [ -n "$RUST_INSTALL_USER" ]; then
  info "提示: Rust 装在 $RUST_INSTALL_USER 用户下，若终端中直接使用 cargo 需先执行 source ~/.cargo/env（npm run tauri build 会自动处理）"
fi
info "后续使用: npm run dev | npm run tauri dev | npm run tauri build"
