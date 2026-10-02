#!/bin/bash
# auto_test.sh
# 目的：编译单元测试二进制（test-settings）及 CMake 依赖，列出编译所需依赖包与构建命令。
# 作为自动化测试平台编译单元测试的统一入口，须放在 tests 目录下，文件名固定为 auto_test.sh。
#
# 构建说明：本工程根目录 CMakeLists.txt 通过 add_subdirectory(tests) 引入单测；
# 使用「工程根 + build-coverage/」影子目录（对齐 peony 的 build-coverage 思路）：
# - PLATFORMTHEME_SKIP_TEST 非 1 时根 CMakeLists 全树加 --coverage，使 libqt5-ukui-style、test-settings 等生成 .gcno/.gcda
#   均生成在该构建目录下，避免在源码树或混用 root 构建导致 *.gcno 路径错乱；
# - 编译仍以当前用户执行，避免 root 属主的 *.gcda。
# 成功后单测路径：<工程根>/build-coverage/tests/auto-test/test-settings

# 安装编译依赖：非 root 时用 sudo 仅提升 apt 权限；编译仍以当前用户执行，避免覆盖率文件属主异常。
APT_BIN="apt"
if [ "${EUID}" -ne 0 ] && command -v sudo >/dev/null 2>&1; then
    APT_BIN="sudo apt"
fi

# 依赖与 debian/control 中 Build-Depends 尽量对齐；lcov/gcovr 供覆盖率汇总。
# 部分 Kylin SDK 包（如 libkysdk-ukuiwindowhelper-dev）仅在麒麟/内部源提供，标准 Ubuntu/Debian 源无此包名，
# 故单独探测后再装，避免整段 apt 失败。
${APT_BIN} install -y \
    build-essential \
    cmake \
    pkg-config \
    dbus-x11 \
    qt5-qmake \
    qtchooser \
    qtbase5-dev \
    qtbase5-dev-tools \
    qtbase5-private-dev \
    qttools5-dev \
    qttools5-dev-tools \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    libqt5svg5-dev \
    libqt5x11extras5-dev \
    kirigami2-dev \
    libfontconfig-dev \
    libfreetype-dev \
    libglib2.0-dev \
    libgsettings-qt-dev \
    libkf5iconthemes-dev \
    libkf5wayland-dev \
    libkf5windowsystem-dev \
    libkysdk-conf2-dev \
    libkysdk-waylandhelper-dev \
    libkysdk-qtwidgets-dev \
    libpeony-dev \
    libx11-dev \
    libxcb1-dev \
    lcov \
    gcovr

if [ $? -ne 0 ]; then
    echo "Error: dependency installation failed."
    exit 1
fi

# 可选：UKUI/Kylin 源中的 kysdk-ukuiwindowhelper 开发包（单测 CMake pkg_check 需要）
_UKW_PKG="libkysdk-ukuiwindowhelper-dev"
if apt-cache show "${_UKW_PKG}" >/dev/null 2>&1; then
    ${APT_BIN} install -y "${_UKW_PKG}" || echo "Warning: ${_UKW_PKG} install failed, CMake may still fail without kysdk-ukuiwindowhelper."
else
    echo "Note: 当前 apt 源中无 ${_UKW_PKG}，已跳过。若本机已安装 kysdk-ukuiwindowhelper（pkg-config 可见）可继续；"
    echo "      否则请启用麒麟/UKUI 软件源后: ${APT_BIN} install -y ${_UKW_PKG}"
fi

cd "$(dirname "$0")" || exit 1

TESTS_DIR=$(pwd)
PROJECT_ROOT=$(cd .. && pwd)
# 与 peony 一致使用独立覆盖率构建目录，便于 gcovr/lcov 只指向此处
BUILD_DIR="${PROJECT_ROOT}/build-coverage"

# 非 1 时根 CMakeLists 会编 tests 并全树启用 --coverage（见工程根 CMakeLists.txt）
export PLATFORMTHEME_SKIP_TEST=0

echo "Configuring and building with coverage (CMake, shadow build in build-coverage/)..."
echo "  PROJECT_ROOT=${PROJECT_ROOT}"
echo "  BUILD_DIR=${BUILD_DIR}"

if [ -d "${BUILD_DIR}" ]; then
    echo "Removing existing build-coverage directory..."
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}" || exit 1

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug

if [ $? -ne 0 ]; then
    echo "Error: cmake configuration failed."
    exit 1
fi

cmake --build "${BUILD_DIR}" -j4

if [ $? -ne 0 ]; then
    echo "Error: build of test-settings failed."
    exit 1
fi

BIN="${BUILD_DIR}/tests/auto-test/test-settings"
if [ ! -x "${BIN}" ]; then
    echo "Error: ${BIN} not found or not executable after build."
    exit 1
fi

echo "Build successful!"
echo "  Test binary: ${BIN}"
echo "  Run (optional): bash ${TESTS_DIR}/auto-test/run-auto-test.sh \"${BIN}\""
echo "Note: *.gcno 在 ${BUILD_DIR} 各目标输出目录；运行 test-settings 后 *.gcda 写入同目录。"
echo "      汇总覆盖率请对工程根使用 gcovr/lcov，并指定 --object-directory ${BUILD_DIR}"
echo "      （勿仅对单测子目录或源码根用 --object-directory=.，以免漏掉 libqt5-ukui-style 等目标的 .gcno）。"
exit 0
