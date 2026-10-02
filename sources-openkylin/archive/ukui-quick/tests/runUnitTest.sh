#!/bin/bash
# 白盒单元测试 - 执行入口脚本
# 功能：执行 CTest 注册测试清单，生成覆盖率报告

set -u
set -o pipefail

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
RESULTS_DIR="$PROJECT_DIR/results"
LOG_FILE="$RESULTS_DIR/unit_test.log"
COVERAGE_FILE="$PROJECT_DIR/ukui-quick-coverage.xml"
COBERTURA_OUTPUT_FILE="$PROJECT_DIR/ukui-quick-coverage.output"
RESULTS_COVERAGE_FILE="$RESULTS_DIR/coverage.xml"

CTEST_STATUS=0

# Keep the source filter and exclusions identical for both gcovr modes.  The
# object-directory location is the only difference between the normal and
# compatibility invocations.
COVERAGE_FILTER_ARGS=(
    --exclude-branches-by-pattern ".*"
    --merge-mode-functions=separate
    --filter "$PROJECT_DIR/(core|items|platform|framework|modules)/.*"
    --exclude ".*build.*/.*"
    --exclude ".*/test/.*"
    --exclude ".*/autotest/.*"
    --exclude ".*/tests/.*"
    --exclude ".*/wayland/protocol/.*"
    --exclude ".*/debian/.*"
    --exclude ".*/cmake-extend/.*"
    --exclude ".*/scenegraph/.*"
    --exclude ".*/generated/.*"
    --exclude ".*/lottie-runtime-benchmark\\.cpp$"
    --exclude ".*/(fixture|fixtures)/.*"
    # Do not merge test-plugin/test-seam objects into production source data.
    # Their source files are excluded above as well, but their object data can
    # otherwise make the same production path appear with test-only macros.
    --gcov-exclude ".*/CMakeFiles/[^/]*test[^/]*/.*"
    --gcov-exclude ".*/CMakeFiles/[^/]*benchmark[^/]*/.*"
)

fail()
{
    echo ">>> $1" >&2
    exit -1
}

generate_primary_coverage()
{
    gcovr -r "$PROJECT_DIR" \
        -j 1 \
        -x \
        --object-directory "$BUILD_DIR" \
        --output "$COBERTURA_OUTPUT_FILE" \
        --sonarqube "$COVERAGE_FILE" \
        "${COVERAGE_FILTER_ARGS[@]}" \
        "$BUILD_DIR"
}

generate_fallback_coverage()
{
    gcovr -r "$PROJECT_DIR" \
        -j 1 \
        -x \
        --object-directory "$PROJECT_DIR" \
        --output "$COBERTURA_OUTPUT_FILE" \
        --sonarqube "$COVERAGE_FILE" \
        "${COVERAGE_FILTER_ARGS[@]}" \
        "$BUILD_DIR"
}

refresh_qt_autogen_sources()
{
    local build_config
    local autogen_info_files=()
    local autorcc_info_files=()

    build_config="$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' "$BUILD_DIR/CMakeCache.txt" | head -n 1)"
    if [ -z "$build_config" ]; then
        build_config="Debug"
    fi
    build_config="${build_config,,}"

    mapfile -t autogen_info_files < <(
        find "$BUILD_DIR" -type f -name "AutogenInfo.json" | sort
    )
    mapfile -t autorcc_info_files < <(
        find "$BUILD_DIR" -type f -name "AutoRcc_*_Info.json" | sort
    )

    if [ "${#autogen_info_files[@]}" -eq 0 ] && [ "${#autorcc_info_files[@]}" -eq 0 ]; then
        return 0
    fi

    echo ">>> 刷新 Qt 自动生成源码..."
    for info_file in "${autogen_info_files[@]}"; do
        if ! cmake -E cmake_autogen "$info_file" "$build_config" >> "$LOG_FILE" 2>&1; then
            fail "Qt 自动生成源码刷新失败，请查看 $LOG_FILE"
        fi
    done

    for info_file in "${autorcc_info_files[@]}"; do
        if ! cmake -E cmake_autorcc "$info_file" "$build_config" >> "$LOG_FILE" 2>&1; then
            fail "Qt 自动生成源码刷新失败，请查看 $LOG_FILE"
        fi
    done
}

# 创建结果输出目录
mkdir -p "$RESULTS_DIR" || fail "结果目录创建失败"
: > "$LOG_FILE" || fail "日志文件初始化失败"

if [ ! -d "$BUILD_DIR" ]; then
    fail "构建目录不存在，请先执行 tests/auto_test.sh"
fi

if ! command -v gcovr >/dev/null 2>&1; then
    fail "未找到 gcovr，请先安装覆盖率工具"
fi

if ! command -v ctest >/dev/null 2>&1; then
    fail "未找到 ctest，请先安装 CMake/CTest"
fi

echo ">>> 清理旧覆盖率数据..."
find "$BUILD_DIR" -type f -name "*.gcda" -delete 2>/dev/null

# CTest's registered test manifest is the authoritative test set.  This keeps
# per-test environment, working directory, timeout, and script-based guards.
echo ">>> 执行 CTest 注册测试清单..."
ctest --test-dir "$BUILD_DIR" \
    --output-on-failure \
    --timeout 60 \
    --no-tests=error 2>&1 | tee -a "$LOG_FILE"
CTEST_STATUS=${PIPESTATUS[0]}
echo ">>> CTest 返回码: $CTEST_STATUS"

# 生成覆盖率报告（gcovr xml 格式）
echo ">>> 生成覆盖率报告..."

refresh_qt_autogen_sources
find "$BUILD_DIR" -type f -name "*.gcov" -delete 2>/dev/null
rm -f "$COVERAGE_FILE" "$COBERTURA_OUTPUT_FILE" "$RESULTS_COVERAGE_FILE"
if ! generate_primary_coverage >> "$LOG_FILE" 2>&1; then
    echo ">>> 首次覆盖率生成失败，回退到兼容模式..." | tee -a "$LOG_FILE"
    find "$BUILD_DIR" -type f -name "*.gcov" -delete 2>/dev/null
    rm -f "$COVERAGE_FILE" "$COBERTURA_OUTPUT_FILE" "$RESULTS_COVERAGE_FILE"
    if ! generate_fallback_coverage >> "$LOG_FILE" 2>&1; then
        fail "覆盖率报告生成失败，请查看 $LOG_FILE"
    fi
fi

# 确认覆盖率文件已生成
if [ ! -f "$COVERAGE_FILE" ]; then
    fail "覆盖率报告未生成"
fi

cp "$COVERAGE_FILE" "$RESULTS_COVERAGE_FILE" || fail "覆盖率报告复制失败"

echo ">>> 覆盖率报告已生成: $COVERAGE_FILE"

# 测试失败时返回 -1
if [ "$CTEST_STATUS" -ne 0 ]; then
    fail "CTest 存在失败测试，请查看 $LOG_FILE"
fi

exit 0
