#!/bin/bash

# 脚本功能：生成不同格式的覆盖率报告
# 使用方法：
#   ./getXml.sh                            # 生成SonarQube XML报告
#   ./getXml.sh html                       # 生成HTML报告
#   ./getXml.sh html-details               # 生成详细HTML报告
#   ./getXml.sh all                        # 生成所有类型的报告
#   ./getXml.sh lines                      # 生成仅包含行覆盖率的报告
#   ./getXml.sh branches                   # 生成仅包含分支覆盖率的报告
#   ./getXml.sh lines-xml                  # 生成仅行覆盖率的SonarQube XML报告
#   ./getXml.sh branches-xml               # 生成仅分支覆盖率的SonarQube XML报告

# 获取项目根目录
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TESTS_DIR="$ROOT_DIR/tests"
REPORT_DIR="$ROOT_DIR/coverage_report"

# 创建报告目录
mkdir -p "$REPORT_DIR"

case "$1" in
    html)
        echo "生成HTML覆盖率报告..."
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches
        echo "HTML报告已生成到 $REPORT_DIR/coverage.html"
        ;;
    html-details)
        echo "生成详细HTML覆盖率报告..."
        gcovr -r "$ROOT_DIR" \
              --html-details "$REPORT_DIR/index.html" \
              --object-directory="$TESTS_DIR" \
              --exclude="$ROOT_DIR/tests/.*" \
              --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/build-unitTest-unknown-Debug/moc_*" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/calculate/cal" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/calc_programmer" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/common" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/main.cpp" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/kabase/utils.hpp" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/kabase/buriedpoint.hpp" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/intel_plug/calculator.cpp" \
	      --exclude="/home/kylin/桌面/kylin-calculator/kylin-calculator/calculate/input" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --html-title="Kylin Calculator 覆盖率详情" \
              --html-medium-threshold 70 \
              --html-high-threshold 85
        echo "详细HTML报告已生成到 $REPORT_DIR/index.html"
        ;;
    lines)
        echo "生成仅包含行覆盖率的HTML报告..."
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/lines-coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --txt-metric line
        echo "仅行覆盖率HTML报告已生成到 $REPORT_DIR/lines-coverage.html"
        ;;
    branches)
        echo "生成仅包含分支覆盖率的HTML报告..."
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/branches-coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --txt-metric branch
        echo "仅分支覆盖率HTML报告已生成到 $REPORT_DIR/branches-coverage.html"
        ;;
    lines-xml)
        echo "生成仅行覆盖率的SonarQube XML报告..."
        gcovr -r "$ROOT_DIR" \
              -x \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --sonarqube "$REPORT_DIR/lines-kylin-video.xml" \
              --txt-metric line
        echo "仅行覆盖率SonarQube XML报告已生成到 $REPORT_DIR/lines-kylin-video.xml"
        ;;
    branches-xml)
        echo "生成仅分支覆盖率的SonarQube XML报告..."
        gcovr -r "$ROOT_DIR" \
              -x \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --sonarqube "$REPORT_DIR/branches-kylin-video.xml" \
              --txt-metric branch
        echo "仅分支覆盖率SonarQube XML报告已生成到 $REPORT_DIR/branches-kylin-video.xml"
        ;;
    all)
        echo "生成所有类型的覆盖率报告..."
        # SonarQube XML报告
        gcovr -r "$ROOT_DIR" \
              -x \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --sonarqube "$REPORT_DIR/kylin-video.xml"

        # HTML摘要报告
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches

        # 详细HTML报告
        gcovr -r "$ROOT_DIR" \
              --html-details "$REPORT_DIR/index.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude="ui_*" \
              --exclude="unit*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --html-title="Kylin Video 覆盖率详情" \
              --html-medium-threshold 70 \
              --html-high-threshold 85

        # 仅行覆盖率报告
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/lines-coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --txt-metric line

        # 仅分支覆盖率报告
        gcovr -r "$ROOT_DIR" \
              --html "$REPORT_DIR/branches-coverage.html" \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --exclude-unreachable-branches \
              --exclude-throw-branches \
              --txt-metric branch

        echo "所有报告已生成到 $REPORT_DIR/"
        ;;
    *)
        echo "生成SonarQube XML覆盖率报告..."
        gcovr -r "$ROOT_DIR" \
              -x \
              --object-directory="$TESTS_DIR" \
              --exclude=".*tests/.*" \
              --exclude="moc_*" \
              --sonarqube "$REPORT_DIR/kylin-video.xml"
        echo "SonarQube XML报告已生成到 $REPORT_DIR/kylin-video.xml"
        ;;
esac

echo ""
echo "使用方法:"
echo "  ./getXml.sh                # 生成SonarQube XML报告"
echo "  ./getXml.sh html           # 生成HTML报告"
echo "  ./getXml.sh html-details   # 生成详细HTML报告"
echo "  ./getXml.sh lines          # 生成仅包含行覆盖率的报告"
echo "  ./getXml.sh branches       # 生成仅包含分支覆盖率的报告"
echo "  ./getXml.sh lines-xml      # 生成仅行覆盖率的SonarQube XML报告"
echo "  ./getXml.sh branches-xml   # 生成仅分支覆盖率的SonarQube XML报告"
echo "  ./getXml.sh all            # 生成所有类型的报告"
echo ""
echo "报告颜色说明:"
echo "行覆盖率颜色说明："
echo "  绿色行：已被执行的代码行"
echo "  红色行：未被执行的代码行"
echo ""
echo "条件覆盖率（分支）颜色说明："
echo "  绿色分支：完全覆盖的条件分支"
echo "  黄色分支：部分覆盖的条件分支"
echo "  红色分支：未覆盖的条件分支"
