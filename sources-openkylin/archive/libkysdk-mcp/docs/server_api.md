# libkysdk-mcp Server 接口文档

本文档面向 `libkysdk-mcp` 的服务端使用者，重点说明仓库当前 `server`
模块暴露的公开接口、协议数据结构、传输配置，以及与现有实现保持一致的
行为边界。

## 1. 公开头文件

服务端开发主要涉及以下公开头文件：

- `include/server/server.h`
  - 通用服务端门面类 `mcp::MCPServer`
- `include/common/protocol.h`
  - 协议对象、能力声明、Tool/Resource/Prompt 数据结构、回调类型
- `include/common/message.h`
  - JSON-RPC/MCP 消息、异常和错误码定义
- `include/common/transport.h`
  - 传输层类型、`TransportConfig` 配置结构和 `Transport` 抽象

如果以安装后的 SDK 方式接入，对应头文件会安装到
`kylin-ai/mcp/server/` 目录下，文件名保持一致。

## 2. 服务端整体模型

### 2.1 类关系

```text
mcp::MCPServer
```

### 2.2 服务端职责

`MCPServer` 负责：

- 启动和停止传输层
- 处理 `initialize`、`ping`
- 处理 `tools/list`、`tools/call`
- 处理 `resources/list`、`resources/read`
- 处理 `resources/templates/list`
- 处理 `prompts/list`、`prompts/get`
- 处理 `logging/setLevel`
- 处理 `$/cancelRequest`（通过注册的 `CancelHandler`）
- 向 client 发起 `roots/list` 请求，处理 `notifications/roots/list_changed`
- 向 client 发送 `notifications/progress` 进度通知
- 维护 Tool / Resource / ResourceTemplate / Prompt 注册表
- 在 SSE 模式下为连接建立阶段返回服务端握手信息

当前实现中，`logging/setLevel` 与 `resources/templates/list` 均已提供请求处理；
其中 `ResourceTemplate` 通过 `registerResourceTemplate()` 等公开接口管理。

### 2.3 当前能力清单（明确支持 / 不支持）

以下结论基于当前仓库代码与公开接口，按“可直接使用的公开能力”给出。

#### ✅ 当前支持的能力

- 基础请求：`initialize`、`ping`
- Tool：`tools/list`、`tools/call`，以及 `registerTool()` / `unregisterTool()` /
  `clearTools()` / `listTools()`
- Resource：`resources/list`、`resources/read`，以及 `registerResource()` /
  `unregisterResource()` / `clearResources()` / `listResources()`
- ResourceTemplate：`resources/templates/list`，以及
  `registerResourceTemplate()` / `unregisterResourceTemplate()` /
  `clearResourceTemplates()` / `listResourceTemplates()`
- Prompt：`prompts/list`、`prompts/get`，以及 `registerPrompt()` /
  `unregisterPrompt()` / `clearPrompts()` / `listPrompts()`
- Logging：`logging/setLevel`（修改服务端本地日志级别）
- Progress / Cancellation：`notifications/progress`、`$/cancelRequest`
- Resource subscriptions：`resources/subscribe`、`resources/unsubscribe`
- Completion：`completion/complete`（需注册 Completion handler）
- Elicitation（server → client）：`requestElicitation()`
- Sampling（server → client）：`requestCreateMessage()`
- Task：`tasks/get`、`tasks/result`、`tasks/list`、`tasks/cancel`
- Roots（server → client）：
  `requestRootsList()` + `setRootsListChangedHandler()`，对应 `roots/list` 与
  `notifications/roots/list_changed`
- 服务端变更通知：
  `notifyResourcesChanged()`、`notifyToolsChanged()`、`notifyPromptsChanged()`
  （IPC / SSE 均可下发；SSE 需先建立事件流通道：`{endpoint}/sse` 或
  `streamable_http=true` 时的 `GET {endpoint}`）
- JSON-RPC 错误映射：未知方法 `method_not_found`、参数错误 `invalid_params`、
  `mcp_exception` 错误码透传

#### ❌ 当前不支持的能力

- Server 向 client 发送日志消息的标准公开接口（仅支持 `logging/setLevel`）
- HTTPS 服务端 TLS 监听（传输层无证书/私钥配置入口）

#### ⚠️ 部分支持 / 使用前提

- SSE 下发通知与 server→client 请求依赖已建立的 SSE 反向通道；未建立时相关调用会失败。
- `TransportConfig::sse.protocol == TransportProtocol::kHttps` 在服务端模式下会被
  拒绝（`validate()` 返回 `false`，`start()` 也会返回 `false`）；当前服务端仅支持
  `kHttp` 监听。
- `list*` 接口支持基于 `cursor` 的分页；同时实现还接受扩展参数 `limit`（非标准字段）。

#### 稳定性说明（2026-04-06）

