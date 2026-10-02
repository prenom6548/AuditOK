/**
 * @file protocol.h
 * @brief MCP Protocol Core Definitions and Utilities
 *
 * This file contains the core protocol definitions, data structures,
 * and utility functions for the Model Context Protocol (MCP).
 * Implements the 2024-11-05 and 2025-11-25 protocol specifications.
 * Supports backward compatibility between versions.
 */

#ifndef KYSDK_MCP_PROTOCOL_COMMON_H
#define KYSDK_MCP_PROTOCOL_COMMON_H

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "message.h"

namespace mcp {

namespace protocol_detail {
inline std::string payloadPreview(const json& j, size_t max_len = 256) {
    std::string dumped = j.dump();
    if (dumped.size() > max_len) {
        dumped = dumped.substr(0, max_len) + "...";
    }
    return dumped;
}

inline std::string invalidPayloadMessage(const std::string& reason,
                                         const json& payload) {
    return reason + ", payload=" + payloadPreview(payload);
}
}  // namespace protocol_detail

// MCP Protocol Methods
namespace methods {
// Lifecycle methods
constexpr const char* INITIALIZE = "initialize";
constexpr const char* PING = "ping";

// Resource methods
constexpr const char* RESOURCES_LIST = "resources/list";
constexpr const char* RESOURCES_READ = "resources/read";
constexpr const char* RESOURCES_TEMPLATES_LIST = "resources/templates/list";

// Tool methods
constexpr const char* TOOLS_LIST = "tools/list";
constexpr const char* TOOLS_CALL = "tools/call";

// Prompt methods
constexpr const char* PROMPTS_LIST = "prompts/list";
constexpr const char* PROMPTS_GET = "prompts/get";

// Logging methods
constexpr const char* LOGGING_SET_LEVEL = "logging/setLevel";

// Task methods (new in 2025-11-25)
constexpr const char* TASKS_GET = "tasks/get";
constexpr const char* TASKS_RESULT = "tasks/result";
constexpr const char* TASKS_LIST = "tasks/list";
constexpr const char* TASKS_CANCEL = "tasks/cancel";

// Elicitation methods (new in 2025-11-25)
constexpr const char* ELICITATION_CREATE = "elicitation/create";

// Completion methods
constexpr const char* COMPLETION_COMPLETE = "completion/complete";

// Sampling methods
constexpr const char* SAMPLING_CREATE_MESSAGE = "sampling/createMessage";

// Resources methods
constexpr const char* RESOURCES_SUBSCRIBE = "resources/subscribe";
constexpr const char* RESOURCES_UNSUBSCRIBE = "resources/unsubscribe";

// Roots methods
constexpr const char* ROOTS_LIST = "roots/list";

// Cancellation
constexpr const char* CANCEL_REQUEST = "$/cancelRequest";

// Notifications
constexpr const char* NOTIFICATION_INITIALIZED = "notifications/initialized";
constexpr const char* NOTIFICATION_CANCELLED = "notifications/cancelled";
constexpr const char* NOTIFICATION_RESOURCES_LIST_CHANGED =
    "notifications/resources/list_changed";
constexpr const char* NOTIFICATION_RESOURCE_UPDATED =
    "notifications/resources/updated";
constexpr const char* NOTIFICATION_PROMPTS_LIST_CHANGED =
    "notifications/prompts/list_changed";
constexpr const char* NOTIFICATION_TOOLS_LIST_CHANGED =
    "notifications/tools/list_changed";
constexpr const char* NOTIFICATION_ROOTS_LIST_CHANGED =
    "notifications/roots/list_changed";
constexpr const char* NOTIFICATION_MESSAGE = "notifications/message";
constexpr const char* NOTIFICATION_PROGRESS = "notifications/progress";

// Task notifications (new in 2025-11-25)
constexpr const char* NOTIFICATION_TASKS_STATUS = "notifications/tasks/status";

// Elicitation notifications (new in 2025-11-25)
constexpr const char* NOTIFICATION_ELICITATION_COMPLETE =
    "notifications/elicitation/complete";
}  // namespace methods

// MCP Protocol Version Constants
constexpr const char* PROTOCOL_VERSION_2024_11_05 = "2024-11-05";
constexpr const char* PROTOCOL_VERSION_2025_11_25 = "2025-11-25";
constexpr const char* PROTOCOL_VERSION =
    PROTOCOL_VERSION_2025_11_25;  // Default for backward compatibility

// Client Capabilities
struct ClientCapabilities {
    json experimental = json::object();
    json logging = json::object();
    json prompts = json::object();
    json resources = json::object();
    json tools = json::object();
    json sampling = json::object();
    json elicitation = json::object();
    json tasks = json::object();
    json roots = json::object();

    json toJson() const {
        json caps = json::object();
        if (!experimental.empty()) caps["experimental"] = experimental;
        if (!logging.empty()) caps["logging"] = logging;
        if (!prompts.empty()) caps["prompts"] = prompts;
        if (!resources.empty()) caps["resources"] = resources;
        if (!tools.empty()) caps["tools"] = tools;
        if (!sampling.empty()) caps["sampling"] = sampling;
        if (!elicitation.empty()) caps["elicitation"] = elicitation;
        if (!tasks.empty()) caps["tasks"] = tasks;
        if (!roots.empty()) caps["roots"] = roots;
        return caps;
    }

