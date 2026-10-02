## 1. 准备与基线

- [x] 1.1 阅读 `src/client/client/client.cpp` 中 `parse_endpoint`（约第 1000-1110 行）与 `setClientInfo`（约第 700-705 行）实现，确认改动落点。
- [x] 1.2 阅读 `src/server/server.cpp` 中 `setServerInfo`（约第 609-614 行）与 `subscribeResource` / `unsubscribeResource`（约第 383-440 行）实现，确认改动落点与 `registered_resources_` 锁/容器类型。
- [x] 1.3 跑一次基线全量测试 `cd build && ctest --output-on-failure -j$(nproc)`，记录当前通过 / 失败用例，确保后续回归对比有依据。
- [x] 1.4 全局 `rg "stoi\(.*port" src/` 与 `rg "parse_endpoint" src/`，确认除 `client.cpp:1075` 外没有其他需要同步修补的端口解析点。

## 2. 客户端 connect 校验（client-param-validation 之 connect 部分）

- [x] 2.1 在 `src/client/client/client.cpp::parse_endpoint` 中用 `try { std::stoi(port_str); } catch (const std::exception&) { throw mcp_exception(ErrorCode::kInvalidParams, "Invalid port number: " + port_str); }` 包裹端口解析；附加端口范围检查（`port < 1 || port > 65535` 同样抛 `kInvalidParams`）。
- [x] 2.2 在 `parse_endpoint` 末尾移除 IPC fallback 分支，对既不是 `cmd:` 也不是 `http(s)://` 的输入抛 `mcp_exception(ErrorCode::kInvalidParams, "Unsupported endpoint scheme: " + scheme)`；保留对裸命令的合法路径（如有现存场景需评估）。
- [x] 2.3 校对 `parse_endpoint` 现有的 `Empty endpoint` 与 `Empty IPC command in endpoint` 路径，确保异常类型已是 `mcp_exception(kInvalidParams)`（不是的就改齐）。
- [x] 2.4 在 `include/client/client.h::connect` 接口注释处补充 `@throws mcp_exception(kInvalidParams)` 的四种触发条件（port 非法 / 未知 scheme / 空 endpoint / cmd 空命令）。

## 3. 客户端 setClientInfo 校验

- [x] 3.1 在 `src/client/client/client.cpp::setClientInfo` 函数入口加入：`if (name.empty()) throw mcp_exception(ErrorCode::kInvalidParams, "Client name must be non-empty");` 与对应的 version 检查；先 name 后 version 顺序与 spec 一致。
- [x] 3.2 在 `include/client/client.h::setClientInfo` 接口注释处补充 `@throws mcp_exception(kInvalidParams)` 说明。

## 4. 服务端 setServerInfo 校验

- [x] 4.1 在 `src/server/server.cpp::setServerInfo` 函数入口加入与客户端对称的空字符串校验，错误消息使用 `"Server name must be non-empty"` / `"Server version must be non-empty"`。
- [x] 4.2 在 `include/server/server.h::setServerInfo` 接口注释处补充 `@throws mcp_exception(kInvalidParams)` 说明。

## 5. 服务端 subscribeResource / unsubscribeResource URI 校验

- [x] 5.1 在 `src/server/server.cpp::subscribeResource` 当前的「空 URI 抛错」后，新增「URI 不在 `registered_resources_` 中则抛 `mcp_exception(kInvalidParams, "Resource URI not registered: " + uri)`」检查；保持已有「重复订阅幂等」行为不变。
- [x] 5.2 在 `src/server/server.cpp::unsubscribeResource` 做对称修改：未注册 URI 抛错；已注册但未订阅过的 URI 保持幂等返回；多次取消订阅同一 URI 保持幂等。
- [x] 5.3 确认对 `registered_resources_` 的读取在已有同步策略下安全（与 `registerResource` / `unregisterResource` 的锁一致）；若现有未加锁，新增的查询调用 MUST 与之走同一锁，不引入额外锁。
- [x] 5.4 在 `include/server/server.h::subscribeResource` 与 `unsubscribeResource` 接口注释处补充 `@throws mcp_exception(kInvalidParams)` 说明。

## 6. 单元测试 — 客户端

