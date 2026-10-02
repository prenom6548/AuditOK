#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

DOWNLOAD_URL="http://172.30.60.71/package/bak0905/shelltest/top.openkylin.kylin-calculator-x86_64-1.2.0.0-ok6.15k0.10-stable-binary-squashfs.ok"

# 使用 wget 下载
wget "$DOWNLOAD_URL"

# 检查是否下载成功
if [ $? -eq 0 ]; then
    echo "✅ 下载成功: $DOWNLOAD_URL"
else
    echo "❌ 下载失败: $DOWNLOAD_URL"
    exit 1
fi

filename=$(basename "$DOWNLOAD_URL")

# 使用PCRE正则表达式匹配（需要grep -P支持）
if [[ $filename =~ ^([^[:space:]/]+)-([a-zA-Z0-9_]+)-([^[:space:]/]+)-([a-zA-Z_]+)-([a-zA-Z_]+)-squashfs\.ok$ ]]; then
    # 将匹配结果存入数组
    result=(
        "${BASH_REMATCH[1]}"  # id
        "${BASH_REMATCH[2]}"  # arch
        "${BASH_REMATCH[3]}"  # version
        "${BASH_REMATCH[4]}"  # channel
        "${BASH_REMATCH[5]}"  # module
    )
else
    echo "Error: Filename format does not match expected pattern."
fi

sudo kaiming install  $filename -y
if [ $? -ne 0 ]; then
    echo "安装失败:$filename"
    exit 1
fi

id=${result[0]}
echo $id
found_dir=$(find /opt/kaiming/layers/stable -type d -name "${result[0]}" 2>/dev/null)
if [ -n "$found_dir" ]; then
    echo "安装成功:$filename"
else
    echo "安装失败:$filename"
    exit 1
fi