    static ClientCapabilities fromJson(const json& j) {
        ClientCapabilities caps;
        if (j.contains("experimental")) caps.experimental = j["experimental"];
        if (j.contains("logging")) caps.logging = j["logging"];
        if (j.contains("prompts")) caps.prompts = j["prompts"];
        if (j.contains("resources")) caps.resources = j["resources"];
        if (j.contains("tools")) caps.tools = j["tools"];
        if (j.contains("sampling")) caps.sampling = j["sampling"];
        if (j.contains("elicitation")) caps.elicitation = j["elicitation"];
        if (j.contains("tasks")) caps.tasks = j["tasks"];
        if (j.contains("roots")) caps.roots = j["roots"];
        return caps;
    }
};

// Server Capabilities
struct ServerCapabilities {
    json experimental = json::object();
    json prompts = json::object();
    json resources = json::object();
    json tools = json::object();
    json logging = json::object();
    json sampling = json::object();
    json completions = json::object();
    json elicitation = json::object();
    json tasks = json::object();

    json toJson() const {
        json caps = json::object();
        if (!experimental.empty()) caps["experimental"] = experimental;
        if (!prompts.empty()) caps["prompts"] = prompts;
        if (!resources.empty()) caps["resources"] = resources;
        if (!tools.empty()) caps["tools"] = tools;
        if (!logging.empty()) caps["logging"] = logging;
        if (!sampling.empty()) caps["sampling"] = sampling;
        if (!completions.empty()) caps["completions"] = completions;
        if (!elicitation.empty()) caps["elicitation"] = elicitation;
        if (!tasks.empty()) caps["tasks"] = tasks;
        return caps;
    }

    static ServerCapabilities fromJson(const json& j) {
        ServerCapabilities caps;
        if (j.contains("experimental")) caps.experimental = j["experimental"];
        if (j.contains("prompts")) caps.prompts = j["prompts"];
        if (j.contains("resources")) caps.resources = j["resources"];
        if (j.contains("tools")) caps.tools = j["tools"];
        if (j.contains("logging")) caps.logging = j["logging"];
        if (j.contains("sampling")) caps.sampling = j["sampling"];
        if (j.contains("completions")) caps.completions = j["completions"];
        if (j.contains("elicitation")) caps.elicitation = j["elicitation"];
        if (j.contains("tasks")) caps.tasks = j["tasks"];
        return caps;
    }
};

// Client Info
struct ClientInfo {
    std::string name;
    std::string version;

    json toJson() const { return {{"name", name}, {"version", version}}; }

    static ClientInfo fromJson(const json& j) {
        ClientInfo info;
        info.name = j.value("name", "");
        info.version = j.value("version", "");
        return info;
    }
};

// Server Info
struct ServerInfo {
    std::string name;
    std::string version;

    json toJson() const { return {{"name", name}, {"version", version}}; }

    static ServerInfo fromJson(const json& j) {
        ServerInfo info;
        info.name = j.value("name", "");
        info.version = j.value("version", "");
        return info;
    }
};

// ========== 2025-11-25 Common Types (defined early for use in other
// structures) ==========

// Metadata structure for _meta field (2025-11-25)
struct MetaData {
    json progressToken = nullptr;

    json toJson() const {
        json j = json::object();
        if (!progressToken.is_null()) {
            j["progressToken"] = progressToken;
        }
        return j;
    }

    static MetaData fromJson(const json& j) {
        MetaData meta;
        if (j.contains("progressToken")) {
            meta.progressToken = j["progressToken"];
        }
        return meta;
    }

    bool empty() const { return progressToken.is_null(); }
};

// Initialize Request Parameters (extended for 2025-11-25 with optional _meta)
struct InitializeParams {
    std::string protocolVersion = PROTOCOL_VERSION;
    ClientCapabilities capabilities;
    ClientInfo clientInfo;
    MetaData _meta;  // Optional field for 2025-11-25

    json toJson() const {
        json j = {{"protocolVersion", protocolVersion},
                  {"capabilities", capabilities.toJson()},
                  {"clientInfo", clientInfo.toJson()}};
        // Only include _meta if not empty (2025-11-25)
        if (!_meta.empty()) {
            j["_meta"] = _meta.toJson();
        }
        return j;
    }

    static InitializeParams fromJson(const json& j) {
        InitializeParams params;
        params.protocolVersion = j.value("protocolVersion", PROTOCOL_VERSION);
        if (j.contains("capabilities")) {
            params.capabilities =
                ClientCapabilities::fromJson(j["capabilities"]);
        }
        if (j.contains("clientInfo")) {
            params.clientInfo = ClientInfo::fromJson(j["clientInfo"]);
        }
        // Parse _meta if present (2025-11-25)
        if (j.contains("_meta")) {
            params._meta = MetaData::fromJson(j["_meta"]);
        }
        return params;
    }
};

// Initialize Response Result (extended for 2025-11-25 with optional
// instructions)
struct InitializeResult {
    std::string protocolVersion = PROTOCOL_VERSION;
    ServerCapabilities capabilities;
    ServerInfo serverInfo;
    std::string instructions;  // Optional field for 2025-11-25

