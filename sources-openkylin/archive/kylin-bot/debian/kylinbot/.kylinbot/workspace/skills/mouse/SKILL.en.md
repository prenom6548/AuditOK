---
name: mouse
description: Mouse-related operations
version: 0.0.1
author: ukui
tags:
  - mouse
---

# Mouse conversation tests

## Test flow

### 1. Get cursor theme
> What's the current cursor theme

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.GetCursorTheme
```

### 2. Set cursor theme
> Change the cursor theme to xxx

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetCursorTheme "default"
```

### 3. Locate pointer
> Locate the pointer with Ctrl

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetLocatePointer 1
```
Values: 0=off, 1=Ctrl, 2=shake

### 4. Mouse speed
> Set mouse speed to 1

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMotionAcceleration 1
```

### 5. Mouse acceleration
> Enable mouse acceleration

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMouseAccel true
```

### 6. Wheel speed
> Set wheel speed to 1

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetWheelSpeed 1
```

### 7. Cursor size
> Set cursor size to 32

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetCursorSize 32
```

### 8. Double-click speed
> Set double-click speed to 400

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetDoubleClick 400
```

### 9. Left-handed mode
> Switch to left-handed mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetLeftHanded true
```

### 10. Middle button
> Enable the middle mouse button

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetMiddleButtonEnabled true
```

### 11. Natural scrolling (mouse)
> Enable natural scrolling for the mouse

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.SetNaturalScroll true
```

### 12. Get mouse count
> How many mice are there

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Mouse \
  --method org.ukui.Framework.Devices.Mouse.GetMouseCount
```

---

## Sample conversation

**You**: Make the cursor 32 and set speed to 0.5
**Bot**: ✅ Cursor set to 32px, speed 0.5

**You**: Enable Ctrl pointer location
**Bot**: ✅ Enabled, press Ctrl to locate the pointer

**You**: Switch to left-handed mode
**Bot**: ✅ Switched to left-handed mode (buttons swapped)

**You**: Set wheel speed to 4
**Bot**: ✅ Wheel speed set to 4
