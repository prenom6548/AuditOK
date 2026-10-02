#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import dbus
import dbus.exceptions
import getpass
import argparse
import json
import time
from pathlib import Path

# DBus 服务信息
SERVICE_NAME = "com.ukui.bluetooth"
OBJECT_PATH = "/com/ukui/bluetooth"
INTERFACE_NAME = "com.ukui.bluetooth"

# 连接状态码说明
CONNECT_MESSAGES = {
    0: "连接成功",
    -1: "蓝牙地址异常",
    -2: "蓝牙设备不存在",
    -3: "无默认蓝牙适配器",
    -4: "配对失败",
    -5: "连接过程中",
    50: "无蓝牙适配器",
    51: "连接进行中",
    52: "已连接",
    53: "设备不存在",
    54: "蓝牙音频数量最大值",
    55: "其他错误"
}

def get_dbus_interface():
    """获取 DBus 接口对象"""
    bus = dbus.SystemBus()
    proxy = bus.get_object(SERVICE_NAME, OBJECT_PATH)
    return dbus.Interface(proxy, INTERFACE_NAME)

def _set_adapter_attr(key, value):
    """
    设置默认适配器的单个属性
    key: 属性名（如 "Powered", "Discoverable" 等）
    value: bool 或 str，自动转换为 DBus 类型
    返回: 成功 True，失败 False
    """
    try:
        iface = get_dbus_interface()
        if isinstance(value, bool):
            dbus_value = dbus.Boolean(value)
        elif isinstance(value, str):
            dbus_value = dbus.String(value)
        else:
            dbus_value = value
        params = dbus.Dictionary({key: dbus_value}, signature='sv')
        success = iface.setDefaultAdapterAttr(params)
        return bool(success)
    except dbus.exceptions.DBusException as e:
        print(f"DBus 调用失败: {e}")
        return False

def cmd_scan_on():
    """打开蓝牙扫描（注册客户端）"""
    dbusid =  dbus.SystemBus().get_unique_name()
    username = getpass.getuser()
    app_type = 0

    params = {
        "dbusid": dbusid,
        "username": username,
        "type": dbus.Int32(app_type),
        "pid": dbus.Int64(os.getpid()),
    }

    try:
        iface = get_dbus_interface()
        result = iface.registerClient(params)
        if "result" in result:
            env_pc = result.get("envPC", 0)
            print("已完成扫描")
            time.sleep(10)
            return 0
        else:
            print("扫描失败")
            return 1
    except dbus.exceptions.DBusException as e:
        print(f"DBus 调用失败: {e}")
        return 1

def cmd_scan_off():
    print("扫描结束")
    pass

def cmd_connect(address):
    """连接蓝牙设备"""
    if not address or ":" not in address:
        print("✗ 蓝牙地址格式错误，应类似 AA:BB:CC:DD:EE:FF")
        return 1
    print("开始蓝牙连接")
    try:
        iface = get_dbus_interface()
        ret = iface.devConnect(address)

        time.sleep(4)
        attrs = iface.getDevAttr(address)
        connected = bool(attrs.get('Connected', False))
        if connected:
            print("设备已成功连接")
        else:
            print("设备连接失败")
        return 0
    except dbus.exceptions.DBusException as e:
        print(f"连接失败")
        return 1

def cmd_list_devices():
    """获取已扫描到的蓝牙设备列表，并显示序号、蓝牙地址和名称"""
    try:
        iface = get_dbus_interface()
        devices = iface.getDefaultAdapterAllDev()
        device_list = [str(addr) for addr in devices]
        
        if not device_list:
            print("未扫描到任何蓝牙设备")
            return 0
        
        print("已扫描到的蓝牙设备：")
        print(f"{'序号':<4} {'蓝牙地址':<18} 设备名称")
        print("-" * 50)
        
        for idx, addr in enumerate(device_list, start=1):
            try:
                attrs = iface.getDevAttr(addr)
                name = str(attrs.get('Name', '未知设备'))
                # 兼容可能的字节串
                if isinstance(name, bytes):
                    name = name.decode('utf-8', errors='replace')
                print(f"{idx:<4} {addr:<18}  {name}")
            except dbus.exceptions.DBusException as e:
                print(f"{idx:<4} {addr:<18}  [获取信息失败: {e}]")
        
        return 0
    except dbus.exceptions.DBusException as e:
        print(f"获取设备列表失败: {e}")
        return 1

