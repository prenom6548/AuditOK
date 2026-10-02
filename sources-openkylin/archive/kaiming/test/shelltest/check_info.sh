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
app_name="$1"
# 要检查的父目录
parent_dir="/opt/kaiming/layers"
# 使用 kaiming info 获取信息并提取 Id
info_output=$(kaiming info $app_name| sed 's/\x1B\[[0-9;]*[mGK]//g')
id_value=$(echo "$info_output" | awk 'NR==2 {print $2}')

if [ "$app_name" = "$id_value" ]; then
    # 使用find命令查找第4层目录中是否存在目标目录
    if find "$parent_dir" -mindepth 4 -maxdepth 4 -type d -name "$app_name" | grep -q .; then
        echo "已安装${app_name}"
    else
        echo "未安装${app_name}"
        exit 1
    fi
else
    echo "未安装${app_name}"
    exit 1
fi
