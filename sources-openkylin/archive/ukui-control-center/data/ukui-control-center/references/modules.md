# 控制面板模块跳转

统一使用：

```bash
ukui-control-center -m <模块名>
```

## 账户

| 功能 | 命令 |
|------|------|
| 账户信息 | `ukui-control-center -m userinfo` |
| 登录选项 | `ukui-control-center -m biometrics` |
| 云账户 | `ukui-control-center -m "cloud account"` |

## 系统

| 功能 | 命令 |
|------|------|
| 显示器 | `ukui-control-center -m display` |
| 声音 | `ukui-control-center -m audio` |
| 电源 | `ukui-control-center -m power` |
| 远程桌面 | `ukui-control-center -m vino` |
| 关于 | `ukui-control-center -m about` |
| 系统启动 | `ukui-control-center -m boot` |
| 辅助功能 | `ukui-control-center -m accessibility` |

## 设备

| 功能 | 命令 |
|------|------|
| 打印机 | `ukui-control-center -m printer` |
| 鼠标 | `ukui-control-center -m mouse` |
| 键盘 | `ukui-control-center -m keyboard` |
| 快捷键 | `ukui-control-center -m shortcut` |
| 蓝牙 | `ukui-control-center -m bluetooth` |
| 多屏协同 | `ukui-control-center -m projection` |

## 网络

| 功能 | 命令 |
|------|------|
| 有线网络 | `ukui-control-center -m netconnect` |
| 无线网络 | `ukui-control-center -m wlanconnect` |
| 热点 | `ukui-control-center -m mobilehotspot` |
| 代理 | `ukui-control-center -m proxy` |
| VPN | `ukui-control-center -m vpn` |

## 个性化

| 功能 | 命令 |
|------|------|
| 背景 | `ukui-control-center -m wallpaper` |
| 主题 | `ukui-control-center -m theme` |
| 字体 | `ukui-control-center -m fonts` |

## 时间语言

| 功能 | 命令 |
|------|------|
| 时间和日期 | `ukui-control-center -m date` |
| 区域语言 | `ukui-control-center -m area` |

## 更新 / 安全 / 应用 / 搜索

| 功能 | 命令 |
|------|------|
| 备份 | `ukui-control-center -m backup` |
| 更新 | `ukui-control-center -m upgrade` |
| 安全 | `ukui-control-center -m securitycenter` |
| 开机启动 | `ukui-control-center -m autostart` |
| 默认应用 | `ukui-control-center -m defaultapp` |
| 搜索 | `ukui-control-center -m search` |

## 流程

```
用户请求 → 识别意图 → 匹配模块名 → 执行 ukui-control-center -m <模块>
```
