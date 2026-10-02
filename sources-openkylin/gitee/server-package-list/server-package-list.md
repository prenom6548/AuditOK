# openKylin 高性能版本核心软件包列表

本文档用于展示 openKylin 高性能版本面向服务器场景提供的核心能力。

实际构建输入以 [package-lists/](./package-lists/) 下的列表组合为准。本文档按能力模块展示核心软件包，不展开 `base.list` 中的最小系统基础包。

## 启动与系统初始化

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| initramfs-tools | 提供 initramfs 生成和维护能力，支撑早期系统启动。 | [仓库链接](https://gitee.com/openkylin/initramfs-tools) |
| grub-efi-amd64 | 提供 amd64 UEFI 启动支持。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| grub-efi-amd64-bin | 提供 amd64 UEFI GRUB 二进制组件。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| grub-efi-riscv64 | 提供 RISC-V UEFI 启动支持。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| grub-efi-riscv64-bin | 提供 RISC-V UEFI GRUB 二进制组件。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| grub2-common | 提供 GRUB 公共组件。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| grub-pc-bin | 提供 amd64 Legacy BIOS 启动所需的 GRUB 组件。 | [仓库链接](https://gitee.com/openkylin/grub2) |
| efibootmgr | 提供 UEFI 启动项管理能力。 | [仓库链接](https://gitee.com/openkylin/efibootmgr) |
| casper | 提供 Live ISO 启动支持。 | [仓库链接](https://gitee.com/openkylin/casper) |

## 网络配置与远程管理

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| netplan.io | 提供服务器网络配置能力。 | [仓库链接](https://gitee.com/openkylin/netplan.io) |
| openssh-server | 提供 SSH 远程登录能力。 | [仓库链接](https://gitee.com/openkylin/openssh) |
| openssh-sftp-server | 提供基于 SSH 的 SFTP 文件传输能力。 | [仓库链接](https://gitee.com/openkylin/openssh) |
| modemmanager | 提供移动宽带设备管理能力。 | [仓库链接](https://gitee.com/openkylin/modemmanager) |
| usb-modeswitch | 支持 USB 网络设备的工作模式切换。 | [仓库链接](https://gitee.com/openkylin/usb-modeswitch) |
| wireless-regdb | 提供无线网络区域规则数据库。 | [仓库链接](https://gitee.com/openkylin/wireless-regdb) |
| wpasupplicant | 提供 WPA/WPA2 无线网络认证支持。 | [仓库链接](https://gitee.com/openkylin/wpa) |

## 日志、时间与生命周期维护

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| appstream | 提供软件元数据处理能力，支撑软件信息查询与管理。 | [仓库链接](https://gitee.com/openkylin/appstream) |
| chrony | 提供服务器时间同步能力。 | [仓库链接](https://gitee.com/openkylin/chrony) |
| cron | 提供定时任务能力。 | [仓库链接](https://gitee.com/openkylin/cron) |
| distro-info | 提供发行版版本信息查询能力。 | [仓库链接](https://gitee.com/openkylin/distro-info) |
| distro-info-data | 提供发行版版本信息数据。 | [仓库链接](https://gitee.com/openkylin/distro-info-data) |
| needrestart | 检查系统更新后需要重启的服务或进程。 | anything |
| rsyslog | 提供传统系统日志服务，便于日志落盘和转发。 | [仓库链接](https://gitee.com/openkylin/rsyslog) |

## 控制台与命令行交互体验

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| bash-completion | 提供 Bash 命令补全能力。 | [仓库链接](https://gitee.com/openkylin/bash-completion) |
| bsdextrautils | 提供若干常用系统辅助工具。 | [仓库链接](https://gitee.com/openkylin/util-linux) |
| command-not-found | 在命令不存在时提示可能的软件包。 | anything |
| console-setup | 提供控制台字体和键盘配置能力。 | [仓库链接](https://gitee.com/openkylin/console-setup) |
| console-setup-linux | 提供 Linux 控制台相关配置支持。 | [仓库链接](https://gitee.com/openkylin/console-setup) |
| ed | 提供基础行编辑器。 | [仓库链接](https://gitee.com/openkylin/ed) |
| ftp | 提供 FTP 客户端工具。 | anything |
| groff-base | 提供 man 手册渲染所需的基础工具。 | [仓库链接](https://gitee.com/openkylin/groff) |
| info | 提供 GNU info 文档查看工具。 | [仓库链接](https://gitee.com/openkylin/texinfo) |
| install-info | 提供 info 文档索引维护工具。 | [仓库链接](https://gitee.com/openkylin/texinfo) |
| inetutils-telnet | 提供 telnet 客户端实现。 | anything |
| kbd | 提供虚拟控制台键盘映射和字体工具。 | [仓库链接](https://gitee.com/openkylin/kbd) |
| keyboard-configuration | 提供键盘布局配置数据。 | [仓库链接](https://gitee.com/openkylin/console-setup) |
| man-db | 提供 man 手册数据库和查询工具。 | [仓库链接](https://gitee.com/openkylin/man-db) |
| manpages | 提供常用命令和接口的手册页。 | [仓库链接](https://gitee.com/openkylin/manpages) |
| nano | 提供简单易用的命令行文本编辑器。 | [仓库链接](https://gitee.com/openkylin/nano) |
| screen | 提供终端会话保持和复用能力。 | anything |
| telnet | 提供 telnet 客户端入口包。 | anything |
| tnftp | 提供 FTP 客户端实现。 | anything |
| tmux | 提供终端复用和会话管理能力。 | [仓库链接](https://gitee.com/openkylin/tmux) |
| wget | 提供命令行文件下载工具。 | [仓库链接](https://gitee.com/openkylin/wget) |
| whiptail | 提供脚本交互式文本界面组件。 | [仓库链接](https://gitee.com/openkylin/newt) |
| xkb-data | 提供键盘布局和 XKB 数据。 | [仓库链接](https://gitee.com/openkylin/xkeyboard-config) |

## 基础存储与文件系统

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| btrfs-progs | 提供 Btrfs 文件系统创建、检查和维护工具。 | [仓库链接](https://gitee.com/openkylin/btrfs-progs) |
| cryptsetup | 提供磁盘加密配置和管理能力。 | [仓库链接](https://gitee.com/openkylin/cryptsetup) |
| cryptsetup-bin | 提供 cryptsetup 命令行工具。 | [仓库链接](https://gitee.com/openkylin/cryptsetup) |
| dosfstools | 提供 FAT/VFAT 文件系统创建和检查工具。 | [仓库链接](https://gitee.com/openkylin/dosfstools) |
| eject | 提供弹出或卸载可移动介质工具。 | [仓库链接](https://gitee.com/openkylin/util-linux) |
| exfatprogs | 提供 exFAT 文件系统创建和检查工具。 | anything |
| fdisk | 提供传统磁盘分区管理工具。 | [仓库链接](https://gitee.com/openkylin/util-linux) |
| fuse3 | 提供用户态文件系统支持。 | [仓库链接](https://gitee.com/openkylin/fuse3) |
| gdisk | 提供 GPT 分区管理工具。 | [仓库链接](https://gitee.com/openkylin/gdisk) |
| hdparm | 提供磁盘参数查看和基础诊断能力。 | [仓库链接](https://gitee.com/openkylin/hdparm) |
| kpartx | 提供分区映射创建工具，常用于镜像和多路径设备处理。 | anything |
| lvm2 | 提供 LVM 逻辑卷管理能力。 | [仓库链接](https://gitee.com/openkylin/lvm2) |
| mdadm | 提供 Linux 软件 RAID 管理能力。 | anything |
| ntfs-3g | 提供 NTFS 文件系统读写支持。 | [仓库链接](https://gitee.com/openkylin/ntfs-3g) |
| os-prober | 提供磁盘中其他操作系统探测能力。 | [仓库链接](https://gitee.com/openkylin/os-prober) |
| parted | 提供磁盘分区管理能力。 | [仓库链接](https://gitee.com/openkylin/parted) |
| squashfs-tools | 提供 SquashFS 文件系统创建和解包工具。 | [仓库链接](https://gitee.com/openkylin/squashfs-tools) |
| uuid-runtime | 提供 UUID 生成和管理工具。 | [仓库链接](https://gitee.com/openkylin/util-linux) |
| xfsprogs | 提供 XFS 文件系统创建、检查和维护工具。 | [仓库链接](https://gitee.com/openkylin/xfsprogs) |

## 运维与诊断

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| bind9-dnsutils | 提供 DNS 解析诊断工具，例如 `dig`、`nslookup`。 | [仓库链接](https://gitee.com/openkylin/bind9) |
| bc | 提供命令行计算工具。 | [仓库链接](https://gitee.com/openkylin/bc) |
| binutils | 提供二进制文件分析和处理工具。 | [仓库链接](https://gitee.com/openkylin/binutils) |
| bolt | 提供 Thunderbolt 设备管理能力。 | anything |
| bpfcc-tools | 提供基于 BPF 的系统观测和诊断工具。 | [仓库链接](https://gitee.com/openkylin/bpfcc) |
| bpftool | 提供 BPF 程序和 map 的查看与管理工具。 | [仓库链接](https://gitee.com/openkylin/linux) |
| dmidecode | 提供 DMI/SMBIOS 硬件信息查看能力。 | [仓库链接](https://gitee.com/openkylin/dmidecode) |
| ethtool | 提供网卡参数查看和链路诊断能力。 | [仓库链接](https://gitee.com/openkylin/ethtool) |
| gawk | 提供 awk 文本处理工具。 | [仓库链接](https://gitee.com/openkylin/gawk) |
| gettext-base | 提供基础国际化文本处理工具。 | [仓库链接](https://gitee.com/openkylin/gettext) |
| git | 提供源码获取和版本管理工具。 | [仓库链接](https://gitee.com/openkylin/git) |
| htop | 提供交互式进程和资源查看能力。 | anything |
| hwdata | 提供硬件设备识别数据。 | [仓库链接](https://gitee.com/openkylin/hwdata) |
| ieee-data | 提供 IEEE OUI 等网络设备识别数据。 | [仓库链接](https://gitee.com/openkylin/ieee-data) |
| iso-codes | 提供国家、语言等标准代码数据。 | [仓库链接](https://gitee.com/openkylin/iso-codes) |
| iputils-tracepath | 提供网络路径和 MTU 诊断能力。 | [仓库链接](https://gitee.com/openkylin/iputils) |
| jq | 提供 JSON 命令行处理工具。 | [仓库链接](https://gitee.com/openkylin/jq) |
| lshw | 提供硬件信息汇总查看能力。 | [仓库链接](https://gitee.com/openkylin/lshw) |
| lsof | 提供进程打开文件、端口和设备的排查能力。 | [仓库链接](https://gitee.com/openkylin/lsof) |
| mtr-tiny | 提供网络路径和丢包诊断能力。 | anything |
| mtd-utils | 提供 MTD/Flash 设备管理工具。 | anything |
| numactl | 提供 NUMA 拓扑查看和控制能力。 | [仓库链接](https://gitee.com/openkylin/numactl) |
| patch | 提供补丁应用工具。 | [仓库链接](https://gitee.com/openkylin/patch) |
| pciutils | 提供 PCI 设备查看能力，例如 `lspci`。 | [仓库链接](https://gitee.com/openkylin/pciutils) |
| pnp.ids | 提供即插即用设备 ID 数据。 | [仓库链接](https://gitee.com/openkylin/hwdata) |
| rsync | 提供高效文件同步能力。 | [仓库链接](https://gitee.com/openkylin/rsync) |
| sos | 提供系统诊断信息收集工具。 | [仓库链接](https://gitee.com/openkylin/sos) |
| strace | 提供系统调用追踪能力。 | [仓库链接](https://gitee.com/openkylin/strace) |
| sysstat | 提供系统性能采集能力，例如 `sar`、`iostat`。 | [仓库链接](https://gitee.com/openkylin/sysstat) |
| tcpdump | 提供网络抓包分析能力。 | anything |
| trace-cmd | 提供内核 ftrace 跟踪工具。 | anything |
| usbutils | 提供 USB 设备查看能力，例如 `lsusb`。 | [仓库链接](https://gitee.com/openkylin/usbutils) |

## 安全与维护

| 软件包 | 说明 | 仓库链接 |
| --- | --- | --- |
| apparmor | 提供应用访问控制与基础安全加固能力。 | [仓库链接](https://gitee.com/openkylin/apparmor) |
| polkitd | 提供系统权限授权服务。 | [仓库链接](https://gitee.com/openkylin/policykit-1) |
| iptables | 提供传统 iptables 防火墙和兼容能力。 | [仓库链接](https://gitee.com/openkylin/iptables) |
| nftables | 提供现代 Linux 防火墙能力。 | [仓库链接](https://gitee.com/openkylin/nftables) |
| ufw | 提供简化的防火墙配置工具。 | anything |
