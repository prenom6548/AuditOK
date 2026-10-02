## Context

26s-kart 模块思维导图测试用例梳理过程中识别出 4 处客户端/服务端参数校验缺口（详见 `proposal.md`）。当前 SDK 内部错误模型已统一为 `mcp_exception(ErrorCode)`，但部分公开接口的边界校验存在三类问题：

1. **异常类型逃逸**：`parse_endpoint` 中 `std::stoi(port_str)` 未捕获，端口越界直接抛出 `std::out_of_range`，跳出 SDK 的 `mcp_exception` 体系（`src/client/client/client.cpp:1075`）。
2. **静默接受非法输入**：`setClientInfo` / `setServerInfo` 对空字符串、`subscribeResource` 对未注册 URI 都直接接受，缺陷被推迟到协议层或彻底吞掉。
3. **同一接口处理粒度倒置**：`subscribeResource` 对「空 URI」会抛异常，但对「未注册 URI」反而静默接受——两个都是「非法输入」，前者更具体的反而处理更弱。

参与方：SDK 维护方（行为定义）、思维导图测试编写人员（断言依据）、下游业务接入方（异常处理路径）。

约束：
- AGENTS.md 3.2：「统一错误语义：错误码、异常、状态对象不可混乱叠加」。
- AGENTS.md 3.1.1：「未经明确要求，不得破坏公开 API/ABI」——本变更不修改签名，但行为语义变更属于 BREAKING，需在 CHANGELOG 显式标注。
- 已存在的同类校验风格：`MCPClient::initialize` 对非法 `protocolVersion` 抛 `mcp_exception(kInvalidParams)`（`src/client/client/client.cpp:313-316`）；`subscribeResource` 对空 URI 抛 `mcp_exception("resources/subscribe requires field: uri")`（`src/server/server.cpp:383-402`）。本变更与之对齐，不引入新的错误处理模式。

## Goals / Non-Goals

**Goals:**

- 让 4 处缺口都遵循「统一以 `mcp_exception(kInvalidParams)` 在接口边界拒绝」的语义。
- 修复 `connect` 端口越界的真 BUG，使所有异常都落在 `mcp_exception` 体系内。
- 给 `subscribeResource` / `unsubscribeResource` 提供「订阅了不存在的资源」的快速失败信号，消除沉默失败。
- 接口签名保持向前兼容（仅行为语义收紧）。
- 每条新校验都有正例 + 反例单元测试覆盖。

**Non-Goals:**

