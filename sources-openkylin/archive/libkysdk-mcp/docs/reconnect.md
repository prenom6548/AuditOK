# 连接断开恢复

本文说明 `libkysdk-mcp` 的 transport 自动重连与客户端 session 恢复行为。

## 触发条件

自动重连默认关闭。调用方需要显式设置：

```cpp
config.auto_reconnect = true;
```

transport 在以下场景会把状态从 `kConnected` 切换到 `kReconnecting`：

- IPC 子进程退出，或 pipe EOF / pipe closed。
- SSE 长连接被服务端关闭。
- `sse.keepalive_interval > 0` 时，keepalive 连续超时。

IPC 场景只有同时设置 `ipc.respawn_on_disconnect=true` 时，重连循环才会重新
执行原 `ipc.command` / `ipc.args` 拉起子进程。该字段默认关闭。

## Backoff 公式

第 `attempt` 次重连的等待时间为：

```text
min(reconnect_interval * attempt + jitter, max_reconnect_interval)
```

其中：

- `attempt` 从 1 开始。
- `jitter` 为 `[0, reconnect_interval]` 范围内的随机秒数。
- `max_reconnect_attempts` 限制连续失败的最大次数。

当重试预算耗尽时，transport 状态保持 `kError`，不会继续自动重试。调用方
可以显式调用 `connect()` 或客户端层 `MCPClient::reconnect()` 重新开始。

## Session 恢复行为

`MCPClient` 在 transport 自动重连成功后会运行 reconnect handler：

- 未注册自定义 handler 时，默认 handler 会使用缓存的
  `InitializeParams` 重发 `initialize`。
- `initialize` 成功后，默认 handler 会按原顺序恢复已成功订阅的 resource
  URI。
- 任一步失败都会终止默认恢复，并把失败原因记录到 reconnect event。

调用方可以通过 `setReconnectHandler()` 替换默认行为：

```cpp
client.setReconnectHandler([](const mcp::ReconnectEvent& event) {
    if (!event.success) {
        std::cerr << "Reconnect failed: " << event.reason << std::endl;
        return false;
    }
    std::cout << "Reconnect succeeded" << std::endl;
    return true;
});
```

传入空 handler 会恢复默认 session 恢复行为。

## In-flight 请求语义

断线发生时，transport 会完成所有正在等待响应的请求，并返回非 200 响应或
抛出上层异常。错误信息包含 `connection lost`、`reconnected` 或
`reconnecting` 等原因。

SDK 不会在重连后自动重放 in-flight 请求。MCP 方法不都具备幂等性，自动重放
可能造成重复副作用。调用方应在 reconnect handler 报告成功后，自行决定是否
重新发起业务请求。

重连期间新请求会立即失败，不会排队等待重连完成。
