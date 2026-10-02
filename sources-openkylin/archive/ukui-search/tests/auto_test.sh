#!/bin/bash

set -u

if [ "${SKIP_APT_INSTALL:-0}" = "1" ]; then
    echo "SKIP_APT_INSTALL=1, skip dependency installation."
    exit 0
fi

if ! command -v apt >/dev/null 2>&1; then
    echo "Error: apt command not found."
    exit 1
fi

apt install -y \
    build-essential file lcov gcovr \
    debhelper-compat pkgconf cmake \
    libgsettings-qt-dev \
    qtbase5-dev qt5-qmake qtchooser qtscript5-dev qttools5-dev \
    libxapian-dev libquazip5-dev libglib2.0-dev libqt5x11extras5-dev \
    libuchardet-dev libpoppler-qt5-dev libukui-log4qt-dev libqt5xdg-dev \
    libukcc-dev libtesseract-dev libleptonica-dev libkysdk-qtwidgets-dev \
    qtdeclarative5-dev libqt5remoteobjects5-dev \
    libkysdk-systime-dev libkysdk-datacollect-dev libkysdk-proc-dev \
    libukui-file-metadata-dev libkyai-data-management-client-dev \
    libkysdk-ai-common-dev libukui-quick-dev libkf5wayland-dev \
    libsqlite3-dev libukui-service-interface-dev

if [ $? -ne 0 ]; then
    echo "Error: apt install failed."
    exit 1
fi

echo "Dependency installation finished."
