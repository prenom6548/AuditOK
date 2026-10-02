/**
 * @file filesystem_client_http_example.cpp
 * @brief MCP FileSystem Client HTTP 传输示例
 *
 * 这个示例展示了如何使用MCPClient类通过HTTP传输方式连接文件系统服务器，
 * 并演示所有文件系统操作工具和资源管理功能。
 */

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"
#include "client.h"
#include "common/transport.h"

using namespace mcp;
using namespace mcp_logger;

// 初始化日志系统
void initialize_logger() {
    Logger& logger = Logger::getInstance();
    LogConfig config;
    config.level = mcp_logger::LogLevel::DEBUG_LEVEL;
    config.target = mcp_logger::LogTarget::BOTH;
    config.file_path = "logs/mcp_filesystem_client_http.log";
    config.enable_timestamp = true;
    config.enable_thread_id = true;
    config.enable_file_location = true;
    config.format =
        "[{timestamp}] [{level}] [{thread_id}] {file_location} - {message}";
    logger.initialize(config);
}

// 打印客户端信息
void printClientInfo() {
    std::cout << "🚀 MCP FileSystem Client HTTP 示例" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "📋 功能特性:" << std::endl;
    std::cout << "  ✅ 基于HTTP传输的文件系统客户端" << std::endl;
    std::cout << "  ✅ 完整的文件系统操作工具测试" << std::endl;
    std::cout << "  ✅ 文件系统资源发现和管理" << std::endl;
    std::cout << "  ✅ HTTP/RESTful API 支持" << std::endl;
    std::cout << "=================================" << std::endl;
}

