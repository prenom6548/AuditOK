#!/bin/bash
# 下载 Qwen3-Embedding-0.6B 模型的大文件
# 用于部署时自动获取模型权重

set -e  # 遇到错误立即退出

MODEL_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Model directory: $MODEL_DIR"

# 检查是否已存在
if [ -f "$MODEL_DIR/model.safetensors" ]; then
    echo "✓ Model files already exist. Skipping download."
    echo "  If you want to re-download, delete the files first."
    exit 0
fi

echo "Downloading Qwen3-Embedding-0.6B model files..."

# 下载 model.safetensors (1.2GB)
if [ ! -f "$MODEL_DIR/model.safetensors" ]; then
    echo "Downloading model.safetensors (1.2GB)..."
    wget -c \
        "https://modelscope.cn/models/Qwen/Qwen3-Embedding-0.6B/resolve/master/model.safetensors" \
        -O "$MODEL_DIR/model.safetensors.tmp"

    # 验证下载成功
    if [ -f "$MODEL_DIR/model.safetensors.tmp" ]; then
        mv "$MODEL_DIR/model.safetensors.tmp" "$MODEL_DIR/model.safetensors"
        echo "✓ model.safetensors downloaded successfully"
    else
        echo "✗ Failed to download model.safetensors"
        exit 1
    fi
else
    echo "✓ model.safetensors already exists"
fi

echo ""
echo "========================================"
echo "✓ All model files ready!"
echo "========================================"
echo "Model is ready to use at: $MODEL_DIR"
