---
name: mouse
description: 鼠标相关操作
version: 0.0.1
author: ukui
tags:
  - 鼠标
---

# 鼠标 (Mouse) 对话测试

## 测试流程

### 1. 查光标主题
> 现在什么光标主题

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.GetCursorTheme
```

### 2. 设置光标主题
> 光标主题换成xxx

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetCursorTheme "default"
```

### 3. 指针定位
> 按Ctrl定位指针

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetLocatePointer 1
```
可选值: 0=不定位, 1=Ctrl定位, 2=晃动定位

### 4. 鼠标速度
> 鼠标速度调到1

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMotionAcceleration 1
```

### 5. 鼠标加速
> 开启鼠标加速

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMouseAccel true
```

### 6. 滚轮速度
> 滚轮速度调到1

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetWheelSpeed 1
```

### 7. 光标大小
> 光标调到32

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetCursorSize 32
```

### 8. 双击速度
> 双击速度调到400

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetDoubleClick 400
```

### 9. 左手模式
> 切换左手模式

```


gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetLeftHanded true
```

### 10. 中键
> 开启鼠标中键

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMiddleButtonEnabled true
```

### 11. 自然滚动（鼠标）
> 开启鼠标自然滚动

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetNaturalScroll true
```

### 12. 查鼠标数量
> 有几个鼠标

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.GetMouseCount
```

---

## 完整对话样例

**你**: 鼠标光标调大到32，速度调0.5
**誩**: ✅ 光标设为32px，速度0.5

**你**: 开启Ctrl定位指针
**誩**: ✅ 已开启，按Ctrl键定位指针

**你**: 切到左手模式
**誩**: ✅ 已切换到左手模式（左右键互换）

**你**: 滚轮速度调到4
**誩**: ✅ 滚轮速度已设为4
