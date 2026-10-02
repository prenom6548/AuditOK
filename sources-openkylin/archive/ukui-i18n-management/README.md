# ukui-i18n-management

语言管理包（Debian），提供语言配置软件包列表及联网后的自动检测与桌面通知功能。

## 功能概述

- **配置文件**：`/etc/ukui-i18n-management/packages.conf` 定义需检测的 deb 包名列表
- **系统服务**：`ukui-i18n-management.service`（oneshot，开机联网后执行一次）
- **网络监听**：`NetworkManager dispatcher` 监听连通性变化，手动连网后再次触发检测
- **通知跳转**：通过 `org.freedesktop.Notifications` D-Bus 发送通知，监听 `ActionInvoked` 信号，点击消息体跳转 `ukui-control-center`；全部已安装则不通知

## 场景一：初始安装后检测

**前置条件**：系统初次安装完成，存在未完整安装的语言配置包。

**操作流程**：

| 场景 | 行为 |
|------|------|
| 登录后无网络 | 检测脚本发现网络不可用，**不弹通知**，静默退出 |
| 手动连接网络后 | NetworkManager 上报 `connectivity-change`（`CONNECTIVITY_STATE=full`），**自动触发检测并弹通知** |
| 开机即联网 | `network-online.target` 就绪后执行一次检测 |

> 注意：服务本身**不是常驻进程**，不持续监听网络；手动连网后由 NetworkManager dispatcher 执行 `systemctl restart` 重新触发检测（`start` 对已完成的 oneshot 服务无效）。

## 目录结构

```
.
├── debian/                                    # Debian 打包文件
├── etc/ukui-i18n-management/
│   └── packages.conf                          # 语言配置包列表
├── etc/NetworkManager/dispatcher.d/
│   └── 99-ukui-i18n-management                # 网络连通性变化监听
├── lib/systemd/system/
│   └── ukui-i18n-management.service           # systemd 服务单元
└── usr/lib/ukui-i18n-management/
    ├── check-packages.sh                      # 包检测脚本
    └── send-notification.py                   # 通知发送与 ActionInvoked 监听
```

## 构建与安装

```bash
# 构建 deb 包（跳过签名，无需 GPG 私钥）
debuild -us -uc -b
# 或
dpkg-buildpackage -us -uc -b

# 安装（在上级目录找到生成的 .deb）
sudo dpkg -i ../ukui-i18n-management_1.0.0-ok1_all.deb
```

## 配置说明

编辑 `/etc/ukui-i18n-management/packages.conf`，使用 `LANGUAGE` + `PACKAGE` 列表划分不同语言区块：

```
LANGUAGE: zh_CN
    - PACKAGE:
        - apt
        - dpkg
        - ukui-control-center
        - language-pack-zh-hans

LANGUAGE: en_US
    - PACKAGE:
        - apt
        - language-pack-en
    - CONTROL-CENTER: ukui-control-center
```

系统读取**当前桌面用户**的 `LANGUAGE` 环境变量（如 `zh_CN:zh`），自动选择对应区块，匹配顺序：

1. 按 `LANGUAGE` 列表逐项精确匹配（先 `zh_CN`，再 `zh`）
2. 按 `LANGUAGE` 列表逐项前缀匹配
3. 若用户未设置 `LANGUAGE`，回退读取 `LANG`
4. `LANGUAGE: default` 区块
5. 文件中首个 `LANGUAGE` 区块

可选的 `CONTROL-CENTER` 用于指定通知点击后跳转的控制面板，未配置时默认 `ukui-control-center`。

修改配置后重新触发检测：

```bash
sudo systemctl start ukui-i18n-management.service
```

## 日志查看

```bash
journalctl -u ukui-i18n-management.service
journalctl -t ukui-i18n-management
```

## 通知机制

遵循 UKUI4.0 桌面环境通知协议，通过 D-Bus 接口 `org.freedesktop.Notifications` 发送通知：

- **发送**：调用 `Notify` 方法，在 `actions` 中注册 `default` 动作（点击消息体触发）
- **hints 类型**：UKUI 要求 `a{sv}`（非标准 `a{ss}`），须使用 `gdbus` 发送
- **跳转**：`send-notification.py` 在同一 D-Bus 连接中监听 `ActionInvoked`，收到 `default` 时启动 `ukui-control-center`

## 依赖

- `dpkg` — 包状态查询
- `systemd` — 服务管理
- `python3-dbus`、`python3-gi` — 在同一 D-Bus 连接中发送通知并接收 `ActionInvoked`
- `iputils-ping` 或 `inetutils-ping` — 网络连通性检测
- `network-manager`（推荐）— 提供手动连网后的连通性变化监听
