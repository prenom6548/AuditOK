## Context

`libkysdk-mcp` 当前的客户端 / transport 层只支持单次连接：`MCPClient::connect` 建立连接 → 工作期间假定连接持续可用 → `disconnect` 关闭。`auto_reconnect` 与 `reconnect_interval` 配置字段早就存在于 `include/common/transport.h:183-184` 和 `src/net/netio.h:52-53`，但全仓库没有任何 `if (auto_reconnect)` 控制流，仅做序列化用途（详见 `transport.cpp:411/475/1727`、`netio.cpp:1556`）。

实际运行中两个高发故障场景没有兜底：

1. **IPC 子进程异常退出**：管道写端关闭后，`isConnected()` 仍返回 true；后续每个请求都会落到死管道上，靠 `read_timeout_ms` 才能超时，5 次请求级重试（`client.cpp:726-748`）只是在死连接上空转。
2. **SSE 流被服务端关闭**：netio 层不会主动通知 transport，行为同上。

PDF 思维导图把这两条作为正式测试用例（E1 网络中断、E2 服务端崩溃后恢复），但 PDF 自身也标了「问题：在 http 传输方式下，服务端可以部署到另一台机器上，测试网络断连吗？」——说明编写时尚未拍板。本设计要给出可落地的最小方案，让这两条用例真正能写出来并通过。

## Goals / Non-Goals

**Goals:**

- transport 层具备**被动断线检测**：IPC 子进程退出 / SSE keepalive 失败时，`status_` 自动翻到 `kError`，让上层 `isConnected()` 立刻反映真实状态。
- 当 `config.*.auto_reconnect == true` 时，transport 层启动后台重连循环：按 `reconnect_interval` 做线性或指数退避，最多重试 `max_reconnect_attempts` 次（新增字段，默认 5）。
- 客户端层提供 `reconnect()` 显式接口和 `setReconnectHandler(callback)` 状态回调，让用户可以在自动重连不满足需要时手动接管。
- 客户端在 transport 重连成功后自动恢复 session：重发 `initialize`、恢复之前 `subscribeResource` 的列表；in-flight 请求统一以 `mcp_exception(kInternalError, "reconnected during request")` 失败。
- 覆盖 PDF E1/E2 的端到端集成测试可写、可稳定通过。

**Non-Goals:**

- 不实现请求级"重连后自动重发"——in-flight 请求一律失败上报，由用户重试。理由：MCP 协议没有幂等性保证，盲目重发可能造成重复执行。
- 不在 HTTPS 模式下做特殊处理（当前 SSE 客户端协议固定为 kHttp，HTTPS 尚未列入支持范围）。
- 不实现服务端到客户端的反向重连（服务端是被动 accept，不需要重连概念）。
- 不调整请求级重试策略（`max_attempts=5` 的现有逻辑）。

## Decisions

### D1 — 被动断线检测的实现机制

**选 A：transport 层增加 `onConnectionLost` 回调，由 IPC/SSE 子模块在各自的事件循环中触发。**

- **替代方案 B**：每次 `sendRequest` 失败时由 transport 自己推断断开。**否决理由**：依赖请求触发，"幽灵连接"期间状态不会更新；而且无法处理服务端主动关闭流（无请求在飞时）。
- IPC：在已有 `readThreadFunc` 中检测 `read()` 返回 0（EOF）或子进程已退出（`waitpid(WNOHANG)`），调用回调；写端额外用 `MSG_NOSIGNAL` / `SIG_IGN(SIGPIPE)` 避免进程崩溃。
- SSE：netio 客户端在 SSE 长连接断开（httplib 回调）或 keepalive 心跳超时（新增 30s 心跳）时调用回调。

### D2 — 重连循环的所有权

**选 transport 层独立后台线程**。

- **替代方案**：在客户端层做循环。**否决理由**：transport 已经管理自己的生命周期和状态机，把重连放在 transport 才能做到对上层"透明的状态恢复"；客户端再做一层会重复实现 backoff、超时等通用逻辑。
- 线程在 `connect()` 成功且 `auto_reconnect=true` 时启动；`disconnect()` 时停止；断线检测回调把"重连请求"投递到该线程的条件变量。
- backoff 策略：第 N 次重试间隔 = `min(reconnect_interval * N, max_reconnect_interval)`，加入 [0, reconnect_interval] 随机抖动避免 thundering herd。
- 终止条件：成功 / 达到 `max_reconnect_attempts` / 上层 `disconnect()` 调用。

### D3 — Session 恢复的归属

**放在客户端层，不放在 transport 层。**

