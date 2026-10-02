# 辅助功能

打开页面：`ukui-control-center -m accessibility`

接口：session DBus `org.ukui.ukcc.session.Accessibility`，对象路径 `/Accessibility`。

## 推荐：统一脚本

脚本路径（勿用相对路径 `scripts/...`，工作区 cwd 下常找不到）：

`/opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh`

```bash
SCRIPT=/opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh
bash "$SCRIPT" zoom on|off|status
bash "$SCRIPT" zoom-mode window|fullscreen|status   # 1=窗口 0=全屏
bash "$SCRIPT" color-filter on|off
bash "$SCRIPT" screen-reader on|off
bash "$SCRIPT" reader-volume 60
bash "$SCRIPT" reader-speed 70
```

关停类请优先说「停止」（如「停止屏幕朗读」「停止色彩滤镜」），再执行脚本或下方 gdbus/gsettings。

## 放大镜开关

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setZoomSwitch true
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setZoomSwitch false
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.isZooming
```

## 放大视图

| 视图 | 参数值 |
|------|--------|
| 窗口放大 | `1` |
| 全屏放大 | `0` |

禁止 `gsettings set org.ukui.control-center zoom-mode`（无此键）。

```bash
# 窗口放大
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setZoomMode 1
# 全屏放大
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setZoomMode 0
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.GetType
```

## 色彩滤镜类型

| 滤镜 | 参数值 |
|------|--------|
| 红/绿滤镜（红色盲） | `Protanopia` |
| 绿/红滤镜（绿色盲） | `Deuteranopia` |
| 蓝/黄滤镜（蓝色盲） | `Tritanopia` |
| 灰度 | `GrayScale` |
| 反色 | `Invert` |

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setColorFilter "Protanopia"
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.getCurrentColorFilter
```

## 色彩滤镜开关

| 用户说法 | 命令 |
|----------|------|
| 开启 / 打开 / 启用色彩滤镜 | `ukcc_scripts.sh color-filter on` |
| 关闭 / 关掉 / 停止色彩滤镜 | `ukcc_scripts.sh color-filter off` |

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setColorFilterSwitch true
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.setColorFilterSwitch false
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.isColorFilterEnabled
```

## 屏幕朗读

需已安装 orca。

| 用户说法 | 命令 |
|----------|------|
| 开启 / 打开 / 启用屏幕朗读 | `ukcc_scripts.sh screen-reader on` |
| 关闭 / 关掉 / 停止屏幕朗读 | `ukcc_scripts.sh screen-reader off` |

```bash
gsettings set org.gnome.desktop.a11y.applications screen-reader-enabled true
gsettings set org.gnome.desktop.a11y.applications screen-reader-enabled false
gsettings get org.gnome.desktop.a11y.applications screen-reader-enabled

gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.toggleScreenReader
gdbus call --session --dest org.ukui.ukcc.session --object-path /Accessibility \
  --method org.ukui.ukcc.session.Accessibility.isScreenReaderRunning
```

## 朗读音量 / 语速

**调音量/语速 ≠ 开启屏幕朗读。** 禁止对「设置朗读音量到 N」执行 `screen-reader-enabled true`。

| 用户说法 | 命令 |
|----------|------|
| 设置/调整屏幕朗读音量到 60 | `ukcc_scripts.sh reader-volume 60`（推荐）或 `gsettings set org.ukui.control-center reader-volume 60` |
| 设置/调整屏幕朗读语速到 70 | `ukcc_scripts.sh reader-speed 70` 或 `gsettings set org.ukui.control-center reader-speed 70` |

```bash
# 推荐脚本（schema 未装齐时脚本会回退 dconf）
bash scripts/ukcc_scripts.sh reader-volume 60
bash scripts/ukcc_scripts.sh reader-volume status
bash scripts/ukcc_scripts.sh reader-speed 70

# 音量 0–100，默认 50（需已安装含 reader-volume 的 org.ukui.control-center schema）
gsettings set org.ukui.control-center reader-volume 60
gsettings get org.ukui.control-center reader-volume

# 语速 30–100，默认 50（越大越快）
gsettings set org.ukui.control-center reader-speed 70
gsettings get org.ukui.control-center reader-speed
```
