## Why

26s-kart 模块思维导图测试用例梳理过程中，发现 MCP C++ SDK 在「客户端/服务端公开接口的参数校验」上缺少统一原则，留下了 8 处「待研发确认」的空白，其中：

1. `MCPClient::connect("http://host:99999/...")` 端口越界时抛出未捕获的 `std::out_of_range`，跳出了 SDK 的 `mcp_exception` 体系——**调用方按文档 `catch(mcp_exception&)` 会漏接，进程崩溃**，属于真 BUG。
2. `MCPClient::connect("invalid://path")` 静默 fallback 到 IPC 命令解析，行为不符合直觉。
3. `MCPClient::setClientInfo` / `MCPServer::setServerInfo` 对空 `name`/`version` 不做校验，空字符串透传到协议层，污染对端的日志、追踪与限流元数据。
4. `MCPServer::subscribeResource` 对「未注册 URI」静默接受插入订阅 set，导致典型的「以为订阅成功、永远收不到通知」沉默失败，且与「空 URI 抛异常」的处理粒度倒置。

这些不一致使得调用方需要逐个接口读源码才能知道该接什么异常，违反 AGENTS.md「统一错误语义、错误码/异常/状态对象不可混乱叠加」的原则。本次以最小必要改动收紧这 4 处校验。

## What Changes

- **修复**：`MCPClient::connect` 内部 `parse_endpoint` 包裹 `std::stoi` 异常，端口非法统一抛 `mcp_exception(kInvalidParams, "Invalid port number")`。**BUG 修复**。
- **BREAKING**：`MCPClient::connect` 对无法识别 scheme 的 endpoint（既不是 `cmd:` 也不是 `http(s)://`）改为抛 `mcp_exception(kInvalidParams, "Unsupported endpoint scheme")`，不再静默 fallback 到 IPC 命令解析。
- **BREAKING**：`MCPClient::setClientInfo(name, version)` 任一参数为空字符串时抛 `mcp_exception(kInvalidParams, "Client name/version must be non-empty")`。
- **BREAKING**：`MCPServer::setServerInfo(name, version)` 与客户端对齐，空 name/version 抛 `mcp_exception(kInvalidParams, "Server name/version must be non-empty")`。
- **BREAKING**：`MCPServer::subscribeResource(uri)` / `unsubscribeResource(uri)` 校验 URI 已在 `registered_resources_` 中，未注册抛 `mcp_exception(kInvalidParams, "Resource URI not registered: <uri>")`。
- **新增测试**：覆盖上述每条新增校验的正例 + 反例。
- **CHANGELOG**：记录 4 项 breaking change 以及配套迁移建议。

## Capabilities

### New Capabilities

- `client-param-validation`: `MCPClient` 公开接口（`connect` / `setClientInfo`）的参数校验契约，统一异常类型与错误码，固化 endpoint 解析规则。
- `server-param-validation`: `MCPServer` 公开接口（`setServerInfo` / `subscribeResource` / `unsubscribeResource`）的参数校验契约，订阅类接口校验 URI 已注册。

### Modified Capabilities

（无现有 spec；本变更同时创建上述新 capabilities，对应行为约束在 specs/ 下首次落地。）

## Impact

- **代码**：
  - `src/client/client/client.cpp`：`parse_endpoint`（端口/scheme 校验）、`setClientInfo`（空串校验）。
  - `src/server/server.cpp`：`setServerInfo`（空串校验）、`subscribeResource` / `unsubscribeResource`（URI 已注册校验）。
  - `include/client/client.h` / `include/server/server.h`：接口注释补充「Throws」描述。
- **公开 API/ABI**：签名不变，但行为变更属于 BREAKING——之前接受的非法输入将开始抛 `mcp_exception(kInvalidParams)`。
- **测试**：`tests/` 下新增/更新单元测试覆盖每条校验的正例 + 反例；若有依赖旧「宽松接受」行为的测试需同步调整。
- **文档**：`CHANGELOG.md` 新增 breaking change 条目；接口文档（doxygen 注释）补充 throws 说明。
- **风险**：
  - 依赖「未注册 URI 静默接受」行为的下游用户会立刻看到异常；通过 release note 显著提示并给出最小迁移示例（先 `registerResource` 再 `subscribeResource`）可缓解。
  - `connect` 不再 fallback 到 IPC 命令解析，可能影响极少数把命令路径写成 `xxx://yyy` 形式的用法（罕见）。
- **依赖**：无新增第三方依赖。
