#include "common/transport.h"
#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include "logger.h"

using namespace mcp_transport;

// 全局变量用于信号处理
static bool g_running = true;

// 信号处理函数
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        mcp_logger::getModuleLogger("transport_dual")->info("Received signal, shutting down...");
        g_running = false;
    }
}

// 双模式服务端请求处理器
class DualModeServerRequestHandler : public TransportServerRequestHandler {
public:
    std::string handleRequest(const std::string& method, const std::string& params, const std::string& request_id) override {
        mcp_logger::getModuleLogger("transport_dual")->info("Server handling request: " + method + " (ID: " + request_id + ")");
        
        if (method == "ping") {
            return "{\"message\": \"pong from dual mode server\", \"timestamp\": " + std::to_string(std::time(nullptr)) + "}";
        }
        else if (method == "echo") {
            return "{\"echo\": " + params + ", \"source\": \"dual_mode_server\"}";
        }
        else if (method == "get_mode") {
            return "{\"mode\": \"dual_mode\", \"server_name\": \"MCP Dual Mode Transport\"}";
        }
        else if (method == "shutdown") {
            g_running = false;
            return "{\"message\": \"Server shutdown initiated\"}";
        }
        else {
            return "{\"error\": \"Unknown method: " + method + "\"}";
        }
    }
    
    void handleNotification(const std::string& method, const std::string& params) override {
        mcp_logger::getModuleLogger("transport_dual")->info("Server handling notification: " + method);
        
        if (method == "log") {
            try {
                auto json_params = nlohmann::json::parse(params);
                std::string message = json_params["message"].get<std::string>();
                mcp_logger::getModuleLogger("transport_dual")->info("Client log: " + message);
            } catch (const std::exception& e) {
                mcp_logger::getModuleLogger("transport_dual")->error("Invalid log notification: " + std::string(e.what()));
            }
        }
    }
};

// 双模式客户端功能
class DualModeClient {
private:
    Transport* transport_;
    std::string client_id_;
    
public:
    DualModeClient(Transport* transport) : transport_(transport) {
        client_id_ = "dual_mode_client_" + std::to_string(std::time(nullptr));
    }
    
    void testServerConnection() {
        mcp_logger::getModuleLogger("transport_dual")->info("🔍 Testing server connection...");
        
        // 测试ping
        nlohmann::json request;
        request["jsonrpc"] = "2.0";
        request["id"] = "ping_test";
        request["method"] = "ping";
        request["params"] = nlohmann::json::object();
        
        auto response = transport_->sendRequestSync(request.dump());
        mcp_logger::getModuleLogger("transport_dual")->info("📨 Ping response: " + response.content);
        
        // 测试echo
        request["id"] = "echo_test";
        request["method"] = "echo";
        request["params"] = "Hello from dual mode client!";
        
        response = transport_->sendRequestSync(request.dump());
        mcp_logger::getModuleLogger("transport_dual")->info("📨 Echo response: " + response.content);
        
        // 测试get_mode
        request["id"] = "mode_test";
        request["method"] = "get_mode";
        request["params"] = nlohmann::json::object();
        
        response = transport_->sendRequestSync(request.dump());
        mcp_logger::getModuleLogger("transport_dual")->info("📨 Mode response: " + response.content);
    }
    
