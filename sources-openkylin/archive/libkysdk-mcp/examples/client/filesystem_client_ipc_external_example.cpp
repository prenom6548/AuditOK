/**
 * @file filesystem_client_ipc_external_example.cpp
 * @brief MCP filesystem client example for an external stdio MCP server.
 *
 * This example connects to an external filesystem MCP server binary over IPC
 * (stdio), initializes MCP, and runs a few read-only verification calls.
 *
 * Usage:
 *   ./mcp_filesystem_client_ipc_external_example
 *   ./mcp_filesystem_client_ipc_external_example --server-bin /path/to/mcp-filesystem-server
 *   ./mcp_filesystem_client_ipc_external_example --allow-dir /path/a --allow-dir /path/b
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "client.h"
#include "common/transport.h"

using namespace mcp;
using namespace mcp_transport;

namespace {

constexpr const char* kDefaultServerBinary =
    "/home/kylin/projects/openkylin/mcp-filesystem-server_linux_amd64/"
    "mcp-filesystem-server";

void printUsage(const char* program) {
    std::cout
        << "Usage: " << program << " [options]\n"
        << "  --server-bin <path>   Filesystem MCP server binary path\n"
        << "                        (default: " << kDefaultServerBinary << ")\n"
        << "  --allow-dir <path>    Allowed directory for filesystem server\n"
        << "                        (repeatable; default: current directory '.')\n"
        << "  --help                Show this help message\n";
}

void printToolList(const std::vector<Tool>& tools) {
    std::cout << "Tools (" << tools.size() << "):\n";
    for (const auto& tool : tools) {
        std::cout << "  - " << tool.name;
        if (!tool.description.empty()) {
            std::cout << ": " << tool.description;
        }
        std::cout << '\n';
    }
}

std::string extractTextContent(const ToolCallResult& result) {
    if (result.content.is_array() && !result.content.empty() &&
        result.content[0].contains("text")) {
        return result.content[0]["text"].get<std::string>();
    }
    return result.content.dump(2);
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string server_bin = kDefaultServerBinary;
    std::vector<std::string> allowed_dirs;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server-bin" && i + 1 < argc) {
            server_bin = argv[++i];
        } else if (arg == "--allow-dir" && i + 1 < argc) {
            allowed_dirs.emplace_back(argv[++i]);
        } else if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            printUsage(argv[0]);
            return 1;
        }
    }

    if (allowed_dirs.empty()) {
        allowed_dirs.emplace_back(".");
    }

    std::cout << "External MCP filesystem client example\n";
    std::cout << "Server binary: " << server_bin << '\n';
    std::cout << "Allowed directories:\n";
    for (const auto& dir : allowed_dirs) {
        std::cout << "  - " << dir << '\n';
    }

    try {
        MCPClient client;

        TransportConfig config;
        config.type = TransportType::kIpc;
        config.ipc.command = server_bin;
        config.ipc.args = allowed_dirs;
        config.ipc.read_timeout_ms = 3000;
        config.ipc.max_retry_count = 3;
        config.ipc.enable_logging = false;
        client.setTransportType(TransportType::kIpc);

        if (!client.connectWithConfig(config)) {
            std::cerr << "Connect failed.\n";
            return 1;
        }
        std::cout << "Connected.\n";

        client.setClientInfo("mcp-filesystem-client-ipc-external", "1.0.0");
        InitializeParams params;
        params.protocolVersion = PROTOCOL_VERSION;
        params.clientInfo.name = "mcp-filesystem-client-ipc-external";
        params.clientInfo.version = "1.0.0";
        params.capabilities = mcp::utils::createDefaultClientCapabilities();

        const auto init_result = client.initialize(params);
        client.initialized();
        std::cout << "Initialized with server: " << init_result.serverInfo.name
                  << " v" << init_result.serverInfo.version << '\n';

        const auto tools = client.listTools();
        printToolList(tools.tools);

        const auto allow_result = client.callTool("list_allowed_directories");
        if (allow_result.isError) {
            std::cerr << "list_allowed_directories error: "
                      << allow_result.errorMessage << '\n';
        } else {
            std::cout << "list_allowed_directories result:\n"
                      << allow_result.content.dump(2) << '\n';
        }

        const auto list_result =
            client.callTool("list_directory", {{"path", allowed_dirs.front()}});
        if (list_result.isError) {
            std::cerr << "list_directory error: " << list_result.errorMessage
                      << '\n';
        } else {
            std::cout << "list_directory result (" << allowed_dirs.front()
                      << "):\n"
                      << list_result.content.dump(2) << '\n';
        }

        const std::string test_file = "/tmp/mcp_filesystem_client_example.txt";
        const std::string test_content =
            "hello from mcp filesystem client ipc external example";

        const auto write_result =
            client.callTool("write_file",
                            {{"path", test_file}, {"content", test_content}});
        if (write_result.isError) {
            std::cerr << "write_file error: " << write_result.errorMessage
                      << '\n';
            client.disconnect();
            return 1;
        }
        std::cout << "write_file success: " << test_file << '\n';

        const auto read_result =
            client.callTool("read_file", {{"path", test_file}});
        if (read_result.isError) {
            std::cerr << "read_file error: " << read_result.errorMessage
                      << '\n';
            client.disconnect();
            return 1;
        }
        std::cout << "read_file content:\n"
                  << extractTextContent(read_result) << '\n';

        client.disconnect();
        std::cout << "Disconnected.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
}
