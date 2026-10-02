/**
 * @file client.cpp
 * @brief MCP Client Implementation
 *
 * This file provides the implementation of the MCP client functionality.
 */

#include "client.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#include <utility>

#include "common/transport.h"
#include "logger.h"

// Logging macros
#define MCP_LOG_DEBUG(msg)                                           \
    mcp_logger::Logger::getInstance().debug(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define MCP_LOG_INFO(msg)                                           \
    mcp_logger::Logger::getInstance().info(msg, __FILE__, __LINE__, \
                                           __FUNCTION__)
#define MCP_LOG_WARNING(msg)                                           \
    mcp_logger::Logger::getInstance().warning(msg, __FILE__, __LINE__, \
                                              __FUNCTION__)
#define MCP_LOG_ERROR(msg)                                           \
    mcp_logger::Logger::getInstance().error(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define MCP_LOG_FATAL(msg)                                           \
    mcp_logger::Logger::getInstance().fatal(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)

namespace mcp {

namespace {
constexpr const char* kMcpProtocolVersionHeader = "MCP-Protocol-Version";
constexpr const char* kMcpProtocolVersionHeaderLower = "mcp-protocol-version";
constexpr const char* kAcceptHeader = "Accept";
constexpr const char* kJsonAndEventStreamAcceptValue =
    "application/json, text/event-stream";

Task parseTaskFromResult(const json& result, const std::string& action) {
    const json* task_payload = &result;
    if (result.is_object() && result.contains("task")) {
        if (!result["task"].is_object()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Invalid task payload in " + action +
                    " response: field 'task' must be an object");
        }
        task_payload = &result["task"];
    }

    if (!task_payload->is_object()) {
        throw mcp_exception(ErrorCode::kInternalError,
                            "Invalid task payload in " + action +
                                " response: " + result.dump());
    }

    return Task::fromJson(*task_payload);
}

std::vector<std::string> split_command_line(const std::string& command_line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '\0';
    bool escaping = false;

    for (char ch : command_line) {
        if (escaping) {
            current.push_back(ch);
            escaping = false;
            continue;
        }

        if (ch == '\\') {
            escaping = true;
            continue;
        }

        if (in_quotes) {
            if (ch == quote_char) {
                in_quotes = false;
            } else {
                current.push_back(ch);
            }
            continue;
        }

        if (ch == '"' || ch == '\'') {
            in_quotes = true;
            quote_char = ch;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}
}  // namespace

// MCP Client Implementation
class MCPClient::Impl {
public:
    Impl()
        : connected_(false),
          transport_type_(mcp_transport::TransportType::kAuto),
          current_endpoint_("/"),
          current_endpoint_index_(0) {
        MCP_LOG_DEBUG("MCPClient::Impl created");

        try {
            Root root;
            root.uri = "file://" + std::filesystem::current_path().string();
            root.name = "workspace";
            roots_.push_back(root);
        } catch (const std::exception& e) {
            MCP_LOG_WARNING("Failed to initialize default roots: " +
                            std::string(e.what()));
        }

        configureTransportCallbacks();
    }

    void configureTransportCallbacks() {
        transport_.setMessageCallback(
            [this](const mcp_transport::TransportMessage& message) {
                handle_transport_message(message);
            });

        transport_.setConnectionCallback(
            [this](const mcp_transport::ConnectionInfo& info) {
                MCP_LOG_INFO("Transport connection status changed: " +
                             std::to_string(static_cast<int>(info.status)));
                const bool was_connected = connected_;
                connected_ =
                    (info.status == mcp_transport::TransportStatus::kConnected);
                if (connected_ && !was_connected && !connecting_) {
                    std::thread([this]() {
                        runReconnectRecovery(true, "");
                    }).detach();
                }
            });

        transport_.setErrorCallback([this](const std::string& error) {
            MCP_LOG_ERROR("Transport error: " + error);
            connected_ = false;
            if (!connecting_ && transport_.getStatus() ==
                                    mcp_transport::TransportStatus::kError) {
                std::thread([this, error]() {
                    runReconnectRecovery(false, error);
                }).detach();
            }
        });
    }

    ~Impl() {
        disconnect();
        MCP_LOG_DEBUG("MCPClient::Impl destroyed");
    }

    bool connect(const std::string& endpoint) {
        MCP_LOG_INFO("Connecting to MCP server at: " + endpoint);

        // Parse endpoint and create transport config
        mcp_transport::TransportConfig config =
            parse_endpoint(endpoint, transport_type_);
        return connectWithConfig(config);
    }

    bool connectWithConfig(const mcp_transport::TransportConfig& config) {
        MCP_LOG_INFO("Connecting with transport config");

        mcp_transport::TransportConfig effective_config = config;
        if (!effective_config.validate()) {
            MCP_LOG_ERROR(
                "Transport config validation failed; refusing to connect");
            connected_ = false;
            connecting_ = false;
            return false;
        }
        if (effective_config.type == mcp_transport::TransportType::kSse) {
            // Keep protocol version header available for connect probe and SSE
            // stream request before initialize.
            effective_config.sse.headers[kMcpProtocolVersionHeader] =
                negotiated_protocol_version_;
            effective_config.sse.headers[kMcpProtocolVersionHeaderLower] =
                negotiated_protocol_version_;
        }

        // Set transport type
        transport_type_ = effective_config.type;
        last_config_ = effective_config;
        has_last_config_ = true;

        // Start transport
        configureTransportCallbacks();
        connecting_ = true;
        if (!transport_.start(effective_config)) {
            MCP_LOG_ERROR("Failed to start transport");
            connected_ = false;
            connecting_ = false;
            return false;
        }
        connecting_ = false;

        connected_ = true;
        MCP_LOG_INFO("Connected to MCP server successfully");
        return true;
    }

    void disconnect() {
        if (connected_) {
            MCP_LOG_INFO("Disconnecting from MCP server");
            try {
                // Detach callbacks before stopping transport to avoid late
                // callbacks touching this object during teardown.
                transport_.setMessageCallback(nullptr);
                transport_.setConnectionCallback(nullptr);
                transport_.setErrorCallback(nullptr);
                transport_.stop();
            } catch (const std::exception& e) {
                MCP_LOG_ERROR(std::string("Transport stop threw exception: ") +
                              e.what());
            } catch (...) {
                MCP_LOG_ERROR(
                    "Transport stop threw unknown exception during disconnect");
            }
            connected_ = false;
            MCP_LOG_INFO("Disconnected from MCP server");
        }
    }

    bool reconnect() {
        mcp_transport::TransportConfig config;
        bool has_config = false;
        {
            std::lock_guard<std::mutex> lock(recovery_mutex_);
            has_config = has_last_config_;
            if (has_config) {
                config = last_config_;
            }
        }
        if (!has_config) {
            runReconnectRecovery(false, "No previous transport config");
            return false;
        }

        std::lock_guard<std::mutex> reconnect_lock(reconnect_mutex_);
        if (connected_ || transport_.getStatus() !=
                              mcp_transport::TransportStatus::kDisconnected) {
            disconnect();
        }

        const bool transport_ok = connectWithConfig(config);
        if (!transport_ok) {
            runReconnectRecovery(false, "Transport reconnect failed");
            return false;
        }

        return runReconnectRecovery(true, "");
    }

    void setReconnectHandler(ReconnectHandler handler) {
        std::lock_guard<std::mutex> lock(recovery_mutex_);
        reconnect_handler_ = std::move(handler);
    }

    bool isConnected() const { return connected_ && transport_.isRunning(); }

    void setTransportType(mcp_transport::TransportType type) {
        transport_type_ = type;
        MCP_LOG_DEBUG("Transport type set to: " +
                      std::to_string(static_cast<int>(type)));
    }

    mcp_transport::TransportType getTransportType() const {
        return transport_type_;
    }

    mcp_transport::TransportStatus getTransportStatus() const {
        return transport_.getStatus();
    }

    InitializeResult initialize(const InitializeParams& params) {
        MCP_LOG_INFO("Initializing MCP client");
        InitializeParams effective_params = params;
        if (effective_params.clientInfo.name.empty() &&
            !client_info_.name.empty()) {
            effective_params.clientInfo = client_info_;
        }
        if (effective_params.capabilities.toJson().empty() &&
            !client_capabilities_.toJson().empty()) {
            effective_params.capabilities = client_capabilities_;
        }

        // Validate protocol version
        if (!utils::isValidProtocolVersion(effective_params.protocolVersion)) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Unsupported protocol version: " +
                                    effective_params.protocolVersion);
        }

        // Create initialize request
        auto req =
            request::create(methods::INITIALIZE, effective_params.toJson());
        MCP_LOG_DEBUG("Sending initialize request: " + req.toJson().dump());

        // Send request and get response
        MCP_LOG_DEBUG("Sending initialize request: " + req.toJson().dump());

        auto response = send_request_internal(req);

        if (response.isError()) {
            std::string error_msg =
                "Initialize failed: " + response.error.dump();
            MCP_LOG_ERROR(error_msg);
            throw mcp_exception(ErrorCode::kInternalError, error_msg);
        }

        // Parse initialize result
        InitializeResult result = InitializeResult::fromJson(response.result);
        if (!result.protocolVersion.empty()) {
            negotiated_protocol_version_ = result.protocolVersion;
        }
        server_info_ = result.serverInfo;
        server_capabilities_ = result.capabilities;
        cached_init_params_ = effective_params;
        has_cached_init_params_ = true;

        MCP_LOG_INFO("MCP client initialized successfully");
        MCP_LOG_INFO("Server: " + server_info_.name + " v" +
                     server_info_.version);

        return result;
    }

    void initialized() {
        MCP_LOG_INFO("Sending initialized notification");
        auto notification =
            request::createNotification(methods::NOTIFICATION_INITIALIZED);
        send_notification_internal(notification);
    }

    void ping() {
        MCP_LOG_DEBUG("Sending ping request");
        auto req = request::create(methods::PING);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Ping failed: " + response.error.dump());
        }
        const bool is_empty_object =
            response.result.is_object() && response.result.empty();
        if (!response.result.is_null() && !is_empty_object) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Ping failed: invalid result payload, expected "
                                "null or empty object");
        }

        MCP_LOG_DEBUG("Ping successful");
    }

    ListToolsResult listTools(const std::string& cursor) {
        MCP_LOG_DEBUG("Requesting tools list");
        json params = json::object();
        if (!cursor.empty()) {
            params["cursor"] = cursor;
        }
        auto req = request::create(methods::TOOLS_LIST, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to list tools: " + response.error.dump());
        }

        ListToolsResult result = ListToolsResult::fromJson(response.result);
        MCP_LOG_INFO("Retrieved " + std::to_string(result.tools.size()) +
                     " tools");
        return result;
    }

    ToolCallResult callTool(const ToolCallParams& params) {
        MCP_LOG_INFO("Calling tool: " + params.name);
        auto req = request::create(methods::TOOLS_CALL, params.toJson());
        auto response = send_request_internal(req);

        if (response.isError()) {
            ToolCallResult result;
            result.isError = true;
            result.errorMessage = response.error.dump();
            MCP_LOG_ERROR("Tool call failed: " + result.errorMessage);
            return result;
        }

        ToolCallResult result = ToolCallResult::fromJson(response.result);
        MCP_LOG_INFO("Tool call completed successfully");
        return result;
    }

    ToolCallResult callTool(const std::string& name, const json& arguments) {
        MCP_LOG_INFO("Calling tool: " + name);
        ToolCallParams params;
        params.name = name;
        params.arguments = arguments;
        return callTool(params);
    }

    ListResourcesResult listResources(const std::string& cursor) {
        MCP_LOG_DEBUG("Requesting resources list");
        json params = json::object();
        if (!cursor.empty()) {
            params["cursor"] = cursor;
        }
        auto req = request::create(methods::RESOURCES_LIST, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to list resources: " + response.error.dump());
        }

        ListResourcesResult result =
            ListResourcesResult::fromJson(response.result);
        MCP_LOG_INFO("Retrieved " + std::to_string(result.resources.size()) +
                     " resources");
        return result;
    }

    ListResourceTemplatesResult listResourceTemplates(
        const std::string& cursor) {
        MCP_LOG_DEBUG("Requesting resource templates list");
        json params = json::object();
        if (!cursor.empty()) {
            params["cursor"] = cursor;
        }
        auto req = request::create(methods::RESOURCES_TEMPLATES_LIST, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to list resource templates: " + response.error.dump());
        }

        ListResourceTemplatesResult result =
            ListResourceTemplatesResult::fromJson(response.result);
        MCP_LOG_INFO("Retrieved " +
                     std::to_string(result.resourceTemplates.size()) +
                     " resource templates");
        return result;
    }

    ReadResourceResult readResource(const std::string& uri) {
        MCP_LOG_INFO("Reading resource: " + uri);
        auto req = request::create(methods::RESOURCES_READ, {{"uri", uri}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to read resource: " + response.error.dump());
        }

        MCP_LOG_INFO("Resource read successfully");
        return ReadResourceResult::fromJson(response.result);
    }

    ListPromptsResult listPrompts(const std::string& cursor) {
        MCP_LOG_DEBUG("Requesting prompts list");
        json params = json::object();
        if (!cursor.empty()) {
            params["cursor"] = cursor;
        }
        auto req = request::create(methods::PROMPTS_LIST, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to list prompts: " + response.error.dump());
        }

        ListPromptsResult result = ListPromptsResult::fromJson(response.result);
        MCP_LOG_INFO("Retrieved " + std::to_string(result.prompts.size()) +
                     " prompts");
        return result;
    }

    PromptResult getPrompt(const std::string& name, const json& arguments) {
        MCP_LOG_INFO("Getting prompt: " + name);
        auto req = request::create(methods::PROMPTS_GET,
                                   {{"name", name}, {"arguments", arguments}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to get prompt: " + response.error.dump());
        }

        MCP_LOG_INFO("Prompt retrieved successfully");
        return PromptResult::fromJson(response.result);
    }

    void setLoggingLevel(const std::string& level) {
        MCP_LOG_INFO("Setting logging level");
        auto req =
            request::create(methods::LOGGING_SET_LEVEL, json{{"level", level}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to set logging level: " + response.error.dump());
        }
    }

    Task getTask(const std::string& task_id) {
        MCP_LOG_INFO("Requesting task: " + task_id);
        auto req = request::create(methods::TASKS_GET, {{"taskId", task_id}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Failed to get task: " + response.error.dump());
        }

        return parseTaskFromResult(response.result, "tasks/get");
    }

    TaskResult getTaskResult(const std::string& task_id) {
        MCP_LOG_INFO("Requesting task result: " + task_id);
        auto req =
            request::create(methods::TASKS_RESULT, {{"taskId", task_id}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to get task result: " + response.error.dump());
        }

        TaskResult task_result = TaskResult::fromJson(response.result);
        if (task_result.taskId.empty()) {
            task_result.taskId = task_id;
        }
        return task_result;
    }

    ListTasksResult listTasks(const std::string& cursor) {
        MCP_LOG_DEBUG("Requesting task list");
        json params = json::object();
        if (!cursor.empty()) {
            params["cursor"] = cursor;
        }

        auto req = request::create(methods::TASKS_LIST, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to list tasks: " + response.error.dump());
        }

        return ListTasksResult::fromJson(response.result);
    }

    Task cancelTask(const std::string& task_id) {
        MCP_LOG_INFO("Cancelling task: " + task_id);
        auto req =
            request::create(methods::TASKS_CANCEL, {{"taskId", task_id}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to cancel task: " + response.error.dump());
        }

        return parseTaskFromResult(response.result, "tasks/cancel");
    }

    CompleteResult complete(const json& params) {
        // 最小预检：明显不可能成功的请求直接本地失败，
        // 避免无谓的 round-trip，错误堆栈也更短。详细字段校验仍由 server 端兜底。
        if (params.is_null() || !params.is_object() || params.empty()) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "complete(): params must be a non-empty JSON object "
                "containing at least 'ref' and 'argument'");
        }
        if (!params.contains("ref") || !params.contains("argument")) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "complete(): params must contain both 'ref' and 'argument'");
        }

        MCP_LOG_INFO("Sending completion request");
        auto req = request::create(methods::COMPLETION_COMPLETE, params);
        auto response = send_request_internal(req);

        if (response.isError()) {
            ErrorCode code = ErrorCode::kInternalError;
            std::string message = "Failed to complete request";
            if (response.error.is_object()) {
                if (response.error.contains("code") &&
                    response.error["code"].is_number_integer()) {
                    code = static_cast<ErrorCode>(
                        response.error["code"].get<int>());
                }
                if (response.error.contains("message") &&
                    response.error["message"].is_string()) {
                    message = response.error["message"].get<std::string>();
                }
            }
            throw mcp_exception(code, message);
        }

        return CompleteResult::fromJson(response.result);
    }

    void subscribeResource(const std::string& uri) {
        MCP_LOG_INFO("Subscribing resource: " + uri);
        auto req =
            request::create(methods::RESOURCES_SUBSCRIBE, {{"uri", uri}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to subscribe resource: " + response.error.dump());
        }
        trackSubscription(uri);
    }

    void unsubscribeResource(const std::string& uri) {
        MCP_LOG_INFO("Unsubscribing resource: " + uri);
        auto req =
            request::create(methods::RESOURCES_UNSUBSCRIBE, {{"uri", uri}});
        auto response = send_request_internal(req);

        if (response.isError()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Failed to unsubscribe resource: " + response.error.dump());
        }
        untrackSubscription(uri);
    }

    void sendNotification(const std::string& method, const json& params) {
        MCP_LOG_DEBUG("Sending notification: " + method);
        auto notification = request::createNotification(method, params);
        send_notification_internal(notification);
    }

    void setNotificationHandler(const std::string& method,
                                NotificationHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        notification_handlers_[method] = handler;
        MCP_LOG_DEBUG("Set notification handler for: " + method);
    }

    void setProgressHandler(ProgressHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        progress_handler_ = std::move(handler);
        MCP_LOG_DEBUG("Set progress notification handler");
    }

    void setCancelRequestHandler(CancelHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        cancel_request_handler_ = std::move(handler);
        MCP_LOG_DEBUG("Set cancel request handler");
    }

    void setToolCallHandler(ToolCallHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        tool_call_handler_ = handler;
        MCP_LOG_DEBUG("Set tool call handler");
    }

    void setResourceReadHandler(ResourceReadHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        resource_read_handler_ = handler;
        MCP_LOG_DEBUG("Set resource read handler");
    }

    void setPromptGetHandler(PromptGetHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        prompt_get_handler_ = handler;
        MCP_LOG_DEBUG("Set prompt get handler");
    }

    void setElicitationHandler(ElicitationHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        elicitation_handler_ = std::move(handler);
        MCP_LOG_DEBUG("Set elicitation handler");
    }

    void setSamplingHandler(SamplingHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        sampling_handler_ = std::move(handler);
        MCP_LOG_DEBUG("Set sampling handler");
    }

    void setClientInfo(const std::string& name, const std::string& version) {
        if (name.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Client name must be non-empty");
        }
        if (version.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Client version must be non-empty");
        }

        client_info_.name = name;
        client_info_.version = version;
        MCP_LOG_DEBUG("Set client info: " + name + " v" + version);
    }

    void setCapabilities(const ClientCapabilities& capabilities) {
        client_capabilities_ = capabilities;
        MCP_LOG_DEBUG("Set client capabilities");
    }

    ServerInfo getServerInfo() const { return server_info_; }

    ServerCapabilities getServerCapabilities() const {
        return server_capabilities_;
    }

    // Endpoint management methods
    void setEndpoints(const std::vector<std::string>& endpoints) {
        endpoints_ = endpoints;
        if (!endpoints_.empty()) {
            current_endpoint_ = endpoints_[0];
            current_endpoint_index_ = 0;
            MCP_LOG_INFO("Set " + std::to_string(endpoints_.size()) +
                         " endpoints, current: " + current_endpoint_);
        }
    }

    void setCurrentEndpoint(const std::string& endpoint) {
        current_endpoint_ = endpoint;
        MCP_LOG_INFO("Set current endpoint: " + current_endpoint_);
    }

    std::string getCurrentEndpoint() const { return current_endpoint_; }

    std::vector<std::string> getEndpoints() const { return endpoints_; }

    void setRoots(const std::vector<Root>& roots) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        roots_ = roots;
    }

    std::vector<Root> getRoots() const {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        return roots_;
    }

    // Rotate to next endpoint
    void rotate_endpoint() {
        if (endpoints_.size() > 1) {
            current_endpoint_index_ =
                (current_endpoint_index_ + 1) % endpoints_.size();
            current_endpoint_ = endpoints_[current_endpoint_index_];
            MCP_LOG_INFO("Rotated to endpoint: " + current_endpoint_);
        }
    }

    bool runReconnectRecovery(bool success, const std::string& reason) {
        ReconnectEvent event;
        event.success = success;
        event.reason = reason;
        event.status = transport_.getStatus();

        ReconnectHandler handler;
        {
            std::lock_guard<std::mutex> lock(recovery_mutex_);
            handler = reconnect_handler_;
        }

        if (handler) {
            return handler(event);
        }
        if (!success) {
            return false;
        }
        std::string failure_reason;
        bool recovered = runDefaultReconnectHandler(&failure_reason);
        if (!recovered && !failure_reason.empty()) {
            event.success = false;
            event.reason = failure_reason;
            event.status = transport_.getStatus();
        }
        return recovered;
    }

    bool runDefaultReconnectHandler(std::string* failure_reason) {
        InitializeParams init_params;
        std::vector<std::string> subscriptions;
        {
            std::lock_guard<std::mutex> lock(recovery_mutex_);
            if (!has_cached_init_params_) {
                return true;
            }
            init_params = cached_init_params_;
            subscriptions = subscribed_uris_;
        }

        try {
            initialize(init_params);
            for (const auto& uri : subscriptions) {
                sendSubscribeRequest(uri);
            }
            return true;
        } catch (const std::exception& e) {
            if (failure_reason) {
                *failure_reason = e.what();
            }
            MCP_LOG_ERROR("Default reconnect recovery failed: " +
                          std::string(e.what()));
            return false;
        } catch (...) {
            if (failure_reason) {
                *failure_reason =
                    "Default reconnect recovery failed with unknown error";
            }
            MCP_LOG_ERROR(
                "Default reconnect recovery failed with unknown error");
            return false;
        }
    }

    void sendSubscribeRequest(const std::string& uri) {
        auto req =
            request::create(methods::RESOURCES_SUBSCRIBE, {{"uri", uri}});
        auto response = send_request_internal(req);
        if (response.isError()) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Failed to restore resource subscription: " +
                                    response.error.dump());
        }
    }

    void trackSubscription(const std::string& uri) {
        std::lock_guard<std::mutex> lock(recovery_mutex_);
        if (std::find(subscribed_uris_.begin(), subscribed_uris_.end(), uri) ==
            subscribed_uris_.end()) {
            subscribed_uris_.push_back(uri);
        }
    }

    void untrackSubscription(const std::string& uri) {
        std::lock_guard<std::mutex> lock(recovery_mutex_);
        subscribed_uris_.erase(
            std::remove(subscribed_uris_.begin(), subscribed_uris_.end(), uri),
            subscribed_uris_.end());
    }

