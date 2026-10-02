#!/bin/bash
# 测试运行脚本
# 此脚本位于 tests/ 目录下，用于编译和运行所有单元测试

set -e

echo "========================================="
echo "Platform Security Patrol - Test Runner"
echo "========================================="
echo ""

# 获取项目根目录（tests 目录的父目录）
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# 检查项目根目录是否存在 CMakeLists.txt
if [ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found in project root."
    echo "Please ensure this script is in the tests/ directory."
    exit 1
fi

# 创建构建目录
BUILD_DIR="$PROJECT_ROOT/build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 配置项目（启用测试）
echo "Configuring project with tests enabled..."
cmake "$PROJECT_ROOT" -DBUILD_TESTING=ON

# 编译
echo ""
echo "Building project and tests..."
make -j$(nproc)

# 运行测试
echo ""
echo "========================================="
echo "Running tests..."
echo "========================================="
ctest --verbose --output-on-failure

# 返回退出码
TEST_RESULT=$?

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo "✅ All tests passed!"
else
    echo "❌ Some tests failed. Check the output above for details."
fi

exit $TEST_RESULT
