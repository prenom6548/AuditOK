/**
 * @file client.h
 * @brief MCP client facade for transport and protocol operations.
 *
 * Exposes high-level APIs to connect to an MCP server, run protocol methods,
 * register handlers, and query negotiated server metadata.
 */

#ifndef KYSDK_MCP_CLIENT_H
#define KYSDK_MCP_CLIENT_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/protocol.h"

namespace mcp {}  // namespace mcp

namespace mcp_transport {
enum class TransportType : int;
enum class TransportStatus : int;
struct TransportConfig;
}  // namespace mcp_transport

namespace mcp {

struct ReconnectEvent {
    bool success = false;
    std::string reason;
    mcp_transport::TransportStatus status;
};

using ReconnectHandler = std::function<bool(const ReconnectEvent&)>;

/**
 * @brief MCP client facade.
 *
 * Wraps the transport layer and exposes synchronous helpers for MCP requests
 * and notifications.
 *
 * Thread-safety:
 * - Concurrent method calls are not guaranteed to be fully thread-safe.
 * - Handler registration is internally synchronized.
 * - Inbound handlers run on transport callback threads and are not guaranteed
 *   to run on the main thread.
 * - Applications must synchronize access to shared business state in handlers.
 */
class MCPClient {
public:
    /**
     * @brief Construct an MCP client instance.
     */
    MCPClient();

    /**
     * @brief Destroy the client instance.
     *
     * Ensures the transport is disconnected before destruction.
     */
    virtual ~MCPClient();

    /**
     * @brief Connect to a server endpoint string.
     *
     * Supported endpoint formats include:
     * - `cmd:<command ...>` for IPC process transport.
     * - `http(s)://host[:port][/path]` for SSE transport.
     * - Other non-empty values without `://` are treated as IPC command lines
     *   unless transport type is explicitly set to SSE.
     *
     * @param endpoint Endpoint string.
     * @return `true` when transport startup succeeds, otherwise `false`.
     * @throws mcp_exception with `kInvalidParams` if endpoint is empty, a
     *         `cmd:` endpoint has no command, the HTTP(S) port is invalid, or
     *         the endpoint contains an unsupported scheme.
     */
    bool connect(const std::string& endpoint);

    /**
     * @brief Connect with an explicit transport configuration.
     * @param config Transport configuration.
     * @return `true` when transport startup succeeds, otherwise `false`.
     */
    bool connectWithConfig(const mcp_transport::TransportConfig& config);

    /**
     * @brief Disconnect and stop the underlying transport.
     */
    void disconnect();

    /**
     * @brief Reconnect transport and run the registered session recovery
     * handler.
     * @return `true` when transport reconnect and recovery succeed.
     */
    bool reconnect();

    /**
     * @brief Set handler invoked after manual or automatic reconnect attempts.
     * @param handler Recovery callback. Empty handler restores default
     * behavior.
     */
    void setReconnectHandler(ReconnectHandler handler);

    /**
     * @brief Query whether the client is currently connected.
     * @return `true` when connection state and transport state are both active.
     */
    bool isConnected() const;

    /**
     * @brief Set transport type for subsequent `connect(endpoint)` parsing.
     * @param type Transport type policy.
     *
     * Behavior:
     * - `kAuto`: endpoint string determines transport type.
     * - `kIpc`: endpoint is parsed as IPC command.
     * - `kSse`: endpoint must be `http(s)://...`, otherwise `connect` throws.
     */
    void setTransportType(mcp_transport::TransportType type);

    /**
     * @brief Get the currently configured transport type.
     * @return Transport type value.
     */
    mcp_transport::TransportType getTransportType() const;

    /**
     * @brief Get current runtime status from the transport layer.
     * @return Transport status enum.
     */
    mcp_transport::TransportStatus getTransportStatus() const;

