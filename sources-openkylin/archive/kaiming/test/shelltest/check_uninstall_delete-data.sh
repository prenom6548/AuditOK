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

if [ $# -eq 0 ]; then
    echo "请提供应用名称+--delete-data作为参数"
    exit 1
fi

# 要检查的父目录
parent_dir="/opt/kaiming/layers"
userdata_dir="$HOME/.var/app"
# 要查找的目标目录名称
target_dir="$1"


if ! find "$parent_dir" -mindepth 1 -maxdepth 1 -type d -name "$target_dir" | grep -q .; then
    kaiming run "$target_dir" > /dev/null 2>&1 &
    sleep 20

# 步骤一：检查目标是否运行
    if ! is_app_running "$target"; then
        echo "没有运行该目标：$target"
        exit 1
    fi
    kaiming kill "$target_dir"
fi
echo "已运行${target_dir}"
# 使用find命令查找第4层目录中是否存在目标目录
if find "$parent_dir" -mindepth 4 -maxdepth 4 -type d -name "$target_dir" | grep -q .; then
    sudo kaiming uninstall -y --delete-data ${target_dir}
else
    echo "未安装${target_dir}"
    exit 1
fi
if find "$parent_dir" -mindepth 4 -maxdepth 4 -type d -name "$target_dir" | grep -q .; then
    echo "卸载失败"
    exit 1 
fi

if find "$parent_dir" -mindepth 1 -maxdepth 1 -type d -name "$target_dir" | grep -q .; then
    echo "卸载失败"
    exit 1 
fi
echo "成功卸载${target_dir}"
