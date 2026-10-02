# Hermes Agent 关键技术文档

## 1. 本地代码阅读路线

### 第一层：全局入口

| 文件/目录 | 阅读目的 |
|---|---|
| `README.md` / `README.zh-CN.md` | 产品定位、安装、功能摘要。 |
| `pyproject.toml` | 包入口、依赖、extras、测试配置、发行范围。 |
| `kylin_agent_runtime_cli/main.py` | `hermes` 命令入口和子命令装配。 |
| `cli.py` | 经典 CLI 的输入、展示、slash command、会话操作。 |
| `run_agent.py` | `AIAgent` 外壳、构造参数、回调、工具和会话状态。 |
| `agent/conversation_loop.py` | 主对话循环实现，当前最关键运行路径。 |

### 第二层：Agent 核心

| 文件/目录 | 阅读目的 |
|---|---|
| `agent/prompt_builder.py` | 系统提示词如何组装。 |
| `agent/prompt_caching.py` | Anthropic/Claude prompt cache 如何标记。 |
| `agent/context_compressor.py` | 历史压缩算法和保留策略。 |
| `agent/context_engine.py` | 可插拔上下文引擎抽象。 |
| `agent/auxiliary_client.py` | 辅助模型调用解析和执行。 |
| `agent/model_metadata.py` | context length、token 粗估、模型元数据。 |
| `agent/error_classifier.py` | API 错误分类和 fallback 原因。 |
| `agent/iteration_budget.py` | 工具循环预算。 |
| `agent/memory_manager.py` | 内置记忆与 memory provider 协调。 |
| `agent/memory_provider.py` | 外部记忆插件 ABC。 |

### 第三层：工具系统

| 文件/目录 | 阅读目的 |
|---|---|
| `tools/registry.py` | 工具注册、发现、schema、dispatch 的核心。 |
| `model_tools.py` | 工具定义过滤、参数 coercion、函数调用入口。 |
| `toolsets.py` | 工具集和平台工具暴露面。 |
| `tools/file_tools.py` | 文件读写搜索 patch。 |
| `tools/terminal_tool.py` | 终端执行和环境桥接。 |
| `tools/process_registry.py` | 后台进程管理。 |
| `tools/browser_tool.py` | 浏览器自动化工具。 |
| `tools/mcp_tool.py` | MCP server 动态工具发现和调用。 |
| `tools/approval.py` | 危险命令检测和审批模式。 |
| `tools/environments/` | local、docker、ssh、modal、daytona、singularity 等终端后端。 |

### 第四层：CLI、TUI 和 Dashboard

| 文件/目录 | 阅读目的 |
|---|---|
| `kylin_agent_runtime_cli/commands.py` | slash command 注册表和派生 lookup。 |
| `kylin_agent_runtime_cli/config.py` | 默认配置、环境变量、迁移。 |
| `kylin_agent_runtime_cli/runtime_provider.py` | provider runtime 解析。 |
| `kylin_agent_runtime_cli/model_switch.py` | `/model` 共享逻辑。 |
| `kylin_agent_runtime_cli/skin_engine.py` | CLI skin/theme 数据驱动系统。 |
| `ui-tui/src/` | Ink TUI 前端。 |
| `tui_gateway/` | TUI Python JSON-RPC 后端。 |
| `kylin_agent_runtime_cli/pty_bridge.py` | Dashboard 嵌入 TUI 的 PTY bridge。 |
| `kylin_agent_runtime_cli/web_server.py` | Dashboard API 和 WebSocket。 |
| `web/src/pages/ChatPage.tsx` | Dashboard `/chat` xterm 页面。 |

### 第五层：Gateway

| 文件/目录 | 阅读目的 |
|---|---|
| `gateway/run.py` | GatewayRunner、消息分派、命令、agent 创建、cron ticker。 |
| `gateway/platforms/base.py` | 平台 adapter 基类和 active session guard。 |
| `gateway/platforms/` | 各消息平台 adapter。 |
| `gateway/session.py` | Gateway session key 和持久化。 |
| `gateway/delivery.py` | 跨平台投递。 |
| `gateway/pairing.py` | DM pairing 授权。 |
| `gateway/hooks.py` | Gateway hook 发现和触发。 |
| `gateway/status.py` | token lock 和进程状态。 |

### 第六层：扩展系统

| 文件/目录 | 阅读目的 |
|---|---|
| `kylin_agent_runtime_cli/plugins.py` | 通用插件系统。 |
| `plugins/memory/` | 内置 memory provider 插件示例。 |
| `plugins/model-providers/` | 模型 provider 插件。 |
| `plugins/context_engine/` | 上下文引擎插件。 |
| `plugins/image_gen/` | 图像生成 provider。 |
| `plugins/video_gen/` | 视频生成 provider。 |
| `skills/` | 内置技能。 |
| `optional-skills/` | 可选官方技能。 |
| `tools/skills_hub.py` | optional skills 安装源适配。 |