    /**
     * @brief Send MCP `initialize` request.
     * @param params Initialize payload.
     * @return Parsed initialize result from server.
     * @throws mcp_exception on invalid parameters or server-side failure.
     */
    InitializeResult initialize(const InitializeParams& params);

    /**
     * @brief Send MCP `notifications/initialized`.
     */
    void initialized();

    /**
     * @brief Send MCP `ping` request.
     * @throws mcp_exception on request failure.
     */
    void ping();

    /**
     * @brief Request the tool list from server.
     * @return Strongly-typed paginated tool list result.
     * @throws mcp_exception on request failure.
     */
    ListToolsResult listTools(const std::string& cursor = "");

    /**
     * @brief Invoke a tool with explicit call parameters.
     * @param params Tool call payload.
     * @return Tool call result. On remote error, `isError` is set in result.
     */
    ToolCallResult callTool(const ToolCallParams& params);

    /**
     * @brief Invoke a tool by name and JSON arguments.
     * @param name Tool name.
     * @param arguments Tool arguments. Defaults to empty object.
     * @return Tool call result. On remote error, `isError` is set in result.
     */
    ToolCallResult callTool(const std::string& name,
                            const json& arguments = json::object());

    /**
     * @brief Request the resource list from server.
     * @return Strongly-typed paginated resource list result.
     * @throws mcp_exception on request failure.
     */
    ListResourcesResult listResources(const std::string& cursor = "");

    /**
     * @brief Request resource templates from server.
     * @return Strongly-typed paginated resource template list result.
     * @throws mcp_exception on request failure or invalid response payload.
     */
    ListResourceTemplatesResult listResourceTemplates(
        const std::string& cursor = "");

    /**
     * @brief Read resource content by URI.
     * @param uri Resource URI.
     * @return Strongly-typed resource read result.
     * @throws mcp_exception on request failure.
     */
    ReadResourceResult readResource(const std::string& uri);

    /**
     * @brief Request the prompt list from server.
     * @return Strongly-typed paginated prompt list result.
     * @throws mcp_exception on request failure.
     */
    ListPromptsResult listPrompts(const std::string& cursor = "");

    /**
     * @brief Request a prompt by name.
     * @param name Prompt name.
     * @param arguments Prompt arguments. Defaults to empty object.
     * @return Strongly-typed prompt result.
     * @throws mcp_exception on request failure.
     */
    PromptResult getPrompt(const std::string& name,
                           const json& arguments = json::object());

    /**
     * @brief Send `logging/setLevel` request.
     * @param level Log level string (e.g. "debug", "info", "warning", "error").
     * @throws mcp_exception on request failure.
     */
    void setLoggingLevel(const std::string& level);

    /**
     * @brief Send `tasks/get` request.
     * @param task_id Task identifier.
     * @return Strongly-typed Task.
     * @throws mcp_exception on request failure.
     */
    Task getTask(const std::string& task_id);

    /**
     * @brief Send `tasks/result` request.
     * @param task_id Task identifier.
     * @return Strongly-typed task result payload.
     * @throws mcp_exception on request failure.
     */
    TaskResult getTaskResult(const std::string& task_id);

    /**
     * @brief Send `tasks/list` request.
     * @param cursor Optional pagination cursor.
     * @return Strongly-typed task list result.
     * @throws mcp_exception on request failure.
     */
    ListTasksResult listTasks(const std::string& cursor = "");

    /**
     * @brief Send `tasks/cancel` request.
     * @param task_id Task identifier.
     * @return Strongly-typed cancelled Task.
     * @throws mcp_exception on request failure.
     */
    Task cancelTask(const std::string& task_id);

    /**
     * @brief Send `completion/complete` request.
     * @param params Completion request payload.
     * @return Strongly-typed completion result.
     * @throws mcp_exception on request failure.
     */
    CompleteResult complete(const json& params);

    /**
     * @brief Send `resources/subscribe` request.
     * @param uri Resource URI.
     * @throws mcp_exception on request failure.
     */
    void subscribeResource(const std::string& uri);

