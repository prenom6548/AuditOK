# Changelog

All notable changes to this project are documented in this file.

The format follows Keep a Changelog, and this project uses Semantic Versioning.

## [Unreleased] - 2026-04-06

### Added
- Added root `LICENSE` file and clarified project license as GPL-3.0-or-later.
- Added initial project changelog for release tracking.
- Added transport auto-reconnect support for IPC and SSE:
  - `auto_reconnect`, `reconnect_interval`, `max_reconnect_attempts`, and
    `max_reconnect_interval` now drive the background reconnect loop.
  - IPC can respawn the configured child process when
    `ipc.respawn_on_disconnect=true`.
  - SSE can use optional `sse.keepalive_interval` heartbeat detection.
- Added client session recovery after reconnect:
  - `MCPClient::reconnect()` explicitly rebuilds the transport connection.
  - `MCPClient::setReconnectHandler()` allows custom recovery callbacks.
  - The default handler replays `initialize` and tracked resource
    subscriptions.

### BREAKING CHANGES
- `MCPClient::connect("http://host:99999/path")` now rejects ports outside
  `1..65535` with `mcp_exception(ErrorCode::kInvalidParams, ...)`.
  Use a valid TCP port, for example
  `client.connect("http://127.0.0.1:18080/mcp")`.
- `MCPClient::connect()` no longer treats endpoint strings containing an
  unknown `://` scheme as IPC commands. Use `cmd:<command ...>` for IPC or
  `http(s)://host[:port][/path]` for SSE.
- `MCPClient::setClientInfo()` now requires non-empty `name` and `version`
  values. Set both fields before initialize, for example
  `client.setClientInfo("my-client", "1.0.0")`.
- `MCPServer::setServerInfo()` now requires non-empty `name` and `version`
  values. Set both fields before `start()`, for example
  `server.setServerInfo("my-server", "1.0.0")`.
- `MCPServer::subscribeResource()` and `unsubscribeResource()` now require the
  URI to be registered first. Call `registerResource(resource, handler)` before
  subscribing to `resource.uri`.

### Changed
- `TransportConfig::auto_reconnect` and `reconnect_interval` changed from
  serialized no-op fields to effective runtime behavior. The default remains
  disabled, so legacy configurations that do not opt in keep existing behavior.
- Replaced placeholder `README.en.md` with a complete English guide.
- Clarified Debian package naming semantics while keeping aggregate package names:
  `libkysdk-mcp` runtime contains both client/server shared libraries.
- Added explicit thread-safety notes to public headers:
  - `MCPClient` callback-thread and shared-state expectations.
  - `MCPServer` handler-thread and reentrancy expectations.
- Updated transport behavior for server mode SSE protocol:
  - `TransportConfig::validate()` now rejects `mode == kServer && sse.protocol == kHttps`.
  - `Transport::start()` now returns failure with explicit error when server mode requests HTTPS listen.
- Documented public JSON dependency policy (`nlohmann::json` / installed `json.hpp`).

### Removed
- Removed legacy client-initiated APIs:
  - `MCPClient::createElicitation(const json&)`
  - `MCPClient::createMessage(const json&)`
- Removed `examples/server/filesystem_server.cpp` from `kysdk_mcp_server` library source list.

### Fixed
- `MCPClient::connect()` no longer lets out-of-range HTTP(S) ports escape as
  `std::out_of_range`; invalid ports are reported as `mcp_exception` with
  `ErrorCode::kInvalidParams`.
- Replaced placeholder copyright text in `debian/copyright`.
- Aligned server API documentation with current HTTPS server limitations and startup behavior.
- Root-fixed IPC timeout lifecycle race in `PipeCommunication`:
  - Replaced detached timeout worker with managed timeout threads.
  - Added stop-time cancellation/notification and thread join for timeout workers.
  - Prevented teardown-time `std::system_error` aborts caused by stale mutex access.
- Unified IPC request timeout handling in transport layer to use configured
  `ipc.read_timeout_ms` instead of hard-coded timeout behavior.
- Hardened logger test isolation to eliminate intermittent `ModuleLogger` test flakiness
  (explicit test file cleanup and stable log-level setup).
- Root-fixed SSE resource-change notification race in server mode:
  - `notifyResourcesChanged()` now retries `notifications/resources/list_changed`
    once when the first emit races SSE stream readiness but subsequent
    `notifications/resources/updated` emission already succeeded.
  - Prevents partial notification delivery (`updated` without `list_changed`)
    under connection-warmup timing.

## Versioning Policy

- Patch version: bug fixes and internal changes with no public API break.
- Minor version: backward-compatible API additions or behavior extensions.
- Major version: any public API or behavior breaking change.
