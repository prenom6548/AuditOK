# Examples Index

仓库根目录下的 `examples/` 只保留两个用户入口：`client/` 和 `server/`。

## 目录说明

- `client/`: 用户直接运行的客户端示例，以及 `client/developer/` 下沉的开发调试示例
- `server/`: 用户直接运行的服务端示例，以及 `server/developer/` 下沉的开发调试示例

## 推荐入口

首次阅读建议按下面顺序：

1. `server/filesystem_server_ipc_example.cpp`
2. `client/client_ipc_demo.cpp`
3. `server/filesystem_server_http_example.cpp`
4. `client/client_http_demo.cpp`
5. `server/developer/transport/transport_server_client_example.cpp`
6. `server/server_full_example.cpp`
7. `client/client_full_example.cpp`

## 快速上手

### IPC 流程

1. 启动 `build/bin/server_filesystem_ipc_example`
2. 另开终端运行 `build/bin/mcp_client_ipc_demo`

### HTTP 流程

1. 启动 `build/bin/server_filesystem_http_example`
2. 另开终端运行 `build/bin/mcp_client_http_demo`

### 传输层联调

1. 启动 `build/bin/transport_server_example`
2. 另开终端运行 `build/bin/transport_server_client_example`

### Full Example 回归联调

这组示例用于覆盖更完整的 MCP 能力链路，推荐在修改协议处理、
transport、roots、通知或取消逻辑后执行。

#### IPC 模式

客户端直接拉起服务端：

```bash
./build/bin/mcp_client_full_example --ipc "./build/bin/mcp_server_full_example --ipc"
```

#### HTTP/SSE 模式

1. 启动 `build/bin/mcp_server_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp`
2. 另开终端运行 `build/bin/mcp_client_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp`

建议重点关注：

- `initialize`、`ping`、`tools/resources/prompts` 主流程
- 未知 tool/resource/prompt 的负向路径
- `elicitation/create`、`sampling/createMessage` 的 server->client 方向约束
  （client 侧应注册 handler，不主动发起请求）
- `notifications/progress`
- `$/cancelRequest`
- `notifications/roots/list_changed`
- `roots/list` 初始返回和 roots 变更后的二次刷新
- 动态 `tools/resources/prompts list_changed` 通知

持续联调脚本位置：

- `examples/server/developer/transport/run_transport_test.sh`

## 编译

```bash
cmake -S . -B build
cmake --build build -j4
```

## 常用可执行文件

- `build/bin/server_filesystem_ipc_example`
- `build/bin/mcp_client_ipc_demo`
- `build/bin/server_filesystem_http_example`
- `build/bin/mcp_client_http_demo`
- `build/bin/mcp_server_full_example`
- `build/bin/mcp_client_full_example`
- `build/bin/transport_server_example`
- `build/bin/transport_server_client_example`

## 说明

- 根目录构建会统一编译这些示例。
- 顶层只暴露 `client/` 和 `server/` 两个目录。
- 协议、传输、网络、日志等底层样例仍保留，但统一下沉到 `client/developer/` 或 `server/developer/`。

## 使用边界与最小验证

- 这些示例用于 API 用法演示与联调回归，不作为生产级容错实现模板。
- 对外集成时建议将错误处理、重试策略、超时治理与监控日志按业务标准补齐。

建议最小验证步骤：

1. 跑通一个 IPC 组合：`server_filesystem_ipc_example` + `mcp_client_ipc_demo`
2. 跑通一个 HTTP/SSE 组合：`server_filesystem_http_example` + `mcp_client_http_demo`
3. 跑一轮 full example（IPC 或 HTTP/SSE 任一）并检查关键链路：
   `initialize/ping`、`tools/resources/prompts`、`progress`、`cancel`、`roots/list_changed`
