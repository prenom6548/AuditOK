#!/bin/bash
# runUnitTest.sh
# 目的：发现并执行单元测试二进制（与 peony/tests/runUnitTest.sh 一致，作为自动化测试平台统一入口）。
# 脚本名称固定，须放在 tests 目录下，不可改名。
#
# 说明：本工程单测由 CMake 生成，默认路径为 <工程根>/build-coverage/tests/auto-test/test-settings；
# 同时递归扫描 tests/ 下已有构建目录（如 tests/build）；覆盖率对象目录为工程根下 build-coverage/。
# 图形：未设置 QT_QPA_PLATFORM 且存在 DISPLAY/WAYLAND_DISPLAY 时不使用 offscreen，便于需要人工点的单测。

# 不强制 offscreen：有 DISPLAY / WAYLAND_DISPLAY 时用默认 QPA（可正常弹窗）；无图形环境时再回退 offscreen
#if [ -z "${QT_QPA_PLATFORM:-}" ]; then
#   if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
#      export QT_QPA_PLATFORM=offscreen
# fi
#fi
export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="${QT_LOGGING_RULES:-qt.qpa.*=false}"
export QT_FATAL_WARNINGS="${QT_FATAL_WARNINGS:-0}"

cd "$(dirname "$0")" || exit 1

BASE_DIR=$(pwd)
PROJECT_ROOT=$(cd "${BASE_DIR}/.." && pwd)
BUILD_COVERAGE="${PROJECT_ROOT}/build-coverage"

# 运行期加载本仓库 CMake 构建出的 libqt5-ukui-style（与 test-settings 链接一致）
if [ -d "${BUILD_COVERAGE}/libqt5-ukui-style" ]; then
    export LD_LIBRARY_PATH="${BUILD_COVERAGE}/libqt5-ukui-style:${LD_LIBRARY_PATH:-}"
fi
if [ -d "${BUILD_COVERAGE}/qt5-ukui-platformtheme" ]; then
    export LD_LIBRARY_PATH="${BUILD_COVERAGE}/qt5-ukui-platformtheme:${LD_LIBRARY_PATH:-}"
fi

# 若曾用 root 编译，可能导致运行期无法写 .gcda
for d in "${BASE_DIR}/build" "${BUILD_COVERAGE}"; do
    if [ -d "${d}" ] && [ ! -w "${d}" ]; then
        echo "错误：目录不可写，无法生成覆盖率文件: ${d}" >&2
        echo "请执行后重试: sudo chown -R $(id -un):$(id -gn) ${d}" >&2
        exit 1
    fi
done

if ! command -v file &> /dev/null; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit 1
fi

binary_count=0
failed_count=0

# 扫描起始目录：tests/ 与 CMake 默认单测输出树
FIND_ROOTS=("${BASE_DIR}")
if [ -d "${BUILD_COVERAGE}/tests" ]; then
    FIND_ROOTS+=("${BUILD_COVERAGE}/tests")
fi

echo "===== 开始扫描单元测试二进制（${FIND_ROOTS[*]}）====="
echo

while IFS= read -r -d '' file; do
    if [ -L "${file}" ]; then
        continue
    fi
    if [[ "${file}" =~ \.(so|a|o)$ ]]; then
        continue
    fi
    # 排除 CMake 内部与 autogen 中间文件
    if [[ "${file}" == *"/CMakeFiles/"* ]] || [[ "${file}" == *"_autogen/"* ]]; then
        continue
    fi
    if [ ! -x "${file}" ]; then
        continue
    fi
    if ! file -b "${file}" | grep -qiE 'elf|executable|binary'; then
        continue
    fi
    if file -b "${file}" | grep -qiE 'text|ascii|script'; then
        continue
    fi

    base=$(basename "${file}")
    echo "=============运行程序：${file}=================="
    "${file}"
    rc=$?
    if [ "${rc}" -ne 0 ]; then
        echo "-------------程序失败（exit=${rc}）：${file}-------------" >&2
        failed_count=$((failed_count + 1))
    fi
    echo "=============程序结束：${file}=================="
    binary_count=$((binary_count + 1))
done < <(find "${FIND_ROOTS[@]}" -type f -print0 2>/dev/null)

echo

if [ "${binary_count}" -eq 0 ]; then
    echo "错误：未找到可执行的单元测试二进制。" >&2
    echo "请先编译，例如: bash ${BASE_DIR}/auto_test.sh 或 cmake --build ${BUILD_COVERAGE} --target test-settings" >&2
    exit 2
fi

echo "===== 共执行 ${binary_count} 个符合条件的二进制程序 ====="

GCOVR_XML_OUTPUT="${GCOVR_XML_OUTPUT:-${PROJECT_ROOT}/qt5-ukui-platformtheme.xml}"

if ! command -v gcovr &> /dev/null; then
    echo "警告：未安装 gcovr，跳过 XML 生成。" >&2
else
    if [ ! -d "${BUILD_COVERAGE}" ]; then
        echo "错误：缺少 ${BUILD_COVERAGE}，无法生成覆盖率 XML（需带覆盖率选项的 CMake 构建）。" >&2
        exit 1
    fi
    chmod -R u+w "${BUILD_COVERAGE}" 2>/dev/null || true

    args=(
        --root "${PROJECT_ROOT}"
        --object-directory "${BUILD_COVERAGE}"
        --sonarqube "${GCOVR_XML_OUTPUT}"
        --exclude-branches-by-pattern='.*'
    )

    if [ "${PLATFORMTHEME_GCOVR_STRICT:-0}" != 1 ]; then
        if gcovr --help 2>&1 | grep -q 'source_not_found'; then
            args+=(--gcov-ignore-errors=all)
        elif gcovr --help 2>&1 | grep -q 'gcov-ignore-errors'; then
            args+=(--gcov-ignore-errors=no_working_dir_found)
        fi
    fi
    # 多个单测目标静态链入同一 .cpp 时会产生多份 .gcda，需合并函数行号
    if gcovr --help 2>&1 | grep -q 'merge-mode-functions'; then
        args+=(--merge-mode-functions=merge-use-line-max)
    fi

    echo "===== 开始生成覆盖率 XML：${GCOVR_XML_OUTPUT} ====="
    if ! gcovr "${args[@]}"; then
        echo "错误：覆盖率 XML 生成失败。" >&2
        exit 1
    fi
fi

if [ "${failed_count}" -gt 0 ]; then
    echo "错误：共有 ${failed_count} 个测试程序执行失败。" >&2
    exit 1
fi

exit 0