- IPC 超时线程生命周期竞态已完成根修复：
  `PipeCommunication` 超时处理不再使用 detached 后台线程，而采用可管理、可回收的线程机制，
  并在关闭流程中统一唤醒与 join，避免析构期访问失效对象状态。
- 当前 IPC 请求超时以 transport 配置 `ipc.read_timeout_ms` 为准；
  超时后返回错误并由上层重试策略决定是否重试。

## 3. 核心类接口

### 3.1 `mcp::MCPServer`

#### 生命周期

```cpp
MCPServer();
virtual ~MCPServer();
bool start(const mcp_transport::TransportConfig& config);
void stop();
bool isRunning() const;
```

说明：

- `start()` 使用 `TransportConfig` 启动服务端监听，并在内部绑定
  `TransportServerRequestHandler`。
- `stop()` 停止服务。
- 析构时如果服务还在运行，会先执行停止。
- `start()` 不会主动校验 `config.mode == TransportMode::kServer`，但服务端
  使用时应显式按服务端模式配置。

#### 服务标识与能力

```cpp
void setServerInfo(const std::string& name, const std::string& version);
ServerInfo getServerInfo() const;
void setCapabilities(const ServerCapabilities& capabilities);
```

说明：

- `setServerInfo()` 用于初始化握手时返回 `serverInfo`。
- `setServerInfo()` 的 `name` 与 `version` 必须为非空字符串，否则抛
  `mcp_exception(ErrorCode::kInvalidParams, ...)`。
- `setCapabilities()` 用于初始化握手时返回 `capabilities`。

#### Tool 管理

```cpp
bool registerTool(const Tool& tool, ToolCallHandler handler);
bool unregisterTool(const std::string& name);
void clearTools();
std::vector<Tool> listTools() const;
```

行为约束：

- 工具名为空或处理器为空时注册失败。
- 同名工具重复注册返回 `false`。
- `tools/call` 时如果找不到处理器，返回 `ToolCallResult{ isError = true }`。
- Tool handler 中抛出的标准异常会被包装为错误结果。
- Tool handler 当前由请求处理链路**同步直接调用**，不会在 `MCPServer` 层额外切换到
  新线程或任务队列；因此它运行在“当前处理该请求的线程”中，而不保证是业务主线程。
- 如果 handler 执行耗时较长，会直接阻塞当前请求的处理与响应返回。
- `ToolCallResult::toJson()` 在错误场景下序列化为 `{ "error": "..." }`，
  成功场景序列化为 `{ "content": ... }`。
- 运行期如果存在并发请求或动态注册/注销，业务回调应自行保证可重入和状态一致性。

#### Resource 管理

```cpp
bool registerResource(const Resource& resource, ResourceReadHandler handler);
bool unregisterResource(const std::string& uri);
void clearResources();
std::vector<Resource> listResources() const;
```

行为约束：

- `resource.uri` 为空或处理器为空时注册失败。
- 同 URI 资源重复注册返回 `false`。
- `resources/read` 时如果未找到资源处理器，会抛出 `mcp_exception`。
- 资源读取中的标准异常会被包装为
  `mcp_exception(ErrorCode::kInternalError, ...)`。
- 运行期如果存在并发请求或动态注册/注销，业务回调应自行保证可重入和状态一致性。

#### ResourceTemplate 管理

```cpp
bool registerResourceTemplate(const ResourceTemplate& resource_template);
bool unregisterResourceTemplate(const std::string& uri_template);
void clearResourceTemplates();
std::vector<ResourceTemplate> listResourceTemplates() const;
```

行为约束：

- `resource_template.uriTemplate` 为空时注册失败。
- 同 `uriTemplate` 模板重复注册返回 `false`。
- `resources/templates/list` 返回 `{ "resourceTemplates": [...] }`，
  其中每一项为 `ResourceTemplate::toJson()` 的结果。

#### Prompt 管理

```cpp
bool registerPrompt(const Prompt& prompt, PromptGetHandler handler);
bool unregisterPrompt(const std::string& name);
void clearPrompts();
std::vector<Prompt> listPrompts() const;
```

行为约束：

- Prompt 名为空或处理器为空时注册失败。
- 同名 Prompt 重复注册返回 `false`。
- `prompts/get` 时如果未找到处理器，会抛出 `mcp_exception`。
- Prompt handler 抛出的异常会由更外层请求处理逻辑统一转换为 JSON-RPC error
  响应；如果业务需要更细粒度错误码，应主动抛出 `mcp_exception`。
- 运行期如果存在并发请求或动态注册/注销，业务回调应自行保证可重入和状态一致性。

#### 变更通知

```cpp
bool notifyResourcesChanged();
bool notifyToolsChanged();
bool notifyPromptsChanged();
```

当前实现限制：

