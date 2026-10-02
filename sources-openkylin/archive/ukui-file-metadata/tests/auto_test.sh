#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

install_build_dependencies()
{
    if [ "${SKIP_APT_INSTALL:-0}" = "1" ]; then
        return
    fi

    if ! command -v apt-get >/dev/null 2>&1; then
        echo "apt-get not found, skip dependency installation."
        return
    fi

    local packages=(
        build-essential
        file
        gcovr
        lcov
        debhelper-compat
        pkgconf
        qtbase5-dev
        qtchooser
        qtscript5-dev
        libquazip5-dev
        libuchardet-dev
        libpoppler-qt5-dev
        libavcodec-dev
        libavformat-dev
        libavutil-dev
        libswscale-dev
        libtesseract-dev
        libleptonica-dev
        libminizip-dev
        cmake
        qttools5-dev
        libtag1-dev
    )

    if [ "$(id -u)" -eq 0 ]; then
        apt-get update
        apt-get install -y "${packages[@]}"
    else
        sudo apt-get update
        sudo apt-get install -y "${packages[@]}"
    fi
}

install_build_dependencies
