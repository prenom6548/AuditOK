#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>

#include "../log/logger.h"
#include "common/message.h"
#include "common/protocol.h"

namespace mcp {
namespace internal {

class MCPServerHandlerContext {
public:
    virtual ~MCPServerHandlerContext() = default;

    virtual bool isRunning() const = 0;
    virtual InitializeResult initialize(const InitializeParams& params) = 0;
    virtual void ping() = 0;
    virtual std::vector<Tool> listTools() const = 0;
    virtual ToolCallResult callTool(const ToolCallParams& params) = 0;
    virtual std::vector<Resource> listResources() const = 0;
    virtual std::vector<ResourceTemplate> listResourceTemplates() const = 0;
    virtual ReadResourceResult readResource(const std::string& uri) = 0;
    virtual void subscribeResource(const std::string& uri) = 0;
    virtual void unsubscribeResource(const std::string& uri) = 0;
    virtual std::vector<Prompt> listPrompts() const = 0;
    virtual PromptResult getPrompt(const std::string& name, const json& arguments) = 0;
    virtual Task getTask(const std::string& task_id) = 0;
    virtual TaskResult getTaskResult(const std::string& task_id) = 0;
    virtual ListTasksResult listTasks(const std::string& cursor) = 0;
    virtual Task cancelTask(const std::string& task_id) = 0;
    virtual ElicitCreateResult createElicitation(const json& params) = 0;
    virtual CompleteResult complete(const json& params) = 0;
    virtual CreateMessageResult createMessage(const json& params) = 0;
    virtual void setLoggingLevel(const json& params) = 0;
    virtual void initialized() = 0;
    virtual const ServerInfo& getServerInfo() const = 0;
    virtual void handleCancelRequest(const CancelParams& params) = 0;
    virtual void handleRootsListChanged() = 0;
};

inline std::string toLowerCopy(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

inline mcp_logger::LogLevel parseLogLevel(const json& params) {
    if (!params.is_object()) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "logging/setLevel params must be an object");
    }

    if (!params.contains("level")) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "logging/setLevel requires field: level");
    }

    const json& level = params["level"];
    if (level.is_number_integer()) {
        switch (level.get<int>()) {
            case 0:
                return mcp_logger::LogLevel::DEBUG;
            case 1:
                return mcp_logger::LogLevel::INFO;
            case 2:
                return mcp_logger::LogLevel::WARNING;
            case 3:
                return mcp_logger::LogLevel::ERROR;
            case 4:
                return mcp_logger::LogLevel::FATAL;
            default:
                break;
        }
    } else if (level.is_string()) {
        const std::string normalized = toLowerCopy(level.get<std::string>());
        if (normalized == "debug") {
            return mcp_logger::LogLevel::DEBUG;
        }
        if (normalized == "info") {
            return mcp_logger::LogLevel::INFO;
        }
        if (normalized == "warning" || normalized == "warn") {
            return mcp_logger::LogLevel::WARNING;
        }
        if (normalized == "notice") {
            return mcp_logger::LogLevel::INFO;
        }
        if (normalized == "error") {
            return mcp_logger::LogLevel::ERROR;
        }
        if (normalized == "critical" || normalized == "alert" ||
            normalized == "emergency") {
            return mcp_logger::LogLevel::FATAL;
        }
        if (normalized == "fatal") {
            return mcp_logger::LogLevel::FATAL;
        }
    }

    throw mcp_exception(ErrorCode::kInvalidParams, "Unsupported logging level");
}

inline std::string logLevelToString(mcp_logger::LogLevel level) {
    switch (level) {
        case mcp_logger::LogLevel::DEBUG:
            return "debug";
        case mcp_logger::LogLevel::INFO:
            return "info";
        case mcp_logger::LogLevel::WARNING:
            return "warning";
        case mcp_logger::LogLevel::ERROR:
            return "error";
        case mcp_logger::LogLevel::FATAL:
            return "fatal";
    }

    return "info";
}

inline json parseParamsOrObject(const std::string& params) {
    if (params.empty()) {
        return json::object();
    }

    try {
        return json::parse(params);
    } catch (const std::exception& e) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "Invalid params JSON: " + std::string(e.what()));
    }
}

