---
name: kylin-video-control
description: "控制麒麟影音播放器。用户说：打开麒麟影音、播放视频、暂停视频、继续播放、下一个视频、上一个视频、倍速播放、1.5倍速、慢放、快放、单曲循环、列表循环、随机播放、播放模式切换、打开本地视频时使用此技能。不要用于音乐或音频播放控制。"
version: 0.1.0
author: kylin-video-team
tags:
  - 影音
  - 播放控制
  - DBus
  - 麒麟影音
names:
  zh-CN: 麒麟影音控制
---
# 麒麟影音控制

## 使用场景

用户要求控制麒麟影音（kylin-video）播放器时使用，包括：

- 打开/启动麒麟影音应用
- 打开并播放指定路径的视频文件
- 播放当前视频
- 暂停当前视频
- 切换到上一个视频
- 切换到下一个视频
- 调整播放倍速（慢放或快放）
- 切换播放模式（随机、单曲循环、列表循环）

## 输入说明

| 功能 | 工具名 | 输入参数 | 类型 | 必填 | 说明 |
|------|--------|----------|------|------|------|
| 打开应用 | `open_video` | 无 | — | — | 启动麒麟影音应用 |
| 播放指定文件 | `play_file` | file_path | 字符串 | 是 | 待播放视频文件的本地绝对路径 |
| 播放视频 | `play` | 无 | — | — | 通过 DBus Play 方法播放 |
| 暂停视频 | `pause` | 无 | — | — | 通过 DBus Pause 方法暂停 |
| 下一个视频 | `next` | 无 | — | — | 切换到下一个视频 |
| 上一个视频 | `previous` | 无 | — | — | 切换到上一个视频 |
| 倍速播放 | `set_speed` | speed | 浮点数 | 是 | 支持 0.5 / 0.75 / 1.0 / 1.25 / 1.5 / 2.0 |
| 单曲循环 | `mode_single_loop` | 无 | — | — | 设置 LoopStatus=Track |
| 列表循环 | `mode_list_loop` | 无 | — | — | 设置 LoopStatus=Playlist, Shuffle=false |
| 随机播放 | `mode_random` | 无 | — | — | 设置 Shuffle=true |

## 输出说明

各 DBus 调用返回 `method_return` 表示操作成功；若播放器未运行或状态不匹配，DBus 调用将返回错误。本 skill 仅负责发送控制指令，不负责文件选择或播放列表管理。

## 执行流程

本 skill 通过 MPRIS 标准的 DBus 接口与麒麟影音通信：

- 服务名：`org.mpris.MediaPlayer2.KylinVideo`
- 对象路径：`/org/mpris/MediaPlayer2`
- 播放控制接口：`org.mpris.MediaPlayer2.Player`
- 属性设置接口：`org.freedesktop.DBus.Properties`

参考源码：`dbusadapter.h` / `dbusadapter.cpp`（kylin-video 项目 `src/core/` 目录下）。

### 1. 打开应用

```shell
kylin-video &
```

播放器启动后会自动注册 DBus 服务 `org.mpris.MediaPlayer2.KylinVideo`，之后方可响应其他 DBus 控制命令。

### 2. 播放指定视频文件

直接通过命令行启动播放器并打开指定视频：

```shell
kylin-video /path/to/video.mp4
```

此命令会启动麒麟影音并自动播放该视频文件，同时注册 DBus 服务供后续控制。

### 3. 播放控制（播放/暂停/上一个/下一个）

通过 `org.mpris.MediaPlayer2.Player` 接口调用对应方法：

```shell
# 播放
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.mpris.MediaPlayer2.Player.Play

# 暂停
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.mpris.MediaPlayer2.Player.Pause

# 下一个
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.mpris.MediaPlayer2.Player.Next

# 上一个
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.mpris.MediaPlayer2.Player.Previous
```

### 4. 倍速播放

通过 `org.freedesktop.DBus.Properties.Set` 设置 `Rate` 属性。仅当播放器处于 Playing 或 Paused 状态时才生效。

```shell
# 示例：设置为 1.5 倍速
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.freedesktop.DBus.Properties.Set \
  string:"org.mpris.MediaPlayer2.Player" \
  string:"Rate" \
  variant:double:1.5
```

