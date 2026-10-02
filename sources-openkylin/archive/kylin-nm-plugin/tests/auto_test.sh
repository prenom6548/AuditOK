APT_BIN="apt"
if [ "${EUID}" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
    APT_BIN="sudo apt"
fi
${APT_BIN} install -y \
	debhelper \
	libcap-dev \
	libgsettings-qt-dev \
	libkf5networkmanagerqt-dev\
	libkf5windowsystem-dev \
	libkylin-nm-base \
	libkysdk-qtwidgets-dev \
	libnm-dev \
	libqt5svg5-dev \
	libqt5x11extras5-dev \
	libsecret-1-dev \
	libx11-dev \
	qt5-qmake \
	qtbase5-dev \
	qtbase5-dev-tools \
	qtchooser \
	qtscript5-dev \
	qttools5-dev-tools \
	libgtest-dev \

echo "apt install successful!!!"

cd "$(dirname "$0")" || exit 1 
if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build  # 强制删除build目录及所有内容
fi
mkdir build
cd build
echo "running qmake"
qmake  ..
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