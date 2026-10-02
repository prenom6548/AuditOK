#!/bin/bash
# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
apt install -y build-essential:native pkg-config debhelper-compat libcap-dev libdbus-glib-1-dev libgsettings-qt-dev libgtk-3-dev libkf5networkmanagerqt-dev libkf5windowsystem-dev libkysdk-qtwidgets-dev libnm-dev libnma-dev libqt5svg5-dev libqt5x11extras5-dev libsecret-1-dev libx11-dev qt5-qmake qtbase5-dev qtbase5-dev-tools qtchooser qtscript5-dev qttools5-dev-tools  lcov libgtest-dev libgmock-dev libdconf-dev

cd "$(dirname "$0")" || exit 1 
if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build  # 强制删除build目录及所有内容
fi
cd ../
echo "Build  souce code start!"

echo "running qmake souce code"
qmake *.pro
if [ $? -ne 0 ]; then
    echo "Error: qmake failed."
    exit -1
fi
echo "Running make..."
make -j4
if [ $? -ne 0 ]; then
    echo "Error: make failed."
    exit -1
fi
echo "Build  souce code end!"

cd tests/
mkdir build
cd build
echo "running qmake"
qmake ..

if [ $? -ne 0 ]; then
    echo "Error: qmake failed."
    exit -1
fi
echo "Running make..."
make -j4
echo "Build successful!"
exit 0

