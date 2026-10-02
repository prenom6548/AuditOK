# wlcctrl

`wlcctrl` 是一个用于 kylin-wlcom 合成器的命令行工具。

## 概述

```bash
wlcctrl [command] [options...]
```

## 选项

### 常用选项

- `--help`
  - 显示帮助信息并退出。

- `-v, --version`
  - 显示wlrcctrl版本并退出。

- `--exec` <str>
  - 执行一个shell命令。

- `--sleep` <int_ms>
  - 设置暂停时长。此命令可以与其他任何命令组合使用。`_int_ms_` 是延迟时间，单位是毫秒。

  示例：
  ```bash
  wlcctrl --keydown a --sleep 1000 --keyup a
  wlcctrl --mousepress 1 --sleep 200 --mouserelease 1
  ```

- `--path` <str>
  - 设置截图保存路径，图像格式为PNG。默认情况下保存在当前工作目录中。
    - 与 `--windowcapture` 一起使用：设置窗口截图保存路径。
    - 与 `--workspacecapture` 一起使用：设置工作区截图保存路径。
    - 与 `--fullscreencapture`一起使用: 设置全屏截图保存路径。

  示例：
  ```bash
  wlcctrl --windowcapture uuid --path "/home/kylin/xxx.png"
  ```

- `-o` <o_uuid>
  - 指定输出设备。`_o_uuid_` 是输出设备的UUID。

## 键盘选项

- `--keystring <string>`
  - 模拟字符串输入。

- `--key` <key>
  - 发送指定的按键事件。按键可以是单个键或给定的组合键。

  字符名称映射：
  ```plaintext
  "PrintScreen" 映射为 "sys_req", "esc" 映射为 "escape", "ctrl" 映射为 "control_l", "shift" 映射为 "shift_l", "alt" 映射为 "alt_l", "winleft" 映射为 "super_l", "enter" 映射为 "return".
  ```

  示例：
  ```bash
  wlcctrl --key control_l+alt_l+t
  wlcctrl --key super_l
  ```

- `--keyup` <key>
  - 释放单个键。

- `--keydown` <key>
  - 按下单个键。

## 鼠标选项

- `--mousebutton` [button]
  - 点击鼠标按钮。

  按钮代码：BTN_LEFT->1 | BTN_RIGHT->2 | BTN_MIDDLE->3 | BTN_EXTRA->4 | BTN_SIDE->5 | BTN_FORWARD->6 | BTN_BACK->7

  示例：
  ```bash
  wlcctrl --mousebutton 2
  ```

- `--mousepress` [button]
  - 按下鼠标按钮。

  示例：
  ```bash
  wlcctrl --mousepress 2 --mouserelease 2
  ```

- `--mouserelease` [button]
  - 释放鼠标按钮。

- `--mousemove` <dx,dy>
  - 移动光标。`_dx_` 是向右方向的位移，`_dy_` 是向下方向的位移。

  示例：
  ```bash
  wlcctrl --mousemove 100,500
  ```

- `--scroll` [options...]
  - 滚动轴。
    - `-x <dx>`: dx 是水平滚动的量。
    - `-y <dy>`: dy 是垂直滚动的量。
    允许为负数。

  示例：
  ```bash
  wlcctrl --scroll -y 10
  wlcctrl --scroll -x 10 -y 20
  ```

- `--getmouselocation`
  - 获取当前鼠标位置。

## 窗口选项

- `-l, --list`
  - 列出所有窗口名称。

- `--windowmove` <w_uuid> [options...]
  - 将窗口移动到指定位置。

  示例：
  ```bash
  wlcctrl --windowmove 182fcd8f-6136-413b-aa1b-c77b066ebe23 -x 200 -y 400
  ```

- `--windowsize` <w_uuid> [options...]
  - 调整窗口大小。
    - `-w <width>`: width 是期望的窗口宽度。
    - `-h <height>`: height 是期望的窗口高度。

  示例：
  ```bash
  wlcctrl --windowsize 182fcd8f-6136-413b-aa1b-c77b066ebe23 -w 800 -h 600
  ```

- `--windowminimize` <w_uuid>
  - 最小化窗口。

- `--windowmaximize` <w_uuid>
  - 最大化窗口。

- `--windowfullscreen` <w_uuid>
  - 全屏显示窗口。

- `--windowenter` <w_uuid:uuid>
  - 将窗口添加到指定工作区。

- `--windowleave` <w_uuid:uuid>
  - 将窗口从指定的工作区移除。

- `--movewindowtoworkspace` <w_uuid:uuid>
  - 将窗口移动到指定的工作区。

- `--movewindowtooutput` <w_uuid:o_uuid>
  - 将窗口移动到指定的输出设备。

- `--windowmap` <w_uuid>
  - 恢复窗口。

- `--windowclose` <w_uuid>
  - 关闭窗口。

- `--windowkill` <app_id>
  - 强制关闭指定名称的所有窗口。

- `--windowactivate` <w_uuid>
  - 激活指定的窗口。

- `--windowcapture` <w_uuid>
  - 窗口截图。

- `--getactivewindow`
  - 获取当前活动的窗口。

- `--getwindowname` <w_uuid>
  - 查询特定窗口的应用ID、标题、图标和进程ID。

- `--getwindowgeometry` <w_uuid>
  - 查询特定窗口的几何信息。

- `--search` <name>
  - 通过检查窗口应用ID、图标和标题是否与正则表达式模式匹配来获取窗口UUID列表。

## 输出设备选项

- `--outputs`
  - 列出所有已知输出设备的名称。

- `--getdisplaygeometry` <o_uuid>
  - 获取指定输出设备的几何信息。

- `--fullscreencapture`
  - 截取整个屏幕的截图。

- `--setarea` <x,y,width,height>
  - 区域截图。设置矩形区域的起点和大小。

  示例：
  ```bash
  wlcctrl --setarea 10,40,500,100 --path "/home/kylin/xxx.png"
  ```

- `--getpixelcolor` <x,y>
  - 获取指定位置像素的RGBA值。

## 工作区选项

- `--workspace`
  - 列出所有已知工作区的名称。

- `--workspacecapture` <uuid:o_uuid>
  - 工作区截图。

- `--workspaceadd` <num>
  - 添加新的工作区。默认添加一个新工作区。

- `--workspaceremove` <uuid>
  - 移除指定的工作区。

- `--workspaceactive` <uuid>
  - 激活指定的工作区。