- 不重新设计 SDK 的错误模型（不引入 `expected<T,E>` / 状态对象等）。
- 不变更 `MCPClient::initialize` 的协议版本白名单（已合理）。
- 不调整扩展协议（setLoggingLevel / tasks/*）的校验——服务端 dispatch 已统一抛 `mcp_exception`（`src/server/server_dispatch.h:54-108` / `src/server/server.cpp:405,421,481`）。
- 不增加 `strictSubscribe` 等配置开关——保持默认严格行为，避免实现复杂度。
- 不修改 HTTPS 服务端缺失等正交问题（另立提案）。

## Decisions

### Decision 1：用 `mcp_exception(kInvalidParams)` 而非返回 bool / 状态对象

**选择**：所有 4 处校验失败统一抛 `mcp_exception(ErrorCode::kInvalidParams, "<message>")`。

**理由**：
- 与现有 `initialize` / 服务端 dispatch 的异常类型完全一致，调用方可以用单一 `catch(mcp_exception&)` 收拢所有边界错误。
- `connect` / `setClientInfo` / `setServerInfo` 当前的返回签名（`bool` / `void`）若改为 error-returning 会破坏 ABI；用异常完全兼容现有调用模板。
- 错误码 `kInvalidParams` 对应 JSON-RPC `-32602`，跨边界传播时语义清晰。

**Alternatives considered**：
- 返回 bool + last-error：与现有 SDK 风格不符，且 `void` 接口（setClientInfo）需要重新设计签名。
- 仅写日志不抛异常：违反「沉默失败是头号 BUG」原则，本提案核心动机就是消除沉默失败。

### Decision 2：`connect("invalid://path")` 不再 fallback 到 IPC 命令解析

**选择**：未知 scheme（既不是 `cmd:` 也不是 `http(s)://`）抛 `mcp_exception(kInvalidParams, "Unsupported endpoint scheme")`，不再尝试当作命令路径。

**理由**：
- 用户写出 `xxx://yyy` 形式时几乎一定是想用 URL 语法，fallback 到 IPC 会执行一个奇怪的「命令」，错得更隐蔽。
- 当前 IPC fallback 路径要求用户写 `cmd:./server --args` 或裸命令；保留这两种官方写法不受影响。
- 错误消息明确列出 supported schemes，引导用户改用 `cmd:` 前缀。

**Alternatives considered**：
- 保留 fallback 仅加 warning 日志：调用方仍可能错过 warning，沉默失败问题不变。

### Decision 3：`subscribeResource` 在服务端做「已注册」校验，而非客户端

**选择**：校验放在 `MCPServer::subscribeResource` / `unsubscribeResource`，客户端 `MCPClient::subscribeResource` 不做本地校验（保持薄绑定）。

**理由**：
- 客户端无法本地知道服务端注册了哪些资源（资源是服务端状态），强行做客户端缓存会引入新的一致性问题。
- 服务端是唯一拥有 `registered_resources_` 真相的位置，校验最终也只能在这里。
- 客户端透传 URI 与服务端校验失败后通过 JSON-RPC error 返回——`MCPClient` 的 `subscribeResource` 调用 RPC，会自然抛出对应的 `mcp_exception`。

**Alternatives considered**：
- 双侧都校验：客户端校验需要额外暴露资源列表查询，复杂度不值。

### Decision 4：`unsubscribeResource` 对「已注册但未订阅过」的 URI 是否抛错？

**选择**：**不抛错**，幂等接受（与思维导图「多次取消订阅 → 无异常，取消订阅成功」预期一致）。仅对「URI 未注册」抛错。

**理由**：
- 「资源存在，只是没订阅过」是合法状态，类似 Linux `signal(handler) + ignore = noop`。
- 思维导图明确「多次取消订阅 → 取消订阅成功」期望幂等，不能改。
- 「资源完全不存在」才是调用方传错 URI 的 BUG 信号，需要抛。

### Decision 5：错误消息携带哪些信息

- `connect`：`"Invalid port number: <raw_port_str>"`、`"Unsupported endpoint scheme: <scheme>"`。
- `setClientInfo` / `setServerInfo`：`"Client name must be non-empty"` / `"Client version must be non-empty"`（分两条消息便于定位是哪个字段）。
- `subscribe` / `unsubscribe`：`"Resource URI not registered: <uri>"`。

**理由**：日志/调试时能直接定位是哪个值非法；不泄露内部状态（注册资源列表）。

## Risks / Trade-offs

| 风险 | 缓解 |
|---|---|
| 下游用户依赖「未注册 URI 静默接受」行为，升级后立即看到异常 | (1) `CHANGELOG.md` BREAKING CHANGES 段明确列出 4 项；(2) 提供最小迁移示例（先 `registerResource` 再 `subscribeResource`）；(3) 异常消息携带 URI，便于排查 |
| 用户依赖 `connect("invalid://path")` 的 IPC fallback | 极罕见——这种写法本来就是「碰巧能跑」。CHANGELOG 注明 supported schemes |
| `setClientInfo("", "")` 在某些测试 stub 中被使用 | 在本提案的 tasks 阶段全量扫描 `tests/` 下相关用例并同步更新 |
| 引入新异常路径可能未被 RAII 资源回收 | 这 4 处都在「公开接口入口」抛出，函数尚未持有资源；逐项 review 确认无悬挂资源 |

## Migration Plan

1. 升级到含本变更的版本后：
   - 若收到 `mcp_exception(kInvalidParams, "Resource URI not registered: ...")`：检查调用 `MCPServer::registerResource` 是否在 `subscribeResource` 之前完成。
   - 若收到 `"Invalid port number"` / `"Unsupported endpoint scheme"`：检查 endpoint 字符串格式，确认使用 `cmd:<command>` 或 `http(s)://host:port/path`。
   - 若收到 `"Client/Server name/version must be non-empty"`：补齐 `setClientInfo` / `setServerInfo` 调用的实参。
2. 回滚策略：本变更纯代码改动，无配置/数据迁移，回滚只需 revert commit。
