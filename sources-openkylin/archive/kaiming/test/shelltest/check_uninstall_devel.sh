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
id=$1   
architecture=$(arch)

sudo kaiming uninstall -y $id//devel
if [ $? -ne 0 ]; then
    echo "${id}卸载失败"
    exit 1
fi
home="$HOME"
base_dir="${home}/.local/share/kaiming/layers/stable/${architecture}/app/${id}/devel"
echo "$base_dir"
if [ -d "$base_dir" ]; then
        echo "卸载成功:$id"
    else
        echo "卸载失败:$id"
        exit 1
fi
