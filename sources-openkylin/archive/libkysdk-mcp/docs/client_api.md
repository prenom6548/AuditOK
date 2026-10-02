# libkysdk-mcp Client 接口文档

本文档面向 `libkysdk-mcp` 的客户端使用者，重点说明当前 `client`
模块公开接口、支持/不支持能力、完整示例入口与完整工程配置方式。

## 1. 公开头文件

客户端开发主要涉及以下公开头文件：

- `include/client/client.h`
  - 客户端门面类 `mcp::MCPClient`
- `include/common/protocol.h`
  - 协议对象、能力声明、Tool/Resource/Prompt 数据结构、回调类型
- `include/common/message.h`
  - JSON-RPC/MCP 消息、异常和错误码定义
- `include/common/transport.h`
  - 传输层类型、`TransportConfig` 配置结构和 `Transport` 抽象

安装版 SDK 的头文件安装目录（client 相关在其子目录）：

- `kylin-ai/mcp/`

### 1.1 JSON 公开依赖策略

- `nlohmann::json` 是当前 SDK 的公开类型依赖，属于设计决策。
- 安装产物会公开 `kylin-ai/mcp/common/json.hpp`，用于与协议对象保持一致的 JSON 表达。
- 下游应通过安装后的公开头路径引用 JSON 类型，不依赖仓库内部相对路径。

## 2. 客户端整体模型

### 2.1 类关系

```text
mcp::MCPClient
```

### 2.2 客户端职责

`MCPClient` 负责：

- 管理连接：`connect` / `connectWithConfig` / `disconnect`
- 发起请求：
  - 基础：`initialize`、`ping`
  - Tools：`tools/list`、`tools/call`
  - Resources：`resources/list`、`resources/read`、
    `resources/templates/list`、`resources/subscribe`、
    `resources/unsubscribe`
  - Prompts：`prompts/list`、`prompts/get`
  - 扩展：`logging/setLevel`、`tasks/*`、`completion/complete`
- 发送通知：`initialized`、`sendNotification`
- 处理 server -> client 请求：`roots/list`、`tools/call`、
  `resources/read`、`prompts/get`、`elicitation/create`、
  `sampling/createMessage`
- 维护客户端/服务端元信息、endpoint 列表和 roots 快照

### 2.3 当前支持能力（明确支持）

以下能力在当前 `MCPClient` 公开 API 中可直接使用：

- 生命周期与状态
  - `connect` / `connectWithConfig` / `disconnect` / `isConnected`
  - `setTransportType` / `getTransportType` / `getTransportStatus`
- 初始化与协商
  - `setClientInfo` / `setCapabilities`
  - `initialize` / `initialized`
  - `getServerInfo` / `getServerCapabilities`
- Tool / Resource / Prompt 主流程
  - `listTools` / `callTool`
  - `listResources` / `listResourceTemplates` / `readResource`
  - `listPrompts` / `getPrompt`
- 扩展方法
  - `setLoggingLevel`
  - `getTask` / `getTaskResult` / `listTasks` / `cancelTask`
  - `complete`
  - `subscribeResource` / `unsubscribeResource`
- 通知与入站请求处理
  - `sendNotification` / `setNotificationHandler`
  - `setProgressHandler` / `setCancelRequestHandler`
  - `setToolCallHandler` / `setResourceReadHandler` /
    `setPromptGetHandler` / `setElicitationHandler` /
    `setSamplingHandler`
  - `setRoots` / `getRoots`（供 `roots/list` 入站请求返回）
- endpoint 管理
  - `setEndpoints` / `setCurrentEndpoint` / `getCurrentEndpoint` /
    `getEndpoints`

### 2.4 当前不支持能力（明确不支持）

以下能力在当前 client 公共层不提供或未实现：

- ❌ WebSocket 传输接入
  - 公开 `TransportType` 仅支持 `kIpc` / `kSse` / `kAuto`。
- ❌ 公共异步 MCP 请求 API
  - `MCPClient` 当前公开请求接口均为同步阻塞风格。
- ❌ 自动端点故障切换策略
  - 虽有 endpoint 列表管理接口，但无公开“自动探活 + 自动切换”机制。
- ❌ Client 侧注册表能力（server 风格）
  - 不提供 `registerTool/registerResource/registerPrompt` 这类注册接口；
    client 仅通过 handler 响应 server 主动请求。
- ❌ Client 主动发起 `elicitation/create` 与 `sampling/createMessage`
  - 这两个方向不符合 MCP 2025 client 语义，对应旧接口已从当前 SDK 删除。
- ❌ 标准化认证协商/会话管理高阶 API
  - 公共层未提供 OAuth、会话续期、凭据刷新等封装。

说明：

- `MCPClient` 中“可发起”的某些方法（如 `tasks/*`、`completion/complete`）
  是否成功，取决于服务端能力声明与运行时策略；不满足条件时会返回协议错误。
- `sampling/createMessage` 与 `elicitation/create` 为 server -> client 方向，
  client 侧通过 handler 处理入站请求，不作为 client 主动发起能力。

