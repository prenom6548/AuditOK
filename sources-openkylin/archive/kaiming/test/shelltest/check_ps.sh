#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | grep -q "$target"
}

# 检查参数
if [ $# -ne 1 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi

target="$1"
kaiming run "$target" > /dev/null 2>&1 &
sleep 20

# 检查目标是否运行
if ! is_app_running "$target"; then
    echo "没有运行该目标：$target"
    exit 1
fi

if kaiming ps | tail -n +2 | awk -v app="$target" '$1 == app {found=1}'; then
    echo "执行成功"
else
    echo "执行失败"
    kaiming kill "$target"
    exit 1
fi

# 执行 kill
kaiming kill "$target"
echo "end"

