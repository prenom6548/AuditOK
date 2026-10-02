---
name: touchpad
description: 触控板相关操作
version: 0.0.1
author: ukui
tags:
  - 触控板
---

# 触摸板 (Touchpad) 对话测试

## 测试流程

### 1. 查开关
> 触摸板开着吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.GetTouchpadEnabled
```

### 2. 开关触摸板
> 关掉触摸板 / 打开触摸板

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTouchpadEnabled false

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTouchpadEnabled true
```

### 3. 插入鼠标禁用触摸板
> 插鼠标时自动关触摸板

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetDisableOnExternalMouse true
```

### 4. 打字禁用
> 打字时禁用触摸板

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetDisableWhileTyping true
```

### 5. 轻触点击
> 开启轻触点击

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTapToClick true
```

### 6. 自然滚动
> 开启自然滚动

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetNaturalScroll true
```

### 7. 滚动类型
> 改成双指滚动

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetScrollType 2
```
可选值: 0=无滚动, 1=边界滚动, 2=双指滚动

### 8. 移动速度
> 触摸板速度调到0.8

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetMotionAcceleration 0.8
```

### 9. 查触摸板数量
> 有几个触摸板

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.GetTouchpadCount
```

---

## 完整对话样例

**你**: 打开触摸板，开启轻触点击和双指滚动
**誩**: ✅ 已开启：触摸板 ✅、轻触点击 ✅、双指滚动 ✅

**你**: 插鼠标时自动关触摸板
**誩**: ✅ 已设置

**你**: 打字时禁用触摸板
**誩**: ✅ 已设置
