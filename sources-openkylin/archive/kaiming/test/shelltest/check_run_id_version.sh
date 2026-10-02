#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
# - 格式${id}/${version}
function is_app_running() {
    local target="$1"
    # 只匹配第一列（appid）
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target"
}

if [ $# -ne 2 ]; then
    echo "请提供应用名称和版本作为参数"
    exit 1
fi
target=$1
version=$2
if is_app_running "$target"; then
    echo "应用'$target/$version'已经运行，请输入其他应用名称和版本"
    exit 0  
fi 


kaiming run $target/$version > /dev/null 2>&1 &
sleep 20
if [ $? -ne 0 ]; then
    echo "运行失败:$target/$version"
    exit 1
fi
# 检查应用是否运行
if is_app_running "$target"; then
    echo "应用'$target/$version'正在运行"
else
    echo "应用'$target/$version'未运行"
    exit 1
fi
kaiming kill $target
echo "end"