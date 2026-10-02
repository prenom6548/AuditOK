#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
# - 格式${id}/${version}
if [ $# -ne 2 ]; then
    echo "请提供应用名称和版本作为参数"
    exit 1
fi
architecture=$(arch)
sudo kaiming install  -y $1/$2
if [ $? -ne 0 ]; then
    echo "安装失败:$1/$2"
    exit 1
fi
base_dir=$(find /opt/kaiming/layers/ -type d -name "$architecture" 2>/dev/null)
if [ -n "$base_dir" ]; then
    found_dir=$(find $base_dir -type d -name "$1" 2>/dev/null)
    if [ -n "$found_dir" ]; then
        found_version=$(find $found_dir -type d -name "$2" 2>/dev/null)
        if [ -n "$found_version" ]; then
            echo "安装成功:$1/$2"
        else
            echo "安装失败:$1/$2"
            exit 1
        fi
    else
        echo "安装失败:$1/$2"
        exit 1
    fi
else
    echo "安装失败:$1/$2"
    exit 1
fi
