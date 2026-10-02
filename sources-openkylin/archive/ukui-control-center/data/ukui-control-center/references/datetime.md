# 时间和日期

打开页面：`ukui-control-center -m date`

接口：session DBus `org.ukui.ukcc.session.Datetime`，对象路径 `/Datetime`。

## 修改时间

流程：未提供目标时间则询问 → 切到手动模式 → 设置时间。

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Datetime \
  --method org.ukui.ukcc.session.Datetime.setTimeMode "manual"

# timeData = Unix 秒 × 1000000（微秒）
gdbus call --session --dest org.ukui.ukcc.session --object-path /Datetime \
  --method org.ukui.ukcc.session.Datetime.setTime $(($(date -d "2026-07-09 17:00:00" +%s)*1000000))

gdbus call --session --dest org.ukui.ukcc.session --object-path /Datetime \
  --method org.ukui.ukcc.session.Datetime.getTimeMode
# manual：手动，automatic：自动同步
```

## 修改时区

| 时区名称 | 参数值 |
|----------|--------|
| 上海/北京时间 | `Asia/Shanghai` |
| 北京 | `Asia/Beijing` |
| 乌鲁木齐 | `Asia/Urumqi` |
| 东京 | `Asia/Tokyo` |
| 伦敦 | `Europe/London` |
| 纽约 | `America/New_York` |

说明：IANA 时区；`Asia/Beijing`、`Asia/Urumqi` 等可能映射为 `Asia/Shanghai`。

```bash
gdbus call --session --dest org.ukui.ukcc.session --object-path /Datetime \
  --method org.ukui.ukcc.session.Datetime.setTimezone "Asia/Shanghai"

gdbus call --session --dest org.ukui.ukcc.session --object-path /Datetime \
  --method org.ukui.ukcc.session.Datetime.getTimezone
```
