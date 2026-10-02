#!/bin/bash

# 脚本功能：为特定C++文件生成详细的覆盖率报告
# 使用方法：
#   ./generate_detailed_coverage.sh                    # 生成所有文件的覆盖率报告
#   ./generate_detailed_coverage.sh global.cpp         # 生成特定文件的覆盖率报告

# 创建输出目录
OUTPUT_DIR="detailed_coverage_report"
mkdir -p "$OUTPUT_DIR"

# 获取项目根目录
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "项目根目录: $ROOT_DIR"
echo "输出目录: $OUTPUT_DIR"

# 如果提供了文件名参数，则只处理该文件
if [ $# -eq 1 ]; then
    FILE_NAME="$1"
    echo "正在为文件 $FILE_NAME 生成详细覆盖率报告..."
    
    # 生成HTML详细报告，只针对指定文件
    gcovr -r "$ROOT_DIR" \
          --html-details "$OUTPUT_DIR/index.html" \
          --object-directory="$ROOT_DIR/tests" \
          --filter=".*$FILE_NAME" \
          --exclude=".*tests/.*" \
          --exclude="moc_*" \
          --exclude-unreachable-branches \
          --exclude-throw-branches \
          --html-title="Kylin Video - $FILE_NAME 覆盖率详情" \
          --html-medium-threshold 70 \
          --html-high-threshold 85
    
    echo "已生成 $FILE_NAME 的详细覆盖率报告到 $OUTPUT_DIR/index.html"
else
    echo "正在为所有文件生成详细覆盖率报告..."
    
    # 生成完整的HTML详细报告
    gcovr -r "$ROOT_DIR" \
          --html-details "$OUTPUT_DIR/index.html" \
          --object-directory="$ROOT_DIR/tests" \
          --exclude=".*tests/.*" \
          --exclude="moc_*" \
          --exclude-unreachable-branches \
          --exclude-throw-branches \
          --html-title="Kylin Video 完整覆盖率详情" \
          --html-medium-threshold 70 \
          --html-high-threshold 85
    
    echo "已生成完整覆盖率报告到 $OUTPUT_DIR/index.html"
fi

echo ""
echo "报告说明："
echo "行覆盖率颜色说明："
echo "  绿色行：已被执行的代码行"
echo "  红色行：未被执行的代码行"
echo ""
echo "条件覆盖率（分支）颜色说明："
echo "  绿色分支：完全覆盖的条件分支"
echo "  黄色分支：部分覆盖的条件分支"
echo "  红色分支：未覆盖的条件分支"
echo ""
echo "要查看报告，请打开 $OUTPUT_DIR/index.html 文件"