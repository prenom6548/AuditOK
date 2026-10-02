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
# architecture=$(arch)
sudo kaiming install --compatibility $1
if [ $? -ne 0 ]; then
    echo "安装失败:$1"
    exit 1
fi
found_dir=$(find /opt/kaiming/layers/ -type d -name "$1" 2>/dev/null)
if [ -n "$found_dir" ]; then
    echo "安装成功:$1"
else
    echo "安装失败:$1"
    exit 1
fi
