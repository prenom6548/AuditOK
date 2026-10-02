# 鼠标设置

打开页面：`ukui-control-center -m mouse`

## 主按键

```bash
gsettings set org.ukui.peripherals-mouse left-handed false   # 左键为主
gsettings set org.ukui.peripherals-mouse left-handed true    # 右键为主
gsettings get org.ukui.peripherals-mouse left-handed
```

## 双击速度

```bash
# 范围 170–1000
gsettings set org.ukui.peripherals-mouse double-click 500
gsettings get org.ukui.peripherals-mouse double-click
```

## 滚轮速度

```bash
# 范围 1–10
gsettings set org.ukui.peripherals-mouse wheel-speed 10
gsettings get org.ukui.peripherals-mouse wheel-speed
```

## 滚动方向

```bash
gsettings set org.ukui.peripherals-mouse natural-scroll false  # 正向
gsettings set org.ukui.peripherals-mouse natural-scroll true   # 反向
gsettings get org.ukui.peripherals-mouse natural-scroll
```

## 指针移动速度

```bash
# 范围 1.0–10.0
gsettings set org.ukui.peripherals-mouse motion-acceleration 5
gsettings get org.ukui.peripherals-mouse motion-acceleration
```

## 鼠标加速

```bash
gsettings set org.ukui.peripherals-mouse mouse-accel false
gsettings set org.ukui.peripherals-mouse mouse-accel true
gsettings get org.ukui.peripherals-mouse mouse-accel
```

## 定位指针

```bash
# 不定位
gsettings set org.ukui.peripherals-mouse locate-pointer false
gsettings set org.ukui.peripherals-mouse shake-cursor false
# Ctrl 显示位置
gsettings set org.ukui.peripherals-mouse locate-pointer true
gsettings set org.ukui.peripherals-mouse shake-cursor false
# 晃动显示位置
gsettings set org.ukui.peripherals-mouse locate-pointer false
gsettings set org.ukui.peripherals-mouse shake-cursor true
```

## 指针大小

```bash
# 小 24 / 中 36 / 大 48
gsettings set org.ukui.peripherals-mouse cursor-size 36
gsettings get org.ukui.peripherals-mouse cursor-size
```

## 文本光标闪烁

```bash
gsettings set org.mate.interface cursor-blink false
gsettings set org.mate.interface cursor-blink true
gsettings get org.mate.interface cursor-blink

# 闪烁频率 100–2500
gsettings set org.mate.interface cursor-blink-time 500
gsettings get org.mate.interface cursor-blink-time
```
