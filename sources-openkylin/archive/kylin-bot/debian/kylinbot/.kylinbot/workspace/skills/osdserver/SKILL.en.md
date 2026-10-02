---
name: osdserver
description: On-screen display (OSD) operations
version: 0.0.1
author: ukui
tags:
  - osdserver
---

# OSD (OsdServer) conversation tests

## Test flow

### 1. Show volume bar
> Show volume OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowVolumeBar 60 100
```

### 2. Volume bar + mute
> Show mute OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowVolumeBarWithMute 60 100 true
```

### 3. Brightness bar
> Show brightness OSD

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowBrightnessBar 70 100
```

### 4. Airplane mode
> Airplane mode on/off

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

### 5. Touchpad status
> Touchpad on/off

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

### 6. Microphone status
> Microphone on/off

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

### 7. Camera status
> Camera on/off

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

### 8. Audio status
> Show audio status

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

### 9. Power mode OSD
> Switch power mode OSD

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

### 10. Wi-Fi status
> Show Wi-Fi status

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

### 11. Custom OSD
> Show a custom icon

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowCustomOSD "my-icon"
```

### 12. Custom progress bar
> Show a custom progress bar

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/OsdServer \
  --method org.ukui.Framework.UI.OsdServer.ShowBarWindow "my-bar" 50 100
```

---

## Sample conversation

**You**: Show volume OSD at 60
**Bot**: [OSD: volume bar 60]

**You**: Show brightness OSD at 70%
**Bot**: [OSD: brightness bar 70]

**You**: Show airplane mode on
**Bot**: [OSD: airplane mode on]

**You**: Show performance mode
**Bot**: [OSD: performance mode icon]
