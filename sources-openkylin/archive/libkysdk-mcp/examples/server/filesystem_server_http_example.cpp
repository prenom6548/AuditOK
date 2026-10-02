/**
 * @file filesystem_server_http_example.cpp
 * @brief MCP FileSystem Server HTTP 传输示例
 *
 * 这个示例展示了如何使用MCPFileSystemServer类通过HTTP传输方式创建文件系统服务器。
 * 演示了基于HTTP的文件系统操作和资源管理功能。
 */

#include <signal.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

#include "logger.h"
#include "filesystem_server.h"
#include "common/transport.h"

using namespace mcp;
using namespace mcp_transport;

// 全局变量用于优雅关闭
static std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n🛑 收到停止信号，正在关闭服务器..."
                  << std::endl;
        g_running = false;
    }
}

// 打印服务器信息
void printServerInfo() {
    std::cout << "🚀 MCP FileSystem Server HTTP 示例" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "📋 功能特性:" << std::endl;
    std::cout << "  ✅ 基于HTTP传输的文件系统服务器" << std::endl;
    std::cout << "  ✅ 安全的路径验证和解析" << std::endl;
    std::cout << "  ✅ 可配置的读写权限控制" << std::endl;
    std::cout << "  ✅ 完整的文件系统操作工具" << std::endl;
    std::cout << "  ✅ 自动资源发现和管理" << std::endl;
    std::cout << "  ✅ HTTP/RESTful API 支持" << std::endl;
    std::cout << "=================================" << std::endl;
}

// 打印使用说明
void printUsage() {
    std::cout << "\n📖 使用说明:" << std::endl;
    std::cout << "  1. 服务器启动后，通过HTTP与客户端通信" << std::endl;
    std::cout << "  2. 支持的文件系统工具:" << std::endl;
    std::cout << "     - read_file: 读取文件内容" << std::endl;
    std::cout << "     - write_file: 写入文件内容" << std::endl;
    std::cout << "     - list_directory: 列出目录内容" << std::endl;
    std::cout << "     - create_directory: 创建目录" << std::endl;
    std::cout << "     - delete_file: 删除文件或目录" << std::endl;
    std::cout << "     - file_info: 获取文件/目录信息" << std::endl;
    std::cout << "  3. 支持的文件系统资源:" << std::endl;
    std::cout << "     - file://. : 根目录" << std::endl;
    std::cout << "     - file://path : 指定路径的文件或目录" << std::endl;
    std::cout << "  4. HTTP端点: http://localhost:<port><endpoint>"
              << std::endl;
    std::cout << "  5. 按 Ctrl+C 停止服务器" << std::endl;
}

// 初始化日志系统
void initializeLogger() {
    mcp_logger::LogConfig config;
    config.level = mcp_logger::LogLevel::DEBUG;
    config.target = mcp_logger::LogTarget::BOTH;  // 同时输出到文件和控制台
    config.file_path = "logs/mcp_filesystem_server_http.log";
    config.enable_timestamp = true;
    config.enable_thread_id = true;
    config.enable_file_location = true;
    config.format =
        "[{timestamp}] [{level}] [{thread_id}] {file_location} - {message}";
    config.auto_flush = true;

    mcp_logger::Logger::getInstance().initialize(config);
}

