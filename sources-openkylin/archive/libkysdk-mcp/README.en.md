# libkysdk-mcp

`libkysdk-mcp` provides C++ SDK libraries for building MCP clients and servers.

## Build

From the repository root:

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build -j4
```

## Main Binaries

After build, common example binaries are generated under `build/bin/`:

- `server_filesystem_ipc_example`
- `mcp_client_ipc_demo`
- `server_filesystem_http_example`
- `mcp_client_http_demo`
- `mcp_server_full_example`
- `mcp_client_full_example`
- `transport_server_example`
- `transport_server_client_example`

## Quick Start

### IPC

Run server first:

```bash
./build/bin/server_filesystem_ipc_example
```

Then run client in another terminal:

```bash
./build/bin/mcp_client_ipc_demo
```

### HTTP/SSE

Run server first:

```bash
./build/bin/server_filesystem_http_example
```

Then run client in another terminal:

```bash
./build/bin/mcp_client_http_demo
```

### Full Regression Examples

The full examples cover a wider MCP flow including initialize/ping, tools/resources/prompts, progress/cancel, roots refresh, and dynamic list_changed notifications.

IPC mode:

```bash
./build/bin/mcp_client_full_example --ipc "./build/bin/mcp_server_full_example --ipc"
```

HTTP/SSE mode:

```bash
./build/bin/mcp_server_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
./build/bin/mcp_client_full_example --host 127.0.0.1 --port 18080 --endpoint /mcp
```

## Unit Tests

Configure and build with tests enabled:

```bash
cmake -S . -B build-ut -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-ut -j4
```

Run tests:

```bash
cd build-ut
ctest -N
ctest --output-on-failure
```

## Stability Notes

- IPC timeout lifecycle race has been root-fixed. Timeout workers are now
  managed and joined during shutdown instead of detached background access.
- Server-side SSE listen currently supports `TransportProtocol::kHttp` only.
  Configuring `kHttps` in server mode is rejected explicitly at startup.

## Documentation

- Client API: `docs/client_api.md`
- Server API: `docs/server_api.md`
- Examples guide: `examples/README.md`

## License

This project is licensed under GPL-3.0-or-later. See `LICENSE` for details.