private:
    response send_request_internal(const request& req) {
        if (!isConnected()) {
            return response::createError(req.id, ErrorCode::kInternalError,
                                         "Not connected to server");
        }

        try {
            // Convert MCP request to transport message
            mcp_transport::TransportMessage transport_msg;
            // Convert JSON id to string
            if (req.id.is_string()) {
                transport_msg.id = req.id.get<std::string>();
            } else if (req.id.is_number()) {
                transport_msg.id = std::to_string(req.id.get<int>());
            } else {
                transport_msg.id = req.id.dump();
            }
            transport_msg.content = req.toJson().dump();
            transport_msg.method = "POST";
            transport_msg.path = current_endpoint_;
            apply_protocol_headers(transport_msg);
            transport_msg.timestamp = std::chrono::system_clock::now();

            MCP_LOG_DEBUG("Sending request via transport: " + req.method +
                          " (ID: " + req.id.dump() + ")");

            // Retry logic
            const int max_attempts = 5;
            int attempt = 0;
            mcp_transport::TransportMessage transport_response;

            while (attempt < max_attempts) {
                ++attempt;
                MCP_LOG_DEBUG("Attempt " + std::to_string(attempt) + ": " +
                              req.method);

                transport_response = transport_.sendRequestSync(transport_msg);
                if (transport_response.status_code == 200) {
                    break;
                }

                MCP_LOG_WARNING("  Retry (" + std::to_string(attempt) + "/" +
                                std::to_string(max_attempts) + "), status: " +
                                std::to_string(transport_response.status_code));
                if (attempt < max_attempts) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(500 * attempt));
                }
            }

            MCP_LOG_DEBUG("Received response:");
            MCP_LOG_DEBUG("  status code: " +
                          std::to_string(transport_response.status_code));

            if (transport_response.status_code == 200) {
                MCP_LOG_DEBUG("  success");

                // Parse response content
                json response_json;
                try {
                    response_json = json::parse(transport_response.content);

                    const bool looks_like_jsonrpc_response =
                        response_json.is_object() &&
                        response_json.contains("jsonrpc") &&
                        response_json.contains("id") &&
                        (response_json.contains("result") ||
                         response_json.contains("error"));
                    if (looks_like_jsonrpc_response) {
                        if (response_json["id"] != req.id) {
                            MCP_LOG_ERROR("  error: response id mismatch");
                            return response::createError(
                                req.id, ErrorCode::kInternalError,
                                "Response id mismatch");
                        }
                        if (response_json.contains("result")) {
                            MCP_LOG_DEBUG("  result: " +
                                          response_json["result"].dump(2));
                            return response::createSuccess(
                                req.id, response_json["result"]);
                        }
                        MCP_LOG_ERROR("  error: " +
                                      response_json["error"].dump(2));
                        return response::createError(
                            req.id, ErrorCode::kInternalError,
                            response_json["error"].dump());
                    }

                    const bool looks_like_flat_error =
                        response_json.is_object() &&
                        response_json.contains("code") &&
                        response_json.contains("message") &&
                        response_json["message"].is_string();
                    if (looks_like_flat_error) {
                        MCP_LOG_ERROR("  error: " + response_json.dump(2));
                        return response::createError(req.id,
                                                     ErrorCode::kInternalError,
                                                     response_json.dump());
                    }

                    MCP_LOG_DEBUG("  content: " + transport_response.content);
                    return response::createSuccess(req.id, response_json);
                } catch (const std::exception& e) {
                    MCP_LOG_DEBUG("  content: " + transport_response.content);
                    MCP_LOG_ERROR("Failed to parse response JSON: " +
                                  std::string(e.what()));
                    return response::createError(req.id, ErrorCode::kParseError,
                                                 "Failed to parse response");
                }
            } else {
                MCP_LOG_DEBUG("  failed (retries exhausted)");
                MCP_LOG_ERROR("  error: " + transport_response.error_message);
                MCP_LOG_DEBUG("  content: " + transport_response.content);

                std::string error_msg = transport_response.error_message.empty()
                                            ? "Transport error"
                                            : transport_response.error_message;
                return response::createError(req.id, ErrorCode::kInternalError,
                                             error_msg);
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("Error sending request: " + std::string(e.what()));
            return response::createError(
                req.id, ErrorCode::kInternalError,
                "Send request error: " + std::string(e.what()));
        }
    }

    void send_notification_internal(const request& notification) {
        if (!isConnected()) {
            MCP_LOG_WARNING("Cannot send notification: not connected");
            return;
        }

        try {
            // Convert MCP notification to transport message
            mcp_transport::TransportMessage transport_msg;
            // Convert JSON id to string
            if (notification.id.is_string()) {
                transport_msg.id = notification.id.get<std::string>();
            } else if (notification.id.is_number()) {
                transport_msg.id = std::to_string(notification.id.get<int>());
            } else {
                transport_msg.id = notification.id.dump();
            }
            transport_msg.content = notification.toJson().dump();
            transport_msg.method = "POST";
            transport_msg.path = current_endpoint_;
            apply_protocol_headers(transport_msg);
            transport_msg.timestamp = std::chrono::system_clock::now();

            MCP_LOG_DEBUG("Sending notification via transport: " +
                          notification.method);

            if (!transport_.sendMessage(transport_msg)) {
                MCP_LOG_WARNING("Failed to send notification: " +
                                notification.method);
            }

        } catch (const std::exception& e) {
            MCP_LOG_ERROR("Error sending notification: " +
                          std::string(e.what()));
        }
    }

    // Parse endpoint string and create transport config
    mcp_transport::TransportConfig parse_endpoint(
        const std::string& endpoint,
        mcp_transport::TransportType preferred_type) {
        mcp_transport::TransportConfig config;

        const bool force_ipc =
            preferred_type == mcp_transport::TransportType::kIpc;
        const bool force_sse =
            preferred_type == mcp_transport::TransportType::kSse;

        // Check whether endpoint is IPC command format (e.g. "cmd:python
        // script.py")
        if (endpoint.find("cmd:") == 0 || force_ipc) {
            std::string command_line =
                endpoint.find("cmd:") == 0
                    ? endpoint.substr(4)  // Strip "cmd:" prefix
                    : endpoint;
            const auto command_parts = split_command_line(command_line);
            if (command_parts.empty()) {
                throw mcp_exception(
                    ErrorCode::kInvalidParams,
                    "Empty IPC command in endpoint: " + endpoint);
            }

            config.type = mcp_transport::TransportType::kIpc;
            config.ipc.command = command_parts.front();
            if (command_parts.size() > 1) {
                config.ipc.args.assign(command_parts.begin() + 1,
                                       command_parts.end());
            }
            config.ipc.buffer_size = 4096;
            config.ipc.read_timeout_ms = 2000;
            config.ipc.max_retry_count = 3;
            config.ipc.enable_logging = true;

            if (force_ipc && endpoint.find("cmd:") != 0) {
                MCP_LOG_INFO(
                    "Parsed IPC endpoint using preferred transport type: " +
                    command_line);
            } else {
                MCP_LOG_INFO("Parsed IPC endpoint: " + command_line);
            }
            return config;
        }

        // Check whether endpoint is HTTP/HTTPS URL format
        std::regex http_regex(
            R"(^(https?):\/\/([^:\/]+)(?::([^\/]+))?(?:\/(.*))?$)");
        std::smatch matches;

        if (std::regex_match(endpoint, matches, http_regex)) {
            std::string protocol = matches[1].str();
            std::string host = matches[2].str();
            std::string port_str = matches[3].str();
            std::string path = matches[4].str();

            int port = protocol == "https" ? 443 : 80;
            if (!port_str.empty()) {
                try {
                    port = std::stoi(port_str);
                } catch (const std::exception&) {
                    throw mcp_exception(ErrorCode::kInvalidParams,
                                        "Invalid port number: " + port_str);
                }
                if (port < 1 || port > 65535) {
                    throw mcp_exception(ErrorCode::kInvalidParams,
                                        "Invalid port number: " + port_str);
                }
            }

            config.type = mcp_transport::TransportType::kSse;
            config.sse.host = host;
            config.sse.port = port;
            config.sse.protocol = (protocol == "https")
                                      ? mcp_transport::TransportProtocol::kHttps
                                      : mcp_transport::TransportProtocol::kHttp;
            config.sse.timeout_seconds = 30;
            config.sse.max_retries = 3;
            config.sse.user_agent = "MCP-Client/1.0";
            config.sse.keep_alive = true;
            config.sse.endpoint = path.empty() ? "/mcp" : "/" + path;

            MCP_LOG_INFO("Parsed HTTP endpoint: " + protocol + "://" + host +
                         ":" + std::to_string(config.sse.port));
            return config;
        }

        if (force_sse) {
            throw mcp_exception(
                ErrorCode::kInvalidParams,
                "SSE transport requires HTTP(S) endpoint: " + endpoint);
        }

        const std::size_t scheme_pos = endpoint.find("://");
        if (scheme_pos != std::string::npos) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Unsupported endpoint scheme: " +
                                    endpoint.substr(0, scheme_pos));
        }

        // Default to IPC and treat endpoint as a command
        const auto command_parts = split_command_line(endpoint);
        if (command_parts.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams, "Empty endpoint");
        }

        config.type = mcp_transport::TransportType::kIpc;
        config.ipc.command = command_parts.front();
        if (command_parts.size() > 1) {
            config.ipc.args.assign(command_parts.begin() + 1,
                                   command_parts.end());
        }
        config.ipc.buffer_size = 4096;
        config.ipc.read_timeout_ms = 2000;
        config.ipc.max_retry_count = 3;
        config.ipc.enable_logging = true;

        MCP_LOG_INFO("Using default IPC configuration for endpoint: " +
                     endpoint);
        return config;
    }

    // Check whether string is valid JSON
    bool is_valid_json(const std::string& content) {
        if (content.empty()) {
            return false;
        }

        // Quick check: JSON should start with { or [
        std::string trimmed = content;
        size_t start = trimmed.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) {
            return false;
        }
        trimmed = trimmed.substr(start);

        if (trimmed.empty() || (trimmed[0] != '{' && trimmed[0] != '[')) {
            return false;
        }

        // Use lightweight syntax validation to avoid constructing a full JSON
        // object.
        return json::accept(trimmed);
    }

    response handle_server_request(const request& req) {
        if (req.method == methods::ROOTS_LIST) {
            json result;
            result["roots"] = json::array();
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            for (const auto& root : roots_) {
                result["roots"].push_back(root.toJson());
            }
            return response::createSuccess(req.id, result);
        }

        if (req.method == methods::TOOLS_CALL) {
            ToolCallHandler handler;
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                handler = tool_call_handler_;
            }
            if (!handler) {
                return response::createError(req.id, ErrorCode::kMethodNotFound,
                                             "No tool call handler registered");
            }

            ToolCallParams params = ToolCallParams::fromJson(req.params);
            return response::createSuccess(req.id, handler(params).toJson());
        }

        if (req.method == methods::RESOURCES_READ) {
            ResourceReadHandler handler;
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                handler = resource_read_handler_;
            }
            if (!handler) {
                return response::createError(
                    req.id, ErrorCode::kMethodNotFound,
                    "No resource read handler registered");
            }

            const std::string uri = req.params.value("uri", "");
            if (uri.empty()) {
                return response::createError(req.id, ErrorCode::kInvalidParams,
                                             "Missing resource uri");
            }
            return response::createSuccess(req.id, handler(uri).toJson());
        }

        if (req.method == methods::PROMPTS_GET) {
            PromptGetHandler handler;
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                handler = prompt_get_handler_;
            }
            if (!handler) {
                return response::createError(
                    req.id, ErrorCode::kMethodNotFound,
                    "No prompt get handler registered");
            }

            const std::string name = req.params.value("name", "");
            if (name.empty()) {
                return response::createError(req.id, ErrorCode::kInvalidParams,
                                             "Missing prompt name");
            }
            const json arguments =
                req.params.value("arguments", json::object());
            return response::createSuccess(req.id,
                                           handler(name, arguments).toJson());
        }

        if (req.method == methods::ELICITATION_CREATE) {
            ElicitationHandler handler;
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                handler = elicitation_handler_;
            }
            if (!handler) {
                return response::createError(
                    req.id, ErrorCode::kMethodNotFound,
                    "No elicitation handler registered");
            }

            return response::createSuccess(req.id,
                                           handler(req.params).toJson());
        }

        if (req.method == methods::SAMPLING_CREATE_MESSAGE) {
            SamplingHandler handler;
            {
                std::lock_guard<std::mutex> lock(handlers_mutex_);
                handler = sampling_handler_;
            }
            if (!handler) {
                return response::createError(req.id, ErrorCode::kMethodNotFound,
                                             "No sampling handler registered");
            }

            return response::createSuccess(req.id,
                                           handler(req.params).toJson());
        }

        return response::createError(
            req.id, ErrorCode::kMethodNotFound,
            "Unsupported client request: " + req.method);
    }

    bool send_response_internal(const response& rpc_response) {
        if (!isConnected()) {
            MCP_LOG_WARNING("Cannot send response: not connected");
            return false;
        }

        try {
            mcp_transport::TransportMessage transport_msg;
            if (rpc_response.id.is_string()) {
                transport_msg.id = rpc_response.id.get<std::string>();
            } else if (rpc_response.id.is_number()) {
                transport_msg.id = std::to_string(rpc_response.id.get<int>());
            } else {
                transport_msg.id = rpc_response.id.dump();
            }
            transport_msg.content =
                rpc_response.toJson(negotiated_protocol_version_).dump();
            transport_msg.method = "POST";
            transport_msg.path = current_endpoint_;
            apply_protocol_headers(transport_msg);
            transport_msg.timestamp = std::chrono::system_clock::now();
            return transport_.sendMessage(transport_msg);
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("Error sending response: " + std::string(e.what()));
            return false;
        }
    }

    // Handle messages from transport
    void handle_transport_message(
        const mcp_transport::TransportMessage& message) {
        // First validate JSON; skip non-protocol payloads
        if (!is_valid_json(message.content)) {
            // Non-JSON message (e.g. log output), skip silently
            std::string preview = message.content.length() > 100
                                      ? message.content.substr(0, 100) + "..."
                                      : message.content;
            MCP_LOG_DEBUG("Skipping non-JSON message: " + preview);
            return;
        }

        try {
            // Parse message payload as JSON
            json message_json = json::parse(message.content);

            if (message_json.contains("method")) {
                if (message_json.contains("id") &&
                    !message_json["id"].is_null()) {
                    request req = request::fromJson(message_json);
                    MCP_LOG_DEBUG("Received server request: " + req.method +
                                  " (ID: " + req.id.dump() + ")");
                    response rpc_response = handle_server_request(req);
                    if (!send_response_internal(rpc_response)) {
                        MCP_LOG_ERROR(
                            "Failed to send response for server request: " +
                            req.method);
                    }
                } else {
                    // This is a notification
                    std::string method = message_json["method"];
                    json params = json::object();
                    if (message_json.contains("params")) {
                        params = message_json["params"];
                    }

                    MCP_LOG_DEBUG("Received notification: " + method);

                    if (method == methods::NOTIFICATION_PROGRESS) {
                        ProgressHandler progress_handler;
                        {
                            std::lock_guard<std::mutex> lock(handlers_mutex_);
                            progress_handler = progress_handler_;
                        }
                        if (progress_handler) {
                            progress_handler(ProgressParams::fromJson(params));
                        }
                    } else if (method == methods::CANCEL_REQUEST ||
                               method == methods::NOTIFICATION_CANCELLED) {
                        CancelHandler cancel_handler;
                        {
                            std::lock_guard<std::mutex> lock(handlers_mutex_);
                            cancel_handler = cancel_request_handler_;
                        }
                        if (cancel_handler) {
                            cancel_handler(CancelParams::fromJson(params));
                        }
                    }

                    // Find matching notification handler
                    std::lock_guard<std::mutex> lock(handlers_mutex_);
                    auto it = notification_handlers_.find(method);
                    if (it != notification_handlers_.end()) {
                        it->second(method, params);
                    }
                }
            } else {
                // This is a response already handled in send_request_internal
                MCP_LOG_DEBUG("Received response message (already handled)");
            }
        } catch (const json::parse_error& e) {
            // JSON parse error; log without spamming
            MCP_LOG_DEBUG("JSON parse error in transport message: " +
                          std::string(e.what()));
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("Error handling transport message: " +
                          std::string(e.what()));
        }
    }

    bool connected_;
    bool connecting_ = false;
    mcp_transport::TransportType transport_type_;
    mcp_transport::Transport transport_;
    mcp_transport::TransportConfig last_config_;
    bool has_last_config_ = false;

    // Endpoint management
    std::vector<std::string> endpoints_;
    std::string current_endpoint_;
    size_t current_endpoint_index_;
    std::string negotiated_protocol_version_ = PROTOCOL_VERSION;
    std::vector<Root> roots_;

    ClientInfo client_info_;
    ClientCapabilities client_capabilities_;
    InitializeParams cached_init_params_;
    bool has_cached_init_params_ = false;
    std::vector<std::string> subscribed_uris_;
    ReconnectHandler reconnect_handler_;
    std::mutex recovery_mutex_;
    std::mutex reconnect_mutex_;
    ServerInfo server_info_;
    ServerCapabilities server_capabilities_;

    mutable std::mutex handlers_mutex_;
    std::map<std::string, NotificationHandler> notification_handlers_;
    ProgressHandler progress_handler_;
    CancelHandler cancel_request_handler_;
    ToolCallHandler tool_call_handler_;
    ResourceReadHandler resource_read_handler_;
    PromptGetHandler prompt_get_handler_;
    ElicitationHandler elicitation_handler_;
    SamplingHandler sampling_handler_;

    void apply_protocol_headers(mcp_transport::TransportMessage& msg) const {
        if (negotiated_protocol_version_.empty()) {
            return;
        }
        msg.headers[kMcpProtocolVersionHeader] = negotiated_protocol_version_;
        msg.headers[kMcpProtocolVersionHeaderLower] =
            negotiated_protocol_version_;
        msg.headers[kAcceptHeader] = kJsonAndEventStreamAcceptValue;
    }
};

