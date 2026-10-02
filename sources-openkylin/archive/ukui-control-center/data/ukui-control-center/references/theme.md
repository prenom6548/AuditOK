# 主题 / 强调色 / 图标 / 光标

打开页面：`ukui-control-center -m theme`

## 主题切换

### 浅色

```bash
gsettings set org.ukui.style style-name ukui-light
gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic false
```

### 深色

```bash
gsettings set org.ukui.style style-name ukui-dark
gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic false
```

### 自动

```bash
gsettings set org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic true
```

### 查询

```bash
gsettings get org.ukui.style style-name
gsettings get org.ukui.SettingsDaemon.plugins.color theme-schedule-automatic
```

## 强调色

| 颜色 | 色值 |
|------|------|
| 果酱紫 | `'120,115,245,1'` |
| 玫红色 | `'228,74,232,1'` |
| 薄雾金 | `'242,181,39,1'` |
| 极光绿 | `'20,166,33,1'` |
| 烈日红 | `'240,73,67,1'` |
| 日暮橙 | `'250,125,15,1'` |
| 蔚蓝 | `'6,192,199,1'` |
| 拂晓蓝 | `'120,115,245,1'` |

```bash
gsettings set org.ukui.style theme-color '120,115,245,1'
gsettings get org.ukui.style theme-color
```

## 图标主题

| 名称 | 值 |
|------|------|
| 光（寻光） | `ukui-icon-theme-default` |
| 印（和印） | `ukui-icon-theme-fashion` |
| 典（启典） | `ukui-icon-theme-classical` |

```bash
gsettings set org.ukui.style icon-theme-name ukui-icon-theme-default
gsettings get org.ukui.style icon-theme-name
```

## 光标主题

| 名称 | 值 |
|------|------|
| 印白 | `DMZ-White` |
| 典蓝 | `blue-crystal` |
| 耀黑 | `dark-sense` |

```bash
gsettings set org.ukui.peripherals-mouse cursor-theme DMZ-White
gsettings get org.ukui.peripherals-mouse cursor-theme
```
