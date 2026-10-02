---
name: touchpad
description: Touchpad-related operations
version: 0.0.1
author: ukui
tags:
  - touchpad
---

# Touchpad conversation tests

## Test flow

### 1. Get on/off state
> Is the touchpad on

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.GetTouchpadEnabled
```

### 2. Toggle touchpad
> Turn the touchpad off / turn the touchpad on

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTouchpadEnabled false

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTouchpadEnabled true
```

### 3. Disable when a mouse is plugged in
> Turn off the touchpad when a mouse is connected

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetDisableOnExternalMouse true
```

### 4. Disable while typing
> Disable the touchpad while typing

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetDisableWhileTyping true
```

### 5. Tap to click
> Enable tap to click

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetTapToClick true
```

### 6. Natural scrolling
> Enable natural scrolling

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetNaturalScroll true
```

### 7. Scroll type
> Switch to two-finger scrolling

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetScrollType 2
```
Values: 0=none, 1=edge scroll, 2=two-finger scroll

### 8. Pointer speed
> Set touchpad speed to 0.8

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.SetMotionAcceleration 0.8
```

### 9. Get touchpad count
> How many touchpads are there

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Touchpad \
  --method org.ukui.Framework.Devices.Touchpad.GetTouchpadCount
```

---

## Sample conversation

**You**: Turn on the touchpad, enable tap to click and two-finger scrolling
**Bot**: ✅ Enabled: touchpad ✅, tap to click ✅, two-finger scrolling ✅

**You**: Disable the touchpad when a mouse is plugged in
**Bot**: ✅ Set

**You**: Disable the touchpad while typing
**Bot**: ✅ Set
