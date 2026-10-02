#!/bin/bash

#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

KAIMING_VERSION=""
VERSION_NUMBER=""

# 重置/opt/kaiming/repo/config配置
if [ -f "/opt/kaiming/repo/config" ]; then
    cat << EOF > /opt/kaiming/repo/config
[core]
repo_version=1
mode=bare-user
min-free-space-size=500MB

[remote "kaiming-repo"]
gpg-verify=false
gpg-verify-summary=false
url=https://archive2.kylinos.cn/KM/repo-v11
EOF
fi

if command -v dpkg-query >/dev/null 2>&1; then
    KAIMING_VERSION=$(dpkg-query -W -f='${Version}' kaiming 2>/dev/null)

    if [ -z "$KAIMING_VERSION" ] || [ "$KAIMING_VERSION" = "(无)" ]; then  
        exit 0
    fi
 
    VERSION_NUMBER=$(echo "$KAIMING_VERSION" | awk -F'.' '{print $1}')  

    # 判断是否成功提取实际版本号
    if [ -z "$VERSION_NUMBER" ]; then
        exit 0
    fi
fi

if [ ! -e /run/dbus/system_bus_socket ]; then 
    exit 0
fi

# 获取活动用户
if command -v loginctl >/dev/null; then
    # 更精确的过滤逻辑：包含 seat 且状态为 active
    session_id=$(loginctl list-sessions --no-legend | awk '/seat/ && $6 == "active" {print $1; exit}')
    if [ -n "$session_id" ]; then
        user=$(loginctl show-session -p Name --value "$session_id")
        type=$(loginctl show-session -p Type --value $session_id)
        uid=$(loginctl show-user $user -p UID --value)
    fi
fi

# 判断桌面环境
if [ "$type" != "x11" ] && [ "$type" != "wayland" ]; then 
    exit 0 
fi

# 获取dbus address
if [ -n "$uid" ]; then
    dbus_addr="unix:path=/run/user/$uid/bus"
    addr="/run/user/$uid/bus"
    if [ ! -e $addr ]; then
        exit 0
    fi
fi

# 检查notify-send是否存在
if ! command -v notify-send >/dev/null; then
    exit 0
fi

# 比较版本号是否小于 1.0
if awk "BEGIN {exit !($VERSION_NUMBER < 1.0)}"; then
    sudo -u $user DBUS_SESSION_BUS_ADDRESS="$dbus_addr" \
    notify-send "Kaiming Version Check" "开明版本已升级，请将开明应用重新下载"
fi

exit 0
