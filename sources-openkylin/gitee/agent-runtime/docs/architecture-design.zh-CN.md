# Hermes Agent 架构设计

## 1. 项目定位

Hermes Agent 是一个可在 CLI、TUI、消息平台、API Server、ACP 编辑器集成、Cron 调度和批处理环境中运行的 AI Agent 框架。它围绕一个同步代理循环组织：模型负责规划和生成工具调用，Hermes 负责提示词构建、工具注册与调度、会话持久化、记忆、技能、上下文压缩、安全审批、平台适配和扩展插件。

核心设计目标：

- 多入口：同一 `AIAgent` 可被 CLI、Gateway、Cron、ACP、Batch Runner 和 Python 库复用。
- 多模型：通过 provider/runtime 解析支持 OpenAI 兼容接口、OpenAI Responses/Codex、Anthropic Messages 以及插件化模型提供商。
- 多工具：工具通过注册表自注册，并按 toolset 暴露给不同平台。
- 多平台：网关把 Telegram、Discord、Slack、WhatsApp、Signal、Email、Matrix、Mattermost、Feishu、DingTalk、WeCom、Weixin、QQBot、Home Assistant、Webhook、API Server 等平台统一成 `MessageEvent`。
- 可扩展：插件、MCP、技能、记忆提供商、上下文引擎和模型提供商都可独立扩展。

## 2. 顶层架构

```text
用户入口
  ├─ CLI: cli.py / kylin_agent_runtime_cli/main.py
  ├─ TUI: ui-tui/ + tui_gateway/
  ├─ Gateway: gateway/run.py + gateway/platforms/
  ├─ ACP: acp_adapter/
  ├─ Cron: cron/
  ├─ API Server: gateway/platforms/api_server.py / dashboard
  └─ Batch: batch_runner.py
        │
        ▼
核心代理层
  ├─ run_agent.py: AIAgent 外壳、配置、回调、入口方法
  ├─ agent/conversation_loop.py: run_conversation 主循环
  ├─ agent/prompt_builder.py: 系统提示词组装
  ├─ kylin_agent_runtime_cli/runtime_provider.py: provider/model/api_mode 解析
  ├─ model_tools.py: 工具 schema 汇总与调度
  ├─ toolsets.py: toolset 定义与平台默认工具
  └─ kylin_agent_runtime_state.py: SQLite 会话存储
        │
        ▼
能力层
  ├─ tools/: 原生工具与运行环境
  ├─ plugins/: 插件、记忆、上下文、模型、图像/视频生成提供商
  ├─ skills/ 和 optional-skills/: 技能文档
  ├─ providers/: legacy provider 兼容
  ├─ gateway/: 消息平台适配与投递
  └─ kylin_agent_runtime_cli/: 配置、设置向导、命令、主题、Web Dashboard
```

## 3. 关键模块职责

| 模块 | 主要职责 |
|---|---|
| `run_agent.py` | 定义 `AIAgent`，持有会话、模型、工具、回调、预算、压缩、记忆、fallback 等状态。 |
| `agent/conversation_loop.py` | 从 `run_agent.py` 抽出的主对话循环，驱动一次用户请求经过模型调用、工具调用、重试、fallback、压缩、持久化。 |
| `model_tools.py` | 导入工具、生成模型可见 schema、处理 tool call、执行插件 hook、桥接同步/异步工具。 |
| `tools/registry.py` | `ToolRegistry` 单例，工具自注册、可用性检查、schema 过滤、统一 dispatch。 |
| `toolsets.py` | 定义工具集合、平台预设和组合 toolset，例如 `hermes-cli`、`hermes-gateway`、`hermes-acp`。 |
| `cli.py` | 经典交互式 CLI，处理 prompt_toolkit 输入、Rich 展示、slash command、会话操作。 |
| `ui-tui/` + `tui_gateway/` | Ink/React 终端 UI 与 Python JSON-RPC 后端，提供完整 TUI。 |
| `gateway/run.py` | 消息网关主进程，负责平台事件、授权、slash command、运行中会话守卫、创建 `AIAgent`、发送回复。 |
| `gateway/platforms/` | 每个平台的适配器，把原始事件归一成 `MessageEvent`，并提供 `send_message()`。 |
| `kylin_agent_runtime_state.py` | CLI/Agent 会话数据库，SQLite + FTS5，用于保存消息、标题、系统提示词、搜索。 |
| `gateway/session.py` | Gateway 会话存储和 session key 构建，隔离平台、聊天类型和线程。 |
| `kylin_agent_runtime_cli/config.py` | 默认配置、环境变量元数据、配置加载和迁移。 |
| `kylin_agent_runtime_cli/commands.py` | Slash command 单一注册表，CLI、Gateway、Telegram、Slack、自动补全和帮助都从这里派生。 |
| `kylin_agent_runtime_cli/plugins.py` | 通用插件发现、加载、hook、工具注册、CLI 子命令注册。 |
| `agent/memory_manager.py` | 内置记忆与外部 memory provider 的协调。 |
| `agent/context_compressor.py` | 默认上下文压缩引擎。 |
| `agent/prompt_caching.py` | Anthropic/Claude 前缀缓存标记和缓存指标。 |

