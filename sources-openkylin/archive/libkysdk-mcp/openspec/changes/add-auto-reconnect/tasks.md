## 1. 配置与公开 API 扩展

- [x] 1.1 在 `include/common/transport.h` 的 `TransportConfig` 顶层新增 `auto_reconnect=false`、`reconnect_interval=5`、`max_reconnect_attempts=5`、`max_reconnect_interval=60`，并在 `TransportConfig::sse` 子结构里新增 `keepalive_interval=0`，注释明确单位与默认含义
- [x] 1.2 在 `include/common/transport.h` 的 `TransportConfig::ipc` 子结构里新增 `bool respawn_on_disconnect = false`，注释明确"必须配合顶层 `auto_reconnect=true` 才生效"
- [x] 1.3 更新 `src/transport/transport.cpp` 的 JSON 序列化/反序列化以覆盖新字段，保持向后兼容（缺失字段使用默认值，旧 `sse.*reconnect*` 字段可读取）
- [x] 1.4 修订 `include/common/transport.h` 上 `auto_reconnect` / `reconnect_interval` 的英文注释，去掉"Reserved"暗示，改为描述当前的生效语义
- [x] 1.5 在 `include/client/client.h` 的 `MCPClient` 公共接口新增 `bool reconnect()` 和 `void setReconnectHandler(ReconnectHandler handler)` 声明，并定义 `using ReconnectHandler = std::function<bool(const ReconnectEvent&)>` 与 `struct ReconnectEvent { bool success; std::string reason; TransportStatus status; }`

## 2. Transport 层断线检测

- [x] 2.1 在 `src/transport/transport.cpp` 的 `Transport::Impl` 类里新增 `std::function<void(const std::string&)> on_connection_lost_` 回调字段及对应注册方法（仅供内部 IPC/SSE 子模块调用）
- [x] 2.2 在 `src/ipc/pipe_communication.cpp` 的 `readThreadFunc` 中增加 EOF / `read()` 返回 0 的检测分支，调用 `on_connection_lost_`；写入路径全部改为 `MSG_NOSIGNAL` 或先 `signal(SIGPIPE, SIG_IGN)`
- [x] 2.3 在 `src/ipc/pipe_communication.cpp` 中对 IPC 客户端模式增加 `waitpid(child_pid_, &status, WNOHANG)` 周期性检查（每 500ms 一次），子进程退出时调用 `on_connection_lost_("child exited")`
- [x] 2.4 在 `src/net/netio.cpp` 的 SSE 客户端事件循环中，把 httplib 底层连接关闭事件桥接到 `on_connection_lost_`
- [x] 2.5 在 `src/net/netio.cpp` 实现可选的 SSE keepalive：当 `keepalive_interval > 0` 时启动心跳线程，连续 3 次心跳响应超时即调用 `on_connection_lost_("keepalive timeout")`
- [x] 2.6 在 transport 层捕获 `on_connection_lost_` 回调，加锁后把 `status_` 从 `kConnected` 翻为 `kError`，遍历 `pending_requests_` 调用 `set_exception` 或填充错误响应

## 3. Transport 层重连循环

- [x] 3.1 在 `Transport::Impl` 内新增 `std::thread reconnect_thread_` 与配套的 `std::condition_variable reconnect_cv_`、`std::atomic<bool> stop_reconnect_`、`std::mutex status_mutex_`
- [x] 3.2 实现 `void reconnectLoop()`：从 `status_mutex_` 取出当前 attempt 计数，按 `min(reconnect_interval * attempt, max_reconnect_interval)` + jitter 等待，调用内部 `connectInternal()` 重试，最多 `max_reconnect_attempts` 次
- [x] 3.3 在 `connect()` 成功路径末尾，如果 `auto_reconnect=true`，启动 `reconnect_thread_`；在 `stop()` / `disconnect()` 时置 `stop_reconnect_=true` + `notify_all` 并 `join`
- [x] 3.4 重连成功后把 `status_` 翻回 `kConnected`，重置 attempt 计数，触发上层注册的 `reconnect_handler_`（在新线程中）
- [x] 3.5 重连失败（达到 `max_reconnect_attempts`）时把 `status_` 保持 `kError`，触发上层 `reconnect_handler_` 报告失败原因
- [x] 3.6 重连期间收到 `sendRequestSync()` 调用时返回 `kInternalError("reconnecting")` 立即拒绝，不阻塞
- [x] 3.7 IPC 模式的 `connectInternal()` 在 `respawn_on_disconnect=true` 时使用同一 `ipc.command` / `ipc.args` 重新 `fork+exec` 子进程；`respawn_on_disconnect=false` 时直接失败

## 4. 客户端层 session 恢复

