## ADDED Requirements

### Requirement: Explicit reconnect API
`MCPClient` SHALL expose a public `reconnect()` method that the caller can invoke from any client state. The method SHALL re-establish the underlying transport connection and then perform session recovery via the registered handler.

#### Scenario: Reconnect from connected state
- **WHEN** the client is currently in `kConnected` state
- **AND** the caller invokes `MCPClient::reconnect()`
- **THEN** the client SHALL first disconnect the existing transport
- **AND** then establish a new transport connection using the previously supplied `TransportConfig`
- **AND** then trigger session recovery via the registered handler
- **AND** the call SHALL return true if both transport reconnect and session recovery succeed; false otherwise

#### Scenario: Reconnect from disconnected state
- **WHEN** the client is in `kDisconnected` or `kError` state
- **AND** the caller invokes `MCPClient::reconnect()`
- **THEN** the client SHALL establish a new transport connection without attempting to disconnect first
- **AND** then trigger session recovery via the registered handler

#### Scenario: Reconnect is idempotent
- **WHEN** the caller invokes `reconnect()` more than once in quick succession
- **THEN** only one reconnect operation SHALL be in flight at any time
- **AND** the second invocation SHALL block until the first one completes
- **AND** the return value SHALL reflect the outcome of the second invocation's own reconnect attempt (not the cached result from the first)

### Requirement: Reconnect status callback
`MCPClient` SHALL allow callers to register a callback via `setReconnectHandler(handler)` that is invoked whenever the transport completes a reconnection attempt — whether triggered by the automatic background loop or by an explicit `reconnect()` call.

#### Scenario: Handler invoked on successful reconnect
- **WHEN** the transport completes a successful reconnect (auto or explicit)
- **THEN** the registered handler SHALL be invoked exactly once with a result indicating success and the post-reconnect transport status
- **AND** the handler SHALL be invoked on a thread distinct from the caller's main thread (so the caller can post UI updates without re-entrancy)

#### Scenario: Handler invoked on reconnect failure
- **WHEN** the automatic background loop exhausts its reconnection budget without success
- **THEN** the registered handler SHALL be invoked exactly once with a result indicating failure and a reason string

#### Scenario: Default handler behavior
- **WHEN** the caller has not registered a custom handler
- **AND** the transport completes a reconnect
- **THEN** the default handler SHALL execute the steps defined in "Automatic session recovery"

#### Scenario: Custom handler replaces default
- **WHEN** the caller registers a custom handler via `setReconnectHandler`
- **AND** the transport completes a reconnect
- **THEN** the default session recovery SHALL be skipped
- **AND** only the custom handler SHALL run
- **AND** the custom handler MAY decide whether to invoke `client.initialize()`, `client.subscribeResource()`, or any other recovery steps

### Requirement: Automatic session recovery
The default reconnect handler SHALL restore the session state that existed at the time of disconnect, so that protocol-level invariants required by MCP (initialized session, active subscriptions) hold after reconnect.

#### Scenario: Session re-initialized after reconnect
- **WHEN** the client successfully reconnects
- **AND** the default reconnect handler runs
- **THEN** the handler SHALL invoke `initialize` with the same `InitializeParams` (protocolVersion, clientInfo, capabilities) that the client used on the original `initialize` call
- **AND** the handler SHALL forward the returned `InitializeResult` so the client's cached `serverInfo` / `serverCapabilities` are refreshed

#### Scenario: Subscriptions restored after reconnect
- **WHEN** the client had subscribed to one or more resources before disconnect
- **AND** the default reconnect handler runs after a successful re-initialize
- **THEN** the handler SHALL invoke `subscribeResource(uri)` for every URI in the client's tracked subscription set
- **AND** the order of re-subscription SHALL be the same as the original subscription order

#### Scenario: Initialize failure aborts recovery
- **WHEN** the default handler's `initialize` call fails (throws `mcp_exception` or returns an error result)
- **THEN** the handler SHALL NOT attempt to restore subscriptions
- **AND** the handler SHALL report failure back to the registered `setReconnectHandler` callback
- **AND** the client SHALL remain in a state where subsequent requests fail until the caller takes corrective action

### Requirement: Tracked subscription set
`MCPClient` SHALL maintain an internal record of currently subscribed resource URIs so the default reconnect handler can replay them. The record SHALL stay in sync with `subscribeResource` / `unsubscribeResource` calls.

#### Scenario: subscribeResource adds to the tracked set
- **WHEN** the caller successfully invokes `subscribeResource(uri)`
- **THEN** the URI SHALL be added to the tracked subscription set

#### Scenario: unsubscribeResource removes from the tracked set
- **WHEN** the caller successfully invokes `unsubscribeResource(uri)`
- **THEN** the URI SHALL be removed from the tracked subscription set

#### Scenario: Failed subscribe does not affect the tracked set
- **WHEN** `subscribeResource(uri)` throws an exception or returns an error
- **THEN** the URI SHALL NOT be present in the tracked subscription set

### Requirement: In-flight requests during reconnect are surfaced as errors
Requests that were waiting for a response when the transport disconnects SHALL be propagated to the caller as `mcp_exception` rather than being silently dropped, swallowed, or auto-retried.

#### Scenario: Caller blocked in sendRequest receives exception
- **WHEN** the caller is blocked in a high-level client method (e.g. `client.callTool`, `client.readResource`) at the moment the transport disconnects
- **THEN** the call SHALL throw `mcp_exception` whose message contains `"reconnected during request"` or `"connection lost"`
- **AND** the exception SHALL propagate within 1 second of the disconnect detection
- **AND** the client SHALL NOT auto-retry the same request even if reconnect later succeeds

#### Scenario: Caller may safely retry after reconnect
- **WHEN** the caller catches a `mcp_exception` from a method that failed due to reconnect
- **AND** waits for the reconnect handler to report success
- **THEN** invoking the same method again SHALL behave as a fresh request
- **AND** the previous failure SHALL NOT cause stale state to leak into the new request (no duplicate ID, no orphan promise)
