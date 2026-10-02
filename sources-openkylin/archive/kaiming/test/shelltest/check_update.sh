#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

# 检查是否提供了参数
if [ $# -eq 0 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi
# 获取应用名称
update_name="$1"
old_version=""
new_version=""
# 要检查的父目录
parent_dir="/opt/kaiming/layers/stable"

# 使用find命令查找第4层目录中是否存在目标目录
found_dir=$(find ${parent_dir} -type d -name "${update_name}" 2>/dev/null)
if [ -n "$found_dir" ]; then
    # 先保存find的结果到变量中
    deep_dir=$(find "$found_dir" -mindepth 2 -maxdepth 2 -type d | head -n 1)
    if [ -n "$deep_dir" ]; then
        old_version=$(basename "$deep_dir")
    else
        echo "未安装${update_name}"
        exit 1
    fi
else
    echo "未安装${update_name}"
    exit 1
fi

sudo kaiming update -y $update_name 
if [ $? -eq 0 ]; then
    found_dir=$(find /opt/kaiming/layers/stable/ -type d -name "${update_name}" 2>/dev/null)
    if [ -n "$found_dir" ]; then
        # 先保存find的结果到变量中
        deep_dir=$(find "$found_dir" -mindepth 2 -maxdepth 2 -type d | head -n 1)
        if [ -n "$deep_dir" ]; then
            new_version=$(basename "$deep_dir")
            if [ "$old_version" == "$new_version" ]; then
                echo "版本未更新"
            else
                # 使用 sort -V 比较版本号
                if printf "%s\n%s\n" "$old_version" "$new_version" | sort -V | head -n1 | grep -q "^$old_version$"; then
                    echo "更新成功：$old_version -> $new_version"
                fi
            fi
        fi
    else
        echo "未安装${update_name}"
        exit 1
    fi
fi
