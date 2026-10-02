#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
uid=$(id -u)
host_name=$(hostname)

target="org.mate-desktop.pluma"
echo $target

function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

kaiming run "$target" > /dev/null 2>&1 &

sleep 20
if is_app_running "$target"; then
    echo "应用'$target'成功运行" > /dev/null 2>&1
else
    echo "应用'$target'未运行"
    exit 1
fi

file_path="/home/$host_name/au dd/1.txt"

mkdir -p "$(dirname "$file_path")"

echo "test content" > "$file_path"
kaiming enter $target xdg-open "$file_path" &> /dev/null &


if grep -q "test content" "$file_path"; then
    echo "测试通过:xdg-open 打开文件内容正确"
else
    echo "测试不通过:xde-open 打开的文件内容错误"
    kaiming kill $target
    exit 1
fi
rm -rf "$(dirname "$file_path")"
kaiming kill $target