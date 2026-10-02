---
name: osdserver
description: osd显示相关操作
version: 0.0.1
author: ukui
tags:
  - osdserver
---

# OSD显示 (OsdServer) 对话测试

## 测试流程

### 1. 显示音量条
> 显示音量OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowVolumeBar 60 100
```

### 2. 音量条+静音
> 显示静音OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowVolumeBarWithMute 60 100 true
```

### 3. 亮度条
> 显示亮度OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowBrightnessBar 70 100
```

### 4. 飞行模式
> 飞行模式开/关

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowFlightModeOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowFlightModeOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowFlightModeOff
```

### 5. 触摸板状态
> 触摸板开/关

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowTouchpadStatusOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowTouchpadStatusOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowTouchpadStatusOff
```

### 6. 麦克风状态
> 麦克风开/关

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowMicrophoneStatusOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowMicrophoneStatusOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowMicrophoneStatusOff
```

### 7. 摄像头状态
> 摄像头开/关

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowCameraStatusOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowCameraStatusOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowCameraStatusOff
```

### 8. 音频状态
> 显示音频状态

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowAudioStatusOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowAudioStatusOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowAudioStatusOff
```

### 9. 电源模式OSD
> 电源模式切换

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowPowerModeOSD 0

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowPowerModePerformance

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowPowerModeEco

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowPowerModeAuto
```

### 10. WiFi状态
> 显示WiFi状态

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowWifiModeOSD true

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowWifiModeOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowWifiModeOff
```

### 11. 自定义OSD
> 显示自定义图标

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowCustomOSD "my-icon"
```

### 12. 自定义进度条
> 显示自定义进度条

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowBarWindow "my-bar" 50 100
```

---

## 完整对话样例

**你**: 显示音量OSD，当前60
**誩**: [OSD显示：音量条60]

**你**: 显示亮度OSD 70%
**誩**: [OSD显示：亮度条70]

**你**: 显示飞行模式已开
**誩**: [OSD显示：飞行模式开]

**你**: 显示性能模式
**誩**: [OSD显示：性能模式图标]