- [x] 6.1 在 `tests/client/client_gtest.cpp` 新增 `TEST(MCPClientConnect, RejectsPortOutOfRange)`：`connect("http://127.0.0.1:99999/mcp")` 期望抛 `mcp_exception` 且 `code() == kInvalidParams`、`what()` 含 `"Invalid port number"`；同时断言 **不抛 `std::out_of_range`**（用 `EXPECT_THROW` + 第二个 try-catch 验证类型）。
- [x] 6.2 在 `tests/client/client_gtest.cpp` 新增 `TEST(MCPClientConnect, RejectsNonNumericPort)`：`connect("http://127.0.0.1:abc/mcp")`。
- [x] 6.3 在 `tests/client/client_gtest.cpp` 新增 `TEST(MCPClientConnect, RejectsUnsupportedScheme)`：`connect("invalid://path")` 与 `connect("ftp://host/file")` 均期望抛 `mcp_exception(kInvalidParams)`，且 **不会** 启动任何子进程（用前置/后置 `ps` 或 mock 验证）。
- [x] 6.4 在 `tests/client/client_gtest.cpp` 新增 `TEST(MCPClientSetClientInfo, RejectsEmptyName)` / `TEST(MCPClientSetClientInfo, RejectsEmptyVersion)` / `TEST(MCPClientSetClientInfo, RejectsBothEmpty)`，分别断言抛 `mcp_exception(kInvalidParams)` 与对应错误消息子串。
- [x] 6.5 在 `tests/client/client_gtest.cpp` 新增 `TEST(MCPClientSetClientInfo, AcceptsValid)`：传入合法值后 `getClientInfo()`（如可访问）返回相同值，或在端到端测试中验证 server 收到的 `clientInfo` 字段。

## 7. 单元测试 — 服务端

- [x] 7.1 在 `tests/server/server_gtest.cpp` 新增 `TEST(MCPServerSetServerInfo, RejectsEmptyName)` / `RejectsEmptyVersion`。
- [x] 7.2 在 `tests/server/server_gtest.cpp` 新增 `TEST(MCPServerSetServerInfo, AcceptsValid)`：传入合法值后 `getServerInfo()` 返回相同值。
- [x] 7.3 在 `tests/server/server_gtest.cpp` 新增 `TEST(MCPServerSubscribe, RejectsUnregisteredUri)`：未先调用 `registerResource` 直接 `subscribeResource("unknown://res")`，期望 `mcp_exception(kInvalidParams)`，消息含 URI 原文。
- [x] 7.4 在 `tests/server/server_gtest.cpp` 新增 `TEST(MCPServerSubscribe, AcceptsRegisteredUri)` 与 `TEST(MCPServerSubscribe, IsIdempotentForRegisteredUri)`。
- [x] 7.5 在 `tests/server/server_gtest.cpp` 新增 `TEST(MCPServerUnsubscribe, RejectsUnregisteredUri)` / `TEST(MCPServerUnsubscribe, IsIdempotentForRegisteredUnsubscribed)` / `TEST(MCPServerUnsubscribe, IsIdempotentMultipleCalls)`。

## 8. 回归与既有测试同步

- [x] 8.1 全量 `rg -n "setClientInfo\(\"\"|setClientInfo\(.*,.*\"\"\)" tests/`、`rg -n "setServerInfo\(\"\"|setServerInfo\(.*,.*\"\"\)" tests/`、`rg -n "subscribeResource\(" tests/`，找出依赖旧「宽松接受」行为的测试用例并改成 `EXPECT_THROW` 或先 `registerResource`。
- [x] 8.2 全量 `rg -n "connect\(\"invalid|connect\(.*:99999" tests/ examples/`，若有依赖旧 fallback 行为的样例，同步更新或注明 deprecated。
- [x] 8.3 跑 `cd build && cmake --build . -j$(nproc)`，构建必须通过且无新增 warning。
- [x] 8.4 跑 `cd build && ctest --output-on-failure -j$(nproc)`，全部用例必须通过；若有失败用例，逐一确认是预期 breaking 还是实现 BUG。

## 9. 文档与发布说明

- [x] 9.1 在 `CHANGELOG.md` 顶部新增 `## Unreleased` → `### BREAKING CHANGES` 段，列出 4 项行为变更（端口越界 / 未知 scheme / setClientInfo & setServerInfo 非空 / subscribe URI 已注册），每项附最小迁移示例。
- [x] 9.2 同段下新增 `### Fixed` 子段，写明「`connect` 端口越界不再抛 `std::out_of_range`」BUG 修复。
- [x] 9.3 若 `docs/` 下有「接口参数校验」或「错误码」相关文档，同步更新；如无则不新增（最小改动原则）。
- [x] 9.4 检查 `examples/` 下示例代码是否触发新校验（特别是 setClientInfo / setServerInfo 调用），如有则补全实参。

## 10. 收尾

- [ ] 10.1 自检 `openspec status --change harden-param-validation` 全部 `done`，所有 tasks 复选框勾选。
- [x] 10.2 准备 commit message：标题遵循仓库 commit 规范（`[BUG]` 用于端口越界修复、`[需求]` 或 `[任务]` 用于其余校验加固），正文覆盖问题描述 / 解决方案 / 自测内容 / 影响域分析 / 单元测试 / 补充说明。
- [ ] 10.3 提交后等待用户确认是否 `openspec archive harden-param-validation` 归档。
