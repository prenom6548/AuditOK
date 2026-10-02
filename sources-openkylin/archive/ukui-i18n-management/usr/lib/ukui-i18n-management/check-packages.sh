#!/bin/bash
# 语言配置软件包检测脚本
#
# 由 ukui-i18n-management.service 调用，触发时机：
#   1. 开机联网后（systemd oneshot）
#   2. 手动连接网络后（NetworkManager dispatcher 监听 connectivity-change）
# 读取 /etc/ukui-i18n-management/packages.conf 中的包名列表，
# 逐一检查是否已安装；若存在未安装的包，则通过
# org.freedesktop.Notifications D-Bus 接口向当前桌面用户发送通知，
# 并监听 ActionInvoked 信号，在用户点击通知消息体时跳转
# ukui-control-center 控制面板。

set -euo pipefail

# 语言配置包列表配置文件路径
CONFIG_FILE="/etc/ukui-i18n-management/packages.conf"
# syslog/journal 日志标识，亦作为通知 app_name
LOG_TAG="ukui-i18n-management"
# 通知点击后跳转的目标控制面板（可由配置文件 control-center 覆盖）
CONTROL_CENTER="ukui-control-center"
# 当前语言区块标识
CURRENT_LANGUAGE=""
# 当前语言区块待检测包列表
CONFIG_PACKAGES=()
# 用户 LANGUAGE 原始值及解析后的候选列表
USER_LANGUAGE_RAW=""
USER_LANGUAGE_CANDIDATES=()
LANGUAGE_MATCH_SPECS=()
# 通知发送与 ActionInvoked 监听脚本（须在同一 D-Bus 连接中完成）
NOTIFY_SCRIPT="/usr/lib/ukui-i18n-management/send-notification.py"
# 短期去重：避免开机检测与 NM 连通性事件在数秒内重复触发
RUN_COOLDOWN_FILE="/run/ukui-i18n-management/last-run"
RUN_COOLDOWN_SEC=10

# 写入系统日志，便于 journalctl -t ukui-i18n-management 排查
log() {
    logger -t "$LOG_TAG" "$@"
}

# 判断网络是否可用
# 优先使用 NetworkManager 的 nm-online（能感知"真正联网"而非仅链路 up），
# 连通性刚恢复时 nm-online 可能短暂未就绪，重试数次后再判定失败。
is_network_available() {
    local attempt

    for attempt in 1 2 3; do
        if command -v nm-online >/dev/null 2>&1; then
            nm-online -q -t 5 2>/dev/null && return 0
        fi

        for host in 223.5.5.5 8.8.8.8 1.1.1.1; do
            if ping -c1 -W3 "$host" >/dev/null 2>&1; then
                return 0
            fi
        done

        [[ "$attempt" -lt 3 ]] && sleep 3
    done
    return 1
}

# 通过 dpkg 查询指定 deb 包是否处于 "install ok installed" 状态
is_package_installed() {
    local pkg="$1"
    dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null \
        | grep -q "install ok installed"
}

# 规范化语言代码（zh_CN.UTF-8 / zh-CN → zh_CN）
normalize_lang_code() {
    local lang="$1"
    lang="${lang%%.*}"
    lang="${lang//-/_}"
    echo "$lang"
}

# 系统 locale 兜底（仅在无法读取用户环境时使用）
get_system_language_fallback() {
    local lang=""

    lang="${LANG:-}"
    if [[ -z "$lang" ]] && command -v localectl >/dev/null 2>&1; then
        lang=$(localectl status 2>/dev/null \
            | awk -F: '/System Locale|LANG/{gsub(/^[ \t]+/,"",$2); print $2; exit}')
    fi
    if [[ -z "$lang" && -f /etc/default/locale ]]; then
        lang=$(grep -m1 '^LANG=' /etc/default/locale 2>/dev/null \
            | cut -d= -f2- | tr -d '"')
    fi

    normalize_lang_code "$lang"
}

