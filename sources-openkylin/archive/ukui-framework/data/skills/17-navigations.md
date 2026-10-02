# 导航 - 打开系统应用 (Navigations) 对话测试

## 测试流程

### 打开控制中心页面

> 打开控制中心-声音设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccAudio
```

> 打开显示器设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccDisplay
```

> 打开主题设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccTheme
```

> 打开电源管理
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccPower
```

> 打开蓝牙
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccBluetooth
```

> 打开触摸板设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccTouchpad
```

> 打开日期时间
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccDate
```

> 打开快捷键设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccShortcut
```

> 打开键盘/输入法
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccKeyboard
```

> 打开无线网络
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccWlanconnect
```

> 打开有线网络
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccNetconnect
```

> 打开用户信息
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccUserInfo
```

> 打开屏保设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccScreensaver
```

> 打开锁屏设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccScreenlock
```

> 打开通知设置
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccNotice
```

> 打开系统更新
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccUpgrade
```

> 打开安全中心
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccSecurityCenter
```

> 打开远程桌面
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccVino
```

> 打开打印机
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccPrinter
```

> 打开网络代理
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccProxy
```

> 打开移动热点
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccMobileHotspot
```

> 打开区域语言
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccArea
```

> 打开备份还原
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccBackup
```

> 打开开机自启
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccAutostart
```

> 打开登录选项
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkccLoginOptions
```

### 打开其他系统应用

> 打开设备管理器
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyomDeviceManager
```

> 打开系统监视器
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUksmAbout
```

> 打开闹钟
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenUkClock
```

> 打开音乐播放器
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyMusic
```

> 打开视频播放器
```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Navigations \
  --method org.ukui.Framework.Navigations.OpenKyVideo
```

---

## 完整对话样例

**你**: 打开控制中心的显示设置
**誩**: ✅ 已打开显示器设置页面

**你**: 打开系统监视器
**誩**: ✅ 已打开系统监视器

**你**: 帮我打开闹钟
**誩**: ✅ 已打开闹钟应用
