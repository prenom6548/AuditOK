#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"
#include "common/transport.h"

using namespace mcp_transport;

namespace {

enum class ClientMode { IPC, SSE };

struct ClientOptions {
    ClientMode mode = ClientMode::SSE;
    std::string host = "127.0.0.1";
    int port = 18080;
    std::string endpoint = "/mcp";
    std::string server_command = "./bin/transport_server_example";
};

ClientOptions parseOptions(int argc, char* argv[]) {
    ClientOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode=sse" || arg == "--sse") {
            opts.mode = ClientMode::SSE;
        } else if (arg == "--mode=ipc" || arg == "--ipc") {
            opts.mode = ClientMode::IPC;
        } else if (arg.rfind("--host=", 0) == 0) {
            opts.host = arg.substr(std::string("--host=").length());
        } else if (arg.rfind("--port=", 0) == 0) {
            try {
                opts.port =
                    std::stoi(arg.substr(std::string("--port=").length()));
            } catch (const std::exception&) {
                mcp_logger::getModuleLogger("transport_server_client")
                    ->warning("Invalid port value: " + arg);
            }
        } else if (arg.rfind("--endpoint=", 0) == 0) {
            opts.endpoint = arg.substr(std::string("--endpoint=").length());
            if (opts.endpoint.empty() || opts.endpoint.front() != '/') {
                opts.endpoint = "/" + opts.endpoint;
            }
        } else if (arg.rfind("--server-command=", 0) == 0) {
            opts.server_command =
                arg.substr(std::string("--server-command=").length());
        }
    }

    return opts;
}

std::map<std::string, std::string> defaultJsonHeaders() {
    return {{"Content-Type", "application/json"},
            {"Accept", "application/json"}};
}

struct ClientState {
    std::string session_id;
};

class ClientEventHandler {
public:
    void onMessage(const TransportMessage& message) {
        auto logger = mcp_logger::getModuleLogger("transport_server_client");
        logger->info("🔔 Notification received (status " +
                     std::to_string(message.status_code) +
                     "): " + message.content);
    }

    void onConnection(const ConnectionInfo& info) {
        auto logger = mcp_logger::getModuleLogger("transport_server_client");
        logger->info("🔌 Connection established");
        logger->info("  - Connection ID: " + info.connection_id);
        logger->info("  - Remote: " + info.remote_address + ":" +
                     std::to_string(info.remote_port));
        logger->info("  - Local: " + info.local_address + ":" +
                     std::to_string(info.local_port));
        if (!info.session_id.empty()) {
            logger->info("  - Session ID (from connection): " +
                         info.session_id);
        }
    }

    void onError(const std::string& error) {
        auto logger = mcp_logger::getModuleLogger("transport_server_client");
        logger->error("❌ Transport error: " + error);
    }
};

