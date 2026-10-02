#!/bin/bash
apt install -y build-essential:native debhelper qtbase5-dev cmake googletest google-mock libqt5svg5-dev libkysdk-qtwidgets-dev libkysdk-sysinfo-dev qt5-qmake qtchooser qtscript5-dev qttools5-dev-tools qtbase5-dev-tools qtmultimedia5-dev libtag1-dev libgsettings-qt-dev libqt5x11extras5-dev libx11-dev libkf5windowsystem-dev libavformat-dev libavutil-dev libukui-log4qt-dev libmpv-dev libkysdk-waylandhelper-dev libsqlite3-dev libpeony-dev libxtst-dev libkysdk-kabase-dev  libkysdk-datacollect-dev lcov libgtest-dev libgmock-dev

rm -rf $PWD/build && mkdir $PWD/build

if [ -d "$PWD/build" ];then
    cd $PWD/build
    qmake ..
    make -j8
    [ $? == 0 ] && echo "Kylin-weather unit test build success" || echo "Kylin-weather unit test build failed"
    cd -
else
    echo "Error $PWD/build folder not exist."
    exit 1
fi


