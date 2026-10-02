---
name: kylin-music-control
description: 通过命令行和 DBus 控制麒麟音乐（kylin-music），包括启动应用、打开本地音频、播放、暂停、上一首、下一首、调节音量或倍速以及设置单曲循环、列表循环或随机播放。用户要求打开麒麟音乐、播放歌曲或音频、暂停音乐、切歌、音乐音量、音乐倍速、音乐循环或随机播放时使用；不要用于视频播放控制。
version: 0.1.0
tags:
  - 音乐
  - 播放控制
  - DBus
names:
  zh-CN: 麒麟影音控制
---

# 麒麟影音控制

## 使用场景

用户要求控制音乐播放、切歌、调节音量、调整播放速度、设置循环模式或随机播放时使用本 skill。

## 输入说明

- `file_path`：可选，要播放的音频文件本地绝对路径。
- `operation`：必选，控制操作类型，包括：play / pause / next / previous / volume_up / volume_down / set_rate / set_loop / set_shuffle。
- `rate`：可选，倍速值，仅 `set_rate` 操作需要，支持 `0.5`、`0.75`、`1.0`、`1.25`、`1.5`、`2.0`。
- `mode`：可选，播放模式，仅 `set_loop` 操作需要，`Track` 为单曲循环，`Playlist` 为列表循环。
- `enable`：可选，是否开启随机播放，仅 `set_shuffle` 操作需要，`true` 为开启，`false` 为关闭（关闭后恢复列表循环）。

## DBus 接口说明

| 属性 | 值 |
|------|-----|
| 服务名 | `org.mpris.MediaPlayer2.KylinMusic` |
| 对象路径 | `/org/mpris/MediaPlayer2` |
| 接口名 | `org.mpris.MediaPlayer2.Player` |

### 可用方法（通过 qdbus 调用）

| 方法 | qdbus 命令 |
|------|-----------|
| 播放 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Play` |
| 暂停 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Pause` |
| 下一首 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Next` |
| 上一首 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Previous` |
| 音量增加 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.VolumeUp` |
| 音量减少 | `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.VolumeDown` |

### 可用属性（通过 org.freedesktop.DBus.Properties 读写）

| 属性 | 类型 | 读写 | 说明 |
|------|------|------|------|
| `Rate` | double | R/W | 倍速，取值 0.5 / 0.75 / 1.0 / 1.25 / 1.5 / 2.0 |
| `LoopStatus` | string | R/W | 播放模式，"Track"=单曲循环 / "Playlist"=列表循环 |
| `Shuffle` | bool | R/W | 随机播放，true=开启 / false=关闭 |
| `Volume` | double | R/W | 音量，范围 [0.0, 1.0] |
| `PlaybackStatus` | string | R | 播放状态，"Playing"/"Paused"/"Stopped" |

### 属性读写命令模板

读取属性：
```
qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get org.mpris.MediaPlayer2.Player <属性名>
```

设置属性：
```
qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Set org.mpris.MediaPlayer2.Player <属性名> <值>
```

## 启动命令

| 场景 | 命令 |
|------|------|
| 启动音乐 | `kylin-music` |
| 启动并播放指定文件 | `kylin-music <文件路径>`（支持本地绝对路径，也支持 `file://` 协议路径，程序会自动转换） |

## 执行流程

### 1. 启动音乐
直接执行 `kylin-music` 命令启动应用。如需播放指定文件，传入文件路径作为参数。

### 2. 播放控制（播放/暂停/下一首/上一首）
通过 `qdbus` 调用对应方法。注意：
- 暂停操作仅在播放状态下生效。
- 播放操作在暂停时恢复播放，在停止时播放列表中第一首。
- 切歌操作（上一首/下一首）仅在播放或暂停状态下生效。

### 3. 音量调节
通过 `qdbus` 调用 `VolumeUp` 或 `VolumeDown` 方法。
也可通过设置 `Volume` 属性精确控制（取值范围 [0.0, 1.0]）。

### 4. 倍速播放
通过设置 `Rate` 属性实现。支持的倍速值：`0.5`、`0.75`、`1.0`、`1.25`、`1.5`、`2.0`。
仅在播放或暂停状态下生效。

### 5. 播放模式切换
- **单曲循环**：设置 `LoopStatus` 为 `Track`
- **列表循环**：设置 `LoopStatus` 为 `Playlist`
- **随机播放**：设置 `Shuffle` 为 `true`（开启随机），设置 `Shuffle` 为 `false`（关闭随机，恢复列表循环）

注意：随机播放和循环模式相互独立，开启随机后 `LoopStatus` 可能不变，但播放顺序为随机。

## 约束限制

- DBus 控制命令依赖 kylin-music 应用已运行。
- 启动命令 `kylin-music` 为单例模式，重复启动不会打开多个窗口。
- 倍速播放仅在非停止状态下生效。
- 音量属性 `Volume` 取值范围为 [0.0, 1.0]，对应实际音量 [0, 100]。

## 异常处理

- 如果 DBus 调用失败，检查 kylin-music 是否在运行：`pgrep -f kylin-music`
- 如果应用未运行，则先执行启动命令再调用 DBus。
- 如果文件路径无效，`kylin-music <路径>` 仍会启动应用但不播放该文件。

## 调用样例

**用户**：播放音乐
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Play`

**用户**：切换到上一首歌
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Previous`

**用户**：调高音量
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.VolumeUp`

**用户**：1.5 倍速播放
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Set org.mpris.MediaPlayer2.Player Rate 1.5`

**用户**：设置随机播放
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Set org.mpris.MediaPlayer2.Player Shuffle true`

**用户**：单曲循环
**智能体**：执行 `qdbus org.mpris.MediaPlayer2.KylinMusic /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Set org.mpris.MediaPlayer2.Player LoopStatus Track`

**用户**：打开音乐播放 /home/user/music/song.mp3
**智能体**：执行 `kylin-music /home/user/music/song.mp3`
