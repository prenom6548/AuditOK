---
name: navigations
description: Open system apps and Control Center pages
version: 0.0.1
author: ukui
tags:
  - system apps
---

# Navigations — open system apps

## Test flow

### Open Control Center pages

> Open Control Center — Sound
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccAudio
```

> Open Display settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccDisplay
```

> Open Theme settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccTheme
```

> Open Power management
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccPower
```

> Open Bluetooth
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccBluetooth
```

> Open Touchpad settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccTouchpad
```

> Open Date & time
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccDate
```

> Open Keyboard shortcuts
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccShortcut
```

> Open Keyboard / input method
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccKeyboard
```

> Open wireless network
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccWlanconnect
```

> Open wired network
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccNetconnect
```

> Open user info
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccUserInfo
```

> Open screensaver settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccScreensaver
```

> Open lock screen settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccScreenlock
```

> Open notification settings
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccNotice
```

> Open system updates
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccUpgrade
```

> Open Security Center
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccSecurityCenter
```

> Open remote desktop
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccVino
```

> Open printers
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccPrinter
```

> Open network proxy
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccProxy
```

> Open mobile hotspot
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccMobileHotspot
```

> Open region & language
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccArea
```

> Open backup & restore
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccBackup
```

> Open startup apps
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccAutostart
```

> Open login options
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccLoginOptions
```

### Open other system apps

> Open Device Manager
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyomDeviceManager
```

> Open System Monitor
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUksmAbout
```

> Open Clock
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkClock
```

> Open Music Player
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyMusic
```

> Open Video Player
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyVideo
```

---

## Sample conversation

**You**: Open Display settings in Control Center
**Bot**: ✅ Opened Display settings

**You**: Open System Monitor
**Bot**: ✅ Opened System Monitor

**You**: Open the Clock app
**Bot**: ✅ Opened Clock