- 这三个接口会尝试通过 `Transport::sendNotification()` 下发通知。
- 在 **IPC 服务端模式**下，通知可真正发送给 client。
- 在 **SSE 服务端模式**下，通知会通过 SSE 事件流推送；若当前没有可用 SSE
  反向通道（例如尚未建立事件流连接），调用会失败并返回 `false`。
- 服务未启动时调用会记录 warning，并返回 `false`。

#### Roots（server → client）

```cpp
std::vector<Root> requestRootsList(int timeout_ms = 10000);
void setRootsListChangedHandler(RootsListChangedHandler handler);
```

行为说明：

- `requestRootsList()` 通过 transport 向 client 发送 `roots/list` JSON-RPC
  请求，并同步等待响应（默认超时 10 秒）。
- 返回 client 报告的 `Root` 列表；超时、解析失败或服务未启动时返回空 vector。
- `setRootsListChangedHandler()` 注册回调。当 client 发送
  `notifications/roots/list_changed` 时，服务端会自动重新执行 `roots/list`
  请求，并将最新的 root 列表传入回调。
- **传输层说明**：`requestRootsList()` 依赖 transport 的 `sendRequest()` 能力。
  IPC 模式通过双向管道实现；SSE 模式通过 SSE 事件流推送请求，client 需先
  连接到 `{endpoint}/sse`（legacy）或在 `streamable_http=true` 时连接
  `GET {endpoint}` 建立事件流通道，然后将响应 POST 回主端点。
- `Root` 数据结构：

```cpp
struct Root {
    std::string uri;   // 必填
    std::string name;  // 可选
};
```

#### Progress / Cancellation

```cpp
bool sendProgress(const ProgressParams& params);
void setCancelHandler(CancelHandler handler);
```

行为说明：

- `sendProgress()` 向 client 发送 `notifications/progress` 通知，传输机制与
  `notifyToolsChanged()` 等一致（IPC 和 SSE 模式均可用）。
- `setCancelHandler()` 注册 `$/cancelRequest` 处理回调。当 client 发送取消
  请求时，回调收到 `CancelParams`，业务层需自行协作中止对应的请求处理。
- 服务端并不强制取消正在执行的 handler，这是一个协作式取消机制。
- `ProgressParams` 数据结构：

```cpp
struct ProgressParams {
    json progressToken;    // 必填，与原始请求中的 _meta.progressToken 对应
    double progress;       // 必填，当前进度值
    double total;          // 可选（> 0 时序列化）
    std::string message;   // 可选
};
```

- `CancelParams` 数据结构：

```cpp
struct CancelParams {
    json requestId;       // 必填，要取消的请求 ID
    std::string reason;   // 可选
};
```

## 4. 调用流程

本节给出服务端接入时最常见的调用顺序，便于结合公开接口快速落地。

### 4.1 服务端启动流程

典型服务端初始化顺序如下：

1. 创建 `mcp::MCPServer` 实例
2. 调用 `setServerInfo()` 设置服务端名称和版本
3. 调用 `setCapabilities()` 声明当前 server 能力
4. 调用 `registerTool()` / `registerResource()` /
   `registerResourceTemplate()` / `registerPrompt()` 注册业务能力
5. 构造 `mcp_transport::TransportConfig`
6. 调用 `start()` 启动监听
7. 运行期按需调用 `notifyResourcesChanged()` / `notifyToolsChanged()` /
   `notifyPromptsChanged()`
8. 服务退出前调用 `stop()`

最小示例：

```cpp
mcp::MCPServer server;

server.setServerInfo("demo-server", "1.0.0");

mcp::ServerCapabilities capabilities;
capabilities.tools = {{"listChanged", true}};
server.setCapabilities(capabilities);

mcp::Tool echo_tool;
echo_tool.name = "echo";
echo_tool.description = "Echo input text";
echo_tool.inputSchema = {
    {"type", "object"},
    {"properties", {{"text", {{"type", "string"}}}}}
};

server.registerTool(echo_tool, [](const mcp::ToolCallParams& params) {
    mcp::ToolCallResult result;
    result.content = nlohmann::json::array({
        {{"type", "text"}, {"text", params.arguments.value("text", "")}}
    });
    return result;
});

mcp_transport::TransportConfig config;
config.type = mcp_transport::TransportType::kIpc;
config.mode = mcp_transport::TransportMode::kServer;

if (!server.start(config)) {
    throw std::runtime_error("Failed to start server");
}
```

### 4.2 客户端调用流程

客户端与服务端交互时，推荐顺序如下：

1. 建立 IPC 或 SSE 连接
2. 发送 `initialize`
3. 收到 `initialize` 响应后，发送 `notifications/initialized`
4. 调用能力枚举接口：
   - `tools/list`
   - `resources/list`
   - `prompts/list`
