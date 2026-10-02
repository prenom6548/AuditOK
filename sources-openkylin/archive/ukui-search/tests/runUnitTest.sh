#!/bin/bash

set -u

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
export QTEST_FUNCTION_TIMEOUT="${QTEST_FUNCTION_TIMEOUT:-60000}"
export UNIT_TEST_BINARY_TIMEOUT="${UNIT_TEST_BINARY_TIMEOUT:-90s}"
unset G_DEBUG

cd "$(dirname "$0")" || exit 1
shopt -s lastpipe

BASE_DIR="$(pwd)/build"
SOURCE_DIR="$(cd .. && pwd)"
PACKAGE_NAME="$(awk '/^Source:/ {print $2; exit}' "${SOURCE_DIR}/debian/control" 2>/dev/null)"
if [ -z "${PACKAGE_NAME}" ]; then
    PACKAGE_NAME="$(basename "${SOURCE_DIR}")"
fi
COVERAGE_XML="${SOURCE_DIR}/${PACKAGE_NAME}-coverage.xml"

KEEP_BUILD=0
if [ "${1:-}" = "--keep-build" ]; then
    KEEP_BUILD=1
    shift
fi

TEST_TARGETS=(
    perftoolsCoreTest
    perftoolsWhiteboxTest
    perftoolsMallocExtensionDefaultsTest
    perftoolsSharedAllocatorTest
    perftoolsSampledAllocatorTest
    perftoolsLargeAllocReportEnabledTest
    perftoolsLargeAllocReportNegativeThresholdTest
    perftoolsPageHeapWhiteboxTest
    perftoolsMemfsAllocatorTest
    perftoolsProcMapsWhiteboxTest
    traditional2SimplifiedTrieTest
    traditional2SimplifiedTest
    pinyin4cppTrieTest
    pinyin4cppDictTrieTest
    storageBaseWhiteboxTest
    hanziToPinyinTest
    chineseSegmentationTest
    cppJiebaWhiteboxTest
    cppJiebaFixtureDictTest
    iconLoaderTest
    dataQueueTest
    searchPluginIfaceWhiteboxTest
    resultItemTest
    documentTest
    searchResultPropertyInfoTest
    actionTransmiterTest
    actionLabelTest
    searchControllerTest
    appDataUtilsTest
    applicationPropertyHelperTest
    applicationInfoLogUtilsTest
    appInfoDbusArgumentTest
    pendingFileTest
    fileExtractionResultTest
    indexStatusRecorderTest
    indexSmallComponentsTest
    fileContentIndexerWhiteboxTest
    fileSystemWatcherWhiteboxTest
    remoteFileEventHelperWhiteboxTest
    fileWatcherWhiteboxTest
    ukuiSearchQdbusWhiteboxTest
    dirWatcherTest
    globalSettingsTest
    fileUtilsTest
    separationLineTest
    libsearchWhiteboxTest
    searchPluginManagerWhiteboxTest
    appSearchTaskWhiteboxTest
    appSearchPluginWhiteboxTest
    aiSearchPluginWhiteboxTest
    dataCollecterWhiteboxTest
    appInfoTableWhiteboxTest
    aiIndexerWhiteboxTest
    pluginKeywordSearchWhiteboxTest
    indexJobsWhiteboxTest
    pluginManagerWhiteboxTest
    searchLineEditTest
    createIndexAskDialogTest
    oneResultTest
    resultViewTest
    resultWidgetWhiteboxTest
    bestListViewWhiteboxTest
    stackPagesWhiteboxTest
    searchResultModelTest
    searchResultManagerTest
    bestListModelTest
    flowLayoutTest
    showMoreLabelTest
    titleLabelTest
    cgroupHelperTest
    pendingAppInfoTest
    pendingAppInfoQueueTest
    signalTransformerTest
    searchDirTest
    configTest
    volumeTest
    dirWatcherServiceTest
    indexDatabasePluginWhiteboxTest
)

