#!/bin/bash

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${TEST_DIR}/.." && pwd)"
BUILD_DIR="${TEST_DIR}/build"
TEST_CXX_FLAGS="${TEST_CXX_FLAGS:--O0 -fno-exceptions}"

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/ukui-menu-tests-runtime}"
mkdir -p "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"
export LIBGL_ALWAYS_SOFTWARE=1
export QT_QUICK_BACKEND=software

check_status() {
    if [ "$1" -ne 0 ]; then
        echo "$2"
        exit -1
    fi
}

apt install -y \
    debhelper-compat \
    cmake \
    pkgconf \
    qtchooser \
    qtbase5-dev \
    qttools5-dev \
    qtdeclarative5-dev \
    libglib2.0-dev \
    libukui-menu-dev \
    libkysdk-datacollect-dev \
    libkysdk-systime-dev \
    libqt5xdg-dev
check_status $? "Error: apt install failed."

if [ -d "${BUILD_DIR}" ]; then
    rm -rf "${BUILD_DIR}"
    check_status $? "Error: failed to remove build directory."
fi

mkdir -p "${BUILD_DIR}"
check_status $? "Error: failed to create build directory."

cd "${BUILD_DIR}"
check_status $? "Error: failed to enter build directory."

cmake -DBUILD_TEST=ON -DCMAKE_CXX_FLAGS="${TEST_CXX_FLAGS}" "${PROJECT_DIR}"
check_status $? "Error: cmake failed."

make -j4
check_status $? "Error: make failed."

"${BUILD_DIR}/recent-file/tests/recentFileTest"
check_status $? "Error: recentFileTest failed."

"${BUILD_DIR}/recent-file/tests/eventTrackTest"
check_status $? "Error: eventTrackTest failed."

"${BUILD_DIR}/ernie-bot/tests/ernieBotTest"
check_status $? "Error: ernieBotTest failed."

exit 0
