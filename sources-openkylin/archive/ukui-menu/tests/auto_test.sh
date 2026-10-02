#!/bin/bash

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${TEST_DIR}/.." && pwd)"
BUILD_DIR="${TEST_DIR}/build"
AUTOTEST_DIR="${BUILD_DIR}/autotest"
AUTOTEST_COMPAT_DIR="${BUILD_DIR}/autotests"

resolve_test_output_dir() {
    if [ -d "${AUTOTEST_DIR}" ]; then
        printf '%s\n' "${AUTOTEST_DIR}"
        return 0
    fi

    if [ -d "${AUTOTEST_COMPAT_DIR}" ]; then
        printf '%s\n' "${AUTOTEST_COMPAT_DIR}"
        return 0
    fi

    return 1
}

check_status() {
    if [ "$1" -ne 0 ]; then
        echo "$2"
        exit -1
    fi
}

install_build_dependencies() {
    apt install -y \
        debhelper-compat \
        cmake \
        make \
        g++ \
        pkgconf \
        qtchooser \
        qtbase5-dev \
        qtbase5-dev-tools \
        qtbase5-private-dev \
        qtdeclarative5-dev \
        qtdeclarative5-dev-tools \
        qtdeclarative5-private-dev \
        qttools5-dev \
        libqt5x11extras5-dev \
        libqt5xdg-dev \
        libkf5windowsystem-dev \
        libkf5wayland-dev \
        libgsettings-qt-dev \
        libglib2.0-dev \
        libxcb1-dev \
        libopencv-dev \
        libkysdk-datacollect-dev \
        libkysdk-package-dev \
        libkysdk-system-dev \
        libkysdk-sysinfo-dev \
        libkysdk-systime-dev \
        libkysdk-waylandhelper-dev \
        libukui-notification-dev \
        libukui-quick-dev \
        libukui-search-dev
    check_status $? "Error: apt install failed."
}

prepare_build_directory() {
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
        check_status $? "Error: failed to remove build directory."
    fi

    mkdir -p "${BUILD_DIR}"
    check_status $? "Error: failed to create build directory."
}

configure_project() {
    cd "${BUILD_DIR}"
    check_status $? "Error: failed to enter build directory."

    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=ON "${PROJECT_DIR}"
    check_status $? "Error: cmake configure failed."
}

build_unit_tests() {
    cmake --build "${BUILD_DIR}" -j"$(nproc)"
    check_status $? "Error: failed to build unit test targets."
}

prepare_test_output() {
    local detected_test_dir

    detected_test_dir="$(resolve_test_output_dir)"
    if [ -z "${detected_test_dir}" ]; then
        echo "Error: unit test output directory not found: ${AUTOTEST_DIR}"
        exit -1
    fi

    mkdir -p "${BUILD_DIR}/userConfigTestHome"
    check_status $? "Error: failed to create userConfigTest HOME directory."

    rm -rf "${AUTOTEST_COMPAT_DIR}"
    check_status $? "Error: failed to remove compatibility test directory."

    if [ "${detected_test_dir}" != "${AUTOTEST_COMPAT_DIR}" ]; then
        ln -s "${detected_test_dir}" "${AUTOTEST_COMPAT_DIR}"
        check_status $? "Error: failed to create compatibility test directory link."
    fi

    echo "Unit test output directory: ${detected_test_dir}"
    if [ -L "${AUTOTEST_COMPAT_DIR}" ]; then
        echo "Compatibility unit test directory: ${AUTOTEST_COMPAT_DIR} -> ${detected_test_dir}"
    fi
}

install_build_dependencies
prepare_build_directory
configure_project
build_unit_tests
prepare_test_output

exit 0
