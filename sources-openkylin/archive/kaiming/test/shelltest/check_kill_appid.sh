#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

# 定义检查函数：判断是否存在某个应用/容器ID/进程ID
function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | grep -q "$target"
}

function get_pid() {
    local target="$1"
    # 只检查第一列（appid）是否匹配
    kaiming ps | tail -n +2 | awk '{print $3}' | grep -q "^$target$"
}

# 检查参数
if [ $# -ne 1 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi

target="$1"
kaiming run "$target" > /dev/null 2>&1 &
sleep 10

# 检查目标是否运行
if ! is_app_running "$target"; then
    echo "没有运行该目标：$target"
    exit 1
fi

appid=$(script -q -c "kaiming ps" /dev/null | tail -n +2 | awk -v app="$target" '$1 == app {print $1}')

# 执行 kaiming kill
kaiming kill "$appid"
echo "end"
if [ $? -ne 0 ]; then
    echo " kaiming kill 执行失败"
    exit 1
fi

sleep 2 

if is_app_running "$target"; then
    echo "kill 失败"
    exit 1
else
    echo "应用已成功被杀死"
    exit 0
fi
                                   