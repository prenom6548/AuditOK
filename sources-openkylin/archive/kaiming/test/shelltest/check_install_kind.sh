#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
if [ $# -ne 2 ]; then
    echo "请提供应用类型和应用名称作为参数"
    exit 1
fi
sudo kaiming install --kind=$1 $2 -y
if [ $? -ne 0 ]; then
    echo "安装$1类型$2失败"
    exit 1
fi
base_dir=("/opt/kaiming/layers/" "$HOME/.local/share/kaiming/layers/")

found_dir=""
for dir in "${base_dir[@]}"; do
    found_dir=$(find "$dir" -type d -name "$2" 2>/dev/null)
    if [ -n "$found_dir" ]; then
        break
    fi
done
echo $found_dir
if [ -n "$found_dir" ]; then
    found_id=$(find "$found_dir" -type d -name "$2" 2>/dev/null)
    echo $found_id
    if [ -n "$found_id" ]; then
        echo "安装$1类型$2成功"
    else
        echo "安装$1类型$2失败"
        exit 1
    fi
else
    echo "安装$1类型的$2失败"
    exit 1
fi
