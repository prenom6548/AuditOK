/**
 * @file server.cpp
 * @brief MCP Server Implementation
 *
 * This file provides the implementation of the MCP server functionality.
 */

#include "server.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <set>
#include <sstream>

#include "common/transport.h"
#include "server_dispatch.h"
#include "session_manager.h"

// Logger is declared locally when needed

namespace mcp {
// Forward declaration
class MCPServerRequestHandler;
class MCPServerConnectionHandler;

// Session ID validation helper
// Validate session ID format (non-empty, valid length, valid charset)
static bool validateSessionId(const std::string& session_id) {
    // Check whether it is empty
    if (session_id.empty()) {
        return false;
    }

    // Session ID length should be 32 (per SessionManager implementation)
    constexpr std::size_t kExpectedLength = 32;
    if (session_id.length() != kExpectedLength) {
        return false;
    }

    // Validate charset: alphanumeric only (per SessionManager implementation)
    // Use efficient character checks
    for (char c : session_id) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z'))) {
            return false;
        }
    }

    return true;
}

static std::string makeUtcTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &tt);
#else
    gmtime_r(&tt, &tm_utc);
#endif

    char buffer[32] = {0};
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm_utc) ==
        0) {
        return "";
    }
    return std::string(buffer);
}

// MCP Server Implementation
class MCPServer::Impl : public internal::MCPServerHandlerContext {
public:
    Impl() : running_(false) {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("MCPServer::Impl created");
        transport_ = std::make_unique<mcp_transport::Transport>();
    }

    virtual ~Impl() {
        if (running_) {
            stop();
        }
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("MCPServer::Impl destroyed");
    }

    bool start(const mcp_transport::TransportConfig& config) {
        mcp_logger::getModuleLogger("mcp_server")->info("Starting MCP server");

        // Set server request handler
        auto handler = std::make_shared<MCPServerRequestHandler>(this);
        transport_->setServerRequestHandler(
            std::static_pointer_cast<
                mcp_transport::TransportServerRequestHandler>(handler));

        // Set connection request handler
        auto connection_handler =
            std::make_shared<MCPServerConnectionHandler>(this);
        transport_->setConnectionRequestHandler(
            std::static_pointer_cast<mcp_transport::SseServerConnectHandler>(
                connection_handler));

        // Set callback handlers
        transport_->setMessageCallback(
            [](const mcp_transport::TransportMessage& message) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->info("Received message: " + message.content);
            });

        if (config.type == mcp_transport::TransportType::kSse) {
            transport_->setConnectionCallback([this](const mcp_transport::
                                                         ConnectionInfo& info) {
                auto logger = mcp_logger::getModuleLogger("mcp_server");
                const std::string& session_id = info.session_id;

                // Validate session ID
                bool is_valid = validateSessionId(session_id);

                if (is_valid) {
                    logger->info(
                        "✅ Connection established with valid session ID: " +
                        session_id);
                    logger->info("   Connection ID: " + info.connection_id);
                    logger->info("   Remote: " + info.remote_address + ":" +
                                 std::to_string(info.remote_port));

                    // Store ConnectionInfo in SessionManager
                    try {
                        // Serialize ConnectionInfo to JSON string
                        nlohmann::json connection_info_json = info.toJson();
                        std::string connection_info_str =
                            connection_info_json.dump();

                        // Create attributes with ConnectionInfo as JSON
                        // Store as string
                        std::unordered_map<std::string, std::string> attributes;
                        attributes["connection_info"] = connection_info_str;
                        attributes["connection_id"] = info.connection_id;
                        attributes["remote_address"] = info.remote_address;
                        attributes["remote_port"] =
                            std::to_string(info.remote_port);
                        attributes["local_address"] = info.local_address;
                        attributes["local_port"] =
                            std::to_string(info.local_port);
                        attributes["status"] =
                            std::to_string(static_cast<int>(info.status));
                        attributes["type"] =
                            std::to_string(static_cast<int>(info.type));

                        // Create Session object
                        Session session;
                        session.id = session_id;
                        session.created_at = std::chrono::steady_clock::now();
                        session.expires_at = session.created_at +
                                             session_manager_.GetSessionTtl();
                        session.attributes = attributes;

                        // Add to SessionManager
                        if (session_manager_.AddSession(session_id, session)) {
                            logger->info(
                                "📦 Session stored in SessionManager: " +
                                session_id);
                        } else {
                            logger->warning(
                                "⚠️ Failed to store session in "
                                "SessionManager: " +
                                session_id);
                        }
                    } catch (const std::exception& e) {
                        logger->error(
                            "❌ Error storing session in SessionManager: " +
                            std::string(e.what()));
                    }
                } else {
                    logger->warning(
                        "⚠️ Connection established with invalid or empty "
                        "session ID");
                    logger->warning("   Connection ID: " + info.connection_id);
                }
            });
        }

        // Start transport
        if (!transport_->start(config)) {
            mcp_logger::getModuleLogger("mcp_server")
                ->error("Failed to start transport");
            return false;
        }

