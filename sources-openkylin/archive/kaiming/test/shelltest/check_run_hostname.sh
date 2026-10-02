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

# 获取宿主机主机名并清理 ANSI 转义序列和空白字符
host_name=$(hostname | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '[:space:]')
echo "宿主机主机名: $host_name"

container_name=$(script -q -c "kaiming enter $target hostname" /dev/null | sed 's/\x1b\[[0-9;]*[a-zA-Z]//g' | tr -d '[:space:]')
echo "容器主机名: $container_name"

if [ "$host_name" == "$container_name" ]; then
    echo "主机名 宿主机与容器内容一致"
else
    echo "主机名 宿主机与容器内容不一致"
    kaiming kill $target
    exit 1
fi
echo "主机名测试通过"

kaiming kill $target
echo "end"