    json toJson() const {
        json j = {{"protocolVersion", protocolVersion},
                  {"capabilities", capabilities.toJson()},
                  {"serverInfo", serverInfo.toJson()}};
        // Only include instructions if not empty (2025-11-25)
        if (!instructions.empty()) {
            j["instructions"] = instructions;
        }
        return j;
    }

    static InitializeResult fromJson(const json& j) {
        InitializeResult result;
        result.protocolVersion = j.value("protocolVersion", PROTOCOL_VERSION);
        if (j.contains("capabilities")) {
            result.capabilities =
                ServerCapabilities::fromJson(j["capabilities"]);
        }
        if (j.contains("serverInfo")) {
            result.serverInfo = ServerInfo::fromJson(j["serverInfo"]);
        }
        // Parse instructions if present (2025-11-25)
        if (j.contains("instructions")) {
            result.instructions = j.value("instructions", "");
        }
        return result;
    }
};

// Tool Definition
struct Tool {
    std::string name;
    std::string description;
    json inputSchema;

    json toJson() const {
        return {{"name", name},
                {"description", description},
                {"inputSchema", inputSchema}};
    }

    static Tool fromJson(const json& j) {
        Tool tool;
        tool.name = j.value("name", "");
        tool.description = j.value("description", "");
        tool.inputSchema = j.value("inputSchema", json::object());
        return tool;
    }
};

// Tool Call Parameters (extended for 2025-11-25 with optional _meta and task)
// Note: TaskMetadata is defined later in this file, so we use forward
// declaration
struct ToolCallParams {
    std::string name;
    json arguments;
    MetaData _meta;  // Optional field for 2025-11-25

    // Forward declaration - TaskMetadata will be defined later
    struct TaskMetadataParams {
        std::optional<long long> ttl_ms;  // Requested time to live in milliseconds

        json toJson() const {
            json j = json::object();
            if (ttl_ms.has_value()) {
                j["ttl"] = ttl_ms.value();
            }
            return j;
        }

        static TaskMetadataParams fromJson(const json& j) {
            TaskMetadataParams metadata;
            if (j.contains("ttl") && j["ttl"].is_number_integer()) {
                metadata.ttl_ms = j["ttl"].get<long long>();
            }
            return metadata;
        }

        bool empty() const { return !ttl_ms.has_value(); }
    };
    TaskMetadataParams task;  // Optional field for 2025-11-25

    json toJson() const {
        json j = {{"name", name}, {"arguments", arguments}};
        // Only include _meta if not empty (2025-11-25)
        if (!_meta.empty()) {
            j["_meta"] = _meta.toJson();
        }
        // Only include task if not empty (2025-11-25)
        if (!task.empty()) {
            j["task"] = task.toJson();
        }
        return j;
    }

    static ToolCallParams fromJson(const json& j) {
        ToolCallParams params;
        params.name = j.value("name", "");
        params.arguments = j.value("arguments", json::object());
        // Parse _meta if present (2025-11-25)
        if (j.contains("_meta")) {
            params._meta = MetaData::fromJson(j["_meta"]);
        }
        // Parse task if present (2025-11-25)
        if (j.contains("task")) {
            params.task =
                ToolCallParams::TaskMetadataParams::fromJson(j["task"]);
        }
        return params;
    }
};

// Tool Call Result (extended for 2025-11-25 with structuredContent)
struct ToolCallResult {
    json content;
    bool isError = false;
    std::string errorMessage;
    json structuredContent;  // Optional field for 2025-11-25

    json toJson() const {
        json j;
        j["isError"] = isError;

        // For error case: include error field and do not include content
        if (isError && !errorMessage.empty()) {
            j["error"] = errorMessage;
        } else {
            // For success case: include content normally
            if (!content.is_null()) {
                j["content"] = content;
            } else {
                j["content"] = json::array();
            }
        }
        // Only include structuredContent if not empty (2025-11-25)
        if (!structuredContent.empty()) {
            j["structuredContent"] = structuredContent;
        }
        return j;
    }

    static ToolCallResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tools/call payload: root must be an object", j));
        }

        ToolCallResult result;
        if (j.contains("isError")) {
            if (!j["isError"].is_boolean()) {
                throw mcp_exception(
                    ErrorCode::kInternalError,
                    protocol_detail::invalidPayloadMessage(
                        "Invalid tools/call payload: field 'isError' must be a boolean",
                        j));
            }
            result.isError = j["isError"].get<bool>();
        }
        // Keep backward compatibility: old payloads may only carry `error`,
        // and new payloads may carry both `isError` and `error`.
        if (j.contains("error")) {
            if (!j["error"].is_string()) {
                throw mcp_exception(
                    ErrorCode::kInternalError,
                    protocol_detail::invalidPayloadMessage(
                        "Invalid tools/call payload: field 'error' must be a string",
                        j));
            }
            result.errorMessage = j.value("error", "");
            result.isError = result.isError || !result.errorMessage.empty();
        }
        if (!j.contains("content") && !j.contains("error") &&
            !j.contains("structuredContent")) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tools/call payload: missing result fields", j));
        }
        if (j.contains("content")) result.content = j["content"];
        // Parse structuredContent if present (2025-11-25)
        if (j.contains("structuredContent")) {
            result.structuredContent = j["structuredContent"];
        }
        return result;
    }
};

