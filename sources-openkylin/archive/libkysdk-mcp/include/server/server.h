/**
 * @file server.h
 * @brief MCP Server Implementation
 *
 * This file provides the server-side implementation of the Model Context
 * Protocol (MCP). Handles server startup, client connections, and request
 * processing.
 */

#ifndef KYSDK_MCP_SERVER_H
#define KYSDK_MCP_SERVER_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "common/protocol.h"

namespace mcp_transport {
struct TransportConfig;
}

namespace mcp {

/**
 * @brief MCP server facade.
 *
 * Exposes the server-side API for starting an MCP endpoint, publishing
 * server metadata and capabilities, and registering handlers for tools,
 * resources, and prompts.
 *
 * Thread-safety:
 * - Handler registration and runtime callback execution can overlap with
 *   request processing.
 * - Tool/resource/prompt/sampling/elicitation handlers execute on request
 *   processing threads and are not guaranteed to run on the main thread.
 * - Applications must ensure handler reentrancy and protect shared state.
 */
class MCPServer {
public:
    /**
     * @brief Construct an MCP server instance.
     */
    MCPServer();

    /**
     * @brief Destroy the server instance.
     *
     * Stops the server first if it is still running.
     */
    virtual ~MCPServer();

    /**
     * @brief Start the server with the given transport configuration.
     * @param config Transport configuration used to create the listening
     *               endpoint and wire protocol handlers.
     * @return `true` if startup succeeded, otherwise `false`.
     */
    bool start(const mcp_transport::TransportConfig& config);

    /**
     * @brief Stop the server and release transport resources.
     */
    void stop();

    /**
     * @brief Query whether the server is currently running.
     * @return `true` if the transport layer has been started and not yet
     *         stopped, otherwise `false`.
     */
    bool isRunning() const;

    /**
     * @brief Set the server identity reported during MCP initialization.
     * @param name Human-readable server name.
     * @param version Server version string.
     * @throws mcp_exception with `kInvalidParams` if name or version is empty.
     */
    void setServerInfo(const std::string& name, const std::string& version);

    /**
     * @brief Get the server identity reported during MCP initialization.
     * @return Current server identity snapshot.
     */
    ServerInfo getServerInfo() const;

    /**
     * @brief Set the server capability declaration exposed to clients.
     * @param capabilities Capability payload returned from the initialize
     *                     handshake.
     */
    void setCapabilities(const ServerCapabilities& capabilities);

    /**
     * @brief Register a tool and its execution handler.
     * @param tool Tool metadata advertised to clients.
     * @param handler Callback invoked when the tool is called.
     * @return `true` if the tool was registered, `false` if a tool with the
     *         same name already exists.
     */
    bool registerTool(const Tool& tool, ToolCallHandler handler);

    /**
     * @brief Remove a previously registered tool.
     * @param name Tool name.
     * @return `true` if the tool existed and was removed, otherwise `false`.
     */
    bool unregisterTool(const std::string& name);

    /**
     * @brief Remove all registered tools and their handlers.
     */
    void clearTools();

    /**
     * @brief List the currently registered tools.
     * @return Snapshot of all tool metadata visible to clients.
     */
    std::vector<Tool> listTools() const;

    /**
     * @brief Register a resource and its read handler.
     * @param resource Resource metadata advertised to clients.
     * @param handler Callback used to read the resource content by URI.
     * @return `true` if the resource was registered, `false` if the URI is
     *         already registered.
     */
    bool registerResource(const Resource& resource,
                          ResourceReadHandler handler);

    /**
     * @brief Remove a previously registered resource.
     * @param uri Resource URI.
     * @return `true` if the resource existed and was removed, otherwise
     *         `false`.
     */
    bool unregisterResource(const std::string& uri);

    /**
     * @brief Remove all registered resources and their handlers.
     */
    void clearResources();

    /**
     * @brief Subscribe to updates for a registered resource URI.
     * @param uri Resource URI.
     * @throws mcp_exception with `kInvalidParams` if uri is empty or has not
     *         been registered.
     */
    void subscribeResource(const std::string& uri);

    /**
     * @brief Unsubscribe from updates for a registered resource URI.
     * @param uri Resource URI.
     * @throws mcp_exception with `kInvalidParams` if uri is empty or has not
     *         been registered.
     */
    void unsubscribeResource(const std::string& uri);

    /**
     * @brief List the currently registered resources.
     * @return Snapshot of all resource metadata visible to clients.
     */
    std::vector<Resource> listResources() const;

    /**
     * @brief Register a resource template exposed via resources/templates/list.
     * @param resource_template Template metadata advertised to clients.
     * @return `true` if the template was registered, `false` if uriTemplate is
     *         empty or duplicated.
     */
    bool registerResourceTemplate(const ResourceTemplate& resource_template);

    /**
     * @brief Remove a previously registered resource template.
     * @param uri_template Template URI pattern.
     * @return `true` if the template existed and was removed, otherwise
     * `false`.
     */
    bool unregisterResourceTemplate(const std::string& uri_template);

    /**
     * @brief Remove all registered resource templates.
     */
    void clearResourceTemplates();

    /**
     * @brief List resource templates exposed via `resources/templates/list`.
     * @return Vector of resource template descriptors.
     */
    std::vector<ResourceTemplate> listResourceTemplates() const;

