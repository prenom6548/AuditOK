#include "protocol.h"
#include <iostream>
#include <iomanip>

using namespace mcp;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void test_client_capabilities() {
    print_separator("测试 Client Capabilities");
    
    // 创建默认客户端能力
    ClientCapabilities caps = utils::createDefaultClientCapabilities();
    
    // 添加自定义能力
    caps.tools["listChanged"] = true;
    caps.resources["subscribe"] = true;
    
    // 转换为 JSON
    json caps_json = caps.toJson();
    std::cout << "Client Capabilities JSON:\n";
    std::cout << caps_json.dump(2) << "\n";
    
    // 从 JSON 反序列化
    ClientCapabilities caps2 = ClientCapabilities::fromJson(caps_json);
    std::cout << "\n反序列化成功！\n";
}

void test_server_capabilities() {
    print_separator("测试 Server Capabilities");
    
    // 创建默认服务器能力
    ServerCapabilities caps = utils::createDefaultServerCapabilities();
    
    // 添加工具支持
    caps.tools = {
        {"listChanged", true}
    };
    
    // 转换为 JSON
    json caps_json = caps.toJson();
    std::cout << "Server Capabilities JSON:\n";
    std::cout << caps_json.dump(2) << "\n";
    
    // 从 JSON 反序列化
    ServerCapabilities caps2 = ServerCapabilities::fromJson(caps_json);
    std::cout << "\n反序列化成功！\n";
}

void test_initialize() {
    print_separator("测试 Initialize 请求/响应");
    
    // 创建初始化请求参数
    InitializeParams init_params;
    init_params.protocolVersion = PROTOCOL_VERSION;
    init_params.clientInfo.name = "cpp-mcp-client";
    init_params.clientInfo.version = "1.0.0";
    init_params.capabilities = utils::createDefaultClientCapabilities();
    
    json init_json = init_params.toJson();
    std::cout << "Initialize Request JSON:\n";
    std::cout << init_json.dump(2) << "\n";
    
    // 创建初始化响应结果
    InitializeResult init_result;
    init_result.protocolVersion = PROTOCOL_VERSION;
    init_result.serverInfo.name = "mcp-server";
    init_result.serverInfo.version = "1.0.0";
    init_result.capabilities = utils::createDefaultServerCapabilities();
    
    json result_json = init_result.toJson();
    std::cout << "\nInitialize Response JSON:\n";
    std::cout << result_json.dump(2) << "\n";
}

void test_tool() {
    print_separator("测试 Tool 定义");
    
    // 创建工具定义
    Tool tool;
    tool.name = "read_file";
    tool.description = "读取文件内容";
    tool.inputSchema = {
        {"type", "object"},
        {"properties", {
            {"path", {
                {"type", "string"},
                {"description", "文件路径"}
            }},
            {"encoding", {
                {"type", "string"},
                {"default", "utf-8"}
            }}
        }},
        {"required", {"path"}}
    };
    
    json tool_json = tool.toJson();
    std::cout << "Tool JSON:\n";
    std::cout << tool_json.dump(2) << "\n";
    
    // 从 JSON 反序列化
    Tool tool2 = Tool::fromJson(tool_json);
    std::cout << "\n反序列化工具: " << tool2.name << "\n";
    std::cout << "描述: " << tool2.description << "\n";
}

void test_tool_call() {
    print_separator("测试 Tool Call");
    
    // 创建工具调用参数
    ToolCallParams params;
    params.name = "read_file";
    params.arguments = {
        {"path", "/tmp/test.txt"},
        {"encoding", "utf-8"}
    };
    
    json params_json = params.toJson();
    std::cout << "Tool Call Params JSON:\n";
    std::cout << params_json.dump(2) << "\n";
    
    // 创建工具调用结果（成功）
    ToolCallResult result;
    result.isError = false;
    result.content = {
        {"text", "文件内容..."}
    };
    
    json result_json = result.toJson();
    std::cout << "\nTool Call Result (Success) JSON:\n";
    std::cout << result_json.dump(2) << "\n";
    
    // 创建工具调用结果（错误）
    ToolCallResult error_result;
    error_result.isError = true;
    error_result.errorMessage = "文件不存在";
    
    json error_json = error_result.toJson();
    std::cout << "\nTool Call Result (Error) JSON:\n";
    std::cout << error_json.dump(2) << "\n";
}

void test_resource() {
    print_separator("测试 Resource");
    
    // 创建资源
    Resource resource;
    resource.uri = "file:///tmp/example.txt";
    resource.name = "示例文件";
    resource.description = "这是一个示例资源";
    resource.mimeType = "text/plain";
    
    json resource_json = resource.toJson();
    std::cout << "Resource JSON:\n";
    std::cout << resource_json.dump(2) << "\n";
    
    // 使用工具函数创建资源
    Resource resource2 = utils::createResourceFromUri(
        "file:///tmp/test.txt",
        "测试文件",
        "测试资源描述"
    );
    
    json resource2_json = resource2.toJson();
    std::cout << "\n使用工具函数创建的 Resource JSON:\n";
    std::cout << resource2_json.dump(2) << "\n";
}

void test_prompt() {
    print_separator("测试 Prompt");
    
    // 创建提示
    Prompt prompt;
    prompt.name = "code_review";
    prompt.description = "代码审查提示";
    prompt.arguments = {"code", "language", "style"};
    
    json prompt_json = prompt.toJson();
    std::cout << "Prompt JSON:\n";
    std::cout << prompt_json.dump(2) << "\n";
    
    // 从 JSON 反序列化
    Prompt prompt2 = Prompt::fromJson(prompt_json);
    std::cout << "\n反序列化提示: " << prompt2.name << "\n";
    std::cout << "参数数量: " << prompt2.arguments.size() << "\n";
}

void test_protocol_version() {
    print_separator("测试协议版本");
    
    std::cout << "MCP 协议版本: " << PROTOCOL_VERSION << "\n";
    
    // 测试版本验证
    bool valid1 = utils::isValidProtocolVersion(PROTOCOL_VERSION);
    bool valid2 = utils::isValidProtocolVersion("2024-11-05");
    bool valid3 = utils::isValidProtocolVersion("invalid");
    
    std::cout << "当前版本是否有效: " << (valid1 ? "是" : "否") << "\n";
    std::cout << "2024-11-05 是否有效: " << (valid2 ? "是" : "否") << "\n";
    std::cout << "invalid 是否有效: " << (valid3 ? "是" : "否") << "\n";
}

void test_tool_from_schema() {
    print_separator("测试从 Schema 创建 Tool");
    
    json schema = {
        {"type", "object"},
        {"properties", {
            {"query", {
                {"type", "string"},
                {"description", "搜索查询"}
            }}
        }},
        {"required", {"query"}}
    };
    
    Tool tool = utils::createToolFromSchema(
        "search",
        "搜索工具",
        schema
    );
    
    json tool_json = tool.toJson();
    std::cout << "从 Schema 创建的 Tool JSON:\n";
    std::cout << tool_json.dump(2) << "\n";
}

int main() {
    std::cout << "MCP Protocol 示例程序\n";
    std::cout << "====================\n";
    
    try {
        test_protocol_version();
        test_client_capabilities();
        test_server_capabilities();
        test_initialize();
        test_tool();
        test_tool_call();
        test_resource();
        test_prompt();
        test_tool_from_schema();
        
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "所有测试完成！\n";
        std::cout << std::string(60, '=') << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
