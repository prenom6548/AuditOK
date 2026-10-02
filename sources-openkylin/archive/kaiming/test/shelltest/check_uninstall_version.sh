#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
if [ $# -ne 2 ]; then
    echo "请提供应用名称和版本作为参数"
    exit 1
fi
architecture=$(arch)
ID=$1
version=$2
base_dir="/opt/kaiming/layers/stable/${architecture}/app/${ID}/binary"

if [ -d "${base_dir}/$version" ]; then
        echo "${ID}已安装${$version}" 
        exit 1
    else
        echo "${ID}未安装该应用{$version}版本"
        exit 1
fi

sudo kaiming uninstall  -y $ID/$version
if [ $? -ne 0 ]; then
    echo "卸载失败:$ID/$version"
    exit 1
fi

if [ -d "${base_dir}/${version}" ]; then
    echo "${ID}卸载失败" 
    exit 1
else
    echo "${ID}:${version}已卸载"
fi