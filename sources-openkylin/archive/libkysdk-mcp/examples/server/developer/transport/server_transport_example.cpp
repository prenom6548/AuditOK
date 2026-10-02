/**
 * @file transport_example.cpp
 * @brief Transport 模块使用示例
 *
 * 这个文件演示了如何使用 transport 模块进行 IPC 和 HTTP 通信。
 * 支持通过配置文件或代码配置来启动不同的传输方式。
 */

#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "logger.h"
#include "common/transport.h"

using namespace mcp_transport;
using namespace mcp_logger;
using json = nlohmann::json;

// 全局变量用于信号处理
std::unique_ptr<Transport> g_transport = nullptr;
bool g_running = true;

// 初始化日志系统
void initializeLogger() {
    Logger& logger = Logger::getInstance();
    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::BOTH;
    config.file_path = "transport_example.log";
    config.enable_timestamp = true;
    config.enable_thread_id = true;
    config.enable_file_location = true;
    config.format =
        "[{timestamp}] [{level}] [{thread_id}] {file_location} - {message}";
    logger.initialize(config);
}

// 信号处理函数
void signalHandler(int signal) {
    mcp_logger::getModuleLogger("transport_example")
        ->info("收到信号 " + std::to_string(signal) + "，正在停止...");
    g_running = false;
    if (g_transport) {
        g_transport->stop();
    }
}

// 消息回调函数
void onMessageReceived(const TransportMessage& message) {
    mcp_logger::getModuleLogger("transport_example")->info("📥 收到消息:");
    mcp_logger::getModuleLogger("transport_example")->info("ID: " + message.id);
    mcp_logger::getModuleLogger("transport_example")
        ->info("内容: " + message.content);
    mcp_logger::getModuleLogger("transport_example")
        ->info("状态码: " + std::to_string(message.status_code));
    if (!message.error_message.empty()) {
        mcp_logger::getModuleLogger("transport_example")
            ->error("错误: " + message.error_message);
    }
}

// 连接回调函数
void onConnectionChanged(const ConnectionInfo& info) {
    mcp_logger::getModuleLogger("transport_example")->info("🔗 连接状态变化:");
    mcp_logger::getModuleLogger("transport_example")
        ->info("连接ID: " + info.connection_id);
    mcp_logger::getModuleLogger("transport_example")
        ->info("状态: " + std::to_string(static_cast<int>(info.status)));
    mcp_logger::getModuleLogger("transport_example")
        ->info("类型: " + std::to_string(static_cast<int>(info.type)));
    mcp_logger::getModuleLogger("transport_example")
        ->info("远程地址: " + info.remote_address + ":" +
               std::to_string(info.remote_port));
}

// 错误回调函数
void onErrorOccurred(const std::string& error) {
    mcp_logger::getModuleLogger("transport_example")->error("❌ 错误: " + error);
}

