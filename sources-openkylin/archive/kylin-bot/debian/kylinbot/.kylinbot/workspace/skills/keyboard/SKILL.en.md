---
name: keyboard
description: Keyboard-related operations
version: 0.0.1
author: ukui
tags:
  - keyboard
---

# Keyboard conversation tests

## Test flow

### 1. Get key repeat
> Does holding a key repeat input

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetRepeatState
```

### 2. Set key repeat
> Enable key repeat

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.SetRepeatState true
```

### 3. Get CapsLock tip
> Does CapsLock show a tip

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetLockTipState
```

### 4. Set CapsLock tip
> Enable CapsLock tip

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.SetLockTipState true
```

### 5. Get keyboard count
> How many keyboards are there

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetKbdCount
```

---

## Sample conversation

**You**: Enable key repeat and CapsLock tip
**Bot**: ✅ Enabled: key repeat ✅, CapsLock tip ✅
