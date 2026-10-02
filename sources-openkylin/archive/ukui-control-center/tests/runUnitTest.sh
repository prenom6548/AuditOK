#!/bin/bash
export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
cd "$(dirname "$0")" || exit 1
# 启用lastpipe解决子shell变量：隔离问题
shopt -s lastpipe 
BASE_DIR=$(pwd)
PROJECT_ROOT=$(cd "${BASE_DIR}/.." && pwd)

# 函数：运行单元测试
run_unit_tests() {
    local pattern="unit_test_*"

    cd build

    # 遍历所有匹配模式的目录
    for dir in $pattern; do
        if [ -d "$dir" ] && [ -x "$dir/$(basename "$dir")" ]; then
            echo "Running tests in $dir..."
            # 执行单元测试
            (cd "$dir" && "./$(basename "$dir")")
            # 收集代码覆盖率数据
            echo "Collecting coverage data..."
            (cd "$dir" && (find ./ -name '*.o' | xargs gcov --preserve-paths))
        else
            echo "Skipping non-existent or non-executable directory: $dir, $dir/$(basename "$dir")"
        fi
    done
}

# 运行各个单元测试
echo "Running unit tests..."
run_unit_tests

GCOVR_XML_OUTPUT="${GCOVR_XML_OUTPUT:-${PROJECT_ROOT}/ukui-control-center.xml}"

gcovr -r  "$PROJECT_ROOT"  -x --object-directory="$BASE_DIR/build" --sonarqube "${GCOVR_XML_OUTPUT}" --exclude-branches-by-pattern=".*" --exclude ".*moc_.*"