// 创建测试环境
void createTestEnvironment(const std::string& root_path) {
    try {
        // 创建测试目录结构
        std::filesystem::create_directories(root_path + "/test_files");
        std::filesystem::create_directories(root_path + "/test_files/subdir");
        std::filesystem::create_directories(root_path + "/docs");
        std::filesystem::create_directories(root_path + "/logs");

        // 创建测试文件
        std::ofstream sample_file(root_path + "/test_files/sample.txt");
        if (sample_file.is_open()) {
            sample_file << "这是一个HTTP测试文件\n";
            sample_file << "用于演示MCP文件系统服务器的HTTP功能\n";
            sample_file << "创建时间: " << std::time(nullptr) << "\n";
            sample_file << "传输方式: HTTP (超文本传输协议)\n";
            sample_file.close();
        }

        std::ofstream readme_file(root_path + "/README.md");
        if (readme_file.is_open()) {
            readme_file << "# MCP FileSystem Server HTTP Demo\n\n";
            readme_file << "这个目录包含用于测试MCP文件系统服务器HTTP功能的示例"
                           "文件和目录。\n\n";
            readme_file << "## 传输方式\n\n";
            readme_file
                << "- **HTTP (超文本传输协议)**: 通过HTTP协议进行通信\n";
            readme_file
                << "- **优势**: 标准化、跨平台、易于调试、适合网络部署\n";
            readme_file << "- **适用场景**: Web应用、远程访问、分布式系统\n\n";
            readme_file << "## 目录结构\n\n";
            readme_file << "- `test_files/`: 测试文件目录\n";
            readme_file << "  - `sample.txt`: 示例文本文件\n";
            readme_file << "  - `subdir/`: 子目录\n";
            readme_file << "- `docs/`: 文档目录\n";
            readme_file << "- `logs/`: 日志目录\n";
            readme_file << "- `README.md`: 本说明文件\n\n";
            readme_file << "## 测试方法\n\n";
            readme_file << "1. 使用 `list_directory` 工具列出目录内容\n";
            readme_file << "2. 使用 `read_file` 工具读取文件内容\n";
            readme_file << "3. 使用 `write_file` 工具创建新文件\n";
            readme_file << "4. 使用 `create_directory` 工具创建新目录\n";
            readme_file << "5. 使用 `file_info` 工具获取文件信息\n\n";
            readme_file << "## HTTP 通信特点\n\n";
            readme_file << "- 标准化协议，易于集成\n";
            readme_file << "- 支持跨网络访问\n";
            readme_file << "- 可以使用标准HTTP工具测试（curl, Postman等）\n";
            readme_file << "- 适合Web应用和远程客户端\n";
            readme_file.close();
        }

        std::ofstream config_file(root_path + "/test_files/http_config.json");
        if (config_file.is_open()) {
            config_file << "{\n";
            config_file
                << "  \"server_name\": \"MCP FileSystem Server HTTP\",\n";
            config_file << "  \"version\": \"1.0.0\",\n";
            config_file << "  \"transport_type\": \"HTTP\",\n";
            config_file << "  \"features\": [\n";
            config_file << "    \"file_operations\",\n";
            config_file << "    \"directory_management\",\n";
            config_file << "    \"path_validation\",\n";
            config_file << "    \"resource_discovery\",\n";
            config_file << "    \"http_communication\"\n";
            config_file << "  ],\n";
            config_file << "  \"http_settings\": {\n";
            config_file << "    \"protocol\": \"HTTP\",\n";
            config_file << "    \"timeout_seconds\": 30,\n";
            config_file << "    \"max_retries\": 3,\n";
            config_file << "    \"keep_alive\": true\n";
            config_file << "  },\n";
            config_file << "  \"created_at\": " << std::time(nullptr) << "\n";
            config_file << "}\n";
            config_file.close();
        }

        std::cout << "✅ HTTP测试环境创建完成" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "⚠️  创建测试环境失败: " << e.what()
                  << std::endl;
    }
}

// 演示服务器功能
void demonstrateServerFeatures(MCPFileSystemServer& server) {
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server_http");
    logger->info("开始演示HTTP服务器功能");

    std::cout << "\n🔧 HTTP服务器功能演示:" << std::endl;

    // 显示根路径
    std::cout << "  📁 根路径: " << server.getRootPath() << std::endl;

    // 显示写权限状态
    std::cout << "  ✏️  写操作: "
              << (server.isWriteEnabled() ? "启用" : "禁用") << std::endl;

    // 显示文件系统资源
    auto resources = server.listFileSystemResources();
    std::cout << "  📋 文件系统资源 (" << resources.size()
              << "个):" << std::endl;
    for (const auto& resource : resources) {
        std::cout << "    - " << resource << std::endl;
    }

    // 添加HTTP特定的资源
    server.addFileSystemResource("test_files/http_config.json", "HTTP配置",
                                 "HTTP传输配置文件");
    server.addFileSystemResource("logs", "日志目录", "服务器日志文件目录");
    std::cout << "  ➕ 添加了HTTP特定资源" << std::endl;

    logger->info("HTTP服务器功能演示完成");
}

