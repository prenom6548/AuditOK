#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
uid=$(id -u)

# if [ $# -ne 1 ]; then
#      echo "请提供应用名称作为参数"
#     exit 0
# fi
target="org.mate-desktop.pluma"
echo $target

function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

kaiming run "$target" > /dev/null 2>&1 &

sleep 8
if is_app_running "$target"; then
    echo "应用'$target'成功运行" > /dev/null 2>&1
else
    echo "应用'$target'未运行"
    exit 1
fi
touch "1.txt"
echo "test content" > "1.txt"
kaiming enter "$target" pluma '1.txt'

if grep -q "test content" "1.txt"; then
    echo "测试通过:'1.txt'文件内容正确"
else
    echo "测试不通过：'1.txt'文件内容缺失或不符合预期"
    kaiming kill $target
    exit 1
fi
rm "1.txt"
kaiming kill $target