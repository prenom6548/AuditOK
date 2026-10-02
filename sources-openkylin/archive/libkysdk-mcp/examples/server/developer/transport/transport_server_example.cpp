#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "logger.h"
#include "common/transport.h"

using namespace mcp_transport;

// 示例服务端请求处理器
class ExampleTransportServerRequestHandler
    : public TransportServerRequestHandler {
public:
    std::string handleRequest(const std::string& method,
                              const std::string& params,
                              const std::string& request_id) override {
        auto logger = mcp_logger::getModuleLogger("transport_server");
        logger->info("Handling request: " + method + " (ID: " + request_id +
                     ")");

        if (method == "ping") {
            const std::string response =
                "{\"message\": \"pong\", \"timestamp\": " +
                std::to_string(std::time(nullptr)) + "}";
            logger->info("Response data: " + response);
            return response;
        } else if (method == "echo") {
            const std::string response = "{\"echo\": " + params + "}";
            logger->info("Response data: " + response);
            return response;
        } else if (method == "add") {
            try {
                auto json_params = nlohmann::json::parse(params);
                int a = json_params["a"].get<int>();
                int b = json_params["b"].get<int>();
                int result = a + b;
                const std::string response =
                    "{\"result\": " + std::to_string(result) + "}";
                logger->info("Response data: " + response);
                return response;
            } catch (const std::exception& e) {
                const std::string response =
                    "{\"error\": \"Invalid parameters: " +
                    std::string(e.what()) + "\"}";
                logger->error("Response data: " + response);
                return response;
            }
        } else if (method == "get_time") {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::string time_str = std::ctime(&time_t);
            // Remove the trailing newline character from ctime()
            if (!time_str.empty() && time_str.back() == '\n') {
                time_str.pop_back();
            }
            const std::string response = "{\"time\": \"" + time_str + "\"}";
            logger->info("Response data: " + response);
            return response;
        } else if (method == "get_info") {
            nlohmann::json info;
            info["server_name"] = "MCP Transport Server";
            info["version"] = "1.0.0";
            info["capabilities"] = nlohmann::json::array(
                {"ping", "echo", "add", "get_time", "get_info"});
            info["uptime"] =
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
            const std::string response = info.dump();
            logger->info("Response data: " + response);
            return response;
        } else {
            const std::string response =
                "{\"error\": \"Unknown method: " + method + "\"}";
            logger->warning("Response data: " + response);
            return response;
        }
    }

    void handleNotification(const std::string& method,
                            const std::string& params) override {
        mcp_logger::getModuleLogger("transport_server")
            ->info("Handling notification: " + method);

        if (method == "log") {
            try {
                auto json_params = nlohmann::json::parse(params);
                std::string message = json_params["message"].get<std::string>();
                mcp_logger::getModuleLogger("transport_server")
                    ->info("Client log: " + message);
            } catch (const std::exception& e) {
                mcp_logger::getModuleLogger("transport_server")
                    ->error("Invalid log notification: " +
                            std::string(e.what()));
            }
        } else if (method == "heartbeat") {
            try {
                auto json_params = nlohmann::json::parse(params);
                std::string client_id =
                    json_params.value("client_id", "unknown");
                mcp_logger::getModuleLogger("transport_server")
                    ->info("Received heartbeat from client: " + client_id);
            } catch (const std::exception& e) {
                mcp_logger::getModuleLogger("transport_server")
                    ->error("Invalid heartbeat notification: " +
                            std::string(e.what()));
            }
        }
    }
};

