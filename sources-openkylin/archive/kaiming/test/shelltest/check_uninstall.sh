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
    exit 1
fi
# 要检查的父目录
parent_dirs=("/opt/kaiming/layers" "$HOME/.local/share/kaiming/layers")

# 要查找的目标目录名称
target_dir="$1"

# 使用find命令查找第4层目录中是否存在目标目录
for parent_dir in "${parent_dirs[@]}"; do
    if [ ! -d "$parent_dir" ]; then
        echo "警告: 路径不存在 -> $base_dir"
        continue
    fi
    if find "$parent_dir" -mindepth 4 -maxdepth 4 -type d -name "$target_dir" | grep -q .; then
        sudo kaiming uninstall -y ${target_dir}
    else
        continue
    fi
    if find "$parent_dir" -mindepth 4 -maxdepth 4 -type d -name "$target_dir" | grep -q .; then
        echo "卸载失败"
        exit 1
    else
        echo "成功卸载${target_dir}"
    fi
done 
