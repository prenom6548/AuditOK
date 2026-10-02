#!/bin/bash
apt install -y debhelper-compat libkysdk-waylandhelper-dev libgsettings-qt-dev libkf5windowsystem-dev libpolkit-qt5-1-dev libqt5x11extras5-dev libukui-log4qt-dev pkgconf qt5-qmake qtbase5-dev qtbase5-dev-tools qtchooser qtscript5-dev libkysdk-qtwidgets-dev libkysdk-soundeffects-dev libkysdk-datacollect-dev qttools5-dev-tools
# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意


cd "$(dirname "$0")" || exit 1 

echo "running qmake"
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

echo "Build successful!"
exit 0
