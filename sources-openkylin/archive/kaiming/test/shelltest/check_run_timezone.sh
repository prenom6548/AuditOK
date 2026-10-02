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

container_env=$(script -q -c "kaiming enter "$target" ls /usr/share/zoneinfo/Asia/Shanghai" /dev/null)

if echo "$container_env" | grep -q "/usr/share/zoneinfo/Asia/Shanghai"; then
    echo "系统时间挂载参数执行成功"
else
    echo "系统时间挂载参数执行失败"
    kaiming kill $target
    exit 1
fi

kaiming kill $target
echo "end"
