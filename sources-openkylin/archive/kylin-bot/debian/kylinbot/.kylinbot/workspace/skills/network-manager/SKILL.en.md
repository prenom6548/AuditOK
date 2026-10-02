---
name: network-manager
description: >
  Manage Linux desktop network connections with NetworkManager (nmcli).
  Use this when the user wants to configure IP addresses (static / DHCP),
  connect to Wi-Fi or Ethernet, join Wi-Fi with a password, turn Wi-Fi or
  Ethernet on or off, disconnect, list available networks, or manage
  network settings on a Linux device.
  Trigger phrases: set IP, connect to network, connect wifi, toggle
  wireless, toggle ethernet, disconnect, network config, wifi connect,
  turn off wifi, set static ip, and similar requests.
---

# Network Manager

All operations use `nmcli` directly. Run commands via the `exec` tool.

## 0. Query status

```bash
# Device status
nmcli device status

# List all connection profiles
nmcli -t -f NAME,UUID,TYPE,DEVICE connection show

# List active connections
nmcli -t -f NAME,DEVICE connection show --active

# Scan available Wi-Fi
nmcli -t -f SSID,SECURITY,SIGNAL device wifi list
```

## 1. Configure IP address

**DHCP:**
```bash
nmcli connection modify "<connection>" ipv4.method auto
nmcli connection up "<connection>"
```

**Static IP:**
```bash
nmcli connection modify "<connection>" \
  ipv4.method manual \
  ipv4.addresses "<IP/prefix>" \
  ipv4.gateway "<gateway>" \
  ipv4.dns "<DNS>"
nmcli connection up "<connection>"
```

Example: `nmcli connection modify "Wired connection 1" ipv4.method manual ipv4.addresses 192.168.1.100/24 ipv4.gateway 192.168.1.1 ipv4.dns 8.8.8.8 && nmcli connection up "Wired connection 1"`

## 2. Activate a connection

```bash
nmcli connection up "<name or UUID>"
```

## 3. Connect to Wi-Fi (with password)

```bash
nmcli device wifi connect "<SSID>" password "<password>"
```

Open network (no password):
```bash
nmcli device wifi connect "<SSID>"
```

## 4. Turn Wi-Fi on or off

```bash
# On
nmcli radio wifi on

# Off
nmcli radio wifi off

# Status
nmcli radio wifi
```

## 5. Turn Ethernet on or off

Find the Ethernet device name (usually `enp*` or `eth*`):
```bash
nmcli -t -f DEVICE,TYPE device status | awk -F: '$2=="ethernet" {print $1}'
```

```bash
# On
nmcli device set <device> managed yes && nmcli device connect <device>

# Off
nmcli device disconnect <device> && nmcli device set <device> managed no
```

## 6. Disconnect

By connection name:
```bash
nmcli connection down "<connection>"
```

By device name:
```bash
nmcli device disconnect <device>
```
