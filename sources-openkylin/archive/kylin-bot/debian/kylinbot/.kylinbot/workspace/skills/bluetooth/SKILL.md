---
name: bluetooth
description: 蓝牙管理工具。支持电源控制、托盘图标、可发现性、自动发现音频设备、音频组合设备、连接设备和查询信息。
---

# UKUI 蓝牙管理工具 (ukui-bluetooth)

本脚本通过 DBus 系统服务 `com.ukui.bluetooth` 实现对蓝牙适配器和设备的命令行控制，提供直观的子命令接口。

## 前置条件

- **操作系统**：Linux（UKUI 桌面环境或任何支持 `com.ukui.bluetooth` 服务的发行版）
- **Python 依赖**：`dbus-python`（通常系统已预装）
- **DBus 服务**：`com.ukui.bluetooth` 必须正在运行
- **权限**：当前用户需有调用该 DBus 服务的权限（一般加入 `bluetooth` 组或使用 `sudo`）

## 脚本文件

脚本名称为 `ukui-bluetooth-cli.py`，使用时通过 `python3 ukui-bluetooth-cli.py <command> [参数]` 执行。

## 命令总览

| 命令 | 子命令/参数 | 功能 | 示例 |
|------|-------------|------|------|
| `power` | `on`, `off` | 开启/关闭蓝牙适配器电源 | `python3 ukui-bluetooth-cli.py power on` |
| `tray` | `on`, `off` | 显示/隐藏任务栏蓝牙图标 | `python3 ukui-bluetooth-cli.py tray on` |
| `discoverable` | `on`, `off` | 设置本机是否可被其他蓝牙设备发现 | `python3 ukui-bluetooth-cli.py discoverable on` |
| `active-connection` | `on`, `off` | 开启/关闭自动发现蓝牙音频设备 | `python3 ukui-bluetooth-cli.py active-connection on` |
| `audio-combine` | `on`, `off` | 开启/关闭蓝牙音频组合设备 | `python3 ukui-bluetooth-cli.py audio-combine on` |
| `list` | 无 | 列出已扫描到的设备（序号、地址、名称） | `python3 ukui-bluetooth-cli.py list` |
| `connect` | `<address>` | 连接指定蓝牙设备（等待4秒后检查连接状态） | `python3 ukui-bluetooth-cli.py connect AA:BB:CC:DD:EE:FF` |
| `info` | `<address>` | 以 JSON 格式显示设备详细信息 | `python3 ukui-bluetooth-cli.py info AA:BB:CC:DD:EE:FF` |
| `scan` | `on`, `off` | （传统模式）注册客户端扫描，`on` 后等待10秒 | `python3 ukui-bluetooth-cli.py scan on` |

> **说明**：`scan` 命令使用 `registerClient`，与直接控制适配器属性不同，一般使用 `active-connection` 和 `discoverable` 即可满足常见需求。

## 详细使用示例

### 1. 开启蓝牙

```bash
$ python3 ukui-bluetooth-cli.py power on
蓝牙已开启
