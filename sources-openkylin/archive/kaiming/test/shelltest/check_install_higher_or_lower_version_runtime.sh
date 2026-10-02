#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
# - 格式${id}/${version}
#!/bin/bash
set -euo pipefail  # 启用严格模式

if [ $# -ne 2 ]; then
    echo "请提供应用名称和版本作为参数" >&2
    exit 1
fi

architecture=$(arch)
ID="$1"
VERSION="$2"
base_dir="/opt/kaiming/layers/stable/${architecture}/runtime/${ID}/binary"

if [ ! -d "$base_dir" ]; then
    echo "未安装:${ID}" >&2
    exit 1
fi

if [ -d "${base_dir}/${VERSION}" ]; then
    echo "已安装${ID}:${VERSION}" >&2
    exit 0
fi

FIRST_DIGIT=$(echo "$VERSION" | grep -oE '^[0-9]')

# 处理通配符扩展
shopt -s nullglob
MATCHING_DIRS=("${base_dir}/${FIRST_DIGIT}"*)
shopt -u nullglob

if [[ ${#MATCHING_DIRS[@]} -eq 0 ]]; then
    echo "$ID 下不存在以 $FIRST_DIGIT 开头的版本" >&2
    exit 1
fi

# 查找最大版本
MAX_VERSION=""
for dir in "${MATCHING_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        VERSION_PART=$(basename "$dir")
        if [ -z "$MAX_VERSION" ] || \
           [[ $(printf "%s\n%s" "$VERSION_PART" "$MAX_VERSION" | sort -V | tail -n1) == "$VERSION_PART" ]]; then
            MAX_VERSION="$VERSION_PART"
        fi
    fi
done

# 安装新版本
if ! sudo kaiming install -y "$1/$2"; then
    echo "安装失败 ${ID}:${VERSION}" >&2
    exit 1
fi

# 版本比较
if [ -d "${base_dir}/${VERSION}" ]; then
    if [[ $(printf "%s\n%s" "$VERSION" "$MAX_VERSION" | sort -V | tail -n1) == "$VERSION" ]]; then
        echo "$VERSION 大于 $MAX_VERSION"
        if [ ! -d "${base_dir}/${MAX_VERSION}" ]; then
            echo "${ID}已删除旧版本，安装新版本${VERSION}"
        else
            echo "${ID}删除旧版本失败" >&2
            exit 1
        fi
    else
        echo "$VERSION 小于或等于 $MAX_VERSION"
        echo "${ID}已安装低版本${VERSION}，与高版本共存"
    fi
else
    echo "安装后验证失败 ${ID}:${VERSION}" >&2
    exit 1
fi
# sudo kaiming install  -y $1/$2
# if [ $? -ne 0 ]; then
#     echo "安装失败:$1/$2"
#     exit 1
# fi
# base_dir=$(find /opt/kaiming/layers/ -type d -name "$architecture" 2>/dev/null)
# if [ -n "$base_dir" ]; then
#     found_dir=$(find $base_dir -type d -name "$1" 2>/dev/null)
#     if [ -n "$found_dir" ]; then
#         found_version=$(find $found_dir -type d -name "$2" 2>/dev/null)
#         if [ -n "$found_version" ]; then
#             echo "安装成功:$1/$2"
#         else
#             echo "安装失败:$1/$2"
#             exit 1
#         fi
#     else
#         echo "安装失败:$1/$2"
#         exit 1
#     fi
# else
#     echo "安装失败:$1/$2"
#     exit 1
# fi
