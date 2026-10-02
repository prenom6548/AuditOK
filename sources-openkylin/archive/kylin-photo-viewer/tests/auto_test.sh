#!/bin/bash
apt install -y debhelper libfreeimage-dev libfreeimageplus-dev libgif-dev libglib2.0-dev libgsettings-qt-dev libkf5windowsystem-dev libkysdk-waylandhelper-dev libkysdk-applications-dev libkysdk-base-dev libkysdk-ocr-dev libkysdk-datacollect-dev libpng-dev libqt5x11extras5-dev libstb-dev pkgconf qt5-qmake qtbase5-dev qtbase5-dev-tools qtchooser qtdeclarative5-dev qtquickcontrols2-5-dev qtscript5-dev qttools5-dev-tools librsvg2-dev libkysdk-filesec-dev libukui-log4qt-dev libpeony-dev


function replace_apt_source(){
    chattr -i /etc/apt/sources.list
    cat << EOF > /etc/apt/sources.list
deb http://archive.kylinos.cn/kylin/KYLIN-ALL .tags--11.0-kylin--2603-rc5 main restricted universe multiverse
deb http://archive.kylinos.cn/kylin/KYLIN-ALL .tags--11.0-kylin-2603--2603-rc5 main restricted universe multiverse
deb https://archive2.kylinos.cn/DEB/KYLIN_DEB V11 main all

EOF
    chattr +i /etc/apt/sources.list
    apt update
}
replace_apt_source

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
