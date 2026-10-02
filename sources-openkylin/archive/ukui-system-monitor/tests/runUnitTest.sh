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
    cd build

    ./unit_test_systemmonitor
}

# 运行各个单元测试
echo "Running unit tests..."
run_unit_tests

GCOVR_XML_OUTPUT="${GCOVR_XML_OUTPUT:-${PROJECT_ROOT}/ukui-system-monitor.xml}"

gcovr -r  "$PROJECT_ROOT"  -x --object-directory="$BASE_DIR/build" --sonarqube "${GCOVR_XML_OUTPUT}" --exclude-branches-by-pattern=".*" --exclude ".*moc_.*"

