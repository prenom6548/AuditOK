#!/bin/bash
# 控制面板设置脚本（辅助功能 / 区域语言 / 字体）
# 用法:
#   ukcc_scripts.sh <功能> <动作或数值>
# 功能:
#   zoom | color-filter | screen-reader
#   reader-volume | 朗读音量 | volume
#   reader-speed  | 朗读语速 | speed
#   language | 系统语言 | lang
#   first-day | 第一天
#   monospace | 等宽字体
#   font-reset | 恢复默认字体
# 动作: on|off|status；音量/语速传数字或 status；语言传 zh_CN|en|… 或 status
set -e

DEST=org.ukui.ukcc.session
PATH_OBJ=/Accessibility
IFACE=org.ukui.ukcc.session.Accessibility
AREA_PATH=/Area
AREA_IFACE=org.ukui.ukcc.session.Area
FONT_PATH=/Font
FONT_IFACE=org.ukui.ukcc.session.Font
SCHEMA=org.gnome.desktop.a11y.applications
KEY=screen-reader-enabled
UKCC_SCHEMA=org.ukui.control-center
MATE_IFACE=org.mate.interface
GNOME_IFACE=org.gnome.desktop.interface
MONO_KEY=monospace-font-name

FEATURE="${1:-}"
ACTION="${2:-}"
ARG3="${3:-}"

usage() {
  cat >&2 <<EOF
用法: $0 <功能> <动作或数值>

功能:
  zoom | magnifier | 放大镜              # on|off|status
  zoom-mode | 放大视图                   # window|fullscreen|1|0|status
  color-filter | filter | 色彩滤镜
  screen-reader | reader | 屏幕朗读
  reader-volume | volume | 朗读音量     # 0-100，例: $0 reader-volume 60
  reader-speed  | speed  | 朗读语速     # 30-100，例: $0 reader-speed 70
  language | lang | 系统语言            # zh_CN|en|bo_CN|mn_MN|zh_HK|status
  first-day | 第一天 | weekday-start    # sunday|monday|status
  monospace | 等宽字体                  # <族名> [字号] 或 status；例: $0 monospace FreeMono 12
  font-reset | 恢复默认字体             # 无额外参数

动作(开关类):
  on | off | status

示例:
  $0 zoom on
  $0 zoom-mode window
  $0 zoom-mode fullscreen
  $0 color-filter off
  $0 screen-reader status
  $0 reader-volume 60
  $0 reader-speed 70
  $0 reader-volume status
  $0 language zh_CN
  $0 language en
  $0 language status
  $0 first-day sunday
  $0 first-day monday
  $0 monospace FreeMono 12
  $0 monospace status
  $0 font-reset
EOF
  exit 2
}

normalize_action() {
  case "${1:-}" in
    on|true|1|open|enable|开启|打开|启用) echo on ;;
    off|false|0|close|disable|关闭|停止|关掉) echo off ;;
    status|get|状态) echo status ;;
    *) echo "" ;;
  esac
}

[ -n "$FEATURE" ] || usage

