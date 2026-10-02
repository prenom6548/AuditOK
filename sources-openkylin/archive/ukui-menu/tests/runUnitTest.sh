#!/bin/bash
set -u

check_status() {
    if [ "$1" -ne 0 ]; then
        echo "$2" >&2
        exit -1
    fi
}

if [ "${1:-}" != "--inside-dbus-session" ]; then
    if ! command -v dbus-run-session &> /dev/null; then
        echo "错误：系统未安装 dbus-run-session，无法隔离 D-Bus 会话！" >&2
        exit -1
    fi

    echo "===== 使用独立 D-Bus session 运行单元测试 ====="
    exec dbus-run-session -- "$0" --inside-dbus-session
fi

shift

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false;kf.windowsystem.warning=false"
export QT_FATAL_WARNINGS=0
export QT_OPENGL=software
export QT_XCB_GL_INTEGRATION=none
export XDG_RUNTIME_DIR="/tmp/ukui-sidebar-tests-runtime"
mkdir -p "${XDG_RUNTIME_DIR}"
check_status $? "错误：创建 XDG_RUNTIME_DIR 失败。"
chmod 700 "${XDG_RUNTIME_DIR}"
check_status $? "错误：设置 XDG_RUNTIME_DIR 权限失败。"
export GSETTINGS_BACKEND=memory
export HOME="${XDG_RUNTIME_DIR}"
export XDG_CACHE_HOME="${XDG_RUNTIME_DIR}/cache"
export XDG_CONFIG_HOME="${XDG_RUNTIME_DIR}/config"
mkdir -p "${XDG_CACHE_HOME}" "${XDG_CONFIG_HOME}"
check_status $? "错误：创建 GSettings 运行目录失败。"
export LIBGL_ALWAYS_SOFTWARE=1
export QT_QUICK_BACKEND=software
export QSG_RHI_BACKEND=software
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
check_status $? "错误：获取 tests 目录失败。"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
check_status $? "错误：获取项目根目录失败。"
BUILD_DIR="${SCRIPT_DIR}/build"
AUTOTEST_DIR="${BUILD_DIR}/autotest"
AUTOTEST_COMPAT_DIR="${BUILD_DIR}/autotests"
COVERAGE_XML="${BUILD_DIR}/coverage.xml"

resolve_test_dir() {
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

TEST_DIR="$(resolve_test_dir)"
check_status $? "错误：tests 目录不存在：${AUTOTEST_DIR} 或 ${AUTOTEST_COMPAT_DIR}"

if ! command -v file &> /dev/null; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit -1
fi

binary_count=0
failed_count=0
failed_tests=()
TEST_TIMEOUT_SECONDS="${TEST_TIMEOUT_SECONDS:-60}"

if ! [[ "${TEST_TIMEOUT_SECONDS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "错误：TEST_TIMEOUT_SECONDS 必须为正整数。" >&2
    exit -1
fi

if ! command -v timeout &> /dev/null; then
    echo "错误：系统未安装 timeout 命令，无法限制单元测试运行时间！" >&2
    exit -1
fi

is_test_binary() {
    local target="$1"
    local file_info
    local base_name

    if [ -L "${target}" ] || [ ! -f "${target}" ] || [ ! -x "${target}" ]; then
        return 1
    fi

    base_name="$(basename "${target}")"

    case "${target}" in
        *.sh|*.py|*.pl|*.so|*.a|*.o|*.cmake|*.txt)
            return 1
            ;;
    esac

    case "${base_name}" in
        *test|*Test)
            ;;
        *)
            return 1
            ;;
    esac

    file_info="$(file -b "${target}")"
    if ! printf '%s\n' "${file_info}" | grep -qiE 'elf|executable|binary'; then
        return 1
    fi

    if printf '%s\n' "${file_info}" | grep -qiE 'text|ascii|script'; then
        return 1
    fi

    return 0
}

echo "===== 开始扫描单元测试二进制程序 ====="
echo "测试目录：${TEST_DIR}"
echo "QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "QT_OPENGL=${QT_OPENGL}"
echo "LIBGL_ALWAYS_SOFTWARE=${LIBGL_ALWAYS_SOFTWARE}"
echo "QT_QUICK_BACKEND=${QT_QUICK_BACKEND}"
echo "QSG_RHI_BACKEND=${QSG_RHI_BACKEND}"
echo "GSETTINGS_BACKEND=${GSETTINGS_BACKEND}"
echo "TEST_TIMEOUT_SECONDS=${TEST_TIMEOUT_SECONDS}"
echo

if [ -z "${TEST_DIR}" ] || [ ! -d "${TEST_DIR}" ]; then
    echo "错误：tests 目录不存在：${TEST_DIR}" >&2
    exit -1
fi

find "${BUILD_DIR}" -type f -name '*.gcda' -delete
check_status $? "错误：清理旧的 .gcda 文件失败。"

while IFS= read -r -d '' file; do
    if ! is_test_binary "${file}"; then
        continue
    fi

    echo "=============运行程序：${file}=================="
    timeout --signal=TERM --kill-after=5s "${TEST_TIMEOUT_SECONDS}s" "${file}"
    exit_code=$?
    if [ "${exit_code}" -ne 0 ]; then
        failed_count=$((failed_count + 1))
        failed_tests+=("${file} (exit=${exit_code})")
        if [ "${exit_code}" -eq 124 ]; then
            echo "!!!!!!!!! 程序超时：${file}，限制：${TEST_TIMEOUT_SECONDS}s !!!!!!!!!"
        else
            echo "!!!!!!!!! 程序失败：${file}，退出码：${exit_code} !!!!!!!!!"
        fi
    fi
    echo "=============程序结束：${file}=================="
    binary_count=$((binary_count + 1))
done < <(find "${TEST_DIR}" -type f -print0)

echo
if [ "${binary_count}" -eq 0 ]; then
    echo "错误：未扫描到单元测试二进制程序" >&2
    exit -1
fi

echo "===== 脚本运行结束，共找到 ${binary_count} 个符合条件的二进制程序，失败 ${failed_count} 个 ====="

if [ "${failed_count}" -ne 0 ]; then
    echo "失败用例列表："
    printf '  %s\n' "${failed_tests[@]}"
    exit -1
fi

if ! command -v gcovr &> /dev/null; then
    echo "错误：系统未安装 gcovr，无法生成覆盖率报告！" >&2
    exit -1
fi

echo "===== 开始生成覆盖率报告 ====="
gcovr \
    --root "${PROJECT_DIR}" \
    --config "${PROJECT_DIR}/gcovr.cfg" \
    --object-directory "${BUILD_DIR}" \
    --gcov-exclude-directories ".*/CMakeFiles/[0-9][^/]*" \
    --merge-mode-functions merge-use-line-min \
    --sonarqube "${COVERAGE_XML}"
check_status $? "错误：生成覆盖率报告失败。"
echo "覆盖率报告：${COVERAGE_XML}"

exit 0
