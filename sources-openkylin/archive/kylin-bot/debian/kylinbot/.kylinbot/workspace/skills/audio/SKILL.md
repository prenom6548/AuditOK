---
name: audio
description: 设备音量相关控制
version: 0.0.1
author: ukui
tags:
  - 音量
---

# 音频 (Audio) 对话测试

## 测试流程

### 1. 查输出音量
> 当前音量多大

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkVolume
```

### 2. 设置输出音量
> 音量调到60

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkVolume 60
```

### 3. 静音
> 静音

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkMute true
```

### 4. 取消静音
> 取消静音

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkMute false
```

### 5. 查静音状态
> 现在静音着吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkMute
```

### 6. 查输出设备
> 当前用的什么输出设备

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkDevice
```

### 7. 查麦克风音量
> 麦克风音量多少

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSourceVolume
```

### 8. 设置麦克风音量
> 麦克风调到50

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSourceVolume 50
```

### 9. 麦克风静音
> 麦克风静音

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSourceMute true
```

### 10. 切换输出设备
> 切换到扬声器输出

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkDevice \
  "alsa_output.pci-0000_00_1f.3.analog-stereo" "analog-output-speaker"
```

---

## 完整对话样例

**你**: 音量调到40
**誩**: ✅ 已设为40

**你**: 静音
**誩**: ✅ 已静音

**你**: 取消静音，调到60
**誩**: ✅ 已取消静音，音量设为60

**你**: 麦克风静音
**誩**: ✅ 麦克风已静音