// 创建示例配置文件
void createSampleConfigs() {
    // 创建 IPC 配置示例
    json ipc_config;
    ipc_config["type"] = 0;  // IPC
    ipc_config["name"] = "mcp_ipc_transport";
    ipc_config["enable_logging"] = true;
    ipc_config["log_level"] = 1;
    ipc_config["log_file_path"] = "transport_ipc.log";

    ipc_config["ipc"]["command"] = "npx";
    ipc_config["ipc"]["args"] = {
        "-y", "@modelcontextprotocol/server-filesystem", "."};
    ipc_config["ipc"]["env_vars"]["NODE_ENV"] = "development";
    ipc_config["ipc"]["buffer_size"] = 4096;
    ipc_config["ipc"]["read_timeout_ms"] = 100;
    ipc_config["ipc"]["max_retry_count"] = 10;
    ipc_config["ipc"]["enable_logging"] = true;
    ipc_config["ipc"]["log_file_path"] = "ipc.log";
    ipc_config["ipc"]["log_level"] = 1;

    std::ofstream ipc_file("transport_ipc_config.json");
    ipc_file << ipc_config.dump(2);
    ipc_file.close();
    std::cout << "✅ 创建 IPC 配置文件: transport_ipc_config.json" << std::endl;

    // 创建 HTTP 配置示例
    json http_config;
    http_config["type"] = 1;  // HTTP
    http_config["name"] = "mcp_http_transport";
    http_config["enable_logging"] = true;
    http_config["log_level"] = 1;
    http_config["log_file_path"] = "transport_http.log";

    http_config["http"]["host"] = "mcp.didichuxing.com";
    http_config["http"]["port"] = 443;
    http_config["http"]["protocol"] = 1;  // HTTPS
    http_config["http"]["timeout_seconds"] = 30;
    http_config["http"]["max_retries"] = 3;
    http_config["http"]["user_agent"] = "MCP-Transport-Example/1.0";
    http_config["http"]["keep_alive"] = true;
    http_config["http"]["auto_reconnect"] = false;
    http_config["http"]["reconnect_interval"] = 5;
    http_config["http"]["headers"]["Content-Type"] = "application/json";
    http_config["http"]["headers"]["Accept"] = "application/json";

    std::ofstream http_file("transport_http_config.json");
    http_file << http_config.dump(2);
    http_file.close();
    std::cout << "✅ 创建 HTTP 配置文件: transport_http_config.json"
              << std::endl;

    // 创建自动选择配置示例
    json auto_config;
    auto_config["type"] = 2;  // AUTO
    auto_config["name"] = "mcp_auto_transport";
    auto_config["enable_logging"] = true;
    auto_config["log_level"] = 1;
    auto_config["log_file_path"] = "transport_auto.log";

    // 同时配置 IPC 和 HTTP，让系统自动选择
    auto_config["ipc"]["command"] = "npx";
    auto_config["ipc"]["args"] = {
        "-y", "@modelcontextprotocol/server-filesystem", "."};
    auto_config["ipc"]["env_vars"]["NODE_ENV"] = "development";
    auto_config["ipc"]["buffer_size"] = 4096;
    auto_config["ipc"]["read_timeout_ms"] = 100;
    auto_config["ipc"]["max_retry_count"] = 10;
    auto_config["ipc"]["enable_logging"] = true;
    auto_config["ipc"]["log_file_path"] = "ipc_auto.log";
    auto_config["ipc"]["log_level"] = 1;

    auto_config["http"]["host"] = "mcp.didichuxing.com";
    auto_config["http"]["port"] = 443;
    auto_config["http"]["protocol"] = 1;  // HTTPS
    auto_config["http"]["timeout_seconds"] = 30;
    auto_config["http"]["max_retries"] = 3;
    auto_config["http"]["user_agent"] = "MCP-Transport-Example/1.0";
    auto_config["http"]["keep_alive"] = true;
    auto_config["http"]["auto_reconnect"] = false;
    auto_config["http"]["reconnect_interval"] = 5;
    auto_config["http"]["headers"]["Content-Type"] = "application/json";
    auto_config["http"]["headers"]["Accept"] = "application/json";

    std::ofstream auto_file("transport_auto_config.json");
    auto_file << auto_config.dump(2);
    auto_file.close();
    std::cout << "✅ 创建自动选择配置文件: transport_auto_config.json"
              << std::endl;
}

