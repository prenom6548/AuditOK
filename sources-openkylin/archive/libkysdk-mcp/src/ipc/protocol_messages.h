#pragma once
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mcp_protocol {

// Entity structure for create_entities tool
struct Entity {
    std::string name;
    std::string entityType;
    std::vector<std::string> observations;

    Entity(const std::string& n, const std::string& type,
           const std::vector<std::string>& obs)
        : name(n), entityType(type), observations(obs) {}
};

// MCP protocol message types
enum class MCPMessageType {
    INITIALIZE,
    PING,
    PONG,
    NOTIFICATION,
    REQUEST,
    RESPONSE
};

// Build MCP initialize message
std::string createInitializeMessage1(
    const std::string& /* client_name */ = "cpp-client",
    const std::string& /* client_version */ = "1.0.0") {
    return R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "initialize",
        "params": {
            "protocolVersion": "2025-11-25",
            "capabilities": {
                "logging": {},
                "prompts": {
                    "listChanged": true
                },
                "resources": {
                    "subscribe": true,
                }
    })";
}
std::string createInitializeMessage(
    const std::string& /* client_name */ = "cpp-client",
    const std::string& /* client_version */ = "1.0.0") {
    return R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "initialize",
        "params": {
            "protocolVersion": "2025-11-25",
            "capabilities": {
            },
            "clientInfo": {
                "name": "cpp-client",
                "version": "1.0.0"
            }
        }
    })";
}

std::string createNotificationsInitialized() {
    return R"({
        "jsonrpc": "2.0",
        "method": "notifications/initialized"
    })";
}
// Build MCP ping message
std::string createPingMessage(int ping_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(ping_id) + R"(,
        "method": "ping"
    })";
}

// Build MCP pong response message
std::string createPongMessage(int pong_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(pong_id) + R"(,
        "result": {}
    })";
}

// Build MCP notification message
std::string createNotificationMessage(
    const std::string& method,
    const std::map<std::string, std::string>& params = {}) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "method": ")" + method +
                       R"(")";

    if (!params.empty()) {
        json += R"(,
        "params": {)";
        bool first = true;
        for (const auto& param : params) {
            if (!first) json += ",";
            json += R"(")" + param.first + R"(": ")" + param.second + R"(")";
            first = false;
        }
        json += "}";
    }

    json += "}";
    return json;
}

// Build MCP tool-call request
std::string createToolCallMessage(
    const std::string& tool_name,
    const std::map<std::string, std::string>& arguments = {},
    int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": ")" +
                       tool_name + R"(")";

    if (!arguments.empty()) {
        json += R"(,
            "arguments": {)";
        bool first = true;
        for (const auto& arg : arguments) {
            if (!first) json += ",";
            json += R"(")" + arg.first + R"(": ")" + arg.second + R"(")";
            first = false;
        }
        json += "}";
    }

    json += R"(
        }
    })";
    return json;
}

// Build list_allowed_directories tool-call request
std::string createListAllowedDirectoriesMessage(int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "list_allowed_directories",
            "arguments": {}
        }
    })";
}

// Helper: escape JSON string
std::string escapeJsonString(const std::string& str) {
    std::string result;
    result.reserve(str.length() + 10);  // Reserve extra space

    for (char c : str) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (c >= 0 && c < 32) {
                    // Control characters
                    result += "\\u";
                    result += "0000";
                    result[result.length() - 4] = '0' + (c >> 12);
                    result[result.length() - 3] = '0' + ((c >> 8) & 15);
                    result[result.length() - 2] = '0' + ((c >> 4) & 15);
                    result[result.length() - 1] = '0' + (c & 15);
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

// Build write_file tool-call request
std::string createWriteFileMessage(const std::string& file_path,
                                   const std::string& content,
                                   int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "write_file",
            "arguments": {
                "path": ")" +
                       escapeJsonString(file_path) + R"(",
                "content": ")" +
                       escapeJsonString(content) + R"("
            }
        }
    })";
    return json;
}

// Build read_file tool-call request
std::string createReadFileMessage(const std::string& file_path,
                                  int head_lines = -1, int tail_lines = -1,
                                  int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "read_file",
            "arguments": {
                "path": ")" +
                       escapeJsonString(file_path) + R"(")";

    if (head_lines > 0) {
        json += R"(,
                "head": )" +
                std::to_string(head_lines);
    }
    if (tail_lines > 0) {
        json += R"(,
                "tail": )" +
                std::to_string(tail_lines);
    }

    json += R"(
            }
        }
    })";
    return json;
}

// Build read_text_file tool-call request
std::string createReadTextFileMessage(const std::string& file_path,
                                      int head_lines = -1, int tail_lines = -1,
                                      int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "read_text_file",
            "arguments": {
                "path": ")" +
                       escapeJsonString(file_path) + R"(")";

    if (head_lines > 0) {
        json += R"(,
                "head": )" +
                std::to_string(head_lines);
    }
    if (tail_lines > 0) {
        json += R"(,
                "tail": )" +
                std::to_string(tail_lines);
    }

    json += R"(
            }
        }
    })";
    return json;
}

