# 键盘设置

打开页面：`ukui-control-center -m keyboard`

## 按键重复输入

```bash
gsettings set org.ukui.peripherals-keyboard repeat false
gsettings set org.ukui.peripherals-keyboard repeat true
gsettings get org.ukui.peripherals-keyboard repeat
```

## 响应重复间隔

```bash
# 范围 200–2100
gsettings set org.ukui.peripherals-keyboard delay 500
gsettings get org.ukui.peripherals-keyboard delay
```

## 重复输入速度

```bash
# 范围 10–110
gsettings set org.ukui.peripherals-keyboard rate 50
gsettings get org.ukui.peripherals-keyboard rate
```

## Caps 键提示

```bash
gsettings set org.ukui.control-center.osd show-lock-tip false
gsettings set org.ukui.control-center.osd show-lock-tip true
gsettings get org.ukui.control-center.osd show-lock-tip
```

## 虚拟键盘大小

```bash
# 范围 20–100
gsettings set org.ukui.virtualkeyboard virtual-keyboard-scale-factor 50
gsettings get org.ukui.virtualkeyboard virtual-keyboard-scale-factor
```

## 虚拟键盘动画

```bash
gsettings set org.ukui.virtualkeyboard animation-enabled true
gsettings set org.ukui.virtualkeyboard animation-enabled false
gsettings get org.ukui.virtualkeyboard animation-enabled
```