std::string toLower(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string extractSessionId(
    const std::map<std::string, std::string>& headers) {
    for (const auto& [key, value] : headers) {
        if (toLower(key) == "x-session-id") {
            return value;
        }
    }
    return {};
}

TransportMessage sendRequestAndLog(
    Transport& transport, ClientState& state, const nlohmann::json& request,
    const std::string& label, const std::string& path,
    const std::map<std::string, std::string>& headers) {
    auto logger = mcp_logger::getModuleLogger("transport_server_client");
    nlohmann::json request_copy = request;
    std::map<std::string, std::string> request_headers = headers;

    if (!state.session_id.empty()) {
        request_copy["id"] = state.session_id;
        request_headers["X-Session-Id"] = state.session_id;
        logger->info("➡️  Sending " + label + " request with session_id=" +
                     state.session_id + ": " + request_copy.dump());
    } else {
        logger->info("➡️  Sending " + label +
                     " request (no session yet): " + request_copy.dump());
    }

    auto response = transport.sendRequestSync(request_copy.dump(), "POST", path,
                                              request_headers);
    logger->info("⬅️  " + label + " response (" +
                 std::to_string(response.status_code) +
                 "): " + response.content);

    const auto new_session_id = extractSessionId(response.headers);
    if (!new_session_id.empty() && new_session_id != state.session_id) {
        logger->info("📦 Updated session id from response header: " +
                     new_session_id);
        state.session_id = new_session_id;
    }

    return response;
}

void testPing(Transport& transport, ClientState& state, const std::string& path,
              const std::map<std::string, std::string>& headers) {
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["method"] = "ping";
    request["params"] = nlohmann::json::object();

    sendRequestAndLog(transport, state, request, "ping", path, headers);
}

void testEcho(Transport& transport, ClientState& state, const std::string& path,
              const std::map<std::string, std::string>& headers) {
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["method"] = "echo";
    request["params"] = "Hello from transport_server_client_example!";

    sendRequestAndLog(transport, state, request, "echo", path, headers);
}

void testAdd(Transport& transport, ClientState& state, const std::string& path,
             const std::map<std::string, std::string>& headers) {
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["method"] = "add";

    nlohmann::json params;
    params["a"] = 21;
    params["b"] = 21;
    request["params"] = params;

    sendRequestAndLog(transport, state, request, "add", path, headers);
}

void testGetTime(Transport& transport, ClientState& state,
                 const std::string& path,
                 const std::map<std::string, std::string>& headers) {
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["method"] = "get_time";
    request["params"] = nlohmann::json::object();

    sendRequestAndLog(transport, state, request, "get_time", path, headers);
}

void testGetInfo(Transport& transport, ClientState& state,
                 const std::string& path,
                 const std::map<std::string, std::string>& headers) {
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["method"] = "get_info";
    request["params"] = nlohmann::json::object();

    sendRequestAndLog(transport, state, request, "get_info", path, headers);
}

void sendLogNotification(Transport& transport, ClientState& state,
                         const std::string& path,
                         const std::map<std::string, std::string>& headers) {
    nlohmann::json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "log";

    nlohmann::json params;
    params["message"] =
        "Client-side log notification from transport_server_client_example";
    notification["params"] = params;

    auto logger = mcp_logger::getModuleLogger("transport_server_client");
    std::map<std::string, std::string> request_headers = headers;
    if (!state.session_id.empty()) {
        request_headers["X-Session-Id"] = state.session_id;
        logger->info("➡️  Sending log notification with session_id=" +
                     state.session_id);
    } else {
        logger->info(
            "➡️  Sending log notification without session (no session "
            "yet)");
    }

    auto response = transport.sendRequestSync(notification.dump(), "POST", path,
                                              request_headers);
    const auto new_session_id = extractSessionId(response.headers);
    if (!new_session_id.empty() && new_session_id != state.session_id) {
        logger->info("📦 Updated session id from notification response: " +
                     new_session_id);
        state.session_id = new_session_id;
    }

    if (response.status_code == 200) {
        logger->info("✅ Log notification delivered");
    } else {
        logger->error("❌ Failed to deliver log notification: " +
                      response.content);
    }
}

void sendHeartbeatNotification(
    Transport& transport, ClientState& state, const std::string& path,
    const std::map<std::string, std::string>& headers) {
    nlohmann::json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "heartbeat";

    nlohmann::json params;
    params["client_id"] = "transport_server_client_example";
    params["timestamp"] = std::time(nullptr);
    params["status"] = "active";
    notification["params"] = params;

    auto logger = mcp_logger::getModuleLogger("transport_server_client");
    std::map<std::string, std::string> request_headers = headers;
    if (!state.session_id.empty()) {
        request_headers["X-Session-Id"] = state.session_id;
        logger->info("➡️  Sending heartbeat notification with session_id=" +
                     state.session_id);
    } else {
        logger->info(
            "➡️  Sending heartbeat notification without session (no "
            "session "
            "yet)");
    }

    auto response = transport.sendRequestSync(notification.dump(), "POST", path,
                                              request_headers);
    const auto new_session_id = extractSessionId(response.headers);
    if (!new_session_id.empty() && new_session_id != state.session_id) {
        logger->info("📦 Updated session id from heartbeat response: " +
                     new_session_id);
        state.session_id = new_session_id;
    }

    if (response.status_code == 200) {
        logger->info("✅ Client heartbeat notification delivered");
    } else {
        logger->error("❌ Failed to deliver client heartbeat notification: " +
                      response.content);
    }
}

void testBatchRequests(Transport& transport, ClientState& state,
                       const std::string& path,
                       const std::map<std::string, std::string>& headers) {
    auto logger = mcp_logger::getModuleLogger("transport_server_client");
    logger->info("📦 Sending batch ping requests...");

    std::vector<std::future<TransportMessage>> futures;
    for (int i = 0; i < 3; ++i) {
        nlohmann::json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "ping";
        request["params"] = nlohmann::json::object();

        std::map<std::string, std::string> request_headers = headers;
        if (!state.session_id.empty()) {
            request["id"] = state.session_id;
            request_headers["X-Session-Id"] = state.session_id;
            logger->info("➡️  Sending batch ping with session_id=" +
                         state.session_id + ": " + request.dump());
        } else {
            logger->info(
                "➡️  Sending batch ping without session (no session "
                "yet): " +
                request.dump());
        }

        futures.push_back(transport.sendRequest(request.dump(), "POST", path,
                                                request_headers));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        auto response = futures[i].get();
        logger->info("📨 Batch response " + std::to_string(i) + " (" +
                     std::to_string(response.status_code) +
                     "): " + response.content);
        const auto new_session_id = extractSessionId(response.headers);
        if (!new_session_id.empty() && new_session_id != state.session_id) {
            logger->info("📦 Updated session id from batch response: " +
                         new_session_id);
            state.session_id = new_session_id;
        }
    }
}

void listenForNotifications(Transport& transport,
                            std::chrono::seconds duration) {
    auto logger = mcp_logger::getModuleLogger("transport_server_client");
    logger->info("👂 Listening for server notifications for " +
                 std::to_string(duration.count()) + " seconds...");

    const auto deadline = std::chrono::steady_clock::now() + duration;
    while (transport.isRunning() &&
           std::chrono::steady_clock::now() < deadline) {
        if (transport.hasMessage()) {
            auto message = transport.getMessage();
            logger->info("📬 Message from server: " + message.content);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    // 初始化日志系统
    mcp_logger::LogConfig log_config;
    log_config.level = mcp_logger::LogLevel::DEBUG;
    log_config.target = mcp_logger::LogTarget::BOTH;
    log_config.file_path = "logs/transport_server_client_example.log";
    log_config.enable_timestamp = true;
    log_config.format = "[{timestamp}] [{level}] {message}";
    log_config.auto_flush = true;

    mcp_logger::Logger::getInstance().initialize(log_config);

    mcp_logger::getModuleLogger("transport_server_client")
        ->info("🚀 Starting Transport Server Client Example");

    const auto options = parseOptions(argc, argv);
    const std::string request_path =
        options.mode == ClientMode::SSE ? options.endpoint : "/";
    const auto json_headers = defaultJsonHeaders();

    // 创建传输实例
    Transport transport;

    // 注册事件回调
    ClientState client_state;
    ClientEventHandler handler;
    transport.setMessageCallback(
        [&handler](const TransportMessage& msg) { handler.onMessage(msg); });
    transport.setConnectionCallback(
        [&handler, &client_state](const ConnectionInfo& info) {
            handler.onConnection(info);
            if (!info.session_id.empty() &&
                info.session_id != client_state.session_id) {
                auto logger =
                    mcp_logger::getModuleLogger("transport_server_client");
                logger->info("📦 Session id updated from connection info: " +
                             info.session_id);
                client_state.session_id = info.session_id;
            }
        });
    transport.setErrorCallback(
        [&handler](const std::string& error) { handler.onError(error); });

    // 根据模式配置客户端
    TransportConfig config;
    if (options.mode == ClientMode::SSE) {
        config = utils::createSseConfig(options.host, options.port,
                                        TransportProtocol::kHttp);
        config.sse.endpoint = options.endpoint;
        config.sse.keep_alive = true;
        config.auto_reconnect = true;
        config.reconnect_interval = 3;
        config.enable_logging = true;
        config.log_level = 1;

        mcp_logger::getModuleLogger("transport_server_client")
            ->info("🔧 Mode: SSE client");
        mcp_logger::getModuleLogger("transport_server_client")
            ->info("  Host: " + options.host);
        mcp_logger::getModuleLogger("transport_server_client")
            ->info("  Port: " + std::to_string(options.port));
        mcp_logger::getModuleLogger("transport_server_client")
            ->info("  Endpoint: " + options.endpoint);
        mcp_logger::getModuleLogger("transport_server_client")
            ->info(
                "📘 Ensure the server is running in SSE mode using the same "
                "endpoint");
    } else {
        config = utils::createIpcConfig(options.server_command);
        config.enable_logging = true;
        config.log_level = 1;
        config.ipc.buffer_size = 4096;
        config.ipc.read_timeout_ms = 100;
        config.ipc.max_retry_count = 10;

        mcp_logger::getModuleLogger("transport_server_client")
            ->info("🔧 Mode: IPC client");
        mcp_logger::getModuleLogger("transport_server_client")
            ->info("  Server command: " + options.server_command);
        mcp_logger::getModuleLogger("transport_server_client")
            ->info(
                "📘 The client will spawn the server command if it is not "
                "already running");
    }

    // 启动客户端
    if (!transport.start(config)) {
        mcp_logger::getModuleLogger("transport_server_client")
            ->error("❌ Failed to start Transport client");
        mcp_logger::Logger::getInstance().shutdown();
        return 1;
    }

    mcp_logger::getModuleLogger("transport_server_client")
        ->info("✅ Transport client started");

    // 等待连接稳定
    std::this_thread::sleep_for(
        std::chrono::seconds(options.mode == ClientMode::SSE ? 3 : 1));

    try {
        testPing(transport, client_state, request_path, json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testEcho(transport, client_state, request_path, json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testAdd(transport, client_state, request_path, json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testGetTime(transport, client_state, request_path, json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testGetInfo(transport, client_state, request_path, json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        sendLogNotification(transport, client_state, request_path,
                            json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        sendHeartbeatNotification(transport, client_state, request_path,
                                  json_headers);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        testBatchRequests(transport, client_state, request_path, json_headers);
    } catch (const std::exception& e) {
        mcp_logger::getModuleLogger("transport_server_client")
            ->error("❌ Request execution failed: " + std::string(e.what()));
    }

    listenForNotifications(
        transport,
        std::chrono::seconds(options.mode == ClientMode::SSE ? 45 : 20));

    // 停止客户端
    mcp_logger::getModuleLogger("transport_server_client")
        ->info("🛑 Stopping Transport client...");
    transport.stop();
    mcp_logger::getModuleLogger("transport_server_client")
        ->info("✅ Transport client stopped");

    // 关闭日志系统
    mcp_logger::Logger::getInstance().shutdown();
    return 0;
}
