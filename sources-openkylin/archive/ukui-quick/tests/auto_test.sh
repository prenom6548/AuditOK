#!/bin/bash
# 白盒单元测试 - 编译入口脚本
# 功能：安装依赖并编译单元测试二进制文件

set -u
set -o pipefail

export DEBIAN_FRONTEND=noninteractive

fail()
{
    echo ">>> $1" >&2
    exit -1
}

# 获取项目根目录（tests 目录的上级）
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
JOBS="$(command -v nproc >/dev/null 2>&1 && nproc || echo 4)"

echo ">>> 项目目录: $PROJECT_DIR"
echo ">>> 构建目录: $BUILD_DIR"

if [ "$(id -u)" -eq 0 ]; then
    # 安装编译依赖（不加 sudo，apt install 加 -y）
    echo ">>> 安装编译依赖..."
    if ! apt-get install -y \
        cmake \
        pkg-config \
        debhelper-compat \
        qtchooser \
        qtbase5-dev \
        qttools5-dev \
        qtdeclarative5-dev \
        libqt5x11extras5-dev \
        libxcb1-dev \
        libopencv-dev \
        libgsettings-qt-dev \
        libkf5windowsystem-dev \
        libkysdk-waylandhelper-dev \
        libxcb-composite0-dev \
        libxcb-damage0-dev \
        libqt5xdg-dev \
        libpipewire-0.3-dev \
        libwayland-dev \
        libkf5wayland-dev \
        libqt5waylandclient5-dev \
        plasma-wayland-protocols \
        extra-cmake-modules \
        qtbase5-private-dev \
        libx11-dev \
        qtwayland5-dev-tools \
        libdrm-dev \
        wayland-protocols \
        kylin-wayland-protocols \
        libglx-dev \
        qtwayland5-private-dev \
        libthorvg-dev \
        unzip \
        lcov \
        gcovr \
        libgtest-dev \
        libgmock-dev; then
        fail "依赖安装失败，请检查软件源或补充 test_white_sources.list"
    fi
else
    echo ">>> 当前不是 root 用户，跳过依赖安装，使用现有构建环境"
fi

if [ -d "$BUILD_DIR" ]; then
    echo ">>> 清理旧构建目录: $BUILD_DIR"
    rm -rf "$BUILD_DIR" || fail "清理旧构建目录失败"
fi

# 编译项目（开启测试 + 覆盖率参数）
echo ">>> 配置 CMake..."
if ! cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TEST=ON \
    -DBUILD_INTERACTIVE_TESTS=OFF \
    -DUKUI_QUICK_ENABLE_COVERAGE=ON; then
    fail "CMake 配置失败"
fi

echo ">>> 编译单元测试..."
if ! cmake --build "$BUILD_DIR" -j"$JOBS"; then
    fail "单元测试编译失败"
fi

echo ">>> 单元测试编译成功"
exit 0
