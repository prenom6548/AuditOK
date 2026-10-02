# Hermes Agent 工作流程

## 1. CLI 交互流程

```text
启动 hermes
  → kylin_agent_runtime_cli/main.py 解析命令
  → cli.py 初始化 HermesCLI
  → 加载 config.yaml、.env、skin、session、工具设置
  → 用户输入普通消息或 slash command
  → slash command 由 COMMAND_REGISTRY 解析并分派
  → 普通消息进入 AIAgent.run_conversation()
  → 显示模型回复、工具进度、reasoning、审批提示
  → 保存会话
```

主要涉及文件：

- `kylin_agent_runtime_cli/main.py`
- `cli.py`
- `kylin_agent_runtime_cli/commands.py`
- `run_agent.py`
- `agent/conversation_loop.py`
- `kylin_agent_runtime_state.py`

## 2. TUI 工作流程

```text
kylin-agent-runtime --tui
  → Node/Ink 前端启动
  → 通过 stdio JSON-RPC 连接 tui_gateway Python 后端
  → TypeScript 负责渲染 transcript、composer、activity、prompts
  → Python 后端负责 session、slash command、AIAgent 和工具
  → 工具/消息/审批以 JSON-RPC event 推送回 Ink
```

Dashboard 的 `/chat` 不重写聊天 UI，而是通过 PTY 嵌入真实 `kylin-agent-runtime --tui`。

主要涉及文件：

- `ui-tui/src/app.tsx`
- `tui_gateway/server.py`
- `kylin_agent_runtime_cli/pty_bridge.py`
- `kylin_agent_runtime_cli/web_server.py`
- `web/src/pages/ChatPage.tsx`

## 3. Gateway 消息流程

```text
kylin-agent-runtime gateway start
  → gateway/run.py 启动 GatewayRunner
  → 加载平台 adapter
  → adapter.connect()
  → 平台消息归一成 MessageEvent
  → BaseAdapter 运行中会话守卫
  → GatewayRunner._handle_message()
  → 鉴权 / session key / slash command / 运行中 agent 控制
  → 创建 AIAgent 并运行
  → adapter.send_message() 回复
```

Gateway 支持 active session 双层守卫：

- adapter 层先识别活跃 session，必要时排队并设置 interrupt。
- runner 层处理 `/stop`、`/approve`、`/deny`、`/queue`、`/status` 等控制命令。

## 4. Slash Command 流程

```text
/command 输入
  → kylin_agent_runtime_cli/commands.py resolve_command()
  → 得到 canonical CommandDef
  → CLI: HermesCLI.process_command()
  → Gateway: GatewayRunner._handle_message() 分派到 _handle_*_command()
  → 输出文本、状态变更或继续进入 agent
```

命令注册的单一事实来源是 `COMMAND_REGISTRY`。新增 alias 只需改 `CommandDef.aliases`，帮助、自动补全、Telegram 菜单、Slack 路由会自动派生。

## 5. 工具调用流程

```text
模型选择工具
  → 返回 tool_calls
  → agent loop 解析 tool name 和 arguments
  → agent-level 工具优先处理
  → model_tools.handle_function_call()
  → 插件 pre hook
  → registry.dispatch()
  → 工具 handler 执行
  → 插件 post hook
  → 工具结果追加为 tool message
  → 下一轮模型调用
```

工具可以来自：

- `tools/` 原生工具。
- MCP server 动态工具。
- 通用插件注册的工具。
- Memory provider 注入的工具。

## 6. 技能加载流程

```text
用户显式调用 /<skill> 或 /skills
  → agent/skill_commands.py 或 kylin_agent_runtime_cli/skills_hub.py
  → 扫描 ~/.kylin-agent-runtime/skills/、skills/、optional-skills/
  → 把 SKILL.md 内容作为用户消息注入
  → agent 按技能说明继续执行任务
```

技能用于给模型提供过程化知识，不是 Python 插件。复杂技能可以包含 `scripts/`、`references/`、`templates/`。

## 7. Cron 调度流程

```text
Scheduler tick
  → cron/jobs.py 读取 jobs
  → 找到 due job
  → 创建新 AIAgent
  → 注入 job prompt 和附加技能
  → 执行任务
  → 通过 gateway delivery 投递结果
  → 更新 next_run / 状态
```

Cron 适合日报、备份、审计、定时监控、跨平台通知。

## 8. ACP 编辑器流程

```text
编辑器启动 ACP 客户端
  → hermes-acp 入口
  → acp_adapter/ 提供协议服务
  → 使用 hermes-acp toolset
  → AIAgent 执行代码相关任务
  → 文件 diff、工具活动、终端结果回传编辑器
```

ACP 使用面向代码编辑器的工具集，通常不暴露消息投递、TTS、clarify 等交互式平台工具。

## 9. 插件流程

```text
导入 model_tools.py 或显式 discover_plugins()
  → PluginManager 扫描 bundled、user、project、pip entry points
  → 读取 plugin.yaml / manifest
  → 调用 register(ctx)
  → 插件注册工具、hook、CLI 子命令
  → agent/tool runtime 触发 hook 或 tool dispatch
```

插件应扩展通用 surface，不应把插件特定逻辑写进 `run_agent.py`、`cli.py`、`gateway/run.py` 等核心文件。

## 10. Dashboard Chat 流程

```text
浏览器打开 kylin-agent-runtime dashboard /chat
  → React 页面建立 /api/pty WebSocket
  → 后端 pty_bridge 启动 kylin-agent-runtime --tui
  → xterm.js 渲染真实 TUI
  → resize/input/output 都以 PTY 字节帧传输
```

React 可以添加 sidebar、inspector、status panel，但不能重写主聊天 transcript/composer。

