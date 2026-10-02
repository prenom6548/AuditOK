#include "netio.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <thread>
#include <utility>

#include "../log/logger.h"

namespace netio {

// Message implementation
nlohmann::json Message::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["type"] = static_cast<int>(type);
    j["method"] = method;
    j["path"] = path;
    j["body"] = body;
    j["headers"] = headers;
    j["status_code"] = status_code;
    j["error_message"] = error_message;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                         timestamp.time_since_epoch())
                         .count();
    return j;
}

Message Message::fromJson(const nlohmann::json& j) {
    Message msg;
    msg.id = j.value("id", "");
    msg.type = static_cast<MessageType>(j.value("type", 0));
    msg.method = j.value("method", "");
    msg.path = j.value("path", "");
    msg.body = j.value("body", "");
    msg.status_code = j.value("status_code", 200);
    msg.error_message = j.value("error_message", "");

    if (j.contains("headers") && j["headers"].is_object()) {
        for (auto& [key, value] : j["headers"].items()) {
            msg.headers[key] = value.get<std::string>();
        }
    }

    if (j.contains("timestamp")) {
        auto timestamp_ms = j["timestamp"].get<uint64_t>();
        msg.timestamp = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(timestamp_ms));
    } else {
        msg.timestamp = std::chrono::system_clock::now();
    }

    return msg;
}

// ConnectionInfo implementation
nlohmann::json ConnectionInfo::toJson() const {
    nlohmann::json j;
    j["connection_id"] = connection_id;
    j["remote_address"] = remote_address;
    j["remote_port"] = remote_port;
    j["local_address"] = local_address;
    j["local_port"] = local_port;
    j["status"] = static_cast<int>(status);
    j["connected_at"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                            connected_at.time_since_epoch())
                            .count();
    j["last_activity"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                             last_activity.time_since_epoch())
                             .count();
    j["session_id"] = session_id;
    return j;
}

// NetIO internal implementation
class NetIO::Impl {
public:
    Impl()
        : client_(nullptr),
          server_(nullptr),
          status_(ConnectionStatus::DISCONNECTED),
          server_running_(false),
          config_(),
          connection_id_(generate_connection_id()),
          statistics_() {}

    ~Impl() {
        try {
            disconnect();

            // Ensure server thread is cleaned up correctly
            if (server_thread_.joinable()) {
                if (server_) {
                    server_->stop();
                }
                server_thread_.join();
            }

            server_.reset();
            server_running_ = false;
        } catch (const std::exception& e) {
            // Destructor must not throw; only log errors
            mcp_logger::getModuleLogger("netio")->error(
                "Error in destructor: " + std::string(e.what()));
        }
    }

    bool connect(const NetConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        mcp_logger::getModuleLogger("netio")->info(
            "Connecting to " + config.host + ":" + std::to_string(config.port) +
            " using " +
            (config.protocol == Protocol::HTTPS ? "HTTPS" : "HTTP"));

        try {
            config_ = config;

            // Create client by protocol
            httplib::Result result;
            if (config.protocol == Protocol::HTTPS) {
                ssl_client_ = std::make_unique<httplib::SSLClient>(config.host,
                                                                   config.port);
                // Set timeout
                ssl_client_->set_connection_timeout(config.timeout_seconds);
                ssl_client_->set_read_timeout(config.timeout_seconds);
                ssl_client_->set_write_timeout(config.timeout_seconds);

                // Set default headers
                httplib::Headers default_headers;
                default_headers.insert({"User-Agent", config.user_agent});
                for (const auto& [key, value] : config.headers) {
                    default_headers.insert({key, value});
                }
                ssl_client_->set_default_headers(default_headers);

                mcp_logger::getModuleLogger("netio")->info(
                    "Sending GET request to / for connection test...");
                result = ssl_client_->Get("/mcp");
            } else {
                client_ =
                    std::make_unique<httplib::Client>(config.host, config.port);
                // Set timeout
                client_->set_connection_timeout(config.timeout_seconds);
                client_->set_read_timeout(config.timeout_seconds);
                client_->set_write_timeout(config.timeout_seconds);

                // Set default headers
                httplib::Headers default_headers;
                default_headers.insert({"User-Agent", config.user_agent});
                for (const auto& [key, value] : config.headers) {
                    default_headers.insert({key, value});
                }
                client_->set_default_headers(default_headers);

                mcp_logger::getModuleLogger("netio")->info(
                    "Sending GET request to /mcp for connection test...");
                result = client_->Get("/mcp");
            }

            if (result) {
                mcp_logger::getModuleLogger("netio")->info(
                    "Connection result: " + result->body);

                for (const auto& [key, value] : result->headers) {
                    mcp_logger::getModuleLogger("netio")->info(
                        "Connection header - " + key + ": " + value);
                }

                status_ = ConnectionStatus::CONNECTED;
                connection_info_.connection_id = generate_connection_id();
                connection_info_.remote_address = config.host;
                connection_info_.remote_port = config.port;
                connection_info_.status = status_;
                connection_info_.connected_at =
                    std::chrono::system_clock::now();
                connection_info_.last_activity = connection_info_.connected_at;
                if (result->has_header("X-Session-Id")) {
                    connection_info_.session_id =
                        result->get_header_value("X-Session-Id");
                } else {
                    mcp_logger::getModuleLogger("netio")->info(
                        "No session id found in connection headers");
                    connection_info_.session_id = "";
                }

                updateStatistics("connections", 1);
                mcp_logger::getModuleLogger("netio")->info(
                    "Successfully connected to " + config.host + ":" +
                    std::to_string(config.port) +
                    " with connection ID: " + connection_id_);

                if (connection_callback_) {
                    connection_callback_(connection_info_);
                } else {
                    mcp_logger::getModuleLogger("netio")->error(
                        "No connection callback found");
                }

                stop_client_sse_stream_ = false;
                startClientSseStreamThreadLocked();
                startKeepaliveThreadLocked();

                return true;
            } else {
                status_ = ConnectionStatus::ERROR;
                mcp_logger::getModuleLogger("netio")->error(
                    "Failed to connect to " + config.host + ":" +
                    std::to_string(config.port));
                if (error_callback_) {
                    error_callback_("Failed to connect to " + config.host +
                                    ":" + std::to_string(config.port));
                }
                return false;
            }
        } catch (const std::exception& e) {
            status_ = ConnectionStatus::ERROR;
            mcp_logger::getModuleLogger("netio")->error("Connection error: " +
                                                        std::string(e.what()));
            if (error_callback_) {
                error_callback_("Connection error: " + std::string(e.what()));
            }
            return false;
        }
    }

