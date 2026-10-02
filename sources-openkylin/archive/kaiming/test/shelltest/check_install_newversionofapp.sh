#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

#!/bin/bash

if [ $# -ne 2 ]; then
    echo "请提供应用名称和版本作为参数"
    exit 1
fi

ID=$1
TARGET_VERSION=$2
architecture=$(arch)
base_dir="/opt/kaiming/layers/stable/${architecture}/app/${ID}/binary"

# 获取所有版本目录并找出最大版本
MAX_VERSION=""
if [ -d "$base_dir" ]; then
    for dir in "$base_dir"/*; do
        if [ -d "$dir" ]; then
            VERSION_PART=$(basename "$dir")
            if [ -z "$MAX_VERSION" ] || \
               [[ $(printf "%s\n%s" "$VERSION_PART" "$MAX_VERSION" | sort -V | tail -n1) == "$VERSION_PART" ]]; then
                MAX_VERSION="$VERSION_PART"
            fi
        fi
    done
fi

if [ -z "$MAX_VERSION" ]; then
    echo "未安装该应用"
    exit 1
fi

# 版本比较
if [ "$MAX_VERSION" == "$TARGET_VERSION" ]; then
    echo "已安装该版本"
i    exit 0
fi

# 安装新版本
echo "正在安装新版本 $TARGET_VERSION..."
sudo kaiming install -y "$ID/$TARGET_VERSION"
if [ $? -ne 0 ]; then
    echo "安装失败: $ID/$TARGET_VERSION"
    exit 1
fi

# 验证安装结果
if [ ! -d "${base_dir}/${TARGET_VERSION}" ]; then
    echo "安装失败: $ID/$TARGET_VERSION"
    exit 1
fi

# 删除旧版本（可选）
if [ -d "${base_dir}/${MAX_VERSION}" ]; then
    echo "正在删除旧版本 $MAX_VERSION..."
    sudo rm -rf "${base_dir}/${MAX_VERSION}"
    if [ $? -ne 0 ]; then
        echo "警告: 删除旧版本失败，但不影响新版本使用"
    fi
fi

echo "安装成功: $ID/$TARGET_VERSION"
exit 0
