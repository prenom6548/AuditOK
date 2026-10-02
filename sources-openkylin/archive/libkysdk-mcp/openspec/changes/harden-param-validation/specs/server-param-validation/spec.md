## ADDED Requirements

### Requirement: setServerInfo 校验 name / version 非空

`MCPServer::setServerInfo(const std::string& name, const std::string& version)` SHALL 拒绝空字符串入参，抛出 `mcp_exception(ErrorCode::kInvalidParams)`。

#### Scenario: name 为空

- **WHEN** 调用方传入 `setServerInfo("", "1.0.0")`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Server name must be non-empty"`。

#### Scenario: version 为空

- **WHEN** 调用方传入 `setServerInfo("my-server", "")`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Server version must be non-empty"`。

#### Scenario: 正常调用

- **WHEN** 调用方传入 `setServerInfo("my-server", "1.0.0")`
- **THEN** 接口 MUST 成功返回，客户端 `initialize` 结果中 `serverInfo.name == "my-server"`、`serverInfo.version == "1.0.0"`。

### Requirement: subscribeResource 校验 URI 已注册

`MCPServer::subscribeResource(const std::string& uri)` SHALL 校验 `uri` 已存在于 `registered_resources_`，未注册抛出 `mcp_exception(ErrorCode::kInvalidParams)`。空 URI 处理保持不变（仍抛 `mcp_exception`）。

#### Scenario: 未注册 URI

- **WHEN** 调用方对从未通过 `registerResource` 注册过的 URI（如 `"unknown://res"`）调用 `subscribeResource`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Resource URI not registered"` 与 URI 原文。

#### Scenario: 空 URI

- **WHEN** 调用方传入 `subscribeResource("")`
- **THEN** 接口 MUST 抛出 `mcp_exception`，错误消息 MUST 包含 `"requires field: uri"`（保持现有行为，纳入规范）。

#### Scenario: 已注册 URI 首次订阅

- **WHEN** 调用方先 `registerResource(res)`（`res.uri = "preflight://r1"`），再 `subscribeResource("preflight://r1")`
- **THEN** 接口 MUST 成功返回，订阅集合包含 `"preflight://r1"`。

#### Scenario: 已注册 URI 多次订阅（幂等）

- **WHEN** 调用方对同一 URI 连续调用 `subscribeResource` 多次（前提该 URI 已注册）
- **THEN** 接口 MUST 全部成功返回，不抛异常；订阅集合中该 URI 仍只保留一份。

### Requirement: unsubscribeResource 校验 URI 已注册

`MCPServer::unsubscribeResource(const std::string& uri)` SHALL 校验 `uri` 已存在于 `registered_resources_`，未注册抛出 `mcp_exception(ErrorCode::kInvalidParams)`；对「已注册但未订阅过」的 URI 保持幂等（不抛错）。

#### Scenario: 未注册 URI

- **WHEN** 调用方对从未通过 `registerResource` 注册过的 URI（如 `"unknown://res"`）调用 `unsubscribeResource`
- **THEN** 接口 MUST 抛出 `mcp_exception(ErrorCode::kInvalidParams)`，错误消息 MUST 包含 `"Resource URI not registered"` 与 URI 原文。

#### Scenario: 空 URI

- **WHEN** 调用方传入 `unsubscribeResource("")`
- **THEN** 接口 MUST 抛出 `mcp_exception`，错误消息 MUST 包含 `"requires field: uri"`（保持现有行为，纳入规范）。

#### Scenario: 已注册但未订阅过的 URI

- **WHEN** 调用方先 `registerResource(res)`，但 **未** 调用过 `subscribeResource(res.uri)`，直接调用 `unsubscribeResource(res.uri)`
- **THEN** 接口 MUST 成功返回（幂等），不抛异常。

#### Scenario: 已订阅 URI 取消订阅

- **WHEN** 调用方先 `registerResource(res)` + `subscribeResource(res.uri)`，再调用 `unsubscribeResource(res.uri)`
- **THEN** 接口 MUST 成功返回，订阅集合不再包含该 URI。

#### Scenario: 多次取消订阅同一 URI

- **WHEN** 调用方对同一已注册 URI 连续调用 `unsubscribeResource` 多次
- **THEN** 接口 MUST 全部成功返回，不抛异常（幂等）。