    bool disconnect() {
        std::string remote_address;
        int remote_port = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            remote_address = connection_info_.remote_address;
            remote_port = connection_info_.remote_port;
        }

        mcp_logger::getModuleLogger("netio")->info(
            "Disconnecting from " + remote_address + ":" +
            std::to_string(remote_port));

        stopKeepaliveThreadLocked();
        stopClientSseStreamThreadLocked();

        std::lock_guard<std::mutex> lock(mutex_);
        if (client_) {
            client_->stop();
            client_.reset();
        }

        if (ssl_client_) {
            ssl_client_->stop();
            ssl_client_.reset();
        }

        status_ = ConnectionStatus::DISCONNECTED;
        connection_info_.status = status_;

        mcp_logger::getModuleLogger("netio")->info("Successfully disconnected");

        return true;
    }

    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return status_ == ConnectionStatus::CONNECTED &&
               (client_ != nullptr || ssl_client_ != nullptr);
    }

    ConnectionInfo get_connection_info() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_info_;
    }

    std::future<Message> sendRequest(const Message& message) {
        return std::async(std::launch::async, [this, message]() {
            return sendRequest_impl(message);
        });
    }

    std::future<Message> sendRequest(
        const std::string& method, const std::string& path,
        const std::string& body,
        const std::map<std::string, std::string>& headers) {
        Message msg = utils::createRequest(method, path, body, headers);
        return sendRequest(msg);
    }

    Message sendRequest_sync(const Message& message) {
        return sendRequest_impl(message);
    }

    Message sendRequest_sync(
        const std::string& method, const std::string& path,
        const std::string& body,
        const std::map<std::string, std::string>& headers) {
        Message msg = utils::createRequest(method, path, body, headers);
        return sendRequest_sync(msg);
    }

    bool startServer(const NetConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Stop server first if it is already running
        if (server_running_ || server_thread_.joinable()) {
            mcp_logger::getModuleLogger("netio")->warning(
                "Server is already running, stopping it first");
            stopServerUnsafe();  // Do not lock here because lock is already
                                 // held
        }

        mcp_logger::getModuleLogger("netio")->info("Starting server on " +
                                                   config.host + ":" +
                                                   std::to_string(config.port));

        try {
            config_ = config;
            server_ = std::make_unique<httplib::Server>();

            // Set handler for specific endpoint
            std::string endpoint =
                config.endpoint.empty() ? "/" : config.endpoint;
            mcp_logger::getModuleLogger("netio")->info(
                "Setting up server endpoint: " + endpoint);

            // SSE event-stream endpoint (legacy path).
            std::string sse_endpoint =
                endpoint + (endpoint.back() == '/' ? "sse" : "/sse");
            mcp_logger::getModuleLogger("netio")->info(
                "Setting up SSE endpoint: " + sse_endpoint);

            auto accepts_event_stream = [](const httplib::Request& req) {
                auto it = req.headers.find("Accept");
                if (it == req.headers.end()) {
                    return false;
                }
                std::string accept = it->second;
                std::transform(
                    accept.begin(), accept.end(), accept.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                return accept.find("text/event-stream") != std::string::npos;
            };

            auto get_header_value_ci = [](const httplib::Request& req,
                                          const char* key_a,
                                          const char* key_b) -> std::string {
                auto it = req.headers.find(key_a);
                if (it != req.headers.end()) {
                    return it->second;
                }
                it = req.headers.find(key_b);
                if (it != req.headers.end()) {
                    return it->second;
                }
                return "";
            };

            auto is_origin_allowed = [config, get_header_value_ci](
                                         const httplib::Request& req) {
                const std::string origin =
                    get_header_value_ci(req, "Origin", "origin");
                if (origin.empty()) {
                    return true;
                }

                std::string expected_origin =
                    (config.protocol == Protocol::HTTPS ? "https://" : "http://") +
                    config.host + ":" + std::to_string(config.port);
                return origin == expected_origin;
            };

            auto sse_connect_handler = [this, endpoint](
                                           const httplib::Request& /*req*/,
                                           httplib::Response& res) {
                std::string client_id = generate_connection_id();
                std::string session_id = generate_connection_id();
                mcp_logger::getModuleLogger("netio")->info(
                    "SSE client connected: " + client_id);

                res.set_header("Cache-Control", "no-cache");
                res.set_header("Connection", "keep-alive");
                res.set_header("X-Accel-Buffering", "no");
                res.set_header("Mcp-Session-Id", session_id);
                res.set_header("mcp-session-id", session_id);
                res.set_header("X-Session-Id", session_id);
                mcp_logger::getModuleLogger("netio")->info(
                    "SSE connect response headers: Mcp-Session-Id=" +
                    session_id);
                {
                    std::lock_guard<std::mutex> lock(sse_session_mutex_);
                    last_sse_session_id_ = session_id;
                }

                // Register the SSE client immediately to avoid race where POST
                // /mcp arrives before the first chunk callback executes.
                {
                    std::lock_guard<std::mutex> lock(sse_mutex_);
                    SseClient client;
                    client.client_id = client_id;
                    client.session_id = session_id;
                    client.connected_at = std::chrono::system_clock::now();
                    sse_clients_[client_id] = std::move(client);
                }

                std::string initial_event =
                    "event: endpoint\ndata: " + endpoint + "\n\n";

                res.set_chunked_content_provider(
                    "text/event-stream",
                    [this, client_id, session_id, initial_event](
                        size_t offset, httplib::DataSink& sink) -> bool {
                        // First call: register client and send initial event
                        if (offset == 0) {
                            if (!sink.write(initial_event.c_str(),
                                            initial_event.size())) {
                                return false;
                            }
                        }

                        // Wait for events or shutdown signal
                        std::unique_lock<std::mutex> lock(sse_mutex_);
                        // Keep the poll interval short so queued server->client
                        // requests are delivered promptly instead of waiting
                        // for the next long keepalive window.
                        sse_cv_.wait_for(
                            lock, std::chrono::seconds(1), [this, &client_id] {
                                auto it = sse_clients_.find(client_id);
                                return sse_shutdown_.load() ||
                                       (it != sse_clients_.end() &&
                                        !it->second.event_queue.empty());
                            });

                        if (sse_shutdown_.load()) {
                            return false;
                        }

                        auto it = sse_clients_.find(client_id);
                        if (it == sse_clients_.end()) {
                            return false;
                        }

                        // Drain this client queue
                        while (!it->second.event_queue.empty()) {
                            std::string event =
                                std::move(it->second.event_queue.front());
                            it->second.event_queue.pop();
                            lock.unlock();
                            if (!sink.write(event.c_str(), event.size())) {
                                // Write failed, client disconnected
                                std::lock_guard<std::mutex> lk(sse_mutex_);
                                sse_clients_.erase(client_id);
                                return false;
                            }
                            lock.lock();
                            it = sse_clients_.find(client_id);
                            if (it == sse_clients_.end()) {
                                return false;
                            }
                        }

                        // Send keepalive comment when timed out with no data
                        if (it->second.event_queue.empty()) {
                            lock.unlock();
                            std::string keepalive = ": keepalive\n\n";
                            return sink.write(keepalive.c_str(),
                                              keepalive.size());
                        }

                        return true;
                    },
                    [this, client_id](bool /*success*/) {
                        std::lock_guard<std::mutex> lock(sse_mutex_);
                        sse_clients_.erase(client_id);
                        mcp_logger::getModuleLogger("netio")->info(
                            "SSE client disconnected: " + client_id);
                    });
            };

            // Main endpoint: normal GET or streamable HTTP SSE GET.
            server_->Get(endpoint, [this, accepts_event_stream, config,
                                    sse_connect_handler, is_origin_allowed](
                                       const httplib::Request& req,
                                       httplib::Response& res) {
                if (!is_origin_allowed(req)) {
                    res.status = 403;
                    res.set_content(
                        R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Forbidden origin"}})",
                        "application/json");
                    return;
                }
                if (config.streamable_http && accepts_event_stream(req)) {
                    mcp_logger::getModuleLogger("netio")->info(
                        "Server received streamable HTTP SSE GET: " + req.path);
                    sse_connect_handler(req, res);
                    return;
                }
                if (config.streamable_http) {
                    res.status = 405;
                    res.set_content(
                        R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Method not allowed"}})",
                        "application/json");
                    return;
                }
                mcp_logger::getModuleLogger("netio")->info(
                    "Server received GET request: " + req.path);
                handleGet(req, res);
            });

            server_->Post(endpoint, [this, is_origin_allowed](
                                         const httplib::Request& req,
                                         httplib::Response& res) {
                if (!is_origin_allowed(req)) {
                    res.status = 403;
                    res.set_content(
                        R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Forbidden origin"}})",
                        "application/json");
                    return;
                }
                mcp_logger::getModuleLogger("netio")->info(
                    "Server received POST request: " + req.path);
                handleRequest(req, res);
            });

            server_->Put(endpoint, [this](const httplib::Request& req,
                                          httplib::Response& res) {
                mcp_logger::getModuleLogger("netio")->info(
                    "Server received PUT request: " + req.path);
                handleRequest(req, res);
            });

            server_->Delete(endpoint, [this, config, is_origin_allowed,
                                       get_header_value_ci](
                                          const httplib::Request& req,
                                          httplib::Response& res) {
                if (!is_origin_allowed(req)) {
                    res.status = 403;
                    res.set_content(
                        R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Forbidden origin"}})",
                        "application/json");
                    return;
                }
                mcp_logger::getModuleLogger("netio")->info(
                    "Server received DELETE request: " + req.path);
                if (config.streamable_http) {
                    std::string session_id =
                        get_header_value_ci(req, "Mcp-Session-Id",
                                            "mcp-session-id");
                    if (session_id.empty()) {
                        session_id = get_header_value_ci(req, "X-Session-Id",
                                                         "x-session-id");
                    }
                    if (session_id.empty()) {
                        res.status = 400;
                        res.set_content(
                            R"({"jsonrpc":"2.0","error":{"code":-32600,"message":"Missing session id"}})",
                            "application/json");
                        return;
                    }

                    bool removed = false;
                    {
                        std::lock_guard<std::mutex> lock(sse_mutex_);
                        for (auto it = sse_clients_.begin();
                             it != sse_clients_.end();) {
                            if (it->second.session_id == session_id) {
                                it = sse_clients_.erase(it);
                                removed = true;
                            } else {
                                ++it;
                            }
                        }
                    }
                    {
                        std::lock_guard<std::mutex> lock(sse_session_mutex_);
                        if (last_sse_session_id_ == session_id) {
                            last_sse_session_id_.clear();
                        }
                    }

                    if (removed) {
                        res.status = 200;
                        res.set_content(R"({"ok":true})", "application/json");
                    } else {
                        res.status = 404;
                        res.set_content(R"({"ok":false})", "application/json");
                    }
                    return;
                }
                handleRequest(req, res);
            });

            // OAuth dynamic client registration is not implemented by this SDK.
            // Return a JSON OAuth error instead of httplib default HTML 404
            // page.
            auto oauth_register_handler = [](const httplib::Request& /*req*/,
                                             httplib::Response& res) {
                nlohmann::json error_body = {
                    {"error", "invalid_request"},
                    {"error_description",
                     "OAuth dynamic client registration is not supported by "
                     "this server"}};
                res.status = 400;
                res.set_content(error_body.dump(), "application/json");
            };
            server_->Post("/register", oauth_register_handler);
            server_->Post("/register/", oauth_register_handler);

            // Legacy SSE endpoint is kept for backward compatibility.
            server_->Get(sse_endpoint, sse_connect_handler);

            // Start server in a separate thread to avoid blocking
            server_thread_ = std::thread([this, config]() {
                try {
                    mcp_logger::getModuleLogger("netio")->info(
                        "Server thread started, listening on " + config.host +
                        ":" + std::to_string(config.port));
                    server_running_ = server_->listen(config.host, config.port);

                    if (server_running_) {
                        updateStatistics("server_starts", 1);
                        mcp_logger::getModuleLogger("netio")->info(
                            "Server successfully started on " + config.host +
                            ":" + std::to_string(config.port));
                    } else {
                        mcp_logger::getModuleLogger("netio")->info(
                            "Server stopped listening");
                    }
                } catch (const std::exception& e) {
                    mcp_logger::getModuleLogger("netio")->error(
                        "Server thread error: " + std::string(e.what()));
                    server_running_ = false;
                    if (error_callback_) {
                        error_callback_("Server thread error: " +
                                        std::string(e.what()));
                    }
                }
            });

            // Wait briefly to ensure server thread starts
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            return true;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("netio")->error("Server start error: " +
                                                        std::string(e.what()));

            // Cleanup resources
            if (server_thread_.joinable()) {
                server_thread_.join();
            }
            server_.reset();
            server_running_ = false;

            if (error_callback_) {
                error_callback_("Server start error: " + std::string(e.what()));
            }
            return false;
        }
    }

    bool stopServer() {
        std::lock_guard<std::mutex> lock(mutex_);
        return stopServerUnsafe();
    }

    bool stopServerUnsafe() {
        mcp_logger::getModuleLogger("netio")->info("Stopping server");

        // Close all SSE connections first
        {
            std::lock_guard<std::mutex> lock(sse_mutex_);
            sse_shutdown_ = true;
            sse_cv_.notify_all();
        }

        if (server_) {
            // Stop server
            server_->stop();

            // Wait for server thread to exit
            if (server_thread_.joinable()) {
                server_thread_.join();
            }

            server_.reset();
        }

        server_running_ = false;

        // Reset SSE state
        {
            std::lock_guard<std::mutex> lock(sse_mutex_);
            sse_shutdown_ = false;
            sse_clients_.clear();
        }

        mcp_logger::getModuleLogger("netio")->info("Server stopped");

        return true;
    }

    bool is_server_running() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return server_running_;
    }

    void set_message_callback(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        message_callback_ = callback;
    }

    void set_server_request_handler(
        std::shared_ptr<ServerRequestHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        server_request_handler_ = std::move(handler);
    }

    void set_connection_request_handler(
        std::shared_ptr<ConnectionRequestHandler> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_request_handler_ = std::move(handler);
    }

    void set_connection_callback(ConnectionCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_callback_ = callback;
    }

    void set_error_callback(ErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_callback_ = callback;
    }

    void set_connection_lost_callback(ConnectionLostCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_lost_callback_ = std::move(callback);
    }

    void updateConfig(const NetConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;
    }

    NetConfig getConfig() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_;
    }

    nlohmann::json getStatistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return statistics_;
    }

    bool healthCheck() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!client_ && !ssl_client_) {
            mcp_logger::getModuleLogger("netio")->warning(
                "Health check failed: client not connected");
            return false;
        }

        try {
            httplib::Result result;
            if (ssl_client_) {
                result = ssl_client_->Get("/health");
            } else {
                result = client_->Get("/health");
            }
            bool healthy = result != nullptr && result->status == 200;
            mcp_logger::getModuleLogger("netio")->debug(
                "Health check result: " +
                std::string(healthy ? "healthy" : "unhealthy"));
            return healthy;
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("netio")->error("Health check error: " +
                                                        std::string(e.what()));
            return false;
        }
    }

    bool sendSseEvent(const std::string& data) {
        std::string formatted = "event: message\ndata: " + data + "\n\n";

        std::lock_guard<std::mutex> lock(sse_mutex_);
        if (sse_clients_.empty()) {
            return false;
        }
        for (auto& [id, client] : sse_clients_) {
            client.event_queue.push(formatted);
        }
        sse_cv_.notify_all();
        return true;
    }