TEST_STATE_ROOT=""

SELECTED_TARGETS=("$@")
if [ "${#SELECTED_TARGETS[@]}" -gt 0 ]; then
    TEST_TARGETS=("${SELECTED_TARGETS[@]}")
fi

should_run_binary() {
    local binary_name="$1"
    if [ "${#SELECTED_TARGETS[@]}" -eq 0 ]; then
        return 0
    fi

    local target
    for target in "${SELECTED_TARGETS[@]}"; do
        if [ "${binary_name}" = "${target}" ]; then
            return 0
        fi
    done
    return 1
}

if [ "${KEEP_BUILD}" -eq 0 ] && [ -d "${BASE_DIR}" ]; then
    echo "Removing existing build directory..."
    rm -rf "${BASE_DIR}"
fi

mkdir -p "${BASE_DIR}"
cd "${BASE_DIR}" || exit 1

echo "running cmake"
cmake ../.. \
    -DBUILD_TEST=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS_DEBUG="-O0 -g --coverage" \
    -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g --coverage" \
    -DCMAKE_EXE_LINKER_FLAGS_DEBUG="--coverage" \
    -DCMAKE_SHARED_LINKER_FLAGS_DEBUG="--coverage"
if [ $? -ne 0 ]; then
    echo "Error: cmake failed."
    exit 1
fi

echo "Running test target build..."
BUILD_JOBS="$(nproc 2>/dev/null || echo 4)"
if [ "${BUILD_JOBS}" -gt 1 ]; then
    BUILD_JOBS=$((BUILD_JOBS - 1))
fi
cmake --build . --target "${TEST_TARGETS[@]}" -j"${BUILD_JOBS}"
if [ $? -ne 0 ]; then
    echo "Error: test target build failed."
    exit 1
fi

SCHEMA_DIR="${BASE_DIR}/gsettings-schemas"
mkdir -p "${SCHEMA_DIR}"
cp "${SOURCE_DIR}/data/org.ukui.search.data.gschema.xml" "${SCHEMA_DIR}/"
if command -v glib-compile-schemas >/dev/null 2>&1; then
    glib-compile-schemas "${SCHEMA_DIR}"
else
    echo "错误：系统未安装 glib-compile-schemas，无法为索引白盒用例准备 GSettings schema。" >&2
    exit 1
fi

TEST_STATE_ROOT="$(mktemp -d "${TMPDIR:-/var/tmp}/ukui-search-ut.XXXXXX")"
if [ -z "${TEST_STATE_ROOT}" ] || [ ! -d "${TEST_STATE_ROOT}" ]; then
    echo "错误：无法创建白盒测试临时运行目录。" >&2
    exit 1
fi

TEST_HOME="${TEST_STATE_ROOT}/home"
TEST_RUNTIME_DIR="${TEST_STATE_ROOT}/run"
mkdir -p "${TEST_HOME}/.cache" "${TEST_HOME}/.config" "${TEST_HOME}/.local/share" "${TEST_RUNTIME_DIR}"
chmod 700 "${TEST_RUNTIME_DIR}"
export HOME="${TEST_HOME}"
export XDG_CACHE_HOME="${TEST_HOME}/.cache"
export XDG_CONFIG_HOME="${TEST_HOME}/.config"
export XDG_DATA_HOME="${TEST_HOME}/.local/share"
export XDG_RUNTIME_DIR="${TEST_RUNTIME_DIR}"
export GSETTINGS_SCHEMA_DIR="${SCHEMA_DIR}"
export GSETTINGS_BACKEND=memory

if ! command -v dbus-daemon >/dev/null 2>&1; then
    echo "错误：系统未安装 dbus-daemon，无法为白盒测试准备 session bus。" >&2
    exit 1
fi

