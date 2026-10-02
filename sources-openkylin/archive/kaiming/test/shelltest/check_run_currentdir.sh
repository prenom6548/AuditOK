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

# 获取容器内当前工作目录

container_env=$(script -q -c "kaiming enter "$target" pwd | tr -d '[:space:]'" /dev/null)
exit_code=$?

if [ $exit_code -ne 0 ]; then
    echo "❌ 无法进入容器获取工作目录，请确认 kaiming enter 命令可用"
    kaiming kill "$target"
    exit 1
fi

# 比较目录
if [ "$original_cwd" = "$container_cwd" ]; then
    echo "容器工作目录与宿主机一致"
else
    echo "容器工作目录与宿主机不一致"
    kaiming kill "$target"
    exit 1
fi

kaiming kill "$target"