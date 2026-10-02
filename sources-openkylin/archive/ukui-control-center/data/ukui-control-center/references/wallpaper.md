# 背景设置

打开页面：`ukui-control-center -m wallpaper`

接口：session DBus `org.ukui.ukcc.session.Wallpaper`，对象路径 `/Wallpaper`。

## 背景图片

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.setWallpaper "picture" "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.getWallpaper

gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.getWallpaperMode
# picture：图片，color：颜色
```

支持本地 jpg/jpeg/png/gif/bmp 等。

## 背景颜色

```bash
# 颜色为 # 开头的 6 位十六进制
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.setWallpaper "color" "#2E3440"
```

## 显示方式

参数类型为**字符串**，禁止传整数 `0`/`1`（会被当成无效值静默忽略）。

中文与参数对照（以控制面板翻译为准，勿按英文望文生义）：

| 显示方式（中文） | 参数值 | 英文源串 |
|------------------|--------|----------|
| 填充 | `scaled` | Scaled |
| 平铺 | `wallpaper` | Wallpaper |
| 居中 | `centered` | Centered |
| 拉伸 | `stretched` | Stretched |
| 适应 | `zoom` | Zoom |
| 跨区 | `spanned` | Spanned |

注意：**填充 = `scaled`，适应 = `zoom`**（不要把「填充」写成 `zoom`）。

```bash
# 填充
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.setDisplayType "scaled"
# 适应
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.setDisplayType "zoom"
# 平铺
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.setDisplayType "wallpaper"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.freedesktop.DBus.Properties.Get org.ukui.ukcc.session.Wallpaper displayType
```

## 恢复默认

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Wallpaper \
  --method org.ukui.ukcc.session.Wallpaper.resetDefault
```