    void sendHeartbeat() {
        nlohmann::json notification;
        notification["jsonrpc"] = "2.0";
        notification["method"] = "log";
        
        nlohmann::json params;
        params["message"] = "Heartbeat from " + client_id_;
        params["timestamp"] = std::time(nullptr);
        notification["params"] = params;
        
        transport_->sendRequestSync(notification.dump());
    }
};

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 初始化日志系统
    mcp_logger::LogConfig log_config;
    log_config.level = mcp_logger::LogLevel::INFO;
    log_config.target = mcp_logger::LogTarget::BOTH;
    log_config.file_path = "logs/transport_dual_mode_example.log";
    log_config.enable_timestamp = true;
    log_config.format = "[{timestamp}] [{level}] {message}";
    log_config.auto_flush = true;
    
    mcp_logger::Logger::getInstance().initialize(log_config);
    
    mcp_logger::getModuleLogger("transport_dual")->info("🚀 Starting Transport Dual Mode Example");
    
    // 检查命令行参数
    bool server_mode = false;
    bool client_mode = false;
    std::string server_path;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server") {
            server_mode = true;
        } else if (arg == "--client") {
            client_mode = true;
            if (i + 1 < argc) {
                server_path = argv[i + 1];
                ++i;
            }
        }
    }
    
    // 如果没有指定模式，默认启动服务端模式
    if (!server_mode && !client_mode) {
        server_mode = true;
    }
    
    if (server_mode) {
        mcp_logger::getModuleLogger("transport_dual")->info("🖥️  Starting in SERVER mode");
        
        // 创建服务端传输实例
        Transport server_transport;
        
        // 配置为服务端模式
        TransportConfig server_config = utils::createIpcServerConfig("mcp_dual_mode_server", "1.0.0");
        server_config.enable_logging = true;
        server_config.log_level = 1;
        
        // 设置服务端请求处理器
        auto server_handler = std::make_shared<DualModeServerRequestHandler>();
        server_transport.setServerRequestHandler(server_handler);
        
        // 启动服务端
        if (!server_transport.start(server_config)) {
            mcp_logger::getModuleLogger("transport_dual")->error("❌ Failed to start Transport server");
            return 1;
        }
        
        mcp_logger::getModuleLogger("transport_dual")->info("✅ Transport Server started successfully");
        mcp_logger::getModuleLogger("transport_dual")->info("📡 Server is listening for requests...");
        mcp_logger::getModuleLogger("transport_dual")->info("💡 Use --client <server_path> to connect as client");
        
        // 主循环
        try {
            while (g_running && server_transport.isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                // 检查是否有消息需要处理
                if (server_transport.hasMessage()) {
                    auto message = server_transport.getMessage();
                    mcp_logger::getModuleLogger("transport_dual")->debug("Received message: " + message.content);
                }
            }
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport_dual")->error("❌ Server error: " + std::string(e.what()));
        }
        
        // 停止服务端
        mcp_logger::getModuleLogger("transport_dual")->info("🛑 Stopping Transport server...");
        server_transport.stop();
        mcp_logger::getModuleLogger("transport_dual")->info("✅ Transport Server stopped");
        
    } else if (client_mode) {
        mcp_logger::getModuleLogger("transport_dual")->info("📱 Starting in CLIENT mode");
        
        if (server_path.empty()) {
            server_path = "./bin/transport_dual_mode_example"; // 默认连接到自己
        }
        
        mcp_logger::getModuleLogger("transport_dual")->info("🔗 Connecting to server: " + server_path);
        
        // 创建客户端传输实例
        Transport client_transport;
        
        // 配置为客户端模式
        TransportConfig client_config = utils::createIpcConfig(server_path);
        client_config.enable_logging = true;
        client_config.log_level = 1;
        
        // 启动客户端
        if (!client_transport.start(client_config)) {
            mcp_logger::getModuleLogger("transport_dual")->error("❌ Failed to start Transport client");
            return 1;
        }
        
        mcp_logger::getModuleLogger("transport_dual")->info("✅ Transport Client started successfully");
        
        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 创建客户端功能实例
        DualModeClient client(&client_transport);
        
        // 测试服务器连接
        client.testServerConnection();
        
        // 定期发送心跳
        std::thread heartbeat_thread([&client, &client_transport]() {
            while (g_running && client_transport.isRunning()) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                if (g_running && client_transport.isRunning()) {
                    client.sendHeartbeat();
                }
            }
        });
        
        // 监听通知
        mcp_logger::getModuleLogger("transport_dual")->info("👂 Listening for notifications...");
        try {
            while (g_running && client_transport.isRunning()) {
                if (client_transport.hasMessage()) {
                    auto message = client_transport.getMessage();
                    mcp_logger::getModuleLogger("transport_dual")->info("📨 Received: " + message.content);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } catch (const std::exception& e) {
            mcp_logger::getModuleLogger("transport_dual")->error("❌ Client error: " + std::string(e.what()));
        }
        
        // 等待心跳线程结束
        if (heartbeat_thread.joinable()) {
            heartbeat_thread.join();
        }
        
        // 停止客户端
        mcp_logger::getModuleLogger("transport_dual")->info("🛑 Stopping Transport client...");
        client_transport.stop();
        mcp_logger::getModuleLogger("transport_dual")->info("✅ Transport Client stopped");
    }
    
    // 关闭日志系统
    mcp_logger::Logger::getInstance().shutdown();
    return 0;
}
