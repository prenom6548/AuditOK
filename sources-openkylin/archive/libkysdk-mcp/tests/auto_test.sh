#!/bin/bash

# Install gcovr 7.1
echo "Installing gcovr 7.1..."
pip install gcovr==7.1 -i https://maven.kylinos.cn/repository/pypi-group/simple/ --break-system-packages

# Install dependencies
echo "Installing dependencies..."
apt install -y \
    build-essential \
    cmake \
    pkg-config \
    libgtest-dev \
    libjsoncpp-dev \
    libglib2.0-dev \
    libgmock-dev \
    libssl-dev

if [ $? -ne 0 ]; then
    echo "Error: apt install failed."
    exit 1
fi

# Build unit tests only
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "Running CMake..."
cmake "$SCRIPT_DIR" -DENABLE_COVERAGE=ON

if [ $? -ne 0 ]; then
    echo "Error: tests cmake failed."
    exit 1
fi

echo "Building tests..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error: tests make failed."
    exit 1
fi

echo ""
echo "========================================"
echo "Build successful!"
echo "========================================"
echo ""
echo "Test binaries are located in:"
echo "  $(pwd)/"
echo ""
echo "Run tests with:"
echo "  ./runUnitTest.sh"
echo ""

exit 0