// 测试 IPC 传输
void testIpcTransport() {
    std::cout << "\n=== 测试 IPC 传输 ===" << std::endl;

    try {
        // 创建 IPC 配置
        auto config = utils::createIpcConfig(
            "npx", {"-y", "@modelcontextprotocol/server-filesystem", "."},
            {{"NODE_ENV", "development"}});
        config.enable_logging = true;
        config.log_level = 1;
        config.log_file_path = "transport_ipc_test.log";

        // 创建传输实例
        auto transport = std::make_unique<Transport>();

        // 设置回调
        transport->setMessageCallback(onMessageReceived);
        transport->setConnectionCallback(onConnectionChanged);
        transport->setErrorCallback(onErrorOccurred);

        // 启动传输
        std::cout << "🚀 启动 IPC 传输..." << std::endl;
        if (!transport->start(config)) {
            std::cout << "❌ 启动 IPC 传输失败" << std::endl;
            return;
        }

        std::cout << "✅ IPC 传输启动成功" << std::endl;

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 发送测试消息
        std::vector<std::string> test_messages = {
            R"({"jsonrpc": "2.0", "method": "initialize", "id": 1, "params": {"protocolVersion": "2024-11-05", "capabilities": {}, "clientInfo": {"name": "transport-example", "version": "1.0.0"}}})",
            R"({"jsonrpc": "2.0", "method": "tools/list", "id": 2, "params": {}})",
            R"({"jsonrpc": "2.0", "method": "ping", "id": 3, "params": {}})"};

        for (size_t i = 0; i < test_messages.size(); ++i) {
            std::cout << "\n📤 发送测试消息 " << (i + 1) << "..." << std::endl;

            auto response = transport->sendRequestSync(test_messages[i]);

            std::cout << "📥 收到响应:" << std::endl;
            std::cout << "状态码: " << response.status_code << std::endl;
            std::cout << "内容: " << response.content << std::endl;

            if (response.status_code != 200) {
                std::cout << "错误: " << response.error_message << std::endl;
            }

            // 等待一下再发送下一条消息
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // 显示统计信息
        std::cout << "\n📊 统计信息:" << std::endl;
        auto stats = transport->getStatistics();
        std::cout << stats.dump(2) << std::endl;

        // 停止传输
        std::cout << "\n🛑 停止 IPC 传输..." << std::endl;
        transport->stop();
        std::cout << "✅ IPC 传输已停止" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ IPC 传输测试失败: " << e.what() << std::endl;
    }
}

// 测试 SSE 传输
void testSseTransport() {
    std::cout << "\n=== 测试 SSE 传输 ===" << std::endl;

    try {
        // 创建 SSE 配置
        auto config = utils::createSseSecureConfig("mcp.didichuxing.com", 443);
        config.enable_logging = true;
        config.log_level = 1;
        config.log_file_path = "transport_sse_test.log";
        config.sse.timeout_seconds = 30;
        config.sse.user_agent = "MCP-Transport-Example/1.0";
        config.sse.headers["Content-Type"] = "application/json";
        config.sse.headers["Accept"] = "application/json";

        // 创建传输实例
        auto transport = std::make_unique<Transport>();

        // 设置回调
        transport->setMessageCallback(onMessageReceived);
        transport->setConnectionCallback(onConnectionChanged);
        transport->setErrorCallback(onErrorOccurred);

        // 启动传输
        std::cout << "🚀 启动 SSE 传输..." << std::endl;
        if (!transport->start(config)) {
            std::cout << "❌ 启动 HTTP 传输失败" << std::endl;
            return;
        }

        std::cout << "✅ HTTP 传输启动成功" << std::endl;

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 发送测试消息
        std::string test_message = R"({
            "jsonrpc": "2.0",
            "method": "tools/list",
            "id": 1,
            "params": {
                "_meta": {
                    "progressToken": 1
                }
            }
        })";

        std::cout << "\n📤 发送测试消息..." << std::endl;

        auto response = transport->sendRequestSync(
            test_message, "POST",
            "/mcp-servers?key=EDPjGgm1axpZxdYn1Z3xxNyRB4nw07KQ");

        std::cout << "📥 收到响应:" << std::endl;
        std::cout << "状态码: " << response.status_code << std::endl;
        std::cout << "内容: " << response.content << std::endl;

        if (response.status_code != 200) {
            std::cout << "错误: " << response.error_message << std::endl;
        }

        // 显示统计信息
        std::cout << "\n📊 统计信息:" << std::endl;
        auto stats = transport->getStatistics();
        std::cout << stats.dump(2) << std::endl;

        // 停止传输
        std::cout << "\n🛑 停止 HTTP 传输..." << std::endl;
        transport->stop();
        std::cout << "✅ HTTP 传输已停止" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ HTTP 传输测试失败: " << e.what() << std::endl;
    }
}

