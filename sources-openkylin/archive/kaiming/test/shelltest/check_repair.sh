#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash  
# 检查输入参数
if [ $# -eq 0 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi
id=$1

architecture=$(arch)

base_dir="/opt/kaiming/layers/stable/${architecture}"
found_dir=$(find $base_dir -type d -name "$id" 2>/dev/null)
if [ -n "$found_dir" ]; then
    found_id=$(find $found_dir -type d -name "files" 2>/dev/null)
    if [ -n "$found_id" ]; then
        sudo rm -rf $found_id
        rm_status=$?  
        if [ $rm_status -ne 0 ]; then
            echo "警告: 删除文件失败 (状态码: $rm_status)"
        fi
    fi
    if ! sudo kaiming repair $id; then
        echo "错误: kaiming repair 命令执行失败"
        exit 1
    fi
    found_id=$(find $found_dir -type d -name "files" 2>/dev/null)
    if [ -d "$found_id" ]; then
        echo "修复成功"
    else
        echo "修复失败"
        exit 1
    fi
else
    echo "未安装${architecture}架构的${id}"
    exit 1
fi