private:
    mutable std::mutex mutex_;
    std::unique_ptr<httplib::Client> client_;
    std::unique_ptr<httplib::SSLClient> ssl_client_;
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    ConnectionStatus status_;
    bool server_running_;
    NetConfig config_;
    std::string connection_id_;
    ConnectionInfo connection_info_;
    nlohmann::json statistics_;

    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    ErrorCallback error_callback_;
    ConnectionLostCallback connection_lost_callback_;
    std::shared_ptr<ServerRequestHandler> server_request_handler_;
    std::shared_ptr<ConnectionRequestHandler> connection_request_handler_;

    // SSE push logic
    struct SseClient {
        std::string client_id;
        std::string session_id;
        std::queue<std::string> event_queue;
        std::chrono::system_clock::time_point connected_at;
    };
    std::mutex sse_mutex_;
    std::map<std::string, SseClient> sse_clients_;
    std::condition_variable sse_cv_;
    std::atomic<bool> sse_shutdown_{false};
    std::mutex sse_session_mutex_;
    std::string last_sse_session_id_;
    std::thread client_sse_thread_;
    std::atomic<bool> stop_client_sse_stream_{false};
    std::unique_ptr<httplib::Client> sse_stream_client_;
    std::unique_ptr<httplib::SSLClient> sse_stream_ssl_client_;
    std::thread keepalive_thread_;
    std::atomic<bool> stop_keepalive_{false};
    std::condition_variable keepalive_cv_;
    std::mutex keepalive_mutex_;

    void notifyConnectionLost(const std::string& reason) {
        ConnectionLostCallback lost_callback;
        ErrorCallback error_callback;
        ConnectionCallback connection_callback;
        ConnectionInfo info;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (status_ == ConnectionStatus::DISCONNECTED ||
                status_ == ConnectionStatus::ERROR) {
                return;
            }
            status_ = ConnectionStatus::ERROR;
            connection_info_.status = status_;
            info = connection_info_;
            lost_callback = connection_lost_callback_;
            error_callback = error_callback_;
            connection_callback = connection_callback_;
        }

        if (error_callback) {
            error_callback(reason);
        }
        if (connection_callback) {
            connection_callback(info);
        }
        if (lost_callback) {
            lost_callback(reason);
        }
    }

    std::shared_ptr<ServerRequestHandler> get_server_request_handler() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return server_request_handler_;
    }

    std::shared_ptr<ConnectionRequestHandler> get_connection_request_handler()
        const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_request_handler_;
    }

    void dispatchClientSseEvent(const std::string& payload) {
        Message message;
        message.id = generate_connection_id();
        message.type = MessageType::NOTIFICATION;
        message.method = "SSE";
        message.path = config_.endpoint;
        message.body = payload;
        message.status_code = 200;
        message.timestamp = std::chrono::system_clock::now();
        if (message_callback_) {
            message_callback_(message);
        }
    }

    void processClientSseBuffer(std::string& buffer) {
        while (true) {
            size_t delimiter_pos = buffer.find("\n\n");
            size_t delimiter_len = 2;
            if (delimiter_pos == std::string::npos) {
                delimiter_pos = buffer.find("\r\n\r\n");
                delimiter_len = 4;
            }
            if (delimiter_pos == std::string::npos) {
                return;
            }

            std::string event_block = buffer.substr(0, delimiter_pos);
            buffer.erase(0, delimiter_pos + delimiter_len);
            if (event_block.empty()) {
                continue;
            }

            std::istringstream stream(event_block);
            std::string line;
            std::string event_type = "message";
            std::string data_payload;
            bool first_data_line = true;
            while (std::getline(stream, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                if (line.rfind("event:", 0) == 0) {
                    event_type = line.substr(6);
                    if (!event_type.empty() && event_type[0] == ' ') {
                        event_type.erase(0, 1);
                    }
                    continue;
                }

                if (line.rfind("data:", 0) == 0) {
                    std::string chunk = line.substr(5);
                    if (!chunk.empty() && chunk[0] == ' ') {
                        chunk.erase(0, 1);
                    }
                    if (!first_data_line) {
                        data_payload.push_back('\n');
                    }
                    first_data_line = false;
                    data_payload += chunk;
                }
            }

            if (event_type == "endpoint" || data_payload.empty()) {
                continue;
            }
            dispatchClientSseEvent(data_payload);
        }
    }

    void startClientSseStreamThreadLocked() {
        if (!client_ && !ssl_client_) {
            return;
        }

        stopClientSseStreamThreadLocked();
        stop_client_sse_stream_ = false;

        const std::string endpoint = config_.endpoint.empty() ? "/" : config_.endpoint;
        const std::string sse_path =
            config_.streamable_http
                ? endpoint
                : endpoint + (endpoint.back() == '/' ? "sse" : "/sse");

        httplib::Headers headers;
        for (const auto& [key, value] : config_.headers) {
            headers.insert({key, value});
        }
        headers.insert({"Accept", "text/event-stream"});

        client_sse_thread_ = std::thread([this, sse_path, headers]() {
            auto logger = mcp_logger::getModuleLogger("netio");
            logger->info("Starting SSE receive loop at path: " + sse_path);
            std::string stream_buffer;

            auto receiver = [this, &stream_buffer](const char* data,
                                                   size_t data_length) -> bool {
                if (stop_client_sse_stream_.load()) {
                    return false;
                }
                stream_buffer.append(data, data_length);
                processClientSseBuffer(stream_buffer);
                return !stop_client_sse_stream_.load();
            };

            httplib::Result result;
            if (config_.protocol == Protocol::HTTPS) {
                sse_stream_ssl_client_ = std::make_unique<httplib::SSLClient>(
                    config_.host, config_.port);
                sse_stream_ssl_client_->set_connection_timeout(
                    config_.timeout_seconds);
                sse_stream_ssl_client_->set_read_timeout(config_.timeout_seconds);
                sse_stream_ssl_client_->set_write_timeout(config_.timeout_seconds);
                result = sse_stream_ssl_client_->Get(sse_path, headers, receiver);
            } else {
                sse_stream_client_ =
                    std::make_unique<httplib::Client>(config_.host, config_.port);
                sse_stream_client_->set_connection_timeout(config_.timeout_seconds);
                sse_stream_client_->set_read_timeout(config_.timeout_seconds);
                sse_stream_client_->set_write_timeout(config_.timeout_seconds);
                result = sse_stream_client_->Get(sse_path, headers, receiver);
            }

            if (!stream_buffer.empty()) {
                processClientSseBuffer(stream_buffer);
            }

            if (!stop_client_sse_stream_.load()) {
                if (result) {
                    logger->warning(
                        "SSE receive loop exited unexpectedly with status: " +
                        std::to_string(result->status));
                } else {
                    logger->warning("SSE receive loop exited without response");
                }
                notifyConnectionLost("connection lost: sse stream closed");
            }
        });
    }

    void stopClientSseStreamThreadLocked() {
        stop_client_sse_stream_ = true;
        if (sse_stream_client_) {
            sse_stream_client_->stop();
        }
        if (sse_stream_ssl_client_) {
            sse_stream_ssl_client_->stop();
        }
        if (client_sse_thread_.joinable()) {
            client_sse_thread_.join();
        }
        sse_stream_client_.reset();
        sse_stream_ssl_client_.reset();
    }

    std::string generate_connection_id() {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);

        return "conn_" + std::to_string(timestamp) + "_" +
               std::to_string(dis(gen));
    }

    Message sendRequest_impl(const Message& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        mcp_logger::getModuleLogger("netio")->debug(
            "Sending request: " + message.method + " " + message.path +
            " (ID: " + message.id + ")");

        if (!client_ && !ssl_client_) {
            Message error_msg = utils::createError(500, "Client not connected");
            mcp_logger::getModuleLogger("netio")->error(
                "Client not connected for request: " + message.id);
            return error_msg;
        }

        try {
            httplib::Headers headers;
            for (const auto& [key, value] : message.headers) {
                headers.insert({key, value});
            }

            httplib::Result result;

            if (message.method == "GET") {
                if (ssl_client_) {
                    result = ssl_client_->Get(message.path, headers);
                } else {
                    result = client_->Get(message.path, headers);
                }
            } else if (message.method == "POST") {
                if (ssl_client_) {
                    result =
                        ssl_client_->Post(message.path, headers, message.body,
                                          "application/json");
                } else {
                    result = client_->Post(message.path, headers, message.body,
                                           "application/json");
                }
            } else if (message.method == "PUT") {
                if (ssl_client_) {
                    result = ssl_client_->Put(message.path, headers,
                                              message.body, "application/json");
                } else {
                    result = client_->Put(message.path, headers, message.body,
                                          "application/json");
                }
            } else if (message.method == "DELETE") {
                if (ssl_client_) {
                    result = ssl_client_->Delete(message.path, headers);
                } else {
                    result = client_->Delete(message.path, headers);
                }
            } else {
                Message error_msg = utils::createError(
                    405, "Method not allowed: " + message.method);
                return error_msg;
            }

            if (result) {
                Message response;
                response.id = message.id;
                response.type = MessageType::RESPONSE;
                response.method = message.method;
                response.path = message.path;
                response.body = result->body;
                response.status_code = result->status;
                response.timestamp = std::chrono::system_clock::now();

                mcp_logger::getModuleLogger("netio")->info(
                    "sendRequest_impl Received headers:");
                for (const auto& [key, value] : result->headers) {
                    response.headers[key] = value;
                    mcp_logger::getModuleLogger("netio")->info("  " + key +
                                                               ": " + value);
                }

                std::optional<std::string> content_type;
                if (auto it = result->headers.find("Content-Type");
                    it != result->headers.end()) {
                    content_type = it->second;
                    mcp_logger::getModuleLogger("netio")->debug(
                        "Response header Content-Type: " + *content_type);
                }

                connection_info_.last_activity = response.timestamp;
                updateStatistics("requests_sent", 1);
                updateStatistics("responses_received", 1);

                mcp_logger::getModuleLogger("netio")->debug(
                    "Received response: " + std::to_string(result->status) +
                    " for request " + message.id);

                if (message_callback_) {
                    mcp_logger::getModuleLogger("netio")->debug(
                        "Calling message callback for response: " + message.id);
                    message_callback_(response);
                }

                return response;
            } else {
                Message error_msg = utils::createError(500, "Request failed");
                mcp_logger::getModuleLogger("netio")->error(
                    "Request failed for " + message.method + " " +
                    message.path + " (ID: " + message.id + ")");
                return error_msg;
            }
        } catch (const std::exception& e) {
            Message error_msg = utils::createError(
                500, "Request error: " + std::string(e.what()));
            mcp_logger::getModuleLogger("netio")->error(
                "Request error for " + message.method + " " + message.path +
                " (ID: " + message.id + "): " + std::string(e.what()));
            return error_msg;
        }
    }

    void startKeepaliveThreadLocked() {
        stopKeepaliveThreadLocked();
        if (config_.keepalive_interval <= 0) {
            return;
        }

        stop_keepalive_.store(false, std::memory_order_release);
        keepalive_thread_ = std::thread([this]() {
            auto logger = mcp_logger::getModuleLogger("netio");
            int failures = 0;
            const auto interval =
                std::chrono::seconds(std::max(1, config_.keepalive_interval));

            while (!stop_keepalive_.load(std::memory_order_acquire)) {
                std::unique_lock<std::mutex> lock(keepalive_mutex_);
                if (keepalive_cv_.wait_for(lock, interval, [this]() {
                        return stop_keepalive_.load(std::memory_order_acquire);
                    })) {
                    break;
                }
                lock.unlock();

                bool ok = false;
                try {
                    if (config_.protocol == Protocol::HTTPS) {
                        httplib::SSLClient client(config_.host, config_.port);
                        client.set_connection_timeout(config_.timeout_seconds);
                        client.set_read_timeout(config_.timeout_seconds);
                        client.set_write_timeout(config_.timeout_seconds);
                        auto result = client.Get("/health");
                        ok = result && result->status >= 200 &&
                             result->status < 500;
                    } else {
                        httplib::Client client(config_.host, config_.port);
                        client.set_connection_timeout(config_.timeout_seconds);
                        client.set_read_timeout(config_.timeout_seconds);
                        client.set_write_timeout(config_.timeout_seconds);
                        auto result = client.Get("/health");
                        ok = result && result->status >= 200 &&
                             result->status < 500;
                    }
                } catch (const std::exception& e) {
                    logger->warning("SSE keepalive exception: " +
                                    std::string(e.what()));
                }

                failures = ok ? 0 : failures + 1;
                if (failures >= 3) {
                    logger->warning("SSE keepalive timeout");
                    notifyConnectionLost("keepalive timeout");
                    break;
                }
            }
        });
    }

    void stopKeepaliveThreadLocked() {
        stop_keepalive_.store(true, std::memory_order_release);
        keepalive_cv_.notify_all();
        if (keepalive_thread_.joinable()) {
            keepalive_thread_.join();
        }
    }

    void handleGet(const httplib::Request& req, httplib::Response& res) {
        try {
            Message request;
            request.id = generate_connection_id();
            request.type = MessageType::REQUEST;
            request.method = req.method;
            request.path = req.path;
            request.body = req.body;
            request.timestamp = std::chrono::system_clock::now();

            for (const auto& [key, value] : req.headers) {
                request.headers[key] = value;
            }

            auto has_non_empty_session_header =
                [](const std::map<std::string, std::string>& headers) {
                    auto has_value = [&headers](const char* key) {
                        auto it = headers.find(key);
                        return it != headers.end() && !it->second.empty();
                    };
                    return has_value("Mcp-Session-Id") ||
                           has_value("mcp-session-id") ||
                           has_value("X-Session-Id") ||
                           has_value("x-session-id");
                };
            if (!has_non_empty_session_header(request.headers)) {
                std::string fallback_session_id;
                {
                    std::lock_guard<std::mutex> lock(sse_mutex_);
                    if (!sse_clients_.empty()) {
                        const auto& first_client = sse_clients_.begin()->second;
                        fallback_session_id = first_client.session_id;
                    }
                }
                if (fallback_session_id.empty()) {
                    std::lock_guard<std::mutex> lock(sse_session_mutex_);
                    fallback_session_id = last_sse_session_id_;
                }
                if (!fallback_session_id.empty()) {
                    request.headers["Mcp-Session-Id"] = fallback_session_id;
                    request.headers["mcp-session-id"] = fallback_session_id;
                    request.headers["X-Session-Id"] = fallback_session_id;
                    request.headers["x-session-id"] = fallback_session_id;
                    mcp_logger::getModuleLogger("netio")->info(
                        "Injected fallback session header for request " +
                        request.id + ": " + fallback_session_id);
                }
            }

            updateStatistics("requests_received", 1);

            auto handler = get_connection_request_handler();
            if (!handler) {
                mcp_logger::getModuleLogger("netio")->error(
                    "No connection request handler found");
                res.status = 404;
                res.set_content("Not Found", "text/plain");
                return;
            }

            {
                Message response;
                response.type = MessageType::RESPONSE;
                response.id = request.id;
                response.timestamp = std::chrono::system_clock::now();

                bool need_response = false;
                try {
                    // Outer core logic: request forwarding
                    need_response = handler->handleRequest(request, response);
                } catch (const std::exception& e) {
                    nlohmann::json error_json;
                    error_json["jsonrpc"] = "2.0";
                    error_json["error"] = {
                        {"code", -32603},
                        {"message",
                         std::string("Internal error: ") + e.what()}};

                    response.status_code = 500;
                    response.headers["Content-Type"] = "application/json";
                    response.body = error_json.dump();
                    need_response = true;
                    mcp_logger::getModuleLogger("netio")->error(
                        "Server request handler exception: " +
                        std::string(e.what()));
                }

                int status_code = response.status_code != 0
                                      ? response.status_code
                                      : (need_response ? 200 : 204);
                res.status = status_code;

                std::string content_type = "application/json";
                auto content_type_it = response.headers.find("Content-Type");
                if (content_type_it != response.headers.end()) {
                    content_type = content_type_it->second;
                }

                // Set header
                for (const auto& [header_key, header_value] :
                     response.headers) {
                    if (!header_key.empty() && !header_value.empty()) {
                        res.set_header(header_key.c_str(),
                                       header_value.c_str());
                    }
                }

                // Set body
                if (!response.body.empty()) {
                    res.set_content(response.body, content_type.c_str());
                } else if (need_response) {
                    res.set_content("", content_type.c_str());
                }

                // Log header contents
                mcp_logger::getModuleLogger("netio")->debug(
                    "handleGet Response headers:");
                for (const auto& [header_key, header_value] :
                     response.headers) {
                    mcp_logger::getModuleLogger("netio")->debug(
                        "  " + header_key + ": " + header_value);
                }

                mcp_logger::getModuleLogger("netio")->debug(
                    "Server responding with status " +
                    std::to_string(res.status) + " for request: " + request.id);
                return;
            }
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("netio")->error(
                "Server request handling error: " + std::string(e.what()));
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }
    }
    void handleRequest(const httplib::Request& req, httplib::Response& res) {
        try {
            Message request;
            request.id = generate_connection_id();
            request.type = MessageType::REQUEST;
            request.method = req.method;
            request.path = req.path;
            request.body = req.body;
            request.timestamp = std::chrono::system_clock::now();

            for (const auto& [key, value] : req.headers) {
                request.headers[key] = value;
            }

            mcp_logger::getModuleLogger("netio")->info(
                "Server received request: " + req.method + " " + req.path +
                " (ID: " + request.id + ")");

            updateStatistics("requests_received", 1);

            auto handler = get_server_request_handler();
            if (handler) {
                Message response;
                response.type = MessageType::RESPONSE;
                response.id = request.id;
                response.timestamp = std::chrono::system_clock::now();

                bool need_response = false;
                try {
                    need_response = handler->handleRequest(request, response);
                } catch (const std::exception& e) {
                    nlohmann::json error_json;
                    error_json["jsonrpc"] = "2.0";
                    error_json["error"] = {
                        {"code", -32603},
                        {"message",
                         std::string("Internal error: ") + e.what()}};

                    response.status_code = 500;
                    response.headers["Content-Type"] = "application/json";
                    response.body = error_json.dump();
                    need_response = true;

                    mcp_logger::getModuleLogger("netio")->error(
                        "Server request handler exception: " +
                        std::string(e.what()));
                }

                int status_code = response.status_code != 0
                                      ? response.status_code
                                      : (need_response ? 200 : 204);
                res.status = status_code;

                std::string content_type = "application/json";
                auto content_type_it = response.headers.find("Content-Type");
                if (content_type_it != response.headers.end()) {
                    content_type = content_type_it->second;
                }

                for (const auto& [header_key, header_value] :
                     response.headers) {
                    if (!header_key.empty() && !header_value.empty()) {
                        res.set_header(header_key.c_str(),
                                       header_value.c_str());
                    }
                }

                if (!response.body.empty()) {
                    res.set_content(response.body, content_type.c_str());
                } else if (need_response) {
                    res.set_content("", content_type.c_str());
                }
                // Log header contents
                mcp_logger::getModuleLogger("netio")->debug(
                    "handleRequest Response headers:");
                for (const auto& [header_key, header_value] :
                     response.headers) {
                    mcp_logger::getModuleLogger("netio")->debug(
                        "  " + header_key + ": " + header_value);
                }
                mcp_logger::getModuleLogger("netio")->debug(
                    "Server responding with status " +
                    std::to_string(res.status) + " for request: " + request.id);
                return;
            }

            if (message_callback_) {
                mcp_logger::getModuleLogger("netio")->debug(
                    "Calling message callback for server request: " +
                    request.id);
                message_callback_(request);
            }

            // Default response
            res.status = 200;
            res.set_content("OK", "text/plain");

            mcp_logger::getModuleLogger("netio")->debug(
                "Server responding with status 200 for request: " + request.id);

        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("netio")->error(
                "Server request handling error: " + std::string(e.what()));
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }
    }

    void updateStatistics(const std::string& key, int increment) {
        if (statistics_.contains(key)) {
            statistics_[key] = statistics_[key].get<int>() + increment;
        } else {
            statistics_[key] = increment;
        }
        statistics_["last_updated"] =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
    }
};

