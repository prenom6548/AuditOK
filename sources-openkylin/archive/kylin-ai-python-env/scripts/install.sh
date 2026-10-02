#!/bin/bash
set -e

# 获取项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH=$(uname -m)

if [ "$ARCH" = "x86_64" ]; then
    ARCH_DIR="amd64"
elif [ "$ARCH" = "aarch64" ]; then
    ARCH_DIR="arm64"
else
    echo "Unsupported architecture: $ARCH" >&2
    exit 1
fi

PYTHON_VERSION=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
VIRTUALENV_INSTALL_DIR="$PROJECT_ROOT/debian/tmp/kylin-ai-python-env"
TARGET_VENV_PATH="/usr/share/kylin-ai-python-env/python-env"


mkdir -p "$VIRTUALENV_INSTALL_DIR"
cp -r "$PROJECT_ROOT/debian/python-env" "$VIRTUALENV_INSTALL_DIR/"

# 修正路径 - 注意这里用 $PROJECT_ROOT 而不是 $(pwd)
sed -i "s|$PROJECT_ROOT/debian/python-env|$TARGET_VENV_PATH|g" "$VIRTUALENV_INSTALL_DIR/python-env/bin/activate"
sed -i "s|$PROJECT_ROOT/debian/python-env|$TARGET_VENV_PATH|g" "$VIRTUALENV_INSTALL_DIR/python-env/bin/activate.csh"
sed -i "s|$PROJECT_ROOT/debian/python-env|$TARGET_VENV_PATH|g" "$VIRTUALENV_INSTALL_DIR/python-env/bin/activate.fish"

# 修正 shebang
find "$VIRTUALENV_INSTALL_DIR/python-env/bin" -type f -executable | while read -r file; do
    if head -n1 "$file" | grep -q "^#!" && file -b "$file" | grep -q "script"; then
        sed -i "1s|.*|#!$TARGET_VENV_PATH/bin/python3|" "$file"
    fi
done

# 修正 .pth 文件
find "$VIRTUALENV_INSTALL_DIR/python-env/lib" -name "*.pth" -exec sed -i "s|$PROJECT_ROOT/debian/python-env|$TARGET_VENV_PATH|g" {} \;

# 文件替换 虚拟环境中部分文件可能存在敏感词，新文件为去掉敏感词的文件
copy_and_replace() {
    local src="$1"   # 第一个参数：源文件路径
    local dst="$2"   # 第二个参数：目标文件路径
    cp -f "$src" "$dst"
}

if [ "$PYTHON_VERSION" = "3.12" ]; then
    echo "检测到 Python 3.12，执行 pip emoji 文件替换"
    SRC_EMOJI="$PROJECT_ROOT/$ARCH_DIR/py$PYTHON_VERSION/pip/_vendor/rich/_emoji_codes.py"
    DST_EMOJI="$VIRTUALENV_INSTALL_DIR/python-env/lib/python$PYTHON_VERSION/site-packages/pip/_vendor/rich/_emoji_codes.py"

    # 检查源文件是否存在
    if [ ! -f "$SRC_EMOJI" ]; then
        echo "错误：源文件不存在: $SRC_EMOJI" >&2
        exit 1
    fi

    # 检查目标文件是否存在
    if [ ! -f "$DST_EMOJI" ]; then
        echo "错误：目标文件不存在: $DST_EMOJI" >&2
        exit 1
    fi

    copy_and_replace "$SRC_EMOJI" "$DST_EMOJI"
    echo "emoji 文件替换成功"
else
    echo "跳过 emoji 文件替换：当前 Python $PYTHON_VERSION，仅在 3.12 替换"
fi