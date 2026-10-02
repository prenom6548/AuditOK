/**
 * @file protocol_example.cpp
 * @brief MCP Protocol 使用示例
 */

#include "common/protocol.h"
#include <iostream>
#include "json.hpp"

using namespace mcp;

int main() {
    std::cout << "MCP Protocol Example" << std::endl;
    
    try {
        // 测试客户端能力创建
        auto client_caps = mcp::utils::createDefaultClientCapabilities();
        std::cout << "Client capabilities created successfully" << std::endl;
        
        // 测试服务器能力创建
        auto server_caps = mcp::utils::createDefaultServerCapabilities();
        std::cout << "Server capabilities created successfully" << std::endl;
        
        // 测试工具创建
        nlohmann::json schema = {
            {"type", "object"},
            {"properties", nlohmann::json::object()}
        };
        auto tool = mcp::utils::createToolFromSchema("test_tool", "A test tool", schema);
        std::cout << "Tool created: " << tool.name << std::endl;
        
        std::cout << "✅ All protocol operations completed successfully" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
