---
name: network-manager
description: >
  通过 NetworkManager（nmcli 命令行工具）管理 Linux 桌面版网络连接。
  适用于用户提出以下需求时：配置 IP 地址（静态 IP / 动态 DHCP）、连接 Wi‑Fi 或有线网络、使用密码连接 Wi‑Fi、开启 / 关闭 Wi‑Fi 或以太网、断开网络连接、列出可用网络，或在 Linux 设备上管理网络设置。
  触发关键词：设置 IP、连接网络、连接 wifi、无线网络开关、有线网络开关、断开网络、network config、wifi connect、turn off wifi、set static ip 及类似请求。
---

# Network Manager

All operations use `nmcli` directly. Execute commands via the `exec` tool.

## 0. 查询信息

```bash
# 设备状态
nmcli device status

# 列出所有连接配置
nmcli -t -f NAME,UUID,TYPE,DEVICE connection show

# 列出活跃连接
nmcli -t -f NAME,DEVICE connection show --active

# 扫描可用 Wi-Fi
nmcli -t -f SSID,SECURITY,SIGNAL device wifi list
```

## 1. 配置 IP 地址

**DHCP：**
```bash
nmcli connection modify "<连接名>" ipv4.method auto
nmcli connection up "<连接名>"
```

**静态 IP：**
```bash
nmcli connection modify "<连接名>" \
  ipv4.method manual \
  ipv4.addresses "<IP/前缀>" \
  ipv4.gateway "<网关>" \
  ipv4.dns "<DNS>"
nmcli connection up "<连接名>"
```

示例：`nmcli connection modify "有线连接 1" ipv4.method manual ipv4.addresses 192.168.1.100/24 ipv4.gateway 192.168.1.1 ipv4.dns 8.8.8.8 && nmcli connection up "有线连接 1"`

## 2. 连接指定连接

```bash
nmcli connection up "<连接名或UUID>"
```

## 3. 连接 Wi-Fi（带密码）

```bash
nmcli device wifi connect "<SSID>" password "<密码>"
```

开放网络（无密码）：
```bash
nmcli device wifi connect "<SSID>"
```

## 4. 打开/关闭 Wi-Fi

```bash
# 开启
nmcli radio wifi on

# 关闭
nmcli radio wifi off

# 查看状态
nmcli radio wifi
```

## 5. 打开/关闭有线网络

找到有线设备名（通常为 `enp*` 或 `eth*`）：
```bash
nmcli -t -f DEVICE,TYPE device status | awk -F: '$2=="ethernet" {print $1}'
```

```bash
# 开启
nmcli device set <设备名> managed yes && nmcli device connect <设备名>

# 关闭
nmcli device disconnect <设备名> && nmcli device set <设备名> managed no
```

## 6. 断开指定连接

按连接名断开：
```bash
nmcli connection down "<连接名>"
```

按设备名断开：
```bash
nmcli device disconnect <设备名>
```
