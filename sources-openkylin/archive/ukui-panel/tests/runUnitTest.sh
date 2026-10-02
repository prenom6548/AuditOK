#!/bin/bash

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
export HOME=/tmp

cd "$(dirname "$0")" || exit -1

shopt -s lastpipe
BASE_DIR=$(pwd)
if [ $? -ne 0 ]; then
    echo "错误：无法获取 tests 目录绝对路径" >&2
    exit -1
fi

if ! command -v file > /dev/null 2>&1; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit -1
fi

RUNTIME_DIR="${BASE_DIR}/.xdg-runtime"
mkdir -p "${RUNTIME_DIR}"
if [ $? -ne 0 ]; then
    echo "错误：无法创建 XDG_RUNTIME_DIR 目录 ${RUNTIME_DIR}" >&2
    exit -1
fi

chmod 700 "${RUNTIME_DIR}"
if [ $? -ne 0 ]; then
    echo "错误：无法设置 XDG_RUNTIME_DIR 目录权限" >&2
    exit -1
fi

export XDG_RUNTIME_DIR="${RUNTIME_DIR}"

BUILD_DIR="${BASE_DIR}/build"
if [ ! -d "${BUILD_DIR}" ]; then
    echo "错误：未找到构建目录 ${BUILD_DIR}，请先执行 auto_test.sh" >&2
    exit -1
fi

if [ ! -w "${BUILD_DIR}" ]; then
    echo "错误：当前用户对 ${BUILD_DIR} 没有写权限，测试程序无法生成覆盖率文件。" >&2
    echo "请重新执行 sudo ./auto_test.sh，或手动修正 build 目录属主后再运行本脚本。" >&2
    exit -1
fi

binary_count=0
failed_count=0

echo "===== 开始递归扫描 ${BASE_DIR} 目录下有执行权限的二进制程序 ====="
echo

find "${BUILD_DIR}" -type f -print0 | while IFS= read -r -d '' file_path; do
    if [ -L "${file_path}" ]; then
        continue
    fi

    case "${file_path}" in
        */CMakeFiles/*|*/CMakeScratch/*|*/_autogen/*|*/CompilerIdC/*|*/CompilerIdCXX/*)
            continue
            ;;
    esac

    if [[ "${file_path}" =~ \.(so|a|o)$ ]]; then
        continue
    fi

    if [ ! -x "${file_path}" ]; then
        continue
    fi

    if [[ ! "$(basename "${file_path}")" =~ Test$ ]]; then
        continue
    fi

    file_info=$(file -b "${file_path}")
    if [ $? -ne 0 ]; then
        echo "警告：无法识别文件类型，跳过 ${file_path}" >&2
        continue
    fi

    if echo "${file_info}" | grep -qiE 'elf|executable|binary' && \
       echo "${file_info}" | grep -qiEv 'text|ascii|script'; then
        echo "=============运行程序：${file_path}=================="
        "${file_path}"
        test_ret=$?
        echo "=============程序结束：${file_path}=================="

        binary_count=$((binary_count + 1))
        if [ ${test_ret} -ne 0 ]; then
            failed_count=$((failed_count + 1))
        fi
    fi
done

echo

if [ "${binary_count}" -eq 0 ]; then
    echo "错误：在 ${BASE_DIR} 及其子目录中未扫描到有执行权限的二进制程序" >&2
    exit -1
fi

if [ "${failed_count}" -ne 0 ]; then
    echo "错误：共有 ${failed_count} 个单元测试程序执行失败" >&2
    exit -1
fi

echo "===== 脚本运行结束，共找到 ${binary_count} 个符合条件的二进制程序 ====="
exit 0
