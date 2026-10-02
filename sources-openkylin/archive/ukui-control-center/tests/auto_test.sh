#!/bin/bash
# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
# 目的：编译出单元测试的二进制文件，内容包含编译单元测试所需的所有依赖包及编译单元测试的命令。
# 该脚本是自动化测试平台编译单元测试的统一入口，必须存放在 tests 目录下，名称固定不可修改。

# 安装编译单元测试所需的依赖包：
# - 若当前用户非 root，优先使用 sudo 仅提升 apt 安装权限；
# - 后续编译保持当前用户执行，避免生成 root 属主的 *.gcno/*.gcda。

APT_BIN="apt"
if [ "${EUID}" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
    APT_BIN="sudo apt"
fi
${APT_BIN} install -y \
    build-essential \
    libqt5svg5-dev \
    libgsettings-qt-dev \
    libglib2.0-dev \
    libqt5x11extras5-dev \
    libxklavier-dev \
    libkf5screen-dev \
    libkf5screen-bin \
    libkf5i18n-dev \
    libkf5windowsystem-dev \
    libkf5xmlgui-dev \
    libkf5globalaccel-dev \
    qtdeclarative5-dev \
    libdconf-dev \
    libxml2-dev \
    qtbase5-dev \
    libx11-dev \
    libxcursor-dev \
    libxkbfile-dev \
    libboost-dev \
    qttools5-dev-tools \
    libxcb-xkb-dev \
    libpolkit-qt5-1-dev \
    libpulse-dev \
    libpwquality-dev \
    xserver-xorg-dev \
    libupower-glib-dev \
    libpam0g-dev \
    libukui-log4qt-dev \
    libkylin-chkname-dev \
    libcups2-dev \
    libkysdk-qtwidgets-dev \
    libkysdk-sysinfo-dev \
    libkysdk-waylandhelper-dev \
    libkysdk-diagnostics-dev \
    libkf5service-dev \
    libkysdk-systime-dev \
    libkysdk-conf2-dev \
    libukui-service-interface-dev \
    libfontconfig-dev \
    libfreetype-dev \
    libkysdk-ukenv-dev \
    libgtest-dev \
    libukui-quick-dev \
    libkysdk-hardware-dev \
    libkysec-dev \
    libkysdk-package-dev \
    libkim-dev \
    gcovr \
    lcov \

if [ $? -ne 0 ]; then
    echo "Error: dependency installation failed."
    exit -1
fi

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

