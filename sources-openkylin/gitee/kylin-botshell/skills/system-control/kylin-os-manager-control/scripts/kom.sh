#!/bin/bash
# kylin-os-manager 麒麟管家技能封装脚本
set -e

# 帮助文档
show_help() {
cat << EOF
用法: $0 run [action]

action 可选值：
  open             打开麒麟管家主界面
  fault-diagnosis  跳转故障诊断页面（并触发诊断）
  junk-clean       跳转垃圾清理页面
  device-manager   跳转设备管理页面

示例：
  $0 run open
  $0 run fault-diagnosis
  $0 run junk-clean
  $0 run device-manager
EOF
}

# 将 action 转换为实际命令
action_to_cmd() {
    case "$1" in
        open)
            echo "kylin-os-manager"
            ;;
        fault-diagnosis)
            echo "kylin-os-manager --jumpTab=FaultDiagnosis --pluginMessage=RunFaultDiagnosis"
            ;;
        junk-clean)
            echo "kylin-os-manager --jumpTab=JunkClean"
            ;;
        device-manager)
            echo "kylin-os-manager --jumpTab=DeviceManager"
            ;;
        *)
            echo ""
            ;;
    esac
}

# 参数解析
action=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        run)
            action="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "{\"success\":false,\"exit_code\":1,\"msg\":\"未知参数: $1\"}"
            exit 1
            ;;
    esac
done

# 校验 action
if [[ -z "$action" ]]; then
    echo "{\"success\":false,\"exit_code\":1,\"msg\":\"缺少 action 参数，请指定要执行的操作\"}"
    exit 1
fi

# 转换为实际命令
cmd=$(action_to_cmd "$action")
if [[ -z "$cmd" ]]; then
    echo "{\"success\":false,\"exit_code\":1,\"msg\":\"不支持的 action: $action\"}"
    exit 1
fi

# 适配图形界面
export DISPLAY=${DISPLAY:-:0}

# 提取主命令名，检查是否存在（针对 GUI 应用，不等待其退出）
main_cmd=$(echo "$cmd" | awk '{print $1}')
if ! command -v "$main_cmd" &>/dev/null; then
    echo "{\"success\":false,\"exit_code\":127,\"action\":\"$action\",\"msg\":\"$main_cmd 命令未找到\"}"
    exit 127
fi

# GUI 应用后台启动，立即返回，不等待窗口关闭
eval "$cmd" > /dev/null 2>&1 &
echo "{\"success\":true,\"exit_code\":0,\"action\":\"$action\",\"msg\":\"操作成功\"}"
