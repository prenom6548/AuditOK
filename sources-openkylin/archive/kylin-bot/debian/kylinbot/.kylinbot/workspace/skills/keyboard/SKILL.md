---
name: keyboard
description: 键盘相关操作
version: 0.0.1
author: ukui
tags:
  - 键盘
---

# 键盘 (Keyboard) 对话测试

## 测试流程

### 1. 查按键重复
> 长按按键会重复输入吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetRepeatState
```

### 2. 设置重复
> 开启按键重复

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.SetRepeatState true
```

### 3. 查CapsLock提示
> CapsLock有提示吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetLockTipState
```

### 4. 设置CapsLock提示
> 开启CapsLock提示

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.SetLockTipState true
```

### 5. 查键盘数量
> 有几个键盘

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Keyboard \
  --method org.ukui.Framework.Devices.Keyboard.GetKbdCount
```

---

## 完整对话样例

**你**: 开启按键重复和CapsLock提示
**誩**: ✅ 已开启：按键重复 ✅、CapsLock提示 ✅
