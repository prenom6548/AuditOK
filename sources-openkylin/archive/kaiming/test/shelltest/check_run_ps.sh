#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

if [ $# -ne 1 ]; then
    echo "请提供应用名称作为参数"
    exit 0
fi
target=$1

function is_app_running() {
    local target="$1"
    # 检查 kaiming ps 输出中是否存在 error 或 annotations
    if kaiming ps | grep -E 'error|annotations' > /dev/null; then
        echo "查看服务类运行状态失败"
        kaiming kill $target
        exit 1
    fi

    # 检查目标应用是否在运行
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

kaiming run "$target" > /dev/null 2>&1 &

sleep 20
if is_app_running "$target"; then
    echo "应用'$target'成功运行" > /dev/null 2>&1
else
    echo "应用'$target'未运行"
    exit 1
fi
echo "查看服务类运行状态成功"
kaiming kill $target
echo "end"
