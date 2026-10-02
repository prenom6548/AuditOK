---
name: shortcut-navigations
description: Shortcut-key related actions
version: 0.0.1
author: ukui
tags:
  - shortcuts
---

# Shortcuts (ShortcutNavigations) conversation tests

## Test flow

### Volume keys
> Volume up / volume down / mute

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.VolumeUp

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.VolumeDown

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MuteToggleState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MicMuteToggleState
```

### Brightness keys
> Brightness up / brightness down

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.BrightUp

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.BrightDown
```

### Touchpad
> Toggle / turn on / turn off the touchpad

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.TouchpadToggleState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.TouchpadOn

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.TouchpadOff
```

### Power
> Power button / shut down

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.PowerDown

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.PowerOff
```

### Network
> Wi-Fi toggle / airplane mode / Bluetooth

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.ToggleWlanSwitch

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.ToggleFlightModeState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.TurnOnFlightModeState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.TurnOffFlightModeState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.ToggleBluetoothState
```

### Screenshots
> Full screen / window / area screenshot

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.Screenshot

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.WindowScreenshot

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.AreaScreenshot
```

### Open apps
> Open xxx

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenFileManager

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenCalculator

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenEmail

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenWWW

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenSettings

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenTerminal

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenGlobalSearch

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenUkuiSidebar

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenWindowSwitch

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenScreensaver

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenClipboard

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenKylinManager

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenKylinScreencap

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenDisplaySwitch

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.SwitchPerformanceMode

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.OpenAIassistant

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.CamToggleState
```

### Media keys

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaPlay

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaPause

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaPlaypause

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaStop

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaNext

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaPrevious

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaRepeat

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.MediaShuffleKey
```

---

## Sample conversation

**You**: Take a screenshot
**Bot**: ✅ Full-screen screenshot triggered

**You**: Turn on airplane mode
**Bot**: ✅ Airplane mode on

**You**: Open the terminal
**Bot**: ✅ Terminal opened

**You**: Next track
**Bot**: ✅ Skipped to the next track

**You**: Switch performance mode
**Bot**: ✅ Switched power mode
