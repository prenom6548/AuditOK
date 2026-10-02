# 远程桌面

打开页面：`ukui-control-center -m vino`

接口：session DBus `com.kylin.RemoteDesktop`，对象路径 `/com/kylin/RemoteDesktop`。

## 允许远程接入本机

```bash
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.GetCurrentDevice

gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.GetServerType

# 协议类型 0 → VNC
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.Start display
# 协议类型 1 → RDP
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.StartRDP display

# 关闭
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.Exit
```

## 远程协议（VNC / RDP）

先 `GetCurrentDevice` 取 display，再：

- VNC：`Start display`
- RDP：`StartRDP display`

## 允许其他用户控制屏幕

```bash
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetControlPermissionStatus true
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetControlPermissionStatus false
```

## 访问时需要确认

```bash
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetVisitConfirmStatus true
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetVisitConfirmStatus false
```

## VNC 密码

```bash
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetAuthMethod true
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetPassword 密码
```

## 允许访问的显示器

```bash
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.GetSupportedOutputDevice
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.GetServerType
# 类型 0 → Start 显示器名；类型 1 → StartRDP 显示器名
```

未提供显示器名时：先 `GetSupportedOutputDevice` 让用户选择，再按协议类型启动。

## 允许连接的客户端数量

```bash
# value 范围 1–10
gdbus call --session --dest com.kylin.RemoteDesktop --object-path /com/kylin/RemoteDesktop \
  --method com.kylin.RemoteDesktop.SetAllowedMaxClient 2
```