        running_ = true;
        mcp_logger::getModuleLogger("mcp_server")
            ->info("MCP server started successfully");
        return true;
    }

    void stop() {
        if (running_) {
            mcp_logger::getModuleLogger("mcp_server")
                ->info("Stopping MCP server");
            running_ = false;
            transport_->stop();
            mcp_logger::getModuleLogger("mcp_server")
                ->info("MCP server stopped");
        }
    }

    bool isRunning() const { return running_; }

    InitializeResult initialize(const InitializeParams& params) {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Handling initialize request from client: " +
                   params.clientInfo.name);

        // Follow MCP lifecycle negotiation:
        // if client proposes an unsupported version, return server-supported
        // version in initialize result instead of failing hard.
        const bool client_version_supported =
            utils::isValidProtocolVersion(params.protocolVersion);
        if (!client_version_supported) {
            mcp_logger::getModuleLogger("mcp_server")
                ->warning("Client requested unsupported protocol version: " +
                          params.protocolVersion +
                          ", falling back to server-supported version: " +
                          std::string(PROTOCOL_VERSION));
        }

        // Store client info
        client_info_ = params.clientInfo;
        client_capabilities_ = params.capabilities;

        // Build compatibility-friendly capabilities for inspector/schema-strict
        // clients. Keep only widely supported keys and drop extension-shaped
        // payloads.
        auto normalize_bool_object =
            [](const json& src,
               std::initializer_list<const char*> allowed_keys) {
                json normalized = json::object();
                if (!src.is_object()) {
                    return normalized;
                }
                for (const char* key : allowed_keys) {
                    auto it = src.find(key);
                    if (it != src.end() && it->is_boolean()) {
                        normalized[key] = *it;
                    }
                }
                return normalized;
            };

        ServerCapabilities response_capabilities = server_capabilities_;
        response_capabilities.prompts = normalize_bool_object(
            response_capabilities.prompts, {"listChanged"});
        response_capabilities.resources = normalize_bool_object(
            response_capabilities.resources, {"listChanged", "subscribe"});
        response_capabilities.tools =
            normalize_bool_object(response_capabilities.tools, {"listChanged"});
        response_capabilities.sampling = json();
        response_capabilities.elicitation = json();

        // Declare only capabilities that this server actually implements.
        // logging capability in MCP indicates notifications/message emission.
        response_capabilities.logging = json();
        response_capabilities.completions = json::object();
        response_capabilities.tasks =
            json{{"list", json::object()}, {"cancel", json::object()}};

        // Create initialize result
        InitializeResult result;
        result.protocolVersion = client_version_supported
                                     ? params.protocolVersion
                                     : PROTOCOL_VERSION;
        result.capabilities = response_capabilities;
        result.serverInfo = server_info_;
        result.instructions =
            "This server supports tools, resources, and prompts.";
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Negotiated protocol version: " + result.protocolVersion);
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Initialize result payload: " + result.toJson().dump());

        mcp_logger::getModuleLogger("mcp_server")
            ->info("Initialize completed successfully");
        return result;
    }

    void initialized() {
        if (client_initialized_) {
            mcp_logger::getModuleLogger("mcp_server")
                ->debug("Client initialized notification received repeatedly");
            return;
        }
        client_initialized_ = true;
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Client initialized notification received");
    }

    void ping() {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Handling ping request");
    }

    std::vector<Tool> listTools() const {
        std::lock_guard<std::mutex> lock(tools_mutex_);
        mcp_logger::getModuleLogger("mcp_server")->debug("Listing tools");
        return tools_;
    }

    ToolCallResult callTool(const ToolCallParams& params) {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Tool call requested: " + params.name);
        ToolCallHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            auto it = tool_handlers_.find(params.name);
            if (it != tool_handlers_.end()) {
                handler = it->second;
            }
        }

        ToolCallResult result;
        if (!handler) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Unknown tool: " + params.name);
        }

        try {
            return handler(params);
        } catch (const std::exception& e) {
            result.isError = true;
            result.errorMessage =
                "Tool execution error: " + std::string(e.what());
            mcp_logger::getModuleLogger("mcp_server")
                ->error("Tool call failed: " + result.errorMessage);
            return result;
        }
    }

    std::vector<Resource> listResources() const {
        std::lock_guard<std::mutex> lock(resources_mutex_);
        mcp_logger::getModuleLogger("mcp_server")->debug("Listing resources");
        return resources_;
    }

    std::vector<ResourceTemplate> listResourceTemplates() const {
        std::lock_guard<std::mutex> lock(resource_templates_mutex_);
        return resource_templates_;
    }

    ReadResourceResult readResource(const std::string& uri) {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Reading resource: " + uri);

        ResourceReadHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            auto it = resource_handlers_.find(uri);
            if (it != resource_handlers_.end()) {
                handler = it->second;
            }
        }

        if (!handler) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Unknown resource: " + uri);
        }

        try {
            return handler(uri);
        } catch (const mcp_exception&) {
            throw;
        } catch (const std::exception& e) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Resource read error: " + std::string(e.what()));
        }
    }

    void subscribeResource(const std::string& uri) {
        if (uri.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "resources/subscribe requires field: uri");
        }

        std::lock_guard<std::mutex> resources_lock(resources_mutex_);
        const auto existing = std::find_if(
            resources_.begin(), resources_.end(),
            [&uri](const Resource& resource) { return resource.uri == uri; });
        if (existing == resources_.end()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Resource URI not registered: " + uri);
        }

        std::lock_guard<std::mutex> lock(resource_subscriptions_mutex_);
        resource_subscriptions_.insert(uri);
    }

    void unsubscribeResource(const std::string& uri) {
        if (uri.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "resources/unsubscribe requires field: uri");
        }

        std::lock_guard<std::mutex> resources_lock(resources_mutex_);
        const auto existing = std::find_if(
            resources_.begin(), resources_.end(),
            [&uri](const Resource& resource) { return resource.uri == uri; });
        if (existing == resources_.end()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Resource URI not registered: " + uri);
        }

        std::lock_guard<std::mutex> lock(resource_subscriptions_mutex_);
        resource_subscriptions_.erase(uri);
    }

    Task getTask(const std::string& task_id) {
        if (task_id.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "tasks/get requires field: taskId");
        }

        std::lock_guard<std::mutex> lock(tasks_mutex_);
        auto it = tasks_.find(task_id);
        if (it == tasks_.end()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Task not found: " + task_id);
        }

        return it->second.task;
    }

    TaskResult getTaskResult(const std::string& task_id) {
        if (task_id.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "tasks/result requires field: taskId");
        }

        std::lock_guard<std::mutex> lock(tasks_mutex_);
        auto it = tasks_.find(task_id);
        if (it == tasks_.end()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Task not found: " + task_id);
        }

        if (!it->second.result.has_value()) {
            return TaskResult{task_id, json::object()};
        }

        return TaskResult{task_id, it->second.result.value()};
    }

    ListTasksResult listTasks(const std::string& cursor) {
        std::size_t offset = 0;
        if (!cursor.empty()) {
            try {
                offset = static_cast<std::size_t>(std::stoull(cursor));
            } catch (const std::exception&) {
                throw mcp_exception(ErrorCode::kInvalidParams,
                                    "tasks/list cursor must be a decimal "
                                    "offset string");
            }
        }

        constexpr std::size_t kTaskPageSize = 50;
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        ListTasksResult result;
        std::vector<Task> all_tasks;
        all_tasks.reserve(tasks_.size());
        for (const auto& pair : tasks_) {
            all_tasks.push_back(pair.second.task);
        }
        std::sort(all_tasks.begin(), all_tasks.end(),
                  [](const Task& lhs, const Task& rhs) {
                      return lhs.taskId < rhs.taskId;
                  });

        if (offset >= all_tasks.size()) {
            return result;
        }

        const std::size_t end =
            std::min(all_tasks.size(), offset + kTaskPageSize);
        for (std::size_t i = offset; i < end; ++i) {
            result.tasks.push_back(all_tasks[i]);
        }
        if (end < all_tasks.size()) {
            result.nextCursor = std::to_string(end);
        }
        return result;
    }

    Task cancelTask(const std::string& task_id) {
        if (task_id.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "tasks/cancel requires field: taskId");
        }

        std::lock_guard<std::mutex> lock(tasks_mutex_);
        auto it = tasks_.find(task_id);
        if (it == tasks_.end()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Task not found: " + task_id);
        }

        it->second.task.status = TaskStatus::kCancelled;
        it->second.task.statusMessage = "Cancelled by client";
        it->second.task.lastUpdatedAt = makeUtcTimestamp();

        notifyTaskStatus(it->second.task);
        return it->second.task;
    }

    ElicitCreateResult createElicitation(const json& params) {
        std::function<json(const json&)> handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            handler = elicitation_handler_;
        }

        ElicitCreateResult result;
        if (handler) {
            result = ElicitCreateResult::fromJson(handler(params));
        } else {
            result.action = "cancel";
        }

        if (params.contains("elicitationId") &&
            params["elicitationId"].is_string()) {
            notifyElicitationComplete(
                params["elicitationId"].get<std::string>(), result);
        }

        return result;
    }

    CompleteResult complete(const json& params) {
        std::function<json(const json&)> handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            handler = completion_handler_;
        }

        if (handler) {
            return CompleteResult::fromJson(handler(params));
        }

        return CompleteResult{};
    }

    CreateMessageResult createMessage(const json& params) {
        std::function<json(const json&)> handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            handler = sampling_handler_;
        }

        if (handler) {
            return CreateMessageResult::fromJson(handler(params));
        }

        return CreateMessageResult{};
    }

    std::vector<Prompt> listPrompts() const {
        std::lock_guard<std::mutex> lock(prompts_mutex_);
        mcp_logger::getModuleLogger("mcp_server")->debug("Listing prompts");
        return prompts_;
    }

    PromptResult getPrompt(const std::string& name, const json& arguments) {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Getting prompt: " + name);

        PromptGetHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            auto it = prompt_handlers_.find(name);
            if (it != prompt_handlers_.end()) {
                handler = it->second;
            }
        }

        if (!handler) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Unknown prompt: " + name);
        }

        return handler(name, arguments);
    }

    void setLoggingLevel(const json& params) {
        const mcp_logger::LogLevel level = internal::parseLogLevel(params);
        mcp_logger::Logger::getInstance().setLevel(level);
        transport_->setLogLevel(static_cast<int>(level));
    }

    bool sendNotification(const std::string& method, const json& params) {
        if (!running_) {
            mcp_logger::getModuleLogger("mcp_server")
                ->warning("Cannot send notification while server is stopped: " +
                          method);
            return false;
        }

        const bool sent = internal::forwardServerNotification(
            running_,
            [this](const std::string& notification_method,
                   const std::string& notification_params) {
                return transport_->sendNotification(notification_method,
                                                    notification_params);
            },
            method, params);

        if (!sent) {
            mcp_logger::getModuleLogger("mcp_server")
                ->warning("Failed to send server notification: " + method);
        }
        return sent;
    }

    void setServerInfo(const std::string& name, const std::string& version) {
        if (name.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Server name must be non-empty");
        }
        if (version.empty()) {
            throw mcp_exception(ErrorCode::kInvalidParams,
                                "Server version must be non-empty");
        }

        server_info_.name = name;
        server_info_.version = version;
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Set server info: " + name + " v" + version);
    }

    void setCapabilities(const ServerCapabilities& capabilities) {
        server_capabilities_ = capabilities;
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Set server capabilities");
    }

    bool registerTool(const Tool& tool, ToolCallHandler handler) {
        if (tool.name.empty() || !handler) {
            return false;
        }

        std::lock_guard<std::mutex> tools_lock(tools_mutex_);
        auto existing = std::find_if(
            tools_.begin(), tools_.end(),
            [&tool](const Tool& item) { return item.name == tool.name; });
        if (existing != tools_.end()) {
            return false;
        }

        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            tool_handlers_[tool.name] = handler;
        }
        tools_.push_back(tool);
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Registered tool: " + tool.name);
        return true;
    }

    bool unregisterTool(const std::string& name) {
        std::lock_guard<std::mutex> lock(tools_mutex_);
        auto old_size = tools_.size();
        tools_.erase(std::remove_if(tools_.begin(), tools_.end(),
                                    [&name](const Tool& tool) {
                                        return tool.name == name;
                                    }),
                     tools_.end());
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            tool_handlers_.erase(name);
        }

        const bool removed = old_size != tools_.size();
        if (removed) {
            mcp_logger::getModuleLogger("mcp_server")
                ->debug("Unregistered tool: " + name);
        }
        return removed;
    }

    void clearTools() {
        std::lock_guard<std::mutex> lock(tools_mutex_);
        tools_.clear();
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            tool_handlers_.clear();
        }
        mcp_logger::getModuleLogger("mcp_server")->debug("Cleared all tools");
    }

    bool registerResource(const Resource& resource,
                          ResourceReadHandler handler) {
        if (resource.uri.empty() || !handler) {
            return false;
        }

        std::lock_guard<std::mutex> resources_lock(resources_mutex_);
        auto existing = std::find_if(resources_.begin(), resources_.end(),
                                     [&resource](const Resource& item) {
                                         return item.uri == resource.uri;
                                     });
        if (existing != resources_.end()) {
            return false;
        }

        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            resource_handlers_[resource.uri] = handler;
        }
        resources_.push_back(resource);
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Registered resource: " + resource.uri);
        return true;
    }

    bool unregisterResource(const std::string& uri) {
        std::lock_guard<std::mutex> lock(resources_mutex_);
        auto old_size = resources_.size();
        resources_.erase(std::remove_if(resources_.begin(), resources_.end(),
                                        [&uri](const Resource& resource) {
                                            return resource.uri == uri;
                                        }),
                         resources_.end());
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            resource_handlers_.erase(uri);
        }

        const bool removed = old_size != resources_.size();
        if (removed) {
            std::lock_guard<std::mutex> subscription_lock(
                resource_subscriptions_mutex_);
            resource_subscriptions_.erase(uri);
            mcp_logger::getModuleLogger("mcp_server")
                ->debug("Unregistered resource: " + uri);
        }
        return removed;
    }

    void clearResources() {
        std::lock_guard<std::mutex> lock(resources_mutex_);
        resources_.clear();
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            resource_handlers_.clear();
        }
        {
            std::lock_guard<std::mutex> subscription_lock(
                resource_subscriptions_mutex_);
            resource_subscriptions_.clear();
        }
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Cleared all resources");
    }

    bool registerResourceTemplate(const ResourceTemplate& resource_template) {
        if (resource_template.uriTemplate.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(resource_templates_mutex_);
        auto existing = std::find_if(
            resource_templates_.begin(), resource_templates_.end(),
            [&resource_template](const ResourceTemplate& item) {
                return item.uriTemplate == resource_template.uriTemplate;
            });
        if (existing != resource_templates_.end()) {
            return false;
        }

        resource_templates_.push_back(resource_template);
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Registered resource template: " +
                    resource_template.uriTemplate);
        return true;
    }

    bool unregisterResourceTemplate(const std::string& uri_template) {
        std::lock_guard<std::mutex> lock(resource_templates_mutex_);
        auto old_size = resource_templates_.size();
        resource_templates_.erase(
            std::remove_if(resource_templates_.begin(),
                           resource_templates_.end(),
                           [&uri_template](const ResourceTemplate& item) {
                               return item.uriTemplate == uri_template;
                           }),
            resource_templates_.end());

        const bool removed = old_size != resource_templates_.size();
        if (removed) {
            mcp_logger::getModuleLogger("mcp_server")
                ->debug("Unregistered resource template: " + uri_template);
        }
        return removed;
    }

    void clearResourceTemplates() {
        std::lock_guard<std::mutex> lock(resource_templates_mutex_);
        resource_templates_.clear();
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Cleared all resource templates");
    }

    bool registerPrompt(const Prompt& prompt, PromptGetHandler handler) {
        if (prompt.name.empty() || !handler) {
            return false;
        }

        std::lock_guard<std::mutex> prompts_lock(prompts_mutex_);
        auto existing = std::find_if(
            prompts_.begin(), prompts_.end(),
            [&prompt](const Prompt& item) { return item.name == prompt.name; });
        if (existing != prompts_.end()) {
            return false;
        }

        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            prompt_handlers_[prompt.name] = handler;
        }
        prompts_.push_back(prompt);
        if (server_capabilities_.prompts.empty()) {
            server_capabilities_.prompts = json{{"listChanged", false}};
        }
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Registered prompt: " + prompt.name);
        return true;
    }

    bool unregisterPrompt(const std::string& name) {
        std::lock_guard<std::mutex> lock(prompts_mutex_);
        auto old_size = prompts_.size();
        prompts_.erase(std::remove_if(prompts_.begin(), prompts_.end(),
                                      [&name](const Prompt& prompt) {
                                          return prompt.name == name;
                                      }),
                       prompts_.end());
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            prompt_handlers_.erase(name);
        }

        const bool removed = old_size != prompts_.size();
        if (removed) {
            mcp_logger::getModuleLogger("mcp_server")
                ->debug("Unregistered prompt: " + name);
        }
        return removed;
    }

    void clearPrompts() {
        std::lock_guard<std::mutex> lock(prompts_mutex_);
        prompts_.clear();
        {
            std::lock_guard<std::mutex> handlers_lock(handlers_mutex_);
            prompt_handlers_.clear();
        }
        mcp_logger::getModuleLogger("mcp_server")->debug("Cleared all prompts");
    }

    bool notifyResourcesChanged() {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Notifying resources changed");
        bool list_changed_sent = sendNotification(
            methods::NOTIFICATION_RESOURCES_LIST_CHANGED, json::object());

        std::vector<std::string> subscribed_uris;
        {
            std::lock_guard<std::mutex> lock(resource_subscriptions_mutex_);
            subscribed_uris.assign(resource_subscriptions_.begin(),
                                   resource_subscriptions_.end());
        }

        bool updated_sent = false;
        for (const auto& uri : subscribed_uris) {
            updated_sent =
                sendNotification(methods::NOTIFICATION_RESOURCE_UPDATED,
                                 json{{"uri", uri}}) ||
                updated_sent;
        }

        if (!list_changed_sent && updated_sent) {
            // If SSE stream readiness races with the first emit, retry
            // list_changed once.
            list_changed_sent = sendNotification(
                methods::NOTIFICATION_RESOURCES_LIST_CHANGED, json::object());
        }

        return list_changed_sent || updated_sent;
    }

    bool notifyToolsChanged() {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Notifying tools changed");
        return sendNotification(methods::NOTIFICATION_TOOLS_LIST_CHANGED,
                                json::object());
    }

    bool notifyPromptsChanged() {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Notifying prompts changed");
        return sendNotification(methods::NOTIFICATION_PROMPTS_LIST_CHANGED,
                                json::object());
    }

    // Get ConnectionInfo by session_id
    std::optional<mcp_transport::ConnectionInfo> getConnectionInfoBySessionId(
        const std::string& session_id) {
        try {
            auto attributes_opt =
                session_manager_.GetSessionAttributes(session_id);
            if (!attributes_opt.has_value()) {
                return std::nullopt;
            }

            const auto& attributes = attributes_opt.value();
            auto it = attributes.find("connection_info");
            if (it == attributes.end()) {
                return std::nullopt;
            }

            const nlohmann::json connection_info_json =
                nlohmann::json::parse(it->second);
            return mcp_transport::ConnectionInfo::fromJson(
                connection_info_json);
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("mcp_server")
                ->error(
                    "Error retrieving ConnectionInfo from SessionManager: " +
                    std::string(e.what()));
            return std::nullopt;
        }
    }

    // Validate session_id
    bool validateSession(const std::string& session_id) {
        return session_manager_.ValidateSession(session_id);
    }

    // Remove session
    bool removeSession(const std::string& session_id) {
        return session_manager_.RemoveSession(session_id);
    }

    // Getter for server_info_
    const ServerInfo& getServerInfo() const { return server_info_; }

    // ---- Roots ----

    bool requestRootsListInternal(int timeout_ms,
                                  std::vector<Root>& roots_out) {
        roots_out.clear();

        if (!running_) {
            mcp_logger::getModuleLogger("mcp_server")
                ->warning("Cannot request roots/list while server is stopped");
            return false;
        }

        // Build a JSON-RPC request for roots/list
        mcp_transport::TransportMessage msg;
        msg.content = request::create(methods::ROOTS_LIST).toJson().dump();
        msg.method = "POST";
        msg.path = "/";
        msg.timestamp = std::chrono::system_clock::now();

        try {
            auto future = transport_->sendRequest(msg);
            if (future.wait_for(std::chrono::milliseconds(timeout_ms)) !=
                std::future_status::ready) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list request timed out");
                return false;
            }

            auto response_msg = future.get();
            if (response_msg.status_code != 200) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list transport error: status=" +
                              std::to_string(response_msg.status_code) +
                              ", error=" + response_msg.error_message +
                              ", content=" + response_msg.content);
                if (response_msg.status_code == 503 &&
                    (response_msg.error_message.find(
                         "No SSE clients connected") != std::string::npos ||
                     response_msg.content.find("No SSE clients connected") !=
                         std::string::npos)) {
                    mcp_logger::getModuleLogger("mcp_server")
                        ->warning(
                            "roots/list requires an active SSE reverse "
                            "channel. "
                            "If using MCP Inspector, set Transport Type to SSE "
                            "and ensure the event stream is established.");
                }
                return false;
            }

            if (response_msg.content.empty()) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list returned empty response body");
                return false;
            }

            const std::size_t first_non_ws =
                response_msg.content.find_first_not_of(" \t\r\n");
            if (first_non_ws == std::string::npos) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list returned whitespace-only response");
                return false;
            }
            const char first_char = response_msg.content[first_non_ws];
            if (first_char != '{' && first_char != '[') {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list returned non-JSON response: " +
                              response_msg.content);
                return false;
            }

            json response_json = json::parse(response_msg.content);
            if (response_json.contains("body") &&
                response_json["body"].is_string()) {
                const std::string body =
                    response_json["body"].get<std::string>();
                if (!body.empty()) {
                    response_json = json::parse(body);
                }
            }

            // Handle JSON-RPC response
            if (response_json.contains("error")) {
                mcp_logger::getModuleLogger("mcp_server")
                    ->warning("roots/list returned error: " +
                              response_json["error"].dump());
                return false;
            }

            json result = response_json;
            if (response_json.contains("result")) {
                result = response_json["result"];
            }
            if (result.contains("roots") && result["roots"].is_array()) {
                for (const auto& root_json : result["roots"]) {
                    roots_out.push_back(Root::fromJson(root_json));
                }
            }

            mcp_logger::getModuleLogger("mcp_server")
                ->info("roots/list returned " +
                       std::to_string(roots_out.size()) + " root(s)");
            return true;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("mcp_server")
                ->error("roots/list request failed: " + std::string(e.what()));
            return false;
        }
    }

    std::vector<Root> requestRootsList(int timeout_ms) {
        std::vector<Root> roots;
        (void)requestRootsListInternal(timeout_ms, roots);
        return roots;
    }

    ElicitCreateResult requestElicitation(const json& params, int timeout_ms) {
        const json result =
            requestClientRpc(methods::ELICITATION_CREATE, params, timeout_ms);
        return ElicitCreateResult::fromJson(result);
    }

    CreateMessageResult requestCreateMessage(const json& params,
                                             int timeout_ms) {
        const json result = requestClientRpc(methods::SAMPLING_CREATE_MESSAGE,
                                             params, timeout_ms);
        return CreateMessageResult::fromJson(result);
    }

    void setRootsListChangedHandler(RootsListChangedHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        roots_list_changed_handler_ = std::move(handler);
    }

    void handleRootsListChanged() override {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Received notifications/roots/list_changed");

        RootsListChangedHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            handler = roots_list_changed_handler_;
        }

        if (handler) {
            std::thread([this, handler = std::move(handler)]() mutable {
                std::vector<Root> roots;
                if (!requestRootsListInternal(10000, roots)) {
                    mcp_logger::getModuleLogger("mcp_server")
                        ->warning(
                            "roots/list request failed after "
                            "notifications/roots/list_changed; invoking "
                            "callback with empty roots");
                }
                handler(roots);
            }).detach();
        }
    }

    // ---- Progress / Cancellation ----

    bool sendProgress(const ProgressParams& params) {
        mcp_logger::getModuleLogger("mcp_server")
            ->debug("Sending progress notification");
        return sendNotification(methods::NOTIFICATION_PROGRESS,
                                params.toJson());
    }

    bool notifyTaskStatus(const Task& task) {
        return sendNotification(methods::NOTIFICATION_TASKS_STATUS,
                                task.toJson());
    }

    bool notifyElicitationComplete(const std::string& elicitation_id,
                                   const ElicitCreateResult& result) {
        if (elicitation_id.empty()) {
            return false;
        }
        return sendNotification(methods::NOTIFICATION_ELICITATION_COMPLETE,
                                json{{"elicitationId", elicitation_id},
                                     {"result", result.toJson()}});
    }

    void setCancelHandler(CancelHandler handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        cancel_handler_ = std::move(handler);
    }

    void handleCancelRequest(const CancelParams& params) override {
        mcp_logger::getModuleLogger("mcp_server")
            ->info("Received $/cancelRequest for request: " +
                   params.requestId.dump());

        CancelHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            handler = cancel_handler_;
        }

        if (handler) {
            handler(params);
        } else {
            mcp_logger::getModuleLogger("mcp_server")
                ->debug(
                    "No cancel handler registered, ignoring $/cancelRequest");
        }
    }

    void setElicitationHandler(std::function<json(const json&)> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        elicitation_handler_ = std::move(handler);
    }

    void setCompletionHandler(std::function<json(const json&)> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        completion_handler_ = std::move(handler);
    }

    void setSamplingHandler(std::function<json(const json&)> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        sampling_handler_ = std::move(handler);
    }

    bool upsertTask(const Task& task, const json& result) {
        if (task.taskId.empty()) {
            return false;
        }

        Task task_copy = task;
        if (task_copy.createdAt.empty()) {
            task_copy.createdAt = makeUtcTimestamp();
        }
        if (task_copy.lastUpdatedAt.empty()) {
            task_copy.lastUpdatedAt = task_copy.createdAt;
        }

        std::lock_guard<std::mutex> lock(tasks_mutex_);
        TaskStorage& entry = tasks_[task.taskId];
        entry.task = std::move(task_copy);
        if (!result.is_null()) {
            entry.result = result;
        }
        return true;
    }

    bool removeTask(const std::string& task_id) {
        if (task_id.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(tasks_mutex_);
        return tasks_.erase(task_id) > 0;
    }

private:
    json requestClientRpc(const std::string& method, const json& params,
                          int timeout_ms) {
        if (!running_) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Cannot send request while server is stopped");
        }

        mcp_transport::TransportMessage msg;
        msg.content = request::create(method, params).toJson().dump();
        msg.method = "POST";
        msg.path = "/";
        msg.timestamp = std::chrono::system_clock::now();

        auto future = transport_->sendRequest(msg);
        if (future.wait_for(std::chrono::milliseconds(timeout_ms)) !=
            std::future_status::ready) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Client request timed out: " + method);
        }

        auto response_msg = future.get();
        if (response_msg.status_code != 200) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Client request transport error: " + method +
                    ", status=" + std::to_string(response_msg.status_code));
        }

        if (response_msg.content.empty()) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Client request returned empty body: " + method);
        }

        json response_json = json::parse(response_msg.content);
        if (response_json.contains("result")) {
            return response_json["result"];
        }

        if (response_json.contains("error")) {
            throw mcp_exception(
                ErrorCode::kInternalError,
                "Client request failed: " + response_json["error"].dump());
        }

        return response_json;
    }

    bool running_;
    ServerInfo server_info_;
    ServerCapabilities server_capabilities_;
    ClientInfo client_info_;
    ClientCapabilities client_capabilities_;
    bool client_initialized_ = false;

    std::unique_ptr<mcp_transport::Transport> transport_;

    // Session management
    SessionManager session_manager_;

    mutable std::mutex tools_mutex_;
    std::vector<Tool> tools_;

    mutable std::mutex resources_mutex_;
    std::vector<Resource> resources_;

    mutable std::mutex resource_templates_mutex_;
    std::vector<ResourceTemplate> resource_templates_;

    mutable std::mutex prompts_mutex_;
    std::vector<Prompt> prompts_;

    mutable std::mutex tasks_mutex_;
    struct TaskStorage {
        Task task;
        std::optional<json> result;
    };
    std::map<std::string, TaskStorage> tasks_;

    mutable std::mutex resource_subscriptions_mutex_;
    std::set<std::string> resource_subscriptions_;

    std::mutex handlers_mutex_;
    std::map<std::string, ToolCallHandler> tool_handlers_;
    std::map<std::string, ResourceReadHandler> resource_handlers_;
    std::map<std::string, PromptGetHandler> prompt_handlers_;
    std::function<json(const json&)> elicitation_handler_;
    std::function<json(const json&)> completion_handler_;
    std::function<json(const json&)> sampling_handler_;
    RootsListChangedHandler roots_list_changed_handler_;
    CancelHandler cancel_handler_;
};