// Resource Definition
struct Resource {
    std::string uri;
    std::string name;
    std::string description;
    std::string mimeType;

    json toJson() const {
        return {{"uri", uri},
                {"name", name},
                {"description", description},
                {"mimeType", mimeType}};
    }

    static Resource fromJson(const json& j) {
        Resource resource;
        resource.uri = j.value("uri", "");
        resource.name = j.value("name", "");
        resource.description = j.value("description", "");
        resource.mimeType = j.value("mimeType", "");
        return resource;
    }
};

// Resource Template Definition
struct ResourceTemplate {
    std::string uriTemplate;
    std::string name;
    std::string description;
    std::string mimeType;
    json annotations = json::object();

    json toJson() const {
        json j = {{"uriTemplate", uriTemplate},
                  {"name", name},
                  {"description", description},
                  {"mimeType", mimeType}};
        if (!annotations.empty()) {
            j["annotations"] = annotations;
        }
        return j;
    }

    static ResourceTemplate fromJson(const json& j) {
        ResourceTemplate tmpl;
        tmpl.uriTemplate = j.value("uriTemplate", "");
        tmpl.name = j.value("name", "");
        tmpl.description = j.value("description", "");
        tmpl.mimeType = j.value("mimeType", "");
        tmpl.annotations = j.value("annotations", json::object());
        return tmpl;
    }
};

// Prompt Definition
struct Prompt {
    std::string name;
    std::string description;
    std::vector<std::string> arguments;

    json toJson() const {
        json args = json::array();
        for (const auto& argument : arguments) {
            args.push_back(
                {{"name", argument}, {"description", ""}, {"required", false}});
        }
        return {
            {"name", name}, {"description", description}, {"arguments", args}};
    }

    static Prompt fromJson(const json& j) {
        Prompt prompt;
        prompt.name = j.value("name", "");
        prompt.description = j.value("description", "");
        if (j.contains("arguments") && j["arguments"].is_array()) {
            for (const auto& arg : j["arguments"]) {
                if (arg.is_string()) {
                    prompt.arguments.push_back(arg.get<std::string>());
                } else if (arg.is_object() && arg.contains("name") &&
                           arg["name"].is_string()) {
                    prompt.arguments.push_back(arg["name"].get<std::string>());
                }
            }
        }
        return prompt;
    }
};

// Root Definition (returned by client via roots/list)
struct Root {
    std::string uri;
    std::string name;

    json toJson() const {
        json j;
        j["uri"] = uri;
        if (!name.empty()) {
            j["name"] = name;
        }
        return j;
    }

    static Root fromJson(const json& j) {
        Root root;
        root.uri = j.value("uri", "");
        root.name = j.value("name", "");
        return root;
    }
};

// ========== 2025-11-25 New Types and Extensions ==========

// Common Types (2025-11-25)
using RequestId = json;      // string | integer
using ProgressToken = json;  // string | integer
using Cursor = std::string;

// Role enum
enum class Role { kAssistant, kUser };

inline std::string roleToString(Role role) {
    return role == Role::kAssistant ? "assistant" : "user";
}

inline Role roleFromString(const std::string& role) {
    return role == "assistant" ? Role::kAssistant : Role::kUser;
}

// Content Types (2025-11-25)
struct TextContent {
    std::string type = "text";
    std::string text;

    json toJson() const { return {{"type", type}, {"text", text}}; }

    static TextContent fromJson(const json& j) {
        TextContent content;
        content.type = j.value("type", "text");
        content.text = j.value("text", "");
        return content;
    }
};

struct ImageContent {
    std::string type = "image";
    std::string data;
    std::string mimeType;

    json toJson() const {
        return {{"type", type}, {"data", data}, {"mimeType", mimeType}};
    }

    static ImageContent fromJson(const json& j) {
        ImageContent content;
        content.type = j.value("type", "image");
        content.data = j.value("data", "");
        content.mimeType = j.value("mimeType", "");
        return content;
    }
};

// AudioContent (new in 2025-11-25)
struct AudioContent {
    std::string type = "audio";
    std::string data;
    std::string mimeType;

    json toJson() const {
        return {{"type", type}, {"data", data}, {"mimeType", mimeType}};
    }

    static AudioContent fromJson(const json& j) {
        AudioContent content;
        content.type = j.value("type", "audio");
        content.data = j.value("data", "");
        content.mimeType = j.value("mimeType", "");
        return content;
    }
};

// ResourceLink (new in 2025-11-25)
struct ResourceLink {
    std::string type = "resource";
    std::string uri;
    std::string text;  // Optional

    json toJson() const {
        json j = {{"type", type}, {"uri", uri}};
        if (!text.empty()) j["text"] = text;
        return j;
    }

    static ResourceLink fromJson(const json& j) {
        ResourceLink link;
        link.type = j.value("type", "resource");
        link.uri = j.value("uri", "");
        link.text = j.value("text", "");
        return link;
    }
};

