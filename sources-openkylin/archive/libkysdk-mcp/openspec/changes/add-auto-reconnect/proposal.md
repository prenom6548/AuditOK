## Why

PDF 思维导图「客户端与服务端-端到端全流程交互测试 / 异常流程」章节明确要求：网络中断或服务端崩溃后客户端能够自动恢复连接（用例 E1/E2）。但当前实现里 `auto_reconnect` 和 `reconnect_interval` 仅作为序列化字段存在，没有任何控制流读取它们；transport 层也不具备「对端断开后主动感知」的能力——IPC 子进程崩溃或 SSE 流断开后 `isConnected()` 仍返回 true，请求只能耗满超时才报错。为兑现 PDF 测试计划并消除「死配置」误导，需要把断线检测和自动重连真正落地。

## What Changes

- **新增**：transport 层 SSE / IPC 双模式的被动断线检测。SSE 通过 keepalive 心跳超时翻 `status_` 到 `kError`；IPC 通过监听子进程退出或写端 `SIGPIPE` 翻状态。
- **新增**：transport 层后台重连循环。`auto_reconnect=true` 时按 `reconnect_interval` 做指数退避，最多重试 `max_reconnect_attempts` 次（新增字段，默认 5）；成功后保持 `connection_id` 不变以便上层关联。
- **新增**：`MCPClient::reconnect()` 显式重连接口和 `MCPClient::setReconnectHandler(callback)` 状态回调，让用户能感知重连事件并自行决定 session 恢复策略。
- **新增**：客户端「session 自动恢复」组件——重连成功后自动重发 `initialize` 并恢复 `subscribeResource` 列表，已有的 in-flight 请求统一以 `mcp_exception(kInternalError, "reconnected")` 兜底失败。
- **修改**：`auto_reconnect` / `reconnect_interval` 字段从 no-op 变为生效配置。**BREAKING**：之前误把 `auto_reconnect = true` 当作禁用项的用户会感受到行为变化（实际几乎不存在，因为默认就是 false 且没有功能依赖）。
- **新增**：覆盖 PDF E1/E2 的端到端集成测试，IPC 模式杀子进程 + SSE 模式中断 stream 各一条。

## Capabilities

### New Capabilities

- `transport-reconnect`: transport 层断线检测、自动重连循环、状态机管理与可配置策略。
- `client-session-recovery`: 客户端在 transport 重连成功后的 session 状态恢复（重 initialize、恢复订阅）和 in-flight 请求兜底失败语义。

### Modified Capabilities

（无现有 spec；本变更同时创建上述新 capabilities，对应行为约束在 specs/ 下首次落地。）

## Impact

- **代码**：
  - `src/transport/transport.cpp`：新增断线检测回调、后台重连线程
  - `src/net/netio.cpp` / `.h`：SSE 客户端 keepalive 与断开通知
  - `src/ipc/pipe_communication.cpp` / `.h`：子进程 wait/SIGCHLD 处理、写端 SIGPIPE 屏蔽与断开通知
  - `src/client/client/client.cpp` / `include/client/client.h`：`reconnect()` API、`setReconnectHandler`、session 恢复逻辑
- **公开 API**：`MCPClient` 新增 `reconnect()` 与 `setReconnectHandler(handler)`；`TransportConfig.sse` 新增 `max_reconnect_attempts`。
- **配置兼容性**：`auto_reconnect` / `reconnect_interval` 语义变化（no-op → 生效）；建议在 release note 中突出。
- **测试**：新增 2 条端到端集成测试（IPC respawn + SSE stream 重建），新增若干 transport-reconnect / client-session-recovery 单元测试。
- **依赖**：无新增第三方依赖；仅使用现有 POSIX `waitpid` / `signal` 和已有的 httplib SSE 客户端能力。
- **风险**：后台重连线程引入新的并发路径；需重点测试断开 → 重连过程中并发请求的处理，以及 session 恢复期间的状态可见性。
