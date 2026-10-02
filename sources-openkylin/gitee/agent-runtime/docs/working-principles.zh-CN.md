# Hermes Agent 工作原理

## 1. 代理循环

Hermes 的核心是 `AIAgent.run_conversation()`。在当前代码中，`run_agent.py` 的方法是薄转发，主体实现位于 `agent/conversation_loop.py`。一次用户请求会在同步循环中反复执行：

```text
构建上下文
  → 调用模型
  → 如果模型返回工具调用，执行工具并把结果写回消息
  → 如果模型返回最终文本，保存并结束
```

内部消息使用 OpenAI 兼容格式：

```python
{"role": "system", "content": "..."}
{"role": "user", "content": "..."}
{"role": "assistant", "content": "...", "tool_calls": [...]}
{"role": "tool", "tool_call_id": "...", "content": "..."}
```

支持 reasoning 的模型会把推理内容保存在 assistant 消息的 `reasoning` 字段，并通过回调展示。

## 2. Prompt 构建

系统提示词由 `agent/prompt_builder.py` 和 `AIAgent._build_system_prompt()` 协同生成。输入来源包括：

- 人格和身份：`SOUL.md`、内置 personality。
- 项目上下文：`AGENTS.md`、`.kylin-agent-runtime.md`、`CLAUDE.md`、`.cursorrules` 等。
- 用户与项目记忆：`MEMORY.md`、`USER.md`、外部 memory provider 返回的上下文。
- 技能：当前加载技能、slash skill、自动匹配技能。
- 工具使用规则：可用工具 schema、平台限制、安全说明。
- 会话临时层：预算警告、上下文压力提示、用户本轮消息。

为了支持 Claude/Anthropic 前缀缓存，继续会话时会优先从 SQLite 恢复上一轮保存的系统提示词，避免每轮重建导致缓存 miss。

## 3. Provider 和 API mode

Hermes 将模型运行时抽象成 provider、model、base_url、api_key、api_mode 的组合。主要 API mode：

| API mode | 用途 |
|---|---|
| `chat_completions` | OpenAI 兼容接口，例如 OpenRouter、自定义 endpoint、多数聚合服务。 |
| `codex_responses` | OpenAI Responses/Codex 路径。 |
| `anthropic_messages` | Anthropic 原生 Messages API。 |

解析顺序通常是：

1. 构造 `AIAgent` 时显式传入的 `api_mode`。
2. provider profile 的默认 API mode。
3. base URL 或 provider 名称启发式判断。
4. 默认回退到 OpenAI-compatible chat completions。

当主 provider 失败时，错误分类器会识别 rate limit、认证、服务端错误、上下文超限等情况，并按配置尝试 fallback provider 或 credential pool。

## 4. 工具系统原理

工具文件在模块顶层调用 `registry.register(...)` 自注册。`model_tools.py` 导入时触发 `discover_builtin_tools()`，扫描 `tools/*.py` 中的顶层注册调用并导入对应模块。

每个工具注册项包含：

- `name`：模型调用的函数名。
- `toolset`：所属工具集。
- `schema`：OpenAI function-calling schema。
- `handler`：真实执行函数。
- `check_fn`：可用性检查，例如环境变量、依赖、服务状态。
- `requires_env`：需要的环境变量，供 UI/配置使用。
- `is_async`：是否为异步 handler。

工具是否能被模型看到由三层决定：

1. 是否成功注册。
2. 是否属于当前平台启用的 toolset。
3. `check_fn` 是否通过。

工具 handler 必须返回 JSON 字符串。异常会被 `registry.dispatch()` 和 `handle_function_call()` 包装成 JSON 错误，保证模型收到结构化结果。

## 5. Toolset 原理

`toolsets.py` 是工具暴露面的控制层。它定义基础工具集、场景工具集和平台工具集：

- 基础工具集：`web`、`file`、`terminal`、`browser`、`memory`、`skills`、`vision` 等。
- 场景工具集：`debugging`、`safe`、`hermes-acp`、`hermes-api-server`。
- 平台工具集：`hermes-cli`、`hermes-telegram`、`hermes-discord`、`hermes-slack`、`hermes-feishu`、`hermes-gateway` 等。

`_HERMES_CORE_TOOLS` 是 CLI 和多数消息平台共享的默认核心工具列表。新增核心工具只注册不够，还必须加入这里或某个 toolset。

## 6. 会话和记忆

Hermes 有两类持久化：

- 会话持久化：保存消息、标题、系统提示词、压缩 lineage、搜索索引。
- 记忆持久化：保存长期用户偏好、项目知识、任务经验。

CLI/Agent 侧主要使用 `kylin_agent_runtime_state.py` 中的 `SessionDB`，基于 SQLite 和 FTS5。Gateway 侧使用 `gateway/session.py` 的 `SessionStore` 和平台 session key。

记忆由 `agent/memory_manager.py` 统一协调：

- 内置 `MEMORY.md` / `USER.md` 作为本地长期记忆。
- 外部 provider 插件可提供 Honcho、Mem0、Supermemory、OpenViking、RetainDB、ByteRover、Hindsight、Holographic 等后端。
- 每轮结束或上下文压缩前会 flush，避免中间信息丢失。

## 7. 上下文压缩

当消息历史接近模型上下文限制时，Hermes 会触发压缩：

- preflight 阶段通常在超过一定上下文比例时触发。
- Gateway 可在会话间更积极压缩。
- 压缩前先 flush memory。
- 保留最近 N 条消息和完整 tool call/tool result 配对。
- 中间历史由辅助模型摘要成 compact summary。
- 压缩后生成子 session lineage，保留可追踪性。

这让长会话可以继续运行，同时尽量减少破坏工具调用结构和消息 alternation 的风险。

## 8. 安全审批

终端工具和危险操作经过 `tools/approval.py` 的模式检测。危险命令包括递归删除、格式化磁盘、无条件 SQL 删除、覆盖系统配置、服务停止、`curl | sh`、进程 kill 等。

审批流程：

```text
terminal command
  → detect_dangerous_command()
  → 如命中危险模式，调用平台 approval_callback
  → CLI/Gateway 等待用户 approve/deny
  → approve 后执行，deny 后返回工具错误
  → 可按 session 或永久 allowlist 记录批准
```

YOLO mode 可跳过危险命令审批，但这是显式配置行为。

## 9. 插件和 Hook

通用插件通过 `kylin_agent_runtime_cli/plugins.py` 发现并加载。插件可以：

- 注册工具。
- 注册 CLI 子命令。
- 在 `pre_tool_call`、`post_tool_call`、`pre_llm_call`、`post_llm_call`、`on_session_start`、`on_session_end` 等 hook 中插入逻辑。
- 提供 dashboard 扩展、上下文引擎、记忆后端等。

模型 provider 插件有独立的 lazy discovery 系统，避免被通用插件重复导入。

## 10. 中断和并发

模型 API 调用通过可中断封装运行：主线程等待 API 线程、interrupt event 或 timeout。用户在 CLI 发送中断、Gateway 收到新消息或 `/stop` 时，可打断当前运行。

工具调用：

- 单个工具通常同步执行。
- 多个工具可用线程池并发执行。
- 交互式工具如 `clarify` 强制顺序执行。
- 并发工具结果按原 tool call 顺序写回，避免破坏模型上下文。