// Task Types (new in 2025-11-25)
enum class TaskStatus {
    kWorking,
    kCompleted,
    kFailed,
    kCancelled,
    kInputRequired
};

inline std::string taskStatusToString(TaskStatus status) {
    switch (status) {
        case TaskStatus::kWorking:
            return "working";
        case TaskStatus::kCompleted:
            return "completed";
        case TaskStatus::kFailed:
            return "failed";
        case TaskStatus::kCancelled:
            return "cancelled";
        case TaskStatus::kInputRequired:
            return "input_required";
        default:
            return "working";
    }
}

inline TaskStatus taskStatusFromString(const std::string& status) {
    if (status == "working") return TaskStatus::kWorking;
    if (status == "completed") return TaskStatus::kCompleted;
    if (status == "failed") return TaskStatus::kFailed;
    if (status == "cancelled") return TaskStatus::kCancelled;
    if (status == "input_required") return TaskStatus::kInputRequired;
    return TaskStatus::kWorking;
}

// TaskMetadata (also defined in ToolCallParams, but standalone version for
// general use)
struct TaskMetadata {
    std::optional<long long> ttl_ms;

    json toJson() const {
        json j = json::object();
        if (ttl_ms.has_value()) j["ttl"] = ttl_ms.value();
        return j;
    }

    static TaskMetadata fromJson(const json& j) {
        TaskMetadata metadata;
        if (j.contains("ttl") && j["ttl"].is_number_integer()) {
            metadata.ttl_ms = j["ttl"].get<long long>();
        }
        return metadata;
    }

    bool empty() const { return !ttl_ms.has_value(); }
};

// Task (new in 2025-11-25)
struct Task {
    std::string taskId;
    TaskStatus status;
    std::string createdAt;
    std::string lastUpdatedAt;
    std::string statusMessage;
    std::optional<long long> ttl_ms;
    std::optional<long long> pollInterval_ms;

    json toJson() const {
        json j = {{"taskId", taskId},
                  {"status", taskStatusToString(status)},
                  {"createdAt", createdAt},
                  {"lastUpdatedAt", lastUpdatedAt}};
        if (!statusMessage.empty()) j["statusMessage"] = statusMessage;
        if (ttl_ms.has_value()) {
            j["ttl"] = ttl_ms.value();
        } else {
            j["ttl"] = nullptr;
        }
        if (pollInterval_ms.has_value()) j["pollInterval"] = pollInterval_ms.value();
        return j;
    }

    static Task fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid task payload: root must be an object", j));
        }
        if (!j.contains("taskId") || !j["taskId"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid task payload: missing or invalid field 'taskId'",
                    j));
        }
        if (!j.contains("status") || !j["status"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid task payload: missing or invalid field 'status'",
                    j));
        }

        Task task;
        task.taskId = j["taskId"].get<std::string>();
        task.status = taskStatusFromString(j["status"].get<std::string>());
        task.createdAt = j.value("createdAt", "");
        task.lastUpdatedAt = j.value("lastUpdatedAt", "");
        task.statusMessage = j.value("statusMessage", "");
        if (j.contains("ttl") && j["ttl"].is_number_integer()) {
            task.ttl_ms = j["ttl"].get<long long>();
        } else {
            task.ttl_ms = std::nullopt;
        }
        if (j.contains("pollInterval") && j["pollInterval"].is_number_integer()) {
            task.pollInterval_ms = j["pollInterval"].get<long long>();
        } else {
            task.pollInterval_ms = std::nullopt;
        }
        return task;
    }
};

// Elicitation Types (new in 2025-11-25)
struct ElicitRequestURLParams {
    std::string mode = "url";
    std::string url;
    std::string message;

    json toJson() const {
        return {{"mode", mode}, {"url", url}, {"message", message}};
    }

    static ElicitRequestURLParams fromJson(const json& j) {
        ElicitRequestURLParams params;
        params.mode = j.value("mode", "url");
        params.url = j.value("url", "");
        params.message = j.value("message", "");
        return params;
    }
};

struct ElicitRequestFormParams {
    std::string mode = "form";
    std::string message;
    json fields;

    json toJson() const {
        return {{"mode", mode}, {"message", message}, {"fields", fields}};
    }

    static ElicitRequestFormParams fromJson(const json& j) {
        ElicitRequestFormParams params;
        params.mode = j.value("mode", "form");
        params.message = j.value("message", "");
        params.fields = j.value("fields", json::object());
        return params;
    }
};

struct ElicitCreateResult {
    std::string action;  // "accept" | "decline" | "cancel"
    json content;        // Only present when action="accept" and mode="form"

    json toJson() const {
        json j = {{"action", action}};
        if (!content.empty() && action == "accept") {
            j["content"] = content;
        }
        return j;
    }

    static ElicitCreateResult fromJson(const json& j) {
        ElicitCreateResult result;
        result.action = j.value("action", "cancel");
        if (j.contains("content")) {
            result.content = j["content"];
        }
        return result;
    }
};


// Progress Notification Parameters (server -> client)
struct ProgressParams {
    json progressToken;
    double progress = 0.0;
    double total = 0.0;
    std::string message;

