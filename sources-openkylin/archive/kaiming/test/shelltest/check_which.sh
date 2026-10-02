#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

if [ $# -ne 1 ]; then
    echo "请提供文件路径作为参数"
    exit 1
fi

path="$1"
arch=$(uname -m)

output=$(kaiming which "$path")

if [[ "$output" == "No package contains the file: "* ]]; then
    echo "未找到包含该文件的包"
    exit 1
fi

appid=$(echo "$output" | awk -F': ' '{print $1}')

base_dir="/opt/kaiming/layers/stable/$arch/app/$appid"

normalized_path=$(basename "$path")

if find "$base_dir" -type f -name "$normalized_path" | grep -q .; then
    echo "文件查找成功"
else
    echo "文件查找失败"
    exit 1
fi