// MCP Server Connection Handler for Transport (SSE connections)
class MCPServerConnectionHandler
    : public mcp_transport::SseServerConnectHandler {
public:
    explicit MCPServerConnectionHandler(
        internal::MCPServerHandlerContext* server_impl)
        : server_impl_(server_impl) {}

    bool handleRequest(const mcp_transport::TransportHttpMessage& request,
                       mcp_transport::TransportHttpMessage& response) override {
        auto logger = mcp_logger::getModuleLogger("mcp_server");
        logger->info("🔌 Handling SSE connection request: method=" +
                     request.method + ", path=" + request.path);

        // Initialize response
        response = mcp_transport::TransportHttpMessage{};
        response.id = request.id;
        response.status_code = 200;
        response.headers["Content-Type"] = "application/json";
        response.headers["Cache-Control"] = "no-cache";
        response.headers["Connection"] = "keep-alive";

        // Record request metadata
        if (!request.body.empty()) {
            const std::size_t preview_len =
                std::min<std::size_t>(request.body.size(), 256);
            logger->debug(
                "📦 Connection request body preview: " +
                request.body.substr(0, preview_len) +
                (request.body.size() > preview_len ? "...(truncated)" : ""));
        }

        // Check whether server is running
        if (!server_impl_ || !server_impl_->isRunning()) {
            logger->warning(
                "⚠️ Server is not running, rejecting connection");
            response.status_code = 503;
            response.body =
                R"({"jsonrpc":"2.0","error":{"code":-32000,"message":"Server not available"}})";
            return true;
        }

        // Validate request path (if required)
        // Add path validation logic here if needed
        if (!request.path.empty()) {
            logger->debug("📍 Connection path: " + request.path);
        }

        // Check whether headers include session ID (client may already have
        // one)
        std::string client_session_id;
        for (const auto& header : request.headers) {
            if (header.first == "X-Session-Id" ||
                header.first == "x-session-id") {
                client_session_id = header.second;
                break;
            }
        }

        // If client provided session ID, validate it
        if (!client_session_id.empty()) {
            if (validateSessionId(client_session_id)) {
                logger->info("📋 Client provided valid session ID: " +
                             client_session_id);
            } else {
                logger->warning(
                    "⚠️ Client provided invalid session ID format, will "
                    "generate new one");
                client_session_id
                    .clear();  // Clear invalid session ID and let transport
                               // layer generate a new one
            }
        }

        // Build success response
        nlohmann::json result;
        result["status"] = "ready";
        result["server"] = server_impl_->getServerInfo().name;
        result["version"] = server_impl_->getServerInfo().version;
        result["protocol"] = PROTOCOL_VERSION;
        // Note: session_id is generated by transport layer and returned via
        // X-Session-Id header If client provided valid session_id, record it
        // here
        if (!client_session_id.empty()) {
            result["session_id"] =
                client_session_id;  // Return client-provided session ID
        }
        // If client did not provide one or it is invalid, transport
        // layer generates a new one and returns it in response header

        nlohmann::json response_json;
        response_json["jsonrpc"] = "2.0";
        response_json["result"] = result;

        response.body = response_json.dump();

        logger->info("✅ SSE connection accepted for path: " + request.path);
        return true;
    }

private:
    internal::MCPServerHandlerContext* server_impl_;
};