5. 按需调用具体能力：
   - `tools/call`
   - `resources/read`
   - `prompts/get`
6. 如需调整服务端本地日志级别，可调用 `logging/setLevel`

说明：

- `initialize` 是首个必须成功的请求。
- `notifications/initialized` 是通知；在 HTTP/SSE 传输上会返回 `202 Accepted`。
- `logging/setLevel` 当前仅修改服务端本地日志级别，不会建立 server 到
  client 的日志推送链路。

### 4.3 最小时序

#### 初始化与能力发现

```text
Client                         Server
  |                               |
  | ---- initialize ----------->  |
  | <--- result(serverInfo...) -  |
  | ---- notifications/initialized
  | ---- tools/list ----------->  |
  | <--- result(tools) ---------  |
  | ---- resources/list ------->  |
  | <--- result(resources) -----  |
  | ---- prompts/list --------->  |
  | <--- result(prompts) -------  |
```

#### Tool 调用

```text
Client                         Server
  |                               |
  | ---- tools/call ---------->   |
  |      {name, arguments}        |
  | <--- result(content/error) -  |
```

#### 服务端通知列表变更

```text
Server                         Client
  |                               |
  | -- notifications/tools/list_changed -->
```

限制说明：

- 上述 `notify*Changed()` 在 IPC 和 SSE 模式下均可真正下发。
- SSE 模式下 client 需先建立事件流通道（`{endpoint}/sse` 或 streamable HTTP 的
  `GET {endpoint}`）。

### 4.4 错误处理流程

当前请求处理链路遵循以下规则：

- 未知方法返回 JSON-RPC `method_not_found`
- 参数解析失败或参数缺失返回 `invalid_params`
- 业务层主动抛出 `mcp_exception` 时，沿用其错误码返回
- 其他未捕获标准异常返回 `internal_error`

示意：

```text
Client                         Server
  |                               |
  | ---- unknown/method ------>   |
  | <--- error{-32601,...} ----   |
```

## 5. 协议数据结构

以下类型主要定义在 `include/common/protocol.h`。

### 5.1 协议版本与方法名

- 服务端默认协议版本：`2025-11-25`
- HTTP 传输层在请求头缺失 `MCP-Protocol-Version` 时，按 `2025-03-26` 作为默认值进行处理
- 常见方法：
  - `initialize`
  - `ping`
  - `tools/list`
  - `tools/call`
  - `resources/list`
  - `resources/read`
  - `resources/templates/list`
  - `prompts/list`
  - `prompts/get`
- `logging/setLevel`
- `roots/list`（server → client 请求）
- `$/cancelRequest`（client → server 通知）
- `notifications/roots/list_changed`
- `notifications/progress`
- `resources/subscribe`
- `resources/unsubscribe`
- `completion/complete`
- `requestElicitation`（server → client）
- `requestCreateMessage`（server → client）
- `tasks/get` / `tasks/result` / `tasks/list` / `tasks/cancel`

### 5.2 能力与身份对象

```cpp
struct ClientCapabilities;
struct ServerCapabilities;
struct ClientInfo;
struct ServerInfo;
struct InitializeParams;
struct InitializeResult;
```

关键字段：

- `ServerCapabilities`
  - `experimental`
  - `prompts`
  - `resources`
  - `tools`
  - `logging`
  - `sampling`
- `ServerInfo`
  - `name`
  - `version`
- `InitializeResult`
  - `protocolVersion`
  - `capabilities`
  - `serverInfo`

### 5.3 Tool / Resource / ResourceTemplate / Prompt

```cpp
struct Tool {
    std::string name;
    std::string description;
    json inputSchema;
};

struct ToolCallParams {
    std::string name;
    json arguments;
};

struct ToolCallResult {
    json content;
    bool isError = false;
    std::string errorMessage;
};

struct Resource {
    std::string uri;
    std::string name;
    std::string description;
    std::string mimeType;
};

struct ResourceTemplate {
    std::string uriTemplate;
    std::string name;
    std::string description;
    std::string mimeType;
    json annotations = json::object();
};

struct Prompt {
    std::string name;
    std::string description;
    std::vector<std::string> arguments;
};
```

### 5.4 Root / Progress / Cancel

```cpp
struct Root {
    std::string uri;
    std::string name;
};

struct ProgressParams {
    json progressToken;
    double progress = 0.0;
    double total = 0.0;
    std::string message;
};

struct CancelParams {
    json requestId;
    std::string reason;
};
```

回调类型：

```cpp
using ToolCallHandler = std::function<ToolCallResult(const ToolCallParams&)>;
using ResourceReadHandler =
    std::function<ReadResourceResult(const std::string& uri)>;
using PromptGetHandler =
    std::function<PromptResult(const std::string& name, const json& arguments)>;
using RootsListChangedHandler = std::function<void(const std::vector<Root>& roots)>;
using CancelHandler = std::function<void(const CancelParams& params)>;
```

