# 字体设置

打开页面：`ukui-control-center -m fonts`

接口：session DBus `org.ukui.ukcc.session.Font`，对象路径 `/Font`。

## 字体大小

| 档位 | 字号（pt） |
|------|------------|
| 1 | `10` |
| 2 | `12` |
| 3 | `13.5` |
| 4 | `15` |

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Font \
  --method org.ukui.ukcc.session.Font.setFontSize 12

gdbus call --session --dest org.ukui.ukcc.session --object-path /Font \
  --method org.ukui.ukcc.session.Font.getFontSize
```

## 等宽字体

**值必须是「字体族 + 空格 + 字号」**，与控制面板一致。禁止只写字体名（如 `FreeMono`），否则 UI 下拉框无法匹配。

控制面板会同时写入 mate 与 gnome；设置时两边都要改。

```bash
# 正确：带字号（字号可用当前 system-font-size，常见 10/11/12）
gsettings set org.mate.interface monospace-font-name 'FreeMono 12'
gsettings set org.gnome.desktop.interface monospace-font-name 'FreeMono 12'

# 或
bash /opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh monospace FreeMono 12

# 查询
gsettings get org.mate.interface monospace-font-name

# 错误（禁止）:
# gsettings set org.mate.interface monospace-font-name 'FreeMono'
```

## 恢复默认字体

**必须**走 Font.resetDefault（会重置界面/文档/等宽字体及 ukui.style 系统字体，并通知桌面刷新）。

禁止：
- `gsettings reset org.mate.interface monospace-font-name`（只清等宽且常不刷新 UI）
- 空口「已恢复」/ 只打开字体页

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Font \
  --method org.ukui.ukcc.session.Font.resetDefault

# 或
bash /opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh font-reset
```