namespace {

constexpr std::size_t kPayloadPreviewLength = 256;

std::string transportTypeToString(TransportType type) {
    switch (type) {
        case TransportType::kIpc:
            return "IPC";
        case TransportType::kSse:
            return "SSE";
        case TransportType::kAuto:
            return "AUTO";
        default:
            return "UNKNOWN";
    }
}

std::string transportStatusToString(TransportStatus status) {
    switch (status) {
        case TransportStatus::kDisconnected:
            return "DISCONNECTED";
        case TransportStatus::kConnecting:
            return "CONNECTING";
        case TransportStatus::kConnected:
            return "CONNECTED";
        case TransportStatus::kError:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string makePayloadPreview(const std::string& payload,
                               std::size_t max_length = kPayloadPreviewLength) {
    if (payload.empty()) {
        return "<empty>";
    }
    if (payload.size() <= max_length) {
        return payload;
    }
    return payload.substr(0, max_length) + "...(truncated)";
}

class ExampleSseConnectHandler : public SseServerConnectHandler {
public:
    bool handleRequest(const TransportHttpMessage& request,
                       TransportHttpMessage& response) override {
        auto logger = mcp_logger::getModuleLogger("transport_server");
        logger->info("🔌 Handling SSE connect request: method=" +
                     request.method + ", path=" + request.path);

        if (!request.body.empty()) {
            logger->debug("📦 Connect payload preview: " +
                          makePayloadPreview(request.body));
        }

        response = TransportHttpMessage{};
        response.id = request.id;
        response.status_code = 200;
        response.headers["Content-Type"] = "application/json";
        response.headers["Cache-Control"] = "no-cache";
        response.headers["Connection"] = "keep-alive";
        response.body = R"({"jsonrpc":"2.0","result":{"status":"ready"}})";

        logger->info("✅ SSE connection accepted for path: " + request.path);
        return true;
    }
};

enum class ExampleMode { IPC, SSE };

struct ExampleOptions {
    ExampleMode mode = ExampleMode::SSE;
    std::string host = "127.0.0.1";
    int port = 18080;
    std::string endpoint = "/mcp";
};

ExampleOptions parseOptions(int argc, char* argv[]) {
    ExampleOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode=sse" || arg == "--sse") {
            opts.mode = ExampleMode::SSE;
        } else if (arg == "--mode=ipc" || arg == "--ipc") {
            opts.mode = ExampleMode::IPC;
        } else if (arg.rfind("--host=", 0) == 0) {
            opts.host = arg.substr(std::string("--host=").length());
        } else if (arg.rfind("--port=", 0) == 0) {
            try {
                opts.port =
                    std::stoi(arg.substr(std::string("--port=").length()));
            } catch (const std::exception&) {
                mcp_logger::getModuleLogger("transport_server")
                    ->warning("Invalid port value: " + arg);
            }
        } else if (arg.rfind("--endpoint=", 0) == 0) {
            opts.endpoint = arg.substr(std::string("--endpoint=").length());
            if (opts.endpoint.empty() || opts.endpoint.front() != '/') {
                opts.endpoint = "/" + opts.endpoint;
            }
        }
    }

    return opts;
}

}  // namespace

struct ServerState {
    std::string session_id;
};

