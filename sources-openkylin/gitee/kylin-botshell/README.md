# Kylin BotShell

麒麟系统 BotShell 技能库，提供系统控制相关的多种技能，包括影音播放、截图录屏、系统管家、安全中心、进程管理等功能的命令行控制能力。

## 项目简介

Kylin BotShell 是面向麒麟操作系统的机器人技能集，通过 DBus 接口和命令行工具实现对系统应用的自动化控制。该项目旨在为智能助手提供统一的系统控制能力，用户可以通过自然语言指令完成系统操作。

## 功能特性

### 🎵 影音控制
- **麒麟影音控制** - 支持音乐和视频播放控制
  - 播放/暂停/上一首/下一首
  - 音量调节
  - 倍速播放
  - 播放模式切换（单曲循环、列表循环、随机播放）

### 📸 截图录屏
- **系统截图** - 快捷截图工具，支持多种模式
  - 全屏截图
  - 窗口截图
  - 延迟截图
  - 自定义保存路径

- **录屏功能** - 屏幕录制能力

### ⚙️ 系统管理
- **麒麟管家** - 系统管理和优化
  - 垃圾清理
  - 启动项管理
  - 系统优化

- **系统监视器** - 进程和系统资源监控
  - 查看进程
  - 结束进程
  - 系统资源监控

- **安全中心** - 系统安全防护
  - 病毒查杀
  - 实时防护状态
  - 安全扫描

## 目录结构

```
kylin-botshell/
├── cli/                    # CLI 相关文件
├── docs/                   # 文档目录
└── skills/                 # 技能模块
    └── system-control/     # 系统控制技能
        ├── kylin-music-control/      # 音乐控制
        ├── kylin-video-control/      # 视频控制
        ├── kylin-screenshot/         # 截图工具
        ├── kylin-screen-recording/   # 录屏功能
        ├── kylin-os-manager-control/ # 麒麟管家
        ├── kylin-security-center/    # 安全中心
        └── kylin-system-monitor/     # 系统监视器
```

## 技术实现

### DBus 接口
部分功能通过 DBus 接口与系统应用通信，使用 `qdbus` 命令进行调用：

```bash
# 读取属性
qdbus org.kde.MPRIS /Player org.freedesktop.DBus.Properties.Get org.mpris.MediaPlayer2.Player PlaybackStatus

# 调用方法
qdbus org.kde.MPRIS /Player org.mpris.MediaPlayer2.Player.Play
```

### Shell 脚本
系统管理功能通过 Shell 脚本执行：

```bash
# 执行系统操作
./scripts/kom.sh action param
./scripts/system-monitor.sh action param
./scripts/screenshot.sh mode delay save_path
```

## 使用前提

- 麒麟操作系统环境
- 已安装相关系统组件（麒麟影音、麒麟管家、安全中心等）
- DBus 服务正常运行
- 图形桌面环境（DISPLAY 环境变量）

## 许可证

本项目遵循开源许可证，具体许可证信息请查看各子模块中的 LICENSE 文件。

## 相关链接

- [openKylin 官网](https://www.openkylin.top/)
- [麒麟系统文档](https://docs.openkylin.top/)