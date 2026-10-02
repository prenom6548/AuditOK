#!/bin/bash

set -e

BIN_DIR="/opt/kaiming-tools/bin"
LIB_DIR="/opt/kaiming-tools/lib/kaiming-builder-gui"
BIN_DIR_DEBIAN="debian/kaiming-builder-gui/opt/kaiming-tools/bin"
LIB_DIR_DEBIAN="debian/kaiming-builder-gui/opt/kaiming-tools/lib/kaiming-builder-gui"
KAIMING_BUILDER_GUI_EXEC="debian/kaiming-builder-gui/opt/kaiming-tools/bin/kaiming-builder-gui"

mkdir -p "$BIN_DIR_DEBIAN" "$LIB_DIR_DEBIAN"

# 复制单个库及其软链接结构
copy_single_lib() {
    local lib="$1"
    if [ ! -e "$lib" ]; then
        echo " 跳过无效库路径: $lib"
        return
    fi

    local base
    base=$(basename "$lib")

    # 特殊处理动态链接器 ld-linux-*.so*
    if [[ "$base" == ld-linux*.so* ]]; then
        # 解析真实路径，只复制实际文件，不保留软链接
        local real_path
        real_path=$(readlink -f "$lib")
        cp "$real_path" "$LIB_DIR_DEBIAN/"
        return
    fi

    local dir
    dir=$(dirname "$lib")

    # 用通配符找出所有相关版本，比如 libssl.so libssl.so.1.1 libssl.so.3
    local matches=("$dir"/"$base"*)

    for match in "${matches[@]}"; do
        local target="$LIB_DIR_DEBIAN/$(basename "$match")"
        if [ ! -e "$target" ]; then
            cp -Pp "$match" "$LIB_DIR_DEBIAN/"
        fi
    done
}

# 分析依赖库并复制
copy_deps() {
    local exe="$1"
    echo " 分析依赖: $exe"

    ldd "$exe" | awk '{print $3}' | grep '^/' | sort -u | while read -r lib; do
        copy_single_lib "$lib"
    done
}

# 设置 RPATH 和 interpreter 的函数
set_interpreter_and_rpath() {
    local exe="$1"

    if file "$exe" | grep -q 'statically linked'; then
        echo " 静态链接，跳过: $exe"
        return
    fi

    # 设置 RPATH
    patchelf --set-rpath "$LIB_DIR" "$exe"

    # 获取原始 interpreter 路径
    local interp
    interp=$(patchelf --print-interpreter "$exe" 2>/dev/null || true)

    if [ -n "$interp" ]; then
        local interp_name
        interp_name=$(basename "$interp")
        local target_interpreter="$LIB_DIR/$interp_name"

        # 设置 interpreter 为我们目录下的那个
        if [ "$interp" != "$target_interpreter" ]; then
            patchelf --set-interpreter "$target_interpreter" "$exe"
        fi
    else
        echo " 没有找到 interpreter: $exe"
    fi
}

# 复制可执行文件依赖
copy_deps "$KAIMING_BUILDER_GUI_EXEC"

# 额外复制动态链接器（例如 ld-linux-x86-64.so.2）
interpreter=$(readelf -l "$KAIMING_BUILDER_GUI_EXEC" | grep 'Requesting program interpreter' | awk -F': ' '{print $2}' | tr -d '[]')
if [ -n "$interpreter" ]; then
    echo "检测到动态链接器: $interpreter"
    copy_single_lib "$interpreter"
fi

# 为 .so 库设置 RPATH（排除 ld-linux-*.so*）
echo " 正在为 .so 库设置 RPATH..."
find "$LIB_DIR_DEBIAN" -type f -name "*.so*" ! -name "ld-linux*.so*" ! -name "libicudata*.so*" | while read -r sofile; do
    patchelf --set-rpath "$LIB_DIR" "$sofile"
done

# 修复所有可执行文件的 RPATH 和 interpreter
echo " 正在修复执行文件..."
for bin in "$BIN_DIR_DEBIAN"/*; do
    if file "$bin" | grep -q 'ELF .* executable'; then
        set_interpreter_and_rpath "$bin"
    fi
done

echo " 所有 RPATH 和 interpreter 设置完毕"
