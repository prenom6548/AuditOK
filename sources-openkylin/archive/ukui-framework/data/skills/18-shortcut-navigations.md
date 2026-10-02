# 快捷键 (ShortcutNavigations) 对话测试

## 测试流程

### 音量键
> 增大音量 / 减小音量 / 静音

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

### 亮度键
> 亮度增加 / 亮度减少

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.BrightUp

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.BrightDown
```

### 触摸板
> 开关触摸板 / 切换 / 开启 / 关闭

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

### 电源
> 电源键按下 / 关机

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.PowerDown

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/ShortcutNavigations \
  --method org.ukui.Framework.ShortcutNavigations.PowerOff
```

### 网络
> WiFi开关 / 飞行模式 / 蓝牙

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

### 截图
> 全屏截图 / 窗口截图 / 区域截图

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

### 打开应用
> 打开xxx

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

### 多媒体键

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

## 完整对话样例

**你**: 帮我截图
**誩**: ✅ 全屏截图已触发

**你**: 开飞行模式
**誩**: ✅ 飞行模式已开启

**你**: 打开终端
**誩**: ✅ 终端已打开

**你**: 下一首
**誩**: ✅ 已切到下一首

**你**: 性能模式切换
**誩**: ✅ 已切换能效模式
