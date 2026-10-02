# Hermes Agent 详细工作流程

## 1. 一次普通用户消息的完整路径

1. 入口层收到用户文本。
   - CLI：`HermesCLI.chat()`。
   - Gateway：`GatewayRunner._handle_message()`。
   - TUI：`tui_gateway` 收到 `prompt.submit`。
2. 入口层确定 session ID、平台、工具集、模型配置和回调。
3. 创建或复用 `AIAgent`。
4. `AIAgent.run_conversation()` 转入 `agent/conversation_loop.py::run_conversation()`。
5. 安装安全 stdio，避免 daemon/headless 环境的 broken pipe 崩溃。
6. 确保 SQLite session 存在，并把当前 session ID 绑定到日志上下文。
7. 设置当前主 provider/model，供辅助任务和工具读取。
8. 如果上一轮启用了 fallback，本轮先恢复主 runtime 重新尝试。
9. 清理用户输入中的非法 surrogate 字符。
10. 恢复或构建系统提示词。
11. 加载 conversation history，并追加本轮 user message。
12. 计算粗略 token 使用量，必要时触发 preflight 压缩。
13. 根据 provider/api_mode 转换消息格式。
14. 生成工具 schema。
15. 应用 prompt caching 标记。
16. 发起可中断模型 API 调用。
17. 如果模型返回文本，进入最终响应保存流程。
18. 如果模型返回 tool calls，进入工具执行流程。
19. 工具结果加入消息历史后继续模型调用。
20. 达到最终回答、预算耗尽、用户中断或不可恢复错误时退出。

## 2. 系统提示词恢复和构建细节

`_restore_or_build_system_prompt()` 使用三种状态判断 session 中的系统提示词：

- `missing`：没有 session row，通常是新会话。
- `null`：row 存在但 `system_prompt` 是 NULL，可能是旧会话或迁移残留。
- `empty`：row 存在但为空字符串，表示上一轮写入路径可能异常。
- `present`：存在可用 prompt，继续会话时直接复用。

处理逻辑：

1. 如果历史非空且 DB 中有 `present` prompt，直接复用，保证 prefix cache 命中。
2. 如果是 `null` 或 `empty`，记录 warning 并重建。
3. 新会话或恢复失败时调用 `_build_system_prompt()`。
4. 新 session 触发插件 `on_session_start`。
5. 把新构建的系统提示词写回 SQLite，供下一轮复用。

## 3. 模型调用细节

模型调用前会准备：

- `messages`：内部 OpenAI 格式，或转换为目标 API 格式。
- `tools`：经过 toolset 和 `check_fn` 过滤的 schema。
- `model`：当前 provider 解析后的实际模型名。
- `reasoning_config`：推理强度、是否显示 reasoning。
- `service_tier`：fast/priority 等服务层设置。
- `stream_callback`：需要流式输出或 TTS 时使用。

调用失败时：

1. `agent/error_classifier.py` 对异常分类。
2. rate limit 会记录 provider 限流状态。
3. 认证错误可触发 token refresh 或 credential pool rotation。
4. 上下文超限可触发压缩或探测更大上下文层级。
5. 服务端错误按 backoff 重试。
6. 符合条件则切换 fallback provider。

## 4. 工具 schema 生成细节

`model_tools.get_tool_definitions()` 的主要步骤：

1. 接收 `enabled_toolsets` 和 `disabled_toolsets`。
2. 通过 `toolsets.resolve_toolset()` 展开组合工具集。
3. 处理旧 toolset 名兼容映射。
4. 调用 `registry.get_definitions()`。
5. 对每个工具执行 `check_fn`，不可用则不暴露。
6. 动态修补部分 schema，例如 `execute_code`、`browser_navigate` 只引用实际可用工具。
7. 返回模型 API 可用的 function/tool schema。

## 5. 工具执行细节

工具调用的执行顺序：

1. 从模型响应中取出 tool name 和 JSON arguments。
2. 修复或解析参数，必要时根据 schema 做类型 coercion。
3. 判断是否为 agent-level 工具：
   - `todo`
   - `memory`
   - `delegate_task`

   `delegate_task` 是子代理分支入口的 canonical 名称；它按“委派任务”的调用意图命名，实际会创建隔离上下文的子 `AIAgent`。