// MCPClient public interface
MCPClient::MCPClient() : impl_(std::make_unique<Impl>()) {
    MCP_LOG_DEBUG("MCPClient created");
}

MCPClient::~MCPClient() { MCP_LOG_DEBUG("MCPClient destroyed"); }

bool MCPClient::connect(const std::string& endpoint) {
    return impl_->connect(endpoint);
}

bool MCPClient::connectWithConfig(
    const mcp_transport::TransportConfig& config) {
    return impl_->connectWithConfig(config);
}

void MCPClient::disconnect() { impl_->disconnect(); }

bool MCPClient::reconnect() { return impl_->reconnect(); }

void MCPClient::setReconnectHandler(ReconnectHandler handler) {
    impl_->setReconnectHandler(std::move(handler));
}

bool MCPClient::isConnected() const { return impl_->isConnected(); }

void MCPClient::setTransportType(mcp_transport::TransportType type) {
    impl_->setTransportType(type);
}

mcp_transport::TransportType MCPClient::getTransportType() const {
    return impl_->getTransportType();
}

mcp_transport::TransportStatus MCPClient::getTransportStatus() const {
    return impl_->getTransportStatus();
}

InitializeResult MCPClient::initialize(const InitializeParams& params) {
    return impl_->initialize(params);
}

