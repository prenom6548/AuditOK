#ifndef KYSDK_MCP_COMMON_TRANSPORT_H
#define KYSDK_MCP_COMMON_TRANSPORT_H

#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "json.hpp"

#if __has_include("logger.h")
#include "logger.h"
#endif

namespace mcp {
struct Message;
}

namespace netio {
struct Message;
}

namespace mcp_transport {

enum class TransportLogLevel {
    kDebug = 0,
    kInfo = 1,
    kWarning = 2,
    kError = 3
};

// Transport type enum
enum class TransportType {
    kIpc = 0,  // Inter-process communication (PipeCommunication)
    kSse = 1,  // SSE communication (NetIO)
    kAuto = 2  // Auto select
};

// Transport mode enum
enum class TransportMode {
    kClient = 0,  // Client mode: connect to a remote service
    kServer = 1   // Server mode: act as the service provider
};

// Transport protocol
enum class TransportProtocol { kHttp = 0, kHttps = 1, kWebSocket = 2 };

// Transport status enum
enum class TransportStatus {
    kDisconnected = 0,
    kConnecting = 1,
    kConnected = 2,
    kError = 3,
    kReconnecting = 4
};

// Global error handling
class TransportError : public std::exception {
public:
    explicit TransportError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

#if __has_include("logger.h")
#define TRANSPORT_LOG_DEBUG(msg)                                     \
    mcp_logger::Logger::getInstance().debug(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define TRANSPORT_LOG_INFO(msg)                                     \
    mcp_logger::Logger::getInstance().info(msg, __FILE__, __LINE__, \
                                           __FUNCTION__)
#define TRANSPORT_LOG_WARNING(msg)                                     \
    mcp_logger::Logger::getInstance().warning(msg, __FILE__, __LINE__, \
                                              __FUNCTION__)
#define TRANSPORT_LOG_ERROR(msg)                                     \
    mcp_logger::Logger::getInstance().error(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define TRANSPORT_LOG_FATAL(msg)                                     \
    mcp_logger::Logger::getInstance().fatal(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define TRANSPORT_MODULE_LOG_DEBUG(module, msg)                       \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->debug( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define TRANSPORT_MODULE_LOG_INFO(module, msg)                       \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->info( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define TRANSPORT_MODULE_LOG_WARNING(module, msg)                       \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->warning( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define TRANSPORT_MODULE_LOG_ERROR(module, msg)                       \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->error( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define TRANSPORT_MODULE_LOG_FATAL(module, msg)                       \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->fatal( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#else
#define TRANSPORT_LOG_DEBUG(msg) do {} while (0)
#define TRANSPORT_LOG_INFO(msg) do {} while (0)
#define TRANSPORT_LOG_WARNING(msg) do {} while (0)
#define TRANSPORT_LOG_ERROR(msg) do {} while (0)
#define TRANSPORT_LOG_FATAL(msg) do {} while (0)
#define TRANSPORT_MODULE_LOG_DEBUG(module, msg) do {} while (0)
#define TRANSPORT_MODULE_LOG_INFO(module, msg) do {} while (0)
#define TRANSPORT_MODULE_LOG_WARNING(module, msg) do {} while (0)
#define TRANSPORT_MODULE_LOG_ERROR(module, msg) do {} while (0)
#define TRANSPORT_MODULE_LOG_FATAL(module, msg) do {} while (0)
#endif

struct TransportHttpMessage {
    std::string id;
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    int status_code = 200;
    std::string error_message;
};

struct TransportMessage {
    std::string id;
    std::string content;
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    int status_code = 200;
    std::string error_message;
    std::chrono::system_clock::time_point timestamp;

    nlohmann::json toJson() const;
    static TransportMessage fromJson(const nlohmann::json& j);
    static TransportMessage fromIpcMessage(const mcp::Message& msg);
    static TransportMessage fromNetioMessage(const netio::Message& msg);
    mcp::Message toIpcMessage() const;
    netio::Message toNetioMessage() const;
};

/**
 * @brief Transport configuration structure
 *
 * Contains all configuration options for the transport layer,
 * including IPC and SSE specific settings.
 */
struct TransportConfig {
    TransportType type = TransportType::kAuto;  ///< Transport type
    TransportMode mode = TransportMode::kClient;  ///< Transport mode
    std::string name = "mcp_transport";  ///< Transport name
    bool auto_reconnect = false;  ///< Enable SDK-managed reconnect
    int reconnect_interval = 5;  ///< Base reconnect interval in seconds
    int max_reconnect_attempts = 5;  ///< Max automatic reconnect attempts
    int max_reconnect_interval = 60;  ///< Max reconnect backoff interval in seconds

    /// IPC-specific configuration
    struct {
        std::string command;                              ///< Command to execute
        std::vector<std::string> args;                   ///< Command arguments
        std::map<std::string, std::string> env_vars;    ///< Environment variables
        int buffer_size = 4096;                          ///< Buffer size
        int read_timeout_ms = 10;                        ///< Read timeout (ms)
        int max_retry_count = 10;                        ///< Max retry attempts
        bool respawn_on_disconnect = false;              ///< Respawn child only when auto_reconnect is true
        bool enable_logging = true;                      ///< Enable logging
        std::string log_file_path;                       ///< Log file path
        TransportLogLevel log_level = TransportLogLevel::kInfo;  ///< Log level
    } ipc;  ///< IPC transport settings

    /// Server-specific configuration
    struct {
        std::string server_name = "mcp_transport_server";       ///< Server name
        std::string server_version = "1.0.0";                   ///< Server version
        std::string description = "MCP Transport Server";       ///< Server description
    } server;  ///< Server transport settings

    /// SSE-specific configuration
    struct {
        std::string host = "localhost";                    ///< Host address
        int port = 8080;                                   ///< Port number
        TransportProtocol protocol = TransportProtocol::kHttp;  ///< Protocol
        bool streamable_http = false;                     ///< Enable streaming HTTP
        int timeout_seconds = 30;                          ///< Request timeout
        int max_retries = 3;                               ///< Max retries
        std::map<std::string, std::string> headers;       ///< Custom headers
        std::string user_agent = "MCP-Transport/1.0";     ///< User agent
        bool keep_alive = true;                            ///< Keep-alive enabled
        int keepalive_interval = 0;                        ///< Keepalive interval in seconds; 0 disables heartbeats
        std::string endpoint = "/mcp";                     ///< Endpoint path
    } sse;  ///< SSE transport settings

    bool enable_logging = true;   ///< Global logging enable flag
    std::string log_file_path;    ///< Global log file path
    int log_level = 1;            ///< Global log level

    /// Create config from JSON file
    static TransportConfig fromJsonFile(const std::string& file_path);
    /// Create config from JSON object
    static TransportConfig fromJson(const nlohmann::json& j);
    /// Convert config to JSON
    nlohmann::json toJson() const;
    /// Validate configuration
    bool validate() const;
};

/**
 * @brief Connection information structure
 *
 * Contains details about an active transport connection.
 */
struct ConnectionInfo {
    std::string connection_id;
    std::string remote_address;
    int remote_port = 0;
    std::string local_address;
    int local_port = 0;
    std::string session_id;
    std::chrono::system_clock::time_point connected_at;
    std::chrono::system_clock::time_point last_activity;
    TransportStatus status = TransportStatus::kDisconnected;
    TransportType type = TransportType::kAuto;

    nlohmann::json toJson() const;
    static ConnectionInfo fromJson(const nlohmann::json& j);
};

class TransportServerRequestHandler {
public:
    virtual ~TransportServerRequestHandler() = default;
    virtual std::string handleRequest(const std::string& method,
                                      const std::string& params,
                                      const std::string& request_id) = 0;
    virtual void handleNotification(const std::string& method,
                                    const std::string& params) = 0;
};

class SseServerConnectHandler {
public:
    virtual ~SseServerConnectHandler() = default;
    virtual bool handleRequest(const TransportHttpMessage& request,
                               TransportHttpMessage& response) = 0;
};

class SseServerRequestHandler {
public:
    virtual ~SseServerRequestHandler() = default;
    virtual bool handleRequest(const TransportHttpMessage& request,
                               TransportHttpMessage& response) = 0;
};

/// Callback for incoming messages
using MessageCallback = std::function<void(const TransportMessage& message)>;

/// Callback for connection status changes
using ConnectionCallback = std::function<void(const ConnectionInfo& info)>;

/// Callback for transport errors
using ErrorCallback = std::function<void(const std::string& error)>;

/**
 * @brief Abstract transport layer interface
 *
 * Provides a unified interface for different transport mechanisms
 * (IPC, SSE, etc.) used by the MCP protocol.
 */
class Transport {
public:
    Transport();
    virtual ~Transport();

    /**
     * @brief Start the transport with configuration
     * @param config Transport configuration
     * @return true if started successfully
     */
    bool start(const TransportConfig& config);

    /**
     * @brief Stop the transport
     * @return true if stopped successfully
     */
    bool stop();

    /**
     * @brief Check if transport is running
     * @return true if transport is active
     */
    bool isRunning() const;

    /**
     * @brief Get current transport status
     * @return Transport status enum
     */
    TransportStatus getStatus() const;

    /**
     * @brief Get connection information
     * @return Connection info structure
     */
    ConnectionInfo getConnectionInfo() const;

    /**
     * @brief Send request asynchronously
     * @param message Message to send
     * @return Future containing response
     */
    std::future<TransportMessage> sendRequest(const TransportMessage& message);

    /**
     * @brief Send request asynchronously with parameters
     * @param content Request content body
     * @param method HTTP method (default POST)
     * @param path Request path (default /)
     * @param headers Optional HTTP headers
     * @return Future containing response
     */
    std::future<TransportMessage> sendRequest(
        const std::string& content, const std::string& method = "POST",
        const std::string& path = "/",
        const std::map<std::string, std::string>& headers = {});

    /**
     * @brief Send a message without expecting response
     * @param message Message to send
     * @return true if sent successfully
     */
    bool sendMessage(const TransportMessage& message);

    /**
     * @brief Send request synchronously
     * @param message Message to send
     * @return Response message
     */
    TransportMessage sendRequestSync(const TransportMessage& message);

    /**
     * @brief Send request synchronously with parameters
     * @param content Request content body
     * @param method HTTP method
     * @param path Request path
     * @param headers Optional HTTP headers
     * @return Response message
     */
    TransportMessage sendRequestSync(
        const std::string& content, const std::string& method = "POST",
        const std::string& path = "/",
        const std::map<std::string, std::string>& headers = {});

    /**
     * @brief Check if there are pending messages
     * @return true if messages are available
     */
    bool hasMessage() const;

    /**
     * @brief Get next available message
     * @return Transport message
     */
    TransportMessage getMessage();

    /**
     * @brief Get all pending messages
     * @return Vector of transport messages
     */
    std::vector<TransportMessage> getAllMessages();

    /**
     * @brief Set message received callback
     * @param callback Callback function
     */
    void setMessageCallback(MessageCallback callback);

    /**
     * @brief Set connection status callback
     * @param callback Callback function
     */
    void setConnectionCallback(ConnectionCallback callback);

    /**
     * @brief Set error callback
     * @param callback Callback function
     */
    void setErrorCallback(ErrorCallback callback);

    /**
     * @brief Set server request handler for server mode
     * @param handler Server request handler
     */
    void setServerRequestHandler(
        std::shared_ptr<TransportServerRequestHandler> handler);

    /**
     * @brief Set SSE server request handler
     * @param handler SSE request handler
     */
    void setSseServerRequestHandler(
        std::shared_ptr<SseServerRequestHandler> handler);

    /**
     * @brief Set connection request handler
     * @param handler Connection handler
     */
    void setConnectionRequestHandler(
        std::shared_ptr<SseServerConnectHandler> handler);

    /**
     * @brief Send response to a request
     * @param request_id Request identifier
     * @param result Response result
     * @return true if sent successfully
     */
    bool sendResponse(const std::string& request_id, const std::string& result);

    /**
     * @brief Send error response
     * @param request_id Request identifier
     * @param error_code Error code
     * @param error_message Error message
     * @return true if sent successfully
     */
    bool sendError(const std::string& request_id, int error_code,
                   const std::string& error_message);

    /**
     * @brief Send a notification
     * @param method Notification method name
     * @param params Notification parameters
     * @return true if sent successfully
     */
    bool sendNotification(const std::string& method, const std::string& params);

    /**
     * @brief Update transport configuration
     * @param config New configuration
     */
    void updateConfig(const TransportConfig& config);

    /**
     * @brief Get current configuration
     * @return Current transport configuration
     */
    TransportConfig getConfig() const;

    /**
     * @brief Get transport statistics
     * @return JSON object with statistics
     */
    nlohmann::json getStatistics() const;

    /**
     * @brief Perform health check
     * @return true if healthy
     */
    bool healthCheck();

    /**
     * @brief Log a message at specified level
     * @param level Log level
     * @param message Message to log
     */
    void log(int level, const std::string& message);

    /**
     * @brief Set log level
     * @param level Log level
     */
    void setLogLevel(int level);

    /**
     * @brief Set log file path
     * @param file_path Path to log file
     */
    void setLogFile(const std::string& file_path);

    /**
     * @brief Enable or disable logging
     * @param enable true to enable logging
     */
    void enableLogging(bool enable);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/// Transport utility functions
namespace utils {

/**
 * @brief Create IPC transport configuration
 * @param command Command to execute
 * @param args Command arguments
 * @param env_vars Environment variables
 * @return Configured TransportConfig for IPC
 */
TransportConfig createIpcConfig(
    const std::string& command, const std::vector<std::string>& args = {},
    const std::map<std::string, std::string>& env_vars = {});

/**
 * @brief Create IPC server transport configuration
 * @param server_name Server name
 * @param server_version Server version
 * @return Configured TransportConfig for IPC server
 */
TransportConfig createIpcServerConfig(
    const std::string& server_name = "mcp_transport_server",
    const std::string& server_version = "1.0.0");

/**
 * @brief Create SSE transport configuration
 * @param host Server host
 * @param port Server port
 * @param protocol Transport protocol
 * @return Configured TransportConfig for SSE
 */
TransportConfig createSseConfig(
    const std::string& host, int port,
    TransportProtocol protocol = TransportProtocol::kHttp);

/**
 * @brief Create SSE server transport configuration
 * @param host Server host
 * @param port Server port
 * @param protocol Transport protocol (`kHttp` only for current server listen)
 * @return Configured TransportConfig for SSE server
 */
TransportConfig createSseServerConfig(
    const std::string& host, int port,
    TransportProtocol protocol = TransportProtocol::kHttp);

/**
 * @brief Create SSE secure transport configuration (client-side connection)
 * @param host Server host
 * @param port Server port
 * @return Configured TransportConfig for SSE over HTTPS
 */
TransportConfig createSseSecureConfig(const std::string& host, int port);

/**
 * @brief Create a transport request message
 * @param content Request body content
 * @param method HTTP method
 * @param path Request path
 * @param headers Additional HTTP headers
 * @return TransportMessage configured as request
 */
TransportMessage createRequest(
    const std::string& content, const std::string& method = "POST",
    const std::string& path = "/",
    const std::map<std::string, std::string>& headers = {});

/**
 * @brief Create a transport response message
 * @param content Response body content
 * @param status_code HTTP status code
 * @param headers Additional HTTP headers
 * @return TransportMessage configured as response
 */
TransportMessage createResponse(
    const std::string& content, int status_code = 200,
    const std::map<std::string, std::string>& headers = {});

/**
 * @brief Create a transport error message
 * @param error_message Error description
 * @param status_code HTTP status code (default 500)
 * @param headers Additional HTTP headers
 * @return TransportMessage configured as error response
 */
TransportMessage createError(
    const std::string& error_message, int status_code = 500,
    const std::map<std::string, std::string>& headers = {});

/**
 * @brief Validate transport configuration
 * @param config Configuration to validate
 * @return true if configuration is valid
 */
bool validateConfig(const TransportConfig& config);

/**
 * @brief Validate transport message
 * @param message Message to validate
 * @return true if message is valid
 */
bool validateMessage(const TransportMessage& message);

/**
 * @brief Convert JSON to string
 * @param j JSON object
 * @return String representation
 */
std::string jsonToString(const nlohmann::json& j);

/**
 * @brief Parse JSON from string
 * @param s String to parse
 * @return Parsed JSON object
 */
nlohmann::json stringToJson(const std::string& s);

}  // namespace utils

}  // namespace mcp_transport

#endif  // KYSDK_MCP_COMMON_TRANSPORT_H
