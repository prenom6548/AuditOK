#!/bin/bash
# 清理 Debian 构建产物脚本
# 用于清理 debian/ 目录中的临时文件，只保留配置文件

set -e

echo "========================================="
echo "Cleaning Debian build artifacts..."
echo "========================================="
echo ""

DEBIAN_DIR="debian"

if [ ! -d "$DEBIAN_DIR" ]; then
    echo "Error: $DEBIAN_DIR directory not found."
    exit 1
fi

echo "Removing build artifacts from $DEBIAN_DIR/..."

# 删除 debhelper 临时目录
rm -rf "$DEBIAN_DIR/.debhelper"
echo "  ✓ Removed .debhelper/"

# 删除构建时间戳
rm -f "$DEBIAN_DIR/debhelper-build-stamp"
echo "  ✓ Removed debhelper-build-stamp"

# 删除临时文件列表
rm -f "$DEBIAN_DIR/files"
echo "  ✓ Removed files"

# 删除变量替换文件
rm -f "$DEBIAN_DIR"/*.substvars
echo "  ✓ Removed *.substvars"

# 删除包构建临时目录（保留 debian/control 等配置文件）
for dir in "$DEBIAN_DIR"/*/; do
    if [ -d "$dir" ]; then
        dirname=$(basename "$dir")
        # 跳过 source 目录（这是标准配置目录）
        if [ "$dirname" != "source" ]; then
            rm -rf "$dir"
            echo "  ✓ Removed $dirname/"
        fi
    fi
done

echo ""
echo "Remaining files in $DEBIAN_DIR/:"
ls -la "$DEBIAN_DIR/" | grep -v "^total" | grep -v "^\.$" | grep -v "^\.\.$"

echo ""
echo "✅ Cleanup complete!"
echo ""
echo "Standard Debian configuration files preserved:"
echo "  - changelog"
echo "  - compat"
echo "  - control"
echo "  - copyright"
echo "  - postinst/postrm/prerm (if exist)"
echo "  - rules"
echo "  - source/format"