void MCPClient::initialized() { impl_->initialized(); }

void MCPClient::ping() { impl_->ping(); }

ListToolsResult MCPClient::listTools(const std::string& cursor) {
    return impl_->listTools(cursor);
}

ToolCallResult MCPClient::callTool(const ToolCallParams& params) {
    return impl_->callTool(params);
}

ToolCallResult MCPClient::callTool(const std::string& name,
                                   const json& arguments) {
    return impl_->callTool(name, arguments);
}

ListResourcesResult MCPClient::listResources(const std::string& cursor) {
    return impl_->listResources(cursor);
}

ListResourceTemplatesResult MCPClient::listResourceTemplates(
    const std::string& cursor) {
    return impl_->listResourceTemplates(cursor);
}

ReadResourceResult MCPClient::readResource(const std::string& uri) {
    return impl_->readResource(uri);
}

ListPromptsResult MCPClient::listPrompts(const std::string& cursor) {
    return impl_->listPrompts(cursor);
}

PromptResult MCPClient::getPrompt(const std::string& name,
                                  const json& arguments) {
    return impl_->getPrompt(name, arguments);
}

void MCPClient::setLoggingLevel(const std::string& level) {
    impl_->setLoggingLevel(level);
}

Task MCPClient::getTask(const std::string& task_id) {
    return impl_->getTask(task_id);
}

