#!/bin/bash

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${TEST_DIR}/.." && pwd)"
BUILD_DIR="${TEST_DIR}/build"

check_status() {
    if [ "$1" -ne 0 ]; then
        echo "$2"
        exit -1
    fi
}

apt install -y \
    debhelper-compat \
    cmake \
    pkgconf \
    qtchooser \
    qtbase5-dev \
    qttools5-dev \
    qtdeclarative5-dev \
    libqt5x11extras5-dev \
    libxcb1-dev \
    libopencv-dev \
    libgsettings-qt-dev \
    libkf5windowsystem-dev \
    libkf5wayland-dev \
    libkysdk-sysinfo-dev \
    libkysdk-waylandhelper-dev \
    libukui-notification-dev \
    libkysdk-datacollect-dev \
    libukui-quick-dev \
    libglib2.0-dev \
    libukui-search-dev \
    libkysdk-systime-dev \
    qtdeclarative5-private-dev \
    qtbase5-private-dev
check_status $? "Error: apt install failed."

if [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}"
    check_status $? "Error: failed to remove build directory."
fi

mkdir -p "${BUILD_DIR}"
check_status $? "Error: failed to create build directory."

cd "${BUILD_DIR}"
check_status $? "Error: failed to enter build directory."

cmake -DBUILD_TEST=ON "${PROJECT_DIR}"
check_status $? "Error: cmake failed."

make -j"$(nproc)"
check_status $? "Error: failed to build unit tests."

exit 0
