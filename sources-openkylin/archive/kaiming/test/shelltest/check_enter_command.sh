#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

uid=$(id -u)

if [ $# -ne 2 ]; then
     echo "请提供应用名称+env(指定command=env)作为参数"
    exit 0
fi
target=$1
command=$2

if [ $command != "env" ]; then
    echo "请提供env作为参数"
    exit 0
fi
function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

if is_app_running "$target"; then
    echo "应用'$target'已经运行" > /dev/null 2>&1
else

    kaiming run "$target" > /dev/null 2>&1 &

    sleep 20
    if is_app_running "$target"; then
        echo "应用'$target'成功运行" > /dev/null 2>&1
    else
        echo "应用'$target'未运行"
        exit 1
    fi
fi 

container_env=$(script -q -c "kaiming enter "$target" $command" /dev/null)

if echo "$container_env" | grep -q "APP_VERSION"; then
    echo "command命令执行成功"
else
    echo "command命令执行失败"
    kaiming kill $target
    exit 1
fi

kaiming kill $target
echo "end"