// MCP Server Request Handler for Transport
class MCPServerRequestHandler
    : public mcp_transport::TransportServerRequestHandler {
public:
    explicit MCPServerRequestHandler(
        internal::MCPServerHandlerContext* server_impl)
        : server_impl_(server_impl) {}

    std::string handleRequest(const std::string& method,
                              const std::string& params,
                              const std::string& request_id) override {
        auto logger = mcp_logger::getModuleLogger("mcp_server");
        logger->info("Handling request: " + method + " (ID: " + request_id +
                     ")");

        // Session ID validation disabled - session_id is no longer validated
        // std::string session_id;
        // try {
        //     // Try extracting session_id from params
        //     json params_json = json::parse(params);
        //     if (params_json.contains("session_id")) {
        //         session_id = params_json["session_id"].get<std::string>();
        //     } else if (validateSessionId(request_id)) {
        //         // If request_id itself is a valid session_id, use it
        //         session_id = request_id;
        //     }
        // } catch (const std::exception&) {
        //     // If parsing fails, try request_id
        //     if (validateSessionId(request_id)) {
        //         session_id = request_id;
        //     }
        // }

        // Session ID validation disabled - no session validation
        // if (method != methods::INITIALIZE && !session_id.empty()) {
        //     if (!server_impl_->validateSession(session_id)) {
        //         logger->warning("⚠️ Invalid or expired session ID: " +
        //         session_id); return json{{"jsonrpc", "2.0"},
        //                    {"id", request_id},
        //                    {"error", {{"code", -32001}, {"message", "Invalid
        //                    or expired session"}}}}.dump();
        //     }
        //     logger->debug("✅ Session validated: " + session_id);
        // } else if (method != methods::INITIALIZE && session_id.empty()) {
        //     logger->warning("⚠️ Missing session ID for method: " +
        //     method); return json{{"jsonrpc", "2.0"},
        //                {"id", request_id},
        //                {"error", {{"code", -32001}, {"message", "Session ID
        //                required"}}}}.dump();
        // }

        logger->debug("Processing request without session validation: " +
                      method);

        return internal::handleServerRequest(*server_impl_, method, params,
                                             request_id);
    }

    void handleNotification(const std::string& method,
                            const std::string& params) override {
        auto logger = mcp_logger::getModuleLogger("mcp_server");
        logger->info("Handling notification: " + method);

        // Session ID validation disabled - session_id is no longer validated
        // std::string session_id;
        // try {
        //     json params_json = json::parse(params);
        //     if (params_json.contains("session_id")) {
        //         session_id = params_json["session_id"].get<std::string>();
        //     }
        // } catch (const std::exception&) {
        //     // Parse failure: continue (some notifications may not need
        //     session_id)
        // }

        // Session ID validation disabled - no session validation
        // if (method != methods::NOTIFICATION_INITIALIZED &&
        // !session_id.empty()) {
        //     if (!server_impl_->validateSession(session_id)) {
        //         logger->warning("⚠️ Invalid or expired session ID for
        //         notification: " + session_id); return; //
        //         On notification failure, do not return error; only log
        //     }
        //     logger->debug("✅ Session validated for notification: " +
        //     session_id);
        // } else if (method != methods::NOTIFICATION_INITIALIZED &&
        // session_id.empty()) {
        //     logger->warning("⚠️ Missing session ID for notification: " +
        //     method); return; // On notification failure, do not return error;
        //     only log
        // }

        logger->debug("Processing notification without session validation: " +
                      method);

        try {
            internal::handleServerNotification(*server_impl_, method, params);
        } catch (const mcp_exception& e) {
            logger->error("Notification handling error: " +
                          std::string(e.what()));
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("mcp_server")
                ->error("Notification handling error: " +
                        std::string(e.what()));
        }
    }

private:
    internal::MCPServerHandlerContext* server_impl_;
};