补充说明：

- `ToolCallResult` 的 JSON 表达并不直接暴露 `isError` 字段，而是通过
  `error` 或 `content` 字段区分。
- `ResourceReadHandler` 返回 `ReadResourceResult`，需填充
  `contents`（元素类型为 `ResourceContent`）。
- `PromptGetHandler` 返回 `PromptResult`，需填充 `messages`
  （元素类型为 `PromptMessage`）。
- `Prompt.arguments` 的 C++ API 仍然是 `std::vector<std::string>`，但在线上
  协议序列化时会输出为对象数组：
  `[{ "name": "...", "description": "", "required": false }]`。
  反序列化同时兼容旧格式字符串数组和新格式对象数组。

## 5. 传输层接口

以下类型定义在 `include/common/transport.h`。

### 5.1 枚举

```cpp
enum class TransportType { kIpc, kSse, kAuto };
enum class TransportMode { kClient, kServer };
enum class TransportStatus { kDisconnected, kConnecting, kConnected, kError };
```

### 5.2 `TransportConfig`

服务端最常用字段如下：

```cpp
TransportConfig config;
config.type = TransportType::kSse;      // 或 IPC
config.mode = TransportMode::kServer;
config.name = "mcp_transport";

config.server.server_name = "my-server";
config.server.server_version = "1.0.0";
config.server.description = "My MCP server";

config.sse.host = "0.0.0.0";
config.sse.port = 8080;
config.sse.protocol = TransportProtocol::kHttp;   // server mode currently supports kHttp only
config.sse.endpoint = "/mcp";

config.ipc.command = "...";
config.ipc.args = {...};
```

补充说明：

- `type = IPC` 时主要依赖 `ipc.*` 配置。
- `type = SSE` 时主要依赖 `sse.*` 配置。
- `mode` 对 server 侧必须设置为 `TransportMode::kServer`。
- `validate()` 可用于启动前校验配置。
- 服务端模式下 `config.sse.protocol` 当前仅支持 `kHttp`。
- 如果在服务端模式配置为 `kHttps`，`validate()` 会失败，`start()` 也会返回 `false`。
- `server.server_name` / `server.server_version` / `server.description`
  属于传输层服务信息，不会自动覆盖 `MCPServer::setServerInfo()` 返回给
  MCP 客户端的 `serverInfo`。

### 5.3 `Transport`

`MCPServer` 内部已经封装了 `Transport`，通常不需要业务层直接操作。
如果要自定义更底层的收发或扩展服务端流程，可关注以下公开接口：

```cpp
bool start(const TransportConfig& config);
bool stop();
bool isRunning() const;
TransportStatus getStatus() const;
ConnectionInfo getConnectionInfo() const;
void setServerRequestHandler(std::shared_ptr<TransportServerRequestHandler>);
void setSseServerRequestHandler(std::shared_ptr<SseServerRequestHandler>);
void setConnectionRequestHandler(std::shared_ptr<SseServerConnectHandler>);
bool sendResponse(const std::string& request_id, const std::string& result);
bool sendError(const std::string& request_id, int error_code,
               const std::string& error_message);
bool sendNotification(const std::string& method, const std::string& params);
```

补充说明：

- `Transport` 还暴露了 `sendRequest`、`sendRequestSync`、消息/连接/错误回调、
  配置更新、统计与健康检查等能力，更适合协议开发或调试场景。
- `MCPServer` 的 `notify*Changed()` 与 `sendProgress()` 最终都会调用
  `Transport::sendNotification()`；是否成功取决于当前运行状态与连接可用性。

## 6. 推荐服务端接入流程

### 6.1 通用 `MCPServer`

