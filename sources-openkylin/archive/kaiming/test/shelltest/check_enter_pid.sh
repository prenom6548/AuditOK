#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

if [ $# -eq 0 ]; then
     echo "请提供应用名称作为参数"
    exit 0
fi
target=$1

function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

if is_app_running "$target"; then
    echo "应用'$target/'已经运行，请输入其他应用名称和版本"
    exit 0  
fi 

kaiming run "$target" > /dev/null 2>&1 &
sleep 20

if ! is_app_running "$target"; then
    echo "应用'$target'未运行" 
    exit 1
fi

pid=$(kaiming ps | tail -n +2 | awk -v app="$target" '$1 == app {print $3}')
container_env=$(script -q -c "kaiming enter "$pid" env" /dev/null)

if echo "$container_env" | grep -q "APP_VERSION"; then
    echo "成功进入容器"
else
    echo "未进入容器"
    kaiming kill $target
    exit 1
fi

kaiming kill $target
echo "end"