    /**
     * @brief Send `resources/unsubscribe` request.
     * @param uri Resource URI.
     * @throws mcp_exception on request failure.
     */
    void unsubscribeResource(const std::string& uri);

    /**
     * @brief Send an arbitrary client notification.
     * @param method Notification method name.
     * @param params Notification payload. Defaults to empty object.
     *
     * If the client is not connected, this call is ignored.
     */
    void sendNotification(const std::string& method,
                          const json& params = json::object());

    /**
     * @brief Register handler for a server notification method.
     * @param method Notification method name.
     * @param handler Callback invoked when that method is received.
     */
    void setNotificationHandler(const std::string& method,
                                NotificationHandler handler);

    /**
     * @brief Register typed handler for `notifications/progress`.
     * @param handler Callback invoked when progress notification is received.
     */
    void setProgressHandler(ProgressHandler handler);

    /**
     * @brief Register typed handler for `$/cancelRequest`.
     * @param handler Callback invoked when cancel request notification is
     *                received.
     */
    void setCancelRequestHandler(CancelHandler handler);

    /**
     * @brief Register server-to-client tool call handler.
     * @param handler Callback used for inbound `tools/call`.
     */
    void setToolCallHandler(ToolCallHandler handler);

    /**
     * @brief Register server-to-client resource read handler.
     * @param handler Callback used for inbound `resources/read`.
     */
    void setResourceReadHandler(ResourceReadHandler handler);

    /**
     * @brief Register server-to-client prompt get handler.
     * @param handler Callback used for inbound `prompts/get`.
     */
    void setPromptGetHandler(PromptGetHandler handler);

    /**
     * @brief Register server-to-client elicitation handler.
     * @param handler Callback used for inbound `elicitation/create`.
     */
    void setElicitationHandler(ElicitationHandler handler);

    /**
     * @brief Register server-to-client sampling handler.
     * @param handler Callback used for inbound `sampling/createMessage`.
     */
    void setSamplingHandler(SamplingHandler handler);

    /**
     * @brief Set client identity sent during `initialize`.
     * @param name Client name.
     * @param version Client version string.
     * @throws mcp_exception with `kInvalidParams` if name or version is empty.
     */
    void setClientInfo(const std::string& name, const std::string& version);

    /**
     * @brief Set client capabilities sent during `initialize`.
     * @param capabilities Capability declaration.
     */
    void setCapabilities(const ClientCapabilities& capabilities);

    /**
     * @brief Replace endpoint candidates used by the client.
     * @param endpoints Ordered endpoint list.
     *
     * When non-empty, the first endpoint becomes current.
     */
    void setEndpoints(const std::vector<std::string>& endpoints);

    /**
     * @brief Set current endpoint path/address used for outgoing messages.
     * @param endpoint Current endpoint value.
     */
    void setCurrentEndpoint(const std::string& endpoint);

    /**
     * @brief Get the currently selected endpoint.
     * @return Endpoint string.
     */
    std::string getCurrentEndpoint() const;

    /**
     * @brief Get all configured endpoint candidates.
     * @return Endpoint list.
     */
    std::vector<std::string> getEndpoints() const;

    /**
     * @brief Set roots that may be returned for `roots/list` requests.
     * @param roots Root descriptors.
     */
    void setRoots(const std::vector<Root>& roots);

    /**
     * @brief Get currently configured roots.
     * @return Root descriptor list.
     */
    std::vector<Root> getRoots() const;

    /**
     * @brief Get server identity received from `initialize`.
     * @return Server info snapshot.
     */
    ServerInfo getServerInfo() const;

    /**
     * @brief Get server capabilities received from `initialize`.
     * @return Server capability snapshot.
     */
    ServerCapabilities getServerCapabilities() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace mcp

#endif  // KYSDK_MCP_CLIENT_H
