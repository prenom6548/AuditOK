#!/bin/bash

# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
apt install -y qtbase5-dev build-essential:native cmake pkg-config debhelper-compat qtbase5-dev libglib2.0-dev libqt5x11extras5-dev libgsettings-qt-dev libwayland-dev libkf5windowsystem-dev libkf5iconthemes-dev libkf5wayland-dev libx11-dev libzip-dev libbz2-dev libreadline-dev libncurses-dev libacl1-dev libdbus-1-dev libdbus-glib-1-dev libpthread-stubs0-dev libukui-log4qt-dev libkysdk-qtwidgets-dev libkysdk-waylandhelper-dev libkysdk-soundeffects-dev libkysdk-datacollect-dev libkysdk-applications-dev libkysdk-base-dev libkysdk-alm-dev libkysdk-sysinfo-dev

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
