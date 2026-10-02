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

if [ -L /run/media ]; then
    host_files=$(ls -A /run/media 2>/dev/null)

    container_env=$(script -q -c "kaiming enter "$target" ls -A /run/media" /dev/null)

    if [ "$host_files" == "$container_files" ]; then
        echo "/run/media 宿主机与容器内容一致"
        exit 1
    fi
fi

# 检查 /media 是否为软链接
if [ -L /media ]; then
    # echo "/media 是软链接，指向 $(readlink /media)，跳过内容比对"
    host_files=$(ls -A /media 2>/dev/null)
    container_files=$(kaiming enter "$target" ls -A /media 2>/dev/null)

    if [ "$host_files" == "$container_files" ]; then
        echo "/media 宿主机与容器内容一致"
        exit 1
    fi
fi
echo "双U盘问题测试通过"

kaiming kill $target
echo "end"