def cmd_device_info(address):
    """获取蓝牙设备详细信息"""
    if not address or ":" not in address:
        print("蓝牙地址格式错误，应类似 AA:BB:CC:DD:EE:FF")
        return 1

    try:
        iface = get_dbus_interface()
        attrs = iface.getDevAttr(address)
        print(json.dumps(attrs, indent=2, ensure_ascii=False))
        return 0
    except dbus.exceptions.DBusException as e:
        print("获取设备信息失败: {e}")
        return -1

def cmd_power(action):
    """开启/关闭蓝牙电源"""
    if action == "on":
        ok = _set_adapter_attr("Powered", True)
        print("蓝牙已开启" if ok else "开启蓝牙失败")
        return 0 if ok else 1
    elif action == "off":
        ok = _set_adapter_attr("Powered", False)
        print("蓝牙已关闭" if ok else "关闭蓝牙失败")
        return 0 if ok else 1
    else:
        print("用法: power {on|off}")
        return 1

def cmd_tray(action):
    """任务栏图标显示/隐藏"""
    if action == "on":
        ok = _set_adapter_attr("trayShow", True)
        print("蓝牙图标已显示" if ok else "显示图标失败")
        return 0 if ok else 1
    elif action == "off":
        ok = _set_adapter_attr("trayShow", False)
        print("蓝牙图标已隐藏" if ok else "隐藏图标失败")
        return 0 if ok else 1
    else:
        print("用法: tray {on|off}")
        return 1

def cmd_discoverable(action):
    """设置设备是否可被附近设备发现"""
    if action == "on":
        ok = _set_adapter_attr("Discoverable", True)
        print("设备现在可被其他设备发现" if ok else "设置可被发现失败")
        return 0 if ok else 1
    elif action == "off":
        ok = _set_adapter_attr("Discoverable", False)
        print("设备已隐藏，不可被发现" if ok else "设置不可被发现失败")
        return 0 if ok else 1
    else:
        print("用法: discoverable {on|off}")
        return 1

def cmd_active_connection(action):
    """自动发现蓝牙音频设备（ActiveConnection）"""
    if action == "on":
        ok = _set_adapter_attr("ActiveConnection", True)
        print("自动发现音频设备已开启" if ok else "开启自动发现失败")
        return 0 if ok else 1
    elif action == "off":
        ok = _set_adapter_attr("ActiveConnection", False)
        print("自动发现音频设备已关闭" if ok else "关闭自动发现失败")
        return 0 if ok else 1
    else:
        print("用法: active-connection {on|off}")
        return 1

def cmd_audio_combine(action):
    """蓝牙音频组合设备开关（AudioCombine）"""
    if action == "on":
        ok = _set_adapter_attr("AudioCombine", True)
        print("蓝牙音频组合设备已开启" if ok else "开启音频组合设备失败")
        return 0 if ok else 1
    elif action == "off":
        ok = _set_adapter_attr("AudioCombine", False)
        print("蓝牙音频组合设备已关闭" if ok else "关闭音频组合设备失败")
        return 0 if ok else 1
    else:
        print("用法: audio-combine {on|off}")
        return 1