rm -f "${TEST_RUNTIME_DIR}/bus"
dbus_stderr_log="${TEST_RUNTIME_DIR}/dbus-daemon.stderr.log"
rm -f "${dbus_stderr_log}"
if ! dbus_daemon_output="$(dbus-daemon --session \
    --address="unix:path=${TEST_RUNTIME_DIR}/bus" \
    --fork \
    --print-address=1 \
    --print-pid=1 2>"${dbus_stderr_log}")"; then
    echo "错误：启动 dbus-daemon 失败" >&2
    if [ -s "${dbus_stderr_log}" ]; then
        cat "${dbus_stderr_log}" >&2
    fi
    exit 1
fi

export DBUS_SESSION_BUS_ADDRESS="$(printf '%s\n' "${dbus_daemon_output}" | sed -n '1p')"
DBUS_SESSION_BUS_PID="$(printf '%s\n' "${dbus_daemon_output}" | sed -n '2p')"
if [ -z "${DBUS_SESSION_BUS_ADDRESS}" ] || [ -z "${DBUS_SESSION_BUS_PID}" ]; then
    echo "错误：dbus-daemon 输出不完整，无法初始化 session bus。" >&2
    printf '%s\n' "${dbus_daemon_output}" >&2
    if [ -s "${dbus_stderr_log}" ]; then
        cat "${dbus_stderr_log}" >&2
    fi
    exit 1
fi
cleanup_dbus_daemon() {
    if [ -n "${DBUS_SESSION_BUS_PID:-}" ]; then
        kill "${DBUS_SESSION_BUS_PID}" 2>/dev/null || true
        wait "${DBUS_SESSION_BUS_PID}" 2>/dev/null || true
    fi
    if [ -n "${TEST_STATE_ROOT:-}" ] && [ -d "${TEST_STATE_ROOT}" ]; then
        rm -rf "${TEST_STATE_ROOT}"
    fi
}
trap cleanup_dbus_daemon EXIT

if ! command -v file >/dev/null 2>&1; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit 1
fi

find "${BASE_DIR}" -name '*.gcda' -delete

binary_count=0
failed_count=0
failed_binaries=()

run_test_binary() {
    local binary_path="$1"
    local binary_name
    local test_home=""
    binary_name="$(basename "${binary_path}")"

    if [ "${binary_name}" = "perftoolsPageHeapWhiteboxTest" ]; then
        if command -v timeout >/dev/null 2>&1; then
            MALLOCSTATS=2 timeout --kill-after=5s "${UNIT_TEST_BINARY_TIMEOUT}" "${binary_path}"
        else
            MALLOCSTATS=2 "${binary_path}"
        fi
        return $?
    fi

    if [ "${binary_name}" = "indexDatabasePluginWhiteboxTest" ]; then
        test_home="$(mktemp -d "${TEST_STATE_ROOT}/index-db-plugin-home.XXXXXX")"
        mkdir -p "${test_home}/.cache" "${test_home}/.config" "${test_home}/.local/share"
        if command -v timeout >/dev/null 2>&1; then
            HOME="${test_home}" XDG_CACHE_HOME="${test_home}/.cache" XDG_CONFIG_HOME="${test_home}/.config" XDG_DATA_HOME="${test_home}/.local/share" UKUI_SEARCH_WHITEBOX_HOME="${test_home}" timeout --kill-after=5s "${UNIT_TEST_BINARY_TIMEOUT}" "${binary_path}"
        else
            HOME="${test_home}" XDG_CACHE_HOME="${test_home}/.cache" XDG_CONFIG_HOME="${test_home}/.config" XDG_DATA_HOME="${test_home}/.local/share" UKUI_SEARCH_WHITEBOX_HOME="${test_home}" "${binary_path}"
        fi
        return $?
    fi

    if [ "${binary_name}" = "indexJobsWhiteboxTest" ]; then
        test_home="$(mktemp -d "${TEST_STATE_ROOT}/index-jobs-home.XXXXXX")"
        mkdir -p "${test_home}/.cache" "${test_home}/.config" "${test_home}/.local/share"
        if command -v timeout >/dev/null 2>&1; then
            HOME="${test_home}" XDG_CACHE_HOME="${test_home}/.cache" XDG_CONFIG_HOME="${test_home}/.config" XDG_DATA_HOME="${test_home}/.local/share" UKUI_SEARCH_WHITEBOX_HOME="${test_home}" timeout --kill-after=5s "${UNIT_TEST_BINARY_TIMEOUT}" "${binary_path}"
        else
            HOME="${test_home}" XDG_CACHE_HOME="${test_home}/.cache" XDG_CONFIG_HOME="${test_home}/.config" XDG_DATA_HOME="${test_home}/.local/share" UKUI_SEARCH_WHITEBOX_HOME="${test_home}" "${binary_path}"
        fi
        return $?
    fi

    if command -v timeout >/dev/null 2>&1; then
        timeout --kill-after=5s "${UNIT_TEST_BINARY_TIMEOUT}" "${binary_path}"
    else
        "${binary_path}"
    fi
}

