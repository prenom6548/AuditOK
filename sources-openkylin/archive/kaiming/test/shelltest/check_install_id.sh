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
architecture=$(arch)
app_id=$1
sudo kaiming install  $1 -y
if [ $? -ne 0 ]; then
    echo "安装失败:$1"
    exit 1
fi
if [[ "$1" == *"ok"* ]]; then
    filename=$(basename "$1" | tr -d '\0') # 提取文件名
    app_id=$(echo "$filename" | sed 's/\(-x86_64.*\)//')
fi
found_dir=$(find /opt/kaiming/layers/ -type d -name "$app_id" 2>/dev/null)
if [ -n "$found_dir" ]; then
    echo "安装成功:$1"
else
    echo "安装失败:$1"
    exit 1
fi
