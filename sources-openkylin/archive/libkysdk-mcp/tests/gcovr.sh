#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Run from project root
cd "$PROJECT_ROOT"

mkdir -p "$PROJECT_ROOT/coverage_report"

# 使用 --root 指定源码根目录，--object-directory 指定编译产物目录
# 使用 --html-details 生成每个文件的详细报告
# --exclude 使用通配模式，兼容不同构建路径（本地 vs CI）
# --gcov-executable 确保使用与编译时相同版本的 gcov
gcovr --root "$PROJECT_ROOT/src" \
    --object-directory="$PROJECT_ROOT/build" \
    --exclude='^.*/base64\.hpp$' \
    --exclude='^.*/httplib\.h$' \
    --exclude='^.*/json\.hpp$' \
    --html-details coverage_report/index.html