// MCPServer public interface
MCPServer::MCPServer() : impl_(std::make_unique<Impl>()) {
    mcp_logger::getModuleLogger("mcp_server")->debug("MCPServer created");
}

MCPServer::~MCPServer() {
    mcp_logger::getModuleLogger("mcp_server")->debug("MCPServer destroyed");
}

bool MCPServer::start(const mcp_transport::TransportConfig& config) {
    return impl_->start(config);
}

void MCPServer::stop() { impl_->stop(); }

bool MCPServer::isRunning() const { return impl_->isRunning(); }

std::vector<Tool> MCPServer::listTools() const { return impl_->listTools(); }

std::vector<Resource> MCPServer::listResources() const {
    return impl_->listResources();
}

bool MCPServer::registerResourceTemplate(
    const ResourceTemplate& resource_template) {
    return impl_->registerResourceTemplate(resource_template);
}

bool MCPServer::unregisterResourceTemplate(const std::string& uri_template) {
    return impl_->unregisterResourceTemplate(uri_template);
}

void MCPServer::clearResourceTemplates() { impl_->clearResourceTemplates(); }

std::vector<ResourceTemplate> MCPServer::listResourceTemplates() const {
    return impl_->listResourceTemplates();
}

std::vector<Prompt> MCPServer::listPrompts() const {
    return impl_->listPrompts();
}