// Build read_media_file tool-call request
std::string createReadMediaFileMessage(const std::string& file_path,
                                       int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "read_media_file",
            "arguments": {
                "path": ")" +
           escapeJsonString(file_path) + R"("
            }
        }
    })";
}

// Build read_multiple_files tool-call request
std::string createReadMultipleFilesMessage(
    const std::vector<std::string>& file_paths, int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "read_multiple_files",
            "arguments": {
                "paths": [)";

    for (size_t i = 0; i < file_paths.size(); ++i) {
        if (i > 0) json += ",";
        json += R"(")" + escapeJsonString(file_paths[i]) + R"(")";
    }

    json += R"(
                ]
            }
        }
    })";
    return json;
}

// Build edit_file tool-call request
std::string createEditFileMessage(
    const std::string& file_path,
    const std::vector<std::pair<std::string, std::string>>& edits,
    bool dry_run = false, int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "edit_file",
            "arguments": {
                "path": ")" +
                       escapeJsonString(file_path) + R"(",
                "edits": [)";

    for (size_t i = 0; i < edits.size(); ++i) {
        if (i > 0) json += ",";
        json += R"(
                    {
                        "oldText": ")" +
                escapeJsonString(edits[i].first) + R"(",
                        "newText": ")" +
                escapeJsonString(edits[i].second) + R"("
                    })";
    }

    json += R"(
                ])";

    if (dry_run) {
        json += R"(,
                "dryRun": true)";
    }

    json += R"(
            }
        }
    })";
    return json;
}

// Build create_directory tool-call request
std::string createCreateDirectoryMessage(const std::string& dir_path,
                                         int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "create_directory",
            "arguments": {
                "path": ")" +
           escapeJsonString(dir_path) + R"("
            }
        }
    })";
}

// Build list_directory tool-call request
std::string createListDirectoryMessage(const std::string& dir_path,
                                       int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "list_directory",
            "arguments": {
                "path": ")" +
           escapeJsonString(dir_path) + R"("
            }
        }
    })";
}

// Build list_directory_with_sizes tool-call request
std::string createListDirectoryWithSizesMessage(
    const std::string& dir_path, const std::string& sort_by = "name",
    int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "list_directory_with_sizes",
            "arguments": {
                "path": ")" +
                       escapeJsonString(dir_path) + R"(",
                "sortBy": ")" +
                       sort_by + R"("
            }
        }
    })";
    return json;
}

// Build directory_tree tool-call request
std::string createDirectoryTreeMessage(const std::string& dir_path,
                                       int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "directory_tree",
            "arguments": {
                "path": ")" +
           escapeJsonString(dir_path) + R"("
            }
        }
    })";
}

// Build move_file tool-call request
std::string createMoveFileMessage(const std::string& source_path,
                                  const std::string& destination_path,
                                  int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "move_file",
            "arguments": {
                "source": ")" +
           escapeJsonString(source_path) + R"(",
                "destination": ")" +
           escapeJsonString(destination_path) + R"("
            }
        }
    })";
}

// Build search_files tool-call request
std::string createSearchFilesMessage(
    const std::string& search_path, const std::string& pattern,
    const std::vector<std::string>& exclude_patterns = {}, int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "tools/call",
        "params": {
            "name": "search_files",
            "arguments": {
                "path": ")" +
                       escapeJsonString(search_path) + R"(",
                "pattern": ")" +
                       escapeJsonString(pattern) + R"(")";

    if (!exclude_patterns.empty()) {
        json += R"(,
                "excludePatterns": [)";
        for (size_t i = 0; i < exclude_patterns.size(); ++i) {
            if (i > 0) json += ",";
            json += R"(")" + escapeJsonString(exclude_patterns[i]) + R"(")";
        }
        json += "]";
    }

    json += R"(
            }
        }
    })";
    return json;
}

// Build get_file_info tool-call request
std::string createGetFileInfoMessage(const std::string& file_path,
                                     int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/call",
        "params": {
            "name": "get_file_info",
            "arguments": {
                "path": ")" +
           escapeJsonString(file_path) + R"("
            }
        }
    })";
}

// Build MCP resource-read request
std::string createResourceReadMessage(const std::string& uri,
                                      int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "resources/read",
        "params": {
            "uri": ")" +
           uri + R"("
        }
    })";
}

// Build MCP prompt request
std::string createPromptMessage(
    const std::string& prompt_name,
    const std::map<std::string, std::string>& arguments = {},
    int request_id = 1) {
    std::string json = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(request_id) +
                       R"(,
        "method": "prompts/get",
        "params": {
            "name": ")" +
                       prompt_name + R"(")";

    if (!arguments.empty()) {
        json += R"(,
            "arguments": {)";
        bool first = true;
        for (const auto& arg : arguments) {
            if (!first) json += ",";
            json += R"(")" + arg.first + R"(": ")" + arg.second + R"(")";
            first = false;
        }
        json += "}";
    }

    json += R"(
        }
    })";
    return json;
}