# 读取当前桌面用户的 LANGUAGE 环境变量，生成候选语言列表
# LANGUAGE 格式示例：zh_CN:zh（冒号分隔优先级列表）
get_user_language_candidates() {
    local user="$1"
    local language_raw="" lang_raw="" part
    local -a candidates=()

    USER_LANGUAGE_RAW=""

    language_raw=$(sudo -u "$user" bash -lc 'printf %s "${LANGUAGE:-}"' 2>/dev/null || true)
    if [[ -n "$language_raw" ]]; then
        USER_LANGUAGE_RAW="$language_raw"
        IFS=':' read -ra parts <<< "$language_raw"
        for part in "${parts[@]}"; do
            part=$(normalize_lang_code "$part")
            [[ -n "$part" ]] && candidates+=("$part")
        done
    fi

    if [[ ${#candidates[@]} -eq 0 ]]; then
        lang_raw=$(sudo -u "$user" bash -lc 'printf %s "${LANG:-}"' 2>/dev/null || true)
        if [[ -n "$lang_raw" ]]; then
            USER_LANGUAGE_RAW="LANG=${lang_raw}"
            candidates+=("$(normalize_lang_code "$lang_raw")")
        fi
    fi

    if [[ ${#candidates[@]} -eq 0 ]]; then
        candidates+=("$(get_system_language_fallback)")
        USER_LANGUAGE_RAW="fallback=${candidates[0]}"
    fi

    USER_LANGUAGE_CANDIDATES=("${candidates[@]}")
}

# 判断语言区块是否匹配候选语言
# match_mode: exact:<code> / prefix:<code> / default / first
language_block_matches() {
    local block_lang="$1" match_spec="$2"
    local match_mode="${match_spec%%:*}"
    local match_lang="${match_spec#*:}"
    local lang_prefix="${match_lang%%_*}"

    case "$match_mode" in
        exact)
            [[ "$block_lang" == "$match_lang" ]]
            ;;
        prefix)
            [[ "$block_lang" == "$lang_prefix" ]]
            ;;
        default)
            [[ "$block_lang" == "default" ]]
            ;;
        first)
            return 0
            ;;
    esac
}

# 构建语言匹配策略列表（基于用户 LANGUAGE 候选）
build_language_match_specs() {
    local -a specs=() candidate

    for candidate in "${USER_LANGUAGE_CANDIDATES[@]}"; do
        specs+=("exact:${candidate}")
    done
    for candidate in "${USER_LANGUAGE_CANDIDATES[@]}"; do
        specs+=("prefix:${candidate}")
    done
    specs+=("default")
    specs+=("first")

    LANGUAGE_MATCH_SPECS=("${specs[@]}")
}

# 解析配置文件中的一行，提取 LANGUAGE / PACKAGE / CONTROL-CENTER 信息
# 配置文件采用 YAML 风格缩进列表：
#   LANGUAGE: zh_CN
#       - PACKAGE:
#           - apt
#           - dpkg
parse_config_line() {
    local line="$1"

    if [[ "$line" =~ ^LANGUAGE:[[:space:]]*(.+)$ ]]; then
        PARSED_TYPE="language"
        PARSED_VALUE="${BASH_REMATCH[1]}"
        PARSED_VALUE="${PARSED_VALUE// /}"
        return 0
    fi

    if [[ "$line" =~ ^[[:space:]]*-[[:space:]]*PACKAGE:[[:space:]]*$ ]]; then
        PARSED_TYPE="package_section"
        PARSED_VALUE=""
        return 0
    fi

    if [[ "$line" =~ ^[[:space:]]*-[[:space:]]*CONTROL-CENTER:[[:space:]]*(.+)$ ]]; then
        PARSED_TYPE="control_center"
        PARSED_VALUE="${BASH_REMATCH[1]}"
        PARSED_VALUE="${PARSED_VALUE// /}"
        return 0
    fi

    if [[ "$line" =~ ^[[:space:]]+-[[:space:]]+([^[:space:]].*)$ ]]; then
        PARSED_TYPE="list_item"
        PARSED_VALUE="${BASH_REMATCH[1]}"
        PARSED_VALUE="${PARSED_VALUE// /}"
        return 0
    fi

    PARSED_TYPE=""
    PARSED_VALUE=""
    return 1
}

# 从配置文件加载当前用户语言对应的包列表
# 根据用户 LANGUAGE 环境变量匹配 LANGUAGE 区块，读取 PACKAGE 列表
load_language_config_for_user() {
    local user="$1"
    local match_spec block_lang matched_lang line
    local parsed_type="" parsed_value=""
    local in_package_section=0
    local found=0

    CONFIG_PACKAGES=()
    CURRENT_LANGUAGE=""

    if [[ ! -f "$CONFIG_FILE" ]]; then
        log "配置文件不存在: $CONFIG_FILE"
        exit 1
    fi

    get_user_language_candidates "$user"
    build_language_match_specs

    log "用户 ${user} 语言环境: ${USER_LANGUAGE_RAW:-unknown}"

    for match_spec in "${LANGUAGE_MATCH_SPECS[@]}"; do
        block_lang=""
        matched_lang=""
        CONTROL_CENTER="ukui-control-center"
        in_package_section=0
        found=0

        while IFS= read -r line || [[ -n "$line" ]]; do
            line="${line%%#*}"
            line="${line%"${line##*[![:space:]]}"}"
            [[ -z "${line//[[:space:]]/}" ]] && continue

            parse_config_line "$line" || continue

            parsed_type="$PARSED_TYPE"
            parsed_value="$PARSED_VALUE"

            if [[ "$parsed_type" == "language" ]]; then
                [[ "$found" -eq 1 ]] && break
                block_lang="$parsed_value"
                in_package_section=0
                continue
            fi

            [[ -z "$block_lang" ]] && continue
            language_block_matches "$block_lang" "$match_spec" || continue

            matched_lang="$block_lang"

            case "$parsed_type" in
                package_section)
                    in_package_section=1
                    ;;
                control_center)
                    CONTROL_CENTER="$parsed_value"
                    found=1
                    ;;
                list_item)
                    if [[ "$in_package_section" -eq 1 && -n "$parsed_value" ]]; then
                        CONFIG_PACKAGES+=("$parsed_value")
                        found=1
                    fi
                    ;;
            esac
        done < "$CONFIG_FILE"

        if [[ "$found" -eq 1 ]]; then
            CURRENT_LANGUAGE="$matched_lang"
            log "使用语言配置区块: ${matched_lang} (用户语言: ${USER_LANGUAGE_RAW}, 匹配: ${match_spec})"
            return 0
        fi
    done

    log "未找到匹配的语言配置区块 (用户语言: ${USER_LANGUAGE_RAW:-unknown})"
    exit 1
}

