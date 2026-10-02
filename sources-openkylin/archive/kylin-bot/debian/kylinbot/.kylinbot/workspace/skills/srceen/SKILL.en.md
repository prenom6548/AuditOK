---
name: screen
description: Screen and display operations
version: 0.0.1
author: ukui
tags:
  - screen
---

# Screen conversation tests

## Test flow

### 1. Get brightness
> Check current display brightness

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetAllScreenBrightness
```

### 2. Get primary brightness
> What's the primary screen brightness

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetPrimaryScreenBrightness
```

### 3. Set brightness
> Set brightness to 60

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetPrimaryScreenBrightness 60
```

### 4. Set a specific display
> Set HDMI-A-1 to 80

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenBrightness "HDMI-A-1" 80
```

### 5. Unify all displays
> Set both displays to 100

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetAllScreenSameBrightness 100
```

### 6. Get display mode
> What's the current display mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetScreenMode
```

### 7. Switch display mode
> Switch to extend mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenMode "extension"
```

Modes: "primaryScreenOnlyMode" (primary only), "mirrorScreenMode" (mirror), "extensionMode" (extend), "secondaryScreenOnlyMode" (secondary only)

### 8. Eye-care mode
> Enable eye-care mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetEyeCareEnabled true
```

### 9. Get eye-care state
> Is eye-care mode on

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.IsEyeCareEnabled
```

### 10. Night mode
> Enable night mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetNightModeEnable true
```

### 11. Disable night mode
> Turn off night mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetNightModeEnable false
```

### 12. Set scale
> Set scale to 125%

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetScreenScale 1.25
```

### 13. Get scale
> What's the current scale

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.GetScreenScale
```

### 14. Auto brightness
> Enable auto brightness

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.SetAutoBrightBrightnessEnable true
```

### 15. Shortcut keys (laptops only)
> Brightness up

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.IncreaseBuiltinScreenBrightness
```

> Brightness down

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Screen \
  --method org.ukui.Framework.Devices.Screen.DecreaseBuiltinBrightness
```

---

## Sample conversation

**You**: Check the current displays
**Bot**: Two displays: HDMI-A-1 brightness 40, DP-2 brightness 100, currently extend mode

**You**: Set both to 80 and enable eye-care
**Bot**: ✅ Both set to 80, eye-care enabled

**You**: Switch to mirror mode
**Bot**: ✅ Switched to mirror mode

**You**: Turn off eye-care and set scale to 125%
**Bot**: ✅ Eye-care off, scale set to 125%