## 4. 依赖链

核心工具依赖链如下：

```text
tools/registry.py
      ↑
tools/*.py
      ↑
model_tools.py
      ↑
run_agent.py / cli.py / gateway/run.py / batch_runner.py / environments
```

这个链条决定了新增原生工具时必须做两件事：

1. 在 `tools/<tool>.py` 顶层调用 `registry.register(...)`。
2. 在 `toolsets.py` 中把工具名加入对应 toolset，否则工具会注册但不会暴露给模型。

## 5. 数据流

### CLI 数据流

```text
用户输入
  → HermesCLI.chat()
  → AIAgent.run_conversation()
  → prompt_builder 构建系统提示词
  → runtime_provider 解析 provider/model/api_mode
  → model_tools.get_tool_definitions() 生成工具 schema
  → 模型 API 调用
  → 如有 tool_calls: model_tools.handle_function_call()
  → 工具结果追加到消息历史后继续循环
  → 最终文本返回 CLI
  → SessionDB 保存会话
```

### Gateway 数据流

```text
平台原始事件
  → gateway/platforms/<platform>.py 适配器
  → MessageEvent
  → GatewayRunner._handle_message()
  → 鉴权、session key、slash command、运行中会话守卫
  → AIAgent.run_conversation()
  → 平台 adapter.send_message()
  → gateway/session.py 保存历史
```

### 工具调用数据流

```text
模型返回 tool_call
  → agent/conversation_loop.py 解析
  → agent-level 工具优先拦截(todo/memory/delegate_task 等)
  → model_tools.handle_function_call()
  → 插件 pre_tool_call hook
  → registry.dispatch()
  → 工具 handler 返回 JSON 字符串
  → 插件 post_tool_call hook
  → 工具结果消息写回上下文
```

## 6. 扩展设计

Hermes 的扩展点分为几类：

- 原生工具：`tools/` + `toolsets.py`，适合需要进入核心发行版的工具。
- 通用插件：`plugins/<name>/` 或用户 `~/.kylin-agent-runtime/plugins/<name>/`，通过 `ctx.register_tool()`、hook、CLI 子命令扩展。
- MCP 工具：由 `tools/mcp_tool.py` 动态发现外部 MCP server 工具。
- 技能：`skills/` 和 `optional-skills/` 提供可按需加载的知识/流程文档。
- Memory provider：`plugins/memory/<name>/` 或外部插件实现 `MemoryProvider`。
- Model provider：`plugins/model-providers/<name>/` 注册 `ProviderProfile`。
- Context engine：`plugins/context_engine/` 替换或增强上下文管理。
- Gateway platform：`gateway/platforms/<name>.py` 实现平台适配器。

## 7. 设计约束

- 配置路径必须 profile-aware：持久状态使用 `get_hermes_home()`，展示路径使用 `display_hermes_home()`。
- `.env` 只放密钥、token、密码；非秘密设置放 `config.yaml`。
- 依赖必须严格固定版本，新增依赖需更新锁文件。
- 插件不应硬编码进核心文件；需要能力时扩展通用插件 API。
- Dashboard 的主聊天体验必须嵌入真实 `kylin-agent-runtime --tui`，不要在 React 里重写 transcript/composer。
