#!/bin/bash
apt install -y build-essential:native debhelper qtbase5-dev qt5-qmake qtchooser qttools5-dev-tools qtbase5-dev-tools qtmultimedia5-dev libmp3lame-dev libgsettings-qt-dev libqt5x11extras5-dev libkf5windowsystem-dev libukui-log4qt-dev libavformat-dev libavcodec-dev libmpv-dev libkysdk-kabase-dev libkysdk-base-dev libkysdk-applications-dev libpeony-dev libkysdk-systime-dev libkysdk-datacollect-dev lcov libgtest-dev libgmock-dev

# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
cp ../data/*.xml  /usr/share/glib-2.0/schemas
glib-compile-schemas /usr/share/glib-2.0/schemas

echo "install gsettings"

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