- transport 只负责"通道连通"——它对 MCP 协议无知。
- 客户端实现 `setReconnectHandler`，默认 handler 会按顺序：`initialize` → 对每个曾订阅的 uri 调 `subscribeResource`。
- 用户可以替换 handler 做自定义恢复（例如不想自动重订阅）。
- 客户端内部维护一个 `subscribed_uris_` 集合，在 `subscribeResource` / `unsubscribeResource` 时同步更新。

### D4 — In-flight 请求的处理

**断开时统一以 `mcp_exception(kInternalError, "reconnected during request")` 失败。**

- 替代方案：缓存请求，重连成功后重发。**否决理由**：MCP 协议未约束幂等性，例如 `callTool` 可能是写操作；自动重发可能造成重复副作用。把这个决策交给用户更安全。
- 实现：`PipeCommunication` 和 `Transport` 各自维护 `pending_requests_` 映射；断线回调触发时遍历所有 promise 调 `set_exception`。

### D5 — 重连期间的请求行为

**选「立即拒绝」**：`status_ == kReconnecting` 时新请求直接返回 `kInternalError("reconnecting")`，不排队。

- 替代方案：排队等待重连成功。**否决理由**：增加客户端复杂度；用户可以基于 `setReconnectHandler` 自己决定要不要排队。

### D6 — 配置字段语义变化的兼容策略

**`auto_reconnect` / `reconnect_interval` 默认值保持不变（false / 5 秒）**，只是从 no-op 变为生效。

- 在 release note 和 header 注释里明确说明"行为变化"。
- 不引入新的 capability 开关 / 不读环境变量。
- 用户如果误配 `true` 后想要回滚，只需改回 false 即可。

### D7 — `MCPClient::reconnect()` 与 `connect()` 的语义边界

- `connect()` / `connectWithConfig()`：要求 transport 当前为 `kDisconnected`，否则报错。**不变**。
- `reconnect()`：内部相当于 `disconnect()` + `connect()` + 触发 session 恢复 handler。可以从任意状态调用，是幂等的。
- 自动重连成功后**会**触发 session 恢复 handler；用户手动 `reconnect()` 也会触发同一个 handler。两条路径走同一个 callback，保证行为一致。

## Risks / Trade-offs

- **后台线程并发风险** → 引入 `status_mutex_` 保护 transport 状态机；所有状态读写走 mutex；编写专门的并发测试（线程 A 调 `sendRequest`，线程 B 触发断线回调，线程 C 调 `disconnect`）。
- **IPC 子进程僵尸问题** → 在子进程退出回调中显式 `waitpid` 收尸；如果用户希望 respawn 子进程，需要在 `connectWithConfig` 时配置 `ipc.command`，重连循环用同一条命令重新 `fork+exec`。
- **SSE keepalive 兼容性** → 不是所有 MCP 服务端都支持心跳。设计上把 keepalive 默认关闭，必须用户在 config 里显式开启 `sse.keepalive_interval > 0` 才生效；否则只靠 httplib 的底层断开回调。
- **重连风暴** → backoff 抖动（D2）+ `max_reconnect_attempts` 上限。
- **session 恢复失败** → 默认 handler 失败时把 transport 翻回 `kError` 并触发用户回调（`setReconnectHandler` 的 result 参数），不再自动重试。
- **测试时序不稳** → IPC 端到端测试通过 `kill -9` 子进程触发断线；SSE 端到端测试通过 server `stop()` 后 `start()` 模拟恢复。两条测试都加上 `wait_for(condition, timeout)` 避免硬 sleep。
- **公开 API 变更**：`MCPClient` 新增方法，`TransportConfig.sse` 新增字段——属于增量变化，不会破坏既有调用方源码兼容。

## Migration Plan

1. **第一阶段（本变更）**：实现 D1-D7，所有现有测试必须继续通过，新增 PDF E1/E2 的端到端测试。
2. **第二阶段（后续可选）**：考虑加入"in-flight 请求重放"白名单（只对 idempotent 的 `ping/list*/get*` 自动重发），但需要在 MCP spec 层先就幂等性达成共识。
3. **回滚策略**：默认配置 `auto_reconnect = false` 不变，用户即使升级版本只要不动配置就保持旧行为；如果自动重连出问题，可以一键关闭。

## Open Questions

- IPC 模式重连时是否需要支持"重启子进程"？当前设计是 `connectWithConfig` 传入的 `command` 重新执行；如果用户的子进程依赖外部生命周期（例如 systemd 管理），重连可能会冲突。建议引入 `ipc.respawn_on_disconnect` 布尔开关，默认 false。
- 是否需要在 `MCPClient` 上暴露 `getReconnectStatus()` 查询当前重连进度（已重试 N/M 次）？暂未列入 goals；如果用户调研发现需要再补。
- SSE keepalive 心跳消息走什么协议方法？倾向于复用 `ping` 请求；如果 ping 超时则判定断线。需要与服务端约定一致行为。