```cpp
#include "server/server.h"
#include "common/protocol.h"
#include "common/transport.h"

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>

using namespace mcp;
using namespace mcp_transport;

namespace {
std::atomic<bool> g_running{true};
std::mutex g_mutex;
std::condition_variable g_cv;

void handleSignal(int) {
    g_running = false;
    g_cv.notify_all();
}
}  // namespace

int main() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    MCPServer server;

    server.setServerInfo("demo-server", "1.0.0");

    ServerCapabilities caps;
    caps.tools = json{{"listChanged", true}};
    caps.resources = json{{"listChanged", true}};
    caps.prompts = json{{"listChanged", true}};
    server.setCapabilities(caps);

    Tool echo_tool;
    echo_tool.name = "echo";
    echo_tool.description = "Echo input text";
    echo_tool.inputSchema = {
        {"type", "object"},
        {"properties", {
            {"text", {{"type", "string"}}}
        }},
        {"required", json::array({"text"})}
    };

    server.registerTool(echo_tool, [](const ToolCallParams& params) {
        ToolCallResult result;
        result.content = json::array({
            json{
                {"type", "text"},
                {"text", params.arguments.value("text", "")}
            }
        });
        return result;
    });

    Resource about_resource;
    about_resource.uri = "memo://server/about";
    about_resource.name = "about";
    about_resource.description = "Basic server description";
    about_resource.mimeType = "application/json";

    server.registerResource(
        about_resource, [](const std::string& uri) -> ReadResourceResult {
            ReadResourceResult result;
            ResourceContent content;
            content.uri = uri;
            content.mimeType = "application/json";
            content.text = json{
                {"name", "demo-server"},
                {"version", "1.0.0"},
                {"features", json::array({"tools", "resources", "prompts"})}
            }.dump();
            result.contents.push_back(content);
            return result;
    });

    Prompt summarize_prompt;
    summarize_prompt.name = "summarize_text";
    summarize_prompt.description = "Create a simple summary prompt";
    summarize_prompt.arguments = {"text", "style"};

    server.registerPrompt(
        summarize_prompt,
        [](const std::string& name, const json& arguments) -> PromptResult {
            (void)name;
            PromptResult result;
            result.description = "Prompt generated by server";

            PromptMessage message;
            message.role = Role::kUser;
            message.content = {
                {"type", "text"},
                {"text",
                 "Summarize the following text in " +
                 arguments.value("style", std::string("brief")) +
                 " style: " + arguments.value("text", std::string(""))}
            };
            result.messages.push_back(message);
            return result;
        });

    // ---- Roots: 监听 client 的 roots 变更 ----
    server.setRootsListChangedHandler([](const std::vector<Root>& roots) {
        std::cout << "Client roots changed, count=" << roots.size() << std::endl;
        for (const auto& root : roots) {
            std::cout << "  root: " << root.uri << " (" << root.name << ")"
                      << std::endl;
        }
    });

    // ---- Cancellation: 注册取消处理器 ----
    server.setCancelHandler([](const CancelParams& params) {
        std::cout << "Cancel requested for request " << params.requestId.dump()
                  << ", reason: " << params.reason << std::endl;
        // 业务层在此协作取消对应的请求处理
    });

    // Snapshot queries for the currently registered items.
    auto tools = server.listTools();
    auto resources = server.listResources();
    auto prompts = server.listPrompts();

    // Example removal calls. In real services these are usually driven by
    // dynamic configuration reload or shutdown logic.
    bool removed_missing_tool = server.unregisterTool("missing-tool");
    bool removed_missing_resource = server.unregisterResource("memo://missing");
    bool removed_missing_prompt = server.unregisterPrompt("missing-prompt");
    (void)tools;
    (void)resources;
    (void)prompts;
    (void)removed_missing_tool;
    (void)removed_missing_resource;
    (void)removed_missing_prompt;

    TransportConfig config;
    config.type = TransportType::kSse;
    config.mode = TransportMode::kServer;
    config.server.server_name = "demo-server";
    config.server.server_version = "1.0.0";
    config.sse.host = "0.0.0.0";
    config.sse.port = 8080;
    config.sse.endpoint = "/mcp";

    if (!config.validate() || !server.start(config)) {
        return 1;
    }

    // ---- Roots: 主动查询 client 的 roots ----
    // SSE 模式下 client 需先建立事件流通道：
    // - legacy: GET /mcp/sse
    // - streamable_http=true: GET /mcp
    auto roots = server.requestRootsList();
    for (const auto& root : roots) {
        std::cout << "Client root: " << root.uri << " (" << root.name << ")"
                  << std::endl;
    }

    // ---- Progress: 在长时间 Tool handler 中发送进度 ----
    // 通常在 ToolCallHandler 内部调用，这里仅作示意
    {
        ProgressParams progress;
        progress.progressToken = "example-token";
        progress.progress = 1;
        progress.total = 3;
        progress.message = "Step 1 of 3";
        server.sendProgress(progress);
    }

    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [] { return !g_running.load(); });

    server.stop();
    return 0;
}
```

说明：

- `MCPServer::start()` 会完成监听端点和底层处理线程的启动，但本身会立即返回，
  不会阻塞当前主线程。
- 因此 demo 里的 “keep running” 应理解为“由业务层自己阻塞等待退出条件”，常见
  做法是等待信号、挂到现有事件循环，或使用条件变量/同步原语保持进程存活。
- 上面的示例已经覆盖 `setServerInfo()`、`setCapabilities()`、
  `registerTool()`、`registerResource()`、`registerPrompt()`、
  `listTools()`、`listResources()`、`listPrompts()`、
  `unregisterTool()`、`unregisterResource()`、`unregisterPrompt()`、
  `setRootsListChangedHandler()`、`requestRootsList()`、
  `setCancelHandler()`、`sendProgress()`、
  `start()`、`stop()`、`isRunning()` 的典型接入方式。

