#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
#- 格式${id}/${version}/${module}/${channel}/${arch} ：完整格式。
if [ $# -ne 5 ]; then
    echo "请提供应用名称、版本、模块、渠道、架构作为参数"
    exit 1
fi
id=$1
version=$2
module=$3
channel=$4
arch=$5
sudo kaiming install  -y $id/$version/$module/$channel/$arch
if [ $? -ne 0 ]; then
    echo "安装失败:$id/$version/$module/$channel/$arch"
    exit 1
fi
base_dir=/opt/kaiming/layers/$channel/$arch/
if [ -n "$base_dir" ]; then
    found_dir=$(find $base_dir -type d -name "$id" 2>/dev/null)
    if [ -n "$found_dir" ]; then
        found_version=$found_dir/$module/$version
        if [ -n "$found_version" ]; then
            echo "安装成功:$id/$version/$module/$channel/$arch"
        else
            echo "安装失败:$id/$version/$module/$channel/$arch"
            exit 1
        fi
    else
        echo "安装失败:$id/$version/$module/$channel/$arch"
        exit 1
    fi
else
    echo "安装失败:$id/$version/$module/$channel/$arch"
    exit 1
fi
