#include "common/transport.h"
#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "logger.h"

using namespace mcp_transport;

// 示例消息处理器
class ExampleMessageHandler {
public:
    void onMessage(const TransportMessage& message) {
        mcp_logger::getModuleLogger("transport_client")->info("Received notification: " + message.content);
    }
    
    void onConnection(const ConnectionInfo& info) {
        mcp_logger::getModuleLogger("transport_client")->info("Connection established: " + info.connection_id);
    }
    
    void onError(const std::string& error) {
        mcp_logger::getModuleLogger("transport_client")->error("Error: " + error);
    }
};

void testPingRequest(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing ping request...");
    
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["id"] = "ping_1";
    request["method"] = "ping";
    request["params"] = nlohmann::json::object();
    
    auto response = transport.sendRequestSync(request.dump());
    mcp_logger::getModuleLogger("transport_client")->info("📨 Ping response: " + response.content);
}

void testEchoRequest(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing echo request...");
    
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["id"] = "echo_1";
    request["method"] = "echo";
    request["params"] = "Hello from Transport client!";
    
    auto response = transport.sendRequestSync(request.dump());
    mcp_logger::getModuleLogger("transport_client")->info("📨 Echo response: " + response.content);
}

void testAddRequest(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing add request...");
    
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["id"] = "add_1";
    request["method"] = "add";
    
    nlohmann::json params;
    params["a"] = 42;
    params["b"] = 58;
    request["params"] = params;
    
    auto response = transport.sendRequestSync(request.dump());
    mcp_logger::getModuleLogger("transport_client")->info("📨 Add response: " + response.content);
}

void testGetTimeRequest(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing get_time request...");
    
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["id"] = "time_1";
    request["method"] = "get_time";
    request["params"] = nlohmann::json::object();
    
    auto response = transport.sendRequestSync(request.dump());
    mcp_logger::getModuleLogger("transport_client")->info("📨 Time response: " + response.content);
}

void testGetInfoRequest(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing get_info request...");
    
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["id"] = "info_1";
    request["method"] = "get_info";
    request["params"] = nlohmann::json::object();
    
    auto response = transport.sendRequestSync(request.dump());
    mcp_logger::getModuleLogger("transport_client")->info("📨 Info response: " + response.content);
}

void testNotification(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing notification...");
    
    nlohmann::json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "log";
    
    nlohmann::json params;
    params["message"] = "This is a test log message from Transport client";
    notification["params"] = params;
    
    auto response = transport.sendRequestSync(notification.dump());
    if (response.status_code == 200) {
        mcp_logger::getModuleLogger("transport_client")->info("✅ Notification sent successfully");
    } else {
        mcp_logger::getModuleLogger("transport_client")->error("❌ Failed to send notification");
    }
}

void testBatchRequests(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing batch requests...");
    
    std::vector<std::future<TransportMessage>> futures;
    
    // 发送多个并发请求
    for (int i = 0; i < 5; ++i) {
        nlohmann::json request;
        request["jsonrpc"] = "2.0";
        request["id"] = "batch_" + std::to_string(i);
        request["method"] = "ping";
        request["params"] = nlohmann::json::object();
        
        futures.push_back(transport.sendRequest(request.dump()));
    }
    
    // 等待所有响应
    for (size_t i = 0; i < futures.size(); ++i) {
        auto response = futures[i].get();
        mcp_logger::getModuleLogger("transport_client")->info("📨 Batch response " + std::to_string(i) + ": " + response.content);
    }
}

void testHeartbeatNotification(Transport& transport) {
    mcp_logger::getModuleLogger("transport_client")->info("🔍 Testing heartbeat notification...");
    
    nlohmann::json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "heartbeat";
    
    nlohmann::json params;
    params["client_id"] = "transport_client_example";
    params["timestamp"] = std::time(nullptr);
    params["status"] = "active";
    notification["params"] = params;
    
    auto response = transport.sendRequestSync(notification.dump());
    if (response.status_code == 200) {
        mcp_logger::getModuleLogger("transport_client")->info("✅ Heartbeat sent successfully");
    } else {
        mcp_logger::getModuleLogger("transport_client")->error("❌ Failed to send heartbeat");
    }
}

int main(int argc, char* argv[]) {
    // 初始化日志系统
    mcp_logger::LogConfig log_config;
    log_config.level = mcp_logger::LogLevel::INFO;
    log_config.target = mcp_logger::LogTarget::BOTH;
    log_config.file_path = "logs/transport_client_example.log";
    log_config.enable_timestamp = true;
    log_config.format = "[{timestamp}] [{level}] {message}";
    log_config.auto_flush = true;
    
    mcp_logger::Logger::getInstance().initialize(log_config);
    
    mcp_logger::getModuleLogger("transport_client")->info("🚀 Starting Transport Client Example");
    
    // 创建传输实例
    Transport transport;
    
    // 设置消息处理器
    ExampleMessageHandler handler;
    transport.setMessageCallback([&handler](const TransportMessage& msg) {
        handler.onMessage(msg);
    });
    transport.setConnectionCallback([&handler](const ConnectionInfo& info) {
        handler.onConnection(info);
    });
    transport.setErrorCallback([&handler](const std::string& error) {
        handler.onError(error);
    });
    
    // 配置为客户端模式
    TransportConfig config;
    if (argc > 1) {
        // 如果提供了服务器路径参数，使用IPC模式
        config = utils::createIpcConfig(argv[1]);
        mcp_logger::getModuleLogger("transport_client")->info("Using IPC mode with server: " + std::string(argv[1]));
    } else {
        // 默认使用IPC模式连接到transport_server_example
        config = utils::createIpcConfig("./bin/transport_server_example");
        mcp_logger::getModuleLogger("transport_client")->info("Using default IPC mode with bin/transport_server_example");
    }
    
    config.enable_logging = true;
    config.log_level = 1; // INFO level
    
    // 启动客户端
    if (!transport.start(config)) {
        mcp_logger::getModuleLogger("transport_client")->error("❌ Failed to start Transport client");
        return 1;
    }
    
    mcp_logger::getModuleLogger("transport_client")->info("✅ Transport Client started successfully");
    
    // 等待服务端启动
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 执行各种测试
    try {
        testPingRequest(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testEchoRequest(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testAddRequest(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testGetTimeRequest(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testGetInfoRequest(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testNotification(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testHeartbeatNotification(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        testBatchRequests(transport);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 监听通知
        mcp_logger::getModuleLogger("transport_client")->info("👂 Listening for notifications for 30 seconds...");
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(30)) {
            if (transport.hasMessage()) {
                auto message = transport.getMessage();
                mcp_logger::getModuleLogger("transport_client")->info("📨 Received: " + message.content);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    } catch (const std::exception& e) {
        mcp_logger::getModuleLogger("transport_client")->error("❌ Test error: " + std::string(e.what()));
    }
    
    // 停止客户端
    mcp_logger::getModuleLogger("transport_client")->info("🛑 Stopping Transport client...");
    transport.stop();
    mcp_logger::getModuleLogger("transport_client")->info("✅ Transport Client stopped");
    
    // 关闭日志系统
    mcp_logger::Logger::getInstance().shutdown();
    return 0;
}
