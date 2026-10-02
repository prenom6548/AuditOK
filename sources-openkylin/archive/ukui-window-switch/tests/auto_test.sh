#!/bin/bash
# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
apt install -y \
build-essential:native \
cmake \
debhelper-compat \
extra-cmake-modules \
pkg-config \
pkg-config \
libdrm-dev \
libfontconfig1-dev \
libfreetype6-dev \
libglib2.0-dev \
libgsettings-qt-dev \
libkf5config-dev \
libkf5coreaddons-dev \
libkf5globalaccel-dev \
libkf5wayland-dev \
libkf5windowsystem-dev \
libkysdk-sysinfo-dev \
libkysdk-waylandhelper-dev \
libmtdev-dev \
libqt5x11extras5-dev \
libxcb-composite0-dev \
libxcb-damage0-dev \
libxcb1-dev \
libxrender-dev \
libkysdk-datacollect-dev \
qtbase5-dev \
qtbase5-private-dev \
qtdeclarative5-dev \
qtdeclarative5-private-dev \
qttools5-dev \
kylin-wayland-compositor-client \
libukui-quick-dev \
libukui-search-dev \
libqt5xdg-dev \
libgtest-dev \
libgmock-dev
cd "$(dirname "$0")" || exit 1 
if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build  # 强制删除build目录及所有内容
fi
mkdir build
cd build
echo "running cmake"
cmake  ..
if [ $? -ne 0 ]; then
    echo "Error: cmake failed."
    exit -1
fi
echo "Running make..."
make -j4
if [ $? -ne 0 ]; then
    echo "Error: make failed."
    exit -1
fi
echo "Build successful!"
exit 0

