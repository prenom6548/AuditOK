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

if [[ $filename =~ ^([^[:space:]/]+)-([a-zA-Z0-9_]+)-([^[:space:]/]+)-([a-zA-Z_]+)-([a-zA-Z_]+)-squashfs\.ok$ ]]; then
    # 将匹配结果存入数组
    result=(
        "${BASH_REMATCH[1]}"  # id
    )
else
    echo "Error: Filename format does not match expected pattern."
fi

if kaiming changelog "${filename}" | grep "BUG"; then
    echo "${result[0]}changelog执行成功"
else
    echo "${result[0]}}changelog执行失败"
    exit 1
fi