`clear*()` 一般用于配置重载或整体重建注册表，例如：

```cpp
server.clearTools();
server.clearResources();
server.clearResourceTemplates();
server.clearPrompts();

// Re-register all exported capabilities after clearing.
server.registerTool(...);
server.registerResource(...);
server.registerResourceTemplate(...);
server.registerPrompt(...);
```

### 6.2 最小示例工程结构

如果希望把上面的 demo 直接落成一个可编译工程，可以使用如下目录结构：

```text
mcp_server_demo/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

`src/main.cpp` 可以直接使用上一节示例代码。

### 6.3 安装版 SDK 的完整 CMake 配置

当 `libkysdk-mcp` 已经安装到系统，且 `pkg-config` 能找到
`kysdk-mcp-server.pc` 时，推荐使用下面这份 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)
project(mcp_server_demo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDK_MCP_SERVER REQUIRED IMPORTED_TARGET kysdk-mcp-server)

add_executable(mcp_server_demo
    src/main.cpp
)

target_link_libraries(mcp_server_demo
    PRIVATE
        PkgConfig::KYSDK_MCP_SERVER
)
```

接入边界说明：

- 当前对外接入边界是“安装后的公开头文件 + 已安装库文件”，不要求也不应依赖
  仓库源码树。
- 业务侧应只包含本文档第 1 节列出的公开头文件，并通过 `kysdk-mcp-server.pc`
  提供的编译与链接参数完成接入。
- 如果你使用的是修复前的旧安装包，编译独立 demo 时可能出现：
  `fatal error: ../internal/common/json.hpp: No such file or directory`
  之类的错误；这说明安装产物不完整，需要重新安装新版 SDK，而不是改为直接引用
  源码目录。

构建方式：

```bash
mkdir -p build
cmake -S . -B build
cmake --build build -j
```

如果 `kysdk-mcp-server.pc` 不在系统默认搜索路径，需要先设置
`PKG_CONFIG_PATH`，例如：

```bash
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}"
```

说明：

- `kysdk-mcp-server.pc` 当前会提供 `-lkysdk-mcp-server` 和对应头文件目录。
- 由于 `.pc` 文件已经声明了公开头文件路径，代码中可直接
  `#include "server/server.h"`、`#include "common/protocol.h"`、
  `#include "common/transport.h"`。
- 当前仓库根目录没有导出 `find_package(kysdk_mcp_server)` 风格的 CMake
  package config，因此这里以 `pkg-config` 作为推荐且稳定的接入方式。
- 如果后续发布新的 CMake package config，应以安装产物提供的导入目标为准；在此
  之前，不建议业务工程通过 `add_subdirectory()` 直接耦合仓库源码布局。

## 7. 错误处理与限制

### 7.1 协议版本

- `initialize` 阶段会校验客户端传入的 `protocolVersion`
- 若客户端版本不受支持，服务端会回退到自身支持版本并在 `initialize` 结果中返回协商后的 `protocolVersion`
- 非 `initialize` 请求若版本头非法，将返回 JSON-RPC `invalid_request`（`-32600`）

### 7.2 HTTP/SSE 传输语义（当前实现）

- `GET {endpoint}`：
  - 当 `streamable_http=true` 且 `Accept` 包含 `text/event-stream` 时，建立 SSE 流
  - 当 `streamable_http=true` 且不满足 SSE 条件时，返回 `405`
  - 当 `streamable_http=false` 时，走普通连接处理逻辑
- `POST {endpoint}`：
  - 普通 JSON-RPC 请求走请求处理链路
  - JSON-RPC notification（无 `id`）返回 `202`
- `DELETE {endpoint}`（`streamable_http=true`）：
  - 依据 `Mcp-Session-Id` / `mcp-session-id` / `X-Session-Id` / `x-session-id` 结束会话
  - 缺失会话头返回 `400`，未找到会话返回 `404`
- `Origin`：
  - 若请求携带 `Origin` 且与当前服务地址不匹配，返回 `403`

### 7.3 参数校验约束（当前实现）

- `tools/call`：
  - `name` 必须是非空字符串
  - `arguments`（若存在）必须是 object
- `resources/read`：
  - `uri` 必须是非空字符串
- `resources/subscribe` / `resources/unsubscribe`：
  - `uri` 必须是非空字符串
  - `uri` 必须已经通过 `MCPServer::registerResource()` 注册
- `prompts/get`：
  - `name` 必须是非空字符串
  - `arguments`（若存在）必须是 object
- 不满足上述约束时，返回 `invalid_params`（`-32602`）

### 7.4 当前实现限制