    json toJson() const {
        json j;
        j["progressToken"] = progressToken;
        j["progress"] = progress;
        if (total > 0.0) j["total"] = total;
        if (!message.empty()) j["message"] = message;
        return j;
    }

    static ProgressParams fromJson(const json& j) {
        ProgressParams p;
        if (j.contains("progressToken")) p.progressToken = j["progressToken"];
        p.progress = j.value("progress", 0.0);
        p.total = j.value("total", 0.0);
        p.message = j.value("message", "");
        return p;
    }
};

// Cancel Request Parameters (client -> server)
struct CancelParams {
    json requestId;
    std::string reason;

    json toJson() const {
        json j;
        j["requestId"] = requestId;
        if (!reason.empty()) j["reason"] = reason;
        return j;
    }

    static CancelParams fromJson(const json& j) {
        CancelParams p;
        if (j.contains("requestId")) p.requestId = j["requestId"];
        p.reason = j.value("reason", "");
        return p;
    }
};

// ========== Strong-typed result structures ==========

// ResourceContent — single item returned by resources/read
struct ResourceContent {
    std::string uri;
    std::string mimeType;
    std::string text;  // text content (mutually exclusive with blob)
    std::string blob;  // base64 binary (mutually exclusive with text)

    json toJson() const {
        json j = {{"uri", uri}};
        if (!mimeType.empty()) j["mimeType"] = mimeType;
        if (!text.empty()) j["text"] = text;
        if (!blob.empty()) j["blob"] = blob;
        return j;
    }

    static ResourceContent fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read content item: must be an object", j));
        }
        if (!j.contains("uri") || !j["uri"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read content item: missing or invalid field 'uri'",
                    j));
        }
        if (j.contains("mimeType") && !j["mimeType"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read content item: field 'mimeType' must be a string",
                    j));
        }
        if (j.contains("text") && !j["text"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read content item: field 'text' must be a string",
                    j));
        }
        if (j.contains("blob") && !j["blob"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read content item: field 'blob' must be a string",
                    j));
        }

        ResourceContent c;
        c.uri = j["uri"].get<std::string>();
        c.mimeType = j.value("mimeType", "");
        c.text = j.value("text", "");
        c.blob = j.value("blob", "");
        return c;
    }
};

// ReadResourceResult — resources/read return value
struct ReadResourceResult {
    std::vector<ResourceContent> contents;

    json toJson() const {
        json arr = json::array();
        for (const auto& c : contents) {
            arr.push_back(c.toJson());
        }
        return json{{"contents", arr}};
    }

    static ReadResourceResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read payload: root must be an object", j));
        }

        ReadResourceResult r;
        if (!j.contains("contents") || !j["contents"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/read payload: missing or invalid field 'contents'",
                    j));
        }
        for (const auto& item : j["contents"]) {
            r.contents.push_back(ResourceContent::fromJson(item));
        }
        return r;
    }
};

// PromptMessage — single message in a prompt response
struct PromptMessage {
    Role role = Role::kAssistant;
    json content;  // text/image/resource mixed content, kept as json

    json toJson() const {
        return {{"role", roleToString(role)}, {"content", content}};
    }

    static PromptMessage fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get message item: must be an object", j));
        }
        if (!j.contains("role") || !j["role"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get message item: missing or invalid field 'role'",
                    j));
        }
        if (!j.contains("content")) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get message item: missing field 'content'",
                    j));
        }

        PromptMessage m;
        m.role = roleFromString(j["role"].get<std::string>());
        m.content = j["content"];
        return m;
    }
};

// PromptResult — prompts/get return value
struct PromptResult {
    std::string description;
    std::vector<PromptMessage> messages;

    json toJson() const {
        json msgs = json::array();
        for (const auto& m : messages) {
            msgs.push_back(m.toJson());
        }
        json j;
        if (!description.empty()) j["description"] = description;
        j["messages"] = msgs;
        return j;
    }

    static PromptResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get payload: root must be an object", j));
        }

        PromptResult r;
        r.description = j.value("description", "");
        if (j.contains("description") && !j["description"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get payload: field 'description' must be a string",
                    j));
        }
        if (!j.contains("messages") || !j["messages"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/get payload: missing or invalid field 'messages'",
                    j));
        }
        for (const auto& item : j["messages"]) {
            r.messages.push_back(PromptMessage::fromJson(item));
        }
        return r;
    }
};

// ListToolsResult — tools/list return value
struct ListToolsResult {
    std::vector<Tool> tools;
    std::string nextCursor;  // empty means no more pages

    json toJson() const {
        json arr = json::array();
        for (const auto& t : tools) {
            arr.push_back(t.toJson());
        }
        json j = {{"tools", arr}};
        if (!nextCursor.empty()) j["nextCursor"] = nextCursor;
        return j;
    }

    static ListToolsResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tools/list payload: root must be an object", j));
        }

        ListToolsResult r;
        if (!j.contains("tools") || !j["tools"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tools/list payload: missing or invalid field 'tools'",
                    j));
        }
        for (const auto& item : j["tools"]) {
            r.tools.push_back(Tool::fromJson(item));
        }
        if (j.contains("nextCursor") && !j["nextCursor"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tools/list payload: field 'nextCursor' must be a string",
                    j));
        }
        r.nextCursor = j.value("nextCursor", "");
        return r;
    }
};