inline std::size_t parseCursorOffset(const json& params) {
    if (!params.contains("cursor")) {
        return 0;
    }

    if (!params["cursor"].is_string()) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "cursor must be a string");
    }

    const std::string cursor = params["cursor"].get<std::string>();
    if (cursor.empty()) {
        return 0;
    }

    std::size_t offset = 0;
    try {
        offset = static_cast<std::size_t>(std::stoull(cursor));
    } catch (const std::exception&) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "cursor must be a decimal offset string");
    }
    return offset;
}

inline std::size_t parsePageLimit(const json& params, bool has_cursor) {
    constexpr std::size_t kDefaultPageSize = 50;
    constexpr std::size_t kMaxPageSize = 500;
    if (!params.contains("limit")) {
        return has_cursor ? kDefaultPageSize
                          : std::numeric_limits<std::size_t>::max();
    }

    if (!params["limit"].is_number_unsigned() &&
        !params["limit"].is_number_integer()) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "limit must be an integer");
    }

    const int64_t raw_limit = params["limit"].get<int64_t>();
    if (raw_limit <= 0) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "limit must be greater than zero");
    }

    return std::min<std::size_t>(static_cast<std::size_t>(raw_limit),
                                 kMaxPageSize);
}

inline json paginateArray(const json& full_array, std::size_t offset,
                          std::size_t limit) {
    json page = json::array();
    if (offset >= full_array.size()) {
        return page;
    }

    const std::size_t end =
        std::min<std::size_t>(full_array.size(), offset + limit);
    for (std::size_t i = offset; i < end; ++i) {
        page.push_back(full_array[i]);
    }
    return page;
}

inline void validateCompleteParams(const json& params) {
    if (!params.is_object()) {
        throw mcp_exception(ErrorCode::kInvalidParams,
                            "completion/complete params must be an object");
    }

    if (!params.contains("ref")) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete requires field: ref");
    }
    const json& ref = params["ref"];
    if (!ref.is_object()) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete field 'ref' must be an object");
    }
    if (!ref.contains("type") || !ref["type"].is_string() ||
        ref["type"].get<std::string>().empty()) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete field 'ref.type' must be a non-empty string");
    }
    const std::string ref_type = ref["type"].get<std::string>();
    if (ref_type == "ref/prompt") {
        if (!ref.contains("name") || !ref["name"].is_string() ||
            ref["name"].get<std::string>().empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "completion/complete ref/prompt requires field: ref.name "
                "(non-empty string)");
        }
    } else if (ref_type == "ref/resource") {
        if (!ref.contains("uri") || !ref["uri"].is_string() ||
            ref["uri"].get<std::string>().empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "completion/complete ref/resource requires field: ref.uri "
                "(non-empty string)");
        }
    } else {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete field 'ref.type' must be 'ref/prompt' or "
            "'ref/resource'");
    }

    if (!params.contains("argument")) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete requires field: argument");
    }
    const json& argument = params["argument"];
    if (!argument.is_object()) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete field 'argument' must be an object");
    }
    if (!argument.contains("name") || !argument["name"].is_string() ||
        argument["name"].get<std::string>().empty()) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete requires field: argument.name (non-empty "
            "string)");
    }
    if (!argument.contains("value") || !argument["value"].is_string()) {
        throw mcp_exception(
            ErrorCode::kInvalidParams,
            "completion/complete requires field: argument.value (string)");
    }

    // MCP 2025-06-18 新增字段：可选 context.arguments（map<string,string>），
    // 用于把已填好的其他参数传给服务端做更精准的补全。当前字段可选，
    // 但一旦客户端传了，结构必须正确。
    if (params.contains("context")) {
        const json& ctx = params["context"];
        if (!ctx.is_object()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "completion/complete field 'context' must be an object");
        }
        if (ctx.contains("arguments")) {
            const json& ca = ctx["arguments"];
            if (!ca.is_object()) {
                throw mcp_exception(
                    ErrorCode::kInvalidParams,
                    "completion/complete field 'context.arguments' must be an "
                    "object");
            }
            for (auto it = ca.begin(); it != ca.end(); ++it) {
                if (!it.value().is_string()) {
                    throw mcp_exception(
                        ErrorCode::kInvalidParams,
                        "completion/complete 'context.arguments' values must "
                        "be strings");
                }
            }
        }
    }
}