// 运行服务器统计信息
void runServerStatistics(MCPFileSystemServer& server) {
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server_http");

    int heartbeat_counter = 0;
    int stats_counter = 0;

    while (g_running && server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        heartbeat_counter++;
        stats_counter++;

        // 每30秒发送一次心跳
        if (heartbeat_counter >= 300) {  // 100ms * 300 = 30s
            heartbeat_counter = 0;
            logger->info("💓 HTTP服务器心跳正常");
        }

        // 每5分钟显示一次统计信息
        if (stats_counter >= 3000) {  // 100ms * 3000 = 5分钟
            stats_counter = 0;
            logger->info("📊 HTTP服务器运行统计:");
            logger->info("  - 运行时间: " + std::to_string(std::time(nullptr)) +
                         " 秒");
            logger->info("  - 根路径: " + server.getRootPath());
            logger->info(
                "  - 资源数量: " +
                std::to_string(server.listFileSystemResources().size()));
            logger->info("  - 传输方式: HTTP (超文本传输协议)");
        }
    }
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 打印服务器信息
    printServerInfo();

    // 解析命令行参数
    std::string root_path = ".";
    bool enable_write = true;
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string endpoint = "/mcp";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--root" && i + 1 < argc) {
            root_path = argv[++i];
        } else if (arg == "--read-only") {
            enable_write = false;
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--endpoint" && i + 1 < argc) {
            endpoint = argv[++i];
            if (endpoint.empty() || endpoint.front() != '/') {
                endpoint = "/" + endpoint;
            }
        } else if (arg == "--help") {
            std::cout << "\n用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --root <path>       设置根路径 (默认: 当前目录)"
                      << std::endl;
            std::cout << "  --read-only         禁用写操作" << std::endl;
            std::cout << "  --host <host>       设置监听主机 (默认: 0.0.0.0)"
                      << std::endl;
            std::cout << "  --port <port>       设置监听端口 (默认: 8080)"
                      << std::endl;
            std::cout << "  --endpoint <path>   设置HTTP端点 (默认: /mcp)"
                      << std::endl;
            std::cout << "  --help             显示此帮助信息" << std::endl;
            return 0;
        }
    }

    std::cout << "📁 根路径: " << root_path << std::endl;
    std::cout << "✏️  写操作: " << (enable_write ? "启用" : "禁用")
              << std::endl;
    std::cout << "🌐 主机: " << host << std::endl;
    std::cout << "🔌 端口: " << port << std::endl;
    std::cout << "📍 端点: " << endpoint << std::endl;

    // 初始化日志系统
    initializeLogger();
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server_http");

    logger->info("🚀 启动 MCP FileSystem Server HTTP 示例");

    try {
        // 创建测试环境
        createTestEnvironment(root_path);

        // 创建文件系统服务器实例
        MCPFileSystemServer server(root_path, enable_write);

        // 演示服务器功能
        demonstrateServerFeatures(server);

        // 启动服务器 (使用HTTP配置)
        logger->info("🚀 启动文件系统服务器 (HTTP模式)...");
        logger->info("📋 HTTP配置信息:");
        logger->info("  - 主机: " + host);
        logger->info("  - 端口: " + std::to_string(port));
        logger->info("  - 端点: " + endpoint);
        logger->info("  - 协议: HTTP");
        logger->info("  - 超时: 30秒");
        logger->info("  - 最大重试: 3");
        logger->info("  - Keep-Alive: 启用");
        logger->info("  - 日志级别: INFO");

        // 创建HTTP传输配置（使用工具函数确保服务器模式正确设置）
        mcp_transport::TransportConfig http_config =
            mcp_transport::utils::createSseServerConfig(
                host, port, TransportProtocol::kHttp);
        http_config.name = "mcp_filesystem_server_http";
        http_config.enable_logging = true;
        http_config.log_level = 0;  // 0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR
        http_config.log_file_path =
            "logs/mcp_filesystem_server_http_transport.log";

        // HTTP/SSE特定配置
        http_config.sse.endpoint = endpoint;
        http_config.sse.timeout_seconds = 30;
        http_config.sse.max_retries = 3;
        http_config.sse.keep_alive = true;
        http_config.auto_reconnect = false;
        http_config.sse.headers["Content-Type"] =
            "application/json; charset=utf-8";
        http_config.sse.headers["Access-Control-Allow-Origin"] = "*";
        http_config.sse.headers["Access-Control-Allow-Methods"] =
            "GET, POST, PUT, DELETE, OPTIONS";
        http_config.sse.headers["Access-Control-Allow-Headers"] =
            "Content-Type, Authorization";
        http_config.sse.user_agent = "MCP-FileSystem-Server-HTTP/1.0";

        // 服务端配置
        http_config.server.server_name = "MCP FileSystem Server HTTP";
        http_config.server.server_version = "1.0.0";
        http_config.server.description =
            "MCP FileSystem Server with HTTP transport";

        logger->info("🔧 应用HTTP传输配置:");
        logger->info("  - 传输类型: SSE (HTTP)");
        logger->info("  - 服务端模式: " +
                     std::string(http_config.mode ==
                                         mcp_transport::TransportMode::kServer
                                     ? "SERVER (启用)"
                                     : "CLIENT (禁用)"));
        logger->info("  - 主机: " + http_config.sse.host);
        logger->info("  - 端口: " + std::to_string(http_config.sse.port));
        logger->info("  - 端点: " + http_config.sse.endpoint);
        logger->info("  - 协议: HTTP");
        logger->info("  - 超时: " +
                     std::to_string(http_config.sse.timeout_seconds) + "秒");
        logger->info("  - 最大重试: " +
                     std::to_string(http_config.sse.max_retries));
        logger->info("  - Keep-Alive: " +
                     std::string(http_config.sse.keep_alive ? "启用" : "禁用"));

        // 验证配置
        if (http_config.mode != mcp_transport::TransportMode::kServer) {
            logger->error("❌ 错误: TransportMode 未正确设置为 SERVER!");
            std::cerr << "❌ 错误: TransportMode 未正确设置为 SERVER!"
                      << std::endl;
            return 1;
        }

        if (server.start(http_config)) {
            logger->info("✅ 文件系统服务器启动成功 (HTTP模式)");
            std::cout << "✅ 文件系统服务器启动成功，正在运行..." << std::endl;
            std::cout << "🌐 HTTP服务地址: http://" << host << ":" << port
                      << endpoint << std::endl;
            std::cout << "💡 测试命令示例:" << std::endl;
            std::cout << "   curl -X POST http://" << host << ":" << port
                      << endpoint << " \\" << std::endl;
            std::cout << "     -H 'Content-Type: application/json' \\"
                      << std::endl;
            std::cout
                << "     -d "
                   "'{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\"}'"
                << std::endl;
            printUsage();

            // 运行服务器统计信息
            runServerStatistics(server);

            // 停止服务器
            logger->info("🛑 正在停止文件系统服务器...");
            server.stop();
            logger->info("✅ 文件系统服务器已停止");
            std::cout << "✅ 文件系统服务器已停止" << std::endl;

        } else {
            logger->error("❌ 文件系统服务器启动失败 (HTTP模式)");
            std::cerr << "❌ 文件系统服务器启动失败" << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        logger->error("❌ 服务器错误: " + std::string(e.what()));
        std::cerr << "❌ 服务器错误: " << e.what() << std::endl;
        return 1;
    }

    // 关闭日志系统
    mcp_logger::Logger::getInstance().shutdown();
    return 0;
}
