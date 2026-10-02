
apt install build-essential g++ libgtest-dev libgmock-dev lcov libtimedate-perl\
                qtbase5-dev qt5-qmake qtchooser qtscript5-dev qttools5-dev-tools qtbase5-dev-tools \
                qtbase5-private-dev libblkid-dev libkysec-dev libkf5windowsystem-dev \
	          libx11-dev libqt5x11extras5-dev libgsettings-qt-dev libkysdk-qtwidgets-dev libkysec-extend-dev \
                libkysdk-waylandhelper-dev libkysdk-soundeffects-dev libkysdk-systime-dev libkysdk-datacollect-dev \
                libkysdk-appconf2-dev libkysdk-conf2-dev -y


rm -rf $PWD/build && mkdir $PWD/build

if [ -d "$PWD/build" ];then
    cd $PWD/build
    qmake .. && make -j8 && echo "yhkylin-backup-tools unit test build success" || echo "yhkylin-backup-tools unit test build failed"
    cd -
else
    echo "Error $PWD/build folder not exist."
    exit 1
fi


