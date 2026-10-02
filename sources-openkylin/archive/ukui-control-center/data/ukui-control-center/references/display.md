# 显示器与亮度

打开页面：`ukui-control-center -m display`

## 列出输出

```bash
kscreen-doctor -o
```

## 分辨率

```bash
# 格式: output.<输出名>.mode.<宽>x<高>@<刷新率>
kscreen-doctor output.HDMI-A-3.mode.1920x1080@60
```

## 旋转

可选值: `none`, `left`, `right`, `inverted`

```bash
kscreen-doctor output.HDMI-A-3.rotation.left
```

## 缩放

```bash
# fractional scaling 仅在 Wayland 下支持
kscreen-doctor output.HDMI-A-3.scale.2
```

## 位置

```bash
# position.X,Y 相对于虚拟桌面左上角
kscreen-doctor output.HDMI-A-3.position.1920,0
```

## 启用 / 禁用

```bash
kscreen-doctor output.HDMI-A-3.enable
kscreen-doctor output.HDMI-A-3.disable
```

## 组合设置

```bash
kscreen-doctor output.HDMI-A-3.mode.1920x1080@60 output.HDMI-A-3.rotation.left output.HDMI-A-3.scale.1
```

## 亮度

### 检测设备类型

```bash
ls /sys/class/power_supply/BAT* 2>/dev/null
# 有结果 → 笔记本；无结果 → 台式机
```

### 笔记本（gsettings）

```bash
gsettings get org.ukui.power-manager brightness-ac
gsettings get org.ukui.power-manager brightness-bat
gsettings set org.ukui.power-manager brightness-ac 80
gsettings set org.ukui.power-manager brightness-bat 50
```

### 台式机（Session DBus）

```bash
dbus-send --session --dest=org.ukui.SettingsDaemon --print-reply \
  /org/ukui/SettingsDaemon/GammaManager \
  org.ukui.SettingsDaemon.GammaManager.setScreenBrightness \
  string:"openclaw" \
  string:"HDMI-A-3" \
  uint32:80

dbus-send --session --dest=org.ukui.SettingsDaemon --print-reply \
  /org/ukui/SettingsDaemon/GammaManager \
  org.ukui.SettingsDaemon.GammaManager.getPrimaryBrightness
```

| 参数 | 类型 | 说明 |
|------|------|------|
| appName | string | 调用者名称 |
| screenName | string | 显示器名，如 `HDMI-A-3`、`eDP-1` |
| screenBrightness | uint32 | 0–100 |

### 流程

```
有电池 → 笔记本 → gsettings brightness-ac/bat
无电池 → 台式机 → setScreenBrightness
```

操作前先用 `kscreen-doctor -o` 确认输出名。