- `MCPServer::initialized()` 当前仅记录日志，没有后续状态机动作
- `notifyResourcesChanged()` / `notifyToolsChanged()` / `notifyPromptsChanged()`
  IPC 和 SSE 模式下均可真正发送通知
- `roots/list`、`$/cancelRequest`、`notifications/progress` 已实现，
  IPC 和 SSE 模式均可用；SSE 模式下 client 需先建立事件流通道
  （`{endpoint}/sse` 或 streamable HTTP 的 `GET {endpoint}`）
- 未匹配的方法会以 JSON-RPC error 语义返回 `method_not_found`
  （`-32601`）；参数解析或必填参数缺失会返回 `invalid_params`（`-32602`）
- `TransportConfig` 仍保留 `kHttps` 协议选项用于 client 侧 HTTPS 连接；
  但 server 端当前没有证书/私钥配置入口，因此服务端模式仅支持 `kHttp`
  的 SSE/HTTP 监听
- SSE 连接阶段会返回 `status/server/version/protocol` 等信息，并通过传输层
  响应头管理 `X-Session-Id`；但后续请求中的 session 校验逻辑当前已关闭

### 7.5 调试与错误定位

服务端联调时，建议先把问题拆成“配置是否有效”“传输是否启动”“请求是否进入
业务处理”“返回值是否符合当前实现”四层逐步定位。

启动前建议先做本地自检：

- 调用 `config.validate()`，优先排除 `TransportConfig` 中的必填项和端口配置问题。
- 明确区分两组服务端信息：
  - `config.server.server_name` / `config.server.server_version` 影响传输层握手信息
  - `MCPServer::setServerInfo()` 影响 MCP `initialize` 返回的 `serverInfo`
- 如果 `start()` 返回 `false`，优先检查 `config.type`、`config.mode`、
  `config.sse.host`、`config.sse.port`、`config.sse.endpoint` 是否与实际监听方式一致。

日志排查建议：

- `TransportConfig` 支持 `enable_logging`、`log_file_path`、`log_level`，可用于打开
  传输层日志；其中 `log_level = 0` 表示 DEBUG，`1/2/3` 分别对应
  INFO/WARNING/ERROR。
- Tool 调用失败时，`src/server/server.cpp` 会记录
  `Tool call failed: ...` 错误日志；如果返回包里出现
  `{ "error": "Tool execution error: ..." }`，通常表示业务 handler 抛出了异常。
- `notifyResourcesChanged()`、`notifyToolsChanged()`、
  `notifyPromptsChanged()` 失败时通常意味着 server 未运行，或 SSE 模式下
  尚未建立可用的事件流通道。

按请求类型定位问题时，可优先观察返回 JSON 的形状：

- `tools/call` 失败通常返回 `ToolCallResult::toJson()` 的错误形式：
  `{ "error": "..." }`
- `resources/read`、`prompts/get` 这类异常路径会在更外层统一转换成
  `{ "error": "Internal error: ..." }`
- 未实现或未匹配的方法会走 JSON-RPC 错误响应，典型错误码为
  `method_not_found`（`-32601`）

这几类返回可以直接帮助判断问题落点：

- 出现 `Unknown tool: ...`：
  先检查 `registerTool()` 是否成功，以及请求里的 `name` 是否与注册名完全一致。
- 出现 `Internal error: ...`：
  先检查 Resource / Prompt handler 内部是否抛出异常，再检查传入参数 JSON
  是否满足业务假设。
- `initialize` 阶段失败：
  优先核对客户端传入的 `protocolVersion` 与请求头 `MCP-Protocol-Version`；
  `initialize` 会协商回退到服务端支持版本，而非直接抛出 `kInvalidParams`。

SSE/HTTP 场景建议额外检查：

- 首次连接时确认响应头里是否返回 `X-Session-Id`，以及握手响应体是否包含
  `status`、`server`、`version`、`protocol` 等字段。
- 当前实现已经关闭后续请求中的 session 校验，所以如果问题表现为“拿到了
  session id 但后续请求仍异常”，通常应先回到方法名、参数和 handler 本身排查，
  不要默认归因到 session 失效。
- 如果客户端是按 MCP over HTTP 更严格的标准实现接入，需要注意：当前公开层
  未提供完整的授权协商机制，且 `limit` 为实现扩展参数（非 MCP 标准字段）。

建议的最小排查顺序：

1. 调用 `config.validate()` 并打印关键配置。
2. 确认 `start()` 返回 `true`，必要时检查传输层日志。
3. 用 `tools/list` / `resources/list` / `prompts/list` 先确认注册表快照是否符合预期。
4. 对单个失败请求，先看返回 JSON 是 `Unknown method`、`Unknown tool` 还是
   `Internal error`。
5. 如果是业务回调异常，再检查对应 handler 的参数校验、异常处理和线程安全。
