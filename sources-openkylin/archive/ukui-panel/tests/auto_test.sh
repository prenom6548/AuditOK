#!/bin/bash

set -u

export DEBIAN_FRONTEND=noninteractive


if [ "$(id -u)" -eq 0 ]; then
    # 安装编译依赖（不加 sudo，apt install 加 -y）
    echo ">>> 安装编译依赖..."
    if ! apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    debhelper-compat \
    qtbase5-dev \
    qtbase5-dev-tools \
    qtdeclarative5-dev \
    libqt5x11extras5-dev \
    libqt5remoteobjects5-dev \
    libqt5xdg-dev \
    libglib2.0-dev \
    libgsettings-qt-dev \
    libwayland-dev \
    libkf5windowsystem-dev \
    libkf5iconthemes-dev \
    libkf5wayland-dev \
    libx11-dev \
    libzip-dev \
    libbz2-dev \
    libreadline-dev \
    libncurses-dev \
    libacl1-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libpthread-stubs0-dev \
    libukui-log4qt-dev \
    libukui-quick-dev \
    libukui-search-dev \
    libkysdk-qtwidgets-dev \
    libkysdk-waylandhelper-dev \
    libkysdk-soundeffects-dev \
    libkysdk-datacollect-dev \
    libkysdk-applications-dev \
    libkysdk-base-dev \
    libkysdk-alm-dev \
    libkysdk-sysinfo-dev \
    libkysdk-systime-dev; then
    fail "依赖安装失败，请检查软件源或补充 test_white_sources.list"
    fi
else
    echo ">>> 当前不是 root 用户，跳过依赖安装，使用现有构建环境"
fi


SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ $? -ne 0 ]; then
    echo "Error: failed to locate script directory."
    exit -1
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
if [ $? -ne 0 ]; then
    echo "Error: failed to locate project root directory."
    exit -1
fi

AUTOTEST_SOURCE_DIR="${ROOT_DIR}/panel/autotest"

BUILD_DIR="${SCRIPT_DIR}/build"

if [ -d "${BUILD_DIR}" ]; then
    echo ">>> 清理旧构建目录: $BUILD_DIR"
    rm -rf "${BUILD_DIR}"
    if [ $? -ne 0 ]; then
        echo "清理旧构建目录失败"
        exit -1
    fi
fi

mkdir -p "${BUILD_DIR}"

echo "配置并执行 cmake..."
cmake -S "${AUTOTEST_SOURCE_DIR}" -B "${BUILD_DIR}"
if [ $? -ne 0 ]; then
    echo "cmake 执行失败."
    exit -1
fi

echo ">>> 编译单元测试..."
cmake --build "${BUILD_DIR}" -j4
if [ $? -ne 0 ]; then
    echo "======单元测试编译失败====="
    exit -1
fi

echo "======单元测试编译成功====="
exit 0
