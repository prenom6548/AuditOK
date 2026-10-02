#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

uid=$(id -u)

if [ $# -ne 1 ]; then
     echo "请提供应用名称作为参数"
    exit 0
fi
target=$1

function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

kaiming run "$target" > /dev/null 2>&1 &

sleep 8
if is_app_running "$target"; then
    echo "应用'$target'成功运行" > /dev/null 2>&1
else
    echo "应用'$target'未运行"
    exit 1
fi

host_files=$(ls -A /run/media | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '[:space:]' 2>/dev/null)
container_files=$(script -q -c "kaiming enter "$target" ls -A /run/media" /dev/null | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '[:space:]')
if [ "$host_files" == "$container_files" ]; then
    echo "/run/media 宿主机与容器内容一致"
else
    echo "/run/media 宿主机与容器内容不一致"
    exit 1
fi
echo "U盘目录测试通过"

kaiming kill $target
echo "end"