void MCPServer::setServerInfo(const std::string& name,
                              const std::string& version) {
    impl_->setServerInfo(name, version);
}

ServerInfo MCPServer::getServerInfo() const { return impl_->getServerInfo(); }

void MCPServer::setCapabilities(const ServerCapabilities& capabilities) {
    impl_->setCapabilities(capabilities);
}

bool MCPServer::registerTool(const Tool& tool, ToolCallHandler handler) {
    return impl_->registerTool(tool, handler);
}

bool MCPServer::unregisterTool(const std::string& name) {
    return impl_->unregisterTool(name);
}

void MCPServer::clearTools() { impl_->clearTools(); }

bool MCPServer::registerResource(const Resource& resource,
                                 ResourceReadHandler handler) {
    return impl_->registerResource(resource, handler);
}

bool MCPServer::unregisterResource(const std::string& uri) {
    return impl_->unregisterResource(uri);
}

void MCPServer::clearResources() { impl_->clearResources(); }

void MCPServer::subscribeResource(const std::string& uri) {
    impl_->subscribeResource(uri);
}

void MCPServer::unsubscribeResource(const std::string& uri) {
    impl_->unsubscribeResource(uri);
}

bool MCPServer::registerPrompt(const Prompt& prompt, PromptGetHandler handler) {
    return impl_->registerPrompt(prompt, handler);
}

