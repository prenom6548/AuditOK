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

if is_app_running "$target"; then
    echo "应用'$target'已经运行" > /dev/null 2>&1
else

    kaiming run "$target" > /dev/null 2>&1 &

    sleep 8
    if is_app_running "$target"; then
        echo "应用'$target'成功运行" > /dev/null 2>&1
    else
        echo "应用'$target'未运行"
        exit 1
    fi
fi 

container_env=$(script -q -c "kaiming enter "$target" env" /dev/null)

# 检查 PATH 是否包含 /usr/libexec 和 /usr/games
path_value=$(echo "$container_env" | grep '^PATH=')
if echo "$path_value" | grep -q "/usr/libexec" && echo "$path_value" | grep -q "/usr/games"; then
    echo "PATH 包含 /usr/libexec 和 /usr/games"
else
    echo "PATH 不完全包含 /usr/libexec 和 /usr/games"
    kaiming kill $target
    exit 1
fi

kaiming kill $target
echo "end"