inline std::string handleServerRequest(MCPServerHandlerContext& context,
                                       const std::string& method,
                                       const std::string& params,
                                       const std::string& request_id) {
    (void)request_id;

    if (method == methods::INITIALIZE) {
        return context
            .initialize(InitializeParams::fromJson(parseParamsOrObject(params)))
            .toJson()
            .dump();
    }

    if (method == methods::PING) {
        context.ping();
        return json::object().dump();
    }

    if (method == methods::TOOLS_LIST) {
        const json request_params = parseParamsOrObject(params);
        const bool has_cursor = request_params.contains("cursor");
        const std::size_t offset = parseCursorOffset(request_params);
        const std::size_t limit = parsePageLimit(request_params, has_cursor);

        json tools = json::array();
        for (const auto& tool : context.listTools()) {
            tools.push_back(tool.toJson());
        }

        json result;
        result["tools"] = paginateArray(tools, offset, limit);
        const std::size_t next_offset = offset + result["tools"].size();
        if (next_offset < tools.size()) {
            result["nextCursor"] = std::to_string(next_offset);
        }
        return result.dump();
    }

    if (method == methods::TOOLS_CALL) {
        const json tool_params = parseParamsOrObject(params);
        if (!tool_params.is_object() || !tool_params.contains("name") ||
            !tool_params["name"].is_string() ||
            tool_params["name"].get<std::string>().empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "tools/call requires field: name (non-empty string)");
        }
        if (tool_params.contains("arguments") &&
            !tool_params["arguments"].is_object()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "tools/call field 'arguments' must be an object");
        }
        return context
            .callTool(ToolCallParams::fromJson(tool_params))
            .toJson()
            .dump();
    }

    if (method == methods::RESOURCES_LIST) {
        const json request_params = parseParamsOrObject(params);
        const bool has_cursor = request_params.contains("cursor");
        const std::size_t offset = parseCursorOffset(request_params);
        const std::size_t limit = parsePageLimit(request_params, has_cursor);

        json resources = json::array();
        for (const auto& resource : context.listResources()) {
            resources.push_back(resource.toJson());
        }

        json result;
        result["resources"] = paginateArray(resources, offset, limit);
        const std::size_t next_offset = offset + result["resources"].size();
        if (next_offset < resources.size()) {
            result["nextCursor"] = std::to_string(next_offset);
        }
        return result.dump();
    }

    if (method == methods::RESOURCES_READ) {
        const json resource_params = parseParamsOrObject(params);
        if (!resource_params.is_object() || !resource_params.contains("uri") ||
            !resource_params["uri"].is_string() ||
            resource_params["uri"].get<std::string>().empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "resources/read requires field: uri (non-empty string)");
        }
        return context
            .readResource(resource_params["uri"].get<std::string>())
            .toJson()
            .dump();
    }

    if (method == methods::RESOURCES_SUBSCRIBE) {
        const json resource_params = parseParamsOrObject(params);
        context.subscribeResource(resource_params.value("uri", ""));
        return json::object().dump();
    }

    if (method == methods::RESOURCES_UNSUBSCRIBE) {
        const json resource_params = parseParamsOrObject(params);
        context.unsubscribeResource(resource_params.value("uri", ""));
        return json::object().dump();
    }

    if (method == methods::RESOURCES_TEMPLATES_LIST) {
        const json request_params = parseParamsOrObject(params);
        const bool has_cursor = request_params.contains("cursor");
        const std::size_t offset = parseCursorOffset(request_params);
        const std::size_t limit = parsePageLimit(request_params, has_cursor);

        json result;
        json templates = json::array();
        for (const auto& t : context.listResourceTemplates()) {
            templates.push_back(t.toJson());
        }
        result["resourceTemplates"] =
            paginateArray(templates, offset, limit);
        const std::size_t next_offset =
            offset + result["resourceTemplates"].size();
        if (next_offset < templates.size()) {
            result["nextCursor"] = std::to_string(next_offset);
        }
        return result.dump();
    }

    if (method == methods::PROMPTS_LIST) {
        const json request_params = parseParamsOrObject(params);
        const bool has_cursor = request_params.contains("cursor");
        const std::size_t offset = parseCursorOffset(request_params);
        const std::size_t limit = parsePageLimit(request_params, has_cursor);

        json prompts = json::array();
        for (const auto& prompt : context.listPrompts()) {
            prompts.push_back(prompt.toJson());
        }

        json result;
        result["prompts"] = paginateArray(prompts, offset, limit);
        const std::size_t next_offset = offset + result["prompts"].size();
        if (next_offset < prompts.size()) {
            result["nextCursor"] = std::to_string(next_offset);
        }
        return result.dump();
    }

    if (method == methods::PROMPTS_GET) {
        const json prompt_params = parseParamsOrObject(params);
        if (!prompt_params.is_object() || !prompt_params.contains("name") ||
            !prompt_params["name"].is_string() ||
            prompt_params["name"].get<std::string>().empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "prompts/get requires field: name (non-empty string)");
        }
        if (prompt_params.contains("arguments") &&
            !prompt_params["arguments"].is_object()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "prompts/get field 'arguments' must be an object");
        }
        return context
            .getPrompt(prompt_params["name"].get<std::string>(),
                       prompt_params.value("arguments", json::object()))
            .toJson()
            .dump();
    }

    if (method == methods::TASKS_GET) {
        const json task_params = parseParamsOrObject(params);
        Task task = context.getTask(task_params.value("taskId", ""));
        return task.toJson().dump();
    }

    if (method == methods::TASKS_RESULT) {
        const json task_params = parseParamsOrObject(params);
        // MCP 2025-11-25 expects the original request result payload.
        // Keep backward compatibility by exposing legacy `taskId`/`result`
        // fields when result is an object.
        const TaskResult task_result =
            context.getTaskResult(task_params.value("taskId", ""));
        if (task_result.result.is_object()) {
            json compat = task_result.result;
            compat["taskId"] = task_result.taskId;
            compat["result"] = task_result.result;
            return compat.dump();
        }
        return json{{"taskId", task_result.taskId},
                    {"result", task_result.result}}
            .dump();
    }

    if (method == methods::TASKS_LIST) {
        const json task_params = parseParamsOrObject(params);
        return context.listTasks(task_params.value("cursor", ""))
            .toJson()
            .dump();
    }

    if (method == methods::TASKS_CANCEL) {
        const json task_params = parseParamsOrObject(params);
        Task task = context.cancelTask(task_params.value("taskId", ""));
        return task.toJson().dump();
    }

    if (method == methods::COMPLETION_COMPLETE) {
        const json complete_params = parseParamsOrObject(params);
        validateCompleteParams(complete_params);
        return context.complete(complete_params).toJson().dump();
    }

    if (method == methods::LOGGING_SET_LEVEL) {
        context.setLoggingLevel(parseParamsOrObject(params));
        return json::object().dump();
    }

    throw mcp_exception(ErrorCode::kMethodNotFound,
                        "Unknown method: " + method);
}

