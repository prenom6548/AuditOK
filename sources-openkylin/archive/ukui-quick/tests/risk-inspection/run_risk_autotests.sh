#!/usr/bin/env bash
# 风险检查专用 - 自动化测试执行脚本
# 说明：
# 1. 默认从 build-risk-inspection 目录读取测试二进制
# 2. 不依赖 tests/runUnitTest.sh
# 3. 输出单独日志，适合手工检查留痕

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build-risk-inspection}"
RESULTS_DIR="$PROJECT_DIR/results/risk-inspection"
LOG_FILE="$RESULTS_DIR/risk-autotests-$(date +%Y%m%d-%H%M%S).log"

mkdir -p "$RESULTS_DIR"
exec > >(tee -a "$LOG_FILE") 2>&1

fail() {
    echo ">>> $1" >&2
    exit 1
}

check_binary() {
    local path="$1"
    if [[ ! -x "$path" ]]; then
        fail "缺少测试二进制：$path，请先执行 tests/risk-inspection/build_risk_test_binaries.sh"
    fi
}

run() {
    local title="$1"
    shift
    echo
    echo "============================================================"
    echo "正在执行：$title"
    echo "执行命令：$*"
    echo "============================================================"
    "$@"
}

echo ">>> 项目目录: $PROJECT_DIR"
echo ">>> 构建目录: $BUILD_DIR"
echo ">>> 日志文件: $LOG_FILE"

check_binary "$BUILD_DIR/framework/autotest/test-widget-metadata"
check_binary "$BUILD_DIR/framework/autotest/test-widget-loader"
check_binary "$BUILD_DIR/platform/autotest/test-theme"
check_binary "$BUILD_DIR/platform/autotest/settings"
check_binary "$BUILD_DIR/platform/autotest/test-window-manager"
check_binary "$BUILD_DIR/platform/autotest/test-wm-interface"
check_binary "$BUILD_DIR/platform/autotest/test-abstract-window-manager"
check_binary "$BUILD_DIR/platform/autotest/test-dbus-connector"
check_binary "$BUILD_DIR/platform/autotest/test-app-launcher"
check_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-config"
check_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-plugin"
check_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-thumbnail-item"
check_binary "$BUILD_DIR/modules/window-thumbnail/autotest/test-player-items-model"
check_binary "$BUILD_DIR/core/autotest/test-shared-engine-component"
check_binary "$BUILD_DIR/core/autotest/test-shared-engine-view"

run "TC-R01-01 非法 metadata 样本必须被拒绝" \
    "$BUILD_DIR/framework/autotest/test-widget-metadata" -txt
run "TC-R01-02/03 合法插件加载、宿主过滤、重复 ID 去重" \
    "$BUILD_DIR/framework/autotest/test-widget-loader" -txt
run "TC-R02-01/02 主题参数读取与透明度回退" \
    env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=generic GSETTINGS_BACKEND=memory \
    "$BUILD_DIR/platform/autotest/test-theme" -txt
run "TC-R02-03/TC-R05-03 设置服务不可用时安全降级" \
    env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=generic GSETTINGS_BACKEND=memory \
    "$BUILD_DIR/platform/autotest/settings" -txt
run "TC-R03-01 会话类型识别" \
    "$BUILD_DIR/platform/autotest/test-window-manager" -txt
run "TC-R03-02 窗口包装层默认实现安全" \
    env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=generic GSETTINGS_BACKEND=memory \
    "$BUILD_DIR/platform/autotest/test-wm-interface" -txt
run "TC-R03-03 窗口高亮调用在有无服务时都安全" \
    "$BUILD_DIR/platform/autotest/test-abstract-window-manager" -txt
run "TC-R05-01 DBus 连接器服务缺失时安全失败" \
    "$BUILD_DIR/platform/autotest/test-dbus-connector" -txt
run "TC-R05-02 AppLauncher 空输入忽略、URI 路由可控" \
    "$BUILD_DIR/platform/autotest/test-app-launcher" -txt
run "TC-R04-01 预览后端选择与默认路径验证" \
    "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-config" -txt
run "TC-R04-02 QML 插件注册与配置注入验证" \
    "$BUILD_DIR/modules/window-thumbnail/autotest/test-window-thumbnail-plugin" -txt
run "TC-R04-03 高频切换、重复激活、资源回收安全" \
    env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=generic \
    "$BUILD_DIR/modules/window-thumbnail/autotest/test-thumbnail-item" -txt
run "TC-R04-04 MPRIS 服务上下线对预览附加能力的影响" \
    "$BUILD_DIR/modules/window-thumbnail/autotest/test-player-items-model" -txt
run "补充验证：共享引擎组件异常输入安全性" \
    "$BUILD_DIR/core/autotest/test-shared-engine-component" -txt
run "补充验证：共享引擎视图异常输入安全性" \
    "$BUILD_DIR/core/autotest/test-shared-engine-view" -txt

echo
echo ">>> 全部风险相关自动化测试执行完成"
echo ">>> 日志文件已保存到：$LOG_FILE"