# 获取当前活跃的图形桌面会话信息
# 服务以 root 运行，但 D-Bus 通知必须发送到已登录的图形用户会话，
# 因此需要通过 loginctl 找到 wayland/x11 类型会话的用户名、UID 和 DISPLAY。
# 成功时输出格式: "<用户名> <uid> <display>"
get_active_desktop_session() {
    if ! command -v loginctl >/dev/null 2>&1; then
        return 1
    fi

    local session user uid display
    while read -r session _; do
        [[ -z "$session" ]] && continue
        local type
        type=$(loginctl show-session "$session" -p Type --value 2>/dev/null || true)
        # 仅关注图形桌面会话，忽略 tty/ssh 等
        [[ "$type" != "wayland" && "$type" != "x11" ]] && continue

        user=$(loginctl show-session "$session" -p Name --value 2>/dev/null || true)
        display=$(loginctl show-session "$session" -p Display --value 2>/dev/null || true)
        [[ -z "$user" ]] && continue

        uid=$(id -u "$user" 2>/dev/null || true)
        if [[ -n "$uid" ]]; then
            echo "$user $uid $display"
            return 0
        fi
    done < <(loginctl list-sessions --no-legend 2>/dev/null || true)

    return 1
}

# 解析图形桌面环境变量
# UKUI Wayland 会话需要 WAYLAND_DISPLAY 和正确的 DISPLAY（通常为 :1 而非 :0）
resolve_graphical_env() {
    local user="$1" uid="$2" loginctl_display="$3"
    local display="" wayland=""

    display=$(sudo -u "$user" bash -lc 'printf %s "$DISPLAY"' 2>/dev/null || true)
    [[ -z "$display" ]] && display="$loginctl_display"
    [[ -z "$display" || "$display" == "?" ]] && display=":1"

    if [[ -S "/run/user/${uid}/wayland-0" ]]; then
        wayland="wayland-0"
    fi

    echo "$display $wayland"
}

