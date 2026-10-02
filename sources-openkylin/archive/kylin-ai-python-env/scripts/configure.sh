#!/bin/bash
set -e

# 获取项目根目录（脚本所在目录的上一级）
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
REQUIREMENTS_DIR="$PROJECT_ROOT/$ARCH_DIR/py$PYTHON_VERSION"

VENV_DIR="$PROJECT_ROOT/debian/python-env"

echo "Creating python-env environment at $VENV_DIR"
python3 -m venv "$VENV_DIR"

echo "Installing packages from $REQUIREMENTS_DIR/requirements.txt"
"$VENV_DIR/bin/pip" install --no-deps --no-cache-dir -i https://mirrors.aliyun.com/pypi/simple/ --extra-index-url https://download.pytorch.org/whl/cpu -f https://csukuangfj.github.io/kaldifeat/cpu-cn.html -r "$REQUIREMENTS_DIR/requirements.txt"
