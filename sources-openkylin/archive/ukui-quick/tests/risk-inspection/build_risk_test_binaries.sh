#!/usr/bin/env bash
# 风险检查专用 - 编译环境准备与测试二进制生成脚本
# 说明：
# 1. 不修改 tests/auto_test.sh 和 tests/runUnitTest.sh
# 2. 使用独立构建目录 build-risk-inspection
# 3. 同时生成本次风险检查所需的自动化测试二进制与交互 demo 二进制

set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-risk-inspection"
JOBS="$(command -v nproc >/dev/null 2>&1 && nproc || echo 4)"
INSTALL_DEPS="auto"
CLEAN_BUILD="no"

usage() {
    cat <<'EOF'
用法：
  bash tests/risk-inspection/build_risk_test_binaries.sh [选项]

选项：
  --install-deps yes|no   是否安装依赖
                          yes: 强制安装
                          no : 不安装
                          默认 auto：root 用户安装，普通用户跳过
  --clean                 清理旧的 build-risk-inspection 后重新构建
  --build-dir PATH        指定构建目录，默认 build-risk-inspection
  --jobs N                指定并行编译数，默认 nproc
  -h, --help              显示帮助

示例：
  bash tests/risk-inspection/build_risk_test_binaries.sh --clean
  bash tests/risk-inspection/build_risk_test_binaries.sh --install-deps no
EOF
}

fail() {
    echo ">>> $1" >&2
    exit 1
}

install_deps() {
    echo ">>> 安装编译依赖..."
    apt-get install -y \
        cmake \
	g++ \
        pkg-config \
        debhelper-compat \
        qtchooser \
        qtbase5-dev \
        qttools5-dev \
        qtdeclarative5-dev \
	qtquickcontrols2-5-dev \
        libqt5x11extras5-dev \
        libxcb1-dev \
        libopencv-dev \
        libgsettings-qt-dev \
        libkf5windowsystem-dev \
        libkysdk-waylandhelper-dev \
        libxcb-composite0-dev \
        libxcb-damage0-dev \
        libqt5xdg-dev \
        libpipewire-0.3-dev \
        libwayland-dev \
        libkf5wayland-dev \
        libqt5waylandclient5-dev \
        plasma-wayland-protocols \
        extra-cmake-modules \
        qtbase5-private-dev \
        libx11-dev \
        qtwayland5-dev-tools \
        libdrm-dev \
        wayland-protocols \
        kylin-wayland-protocols \
        libglx-dev \
        qtwayland5-private-dev \
        lcov \
        gcovr \
        libgtest-dev \
        libgmock-dev || fail "依赖安装失败，请检查软件源"
}

print_binary() {
    local path="$1"
    if [[ -x "$path" ]]; then
        echo "    [已生成] $path"
    else
        echo "    [未生成] $path"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --install-deps)
            INSTALL_DEPS="${2:-}"
            shift 2
            ;;
        --clean)
            CLEAN_BUILD="yes"
            shift
            ;;
        --build-dir)
            BUILD_DIR="${2:-}"
            shift 2
            ;;
        --jobs)
            JOBS="${2:-}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            fail "未知参数: $1，使用 --help 查看帮助"
            ;;
    esac
done

echo ">>> 项目目录: $PROJECT_DIR"
echo ">>> 构建目录: $BUILD_DIR"
echo ">>> 并行任务数: $JOBS"

case "$INSTALL_DEPS" in
    yes)
        [[ "$(id -u)" -eq 0 ]] || fail "--install-deps yes 需要 root 用户执行"
        install_deps
        ;;
    no)
        echo ">>> 按参数要求跳过依赖安装"
        ;;
    auto)
        if [[ "$(id -u)" -eq 0 ]]; then
            install_deps
        else
            echo ">>> 当前不是 root 用户，跳过依赖安装，使用系统现有环境"
        fi
        ;;
    *)
        fail "--install-deps 仅支持 yes / no / auto"
        ;;
esac

if [[ "$CLEAN_BUILD" == "yes" && -d "$BUILD_DIR" ]]; then
    echo ">>> 清理旧构建目录: $BUILD_DIR"
    rm -rf "$BUILD_DIR" || fail "清理构建目录失败"
fi

echo ">>> 配置 CMake..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TEST=ON \
    -DBUILD_INTERACTIVE_TESTS=ON || fail "CMake 配置失败"

echo ">>> 编译本次风险检查所需目标..."
cmake --build "$BUILD_DIR" -j"$JOBS" --target \
    test-widget-metadata \
    test-widget-loader \
    test-theme \
    settings \
    test-window-manager \
    test-wm-interface \
    test-abstract-window-manager \
    test-dbus-connector \
    test-app-launcher \
    test-window-thumbnail-config \
    test-window-thumbnail-plugin \
    test-thumbnail-item \
    test-player-items-model \
    test-shared-engine-component \
    test-shared-engine-view \
    platform-test \
    thumbnail-item-test || fail "目标编译失败"

echo ">>> 编译完成，关键二进制如下："
print_binary "$BUILD_DIR/framework/autotest/test-widget-metadata"
print_binary "$BUILD_DIR/framework/autotest/test-widget-loader"
print_binary "$BUILD_DIR/platform/autotest/test-theme"
print_binary "$BUILD_DIR/platform/autotest/settings"
print_binary "$BUILD_DIR/platform/autotest/test-window-manager"
print_binary "$BUILD_DIR/platform/autotest/test-wm-interface"
print_binary "$BUILD_DIR/platform/autotest/test-abstract-window-manager"
print_binary "$BUILD_DIR/platform/autotest/test-dbus-connector"
print_binary "$BUILD_DIR/platform/autotest/test-app-launcher"
print_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-config"
print_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-plugin"
print_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-thumbnail-item"
print_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-player-items-model"
print_binary "$BUILD_DIR/core/autotest/test-shared-engine-component"
print_binary "$BUILD_DIR/core/autotest/test-shared-engine-view"
print_binary "$BUILD_DIR/platform/test/platform-test"
print_binary "$BUILD_DIR/modules/window-thumbnail/core/test/thumbnail-item-test"

echo ">>> 风险检查二进制准备完成"