// Build MCP tools-list request
std::string createListToolsMessage(int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "tools/list"
    })";
}

// Build MCP resources-list request
std::string createListResourcesMessage(int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "resources/list"
    })";
}

// Build MCP prompts-list request
std::string createListPromptsMessage(int request_id = 1) {
    return R"({
        "jsonrpc": "2.0",
        "id": )" +
           std::to_string(request_id) + R"(,
        "method": "prompts/list"
    })";
}

// Backward-compatible snake_case aliases
inline std::string create_initialize_message1(
    const std::string& client_name = "cpp-client",
    const std::string& client_version = "1.0.0") {
    return createInitializeMessage1(client_name, client_version);
}
inline std::string create_initialize_message(
    const std::string& client_name = "cpp-client",
    const std::string& client_version = "1.0.0") {
    return createInitializeMessage(client_name, client_version);
}
inline std::string create_notifications_initialized() {
    return createNotificationsInitialized();
}
inline std::string create_ping_message(int ping_id = 1) {
    return createPingMessage(ping_id);
}
inline std::string create_pong_message(int pong_id = 1) {
    return createPongMessage(pong_id);
}
inline std::string create_notification_message(
    const std::string& method,
    const std::map<std::string, std::string>& params = {}) {
    return createNotificationMessage(method, params);
}
inline std::string create_tool_call_message(
    const std::string& tool_name,
    const std::map<std::string, std::string>& arguments = {},
    int request_id = 1) {
    return createToolCallMessage(tool_name, arguments, request_id);
}
inline std::string create_list_allowed_directories_message(int request_id = 1) {
    return createListAllowedDirectoriesMessage(request_id);
}
inline std::string escape_json_string(const std::string& str) {
    return escapeJsonString(str);
}
inline std::string create_write_file_message(const std::string& file_path,
                                             const std::string& content,
                                             int request_id = 1) {
    return createWriteFileMessage(file_path, content, request_id);
}
inline std::string create_read_file_message(const std::string& file_path,
                                            int head_lines = -1,
                                            int tail_lines = -1,
                                            int request_id = 1) {
    return createReadFileMessage(file_path, head_lines, tail_lines, request_id);
}
inline std::string create_read_text_file_message(const std::string& file_path,
                                                 int head_lines = -1,
                                                 int tail_lines = -1,
                                                 int request_id = 1) {
    return createReadTextFileMessage(file_path, head_lines, tail_lines,
                                     request_id);
}
inline std::string create_read_media_file_message(const std::string& file_path,
                                                  int request_id = 1) {
    return createReadMediaFileMessage(file_path, request_id);
}
inline std::string create_read_multiple_files_message(
    const std::vector<std::string>& file_paths, int request_id = 1) {
    return createReadMultipleFilesMessage(file_paths, request_id);
}
inline std::string create_edit_file_message(
    const std::string& file_path,
    const std::vector<std::pair<std::string, std::string>>& edits,
    bool dry_run = false, int request_id = 1) {
    return createEditFileMessage(file_path, edits, dry_run, request_id);
}
inline std::string create_create_directory_message(const std::string& dir_path,
                                                   int request_id = 1) {
    return createCreateDirectoryMessage(dir_path, request_id);
}
inline std::string create_list_directory_message(const std::string& dir_path,
                                                 int request_id = 1) {
    return createListDirectoryMessage(dir_path, request_id);
}
inline std::string create_list_directory_with_sizes_message(
    const std::string& dir_path, const std::string& sort_by = "name",
    int request_id = 1) {
    return createListDirectoryWithSizesMessage(dir_path, sort_by, request_id);
}
inline std::string create_directory_tree_message(const std::string& dir_path,
                                                 int request_id = 1) {
    return createDirectoryTreeMessage(dir_path, request_id);
}
inline std::string create_move_file_message(const std::string& source_path,
                                            const std::string& destination_path,
                                            int request_id = 1) {
    return createMoveFileMessage(source_path, destination_path, request_id);
}
inline std::string create_search_files_message(
    const std::string& search_path, const std::string& pattern,
    const std::vector<std::string>& exclude_patterns = {},
    int request_id = 1) {
    return createSearchFilesMessage(search_path, pattern, exclude_patterns,
                                    request_id);
}
inline std::string create_get_file_info_message(const std::string& file_path,
                                                int request_id = 1) {
    return createGetFileInfoMessage(file_path, request_id);
}
inline std::string create_resource_read_message(const std::string& uri,
                                                int request_id = 1) {
    return createResourceReadMessage(uri, request_id);
}
inline std::string create_prompt_message(
    const std::string& prompt_name,
    const std::map<std::string, std::string>& arguments = {},
    int request_id = 1) {
    return createPromptMessage(prompt_name, arguments, request_id);
}
inline std::string create_list_tools_message(int request_id = 1) {
    return createListToolsMessage(request_id);
}
inline std::string create_list_resources_message(int request_id = 1) {
    return createListResourcesMessage(request_id);
}
inline std::string create_list_prompts_message(int request_id = 1) {
    return createListPromptsMessage(request_id);
}

}  // namespace mcp_protocol
