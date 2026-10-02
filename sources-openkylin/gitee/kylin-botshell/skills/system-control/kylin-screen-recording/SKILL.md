---
name: kylin-screen-recording
description: "使用 kylin-screencap 打开麒麟桌面录屏功能。用户要求录屏、开始录屏、屏幕录像、屏幕录制、桌面录制、录制屏幕、录一段屏幕视频或打开录屏工具时使用；不要用于静态截图或截屏。"
version: 0.0.1
author: ""
tags:
  - 系统工具
  - 录屏
---
# 录屏（kylin-screencap）

调用麒麟系统自带 `kylin-screencap` 命令行工具打开录屏功能。

## 触发条件

用户表达以下任一意图时触发：

- "录屏"、"开始录屏"、"屏幕录制"
- "录一段视频"、"帮我录屏"
- "打开录屏工具"、"我要录制屏幕"

## 命令

`kylin-screencap full` 用于打开全桌面录屏，命令执行后会**立刻返回**，无需后台执行（不需要 `&` 或 `setsid`）。

```bash
kylin-screencap full
```

## 执行流程

1. 识别用户录屏意图
2. 直接执行 `kylin-screencap full`
3. 命令返回后告知用户录屏工具已启动

## 示例

| 用户意图 | 命令 |
|----------|------|
| 帮我录屏 | `kylin-screencap full` |
| 开始屏幕录制 | `kylin-screencap full` |
| 录一段桌面视频 | `kylin-screencap full` |
