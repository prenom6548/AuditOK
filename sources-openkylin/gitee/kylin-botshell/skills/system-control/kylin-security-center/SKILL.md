---
name: kylin-security-center
description: "使用 ksc-defender 打开麒麟安全中心的安全体检、病毒防护、账户保护或防火墙界面。用户要求打开安全中心、进行安全体检或系统安全检测、打开病毒扫描或病毒查杀、查看账户保护或账号安全、打开防火墙时使用。"
version: 0.0.1
author: ""
tags:
  - 系统工具
  - 安全中心
---
# 安全中心（ksc-defender）

调用麒麟系统自带 `ksc-defender` 命令行工具打开安全中心对应功能界面。

## 触发条件

用户表达以下任一意图时触发：

- "打开安全中心"、"进入安全中心"
- "安全体检"、"帮我进行安全体检"、"帮我打开安全体检"
- "病毒防护"、"病毒扫描"、"查杀病毒"、"帮我进行安全扫描"、"帮我进行病毒查杀"
- "账户保护"、"账号安全"、"打开账户保护设置"
- "防火墙"、"打开防火墙设置"

## 命令映射

`ksc-defender` 是 GUI 长驻进程，单纯加 `&` **不够**——shell 退出前仍会等待该后台任务，导致工具调用阻塞。

必须使用 `setsid` 脱离终端，并重定向标准输入/输出，再后台运行：

```bash
setsid ksc-defender <参数> </dev/null >/dev/null 2>&1 &
```

| 功能 | CLI 命令 |
|------|----------|
| 安全体检 | `setsid ksc-defender --baseline-ctrl </dev/null >/dev/null 2>&1 &` |
| 病毒防护 | `setsid ksc-defender --virus-scan </dev/null >/dev/null 2>&1 &` |
| 账户保护 | `setsid ksc-defender --account-sec </dev/null >/dev/null 2>&1 &` |
| 防火墙 | `setsid ksc-defender --firewall </dev/null >/dev/null 2>&1 &` |

## 执行流程

1. 从用户对话中识别要打开的功能，映射到上表对应命令
2. 若用户仅说"打开安全中心"而未指定功能，默认执行安全体检命令
3. **必须**使用上表完整命令（含 `setsid`、重定向和 `&`），不可省略
4. 命令发出后立即告知用户对应界面已启动，无需等待进程退出

## 示例

| 用户意图 | 命令 |
|----------|------|
| 帮我做个安全体检 | `setsid ksc-defender --baseline-ctrl </dev/null >/dev/null 2>&1 &` |
| 打开病毒防护 | `setsid ksc-defender --virus-scan </dev/null >/dev/null 2>&1 &` |
| 查看账户保护 | `setsid ksc-defender --account-sec </dev/null >/dev/null 2>&1 &` |
| 打开防火墙 | `setsid ksc-defender --firewall </dev/null >/dev/null 2>&1 &` |