echo "===== 开始递归扫描 ${BASE_DIR} 目录下有执行权限的二进制程序 ====="
echo

find "${BASE_DIR}" -type f -print0 | while IFS= read -r -d $'\0' file; do
    if [ -L "${file}" ]; then
        continue
    fi

    if [[ "${file}" == */CMakeFiles/* ]]; then
        continue
    fi

    if [[ "${file}" =~ \.(so|a|o)$ ]]; then
        continue
    fi

    if [ ! -x "${file}" ]; then
        continue
    fi

    if file -b "${file}" | grep -qiE 'elf|executable|binary' && \
        file -b "${file}" | grep -qiEv 'shared object|text|ascii|script'; then
        if ! should_run_binary "$(basename "${file}")"; then
            continue
        fi
        echo "=============运行程序：${file}=================="
        export UKUI_SEARCH_CURRENT_TEST_BINARY="${file}"
        run_test_binary "${file}"
        status=$?
        echo "=============程序结束：${file}，退出码：${status}=================="
        ((binary_count++))
        if [ "${status}" -ne 0 ]; then
            if [ "${status}" -eq 124 ]; then
                echo "错误：${file} 执行超时，已在 ${UNIT_TEST_BINARY_TIMEOUT} 后被终止"
            fi
            ((failed_count++))
            failed_binaries+=("${file}")
        fi
    fi
done

echo

if [ "${binary_count}" -eq 0 ]; then
    echo "错误：在 ${BASE_DIR} 及其子目录中未扫描到有执行权限的二进制程序"
    exit 2
fi

if [ "${failed_count}" -ne 0 ]; then
    echo "错误：共 ${failed_count}/${binary_count} 个单元测试二进制执行失败"
    echo "失败二进制列表："
    printf '  %s\n' "${failed_binaries[@]}"
    exit 1
fi

if ! command -v gcovr >/dev/null 2>&1; then
    echo "错误：系统未安装 gcovr，无法生成覆盖率 XML。请先执行 tests/auto_test.sh。"
    exit 1
fi

echo "===== 开始生成 gcovr 覆盖率文件：${COVERAGE_XML} ====="
gcovr -r "${SOURCE_DIR}" \
    --object-directory="${BASE_DIR}" \
    --merge-mode-functions=merge-use-line-min \
    --gcov-ignore-parse-errors=negative_hits.warn_once_per_file \
    --sonarqube "${COVERAGE_XML}"
if [ $? -ne 0 ]; then
    echo "错误：gcovr 覆盖率文件生成失败"
    exit 1
fi

echo "===== 覆盖率文件已生成：${COVERAGE_XML} ====="
echo "===== 脚本运行结束，共找到并执行 ${binary_count} 个符合条件的二进制程序 ====="
exit 0
