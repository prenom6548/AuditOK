# 区域语言

打开页面：`ukui-control-center -m area`

接口：session DBus `org.ukui.ukcc.session.Area`，对象路径 `/Area`。

修改区域格式或系统语言后，部分设置需注销或重新登录生效。

## 日期时间和货币格式

| 格式名称 | 参数值 |
|----------|--------|
| 英语（美国） | `en_US.UTF-8` |
| 简体中文（中国） | `zh_CN.UTF-8` |
| 藏语（中国） | `bo_CN.UTF-8` |
| 蒙古语（中国） | `mn_MN.UTF-8` |
| 繁体中文（中国香港） | `zh_HK.UTF-8` |

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setFormatCountry "zh_CN.UTF-8"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.getFormatCountry
```

## 日历格式

| 日历 | 参数值 |
|------|--------|
| 公历 | `solarlunar` |
| 农历 | `lunar` |

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setCalendar "solarlunar"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.getCalendar
```

## 一周的第一天

**必须执行 setFirstDay（shell/gdbus），禁止只用 `region_language` / 打开区域语言页后回复「已执行跳转」。禁止空口成功。**

| 选项 | 参数值 |
|------|--------|
| 周一 | `monday` |
| 周日 / 星期日 | `sunday` |

```bash
# 设为周日
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setFirstDay "sunday"
# 设为周一
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setFirstDay "monday"
# 或
bash /opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh first-day sunday

# 查询
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.freedesktop.DBus.Properties.Get org.ukui.ukcc.session.Area firstDay
```

## 日期短/长格式

`org.ukui.ukcc.session.Area` 无短/长日期格式的设置接口；需通过 `libkydate.h`（kysdk-system）C API：

- `kdk_system_set_short_dateformat("yyyy/MM/dd")`
- `kdk_system_get_shortformat()`
- `kdk_system_set_long_dateformat("yyyy年M月d日")`
- `kdk_system_get_longformat()`

`com.kylin.kysdk.DateServer` 仅提供变更信号，无设置方法。

## 时间格式

**必须执行 Area.setTimeFormat，禁止只用 datetime_settings 打开「时间日期」页面。**

| 格式 | 参数值 |
|------|--------|
| 12 小时制 | `12` |
| 24 小时制 | `24` |

禁止以下错误写法（改了也不会驱动区域语言页/面板时钟）：
- `gsettings set org.ukui.control-center.panel.plugins time '12'`（`time` 不是时间格式键）
- 只写 `hoursystem` 而不调 `setTimeFormat`（UI/时钟还依赖 kysdk 时间格式）

```bash
# 12 小时制
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setTimeFormat "12"
# 24 小时制
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setTimeFormat "24"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.freedesktop.DBus.Properties.Get org.ukui.ukcc.session.Area timeFormat
```

## 系统语言

**必须执行 setLanguage（shell/gdbus），禁止只用 `region_language` / `ukui-control-center -m area` 打开页面后声称已改语言。禁止空口「已设置成功」。**

| 语言 | 参数值 |
|------|--------|
| 简体中文 | `zh_CN` |
| 英语 | `en` |
| 藏语 | `bo_CN` |
| 蒙古语 | `mn_MN` |
| 繁体中文 | `zh_HK` |

```bash
# 设为简体中文（英语用 "en"，以此类推）
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.ukui.ukcc.session.Area.setLanguage "zh_CN"
# 或
bash /opt/kylinbot/skills/ukui-control-center/scripts/ukcc_scripts.sh language zh_CN

# 查询当前语言
gdbus call --session --dest org.ukui.ukcc.session --object-path /Area \
  --method org.freedesktop.DBus.Properties.Get org.ukui.ukcc.session.Area language
```

修改后部分界面需注销或重新登录才完全生效；执行命令后仍须回报已执行的 shell，不得假装成功。