// 测试配置文件加载
void testConfigFileLoading() {
    std::cout << "\n=== 测试配置文件加载 ===" << std::endl;

    try {
        // 创建示例配置文件
        createSampleConfigs();

        // 测试加载 IPC 配置
        std::cout << "📁 加载 IPC 配置文件..." << std::endl;
        auto ipc_config =
            TransportConfig::fromJsonFile("transport_ipc_config.json");
        std::cout << "✅ IPC 配置加载成功" << std::endl;
        std::cout << "类型: " << static_cast<int>(ipc_config.type) << std::endl;
        std::cout << "命令: " << ipc_config.ipc.command << std::endl;

        // 测试加载 SSE 配置
        std::cout << "\n📁 加载 SSE 配置文件..." << std::endl;
        auto sse_config =
            TransportConfig::fromJsonFile("transport_sse_config.json");
        std::cout << "✅ SSE 配置加载成功" << std::endl;
        std::cout << "类型: " << static_cast<int>(sse_config.type) << std::endl;
        std::cout << "主机: " << sse_config.sse.host << ":"
                  << sse_config.sse.port << std::endl;

        // 测试加载自动选择配置
        std::cout << "\n📁 加载自动选择配置文件..." << std::endl;
        auto auto_config =
            TransportConfig::fromJsonFile("transport_auto_config.json");
        std::cout << "✅ 自动选择配置加载成功" << std::endl;
        std::cout << "类型: " << static_cast<int>(auto_config.type)
                  << std::endl;
        std::cout << "IPC 命令: " << auto_config.ipc.command << std::endl;
        std::cout << "SSE 主机: " << auto_config.sse.host << ":"
                  << auto_config.sse.port << std::endl;

        // 测试配置验证
        std::cout << "\n🔍 验证配置..." << std::endl;
        std::cout << "IPC 配置有效: " << (ipc_config.validate() ? "是" : "否")
                  << std::endl;
        std::cout << "SSE 配置有效: " << (sse_config.validate() ? "是" : "否")
                  << std::endl;
        std::cout << "自动选择配置有效: "
                  << (auto_config.validate() ? "是" : "否") << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ 配置文件加载测试失败: " << e.what() << std::endl;
    }
}

