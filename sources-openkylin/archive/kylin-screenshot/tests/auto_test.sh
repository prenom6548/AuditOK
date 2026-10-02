#!/bin/bash
apt install -y libkf5wayland-dev libpulse-dev libqt5svg5-dev libukui-log4qt-dev libxfixes-dev libxinerama-dev libxtst-dev libwayland-dev qtdeclarative5-dev qtdeclarative5-private-dev qttools5-dev libgtest-dev libgmock-dev libdconf-dev debhelper-compat libavcodec-dev libavformat-dev libavutil-dev libgsettings-qt-dev libkf5windowsystem-dev libkysdk-qtwidgets-dev libkysdk-waylandhelper-dev libkysdk-kabase-dev libopencv-core-dev libopencv-dev libqt5x11extras5 libqt5x11extras5-dev libswscale-dev libx11-dev libxfixes3 libxinerama1 qt5-qmake qtbase5-dev libkysdk-datacollect-dev libkysdk-log-dev kylin-wayland-compositor-client qtbase5-private-dev qttools5-dev-tools gcc build-essential g++ qtbase5-dev-tools  libqt5gui5 libqt5core5a  libqt5widgets5 libqt5test5 libkysdk-alm-dev libkysdk-base libkysdk-base-dev libkysdk-log libkysdk-log-dev libkysdk-logrotate lcov  libkysdk-sysinfo libkysdk-sysinfo-dev

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