// 打印使用说明
void printUsage() {
    std::cout << "\n📖 使用说明:" << std::endl;
    std::cout << "  1. 确保文件系统服务器正在运行" << std::endl;
    std::cout << "  2. 默认连接: http://localhost:8080/mcp" << std::endl;
    std::cout << "  3. 支持的命令行参数:" << std::endl;
    std::cout << "     --host <host>       服务器主机 (默认: localhost)"
              << std::endl;
    std::cout << "     --port <port>       服务器端口 (默认: 8080)"
              << std::endl;
    std::cout << "     --endpoint <path>   HTTP端点 (默认: /mcp)" << std::endl;
    std::cout << "     --help             显示此帮助信息" << std::endl;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    std::string host = "localhost";
    int port = 8080;
    std::string endpoint = "/mcp";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--endpoint" && i + 1 < argc) {
            endpoint = argv[++i];
            if (endpoint.empty() || endpoint.front() != '/') {
                endpoint = "/" + endpoint;
            }
        } else if (arg == "--help") {
            printClientInfo();
            printUsage();
            return 0;
        }
    }

    // 打印客户端信息
    printClientInfo();
    std::cout << "🌐 服务器: " << host << ":" << port << std::endl;
    std::cout << "📍 端点: " << endpoint << std::endl;

    // 初始化日志系统
    initialize_logger();
    MCP_LOG_INFO("🚀 启动 MCP FileSystem Client HTTP 示例");

    try {
        // 创建 MCP 客户端实例
        auto client = std::make_unique<MCPClient>();

        // 创建 HTTP/SSE 传输配置
        mcp_transport::TransportConfig config;
        config.type = mcp_transport::TransportType::kSse;
        config.enable_logging = true;
        config.log_level = 1;  // INFO
        config.log_file_path = "logs/mcp_filesystem_client_http_transport.log";

        // HTTP/SSE 配置 - 连接到文件系统服务器
        config.sse.host = host;
        config.sse.port = port;
        config.sse.protocol = mcp_transport::TransportProtocol::kHttp;
        config.sse.timeout_seconds = 30;
        config.sse.max_retries = 3;
        config.sse.user_agent = "MCP-FileSystem-Client-HTTP/1.0";
        config.sse.keep_alive = true;
        config.auto_reconnect = true;
        config.reconnect_interval = 5;
        config.sse.headers["Content-Type"] = "application/json; charset=utf-8";
        config.sse.headers["Accept"] = "application/json, text/event-stream";
        config.sse.headers["User-Agent"] = "MCP-FileSystem-Client-HTTP/1.0";

        // 设置端点
        std::vector<std::string> endpoints = {endpoint};
        client->setEndpoints(endpoints);

        MCP_LOG_INFO("📋 配置信息:");
        MCP_LOG_INFO("  传输类型: HTTP/SSE");
        MCP_LOG_INFO("  主机: " + config.sse.host);
        MCP_LOG_INFO("  端口: " + std::to_string(config.sse.port));
        MCP_LOG_INFO("  端点: " + endpoint);
        MCP_LOG_INFO("  协议: HTTP");

        // 启动连接
        MCP_LOG_INFO("🚀 连接到文件系统服务器...");
        if (!client->connectWithConfig(config)) {
            MCP_LOG_ERROR("❌ 连接失败");
            std::cerr << "❌ 无法连接到服务器 " << host << ":" << port
                      << endpoint << std::endl;
            std::cerr << "💡 提示: 请确保文件系统服务器正在运行" << std::endl;
            return 1;
        }

        MCP_LOG_INFO("✅ 连接成功");
        std::cout << "✅ 已连接到文件系统服务器" << std::endl;

        // 等待连接建立
        MCP_LOG_INFO("⏳ 等待连接建立与服务初始化...");
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // 检查连接状态
        if (!client->isConnected()) {
            MCP_LOG_ERROR("❌ 错误: 客户端未连接");
            std::cerr << "❌ 错误: 客户端未连接" << std::endl;
            return 1;
        }

        // 设置客户端信息
        client->setClientInfo("mcp-filesystem-client-http", "1.0.0");

        // 创建初始化参数
        InitializeParams init_params;
        init_params.protocolVersion = "2025-11-25";
        init_params.clientInfo.name = "mcp-filesystem-client-http";
        init_params.clientInfo.version = "1.0.0";
        init_params.capabilities = utils::createDefaultClientCapabilities();

        // 初始化 MCP 协议
        MCP_LOG_INFO("\n📤 初始化 MCP 协议...");
        try {
            auto result = client->initialize(init_params);
            MCP_LOG_INFO("  ✅ 初始化成功");
            std::cout << "✅ 初始化成功" << std::endl;
            std::cout << "  服务器: " << result.serverInfo.name << " v"
                      << result.serverInfo.version << std::endl;
            std::cout << "  协议版本: " << result.protocolVersion << std::endl;
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ 初始化失败: " + std::string(e.what()));
            std::cerr << "❌ 初始化失败: " << e.what() << std::endl;
            return 1;
        }

        // 发送初始化完成通知
        MCP_LOG_INFO("\n📤 发送初始化完成通知...");
        try {
            client->initialized();
            MCP_LOG_INFO("  ✅ 初始化完成通知发送成功");
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ 初始化完成通知发送失败: " +
                          std::string(e.what()));
        }

        // 获取可用工具列表
        MCP_LOG_INFO("\n📤 获取可用工具列表...");
        std::vector<Tool> tools;
        try {
            tools = client->listTools().tools;
            MCP_LOG_INFO("  ✅ 获取工具列表成功");
            std::cout << "\n📋 可用工具 (" << tools.size()
                      << "个):" << std::endl;
            for (const auto& tool : tools) {
                std::cout << "  - " << tool.name << ": " << tool.description
                          << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ 获取工具列表失败: " + std::string(e.what()));
            std::cerr << "❌ 获取工具列表失败: " << e.what() << std::endl;
            return 1;
        }

        // 获取可用资源列表
        MCP_LOG_INFO("\n📤 获取可用资源列表...");
        std::vector<Resource> resources;
        try {
            resources = client->listResources().resources;
            MCP_LOG_INFO("  ✅ 获取资源列表成功");
            std::cout << "\n📋 可用资源 (" << resources.size()
                      << "个):" << std::endl;
            for (const auto& resource : resources) {
                std::cout << "  - " << resource.name << " (" << resource.uri
                          << ")" << std::endl;
                std::cout << "    描述: " << resource.description << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ 获取资源列表失败: " + std::string(e.what()));
            std::cerr << "⚠️  获取资源列表失败: " << e.what()
                      << std::endl;
        }

        // 测试文件系统工具
        std::cout << "\n🔧 开始测试文件系统工具..." << std::endl;

        // 测试 1: list_directory - 列出根目录
        MCP_LOG_INFO("\n📤 测试 1: list_directory (根目录)");
        try {
            nlohmann::json params = {{"path", "."}};
            auto result = client->callTool("list_directory", params);
            MCP_LOG_INFO("  ✅ list_directory 成功");
            std::cout << "\n✅ 测试 1: list_directory (根目录)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ list_directory 失败: " + std::string(e.what()));
            std::cerr << "❌ list_directory 失败: " << e.what() << std::endl;
        }

        // 测试 2: list_directory - 列出 test_files 目录
        MCP_LOG_INFO("\n📤 测试 2: list_directory (test_files)");
        try {
            nlohmann::json params = {{"path", "test_files"}};
            auto result = client->callTool("list_directory", params);
            MCP_LOG_INFO("  ✅ list_directory 成功");
            std::cout << "\n✅ 测试 2: list_directory (test_files)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ list_directory 失败: " + std::string(e.what()));
            std::cerr << "❌ list_directory 失败: " << e.what() << std::endl;
        }

        // 测试 3: read_file - 读取 README.md
        MCP_LOG_INFO("\n📤 测试 3: read_file (README.md)");
        try {
            nlohmann::json params = {{"path", "README.md"}};
            auto result = client->callTool("read_file", params);
            MCP_LOG_INFO("  ✅ read_file 成功");
            std::cout << "\n✅ 测试 3: read_file (README.md)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::string content = result.content.value("content", "");
                if (content.length() > 500) {
                    std::cout << "  文件内容 (前500字符):" << std::endl;
                    std::cout << "  " << content.substr(0, 500) << "..."
                              << std::endl;
                } else {
                    std::cout << "  文件内容:" << std::endl;
                    std::cout << "  " << content << std::endl;
                }
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ read_file 失败: " + std::string(e.what()));
            std::cerr << "❌ read_file 失败: " << e.what() << std::endl;
        }

        // 测试 4: read_file - 读取 test_files/sample.txt
        MCP_LOG_INFO("\n📤 测试 4: read_file (test_files/sample.txt)");
        try {
            nlohmann::json params = {{"path", "test_files/sample.txt"}};
            auto result = client->callTool("read_file", params);
            MCP_LOG_INFO("  ✅ read_file 成功");
            std::cout << "\n✅ 测试 4: read_file (test_files/sample.txt)"
                      << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::string content = result.content.value("content", "");
                std::cout << "  文件内容:" << std::endl;
                std::cout << "  " << content << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ read_file 失败: " + std::string(e.what()));
            std::cerr << "❌ read_file 失败: " << e.what() << std::endl;
        }

        // 测试 5: file_info - 获取 README.md 信息
        MCP_LOG_INFO("\n📤 测试 5: file_info (README.md)");
        try {
            nlohmann::json params = {{"path", "README.md"}};
            auto result = client->callTool("file_info", params);
            MCP_LOG_INFO("  ✅ file_info 成功");
            std::cout << "\n✅ 测试 5: file_info (README.md)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  文件信息: " << result.content.dump(2)
                          << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ file_info 失败: " + std::string(e.what()));
            std::cerr << "❌ file_info 失败: " << e.what() << std::endl;
        }

        // 测试 6: write_file - 创建新文件
        MCP_LOG_INFO("\n📤 测试 6: write_file (创建新文件)");
        try {
            std::string test_content = "这是通过HTTP客户端创建的文件\n";
            test_content +=
                "创建时间: " + std::to_string(std::time(nullptr)) + "\n";
            test_content += "传输方式: HTTP\n";
            test_content +=
                "这是一个测试文件，用于演示MCP文件系统客户端的HTTP功能。\n";

            nlohmann::json params = {{"path", "test_files/client_test.txt"},
                                     {"content", test_content}};
            auto result = client->callTool("write_file", params);
            MCP_LOG_INFO("  ✅ write_file 成功");
            std::cout << "\n✅ 测试 6: write_file (创建新文件)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ write_file 失败: " + std::string(e.what()));
            std::cerr << "❌ write_file 失败: " << e.what() << std::endl;
        }

        // 测试 7: read_file - 读取刚创建的文件
        MCP_LOG_INFO("\n📤 测试 7: read_file (读取刚创建的文件)");
        try {
            nlohmann::json params = {{"path", "test_files/client_test.txt"}};
            auto result = client->callTool("read_file", params);
            MCP_LOG_INFO("  ✅ read_file 成功");
            std::cout << "\n✅ 测试 7: read_file (读取刚创建的文件)"
                      << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::string content = result.content.value("content", "");
                std::cout << "  文件内容:" << std::endl;
                std::cout << "  " << content << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ read_file 失败: " + std::string(e.what()));
            std::cerr << "❌ read_file 失败: " << e.what() << std::endl;
        }

        // 测试 8: create_directory - 创建新目录
        MCP_LOG_INFO("\n📤 测试 8: create_directory (创建新目录)");
        try {
            nlohmann::json params = {{"path", "test_files/client_test_dir"}};
            auto result = client->callTool("create_directory", params);
            MCP_LOG_INFO("  ✅ create_directory 成功");
            std::cout << "\n✅ 测试 8: create_directory (创建新目录)"
                      << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ create_directory 失败: " +
                          std::string(e.what()));
            std::cerr << "❌ create_directory 失败: " << e.what() << std::endl;
        }

        // 测试 9: list_directory - 列出新创建的目录
        MCP_LOG_INFO("\n📤 测试 9: list_directory (列出新创建的目录)");
        try {
            nlohmann::json params = {{"path", "test_files"}};
            auto result = client->callTool("list_directory", params);
            MCP_LOG_INFO("  ✅ list_directory 成功");
            std::cout << "\n✅ 测试 9: list_directory (列出新创建的目录)"
                      << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ list_directory 失败: " + std::string(e.what()));
            std::cerr << "❌ list_directory 失败: " << e.what() << std::endl;
        }

        // 测试 10: write_file - 在新目录中创建文件
        MCP_LOG_INFO("\n📤 测试 10: write_file (在新目录中创建文件)");
        try {
            std::string test_content = "这是在新目录中创建的文件\n";
            test_content += "通过HTTP客户端创建\n";

            nlohmann::json params = {
                {"path", "test_files/client_test_dir/nested_file.txt"},
                {"content", test_content}};
            auto result = client->callTool("write_file", params);
            MCP_LOG_INFO("  ✅ write_file 成功");
            std::cout << "\n✅ 测试 10: write_file (在新目录中创建文件)"
                      << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ write_file 失败: " + std::string(e.what()));
            std::cerr << "❌ write_file 失败: " << e.what() << std::endl;
        }

        // 测试 11: file_info - 获取目录信息
        MCP_LOG_INFO("\n📤 测试 11: file_info (获取目录信息)");
        try {
            nlohmann::json params = {{"path", "test_files/client_test_dir"}};
            auto result = client->callTool("file_info", params);
            MCP_LOG_INFO("  ✅ file_info 成功");
            std::cout << "\n✅ 测试 11: file_info (获取目录信息)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  目录信息: " << result.content.dump(2)
                          << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ file_info 失败: " + std::string(e.what()));
            std::cerr << "❌ file_info 失败: " << e.what() << std::endl;
        }

        // 测试 12: delete_file - 删除文件
        MCP_LOG_INFO("\n📤 测试 12: delete_file (删除文件)");
        try {
            nlohmann::json params = {{"path", "test_files/client_test.txt"}};
            auto result = client->callTool("delete_file", params);
            MCP_LOG_INFO("  ✅ delete_file 成功");
            std::cout << "\n✅ 测试 12: delete_file (删除文件)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ delete_file 失败: " + std::string(e.what()));
            std::cerr << "❌ delete_file 失败: " << e.what() << std::endl;
        }

        // 测试 13: delete_file - 删除目录
        MCP_LOG_INFO("\n📤 测试 13: delete_file (删除目录)");
        try {
            nlohmann::json params = {{"path", "test_files/client_test_dir"}};
            auto result = client->callTool("delete_file", params);
            MCP_LOG_INFO("  ✅ delete_file 成功");
            std::cout << "\n✅ 测试 13: delete_file (删除目录)" << std::endl;
            if (result.isError) {
                std::cout << "  错误: " << result.errorMessage << std::endl;
            } else {
                std::cout << "  响应: " << result.content.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  ❌ delete_file 失败: " + std::string(e.what()));
            std::cerr << "❌ delete_file 失败: " << e.what() << std::endl;
        }

        // 测试文件系统资源
        std::cout << "\n🔧 开始测试文件系统资源..." << std::endl;

        // 测试资源读取
        for (const auto& resource : resources) {
            if (resource.uri.find("file://") == 0) {
                MCP_LOG_INFO("\n📤 测试资源读取: " + resource.uri);
                try {
                    auto content = client->readResource(resource.uri);
                    MCP_LOG_INFO("  ✅ 资源读取成功");
                    std::cout << "\n✅ 资源读取: " << resource.uri << std::endl;
                    if (!content.contents.empty() &&
                        !content.contents.front().text.empty()) {
                        const std::string& text = content.contents.front().text;
                        if (text.length() > 200) {
                            std::cout
                                << "  内容 (前200字符): " << text.substr(0, 200)
                                << "..." << std::endl;
                        } else {
                            std::cout << "  内容: " << text << std::endl;
                        }
                    } else {
                        std::cout << "  内容: " << content.toJson().dump(2)
                                  << std::endl;
                    }
                } catch (const std::exception& e) {
                    MCP_LOG_ERROR("  ❌ 资源读取失败: " + std::string(e.what()));
                    std::cerr << "❌ 资源读取失败: " << e.what() << std::endl;
                }
                break;  // 只测试第一个文件资源
            }
        }

        // 显示服务器信息
        std::cout << "\n📊 服务器信息:" << std::endl;
        try {
            auto server_info = client->getServerInfo();
            auto server_caps = client->getServerCapabilities();
            std::cout << "  服务器名称: " << server_info.name << std::endl;
            std::cout << "  服务器版本: " << server_info.version << std::endl;
            std::cout << "  服务器能力: " << server_caps.toJson().dump(2)
                      << std::endl;
        } catch (const std::exception& e) {
            MCP_LOG_ERROR("  获取服务器信息失败: " + std::string(e.what()));
        }

        // 测试连接稳定性
        std::cout << "\n🔧 测试连接稳定性..." << std::endl;
        for (int i = 0; i < 3; ++i) {
            MCP_LOG_INFO("  稳定性测试 " + std::to_string(i + 1) + "/3");
            try {
                client->ping();
                std::cout << "  ✅ Ping " << (i + 1) << " 成功" << std::endl;
            } catch (const std::exception& e) {
                MCP_LOG_ERROR("    ❌ Ping " + std::to_string(i + 1) +
                              " 失败: " + std::string(e.what()));
                std::cerr << "  ❌ Ping " << (i + 1) << " 失败: " << e.what()
                          << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        // 断开连接
        std::cout << "\n🛑 断开连接..." << std::endl;
        MCP_LOG_INFO("🛑 断开 MCP 客户端连接...");
        client->disconnect();
        MCP_LOG_INFO("✅ MCP 客户端连接已断开");
        std::cout << "✅ 已断开连接" << std::endl;

    } catch (const std::exception& e) {
        MCP_LOG_ERROR("❌ 错误: " + std::string(e.what()));
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n✅ MCP FileSystem Client HTTP 示例完成" << std::endl;
    MCP_LOG_INFO("✅ MCP FileSystem Client HTTP 示例完成");

    // 关闭日志系统
    Logger::getInstance().shutdown();
    return 0;
}
