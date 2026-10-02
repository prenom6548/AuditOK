#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

username=$(whoami)
if [ $# -ne 2 ]; then
     echo "请提供应用名称+-d[--devel]作为参数"
    exit 0
fi
target=$1
devel=$2
base_dir="/home/$username/.local/share/kaiming/layers/stable/x86_64/base/top.openkylin.base/devel/1.0.0"
runtime_dir="/home/$username/.local/share/kaiming/layers/stable/x86_64/runtime/top.openkylin.ukui/devel/1.0.0"

function is_app_running() {
    local target="$1"
    # 只匹配第一列（appid）
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
} 

if [ ! -d "$base_dir" ]; then
    echo "找不到 base_dir: $base_dir"
    exit 1
fi

if [ ! -d "$runtime_dir" ]; then
    echo "找不到 runtime_dir: $runtime_dir"
    exit 1
fi

if is_app_running "$target"; then
    echo "应用'$target'已经运行，请输入其他应用名称"
    exit 0  
fi 

kaiming run $devel $target  > /dev/null 2>&1 &
sleep 8
# 检查应用是否运行
if is_app_running "$target"; then
    echo "应用'$target'正在运行" > /dev/null 2>&1 
else
    echo "应用'$target'未运行"
    exit 1
fi

# 验证是否进入容器后能找到 gdb（即是否是开发环境）

container_env=$(script -q -c "kaiming enter "$target" which gdb" /dev/null)

if [[ -n "$output" ]]; then
    echo "应用运行在开发模式"
else
    echo "应用未运行在开发模式"
    kaiming kill $target
    exit 1
fi
kaiming kill $target



