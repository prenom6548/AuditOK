#!/bin/bash

set -e

CRUN_SRC="/usr/bin/crun"
XDELTATA3_SRC="/usr/bin/xdelta3"
BIN_DIR="/opt/kaiming-tools/bin"
LIB_DIR="/opt/kaiming-tools/lib"
BIN_DIR_DEBIAN="debian/kaiming/opt/kaiming-tools/bin"
LIB_DIR_DEBIAN="debian/kaiming/opt/kaiming-tools/lib"
KAIMING_CLI="debian/kaiming/opt/kaiming-tools/bin/km-cli"
KAIMING_RUN="debian/kaiming/opt/kaiming-tools/bin/km-run"

mkdir -p "$BIN_DIR_DEBIAN" "$LIB_DIR_DEBIAN"

# 复制单个库及其软链接结构
copy_single_lib() {
    local lib="$1"
    if [ ! -e "$lib" ]; then
        echo " 跳过无效库路径: $lib"
        return
    fi

    local dir base real_path
    dir=$(dirname "$lib")
    base=$(basename "$lib")
    real_path=$(readlink -f "$lib")

    # 复制真实文件
    if [ -f "$real_path" ]; then
        local real_target="$LIB_DIR_DEBIAN/$(basename "$real_path")"
        if [ ! -e "$real_target" ]; then
            cp -p "$real_path" "$LIB_DIR_DEBIAN/"
        fi
    fi

    # 复制软链接结构
    if [ -L "$lib" ]; then
        local link_target="$LIB_DIR_DEBIAN/$base"
        if [ ! -e "$link_target" ]; then
            cp -Pp "$lib" "$LIB_DIR_DEBIAN/"
        fi
    fi

    # 复制同名变体，例如 libssl.so libssl.so.1.1 libssl.so.3 等
    local matches=("$dir"/"$base"*)
    for match in "${matches[@]}"; do
        [ -e "$match" ] || continue
        local match_target="$LIB_DIR_DEBIAN/$(basename "$match")"
        if [ ! -e "$match_target" ]; then
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
copy_deps "$KAIMING_CLI"
copy_deps "$KAIMING_RUN"
copy_deps "$CRUN_SRC"
copy_deps "$XDELTATA3_SRC"
cp -p "$CRUN_SRC" "$BIN_DIR_DEBIAN/crun"
cp -p "$XDELTATA3_SRC" "$BIN_DIR_DEBIAN"

# 额外复制动态链接器（例如 ld-linux-x86-64.so.2）
interpreter=$(readelf -l "$CRUN_SRC" | grep 'Requesting program interpreter' | awk -F': ' '{print $2}' | tr -d '[]')
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
    # v10sp1中ok-engine无法全静态自包含，自包含动态依赖库意义不大，修改其链接器rpath会造成段错误，此处跳过
    bin_name=$(basename "$bin")
    if [ "$bin_name" = "ok-engine" ]; then
        continue
    fi

    if file "$bin" | grep -q 'ELF .* executable'; then
        set_interpreter_and_rpath "$bin"
    fi
done

echo " 所有 RPATH 和 interpreter 设置完毕"
