#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#
#!/bin/bash

if [ $# -ne 1 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi

target="$1"

function is_app_running() {
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

# 获取宿主机当前工作目录
original_cwd=$(pwd)

echo "启动应用 '$target'..."
kaiming run "$target" > /dev/null 2>&1 &
sleep 8

if is_app_running "$target"; then
    echo "应用 '$target' 启动成功"
else
    echo "应用 '$target' 启动失败"
    exit 1
fi

touch "1.txt"
echo "test content" > "1.txt"
path="file:///$original_cwd/1.txt"
echo "打开文件 '$path'"
container_content=$(script -q -c "kaiming enter "$target" cat $path" /dev/null)
echo $container_content

if grep -q "test content" "1.txt"; then
    echo "测试通过:'1.txt'文件内容正确"
else
    echo "测试不通过：'1.txt'文件内容缺失或不符合预期"
    kaiming kill $target
    exit 1
fi
rm "1.txt"
kaiming kill $target