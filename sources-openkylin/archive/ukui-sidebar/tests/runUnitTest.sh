#!/bin/bash
set -u

if ! command -v dbus-run-session &> /dev/null; then
    echo "错误：系统未安装 dbus-run-session，无法隔离 D-Bus 会话！" >&2
    exit 1
fi

export QT_QPA_PLATFORM=offscreen
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_FATAL_WARNINGS=0
export QT_OPENGL=software
export QT_XCB_GL_INTEGRATION=none
export XDG_RUNTIME_DIR="/tmp/ukui-sidebar-tests-runtime"
mkdir -p "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"
export LIBGL_ALWAYS_SOFTWARE=1
export QT_QUICK_BACKEND=software
export QSG_RHI_BACKEND=software
cd "$(dirname "$0")" || exit 1
# 启用lastpipe解决子shell变量：隔离问题
shopt -s lastpipe
BASE_DIR=$(pwd)
SEARCH_DIR="${BASE_DIR}/build/autotests"
# 检查依赖命令是否存在
if ! command -v file &> /dev/null; then
    echo "错误：系统未安装 file 命令，无法识别二进制文件！" >&2
    exit 1
fi
# 初始化计数器：仅统计「有执行权限+非.so/.a/.o后缀+二进制格式」的文件
binary_count=0
failed_count=0
failed_tests=()
# 提示信息
echo "===== 开始扫描单元测试二进制程序 ====="
echo "每个测试二进制使用独立 D-Bus session 运行"
echo "测试目录：${SEARCH_DIR}"
echo "QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "QT_OPENGL=${QT_OPENGL}"
echo "LIBGL_ALWAYS_SOFTWARE=${LIBGL_ALWAYS_SOFTWARE}"
echo "QT_QUICK_BACKEND=${QT_QUICK_BACKEND}"
echo "QSG_RHI_BACKEND=${QSG_RHI_BACKEND}"
echo
# 核心逻辑：遍历 autotests 目录，过滤规则：有执行权限 + 非.so/.a/.o后缀 + 二进制格式
if [ ! -d "${SEARCH_DIR}" ]; then
    echo "错误：单元测试目录不存在：${SEARCH_DIR}"
    exit 2
fi

find "${SEARCH_DIR}" -type f -print0 | while IFS= read -r -d $'\0' file; do
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
        dbus-run-session -- "${file}"
        exit_code=$?
        if [ "${exit_code}" -ne 0 ]; then
            failed_count=$((failed_count + 1))
            failed_tests+=("${file} (exit=${exit_code})")
            echo "!!!!!!!!! 程序失败：${file}，退出码：${exit_code} !!!!!!!!!"
        fi
        echo "=============程序结束：${file}=================="
        # 计数器+1（仅统计符合所有条件的文件）
        ((binary_count++))
    fi
done
echo
# 检查计数器：无符合条件的文件则报错退出
if [ "${binary_count}" -eq 0 ]; then
    echo "错误：未扫描到单元测试二进制程序"
    exit 2  # 非零退出码标识错误
fi

echo "===== 脚本运行结束，共找到 ${binary_count} 个符合条件的二进制程序，失败 ${failed_count} 个 ====="

if [ "${failed_count}" -ne 0 ]; then
    echo "失败用例列表："
    printf '  %s\n' "${failed_tests[@]}"
    exit 1
fi

exit 0