# 后台启动通知进程：在同一 D-Bus 连接中发送 Notify 并监听 ActionInvoked
# 注意：nohup 无法可靠包装 bash 函数，须直接使用 sudo 启动
spawn_notification_handler() {
    local user="$1" uid="$2" display="$3" wayland="$4"
    local title="$5" body="$6"
    local pid

    if [[ ! -x "$NOTIFY_SCRIPT" ]]; then
        log "通知脚本不存在或不可执行: ${NOTIFY_SCRIPT}"
        return 1
    fi

    if [[ -n "$wayland" ]]; then
        nohup sudo -u "$user" \
            DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/${uid}/bus" \
            DISPLAY="$display" \
            WAYLAND_DISPLAY="$wayland" \
            XDG_RUNTIME_DIR="/run/user/${uid}" \
            XDG_CURRENT_DESKTOP="UKUI" \
            "$NOTIFY_SCRIPT" "$title" "$body" \
            --control-center "$CONTROL_CENTER" \
            > /dev/null 2>&1 &
    else
        nohup sudo -u "$user" \
            DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/${uid}/bus" \
            DISPLAY="$display" \
            XDG_RUNTIME_DIR="/run/user/${uid}" \
            XDG_CURRENT_DESKTOP="UKUI" \
            "$NOTIFY_SCRIPT" "$title" "$body" \
            --control-center "$CONTROL_CENTER" \
            > /dev/null 2>&1 &
    fi

    pid=$!
    disown 2>/dev/null || true
    log "已启动通知与 ActionInvoked 监听进程 (pid=${pid}, DISPLAY=${display}, WAYLAND=${wayland:-none})"
}

# 向桌面用户发送缺失语言包的安装提醒通知
send_notification() {
    local missing_count="$1"
    local session_info user uid loginctl_display display wayland env_info
    local title body

    session_info=$(get_active_desktop_session) || {
        log "未找到活跃的图形桌面会话，跳过通知"
        return 0
    }

    read -r user uid loginctl_display <<< "$session_info"

    if [[ ! -S "/run/user/${uid}/bus" ]]; then
        log "用户 ${user} 的 D-Bus 会话不可用，跳过通知"
        return 0
    fi

    env_info=$(resolve_graphical_env "$user" "$uid" "$loginctl_display")
    read -r display wayland <<< "$env_info"

    title="语言配置包未完整安装"
    body="检测到 ${missing_count} 个语言配置软件包尚未安装，请点击打开控制面板完成安装。"

    log "向用户 ${user} 发送 D-Bus 通知，缺失包数量: ${missing_count}"

    spawn_notification_handler "$user" "$uid" "$display" "$wayland" "$title" "$body"
}

# 短期去重，防止开机检测与网络连通性事件连续触发两次
should_skip_recent_run() {
    local last_run now

    [[ ! -f "$RUN_COOLDOWN_FILE" ]] && return 1

    last_run=$(cat "$RUN_COOLDOWN_FILE" 2>/dev/null || echo 0)
    now=$(date +%s)
    (( now - last_run < RUN_COOLDOWN_SEC ))
}

record_run_time() {
    mkdir -p "$(dirname "$RUN_COOLDOWN_FILE")"
    date +%s > "$RUN_COOLDOWN_FILE"
}

main() {
    log "开始语言配置包检测"

    # 无网络时不检测，且不记录去重时间戳，确保连网后 dispatcher 可再次触发
    if ! is_network_available; then
        log "网络不可用，跳过检测"
        exit 0
    fi

    if should_skip_recent_run; then
        log "检测已在 ${RUN_COOLDOWN_SEC}s 内执行过，跳过"
        exit 0
    fi
    record_run_time

    local session_info user uid loginctl_display

    session_info=$(get_active_desktop_session) || {
        log "未找到活跃的图形桌面会话，无法读取用户语言环境"
        exit 0
    }
    read -r user uid loginctl_display <<< "$session_info"

    load_language_config_for_user "$user"

    # 遍历当前用户语言区块中的包列表，收集未安装的包名
    local missing=()
    local pkg
    for pkg in "${CONFIG_PACKAGES[@]}"; do
        [[ -z "$pkg" ]] && continue
        if ! is_package_installed "$pkg"; then
            missing+=("$pkg")
            log "未安装: $pkg"
        fi
    done

    # 全部已安装则静默退出，不弹通知
    if [[ ${#missing[@]} -eq 0 ]]; then
        log "所有语言配置包均已安装，无需通知"
        exit 0
    fi

    log "缺失 ${#missing[@]} 个包: ${missing[*]}"
    # 通知发送失败不影响检测本身，避免服务整体报错
    send_notification "${#missing[@]}" || log "通知流程未完成，但包检测已执行"
}

main "$@"
