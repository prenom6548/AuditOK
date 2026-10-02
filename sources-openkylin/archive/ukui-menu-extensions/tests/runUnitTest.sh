#!/bin/bash
export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/ukui-menu-tests-runtime}"
mkdir -p "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"
export LIBGL_ALWAYS_SOFTWARE=1
export QT_QUICK_BACKEND=software
cd "$(dirname "$0")" || exit 1
# 启用lastpipe解决子shell变量：隔离问题
shopt -s lastpipe
BASE_DIR=$(pwd)
PROJECT_DIR="$(cd "${BASE_DIR}/.." && pwd)"
DEFAULT_BUILD_DIR="${PROJECT_DIR}/build"
BUILD_DIR="${BUILD_DIR:-${DEFAULT_BUILD_DIR}}"
SEARCH_DIR_CANDIDATES=(
    "${BASE_DIR}"
    "${BUILD_DIR}/recent-file/tests"
    "${BUILD_DIR}/ernie-bot/tests"
    "${BUILD_DIR}/tests"
)

if [ "${BUILD_DIR}" = "${DEFAULT_BUILD_DIR}" ]; then
    SEARCH_DIR_CANDIDATES+=(
        "${BASE_DIR}/../../build/tests"
        "${BASE_DIR}/../build/recent-file/tests"
        "${BASE_DIR}/../build/ernie-bot/tests"
        "${BASE_DIR}/../build/tests"
        "${BASE_DIR}/build/recent-file/tests"
        "${BASE_DIR}/build/ernie-bot/tests"
    )
fi

SEARCH_DIRS=()
declare -A seen_dirs=()
for scan_dir in "${SEARCH_DIR_CANDIDATES[@]}"; do
    [ -d "${scan_dir}" ] || continue
    real_scan_dir="$(readlink -f "${scan_dir}")"
    if [ -z "${seen_dirs[${real_scan_dir}]}" ]; then
        SEARCH_DIRS+=("${real_scan_dir}")
        seen_dirs["${real_scan_dir}"]=1
    fi
done
# 检查依赖命令是否存在
if ! command -v file &> /dev/null; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit 1
fi
# 初始化计数器：仅统计「有执行权限+非.so/.a/.o后缀+二进制格式」的文件
binary_count=0
failed_count=0
# 提示信息
echo "===== 开始扫描单元测试二进制程序 ====="
echo
# 核心逻辑：遍历候选目录，过滤规则：有执行权限 + 非.so/.a/.o后缀 + 二进制格式
for scan_dir in "${SEARCH_DIRS[@]}"; do
    [ -d "${scan_dir}" ] || continue

    find "${scan_dir}" -maxdepth 2 -type f -print0 | while IFS= read -r -d $'\0' file; do
        # 跳过符号链接（如需包含，注释此行）
        if [ -L "${file}" ]; then
            continue
        fi
        # 第一步：过滤后缀为.so/.a/.o的文件（直接跳过）
        if [[ "${file}" =~ \.(so|a|o)$ ]]; then
            continue
        fi
        # 第二步：判断是否有可执行权限（无则跳过）
        if [ ! -x "${file}" ]; then
            continue
        fi
        # 第三步：只执行名称符合单元测试约定的程序
        if [[ ! "${file}" =~ [Tt]est$ ]]; then
            continue
        fi
        # 第四步：识别是否为二进制文件（过滤文本/脚本/空文件等）
        if file -b "${file}" | grep -qiE 'elf|executable|binary' && \
           file -b "${file}" | grep -qiEv 'text|ascii|script'; then
            echo "=============运行程序：${file}=================="
            if ! "${file}"; then
                failed_count=$((failed_count + 1))
            fi
            echo "=============程序结束：${file}=================="
            # 计数器+1（仅统计符合所有条件的文件）
            ((binary_count++))
        fi
    done
done
echo
# 检查计数器：无符合条件的文件则报错退出
if [ "${binary_count}" -eq 0 ]; then
    echo "错误：未扫描到单元测试二进制程序"
    exit 2  # 非零退出码标识错误
fi

echo "===== 脚本运行结束，共找到 ${binary_count} 个符合条件的二进制程序，失败 ${failed_count} 个 ====="

if [ "${failed_count}" -ne 0 ]; then
    exit 1
fi

exit 0