- [x] 4.1 在 `MCPClient::Impl` 内新增 `InitializeParams cached_init_params_`、`std::vector<std::string> subscribed_uris_`、`ReconnectHandler reconnect_handler_`、`std::mutex recovery_mutex_` 字段
- [x] 4.2 在 `initialize()` 调用成功时缓存原始 `InitializeParams` 到 `cached_init_params_`
- [x] 4.3 在 `subscribeResource()` / `unsubscribeResource()` 成功路径上同步更新 `subscribed_uris_`，并加测试覆盖"失败的 subscribe 不进集合"
- [x] 4.4 实现 `MCPClient::reconnect()`：加 `recovery_mutex_`，调 `disconnect()` + `connect()` + 触发 `reconnect_handler_`（默认 handler 或用户注册的）
- [x] 4.5 实现默认 `ReconnectHandler`：使用 `cached_init_params_` 调 `initialize`；成功后遍历 `subscribed_uris_` 调 `subscribeResource`；任一步失败立即返回并把失败原因填入 `ReconnectEvent.reason`
- [x] 4.6 实现 `MCPClient::setReconnectHandler()`，允许用户替换默认 handler；空 handler 视为恢复默认
- [x] 4.7 在 transport 自动重连成功时，由 transport → client 桥接调用，使得自动重连和手动 `reconnect()` 走同一个 handler 路径

## 5. 单元测试

- [x] 5.1 `tests/server/transport_gtest.cpp` 新增 transport 状态机并发测试：线程 A 持续 `sendRequest`，线程 B 触发 `on_connection_lost_`，线程 C 调 `disconnect`，断言无死锁、状态最终为 `kDisconnected`
- [x] 5.2 `tests/server/transport_gtest.cpp` 新增 backoff 时序测试：mock 始终拒绝连接，验证 5 次重试间隔接近 `[r, 2r, 3r, 4r, 5r]` ± jitter 且总耗时 ≤ `max_reconnect_attempts * max_reconnect_interval`
- [x] 5.3 `tests/server/pipe_communication_gtest.cpp` 新增子进程退出检测测试：fork 一个会立即 exit 的子进程，确认 `on_connection_lost_` 在 1 秒内被调用
- [x] 5.4 `tests/server/netio_gtest.cpp` 新增 SSE keepalive 超时测试：mock 服务端不响应心跳，验证 3 次超时后触发回调
- [x] 5.5 `tests/client/client_gtest.cpp` 新增 `SubscribedUrisTrackedCorrectly` 测试：覆盖 `subscribeResource` 成功/失败、`unsubscribeResource` 成功/失败 对 `subscribed_uris_` 的影响
- [x] 5.6 `tests/client/client_gtest.cpp` 新增 `ReconnectHandlerInvokedOnSuccess` 和 `ReconnectHandlerInvokedOnFailure` 测试
- [x] 5.7 `tests/client/client_gtest.cpp` 新增 `DefaultReconnectHandlerRestoresSession` 测试：mock server 配合验证重连后 initialize 与 subscribe 各被调用一次

## 6. 端到端集成测试（覆盖 PDF E1/E2）

- [x] 6.1 在 `tests/server/server_gtest.cpp` 新增 `IpcServerCrashTriggersAutoReconnect` 测试：起 mock IPC 服务端子进程 → `connect` + `initialize` → kill -9 子进程 → 等待 `auto_reconnect` 重启子进程 → 验证 `isConnected()` 重新为 true 且 `initialize` 已重做
- [x] 6.2 在 `tests/server/server_gtest.cpp` 新增 `SseServerCrashTriggersAutoReconnect` 测试：起 SSE 服务端 → connect → server.stop() → server.start() 在同端口 → 等待客户端自动重连 → 验证 session 完全恢复
- [x] 6.3 在 `tests/client/client_gtest.cpp` 新增 `ExplicitReconnectRecoversSession` 测试：手动调 `client.reconnect()` 后验证 initialize + subscribe 已自动重做
- [x] 6.4 在 `tests/server/server_gtest.cpp` 新增 `ReconnectAttemptBudgetExhaustedSignalsFailure` 测试：故意让 mock server 拒绝重连，验证 `max_reconnect_attempts` 用尽后 handler 收到失败回调

## 7. 文档与发布

- [x] 7.1 更新 `CHANGELOG.md`：在「新增」分类下记录 transport-reconnect / client-session-recovery 两块功能并标出原 `auto_reconnect` / `reconnect_interval` 由 no-op 变为生效
- [x] 7.2 更新 `README.md` 增加「连接断开恢复」小节，给出 IPC 与 SSE 两个最小示例代码（设 `auto_reconnect=true`、注册 `setReconnectHandler` 打印日志）
- [x] 7.3 在 `docs/` 下新增 `reconnect.md`（若已有相近主题则合并），覆盖触发条件、backoff 公式、session 恢复行为、in-flight 请求语义
- [x] 7.4 在 `debian/changelog` 新增 entry 描述此变更（遵循麒麟仓库的版本号递增规则）

## 8. 回归与构建

- [x] 8.1 在 `build/` 执行 `make -j$(nproc)`，确认零编译错误零警告（`-Wall -Wextra` 下）
- [x] 8.2 跑 `./client_gtest` 与 `./server_gtest` 全量，确保现有 31 + 263 个测试全部通过
- [x] 8.3 跑全部 `tests/` 下的 gtest 二进制（`runUnitTest.sh`），确认无新增失败
- [x] 8.4 用 `gcovr` 生成覆盖率报告，确认 `src/transport/transport.cpp` / `src/ipc/pipe_communication.cpp` / `src/client/client/client.cpp` 中新增逻辑覆盖率 ≥ 80%