def cmd_set(attrs_pairs):
    """
    通用属性设置（支持所有适配器属性）
    示例: set Powered=true trayShow=false Name=MyBT
    """
    if not attrs_pairs:
        print("错误: 未提供任何属性设置")
        return 1

    bool_true = ("true", "on", "1", "yes")
    bool_false = ("false", "off", "0", "no")
    def parse_bool(v):
        lv = v.lower()
        if lv in bool_true:
            return True
        elif lv in bool_false:
            return False
        else:
            raise ValueError(f"无效的布尔值: {v}")

    # 支持的属性列表
    bool_keys = {"Powered", "Discoverable", "Discovering", "ActiveConnection",
                 "trayShow", "trayShowCtl", "DiscoverableCtl", "ActiveConnectionCtl"}
    params = {}
    for pair in attrs_pairs:
        if "=" not in pair:
            print(f"错误: 缺少'=': {pair}")
            return 1
        key, val = pair.split("=", 1)
        key = key.strip()
        val = val.strip()
        if key in bool_keys:
            try:
                params[key] = parse_bool(val)
            except ValueError as e:
                print(f"错误: {e}")
                return 1
        elif key == "Name":
            params[key] = val
        else:
            print(f"警告: 未知属性 '{key}'，忽略")
            continue

    if not params:
        print("错误: 没有有效属性可设置")
        return 1

    all_ok = True
    for k, v in params.items():
        if not _set_adapter_attr(k, v):
            print(f"设置 {k}={v} 失败")
            all_ok = False
    if all_ok:
        print("所有属性设置成功")
        return 0
    else:
        return 1


def main():
    parser = argparse.ArgumentParser(description="蓝牙命令行控制工具")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # scan 子命令
    scan_parser = subparsers.add_parser("scan", help="扫描控制")
    scan_parser.add_argument("action", choices=["on", "off"], help="打开或关闭扫描")

    # connect 子命令
    connect_parser = subparsers.add_parser("connect", help="连接蓝牙设备")
    connect_parser.add_argument("address", help="蓝牙设备地址，如 AA:BB:CC:DD:EE:FF")
    
    # list 子命令
    subparsers.add_parser("list", help="获取已扫描到的蓝牙设备列表")
    
    # info 子命令
    info_parser = subparsers.add_parser("info", help="获取蓝牙设备详细信息")
    info_parser.add_argument("address", help="蓝牙设备地址，如 AA:BB:CC:DD:EE:FF")
    
    # power 开启/关闭蓝牙电源
    power_parser = subparsers.add_parser("power", help="开启/关闭蓝牙电源")
    power_parser.add_argument("action", choices=["on", "off"])
    
    # tray 任务栏图标显示/隐藏
    tray_parser = subparsers.add_parser("tray", help="任务栏图标显示/隐藏")
    tray_parser.add_argument("action", choices=["on", "off"])
    
    # discoverable 设置设备可被发现
    discoverable_parser = subparsers.add_parser("discoverable", help="设置设备可被发现")
    discoverable_parser.add_argument("action", choices=["on", "off"])
    
    # active-connection 自动发现蓝牙音频设备
    active_parser = subparsers.add_parser("active-connection", help="自动发现蓝牙音频设备")
    active_parser.add_argument("action", choices=["on", "off"])
    
    # audio-combine 蓝牙音频组合设备
    audio_combine_parser = subparsers.add_parser("audio-combine", help="蓝牙音频组合设备开关")
    audio_combine_parser.add_argument("action", choices=["on", "off"])

    args = parser.parse_args()

    if args.command == "scan":
        if args.action == "on":
            return cmd_scan_on()
        else:  # off
            return cmd_scan_off()
    elif args.command == "connect":
        return cmd_connect(args.address)
    elif args.command == "list":
        return cmd_list_devices()
    elif args.command == "info":
        return cmd_device_info(args.address)
    elif args.command == "power":
        return cmd_power(args.action)
    elif args.command == "tray":
        return cmd_tray(args.action)
    elif args.command == "discoverable":
        return cmd_discoverable(args.action)
    elif args.command == "active-connection":
        return cmd_active_connection(args.action)
    elif args.command == "audio-combine":
        return cmd_audio_combine(args.action)
    else:
        parser.print_help()
        return 1

if __name__ == "__main__":
    sys.exit(main())