支持的倍速值（来自 `dbusadapter.cpp` 的 `setRate` 方法校验，倍速范围 0.5~2.0）：

| 倍速值 | 说明 | DBus 命令中的 variant 值 |
|--------|------|--------------------------|
| 0.5 | 半速播放 | `variant:double:0.5` |
| 0.75 | 0.75 倍速 | `variant:double:0.75` |
| 1.0 | 正常速度 | `variant:double:1.0` |
| 1.25 | 1.25 倍速 | `variant:double:1.25` |
| 1.5 | 1.5 倍速 | `variant:double:1.5` |
| 2.0 | 两倍速 | `variant:double:2.0` |

### 5. 播放模式切换

播放模式由 `LoopStatus` 和 `Shuffle` 两个属性共同决定：

| 模式 | LoopStatus | Shuffle | 说明 |
|------|-----------|--------|------|
| 单曲循环 | `"Track"` | false | 重复播放当前视频 |
| 列表循环 | `"Playlist"` | false | 列表播放完后从头循环 |
| 随机播放 | — | true | 随机选取列表中视频播放 |

#### 单曲循环

```shell
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.freedesktop.DBus.Properties.Set \
  string:"org.mpris.MediaPlayer2.Player" \
  string:"LoopStatus" \
  variant:string:"Track"
```

#### 列表循环

```shell
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.freedesktop.DBus.Properties.Set \
  string:"org.mpris.MediaPlayer2.Player" \
  string:"LoopStatus" \
  variant:string:"Playlist" \
&& \
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.freedesktop.DBus.Properties.Set \
  string:"org.mpris.MediaPlayer2.Player" \
  string:"Shuffle" \
  variant:boolean:false
```

#### 随机播放

```shell
dbus-send --session --type=method_call --print-reply \
  --dest=org.mpris.MediaPlayer2.KylinVideo \
  /org/mpris/MediaPlayer2 \
  org.freedesktop.DBus.Properties.Set \
  string:"org.mpris.MediaPlayer2.Player" \
  string:"Shuffle" \
  variant:boolean:true
```

## 约束限制

- 所有 DBus 命令依赖 `org.mpris.MediaPlayer2.KylinVideo` 服务已在 session bus 上注册。若播放器未运行，需先执行 `open_video`。
- 播放/暂停/上下切换等操作需要播放器处于 Playing 或 Paused 状态；Stopped 状态下这些操作无效果。
- 倍速设置仅在 Playing 或 Paused 状态下生效，倍速值必须在 0.5~2.0 范围内且为允许的离散值。
- 所有命令为 D-Bus Session Bus 调用，无需 root 权限，不涉及文件写入、删除或系统配置修改。
- 本 skill 仅负责发送控制指令，不负责文件选择或播放列表管理。

## 异常处理

- 若 DBus 调用返回错误 `org.freedesktop.DBus.Error.ServiceUnknown`，说明麒麟影音未运行，提示用户先启动应用。
- 若倍速参数不在允许列表中，DBus 侧的 `setRate` 方法会直接忽略该调用。
- 若 Stopped 状态下调用 Play/Next/Previous 等方法，DBus 侧会直接返回不作处理。

## 调用样例

**用户**：打开麒麟影音

智能体执行：`kylin-video &`

---

**用户**：播放 /home/user/video.mp4 这个视频

智能体执行：`kylin-video /home/user/video.mp4`

---

**用户**：暂停播放

智能体执行：`dbus-send --session ... org.mpris.MediaPlayer2.Player.Pause`

---

**用户**：1.5 倍速播放

智能体执行：`dbus-send --session ... org.freedesktop.DBus.Properties.Set string:"org.mpris.MediaPlayer2.Player" string:"Rate" variant:double:1.5`

---

**用户**：切成随机播放

智能体执行：`dbus-send --session ... org.freedesktop.DBus.Properties.Set string:"org.mpris.MediaPlayer2.Player" string:"Shuffle" variant:boolean:true`

---

**用户**：单曲循环

智能体执行：`dbus-send --session ... org.freedesktop.DBus.Properties.Set string:"org.mpris.MediaPlayer2.Player" string:"LoopStatus" variant:string:"Track"`
