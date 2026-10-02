# ukui-control-center

> **硬性规则（改设置必读）**  
> 1. 「设置 / 开启 / 关闭 / 切换 XXX」→ **必须**用 `references/` 中的 `gdbus` / `gsettings` / `kscreen-doctor` 真正改配置。  
> 2. **禁止**只用 `datetime_settings`、`region_language`、`app_control`、`install_app` 打开设置页或搜应用后声称已完成。  
> 3. 仅当用户明确要求「打开某某设置页面」时，才用 `ukui-control-center -m <模块>`。  
> 4. 放大镜 / 色彩滤镜 / 屏幕朗读不是独立应用：禁止 `app_control`；关停请说「停止…」并执行对应 gdbus/gsettings / `ukcc_scripts.sh … off`。  
> 5. 禁止臆造 DBus 路径（不要用 `org.ukui.SettingsDaemon` 代替 `org.ukui.ukcc.session`）。  
> 6. **系统语言**：用户说「设置/切换系统语言 / 改成英文|中文」→ **必须** shell 执行 `Area.setLanguage`（或 `ukcc_scripts.sh language …`）；禁止只打开区域语言页。

麒麟控制面板（ukui-control-center）设置技能：打开指定模块，或通过 DBus / gsettings / kscreen-doctor 完成功能设置。

## 能力概览

- 打开控制面板并跳转到具体模块（`ukui-control-center -m <模块>`）
- 显示器：分辨率、旋转、缩放、位置、启用/禁用、亮度
- 账户：添加/删除用户、昵称、密码、头像、免密/自动登录、用户组
- 主题：浅色/深色/自动、强调色、图标主题、光标主题
- 背景与字体、时间日期、区域语言、辅助功能、远程桌面、鼠标键盘
- 关于（诊断数据）、系统启动（Grub 验证）

详细命令见 `references/`，按需加载对应文件后执行，勿一次读完所有参考文件。

## 激活条件

用户提及以下内容时使用本技能：

- 打开控制面板 / 打开设置 / ukui-control-center
- 显示器 / 分辨率 / 亮度 / 缩放 / 旋转 / 多屏 / 外接显示器
- 主题 / 浅色 / 深色 / 强调色 / 图标 / 光标
- 账户 / 添加用户 / 删除用户 / 改密码 / 免密登录 / 自动登录
- 辅助功能 / 无障碍 / 放大镜 / 色彩滤镜 / 屏幕朗读 / 停止屏幕朗读 / 停止色彩滤镜
- 时间 / 日期 / 时区 / 时间格式 / 日历格式 / 区域语言 / 系统语言
- 背景 / 壁纸 / 字体
- 远程桌面 / 鼠标 / 键盘 / 蓝牙 / 网络 / 电源 / 关于 / Grub

## 工作流程

1. **区分「打开页面」与「改设置」**：只要「打开设置」→ `-m` 跳转；要「设置成某某」→ 读 `references/` 执行设置命令。
2. **禁止假完成**：不得在仅打开页面后回复「已设置成功」。
3. **缺参先问**：用户名、显示器名、目标值等未提供时先询问。
4. **执行后确认**：用 get/查询命令或回报命令结果；失败时说明原因。

### 模块跳转（统一命令）

```bash
ukui-control-center -m <模块名>
```

| 用户意图 | 模块名 | 命令 |
|----------|--------|------|
| 打开设置 / 显示器 | display | `ukui-control-center -m display` |
| 声音 | audio | `ukui-control-center -m audio` |
| 电源 | power | `ukui-control-center -m power` |
| 关于 | about | `ukui-control-center -m about` |
| 打印机 | printer | `ukui-control-center -m printer` |
| 鼠标 | mouse | `ukui-control-center -m mouse` |
| 键盘 | keyboard | `ukui-control-center -m keyboard` |
| 快捷键 | shortcut | `ukui-control-center -m shortcut` |
| 蓝牙 | bluetooth | `ukui-control-center -m bluetooth` |
| 多屏协同 | projection | `ukui-control-center -m projection` |
| 有线网络 | netconnect | `ukui-control-center -m netconnect` |
| 无线网络 | wlanconnect | `ukui-control-center -m wlanconnect` |
| 热点 | mobilehotspot | `ukui-control-center -m mobilehotspot` |
| 代理 | proxy | `ukui-control-center -m proxy` |
| VPN | vpn | `ukui-control-center -m vpn` |
| 背景 | wallpaper | `ukui-control-center -m wallpaper` |
| 主题 | theme | `ukui-control-center -m theme` |
| 字体 | fonts | `ukui-control-center -m fonts` |
| 时间和日期 | date | `ukui-control-center -m date` |
| 区域语言 | area | `ukui-control-center -m area` |
| 备份 | backup | `ukui-control-center -m backup` |
| 更新 | upgrade | `ukui-control-center -m upgrade` |
| 安全 | securitycenter | `ukui-control-center -m securitycenter` |
| 开机启动 | autostart | `ukui-control-center -m autostart` |
| 默认应用 | defaultapp | `ukui-control-center -m defaultapp` |
| 搜索 | search | `ukui-control-center -m search` |
| 账户信息 | userinfo | `ukui-control-center -m userinfo` |
| 登录选项 | biometrics | `ukui-control-center -m biometrics` |
| 云账户 | cloud account | `ukui-control-center -m "cloud account"` |
| 远程桌面 | vino | `ukui-control-center -m vino` |
| 辅助功能 | accessibility | `ukui-control-center -m accessibility` |
| 系统启动 | boot | `ukui-control-center -m boot` |

完整模块表见 [references/modules.md](references/modules.md)。

## 参考资料（按需加载）

| 场景 | 文件 |
|------|------|
| 模块跳转全集 | [references/modules.md](references/modules.md) |
| 显示器 / 亮度 | [references/display.md](references/display.md) |
| 账户信息 | [references/accounts.md](references/accounts.md) |
| 主题 / 强调色 / 图标 / 光标 | [references/theme.md](references/theme.md) |
| 背景 | [references/wallpaper.md](references/wallpaper.md) |
| 字体 | [references/fonts.md](references/fonts.md) |
| 时间日期 | [references/datetime.md](references/datetime.md) |
| 区域语言 | [references/area.md](references/area.md) |
| 辅助功能 | [references/accessibility.md](references/accessibility.md) |
| 远程桌面 | [references/remote-desktop.md](references/remote-desktop.md) |
| 关于 / 诊断数据 | [references/about.md](references/about.md) |
| 鼠标 | [references/mouse.md](references/mouse.md) |
| 键盘 | [references/keyboard.md](references/keyboard.md) |
| 系统启动 / Grub | [references/boot.md](references/boot.md) |

## 注意事项

- 显示器名从 `kscreen-doctor -o` 获取（如 `eDP-1`、`HDMI-A-3`）。
- 改设置优先用参考文件中的 DBus / gsettings / kscreen-doctor，不要臆造命令。
- 意图映射与参考文件不一致时，以参考文件中的可执行命令为准。
- VPN 等仅跳转、无设置命令的模块：只打开对应页面。
- 修改区域格式或系统语言后，部分设置需注销或重新登录生效。
- Grub 验证操作需要管理员认证，可能较慢。