    /**
     * @brief Register a prompt and its provider callback.
     * @param prompt Prompt metadata advertised to clients.
     * @param handler Callback invoked when a client requests the prompt.
     * @return `true` if the prompt was registered, `false` if a prompt with
     *         the same name already exists.
     */
    bool registerPrompt(const Prompt& prompt, PromptGetHandler handler);

    /**
     * @brief Remove a previously registered prompt.
     * @param name Prompt name.
     * @return `true` if the prompt existed and was removed, otherwise `false`.
     */
    bool unregisterPrompt(const std::string& name);

    /**
     * @brief Remove all registered prompts and their handlers.
     */
    void clearPrompts();

    /**
     * @brief List the currently registered prompts.
     * @return Snapshot of all prompt metadata visible to clients.
     */
    std::vector<Prompt> listPrompts() const;

    /**
     * @brief Notify connected clients that the resource list changed.
     * @return `true` if the notification was sent successfully, otherwise
     *         `false` (for example when the server is stopped or transport
     *         forwarding is unavailable).
     */
    bool notifyResourcesChanged();

    /**
     * @brief Notify connected clients that the tool list changed.
     * @return `true` if the notification was sent successfully, otherwise
     *         `false` (for example when the server is stopped or transport
     *         forwarding is unavailable).
     */
    bool notifyToolsChanged();

    /**
     * @brief Notify connected clients that the prompt list changed.
     * @return `true` if the notification was sent successfully, otherwise
     *         `false` (for example when the server is stopped or transport
     *         forwarding is unavailable).
     */
    bool notifyPromptsChanged();

    // ---- Roots (server -> client) ----

    /**
     * @brief Request the list of roots from the connected client.
     *
     * Sends a `roots/list` JSON-RPC request to the client via the
     * transport layer and waits synchronously for the response.
     *
     * @param timeout_ms Maximum time to wait for the client response,
     *                   in milliseconds.  Defaults to 10 000 ms.
     * @return The roots reported by the client, or an empty vector on
     *         failure / timeout.
     */
    std::vector<Root> requestRootsList(int timeout_ms = 10000);

    /**
     * @brief Register a callback invoked when the client notifies that
     *        its root list has changed.
     *
     * The handler receives the updated root list obtained by an
     * automatic `roots/list` re-query triggered by the notification.
     *
     * @param handler Callback receiving the new root list.
     */
    void setRootsListChangedHandler(RootsListChangedHandler handler);

    /**
     * @brief Send `elicitation/create` request to the connected client.
     * @param params Elicitation request payload.
     * @param timeout_ms Maximum time to wait for response in milliseconds.
     * @return Elicitation result from client.
     * @throws mcp_exception on timeout, transport failure, or invalid response.
     */
    ElicitCreateResult requestElicitation(const json& params,
                                          int timeout_ms = 10000);

    /**
     * @brief Send `sampling/createMessage` request to the connected client.
     * @param params Sampling request payload.
     * @param timeout_ms Maximum time to wait for response in milliseconds.
     * @return Sampling result from client.
     * @throws mcp_exception on timeout, transport failure, or invalid response.
     */
    CreateMessageResult requestCreateMessage(const json& params,
                                             int timeout_ms = 10000);

    // ---- Progress / Cancellation ----

    /**
     * @brief Send a progress notification to the connected client.
     *
     * @param params Progress payload including the progress token,
     *               current progress value, optional total, and message.
     * @return `true` if the notification was sent successfully, otherwise
     *         `false`.
     */
    bool sendProgress(const ProgressParams& params);

    /**
     * @brief Send `notifications/tasks/status` notification.
     * @param task Task status snapshot.
     * @return `true` if sent successfully, otherwise `false`.
     */
    bool notifyTaskStatus(const Task& task);

    /**
     * @brief Send `notifications/elicitation/complete` notification.
     * @param elicitation_id Elicitation request identifier.
     * @param result Elicitation result payload.
     * @return `true` if sent successfully, otherwise `false`.
     */
    bool notifyElicitationComplete(const std::string& elicitation_id,
                                   const ElicitCreateResult& result);

    /**
     * @brief Register a handler for client-initiated cancel requests.
     *
     * When a client sends `$/cancelRequest`, the registered handler is
     * called with the cancellation parameters.  The server-side business
     * logic is responsible for cooperatively aborting the in-flight
     * operation identified by the request ID.
     *
     * @param handler Callback invoked on cancellation.
     */
    void setCancelHandler(CancelHandler handler);

    /**
     * @brief Register server-side handler for `elicitation/create`.
     * @param handler Callback receiving raw request params and returning raw
     * result.
     */
    void setElicitationHandler(const std::function<json(const json&)>& handler);

    /**
     * @brief Register server-side handler for `completion/complete`.
     * @param handler Callback receiving raw request params and returning raw
     * result.
     */
    void setCompletionHandler(const std::function<json(const json&)>& handler);

    /**
     * @brief Register server-side handler for `sampling/createMessage`.
     * @param handler Callback receiving raw request params and returning raw
     * result.
     */
    void setSamplingHandler(const std::function<json(const json&)>& handler);

    /**
     * @brief Upsert a task record that can be queried via `tasks` requests.
     * @param task Task descriptor.
     * @param result Optional task result payload for `tasks/result`.
     * @return `true` on success, otherwise `false` when taskId is empty.
     */
    bool upsertTask(const Task& task, const json& result = json());

    /**
     * @brief Remove a task record.
     * @param task_id Task identifier.
     * @return `true` if removed, otherwise `false`.
     */
    bool removeTask(const std::string& task_id);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mcp

#endif  // KYSDK_MCP_SERVER_H