inline void handleServerNotification(MCPServerHandlerContext& context,
                                     const std::string& method,
                                     const std::string& params) {
    if (method == methods::NOTIFICATION_INITIALIZED) {
        context.initialized();
        return;
    }

    if (method == methods::NOTIFICATION_CANCELLED ||
        method == methods::CANCEL_REQUEST) {
        context.handleCancelRequest(
            CancelParams::fromJson(parseParamsOrObject(params)));
        return;
    }

    if (method == methods::NOTIFICATION_ROOTS_LIST_CHANGED) {
        context.handleRootsListChanged();
        return;
    }

    (void)parseParamsOrObject(params);
}

inline bool forwardServerNotification(
    bool running,
    const std::function<bool(const std::string&, const std::string&)>& sender,
    const std::string& method, const json& params) {
    if (!running || !sender) {
        return false;
    }

    return sender(method, params.dump());
}

inline bool isJsonRpcEnvelope(const json& payload) {
    if (!payload.is_object()) {
        return false;
    }

    if (!payload.contains("jsonrpc") || payload["jsonrpc"] != "2.0") {
        return false;
    }

    return payload.contains("result") || payload.contains("error");
}

inline bool isJsonRpcPayload(const std::string& payload) {
    if (payload.empty()) {
        return false;
    }

    try {
        return isJsonRpcEnvelope(json::parse(payload));
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace internal
}  // namespace mcp
