# kaiming 显示单个应用的信息

``kaiming info`` 命令可以进入显示单个应用的信息。

## 用法

查看 ``kaiming info`` 命令的帮助信息：

```bash
$ kaiming info 【选项】 应用ID # 默认查看本地已安装的应用信息，额外选项有 --remote=kaiming-repo 可以查看远程仓库应用信息
```

## 查看本地已安装应用信息示例

运行``kaiming info`` 命令可以显示本地已安装应用的信息。

```bash
$ kaiming info top.openkylin.Clock

Clock - Ukui Alarm

ID: top.openkylin.Clock
Ref: app/top.openkylin.Clock/x86_64/stable
Arch: x86_64
Branch: stable
Version: 4.0.0.0-ok3.1
InstalledSize: 15.9 MB
Runtime: top.openkylin.Platform/x86_64/2.0

Commit: 6ef92e59b19f41aa1068c26b6e366cc1497b366d4e580b57de05908a36176a23
Parent: 47a1919098a091935ea9146eafed73c245f534a6faddacd51890559851e57d80
Subject: Export top.openkylin.Clock
Date: 2024-05-13 07:56:07 +0000

```

## 查看远程仓库应用信息示例

运行``kaiming info --remote=kaiming-repo `` 命令可以显示本地已安装应用的信息。

```bash
$ kaiming info --remote=kaiming-repo top.openkylin.Clock

ID: top.openkylin.Clock
Ref: app/top.openkylin.Clock/x86_64/stable
Arch: x86_64
Branch: stable
InstalledSize: 1.3 GB

```

