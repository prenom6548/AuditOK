#!/bin/bash

trap ':' INT

if [ $# -eq 0 ]; then
    echo "参数不能为0,请提供完整参数"
    exit 0
fi

appid=$1
isSystemd=$2

variable_params=("${@:3}")

# 检查 systemd-run 是否存在
has_systemd_run=0
if command -v systemd-run >/dev/null 2>&1; then
    has_systemd_run=1
fi

# 判断当前是否为普通用户
if [ "$EUID" -ne 0 ] && [ "$isSystemd" -eq 1 ] && [ "$has_systemd_run" -eq 1 ]; then
    # 普通用户使用 systemd-run --user
    systemd-run --quiet --user --scope -- "${variable_params[@]}"
    ret=$?
else
    # root 用户直接执行
    "${variable_params[@]}"
    ret=$?
fi

# 判断是否有后台进程
while true; do
    num=$(lsns -n -t mnt | grep "${appid}" | awk 'NR==1 {print $3}')
    if [ "$num" -gt 5 ]; then
        sleep 1
    else
        break
    fi
done

echo "kaiming application exit code: $ret"
exit $ret