### 2.5 稳定性说明（2026-04-06）

- IPC 超时线程生命周期竞态已完成根修复：
  `PipeCommunication` 超时处理不再使用 detached 后台线程，而采用可管理、可回收的线程机制，
  并在关闭流程中统一唤醒与 join，避免析构期访问失效对象状态。
- 当前 IPC 请求超时以 transport 配置 `ipc.read_timeout_ms` 为准；
  超时后返回错误并由上层重试策略决定是否重试。

## 3. 核心类接口

### 3.1 `mcp::MCPClient`

#### 生命周期

```cpp
MCPClient();
virtual ~MCPClient();
bool connect(const std::string& endpoint);
bool connectWithConfig(const mcp_transport::TransportConfig& config);
void disconnect();
bool isConnected() const;
```

行为约束：

- `connect(endpoint)` 支持：
  - `cmd:<command ...>`（IPC）
  - `http(s)://host[:port][/path]`（SSE）
  - 不包含 `://` 的其他非空字符串按 IPC 命令处理
- endpoint 为空、`cmd:` 后无命令、HTTP(S) port 非法或包含未知 scheme 时，抛
  `mcp_exception(ErrorCode::kInvalidParams, ...)`。
- 析构时会触发断开流程，保证 transport 停止。

#### 传输控制与状态

```cpp
void setTransportType(mcp_transport::TransportType type);
mcp_transport::TransportType getTransportType() const;
mcp_transport::TransportStatus getTransportStatus() const;
```

说明：

- `setTransportType()` 仅设置偏好类型，不触发自动重连。

#### 初始化与基础请求

```cpp
InitializeResult initialize(const InitializeParams& params);
void initialized();
void ping();
```

行为约束：

- `initialize()` 会校验协议版本，不支持的版本抛 `kInvalidParams`。
- `initialize()` 成功后会缓存 `serverInfo` 与 `serverCapabilities`。
- `initialized()` 是通知，不等待响应。

#### Tool / Resource / Prompt

```cpp
ListToolsResult listTools(const std::string& cursor = "");
ToolCallResult callTool(const ToolCallParams& params);
ToolCallResult callTool(const std::string& name,
                        const json& arguments = json::object());

ListResourcesResult listResources(const std::string& cursor = "");
ListResourceTemplatesResult listResourceTemplates(
    const std::string& cursor = "");
ReadResourceResult readResource(const std::string& uri);

ListPromptsResult listPrompts(const std::string& cursor = "");
PromptResult getPrompt(const std::string& name,
                       const json& arguments = json::object());
```

行为约束：

- `callTool` 在远端返回 error 时不抛异常，而是
  `ToolCallResult.isError = true`。
- 其他大多数请求方法在远端错误时抛 `mcp_exception`。
- 分页列表接口统一由返回对象承载分页字段（`nextCursor`），
  不再使用输出参数。
- 分页列表接口对响应字段采用严格校验：列表字段缺失/类型错误，
  或 `nextCursor` 类型错误时抛异常。

#### 扩展请求

```cpp
void setLoggingLevel(const std::string& level);
Task getTask(const std::string& task_id);
TaskResult getTaskResult(const std::string& task_id);
ListTasksResult listTasks(const std::string& cursor = "");
Task cancelTask(const std::string& task_id);
CompleteResult complete(const json& params);
void subscribeResource(const std::string& uri);
void unsubscribeResource(const std::string& uri);
```

#### 通知与入站请求处理

```cpp
void sendNotification(const std::string& method,
                      const json& params = json::object());
void setNotificationHandler(const std::string& method, NotificationHandler handler);
void setProgressHandler(ProgressHandler handler);
void setCancelRequestHandler(CancelHandler handler);
void setToolCallHandler(ToolCallHandler handler);
void setResourceReadHandler(ResourceReadHandler handler);
void setPromptGetHandler(PromptGetHandler handler);
```

行为约束：

- `sendNotification()` 在未连接时会被忽略（不会抛异常）。
- server 主动请求处理：
  - `roots/list` -> 返回本地 `roots` 快照
  - `tools/call` -> 调用 `ToolCallHandler`
  - `resources/read` -> 调用 `ResourceReadHandler`
  - `prompts/get` -> 调用 `PromptGetHandler`

#### 元信息与 endpoint

```cpp
void setClientInfo(const std::string& name, const std::string& version);
void setCapabilities(const ClientCapabilities& capabilities);
ServerInfo getServerInfo() const;
ServerCapabilities getServerCapabilities() const;

void setEndpoints(const std::vector<std::string>& endpoints);
void setCurrentEndpoint(const std::string& endpoint);
std::string getCurrentEndpoint() const;
std::vector<std::string> getEndpoints() const;

void setRoots(const std::vector<Root>& roots);
std::vector<Root> getRoots() const;
```

行为约束：

- `setClientInfo()` 的 `name` 与 `version` 必须为非空字符串，否则抛
  `mcp_exception(ErrorCode::kInvalidParams, ...)`。

## 4. 调用流程与时序

### 4.1 推荐调用顺序

