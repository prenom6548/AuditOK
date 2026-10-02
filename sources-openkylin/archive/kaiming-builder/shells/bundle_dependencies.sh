#!/bin/bash

set -e

BIN_DIR="/opt/kaiming-tools/bin"
LIB_DIR="/opt/kaiming-tools/lib"
BIN_DIR_DEBIAN="debian/kaiming-builder/opt/kaiming-tools/bin"
LIB_DIR_DEBIAN="debian/kaiming-builder/opt/kaiming-tools/lib"
KAIMING_EXEC="debian/kaiming-builder/opt/kaiming-tools/bin/kaiming-builder"

mkdir -p "$BIN_DIR_DEBIAN" "$LIB_DIR_DEBIAN"

# kaiming-builder用的动态链接库，其依赖kaiming都有依赖并已自包含，故复用

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

# 修复所有可执行文件的 RPATH 和 interpreter
echo " 正在修复执行文件..."
for bin in "$BIN_DIR_DEBIAN"/*; do
    if file "$bin" | grep -q 'ELF .* executable'; then
        set_interpreter_and_rpath "$bin"
    fi
done

echo " 所有 RPATH 和 interpreter 设置完毕"
