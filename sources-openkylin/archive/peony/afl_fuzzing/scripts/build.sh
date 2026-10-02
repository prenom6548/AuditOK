#!/bin/bash
# 构建Peony AFL模糊测试目标（方案1：隔离层+同步API）

set -e

echo "Building Peony AFL fuzzing harness..."

# 检查AFL编译器
if command -v afl-clang++ &> /dev/null; then
    AFL_COMPILER="afl-clang++"
elif command -v afl-g++ &> /dev/null; then
    AFL_COMPILER="afl-g++"
else
    echo "Error: AFL compiler not found"
    echo "Install AFL++: sudo apt-get install afl++"
    exit 1
fi

echo "Using compiler: $AFL_COMPILER"

# Peony库路径
PEONY_LIB_PATH="../../libpeony-qt"
PEONY_INCLUDE_PATH="../../libpeony-qt"

# 检查Peony库
if [ ! -f "$PEONY_LIB_PATH/libpeony.so" ] && [ ! -f "$PEONY_LIB_PATH/libpeony.so.3" ]; then
    echo "Error: Peony library not found"
    echo "Build libpeony-qt first"
    exit 1
fi

# 获取编译标志
QT_FLAGS=$(pkg-config --cflags Qt5Core Qt5Gui Qt5Widgets 2>/dev/null || echo "-I/usr/include/qt5 -I/usr/include/aarch64-linux-gnu/qt5 -I/usr/include/aarch64-linux-gnu/qt5/QtCore -I/usr/include/aarch64-linux-gnu/qt5/QtGui -I/usr/include/aarch64-linux-gnu/qt5/QtWidgets")
QT_LIBS=$(pkg-config --libs Qt5Core Qt5Gui Qt5Widgets 2>/dev/null || echo "-lQt5Core -lQt5Gui -lQt5Widgets")
GLIB_FLAGS=$(pkg-config --cflags --libs glib-2.0 gio-2.0 gobject-2.0 2>/dev/null || echo "-lglib-2.0 -lgio-2.0 -lgobject-2.0")

# 构建隔离层和测试目标
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGETS_DIR="$SCRIPT_DIR/../targets"

$AFL_COMPILER -std=c++11 -O2 -fPIC -DHAVE_AFL \
    -I$PEONY_INCLUDE_PATH \
    $QT_FLAGS \
    -o $TARGETS_DIR/peony_sync_test \
    $TARGETS_DIR/PeonyTestHarness.cpp \
    $TARGETS_DIR/peony_sync_test.cpp \
    -L$PEONY_LIB_PATH -lpeony \
    $GLIB_FLAGS \
    $QT_LIBS

if [ $? -eq 0 ]; then
    echo "✅ Build successful"
    echo "Test target: targets/peony_sync_test"
else
    echo "❌ Build failed"
    exit 1
fi
