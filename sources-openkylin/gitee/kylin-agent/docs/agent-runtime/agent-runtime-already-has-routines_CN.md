# Hermes Agent 自 2026 年 3 月起就已经具备 “Routines” 能力

Anthropic 发布了 Claude Code Routines：定时任务、GitHub 事件触发、API
触发的 agent 运行，以及绑定提示词、仓库和连接器的自动化能力。这是一个有用
的功能，但 Hermes Agent 在 2026 年 3 月就已经具备同类能力。

## 三类触发方式对比

### 1. 定时任务 cron

Claude Code Routines 的示例是：每天凌晨 2 点拉取 Linear 中最重要的 bug，
尝试修复并打开草稿 PR。

Hermes Agent 可以直接使用：

```bash
hermes cron create "0 2 * * *" \
  "Pull the top bug from the issue tracker, attempt a fix, and open a draft PR." \
  --name "Nightly bug fix" \
  --deliver telegram
```

### 2. GitHub 事件 webhook

Claude Code Routines 的示例是：当 PR 修改 `/auth-provider` 模块时，在
`#auth-changes` 频道提醒。

Hermes Agent 可以直接使用：

```bash
hermes webhook subscribe auth-watch \
  --events "pull_request" \
  --prompt "PR #{pull_request.number}: {pull_request.title} by {pull_request.user.login}. Check if it touches the auth-provider module. If yes, summarize the changes." \
  --deliver slack
```

### 3. API 触发

Claude Code Routines 的示例是：读取告警 payload，找到负责服务，并把分诊摘要
发到 `#oncall`。

Hermes Agent 可以直接使用：

```bash
hermes webhook subscribe alert-triage \
  --prompt "Alert: {alert.name} — Severity: {alert.severity}. Find the owning service, investigate, and post a triage summary with proposed first steps." \
  --deliver slack
```

Claude 博文中提到的 backlog triage、文档漂移检查、部署验证、告警关联、
库迁移、自定义 PR review 等场景，在 Hermes Agent 中都已经有可运行方案。

## 差异概览

| 能力 | Claude Code Routines | Hermes Agent |
|---|---|---|
| 定时任务 | 基于 schedule | 任意 cron 表达式 + 自然语言间隔 |
| GitHub 触发 | PR、issue、push | 任意 GitHub webhook 事件 |
| API 触发 | POST 到唯一 endpoint | POST 到带 HMAC 鉴权的 webhook route |
| MCP 连接器 | 原生连接器 | 完整 MCP client 支持 |
| 脚本预处理 | 不支持 | 支持 agent 前运行 Python 脚本并注入上下文 |
| 多技能链 | 不支持 | 支持一次自动化加载多个 skill |
| 每日上限 | Pro 5 次、Enterprise 25 次 | 无固定每日上限 |
| 模型选择 | Claude only | Claude、GPT、Gemini、DeepSeek、Qwen、本地模型等 |
| 投递目标 | GitHub comments | Telegram、Discord、Slack、SMS、email、GitHub comments、webhook、本地文件 |
| 运行基础设施 | Anthropic 云端 | 用户自己的 VPS、家用服务器或笔记本 |
| 数据驻留 | Anthropic 云端 | 用户自己的机器 |
| 开源 | 否 | 是，MIT license |

## Hermes 额外能力

### 脚本注入

可以在 agent 运行前执行 Python 脚本。脚本 stdout 会成为上下文；脚本负责机械性
工作，例如抓取、diff、计算，agent 负责推理。

```bash
hermes cron create "every 1h" \
  "If CHANGE DETECTED, summarize what changed. If NO_CHANGE, respond with [SILENT]." \
  --script ~/.kylin-agent-runtime/scripts/watch-site.py \
  --name "Pricing monitor" \
  --deliver telegram
```

`[SILENT]` 模式意味着只有真正发生变化时才通知，避免无意义刷屏。

### 多技能工作流

可以把多个专业 skill 串起来，例如每天搜索 arXiv 并保存为 Obsidian 笔记：

```bash
hermes cron create "0 8 * * *" \
  "Search arXiv for papers on language model reasoning. Save the top 3 as Obsidian notes." \
  --skills "arxiv,obsidian" \
  --name "Paper digest"
```

### 多投递目标

同一个自动化可以投递到不同目标：

```bash
--deliver telegram
--deliver discord
--deliver slack
--deliver sms:+15551234567
--deliver telegram:-1001234567890:42
--deliver local
```

### 模型无关

夜间分诊可以用 Claude，部署验证可以用 GPT，成本敏感监控可以用 DeepSeek 或本地
模型。自动化系统相同，后端模型可按任务选择。

## 限制差异

Claude Code Routines 在 Pro 方案中每天 5 次，Enterprise 方案中每天 25 次。
Hermes Agent 没有这类每日运行次数上限，实际限制来自用户自己的 API 预算、
机器资源和所选择的模型。