// ListResourcesResult — resources/list return value
struct ListResourcesResult {
    std::vector<Resource> resources;
    std::string nextCursor;  // empty means no more pages

    json toJson() const {
        json arr = json::array();
        for (const auto& r : resources) {
            arr.push_back(r.toJson());
        }
        json j = {{"resources", arr}};
        if (!nextCursor.empty()) j["nextCursor"] = nextCursor;
        return j;
    }

    static ListResourcesResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/list payload: root must be an object", j));
        }

        ListResourcesResult r;
        if (!j.contains("resources") || !j["resources"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/list payload: missing or invalid field 'resources'",
                    j));
        }
        for (const auto& item : j["resources"]) {
            r.resources.push_back(Resource::fromJson(item));
        }
        if (j.contains("nextCursor") && !j["nextCursor"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/list payload: field 'nextCursor' must be a string",
                    j));
        }
        r.nextCursor = j.value("nextCursor", "");
        return r;
    }
};

// ListResourceTemplatesResult — resources/templates/list return value
struct ListResourceTemplatesResult {
    std::vector<ResourceTemplate> resourceTemplates;
    std::string nextCursor;  // empty means no more pages

    json toJson() const {
        json arr = json::array();
        for (const auto& t : resourceTemplates) {
            arr.push_back(t.toJson());
        }
        json j = {{"resourceTemplates", arr}};
        if (!nextCursor.empty()) j["nextCursor"] = nextCursor;
        return j;
    }

    static ListResourceTemplatesResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/templates/list payload: root must be an object",
                    j));
        }

        ListResourceTemplatesResult r;
        if (!j.contains("resourceTemplates") ||
            !j["resourceTemplates"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/templates/list payload: missing or invalid field 'resourceTemplates'",
                    j));
        }
        for (const auto& item : j["resourceTemplates"]) {
            r.resourceTemplates.push_back(ResourceTemplate::fromJson(item));
        }
        if (j.contains("nextCursor") && !j["nextCursor"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid resources/templates/list payload: field 'nextCursor' must be a string",
                    j));
        }
        r.nextCursor = j.value("nextCursor", "");
        return r;
    }
};

// ListPromptsResult — prompts/list return value
struct ListPromptsResult {
    std::vector<Prompt> prompts;
    std::string nextCursor;  // empty means no more pages

    json toJson() const {
        json arr = json::array();
        for (const auto& p : prompts) {
            arr.push_back(p.toJson());
        }
        json j = {{"prompts", arr}};
        if (!nextCursor.empty()) j["nextCursor"] = nextCursor;
        return j;
    }

    static ListPromptsResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/list payload: root must be an object", j));
        }

        ListPromptsResult r;
        if (!j.contains("prompts") || !j["prompts"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/list payload: missing or invalid field 'prompts'",
                    j));
        }
        for (const auto& item : j["prompts"]) {
            r.prompts.push_back(Prompt::fromJson(item));
        }
        if (j.contains("nextCursor") && !j["nextCursor"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid prompts/list payload: field 'nextCursor' must be a string",
                    j));
        }
        r.nextCursor = j.value("nextCursor", "");
        return r;
    }
};

// ListTasksResult — tasks/list return value
struct ListTasksResult {
    std::vector<Task> tasks;
    std::string nextCursor;  // empty means no more pages

    json toJson() const {
        json arr = json::array();
        for (const auto& t : tasks) {
            arr.push_back(t.toJson());
        }
        json j = {{"tasks", arr}};
        if (!nextCursor.empty()) j["nextCursor"] = nextCursor;
        return j;
    }

    static ListTasksResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tasks/list payload: root must be an object", j));
        }

        ListTasksResult r;
        if (!j.contains("tasks") || !j["tasks"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tasks/list payload: missing or invalid field 'tasks'",
                    j));
        }
        for (const auto& item : j["tasks"]) {
            r.tasks.push_back(Task::fromJson(item));
        }
        if (j.contains("nextCursor") && !j["nextCursor"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid tasks/list payload: field 'nextCursor' must be a string",
                    j));
        }
        r.nextCursor = j.value("nextCursor", "");
        return r;
    }
};

// TaskResult — tasks/result return value
struct TaskResult {
    std::string taskId;
    json result = json::object();

    json toJson() const {
        return {{"taskId", taskId}, {"result", result}};
    }

    static TaskResult fromJson(const json& j) {
        TaskResult r;
        if (j.is_object() && j.contains("taskId") && j["taskId"].is_string() &&
            j.contains("result")) {
            // Backward-compatible wrapped payload
            r.taskId = j["taskId"].get<std::string>();
            r.result = j["result"];
        } else {
            // MCP 2025-11-25 compliant payload: raw original result payload
            r.result = j;
        }
        return r;
    }
};

// CompleteResult — completion/complete return value
struct CompleteResult {
    std::vector<std::string> values;
    bool hasMore = false;
    int total = 0;

    json toJson() const {
        json j;
        j["completion"] = json{{"values", values}};
        if (hasMore) j["completion"]["hasMore"] = true;
        if (total > 0) j["completion"]["total"] = total;
        return j;
    }

