---
name: bluetooth
description: Bluetooth management. Supports power control, tray icon, discoverability, auto-discovery of audio devices, audio combine devices, connecting devices, and querying device info.
---

# UKUI Bluetooth management tool (ukui-bluetooth)

This script controls Bluetooth adapters and devices through the DBus system service `com.ukui.bluetooth`, with a simple subcommand interface.

## Prerequisites

- **OS**: Linux (UKUI desktop or any distro that provides `com.ukui.bluetooth`)
- **Python**: `dbus-python` (usually preinstalled)
- **DBus service**: `com.ukui.bluetooth` must be running
- **Permissions**: the current user must be allowed to call the service (typically in the `bluetooth` group, or via `sudo`)

## Script file

The script is `ukui-bluetooth-cli.py`. Run it as `python3 ukui-bluetooth-cli.py <command> [args]`.

## Commands

| Command | Args | Purpose | Example |
|------|-------------|------|------|
| `power` | `on`, `off` | Turn the Bluetooth adapter on or off | `python3 ukui-bluetooth-cli.py power on` |
| `tray` | `on`, `off` | Show or hide the taskbar Bluetooth icon | `python3 ukui-bluetooth-cli.py tray on` |
| `discoverable` | `on`, `off` | Make this machine discoverable | `python3 ukui-bluetooth-cli.py discoverable on` |
| `active-connection` | `on`, `off` | Auto-discover Bluetooth audio devices | `python3 ukui-bluetooth-cli.py active-connection on` |
| `audio-combine` | `on`, `off` | Enable or disable Bluetooth audio combine devices | `python3 ukui-bluetooth-cli.py audio-combine on` |
| `list` | none | List scanned devices (index, address, name) | `python3 ukui-bluetooth-cli.py list` |
| `connect` | `<address>` | Connect a device (wait 4s, then check status) | `python3 ukui-bluetooth-cli.py connect AA:BB:CC:DD:EE:FF` |
| `info` | `<address>` | Show device details as JSON | `python3 ukui-bluetooth-cli.py info AA:BB:CC:DD:EE:FF` |
| `scan` | `on`, `off` | Legacy client scan; `on` waits 10 seconds | `python3 ukui-bluetooth-cli.py scan on` |

> **Note**: `scan` uses `registerClient`. For common cases, `active-connection` and `discoverable` are enough.

## Examples

### 1. Turn Bluetooth on

```bash
$ python3 ukui-bluetooth-cli.py power on
Bluetooth is on
```
