---
name: power
description: 电源管理相关操作
version: 0.0.1
author: ukui
tags:
  - 电源
---

# 电源管理 (Power) 对话测试

## 测试流程

### 1. 查电量
> 还有多少电

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.BatteryPercentage
```

### 2. 充电状态
> 在充电吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetBatteryState
```
返回值: 0=未知, 1=充电, 2=放电, 3=空, 4=充满, 5=准备充, 6=准备断

### 3. 可用时长
> 还能用多久

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.TimeToEmpty
```

### 4. 充满时间
> 还有多久充满

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.TimeToFull
```

### 5. 电源模式
> 当前什么电源模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetPowerMode
```
返回值: 0=性能, 1=平衡, 2=节能

### 6. 切到性能模式
> 切到性能模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerMode 0
```

### 7. 切到节能模式
> 省电模式

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerMode 2
```

### 8. 电源按键动作
> 按电源键会怎样

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetPowerButtonAction
```

### 9. 设置电源键动作
> 按电源键改成休眠

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerButtonAction "hibernate"
```
可选值: "interactive"(询问), "blank"(息屏), "suspend"(睡眠), "hibernate"(休眠), "shutdown"(关机)

### 10. 合盖动作
> 合盖会怎样

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLidCloseAction
```

### 11. 设置合盖
> 合盖改成睡眠

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetLidCloseAction "suspend"
```
可选值: "nothing"(无操作), "blank"(息屏), "suspend"(睡眠), "hibernate"(休眠), "shutdown"(关机)

### 12. AC息屏时间
> 插着电源多久息屏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetScreenTimeoutOnAC
```

### 13. 设置AC息屏时间
> 插电时10分钟息屏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetScreenTimeoutOnAC 600
```
（单位秒: 600=10分钟, 300=5分钟, 1200=20分钟）

### 14. 电池息屏时间
> 用电池多久息屏

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetScreenTimeoutOnDC
```

### 15. AC睡眠时间
> 插着电多久睡眠

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetSleepComputerTimeOnAC
```

### 16. 低电量策略
> 低电量时会怎样

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryAction
```

### 17. 低电量阈值
> 低于多少算低电量

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryActionThreshold
```

### 18. 低电量通知阈值
> 多少电量时提醒

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryNotifyThreshold
```

### 19. 显示剩余时间
> 显示电池剩余时间吗

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsShowLeftTime
```

### 20. 有电池/有盖子
> 有没有电池 / 有没有盖子

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsBatteryPresent

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsLid
```

### 21. AC/DC电源策略
> 插电时电源策略

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetACPowerPolicy

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetDCPowerPolicy
```
返回值: 0=性能, 1=平衡, 2=节能

---

## 完整对话样例

**你**: 电源状态
**誩**: 电量 75%，放电中，可用约 3小时20分，当前平衡模式

**你**: 切到性能模式
**誩**: ✅ 已切换到性能模式

**你**: 合盖改成睡眠
**誩**: ✅ 合盖动作已设为睡眠

**你**: 插电时5分钟息屏，15分钟睡眠
**誩**: ✅ 已设置：AC息屏300秒，AC睡眠900秒

**你**: 电量低于10%时自动休眠
**誩**: ✅ 已设置：低电量动作 "hibernate"，阈值 10%