### 第七层：自动化、研究和集成

| 文件/目录 | 阅读目的 |
|---|---|
| `cron/jobs.py` / `cron/scheduler.py` | 定时任务定义和调度。 |
| `batch_runner.py` | 批处理与轨迹生成。 |
| `agent/trajectory.py` | 轨迹保存格式辅助。 |
| `trajectory_compressor.py` | 轨迹压缩。 |
| `acp_adapter/` | ACP 编辑器协议集成。 |
| `providers/` | legacy provider 兼容层。 |

## 2. 已有英文开发者文档

建议继续阅读 `website/docs/developer-guide/`：

- `architecture.md`：架构总览。
- `agent-loop.md`：代理循环内部机制。
- `prompt-assembly.md`：提示词组装。
- `provider-runtime.md`：provider runtime 解析。
- `tools-runtime.md`：工具注册、toolset 和 dispatch。
- `session-storage.md`：SQLite/FTS5 会话存储。
- `gateway-internals.md`：消息网关内部机制。
- `context-compression-and-caching.md`：上下文压缩和 prompt cache。
- `adding-tools.md`：新增工具。
- `adding-platform-adapters.md`：新增消息平台。
- `model-provider-plugin.md`：模型 provider 插件。
- `memory-provider-plugin.md`：memory provider 插件。
- `context-engine-plugin.md`：上下文引擎插件。
- `image-gen-provider-plugin.md` / `video-gen-provider-plugin.md`：媒体生成插件。
- `creating-skills.md`：技能创作。
- `acp-internals.md`：ACP 内部机制。
- `cron-internals.md`：Cron 内部机制。
- `trajectory-format.md`：轨迹格式。

## 3. 用户功能文档

建议阅读 `website/docs/user-guide/`：

- `cli.md`：CLI 使用。
- `tui.md`：TUI 使用。
- `configuration.md`：配置文件。
- `configuring-models.md`：模型配置。
- `sessions.md`：会话管理。
- `security.md`：安全模型。
- `messaging/index.md`：消息平台总览。
- `features/tools.md`：工具与工具集。
- `features/skills.md`：技能系统。
- `features/memory.md`：记忆。
- `features/context-files.md`：上下文文件。
- `features/context-references.md`：上下文引用。
- `features/cron.md`：定时任务。
- `features/delegation.md`：子代理委派。
- `features/code-execution.md`：代码执行工具。
- `features/browser.md`：浏览器自动化。
- `features/mcp.md`：MCP 集成。
- `features/plugins.md`：插件。
- `features/api-server.md`：API Server。
- `features/acp.md`：编辑器集成。
- `features/web-dashboard.md`：Dashboard。

## 4. 参考文档

建议阅读 `website/docs/reference/`：

- `cli-commands.md`：CLI 命令参考。
- `slash-commands.md`：slash command 参考。
- `tools-reference.md`：工具参考。
- `toolsets-reference.md`：工具集参考。
- `environment-variables.md`：环境变量参考。
- `model-catalog.md`：模型目录。
- `mcp-config-reference.md`：MCP 配置参考。
- `skills-catalog.md`：内置技能目录。
- `optional-skills-catalog.md`：可选技能目录。
- `profile-commands.md`：profile 命令。
- `faq.md`：常见问题。

## 5. 修改代码时的优先阅读路径

新增工具：

1. `website/docs/developer-guide/adding-tools.md`
2. `tools/registry.py`
3. `model_tools.py`
4. `toolsets.py`
5. 相似 `tools/*.py`

修改 Agent 行为：

1. `website/docs/developer-guide/agent-loop.md`
2. `run_agent.py`
3. `agent/conversation_loop.py`
4. `agent/prompt_builder.py`
5. `agent/context_compressor.py`

修改 Gateway：

1. `website/docs/developer-guide/gateway-internals.md`
2. `gateway/run.py`
3. `gateway/platforms/base.py`
4. 对应 `gateway/platforms/<platform>.py`
5. `gateway/session.py`

新增 slash command：

1. `kylin_agent_runtime_cli/commands.py`
2. `cli.py::HermesCLI.process_command`
3. `gateway/run.py::_handle_message` 和对应 handler
4. 测试 CLI/Gateway help、autocomplete、Telegram/Slack 派生输出

新增插件：

1. `website/docs/user-guide/features/plugins.md`
2. `kylin_agent_runtime_cli/plugins.py`
3. `plugins/*` 相似实现
4. 避免修改核心文件，必要时扩展插件上下文 API

新增技能：

1. `website/docs/developer-guide/creating-skills.md`
2. `AGENTS.md` 中技能标准
3. 相似 `skills/<category>/<skill>/SKILL.md`
4. 如重依赖或小众能力，优先放 `optional-skills/`