int main(int argc, char* argv[]) {
    // 初始化日志系统
    mcp_logger::LogConfig log_config;
    log_config.level = mcp_logger::LogLevel::DEBUG;
    log_config.target = mcp_logger::LogTarget::BOTH;
    log_config.file_path = "logs/transport_server_example.log";
    log_config.enable_timestamp = true;
    log_config.format = "[{timestamp}] [{level}] {message}";
    log_config.auto_flush = true;

    mcp_logger::Logger::getInstance().initialize(log_config);

    mcp_logger::getModuleLogger("transport_server")
        ->info("🚀 Starting Transport Server Example");

    const auto options = parseOptions(argc, argv);
    const bool sse_mode = options.mode == ExampleMode::SSE;

    if (sse_mode) {
        mcp_logger::getModuleLogger("transport_server")
            ->info("🔧 Mode: SSE server");
        mcp_logger::getModuleLogger("transport_server")
            ->info("  Host: " + options.host);
        mcp_logger::getModuleLogger("transport_server")
            ->info("  Port: " + std::to_string(options.port));
        mcp_logger::getModuleLogger("transport_server")
            ->info("  Endpoint: " + options.endpoint);
        mcp_logger::getModuleLogger("transport_server")
            ->info("📘 Try: curl -X POST http://" + options.host + ":" +
                   std::to_string(options.port) + options.endpoint +
                   " -d '{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"ping\"}' "
                   "-H 'Content-Type: application/json'");
    } else {
        mcp_logger::getModuleLogger("transport_server")
            ->info("🔧 Mode: IPC server");
    }

    // 创建传输实例
    Transport transport;

    ServerState server_state;
    transport.setMessageCallback([&server_state](const TransportMessage& msg) {
        auto logger = mcp_logger::getModuleLogger("transport_server");
        logger->info("📨 收到消息: id=" + msg.id + ", method=" +
                     (msg.method.empty() ? "<none>" : msg.method) +
                     ", path=" + (msg.path.empty() ? "<none>" : msg.path) +
                     ", status=" + std::to_string(msg.status_code) +
                     ", content=" + msg.content);
    });

    transport.setConnectionCallback(
        [&server_state](const ConnectionInfo& info) {
            auto logger = mcp_logger::getModuleLogger("transport_server");
            logger->info("🔗 Connection callback triggered");
            logger->info("  - Connection ID: " + info.connection_id);
            logger->info("  - Remote: " + info.remote_address + ":" +
                         std::to_string(info.remote_port));
            logger->info("  - Local: " + info.local_address + ":" +
                         std::to_string(info.local_port));
            logger->info("  - Status: " + transportStatusToString(info.status));
            logger->info("  - Type: " + transportTypeToString(info.type));
            logger->info("  - Session ID: " + info.session_id);
            server_state.session_id = info.session_id;
        });

    transport.setErrorCallback([](const std::string& error) {
        mcp_logger::getModuleLogger("transport_server")
            ->error("❗ Transport error callback: " + error);
    });

    // 配置为服务端模式
    TransportConfig config;
    if (sse_mode) {
        config = utils::createSseServerConfig(options.host, options.port,
                                              TransportProtocol::kHttp);
        config.sse.endpoint = options.endpoint;
        config.sse.keep_alive = true;
        config.auto_reconnect = false;
    } else {
        config = utils::createIpcServerConfig("mcp_transport_server", "1.0.0");
    }
    config.enable_logging = true;
    config.log_level = 1;  // INFO level

    // 设置服务端请求处理器
    auto handler = std::make_shared<ExampleTransportServerRequestHandler>();
    transport.setServerRequestHandler(handler);

    if (sse_mode) {
        mcp_logger::getModuleLogger("transport_server")
            ->info("🔌 Installing SSE connect handler");
        transport.setConnectionRequestHandler(
            std::make_shared<ExampleSseConnectHandler>());
    }

    // 启动服务端
    if (!transport.start(config)) {
        mcp_logger::getModuleLogger("transport_server")
            ->error("❌ Failed to start Transport server");
        return 1;
    }

    mcp_logger::getModuleLogger("transport_server")
        ->info("✅ Transport Server started successfully");
    mcp_logger::getModuleLogger("transport_server")
        ->info("📡 Server is listening for requests...");

    std::optional<std::thread> heartbeat_thread;
    if (!sse_mode) {
        heartbeat_thread.emplace([&transport]() {
            int counter = 0;
            while (transport.isRunning()) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                if (transport.isRunning()) {
                    nlohmann::json heartbeat;
                    heartbeat["message"] =
                        "Server heartbeat #" + std::to_string(++counter);
                    heartbeat["timestamp"] = std::time(nullptr);
                    heartbeat["server_info"] = "MCP Transport Server v1.0.0";

                    transport.sendNotification("heartbeat", heartbeat.dump());
                    mcp_logger::getModuleLogger("transport_server")
                        ->info("💓 Sent heartbeat #" + std::to_string(counter));
                }
            }
        });
    }

    // 主循环 - 保持服务运行
    try {
        while (transport.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // 检查是否有消息需要处理
            if (transport.hasMessage()) {
                auto message = transport.getMessage();
                mcp_logger::getModuleLogger("transport_server")
                    ->debug("Received message: " + message.content);
            }
        }
    } catch (const std::exception& e) {
        mcp_logger::getModuleLogger("transport_server")
            ->error("❌ Server error: " + std::string(e.what()));
    }

    // 等待心跳线程结束
    if (heartbeat_thread && heartbeat_thread->joinable()) {
        heartbeat_thread->join();
    }

    // 停止服务
    mcp_logger::getModuleLogger("transport_server")
        ->info("🛑 Stopping Transport server...");
    transport.stop();
    mcp_logger::getModuleLogger("transport_server")
        ->info("✅ Transport Server stopped");

    // 关闭日志系统
    mcp_logger::Logger::getInstance().shutdown();
    return 0;
}