// NetIO public interface implementation
NetIO::NetIO() : impl_(std::make_unique<Impl>()) {}

NetIO::~NetIO() = default;

bool NetIO::connect(const NetConfig& config) { return impl_->connect(config); }

bool NetIO::disconnect() { return impl_->disconnect(); }

bool NetIO::isConnected() const { return impl_->isConnected(); }

ConnectionInfo NetIO::getConnectionInfo() const {
    return impl_->get_connection_info();
}

std::future<Message> NetIO::sendRequest(const Message& message) {
    return impl_->sendRequest(message);
}

std::future<Message> NetIO::sendRequest(
    const std::string& method, const std::string& path, const std::string& body,
    const std::map<std::string, std::string>& headers) {
    return impl_->sendRequest(method, path, body, headers);
}

Message NetIO::sendRequestSync(const Message& message) {
    return impl_->sendRequest_sync(message);
}

Message NetIO::sendRequestSync(
    const std::string& method, const std::string& path, const std::string& body,
    const std::map<std::string, std::string>& headers) {
    return impl_->sendRequest_sync(method, path, body, headers);
}

bool NetIO::startServer(const NetConfig& config) {
    return impl_->startServer(config);
}

bool NetIO::stopServer() { return impl_->stopServer(); }

bool NetIO::isServerRunning() const { return impl_->is_server_running(); }

