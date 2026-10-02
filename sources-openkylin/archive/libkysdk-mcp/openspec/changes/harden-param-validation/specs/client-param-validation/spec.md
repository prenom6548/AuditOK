## ADDED Requirements

### Requirement: connect 接口拒绝非法 endpoint

`MCPClient::connect(const std::string& endpoint)` SHALL 在解析阶段拒绝以下非法输入，统一抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息携带具体非法值。

支持的 endpoint 格式：

- `cmd:<command> [args...]`：IPC 模式，启动子进程。
- `http://<host>:<port>/<path>`、`https://<host>:<port>/<path>`：SSE 模式，连接远端服务。

非法输入及其处理：

#### Scenario: 端口越界

- **WHEN** 调用方传入 `"http://127.0.0.1:99999/mcp"`
- **THEN** `connect` MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Invalid port number"`，且 MUST NOT 抛出任何非 `mcp_exception` 类型的异常（特别是 `std::out_of_range`）。

#### Scenario: 端口为非数字

- **WHEN** 调用方传入 `"http://127.0.0.1:abc/mcp"`
- **THEN** `connect` MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Invalid port number"`。

#### Scenario: 未知 scheme

- **WHEN** 调用方传入既不以 `cmd:` 也不以 `http://` / `https://` 开头的字符串（如 `"invalid://path"` 或 `"ftp://host/file"`）
- **THEN** `connect` MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Unsupported endpoint scheme"`，并且 MUST NOT 尝试将该字符串作为 IPC 命令执行。

#### Scenario: 空字符串

- **WHEN** 调用方传入 `""`
- **THEN** `connect` MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Empty endpoint"`（保持现有行为，纳入规范）。

#### Scenario: cmd: 前缀无命令

- **WHEN** 调用方传入 `"cmd:"` 或 `"cmd:   "`（仅空白）
- **THEN** `connect` MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Empty IPC command"`（保持现有行为，纳入规范）。

#### Scenario: 合法 IPC endpoint

- **WHEN** 调用方传入 `"cmd:./server --ipc"`
- **THEN** `connect` MUST 成功返回 `true`，`isConnected()` 返回 `true`。

#### Scenario: 合法 HTTP endpoint

- **WHEN** 调用方传入 `"http://127.0.0.1:18080/mcp"` 且服务端在运行
- **THEN** `connect` MUST 成功返回 `true`，`isConnected()` 返回 `true`。

### Requirement: setClientInfo 校验 name / version 非空

`MCPClient::setClientInfo(const std::string& name, const std::string& version)` SHALL 拒绝空字符串入参，抛出 `mcp_exception(ErrorCode::kInvalidParams)`。

#### Scenario: name 为空

- **WHEN** 调用方传入 `setClientInfo("", "1.0.0")`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Client name must be non-empty"`。

#### Scenario: version 为空

- **WHEN** 调用方传入 `setClientInfo("my-client", "")`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Client version must be non-empty"`。

#### Scenario: name 与 version 都为空

- **WHEN** 调用方传入 `setClientInfo("", "")`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`（优先报 name 错误，与文档保持一致）。

#### Scenario: 正常调用

- **WHEN** 调用方传入 `setClientInfo("my-client", "1.0.0")`
- **THEN** 接口 MUST 成功返回，后续 `initialize` 上行的 `clientInfo.name == "my-client"`、`clientInfo.version == "1.0.0"`。
