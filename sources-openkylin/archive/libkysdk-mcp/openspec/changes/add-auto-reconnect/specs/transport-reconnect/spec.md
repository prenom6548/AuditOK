## ADDED Requirements

### Requirement: Passive disconnect detection
The transport layer SHALL detect the loss of an underlying connection without requiring an outbound request, and MUST update its observable status accordingly so callers can rely on `isConnected()` to reflect the real connection health.

#### Scenario: IPC child process exits unexpectedly
- **WHEN** the IPC server subprocess terminates (normally or abnormally) while the client transport is in `kConnected` state
- **THEN** the transport status SHALL transition to `kError` within the keepalive window (default ≤ 1 second)
- **AND** subsequent `isConnected()` calls SHALL return false
- **AND** any future `sendRequestSync()` SHALL fail immediately with a non-200 status without waiting for `read_timeout_ms`

#### Scenario: SSE long connection closed by server
- **WHEN** the server closes the SSE stream while the client transport is in `kConnected` state
- **THEN** the transport status SHALL transition to `kError` within the keepalive window
- **AND** `isConnected()` SHALL return false

#### Scenario: SSE keepalive timeout
- **WHEN** `config.sse.keepalive_interval > 0` is configured
- **AND** the client sends `keepalive_interval` consecutive heartbeats without receiving any response within the keepalive timeout
- **THEN** the transport status SHALL transition to `kError`

### Requirement: Automatic reconnection loop
The transport layer SHALL provide an opt-in background reconnection loop driven by the `auto_reconnect`, `reconnect_interval`, and `max_reconnect_attempts` configuration fields.

#### Scenario: Reconnect succeeds within attempt budget
- **WHEN** `config.*.auto_reconnect` is true
- **AND** the transport detects disconnect via the passive detection mechanism
- **THEN** the transport SHALL transition to `kReconnecting` state
- **AND** the transport SHALL attempt to re-establish the connection on a background thread
- **AND** subsequent attempts SHALL use a backoff interval of `min(reconnect_interval * attempt_number, max_reconnect_interval)` with up to ±`reconnect_interval` jitter
- **WHEN** any attempt succeeds before exceeding `max_reconnect_attempts`
- **THEN** the transport status SHALL transition back to `kConnected`
- **AND** the upper layer reconnection handler SHALL be invoked exactly once

#### Scenario: Reconnect attempt budget exhausted
- **WHEN** auto-reconnect is enabled
- **AND** the transport has failed `max_reconnect_attempts` consecutive reconnection attempts
- **THEN** the transport status SHALL transition to `kError` and remain there
- **AND** the reconnection handler SHALL be invoked with a failure outcome
- **AND** no further automatic attempts SHALL be made until the caller invokes `connect()` / `reconnect()` again

#### Scenario: Auto-reconnect disabled
- **WHEN** `config.*.auto_reconnect` is false
- **AND** the transport detects disconnect
- **THEN** the transport status SHALL transition to `kError` and remain there
- **AND** no background reconnection attempt SHALL be made

#### Scenario: Explicit disconnect while reconnecting
- **WHEN** the transport is in `kReconnecting` state
- **AND** the caller invokes `disconnect()` or `stop()`
- **THEN** the background reconnection loop SHALL terminate within 1 second
- **AND** the transport status SHALL transition to `kDisconnected`
- **AND** the reconnection handler SHALL NOT be invoked

### Requirement: In-flight request handling on disconnect
When the transport transitions out of `kConnected` due to a detected disconnect, every request currently waiting for a response SHALL be failed deterministically rather than silently hanging until timeout.

#### Scenario: In-flight requests fail with deterministic error
- **WHEN** at least one request is in flight (the caller is blocked on `sendRequestSync`)
- **AND** the passive detection mechanism reports disconnect
- **THEN** every pending request SHALL be completed with `status_code != 200` and an error payload whose message contains `"reconnected"` or `"connection lost"`
- **AND** the requests SHALL complete within 1 second of the detection event

#### Scenario: New requests during reconnection are rejected
- **WHEN** the transport is in `kReconnecting` state
- **AND** a caller invokes `sendRequestSync()`
- **THEN** the call SHALL return immediately with an error response whose message contains `"reconnecting"` and SHALL NOT queue, retry, or block until reconnection completes

### Requirement: Configuration fields are effective
The configuration fields `auto_reconnect`, `reconnect_interval`, and the newly introduced `max_reconnect_attempts` and `max_reconnect_interval` SHALL drive the reconnection behavior. Default values MUST preserve pre-existing behavior for users who do not opt in.

#### Scenario: Default configuration preserves legacy behavior
- **WHEN** the caller starts the transport without modifying any reconnect-related config fields
- **THEN** `auto_reconnect` SHALL default to false
- **AND** no background reconnection thread SHALL start
- **AND** the transport SHALL behave identically to the version prior to this change

#### Scenario: Opting into auto-reconnect
- **WHEN** the caller sets `config.*.auto_reconnect = true`
- **THEN** the transport SHALL start the background reconnection loop on successful `connect()`
- **AND** `reconnect_interval` SHALL be honored as the base backoff value in seconds
- **AND** `max_reconnect_attempts` (default 5) SHALL bound the total number of consecutive failed attempts

### Requirement: IPC respawn behavior is opt-in
For the IPC transport type, the reconnection loop SHALL only spawn a new subprocess when explicitly authorized via the new configuration field `ipc.respawn_on_disconnect`. If the field is false, reconnection SHALL attempt to re-attach to an existing endpoint (where applicable) rather than starting a new process.

#### Scenario: Respawn enabled
- **WHEN** `ipc.respawn_on_disconnect = true`
- **AND** the subprocess exits unexpectedly
- **AND** auto-reconnect is enabled
- **THEN** the reconnection loop SHALL `fork+exec` the same `ipc.command` / `ipc.args` used for the original connection on each attempt

#### Scenario: Respawn disabled
- **WHEN** `ipc.respawn_on_disconnect = false` (default)
- **AND** the subprocess exits unexpectedly
- **THEN** the reconnection loop SHALL NOT start a new subprocess
- **AND** all reconnection attempts SHALL fail
- **AND** the budget exhaustion path defined in "Automatic reconnection loop" SHALL apply

### Requirement: Status transitions are mutex-protected
All transport status transitions SHALL be serialized through a single mutex so that concurrent reads of `isConnected()` / `getTransportStatus()` from any thread observe a consistent state machine.

#### Scenario: Concurrent status reads remain consistent
- **WHEN** thread A is in the middle of a status transition triggered by the reconnection loop
- **AND** thread B calls `isConnected()` or `getTransportStatus()` concurrently
- **THEN** thread B SHALL observe either the pre-transition or the post-transition state, never an intermediate or torn value
