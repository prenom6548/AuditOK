---
name: power
description: Power management operations
version: 0.0.1
author: ukui
tags:
  - power
---

# Power management conversation tests

## Test flow

### 1. Get battery level
> How much battery is left

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.BatteryPercentage
```

### 2. Charging state
> Is it charging

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetBatteryState
```
Values: 0=unknown, 1=charging, 2=discharging, 3=empty, 4=full, 5=pending charge, 6=pending discharge

### 3. Time remaining
> How long can it last

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.TimeToEmpty
```

### 4. Time to full
> How long until fully charged

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.TimeToFull
```

### 5. Power mode
> What's the current power mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetPowerMode
```
Values: 0=performance, 1=balanced, 2=power saver

### 6. Switch to performance
> Switch to performance mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerMode 0
```

### 7. Switch to power saver
> Power saver mode

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerMode 2
```

### 8. Power button action
> What happens when I press the power button

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetPowerButtonAction
```

### 9. Set power button action
> Change the power button to hibernate

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetPowerButtonAction "hibernate"
```
Values: "interactive" (ask), "blank" (screen off), "suspend", "hibernate", "shutdown"

### 10. Lid close action
> What happens when I close the lid

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLidCloseAction
```

### 11. Set lid close action
> Close lid should sleep

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetLidCloseAction "suspend"
```
Values: "nothing", "blank", "suspend", "hibernate", "shutdown"

### 12. AC screen timeout
> How long until the screen turns off on AC

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetScreenTimeoutOnAC
```

### 13. Set AC screen timeout
> Turn the screen off after 10 minutes on AC

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.SetScreenTimeoutOnAC 600
```
(Seconds: 600=10 min, 300=5 min, 1200=20 min)

### 14. Battery screen timeout
> How long until the screen turns off on battery

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetScreenTimeoutOnDC
```

### 15. AC sleep timeout
> How long until sleep on AC

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetSleepComputerTimeOnAC
```

### 16. Low battery action
> What happens on low battery

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryAction
```

### 17. Low battery threshold
> What percentage is low battery

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryActionThreshold
```

### 18. Low battery notify threshold
> At what percentage should I be notified

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetLowBatteryNotifyThreshold
```

### 19. Show remaining time
> Show remaining battery time

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsShowLeftTime
```

### 20. Has battery / has lid
> Does it have a battery / a lid

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsBatteryPresent

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.IsLid
```

### 21. AC / DC power policy
> Power policy on AC

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetACPowerPolicy

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Power \
  --method org.ukui.Framework.Devices.Power.GetDCPowerPolicy
```
Values: 0=performance, 1=balanced, 2=power saver

---

## Sample conversation

**You**: Power status
**Bot**: Battery 75%, discharging, about 3h20m left, currently balanced mode

**You**: Switch to performance mode
**Bot**: ✅ Switched to performance mode

**You**: Close lid should sleep
**Bot**: ✅ Lid close action set to sleep

**You**: On AC, screen off after 5 minutes, sleep after 15
**Bot**: ✅ Set: AC screen 300s, AC sleep 900s

**You**: Hibernate when battery is below 10%
**Bot**: ✅ Set: low-battery action "hibernate", threshold 10%