bool MCPServer::unregisterPrompt(const std::string& name) {
    return impl_->unregisterPrompt(name);
}

void MCPServer::clearPrompts() { impl_->clearPrompts(); }

bool MCPServer::notifyResourcesChanged() {
    return impl_->notifyResourcesChanged();
}

bool MCPServer::notifyToolsChanged() { return impl_->notifyToolsChanged(); }

bool MCPServer::notifyPromptsChanged() { return impl_->notifyPromptsChanged(); }

std::vector<Root> MCPServer::requestRootsList(int timeout_ms) {
    return impl_->requestRootsList(timeout_ms);
}

void MCPServer::setRootsListChangedHandler(RootsListChangedHandler handler) {
    impl_->setRootsListChangedHandler(std::move(handler));
}

ElicitCreateResult MCPServer::requestElicitation(const json& params,
                                                 int timeout_ms) {
    return impl_->requestElicitation(params, timeout_ms);
}

CreateMessageResult MCPServer::requestCreateMessage(const json& params,
                                                    int timeout_ms) {
    return impl_->requestCreateMessage(params, timeout_ms);
}

bool MCPServer::sendProgress(const ProgressParams& params) {
    return impl_->sendProgress(params);
}

bool MCPServer::notifyTaskStatus(const Task& task) {
    return impl_->notifyTaskStatus(task);
}

bool MCPServer::notifyElicitationComplete(const std::string& elicitation_id,
                                          const ElicitCreateResult& result) {
    return impl_->notifyElicitationComplete(elicitation_id, result);
}

void MCPServer::setCancelHandler(CancelHandler handler) {
    impl_->setCancelHandler(std::move(handler));
}

void MCPServer::setElicitationHandler(
    const std::function<json(const json&)>& handler) {
    impl_->setElicitationHandler(handler);
}

void MCPServer::setCompletionHandler(
    const std::function<json(const json&)>& handler) {
    impl_->setCompletionHandler(handler);
}

void MCPServer::setSamplingHandler(
    const std::function<json(const json&)>& handler) {
    impl_->setSamplingHandler(handler);
}

bool MCPServer::upsertTask(const Task& task, const json& result) {
    return impl_->upsertTask(task, result);
}

bool MCPServer::removeTask(const std::string& task_id) {
    return impl_->removeTask(task_id);
}

}  // namespace mcp