bool NetIO::sendSseEvent(const std::string& data) {
    return impl_->sendSseEvent(data);
}

void NetIO::setMessageCallback(MessageCallback callback) {
    impl_->set_message_callback(callback);
}

void NetIO::setServerRequestHandler(
    std::shared_ptr<ServerRequestHandler> handler) {
    impl_->set_server_request_handler(std::move(handler));
}

void NetIO::setConnectionRequestHandler(
    std::shared_ptr<ConnectionRequestHandler> handler) {
    impl_->set_connection_request_handler(std::move(handler));
}

void NetIO::setConnectionCallback(ConnectionCallback callback) {
    impl_->set_connection_callback(callback);
}

void NetIO::setErrorCallback(ErrorCallback callback) {
    impl_->set_error_callback(callback);
}

void NetIO::setConnectionLostCallback(ConnectionLostCallback callback) {
    impl_->set_connection_lost_callback(std::move(callback));
}

void NetIO::updateConfig(const NetConfig& config) {
    impl_->updateConfig(config);
}

NetConfig NetIO::getConfig() const { return impl_->getConfig(); }

nlohmann::json NetIO::getStatistics() const { return impl_->getStatistics(); }

bool NetIO::healthCheck() { return impl_->healthCheck(); }

// Utility function implementation
namespace utils {

std::string jsonToString(const nlohmann::json& j) { return j.dump(2); }

nlohmann::json stringToJson(const std::string& s) {
    try {
        return nlohmann::json::parse(s);
    } catch (const std::exception& e) {
        return nlohmann::json::object();
    }
}

Message createRequest(const std::string& method, const std::string& path,
                      const std::string& body,
                      const std::map<std::string, std::string>& headers) {
    Message msg;
    msg.id =
        "req_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.type = MessageType::REQUEST;
    msg.method = method;
    msg.path = path;
    msg.body = body;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

Message createResponse(int status_code, const std::string& body,
                       const std::map<std::string, std::string>& headers) {
    Message msg;
    msg.id =
        "resp_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.type = MessageType::RESPONSE;
    msg.status_code = status_code;
    msg.body = body;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

Message createError(int status_code, const std::string& error_message,
                    const std::map<std::string, std::string>& headers) {
    Message msg;
    msg.id =
        "err_" +
        std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count());
    msg.type = MessageType::ERROR;
    msg.status_code = status_code;
    msg.error_message = error_message;
    msg.headers = headers;
    msg.timestamp = std::chrono::system_clock::now();
    return msg;
}

NetConfig createHttpConfig(const std::string& host, int port) {
    NetConfig config;
    config.host = host;
    config.port = port;
    config.protocol = Protocol::HTTP;
    return config;
}

NetConfig createHttpsConfig(const std::string& host, int port) {
    NetConfig config;
    config.host = host;
    config.port = port;
    config.protocol = Protocol::HTTPS;
    return config;
}

bool validateConfig(const NetConfig& config) {
    return !config.host.empty() && config.port > 0 && config.port <= 65535 &&
           config.timeout_seconds > 0;
}

bool validateMessage(const Message& message) {
    return !message.id.empty() && !message.method.empty() &&
           !message.path.empty();
}

}  // namespace utils

// NetConfig implementation
nlohmann::json NetConfig::toJson() const {
    nlohmann::json j;
    j["host"] = host;
    j["port"] = port;
    j["protocol"] = static_cast<int>(protocol);
    j["timeout_seconds"] = timeout_seconds;
    j["max_retries"] = max_retries;
    j["headers"] = headers;
    j["user_agent"] = user_agent;
    j["keep_alive"] = keep_alive;
    j["auto_reconnect"] = auto_reconnect;
    j["reconnect_interval"] = reconnect_interval;
    j["keepalive_interval"] = keepalive_interval;
    j["endpoint"] = endpoint;
    j["streamable_http"] = streamable_http;
    return j;
}

}  // namespace netio