TaskResult MCPClient::getTaskResult(const std::string& task_id) {
    return impl_->getTaskResult(task_id);
}

ListTasksResult MCPClient::listTasks(const std::string& cursor) {
    return impl_->listTasks(cursor);
}

Task MCPClient::cancelTask(const std::string& task_id) {
    return impl_->cancelTask(task_id);
}

CompleteResult MCPClient::complete(const json& params) {
    return impl_->complete(params);
}

void MCPClient::subscribeResource(const std::string& uri) {
    impl_->subscribeResource(uri);
}

void MCPClient::unsubscribeResource(const std::string& uri) {
    impl_->unsubscribeResource(uri);
}

void MCPClient::sendNotification(const std::string& method,
                                 const json& params) {
    impl_->sendNotification(method, params);
}

void MCPClient::setNotificationHandler(const std::string& method,
                                       NotificationHandler handler) {
    impl_->setNotificationHandler(method, handler);
}

void MCPClient::setProgressHandler(ProgressHandler handler) {
    impl_->setProgressHandler(std::move(handler));
}

void MCPClient::setCancelRequestHandler(CancelHandler handler) {
    impl_->setCancelRequestHandler(std::move(handler));
}

void MCPClient::setToolCallHandler(ToolCallHandler handler) {
    impl_->setToolCallHandler(handler);
}

