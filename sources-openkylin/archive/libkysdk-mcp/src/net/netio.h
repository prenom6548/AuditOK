#ifndef NETIO_H
#define NETIO_H

#ifdef MCP_SDK_INTERNAL_BUILD
#include "../common/httplib.h"
#include "../common/json.hpp"
#else
#include "httplib.h"
#include "json.hpp"
#endif
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace netio {

// Connection status enum
enum class ConnectionStatus {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    ERROR = 3
};

// Message type enum
enum class MessageType {
    REQUEST = 0,
    RESPONSE = 1,
    NOTIFICATION = 2,
    ERROR = 3
};

// Protocol type
enum class Protocol { HTTP = 0, HTTPS = 1, WEBSOCKET = 2 };

// Network config struct
struct NetConfig {
    std::string host = "localhost";
    int port = 8080;
    Protocol protocol = Protocol::HTTP;
    int timeout_seconds = 30;
    int max_retries = 3;
    std::map<std::string, std::string> headers;
    std::string user_agent = "NetIO/1.0";
    bool keep_alive = true;
    bool auto_reconnect = false;
    int reconnect_interval = 5;  // seconds
    int keepalive_interval = 0;  // seconds; 0 disables client heartbeats
    std::string endpoint = "/";  // HTTP endpoint path for server mode
    bool streamable_http = false;

    // Convert to JSON
    nlohmann::json toJson() const;
    nlohmann::json to_json() const { return toJson(); }
};

// Message struct
struct Message {
    std::string id;
    MessageType type;
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    int status_code = 200;
    std::string error_message;
    std::chrono::system_clock::time_point timestamp;

    // Convert to JSON
    nlohmann::json toJson() const;
    nlohmann::json to_json() const { return toJson(); }

    // Create from JSON
    static Message fromJson(const nlohmann::json& j);
    static Message from_json(const nlohmann::json& j) { return fromJson(j); }
};

// Server request handler interface
class ServerRequestHandler {
public:
    virtual ~ServerRequestHandler() = default;

    /**
     * @brief Handle requests received by the server
     *
     * @param request Request message received by server
     * @param response Response message to return to client
     * @return true Return true to send response to client
     * @return false Return false when no payload is needed (e.g. notification)
     */
    virtual bool handleRequest(const Message& request, Message& response) = 0;
};

// Connection request handler interface
class ConnectionRequestHandler {
public:
    virtual ~ConnectionRequestHandler() = default;
    virtual bool handleRequest(const Message& request, Message& response) = 0;
};
// Connection info struct
struct ConnectionInfo {
    std::string connection_id;
    std::string remote_address;
    int remote_port;
    std::string local_address;
    int local_port;
    ConnectionStatus status;
    std::chrono::system_clock::time_point connected_at;
    std::chrono::system_clock::time_point last_activity;
    std::string session_id;
    // Convert to JSON
    nlohmann::json toJson() const;
    nlohmann::json to_json() const { return toJson(); }
};

// Event callback types
using MessageCallback = std::function<void(const Message& message)>;
using ConnectionCallback = std::function<void(const ConnectionInfo& info)>;
using ErrorCallback = std::function<void(const std::string& error)>;
using ConnectionLostCallback = std::function<void(const std::string& reason)>;

// Network IO interface class
class NetIO {
public:
    NetIO();
    virtual ~NetIO();

    // Connection management
    bool connect(const NetConfig& config);
    bool disconnect();
    bool isConnected() const;
    bool is_connected() const { return isConnected(); }
    ConnectionInfo getConnectionInfo() const;
    ConnectionInfo get_connection_info() const { return getConnectionInfo(); }

    // Message sending
    std::future<Message> sendRequest(const Message& message);
    std::future<Message> send_request(const Message& message) {
        return sendRequest(message);
    }
    std::future<Message> sendRequest(
        const std::string& method, const std::string& path,
        const std::string& body = "",
        const std::map<std::string, std::string>& headers = {});
    std::future<Message> send_request(
        const std::string& method, const std::string& path,
        const std::string& body = "",
        const std::map<std::string, std::string>& headers = {}) {
        return sendRequest(method, path, body, headers);
    }

    // Synchronous send
    Message sendRequestSync(const Message& message);
    Message send_request_sync(const Message& message) {
        return sendRequestSync(message);
    }
    Message sendRequestSync(
        const std::string& method, const std::string& path,
        const std::string& body = "",
        const std::map<std::string, std::string>& headers = {});
    Message send_request_sync(
        const std::string& method, const std::string& path,
        const std::string& body = "",
        const std::map<std::string, std::string>& headers = {}) {
        return sendRequestSync(method, path, body, headers);
    }

    // Server features
    bool startServer(const NetConfig& config);
    bool start_server(const NetConfig& config) { return startServer(config); }
    bool stopServer();
    bool stop_server() { return stopServer(); }
    bool isServerRunning() const;
    bool is_server_running() const { return isServerRunning(); }

    // SSE server push
    bool sendSseEvent(const std::string& data);

    // Callback setup
    void setMessageCallback(MessageCallback callback);
    void set_message_callback(MessageCallback callback) {
        setMessageCallback(callback);
    }
    void setConnectionCallback(ConnectionCallback callback);
    void set_connection_callback(ConnectionCallback callback) {
        setConnectionCallback(callback);
    }
    void setErrorCallback(ErrorCallback callback);
    void set_error_callback(ErrorCallback callback) {
        setErrorCallback(callback);
    }
    void setConnectionLostCallback(ConnectionLostCallback callback);
    void setServerRequestHandler(std::shared_ptr<ServerRequestHandler> handler);
    void setConnectionRequestHandler(
        std::shared_ptr<ConnectionRequestHandler> handler);

    // Config management
    void updateConfig(const NetConfig& config);
    void update_config(const NetConfig& config) { updateConfig(config); }
    NetConfig getConfig() const;
    NetConfig get_config() const { return getConfig(); }

    // Statistics
    nlohmann::json getStatistics() const;
    nlohmann::json get_statistics() const { return getStatistics(); }

    // Health checks
    bool healthCheck();
    bool health_check() { return healthCheck(); }

private:
    // Internal implementation
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Utility functions
namespace utils {
// JSON utilities
std::string jsonToString(const nlohmann::json& j);
nlohmann::json stringToJson(const std::string& s);
inline std::string json_to_string(const nlohmann::json& j) {
    return jsonToString(j);
}
inline nlohmann::json string_to_json(const std::string& s) {
    return stringToJson(s);
}

// Message utilities
Message createRequest(const std::string& method, const std::string& path,
                      const std::string& body = "",
                      const std::map<std::string, std::string>& headers = {});
inline Message create_request(
    const std::string& method, const std::string& path,
    const std::string& body = "",
    const std::map<std::string, std::string>& headers = {}) {
    return createRequest(method, path, body, headers);
}

Message createResponse(int status_code, const std::string& body = "",
                       const std::map<std::string, std::string>& headers = {});
inline Message create_response(
    int status_code, const std::string& body = "",
    const std::map<std::string, std::string>& headers = {}) {
    return createResponse(status_code, body, headers);
}

Message createError(int status_code, const std::string& error_message,
                    const std::map<std::string, std::string>& headers = {});
inline Message create_error(
    int status_code, const std::string& error_message,
    const std::map<std::string, std::string>& headers = {}) {
    return createError(status_code, error_message, headers);
}

// Config utilities
NetConfig createHttpConfig(const std::string& host, int port);
NetConfig createHttpsConfig(const std::string& host, int port);
inline NetConfig create_http_config(const std::string& host, int port) {
    return createHttpConfig(host, port);
}
inline NetConfig create_https_config(const std::string& host, int port) {
    return createHttpsConfig(host, port);
}

// Validation functions
bool validateConfig(const NetConfig& config);
bool validateMessage(const Message& message);
inline bool validate_config(const NetConfig& config) {
    return validateConfig(config);
}
inline bool validate_message(const Message& message) {
    return validateMessage(message);
}
}  // namespace utils

// Global error handling
class NetIOError : public std::exception {
public:
    explicit NetIOError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

}  // namespace netio

#endif  // NETIO_H