    static CompleteResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid completion/complete payload: root must be an object",
                    j));
        }

        CompleteResult r;
        json comp = j;
        if (j.contains("completion")) comp = j["completion"];
        if (!comp.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid completion/complete payload: field 'completion' must be an object",
                    j));
        }
        if (!comp.contains("values") || !comp["values"].is_array()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid completion/complete payload: missing or invalid field 'values'",
                    j));
        }
        if (comp.contains("hasMore") && !comp["hasMore"].is_boolean()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid completion/complete payload: field 'hasMore' must be a boolean",
                    j));
        }
        if (comp.contains("total") && !comp["total"].is_number_integer()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid completion/complete payload: field 'total' must be an integer",
                    j));
        }
        for (const auto& v : comp["values"]) {
            if (v.is_string()) {
                r.values.push_back(v.get<std::string>());
            } else if (v.is_object() && v.contains("value") &&
                       v["value"].is_string()) {
                // Accept richer completion item objects that contain
                // the canonical string value field.
                r.values.push_back(v["value"].get<std::string>());
            } else {
                throw mcp_exception(
                    ErrorCode::kInternalError,
                    protocol_detail::invalidPayloadMessage(
                        "Invalid completion/complete payload: value item must be a string or object with string field 'value'",
                        v));
            }
        }
        r.hasMore = comp.value("hasMore", false);
        r.total = comp.value("total", 0);
        return r;
    }
};

// CreateMessageResult — sampling/createMessage return value
struct CreateMessageResult {
    Role role = Role::kAssistant;
    json content;  // mixed content types, kept as json
    std::string model;
    std::string stopReason;

    json toJson() const {
        json j = {{"role", roleToString(role)}, {"content", content}};
        if (!model.empty()) j["model"] = model;
        if (!stopReason.empty()) j["stopReason"] = stopReason;
        return j;
    }

    static CreateMessageResult fromJson(const json& j) {
        if (!j.is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid sampling/createMessage payload: root must be an object",
                    j));
        }
        if (!j.contains("role") || !j["role"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid sampling/createMessage payload: missing or invalid field 'role'",
                    j));
        }
        if (!j.contains("content")) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid sampling/createMessage payload: missing field 'content'",
                    j));
        }
        if (j.contains("model") && !j["model"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid sampling/createMessage payload: field 'model' must be a string",
                    j));
        }
        if (j.contains("stopReason") && !j["stopReason"].is_string()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                protocol_detail::invalidPayloadMessage(
                    "Invalid sampling/createMessage payload: field 'stopReason' must be a string",
                    j));
        }

        CreateMessageResult r;
        r.role = roleFromString(j["role"].get<std::string>());
        r.content = j["content"];
        r.model = j.value("model", "");
        r.stopReason = j.value("stopReason", "");
        return r;
    }
};

// Notification Handlers

/// Callback for handling arbitrary server notifications
using NotificationHandler =
    std::function<void(const std::string& method, const json& params)>;

/// Callback for handling progress notifications
using ProgressHandler = std::function<void(const ProgressParams& params)>;

/// Callback for handling server-initiated tool calls
using ToolCallHandler =
    std::function<ToolCallResult(const ToolCallParams& params)>;

/// Callback for handling server-initiated resource read requests
using ResourceReadHandler = std::function<ReadResourceResult(const std::string& uri)>;

/// Callback for handling server-initiated prompt get requests
using PromptGetHandler =
    std::function<PromptResult(const std::string& name, const json& arguments)>;

/// Callback for handling client roots list changes
using RootsListChangedHandler = std::function<void(const std::vector<Root>& roots)>;

/// Callback for handling client cancel requests
using CancelHandler = std::function<void(const CancelParams& params)>;

/// Callback for handling server-initiated elicitation requests
using ElicitationHandler = std::function<ElicitCreateResult(const json& params)>;

/// Callback for handling server-initiated sampling requests
using SamplingHandler = std::function<CreateMessageResult(const json& params)>;

// Utility functions

/// Transport utility functions
namespace utils {

/**
 * @brief Create default client capabilities
 * @return Default ClientCapabilities structure
 */
ClientCapabilities createDefaultClientCapabilities();

/**
 * @brief Create default server capabilities
 * @return Default ServerCapabilities structure
 */
ServerCapabilities createDefaultServerCapabilities();

/**
 * @brief Validate protocol version string
 * @param version Version string to validate
 * @return true if version is supported
 */
bool isValidProtocolVersion(const std::string& version);

/**
 * @brief Create a Tool from JSON schema
 * @param name Tool name
 * @param description Tool description
 * @param schema JSON schema for tool input
 * @return Tool structure
 */
Tool createToolFromSchema(const std::string& name,
                          const std::string& description, const json& schema);

/**
 * @brief Create a Resource from URI
 * @param uri Resource URI
 * @param name Optional resource name
 * @param description Optional resource description
 * @return Resource structure
 */
Resource createResourceFromUri(const std::string& uri,
                               const std::string& name = "",
                               const std::string& description = "");

}  // namespace utils

}  // namespace mcp

#endif  // KYSDK_MCP_PROTOCOL_COMMON_H
