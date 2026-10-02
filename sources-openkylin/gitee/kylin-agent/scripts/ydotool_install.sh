#!/usr/bin/env bash

if [ -z "${BASH_VERSION:-}" ]; then
    printf '[ERROR] 请使用 bash 执行本脚本，例如：sudo bash %s\n' "$0" >&2
    printf '[ERROR] 不要使用 sh 执行，因为 sh 不支持 pipefail 等 bash 选项。\n' >&2
    exit 1
fi

set -Eeuo pipefail

RUN_USER="${2:-${YDOTOOL_RUN_USER:-${SUDO_USER:-${USER:-$(id -un)}}}}"
RUN_HOME_ENTRY="$(getent passwd "$RUN_USER" 2>/dev/null || true)"
RUN_HOME="${RUN_HOME_ENTRY#*:*:*:*:*:}"
RUN_HOME="${RUN_HOME%%:*}"
if [ -z "$RUN_HOME" ] || [ "$RUN_HOME" = "$RUN_HOME_ENTRY" ]; then
    RUN_HOME="$HOME"
fi

DOWNLOAD_DIR="${1:-$RUN_HOME/Downloads/ydotool-src}"
BUILD_DIR="$DOWNLOAD_DIR/build"
RUN_UID="$(id -u "$RUN_USER")"
RUN_GID="$(id -g "$RUN_USER")"
SOCKET_PATH="/run/user/$RUN_UID/.ydotool_socket"
SERVICE_TMP="/tmp/ydotoold.service.$$"

log() {
    printf '[INFO] %s\n' "$*"
}

fail() {
    printf '[ERROR] %s\n' "$*" >&2
    exit 1
}

run_step() {
    local desc="$1"
    shift

    log "$desc"
    "$@" || fail "$desc 失败"
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || fail "缺少命令：$1"
}

require_sudo() {
    if [ "$(id -u)" -eq 0 ]; then
        return 0
    fi

    require_command sudo
    run_step "验证 sudo 权限" sudo -v
}

run_as_root() {
    if [ "$(id -u)" -eq 0 ]; then
        "$@"
    else
        sudo "$@"
    fi
}

ydotool_is_ready() {
    local executable
    executable="$(command -v ydotool 2>/dev/null || true)"
    [ -n "$executable" ] || return 1
    command -v timeout >/dev/null 2>&1 || return 1

    local runtime_dir="/run/user/$RUN_UID"
    local socket_path="$runtime_dir/.ydotool_socket"
    local probe=(env "XDG_RUNTIME_DIR=$runtime_dir" "YDOTOOL_SOCKET=$socket_path"
                 timeout 3 "$executable" mousemove 0 0)

    if [ "$(id -u)" -eq 0 ] && [ "$RUN_UID" -ne 0 ]; then
        command -v runuser >/dev/null 2>&1 || return 1
        runuser -u "$RUN_USER" -- "${probe[@]}" >/dev/null 2>&1
    else
        "${probe[@]}" >/dev/null 2>&1
    fi
}

install_missing_apt_packages() {
    local packages=()

    command -v cmake >/dev/null 2>&1 || packages+=("cmake")
    command -v scdoc >/dev/null 2>&1 || packages+=("scdoc")

    # ydotool 配套工具：截图 grim，剪贴板 wl-clipboard
    command -v grim >/dev/null 2>&1 || packages+=("grim")
    command -v wl-copy >/dev/null 2>&1 || packages+=("wl-clipboard")

    if [ "${#packages[@]}" -eq 0 ]; then
        log "依赖工具已安装：cmake scdoc grim wl-clipboard"
        return 0
    fi

    log "安装缺失的依赖工具：${packages[*]}"
    if run_as_root apt install -y "${packages[@]}"; then
        return 0
    fi

    log "依赖安装失败，更新 apt 软件包索引后重试"
    run_step "更新 apt 软件包索引" run_as_root apt update
    run_step "重新安装缺失的依赖工具：${packages[*]}" run_as_root apt install -y "${packages[@]}"
}

main() {
    if ydotool_is_ready; then
        log "ydotool 客户端及 ydotoold 后端功能正常，跳过重复安装"
        return 0
    fi

    require_command apt
    require_command make
    require_command nproc
    require_command systemctl
    require_sudo

    install_missing_apt_packages
    require_command cmake
    require_command scdoc

    [ -f "$DOWNLOAD_DIR/CMakeLists.txt" ] || \
        fail "本地 ydotool 源码不存在或不完整：$DOWNLOAD_DIR"
    log "使用本地 ydotool 源码，不进行在线下载或更新：$DOWNLOAD_DIR"

    run_step "创建构建目录：$BUILD_DIR" mkdir -p "$BUILD_DIR"
    run_step "配置 ydotool" cmake -S "$DOWNLOAD_DIR" -B "$BUILD_DIR"
    run_step "编译 ydotool" make -C "$BUILD_DIR" -j "$(nproc)"
    run_step "安装 ydotool" run_as_root make -C "$BUILD_DIR" install

    cat >"$SERVICE_TMP" <<EOF
[Unit]
Description=Starts ydotoold Daemon
After=multi-user.target

[Service]
Type=simple
Restart=always
RestartSec=3
ExecStartPre=/bin/mkdir -p /run/user/$RUN_UID
ExecStartPre=/bin/chown $RUN_UID:$RUN_GID /run/user/$RUN_UID
ExecStartPre=/bin/rm -f "$SOCKET_PATH"
ExecStart=/usr/local/bin/ydotoold --socket-path "$SOCKET_PATH" --socket-own $RUN_UID:$RUN_GID
ExecReload=/usr/bin/kill -HUP \$MAINPID
KillMode=process
TimeoutSec=180

[Install]
WantedBy=multi-user.target
EOF

    run_step "生成 systemd 服务文件" test -s "$SERVICE_TMP"
    run_step "安装 systemd 服务文件" run_as_root install -m 0644 "$SERVICE_TMP" /etc/systemd/system/ydotoold.service
    run_step "删除临时服务文件" rm -f "$SERVICE_TMP"
    run_step "重新加载 systemd" run_as_root systemctl daemon-reload
    run_step "启用并启动 ydotoold 服务" run_as_root systemctl enable --now ydotoold
    run_step "重启 ydotoold 服务以应用最新配置" run_as_root systemctl restart ydotoold.service
    run_step "检查 ydotoold 服务状态" systemctl is-active --quiet ydotoold

    log "ydotool 安装完成"
    log "socket 路径：$SOCKET_PATH"
}

main "$@"
