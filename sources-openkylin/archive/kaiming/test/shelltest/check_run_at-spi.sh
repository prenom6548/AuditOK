#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

uid=$(id -u)

if [ $# -ne 1 ]; then
     echo "请提供应用名称作为参数"
    exit 0
fi
target=$1

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


container_env=$(script -q -c "kaiming enter "$target" env" /dev/null)

if echo "$container_env" | grep -q "QT_ACCESSIBILITY=1" && \
   echo "$container_env" | grep -q "QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1"; then
    echo " QT_ACCESSIBILITY 和 QT_LINUX_ACCESSIBILITY_ALWAYS_ON 环境变量设置正确"
else
    echo "缺少必要的 QT 可访问性环境变量"
    kaiming kill "$target"
    exit 1
fi

container_spi=$(script -q -c "kaiming enter "$target" ls /run/user/$uid/at-spi" /dev/null)


if [ -n "$container_spi" ]; then
    echo "/run/user/$uid/at-spi 目录存在"
else
    echo "/run/user/$uid/at-spi 目录不存在"
    kaiming kill "$target"
    exit 1
fi
echo "所有条件满足:QT可访问性变量已设置:且 at-spi 目录存在"
kaiming kill $target
echo "end"
