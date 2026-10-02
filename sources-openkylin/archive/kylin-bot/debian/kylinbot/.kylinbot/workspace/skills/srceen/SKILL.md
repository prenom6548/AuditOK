---
name: screen
description: 屏幕相关操作
version: 0.0.1
author: ukui
tags:
  - 屏幕
---

# 屏幕 (Screen) 对话测试

## 测试流程

### 1. 查亮度
> 查一下当前显示器亮度

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetAllScreenBrightness
```

### 2. 查主屏亮度
> 主屏亮度多少

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetPrimaryScreenBrightness
```

### 3. 调亮度
> 亮度调到60

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetPrimaryScreenBrightness 60
```

### 4. 调某个显示器亮度
> 把HDMI-A-1调到80

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenBrightness "HDMI-A-1" 80
```

### 5. 统一所有显示器亮度
> 两个显示器都调到100

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetAllScreenSameBrightness 100
```

### 6. 查显示模式
> 当前什么显示模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetScreenMode
```

### 7. 切显示模式
> 切换到扩展模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenMode "extension"
```

可选模式: "primaryScreenOnlyMode" (仅主屏), "mirrorScreenMode" (镜像), "extensionMode" (扩展), "secondaryScreenOnlyMode" (仅第二屏)

### 8. 护眼模式
> 开启护眼模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetEyeCareEnabled true
```

### 9. 查护眼状态
> 护眼模式开着吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.IsEyeCareEnabled
```

### 10. 夜间模式
> 开启夜间模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetNightModeEnable true
```

### 11. 关夜间模式
> 关闭夜间模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetNightModeEnable false
```

### 12. 设置缩放
> 缩放率调到125%

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenScale 1.25
```

### 13. 查缩放
> 当前缩放率多少

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetScreenScale
```

### 14. 自动亮度
> 开启自动亮度

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetAutoBrightBrightnessEnable true
```

### 15. 快捷键（仅笔记本）
> 增亮

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.IncreaseBuiltinScreenBrightness
```

> 减亮

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.DecreaseBuiltinBrightness
```

---

## 完整对话样例

**你**: 查下当前显示器情况
**誩**: 两台显示器：HDMI-A-1 亮度40，DP-2 亮度100，当前是扩展模式

**你**: 两台都调到80，开护眼
**誩**: ✅ 已全部设为80，护眼模式已开启

**你**: 切到镜像模式
**誩**: ✅ 已切换到镜像模式

**你**: 关护眼，调到125%缩放
**誩**: ✅ 护眼已关，缩放率已设为125%