case "$FEATURE" in
  reader-volume|volume|朗读音量)
    case "${ACTION}" in
      ""|status|get|状态)
        gsettings get "$UKCC_SCHEMA" reader-volume 2>/dev/null \
          || dconf read /org/ukui/control-center/reader-volume
        ;;
      *)
        if ! [[ "$ACTION" =~ ^[0-9]+$ ]] || [ "$ACTION" -lt 0 ] || [ "$ACTION" -gt 100 ]; then
          echo "朗读音量须为 0-100 整数，当前: $ACTION" >&2
          exit 2
        fi
        if gsettings list-keys "$UKCC_SCHEMA" 2>/dev/null | grep -qx reader-volume; then
          gsettings set "$UKCC_SCHEMA" reader-volume "$ACTION"
        else
          dconf write /org/ukui/control-center/reader-volume "$ACTION"
        fi
        ;;
    esac
    ;;
  reader-speed|speed|朗读语速)
    case "${ACTION}" in
      ""|status|get|状态)
        gsettings get "$UKCC_SCHEMA" reader-speed 2>/dev/null \
          || dconf read /org/ukui/control-center/reader-speed
        ;;
      *)
        if ! [[ "$ACTION" =~ ^[0-9]+$ ]] || [ "$ACTION" -lt 30 ] || [ "$ACTION" -gt 100 ]; then
          echo "朗读语速须为 30-100 整数，当前: $ACTION" >&2
          exit 2
        fi
        if gsettings list-keys "$UKCC_SCHEMA" 2>/dev/null | grep -qx reader-speed; then
          gsettings set "$UKCC_SCHEMA" reader-speed "$ACTION"
        else
          dconf write /org/ukui/control-center/reader-speed "$ACTION"
        fi
        ;;
    esac
    ;;
  zoom|magnifier|放大镜)
    ACTION=$(normalize_action "$ACTION")
    [ -n "$ACTION" ] || usage
    case "$ACTION" in
      on) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setZoomSwitch" true ;;
      off) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setZoomSwitch" false ;;
      status) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.isZooming" ;;
    esac
    ;;
  zoom-mode|放大视图)
    case "${ACTION}" in
      window|1|窗口|窗口放大)
        gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setZoomMode" 1
        ;;
      fullscreen|full|0|全屏|全屏放大)
        gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setZoomMode" 0
        ;;
      ""|status|get|状态)
        gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.GetType"
        ;;
      *)
        echo "放大视图参数: window|fullscreen|1|0|status" >&2
        exit 2
        ;;
    esac
    ;;
  color-filter|filter|色彩滤镜)
    ACTION=$(normalize_action "$ACTION")
    [ -n "$ACTION" ] || usage
    case "$ACTION" in
      on) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setColorFilterSwitch" true ;;
      off) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.setColorFilterSwitch" false ;;
      status) gdbus call --session --dest "$DEST" --object-path "$PATH_OBJ" --method "$IFACE.isColorFilterEnabled" ;;
    esac
    ;;
  screen-reader|reader|屏幕朗读)
    ACTION=$(normalize_action "$ACTION")
    [ -n "$ACTION" ] || usage
    case "$ACTION" in
      on) gsettings set "$SCHEMA" "$KEY" true ;;
      off) gsettings set "$SCHEMA" "$KEY" false ;;
      status) gsettings get "$SCHEMA" "$KEY" ;;
    esac
    ;;
  language|lang|系统语言)
    case "${ACTION}" in
      ""|status|get|状态)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" \
          --method org.freedesktop.DBus.Properties.Get "$AREA_IFACE" language
        ;;
      简体中文|中文|zh_CN|zh-CN)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "zh_CN"
        ;;
      英语|英文|en|en_US|en-US)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "en"
        ;;
      藏语|bo_CN|bo-CN)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "bo_CN"
        ;;
      蒙古语|mn_MN|mn-MN)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "mn_MN"
        ;;
      繁体中文|繁体|zh_HK|zh-HK|zh_TW|zh-TW)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "zh_HK"
        ;;
      *)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setLanguage" "$ACTION"
        ;;
    esac
    ;;
  first-day|第一天|weekday-start|一周第一天)
    case "${ACTION}" in
      ""|status|get|状态)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" \
          --method org.freedesktop.DBus.Properties.Get "$AREA_IFACE" firstDay
        ;;
      sunday|sun|周日|星期日|星期天)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setFirstDay" "sunday"
        ;;
      monday|mon|周一|星期一)
        gdbus call --session --dest "$DEST" --object-path "$AREA_PATH" --method "$AREA_IFACE.setFirstDay" "monday"
        ;;
      *)
        echo "一周第一天参数: sunday|monday|status" >&2
        exit 2
        ;;
    esac
    ;;
  monospace|等宽字体|mono-font)
    case "${ACTION}" in
      ""|status|get|状态)
        gsettings get "$MATE_IFACE" "$MONO_KEY"
        ;;
      *)
        FAMILY="$ACTION"
        SIZE="$ARG3"
        if [ -z "$SIZE" ]; then
          SIZE=$(gsettings get org.ukui.style system-font-size 2>/dev/null | tr -d "'" || echo 11)
        fi
        # 禁止裸族名：必须「族名 字号」
        if [[ "$FAMILY" =~ [0-9]$ ]]; then
          VALUE="$FAMILY"
        else
          VALUE="$FAMILY $SIZE"
        fi
        gsettings set "$MATE_IFACE" "$MONO_KEY" "$VALUE"
        gsettings set "$GNOME_IFACE" "$MONO_KEY" "$VALUE" 2>/dev/null || true
        gsettings get "$MATE_IFACE" "$MONO_KEY"
        ;;
    esac
    ;;
  font-reset|reset-font|恢复默认字体)
    gdbus call --session --dest "$DEST" --object-path "$FONT_PATH" --method "$FONT_IFACE.resetDefault"
    # 与 FontInterface 对齐：再清 gnome 等宽（旧会话库可能未含此逻辑）
    gsettings reset "$GNOME_IFACE" "$MONO_KEY" 2>/dev/null || true
    gsettings get "$MATE_IFACE" "$MONO_KEY"
    ;;
  *)
    usage
    ;;
esac