void MCPClient::setResourceReadHandler(ResourceReadHandler handler) {
    impl_->setResourceReadHandler(handler);
}

void MCPClient::setPromptGetHandler(PromptGetHandler handler) {
    impl_->setPromptGetHandler(handler);
}

void MCPClient::setElicitationHandler(ElicitationHandler handler) {
    impl_->setElicitationHandler(std::move(handler));
}

void MCPClient::setSamplingHandler(SamplingHandler handler) {
    impl_->setSamplingHandler(std::move(handler));
}

void MCPClient::setClientInfo(const std::string& name,
                              const std::string& version) {
    impl_->setClientInfo(name, version);
}

void MCPClient::setCapabilities(const ClientCapabilities& capabilities) {
    impl_->setCapabilities(capabilities);
}

ServerInfo MCPClient::getServerInfo() const { return impl_->getServerInfo(); }

ServerCapabilities MCPClient::getServerCapabilities() const {
    return impl_->getServerCapabilities();
}

// Endpoint management methods
void MCPClient::setEndpoints(const std::vector<std::string>& endpoints) {
    impl_->setEndpoints(endpoints);
}

void MCPClient::setCurrentEndpoint(const std::string& endpoint) {
    impl_->setCurrentEndpoint(endpoint);
}

std::string MCPClient::getCurrentEndpoint() const {
    return impl_->getCurrentEndpoint();
}

std::vector<std::string> MCPClient::getEndpoints() const {
    return impl_->getEndpoints();
}

void MCPClient::setRoots(const std::vector<Root>& roots) {
    impl_->setRoots(roots);
}

std::vector<Root> MCPClient::getRoots() const { return impl_->getRoots(); }

}  // namespace mcp