1. 创建 `MCPClient`
2. 配置 client 信息与能力（可选）
3. 建立连接（IPC 或 SSE）
4. 调用 `initialize`
5. 调用 `initialized`
6. 调用 `listTools/listResources/listPrompts` 做能力发现
7. 按需调用 `callTool/readResource/getPrompt`
8. 结束时 `disconnect`

### 4.2 最小时序

```text
Client                         Server
  |                               |
  | ---- initialize ----------->  |
  | <--- result(serverInfo...) -  |
  | ---- notifications/initialized
  | ---- tools/list ----------->  |
  | <--- result(tools) ---------  |
  | ---- tools/call ----------->  |
  | <--- result(content/error) -  |
```

## 5. 完整示例

### 5.1 仓库内完整示例（推荐）

仓库已提供 client 端全能力示例：

- `examples/client/client_full_example.cpp`

该示例覆盖：

- 连接管理、初始化、`ping`
- tools/resources/prompts 全流程
- 通知处理与自定义通知发送
- server -> client 的 handler 注册
- endpoint 与状态查询
- IPC / SSE 两种模式

### 5.2 示例编译与运行

在仓库根目录执行：

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build -j4
```

IPC 模式（客户端拉起服务端）：

```bash
./build/bin/mcp_client_full_example --ipc "./build/bin/mcp_server_full_example --ipc"
```

SSE 模式：

```bash
./build/bin/mcp_server_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
./build/bin/mcp_client_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
```

### 5.3 最小可运行代码

```cpp
#include <iostream>
#include "client.h"

int main() {
    mcp::MCPClient client;

    if (!client.connect("http://127.0.0.1:18080/mcp")) {
        std::cerr << "connect failed" << std::endl;
        return 1;
    }

    mcp::InitializeParams init;
    init.protocolVersion = mcp::PROTOCOL_VERSION;
    init.clientInfo.name = "demo-client";
    init.clientInfo.version = "1.0.0";

    try {
        auto init_result = client.initialize(init);
        client.initialized();

        auto tools = client.listTools();
        std::cout << "tools: " << tools.tools.size() << std::endl;

        client.ping();
        std::cout << "ping ok" << std::endl;
    } catch (const mcp::mcp_exception& e) {
        std::cerr << "mcp error: " << e.what() << std::endl;
        client.disconnect();
        return 2;
    }

    client.disconnect();
    return 0;
}
```

## 6. 完整工程配置说明

## 6.1 方式：安装版 SDK 接入（推荐发布方式）

`libkysdk-mcp` 安装后，推荐通过 `pkg-config` 使用
`kysdk-mcp-client.pc` 提供编译与链接参数。

`CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_mcp_client_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDK_MCP_CLIENT REQUIRED kysdk-mcp-client)

add_executable(my_mcp_client src/main.cpp)

target_include_directories(my_mcp_client PRIVATE
    ${KYSDK_MCP_CLIENT_INCLUDE_DIRS}
)

target_compile_options(my_mcp_client PRIVATE
    ${KYSDK_MCP_CLIENT_CFLAGS_OTHER}
)

target_link_directories(my_mcp_client PRIVATE
    ${KYSDK_MCP_CLIENT_LIBRARY_DIRS}
)

target_link_libraries(my_mcp_client PRIVATE
    ${KYSDK_MCP_CLIENT_LIBRARIES}
)
```

`src/main.cpp` 使用：

```cpp
#include "client.h"
```

配置与构建：

```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
cmake -S . -B build
cmake --build build -j4
```

## 6.2 pkg-config 关键项（当前安装产物）

`kysdk-mcp-client.pc` 当前导出：

- `Cflags: -I<install-prefix>/include/kylin-ai/mcp -I<install-prefix>/include/kylin-ai/mcp/client -I<install-prefix>/include/kylin-ai/mcp/common`
- `Libs: -L<install-prefix>/lib -lkysdk-mcp-client`
- `Libs.private: -lssl -lcrypto -lpthread`

说明：

- 如果你的构建系统区分静态/动态链接，请按需要补齐 `Libs.private`。
- 如果 `pkg-config --modversion kysdk-mcp-client` 失败，优先检查
  `PKG_CONFIG_PATH` 与安装路径。

## 7. 线程安全与限制说明

- `MCPClient` 内部使用互斥锁保护 handler 注册表与 roots 快照。
- handler 的执行线程由 transport 消息回调线程决定，不保证主线程。
- 若 handler 访问共享业务状态，调用方需自行同步。
- 当前请求路径是同步阻塞模型；高并发场景建议由上层做任务调度与超时治理。

## 8. 常见问题

- `initialize` 报协议版本错误
  - 检查 `InitializeParams.protocolVersion`，不支持版本会抛
    `mcp_exception(ErrorCode::kInvalidParams, ...)`。
- `connect(http://...)` 成功但请求失败
  - 检查服务端监听地址、端口、endpoint 是否一致。
- 某扩展方法持续返回 `method_not_found`
  - 优先检查双方能力协商结果与请求方法名是否一致，再检查服务端运行时是否启用了对应 handler。