4. 非 agent-level 工具进入 `handle_function_call()`。
5. 调用插件 `pre_tool_call`。
6. 终端类危险命令走审批。
7. `registry.dispatch()` 查找 `ToolEntry` 并执行 handler。
8. async handler 通过事件循环桥接到同步路径。
9. 捕获异常并包装为 JSON。
10. 调用插件 `post_tool_call`。
11. 写入 `{"role": "tool", ...}` 消息。

多个工具调用：

- 默认可并发执行。
- 交互式工具或会影响顺序的工具可强制串行。
- 结果按模型返回的 tool call 顺序写回。

## 6. Gateway 消息详细流程

1. 平台 adapter 连接远端服务。
2. 原始平台消息被转换成 `MessageEvent`。
3. `BaseAdapter` 检查 `_active_sessions`。
4. 如果 session 正在运行：
   - 普通消息进入 pending queue，并触发 interrupt。
   - `/approve`、`/deny`、`/stop` 等控制消息可 inline 分派。
5. `GatewayRunner._handle_message()` 接收事件。
6. 通过 `_session_key_for_source()` 生成 session key，格式类似：
   - `agent:main:telegram:private:123456789`
7. 执行鉴权：
   - 平台 allow-all。
   - 平台 allowlist。
   - DM pairing。
   - 全局 allow-all。
   - 默认拒绝。
8. 判断是否 slash command。
9. 如果是命令，使用 `resolve_command()` 解析 canonical name。
10. 分派到 `_handle_*_command()`。
11. 如果是普通消息，加载历史并创建 `AIAgent`。
12. agent 执行期间登记 `_running_agents`。
13. 工具进度、审批、clarify、reasoning 通过回调发送到平台。
14. 最终结果经 `delivery.py` 或 adapter 返回。
15. 会话历史保存到 `SessionStore`。
16. 清理 `_running_agents`，处理 queued follow-up。

## 7. Slash Command 详细流程

新增或解析命令的关键路径：

1. `kylin_agent_runtime_cli/commands.py` 中 `COMMAND_REGISTRY` 定义 `CommandDef`。
2. `_build_command_lookup()` 把 canonical name 和 aliases 建成 lookup。
3. CLI、Gateway、Telegram BotCommands、Slack subcommands、autocomplete、help 都从注册表派生。
4. CLI 入口为 `HermesCLI.process_command()`。
5. Gateway 入口在 `GatewayRunner._handle_message()`，部分命令分派到 `_handle_*_command()`。
6. `cli_only`、`gateway_only` 和 `gateway_config_gate` 控制命令暴露面。

## 8. 会话保存和搜索细节

CLI/Agent 侧：

1. `AIAgent` 确保 session row 存在。
2. 每轮完成后把 messages、metadata、system_prompt、title 等写入 `SessionDB`。
3. 会话内容同时镜像到独立 `memory.db`，其 FTS5 索引仅供 `memory` 工具检索。
4. 压缩会话记录 parent/child lineage。

Gateway 侧：

1. session key 编码 platform、chat type、chat ID、thread/topic 信息。
2. `SessionStore` 按 session key 读写消息。
3. Telegram topic、Discord/Slack thread 等平台特有线程映射在 gateway 层处理。

## 9. Cron 详细流程

1. `cron/scheduler.py` 或 gateway cron ticker 周期唤醒。
2. 读取 jobs 数据。
3. 判断 cron 表达式或 interval 是否到期。
4. 为 job 创建独立 agent 上下文。
5. 加载 job 附加技能和 platform delivery target。
6. 执行 job prompt。
7. 把结果投递到 home channel 或指定平台。
8. 更新 `last_run`、`next_run`、失败次数和状态。

Cron 输出不直接混入普通 gateway 会话历史，避免破坏消息 alternation。

## 10. 文件修改和回滚详细流程

文件工具包括 `read_file`、`write_file`、`patch`、`search_files`。典型代码修改流程：

1. Agent 使用 `search_files` 和 `read_file` 定位上下文。
2. 在写入前 checkpoint 系统可创建工作区快照。
3. 使用 `patch` 或 `write_file` 修改文件。
4. 运行测试或静态检查。
5. 如出现问题，用户可用 `/rollback` 查看和恢复 checkpoint。

开发约束要求对现有用户改动保持保守：不应回滚未授权的用户修改。