// 交互式测试
void interactiveTest() {
    std::cout << "\n=== 交互式测试 ===" << std::endl;

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // 创建传输实例
        g_transport = std::make_unique<Transport>();

        // 设置回调
        g_transport->setMessageCallback(onMessageReceived);
        g_transport->setConnectionCallback(onConnectionChanged);
        g_transport->setErrorCallback(onErrorOccurred);

        // 选择传输类型
        std::cout << "请选择传输类型:" << std::endl;
        std::cout << "1. IPC (进程间通信)" << std::endl;
        std::cout << "2. HTTP (网络通信)" << std::endl;
        std::cout << "3. 自动选择" << std::endl;
        std::cout << "请输入选择 (1-3): ";

        int choice;
        std::cin >> choice;

        TransportConfig config;

        switch (choice) {
            case 1: {
                config = utils::createIpcConfig(
                    "npx",
                    {"-y", "@modelcontextprotocol/server-filesystem", "."},
                    {{"NODE_ENV", "development"}});
                config.enable_logging = true;
                config.log_level = 1;
                config.log_file_path = "transport_interactive.log";
                break;
            }
            case 2: {
                config =
                    utils::createSseSecureConfig("mcp.didichuxing.com", 443);
                config.enable_logging = true;
                config.log_level = 1;
                config.log_file_path = "transport_interactive.log";
                config.sse.timeout_seconds = 30;
                config.sse.user_agent = "MCP-Transport-Interactive/1.0";
                config.sse.headers["Content-Type"] = "application/json";
                config.sse.headers["Accept"] = "application/json";
                break;
            }
            case 3: {
                config.type = TransportType::kAuto;
                config.enable_logging = true;
                config.log_level = 1;
                config.log_file_path = "transport_interactive.log";

                // 配置 IPC
                config.ipc.command = "npx";
                config.ipc.args = {
                    "-y", "@modelcontextprotocol/server-filesystem", "."};
                config.ipc.env_vars["NODE_ENV"] = "development";
                config.ipc.buffer_size = 4096;
                config.ipc.read_timeout_ms = 100;
                config.ipc.max_retry_count = 10;
                config.ipc.enable_logging = true;
                config.ipc.log_file_path = "ipc_interactive.log";
                config.ipc.log_level = mcp_transport::TransportLogLevel::kInfo;

                // 配置 SSE
                config.sse.host = "mcp.didichuxing.com";
                config.sse.port = 443;
                config.sse.protocol = TransportProtocol::kHttps;
                config.sse.timeout_seconds = 30;
                config.sse.max_retries = 3;
                config.sse.user_agent = "MCP-Transport-Interactive/1.0";
                config.sse.keep_alive = true;
                config.auto_reconnect = false;
                config.reconnect_interval = 5;
                config.sse.headers["Content-Type"] = "application/json";
                config.sse.headers["Accept"] = "application/json";
                break;
            }
            default:
                std::cout << "❌ 无效选择" << std::endl;
                return;
        }

        // 启动传输
        std::cout << "\n🚀 启动传输..." << std::endl;
        if (!g_transport->start(config)) {
            std::cout << "❌ 启动传输失败" << std::endl;
            return;
        }

        std::cout << "✅ 传输启动成功" << std::endl;
        std::cout << "输入 'quit' 退出，输入 'status' "
                     "查看状态，输入其他内容作为消息发送"
                  << std::endl;

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 交互式循环
        std::string input;
        while (g_running) {
            std::cout << "\n> ";
            std::getline(std::cin, input);

            if (input == "quit") {
                break;
            } else if (input == "status") {
                auto conn_info = g_transport->getConnectionInfo();
                std::cout << "连接状态: " << static_cast<int>(conn_info.status)
                          << std::endl;
                std::cout << "传输类型: " << static_cast<int>(conn_info.type)
                          << std::endl;
                std::cout << "运行状态: "
                          << (g_transport->isRunning() ? "是" : "否")
                          << std::endl;
                std::cout << "健康检查: "
                          << (g_transport->healthCheck() ? "通过" : "失败")
                          << std::endl;
            } else if (!input.empty()) {
                std::cout << "📤 发送消息: " << input << std::endl;
                auto response = g_transport->sendRequestSync(input);
                std::cout << "📥 收到响应 (状态码: " << response.status_code
                          << "): " << response.content << std::endl;
            }
        }

        // 停止传输
        std::cout << "\n🛑 停止传输..." << std::endl;
        g_transport->stop();
        std::cout << "✅ 传输已停止" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ 交互式测试失败: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "🚀 MCP Transport 模块示例" << std::endl;
    std::cout << "========================" << std::endl;

    if (argc > 1) {
        std::string mode = argv[1];

        if (mode == "ipc") {
            testIpcTransport();
        } else if (mode == "http") {
            testSseTransport();
        } else if (mode == "config") {
            testConfigFileLoading();
        } else if (mode == "interactive") {
            interactiveTest();
        } else {
            std::cout << "用法: " << argv[0] << " [ipc|http|config|interactive]"
                      << std::endl;
            std::cout << "  ipc        - 测试 IPC 传输" << std::endl;
            std::cout << "  http       - 测试 HTTP 传输" << std::endl;
            std::cout << "  config     - 测试配置文件加载" << std::endl;
            std::cout << "  interactive - 交互式测试" << std::endl;
            return 1;
        }
    } else {
        // 默认运行所有测试
        testConfigFileLoading();
        testIpcTransport();
        testSseTransport();

        std::cout << "\n💡 提示: 使用 '" << argv[0]
                  << " interactive' 进行交互式测试" << std::endl;
    }

    std::cout << "\n✅ 示例程序完成" << std::endl;
    return 0;
}
