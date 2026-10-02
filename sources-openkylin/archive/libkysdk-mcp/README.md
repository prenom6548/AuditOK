# MCP FileSystem Server HTTP Demo

这个目录包含用于测试MCP文件系统服务器HTTP功能的示例文件和目录。

## 编译和运行 Example

仓库根目录下的示例源码统一放在 `examples/`，默认通过顶层
`CMakeLists.txt` 的 `BUILD_EXAMPLES` 选项控制编译。

### 1. 编译示例

在仓库根目录执行：

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build -j4
```

编译完成后，示例程序会生成在 `build/bin/` 目录下。

### 2. 常用示例程序

- `build/bin/server_filesystem_ipc_example`
- `build/bin/mcp_client_ipc_demo`
- `build/bin/server_filesystem_http_example`
- `build/bin/mcp_client_http_demo`
- `build/bin/mcp_server_full_example`
- `build/bin/mcp_client_full_example`
- `build/bin/transport_server_example`
- `build/bin/transport_server_client_example`

### 3. 运行 IPC 示例

先启动服务端：

```bash
./build/bin/server_filesystem_ipc_example
```

再打开另一个终端运行客户端：

```bash
./build/bin/mcp_client_ipc_demo
```

### 4. 运行 HTTP 示例

先启动服务端：

```bash
./build/bin/server_filesystem_http_example
```

再打开另一个终端运行客户端：

```bash
./build/bin/mcp_client_http_demo
```

### 5. 运行传输层联调示例

先启动服务端：

```bash
./build/bin/transport_server_example
```

再打开另一个终端运行客户端：

```bash
./build/bin/transport_server_client_example
```

### 6. 运行 Full Example 回归示例

`mcp_server_full_example` 和 `mcp_client_full_example` 用于覆盖更完整的
MCP 请求、通知、roots、cancel 和动态变更链路，适合作为回归联调入口。

#### IPC 模式

客户端会自行拉起服务端进程：

```bash
./build/bin/mcp_client_full_example --ipc "./build/bin/mcp_server_full_example --ipc"
```

#### HTTP/SSE 模式

先启动服务端：

```bash
./build/bin/mcp_server_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
```

再打开另一个终端运行客户端：

```bash
./build/bin/mcp_client_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
```

这组 full example 当前已覆盖并验证：

- `initialize` / `initialized` / `ping`
- `tools` / `resources` / `prompts` 主流程和负向路径
- `resources/subscribe` / `resources/unsubscribe`
- `tasks/get` / `tasks/result` / `tasks/list` / `tasks/cancel`
- `completion/complete`
- `sampling/createMessage` / `elicitation/create` 方向约束（server->client）与
  client 侧 handler 注册
- `notifications/progress`
- `notifications/tasks/status`
- `notifications/elicitation/complete`（当请求携带 `elicitationId` 时）
- `$/cancelRequest`
- `notifications/roots/list_changed`
- `roots/list` 首次请求与变更后的二次刷新
- 动态 `tools/resources/prompts list_changed` 通知

可通过 `--help` 查看参数：

```bash
./build/bin/mcp_server_full_example --help
./build/bin/mcp_client_full_example --help
```

更多示例入口和说明见 `examples/README.md`。

## 连接断开恢复

SDK 支持按配置启用自动重连。默认 `auto_reconnect=false`，旧配置不启用
后台重连；开启后，transport 会在检测到 IPC 子进程退出、SSE 连接断开或
keepalive 超时时进入 `kReconnecting`，按退避策略重试。

### IPC 最小示例

```cpp
mcp::MCPClient client;

mcp_transport::TransportConfig config =
    mcp_transport::utils::createIpcConfig(
        "python3", {"-u", "server.py"});
config.auto_reconnect = true;
config.reconnect_interval = 1;
config.max_reconnect_attempts = 5;
config.max_reconnect_interval = 5;
config.ipc.respawn_on_disconnect = true;

client.setReconnectHandler([](const mcp::ReconnectEvent& event) {
    std::cout << "Reconnect "
              << (event.success ? "succeeded" : "failed")
              << ": " << event.reason << std::endl;
    return event.success;
});

client.connectWithConfig(config);
```

### SSE 最小示例

```cpp
mcp::MCPClient client;

mcp_transport::TransportConfig config =
    mcp_transport::utils::createSseConfig(
        "127.0.0.1", 18080, mcp_transport::TransportProtocol::kHttp);
config.sse.endpoint = "/mcp";
config.sse.keepalive_interval = 1;
config.auto_reconnect = true;
config.reconnect_interval = 1;
config.max_reconnect_attempts = 5;
config.max_reconnect_interval = 5;

client.setReconnectHandler([](const mcp::ReconnectEvent& event) {
    std::cout << "Reconnect "
              << (event.success ? "succeeded" : "failed")
              << ": " << event.reason << std::endl;
    return event.success;
});

client.connectWithConfig(config);
```

未注册自定义 handler 时，客户端默认会在重连成功后重发 `initialize`，并按
原订阅顺序恢复成功订阅过的 resource URI。更完整的触发条件和请求语义见
`docs/reconnect.md`。

## 单元测试

### 1. 配置与编译

在仓库根目录执行：

```bash
cmake -S . -B build-ut -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-ut -j"$(nproc)"
```

### 2. 查看测试列表与执行

建议先进入构建目录再执行 `ctest`：

```bash
cd ./build-ut
ctest -N
ctest --output-on-failure
```

也可以按名称过滤测试：

```bash
ctest -R "client_gtest|protocol_gtest|transport_test|logger_test|pipe_communication_test|netio_test" --output-on-failure
ctest -R "server_gtest|logger_gtest|netio_gtest|pipe_communication_gtest|transport_gtest|session_manager_gtest" --output-on-failure
```

## 稳定性说明

- IPC 超时生命周期竞态已完成根修复：超时线程改为可管理并在关闭流程中统一回收，不再使用 detached 后台线程访问对象状态。
- 服务端 SSE 监听当前仅支持 `TransportProtocol::kHttp`；在 server 模式配置 `kHttps` 会在启动阶段显式报错并返回失败。

### 3. 常见问题：`Total Tests: 0`

如果看到以下输出：

```text
Test project /path/to/libkysdk-mcp
Total Tests: 0
```

请按顺序检查：

1. 是否使用了 `-DBUILD_TESTING=ON` 重新执行过 `cmake -S . -B build-ut ...`
2. 是否在构建目录执行 `ctest`（推荐 `cd build-ut && ctest -N`）
3. 若不切目录，改用绝对路径：`ctest -N --test-dir "/absolute/path/to/build-ut"`

## 传输方式

- **HTTP (超文本传输协议)**: 通过HTTP协议进行通信
- **优势**: 标准化、跨平台、易于调试、适合网络部署
- **适用场景**: Web应用、远程访问、分布式系统

## 目录结构

- `test_files/`: 测试文件目录
  - `sample.txt`: 示例文本文件
  - `subdir/`: 子目录
- `docs/`: 文档目录
- `logs/`: 日志目录
- `README.md`: 本说明文件

## 测试方法

1. 使用 `list_directory` 工具列出目录内容
2. 使用 `read_file` 工具读取文件内容
3. 使用 `write_file` 工具创建新文件
4. 使用 `create_directory` 工具创建新目录
5. 使用 `file_info` 工具获取文件信息

## HTTP 通信特点

- 标准化协议，易于集成
- 支持跨网络访问
- 可以使用标准HTTP工具测试（curl, Postman等）
- 适合Web应用和远程客户端
