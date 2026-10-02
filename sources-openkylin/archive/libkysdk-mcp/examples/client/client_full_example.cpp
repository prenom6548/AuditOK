/**
 * @file client_full_example.cpp
 * @brief MCP Client 全能力展示示例
 *
 * 本示例展示 MCPClient 的所有客户端能力，包括:
 *   1. 连接管理 (connect / disconnect / 状态查询)
 *   2. 协议初始化 (initialize / initialized / ping)
 *   3. 工具操作 (listTools / callTool)
 *   4. 资源操作 (listResources / readResource)
 *   5. 提示操作 (listPrompts / getPrompt)
 *   6. 通知处理 (setNotificationHandler / sendNotification)
 *   7. 服务器信息查询 (getServerInfo / getServerCapabilities)
 *   8. 多端点管理 (setEndpoints / setCurrentEndpoint)
 *   9. 传输类型与状态查询
 *  10. HTTP/SSE 与 IPC 双传输模式支持
 *
 * 配合 mcp_server_full_example 使用以验证完整交互流程。
 *
 * 用法:
 *   ./mcp_client_full_example [--host <host>] [--port <port>] [--ipc <command>]
 */

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "client.h"
#include "common/transport.h"
using namespace mcp;
using namespace mcp_transport;

struct NotificationState {
    std::atomic<int> tools_changed{0};
    std::atomic<int> resources_changed{0};
    std::atomic<int> prompts_changed{0};
    std::atomic<int> progress{0};
    std::atomic<int> roots_changed{0};
    std::atomic<int> cancelled{0};
};

static std::atomic<int> g_failures{0};
static NotificationState g_notifications;
static constexpr const char* kColorGreen = "\033[32m";
static constexpr const char* kColorRed = "\033[31m";
static constexpr const char* kColorReset = "\033[0m";

// ---------------------------------------------------------------------------
// 辅助
// ---------------------------------------------------------------------------
static void section(const std::string& title) {
    std::cout << "\n========== " << title << " ==========" << std::endl;
}

static std::string greenTag(const std::string& tag) {
    return std::string(kColorGreen) + tag + kColorReset;
}

static std::string redTag(const std::string& tag) {
    return std::string(kColorRed) + tag + kColorReset;
}

static void ok(const std::string& msg) {
    std::cout << "  " << greenTag("[OK]") << " " << msg << std::endl;
}
static void fail(const std::string& msg) {
    ++g_failures;
    std::cerr << "  " << redTag("[FAIL]") << " " << msg << std::endl;
}
static void info(const std::string& msg) {
    std::cout << "  " << msg << std::endl;
}

static json extract_system_info_payload(const ToolCallResult& result) {
    if (result.isError) {
        throw std::runtime_error("get_system_info returned error: " +
                                 result.errorMessage);
    }
    if (result.content.empty() || !result.content[0].contains("text")) {
        throw std::runtime_error("get_system_info returned unexpected content");
    }
    return json::parse(result.content[0]["text"].get<std::string>());
}

static std::string transport_status_str(TransportStatus s) {
    switch (s) {
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

static std::vector<std::string> splitCommandLine(
    const std::string& command_line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '\0';
    bool escaping = false;

    for (char ch : command_line) {
        if (escaping) {
            current.push_back(ch);
            escaping = false;
            continue;
        }

        if (ch == '\\') {
            escaping = true;
            continue;
        }

        if (in_quotes) {
            if (ch == quote_char) {
                in_quotes = false;
            } else {
                current.push_back(ch);
            }
            continue;
        }

        if (ch == '"' || ch == '\'') {
            in_quotes = true;
            quote_char = ch;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

// ---------------------------------------------------------------------------
// 各能力测试函数
// ---------------------------------------------------------------------------

// 1. 协议初始化
static bool testInitialize(MCPClient& client) {
    section("1. 协议初始化 (initialize / initialized)");
    try {
        // 设置客户端信息
        client.setClientInfo("mcp-client-full-example", "1.0.0");

        // 设置客户端能力
        ClientCapabilities caps = mcp::utils::createDefaultClientCapabilities();
        client.setCapabilities(caps);
        info("客户端能力: " + caps.toJson().dump());

        // 执行初始化
        InitializeParams params;
        params.protocolVersion = PROTOCOL_VERSION;
        params.clientInfo.name = "mcp-client-full-example";
        params.clientInfo.version = "1.0.0";
        params.capabilities = caps;

        auto result = client.initialize(params);
        ok("初始化成功");
        info("协议版本: " + result.protocolVersion);
        info("服务器: " + result.serverInfo.name + " v" +
             result.serverInfo.version);
        info("服务器能力: " + result.capabilities.toJson().dump(2));

        // 发送初始化完成通知
        client.initialized();
        ok("initialized 通知已发送");
        return true;
    } catch (const std::exception& e) {
        fail(std::string("初始化失败: ") + e.what());
        return false;
    }
}

// 2. Ping 测试
static void testPing(MCPClient& client) {
    section("2. Ping 测试");
    try {
        client.ping();
        ok("Ping 成功");
    } catch (const std::exception& e) {
        fail(std::string("Ping 失败: ") + e.what());
    }
}

static std::string pickAvailableTool(
    const std::vector<Tool>& tools,
    const std::vector<std::string>& candidates) {
    for (const auto& candidate : candidates) {
        auto it = std::find_if(
            tools.begin(), tools.end(),
            [&candidate](const Tool& tool) { return tool.name == candidate; });
        if (it != tools.end()) {
            return candidate;
        }
    }
    return "";
}

// 3. 工具操作
static void testTools(MCPClient& client) {
    section("3. 工具操作 (listTools / callTool)");

    std::vector<Tool> tools;

    // 3.1 列出工具
    try {
        tools = client.listTools().tools;
        ok("获取工具列表成功, 共 " + std::to_string(tools.size()) + " 个工具:");
        for (const auto& tool : tools) {
            info("  - " + tool.name + ": " + tool.description);
            info("    inputSchema: " + tool.inputSchema.dump());
        }
    } catch (const std::exception& e) {
        fail(std::string("获取工具列表失败: ") + e.what());
        return;
    }

    // 3.2 调用 echo 工具
    std::cout << std::endl;
    info("--- 调用 echo 工具 ---");
    try {
        auto result = client.callTool("echo", {{"text", "Hello MCP!"}});
        if (result.isError) {
            fail("echo 工具返回错误: " + result.errorMessage);
        } else {
            ok("echo 调用成功: " + result.content.dump());
        }
    } catch (const std::exception& e) {
        fail(std::string("echo 调用失败: ") + e.what());
    }

    // 3.3 调用 calculate 工具 — 正常计算
    std::cout << std::endl;
    info("--- 调用 calculate 工具 (正常) ---");
    try {
        json args = {{"op", "mul"}, {"a", 6}, {"b", 7}};
        auto result = client.callTool("calculate", args);
        if (result.isError) {
            fail("calculate 返回错误: " + result.errorMessage);
        } else {
            ok("calculate 调用成功: " + result.content.dump());
        }
    } catch (const std::exception& e) {
        fail(std::string("calculate 调用失败: ") + e.what());
    }

    // 3.4 调用 calculate 工具 — 除零错误
    std::cout << std::endl;
    info("--- 调用 calculate 工具 (除零错误) ---");
    try {
        json args = {{"op", "div"}, {"a", 10}, {"b", 0}};
        auto result = client.callTool("calculate", args);
        if (result.isError) {
            ok("预期的错误: " + result.errorMessage);
        } else {
            info("calculate 返回结果: " + result.content.dump());
        }
    } catch (const std::exception& e) {
        fail(std::string("calculate 调用失败: ") + e.what());
    }

    // 3.5 调用系统信息工具（若服务端支持）
    std::cout << std::endl;
    info("--- 调用系统信息工具 ---");
    const std::string system_info_tool = pickAvailableTool(
        tools, {"get_system_info", "system_info", "get_status"});
    if (system_info_tool.empty()) {
        info("跳过: 当前服务端未提供系统信息工具");
    } else {
        try {
            auto result = client.callTool(system_info_tool);
            if (result.isError) {
                fail(system_info_tool + " 返回错误: " + result.errorMessage);
            } else {
                ok(system_info_tool + " 调用成功: " + result.content.dump(2));
            }
        } catch (const std::exception& e) {
            fail(system_info_tool + " 调用失败: " + std::string(e.what()));
        }
    }

    // 3.6 调用 long_running_task (带进度通知)
    std::cout << std::endl;
    info("--- 调用 long_running_task 工具 (3步, 含进度通知) ---");
    try {
        json args = {{"steps", 3}, {"progress_token", "task-001"}};
        auto result = client.callTool("long_running_task", args);
        if (result.isError) {
            fail("long_running_task 返回错误: " + result.errorMessage);
        } else {
            ok("long_running_task 调用成功: " + result.content.dump());
        }
    } catch (const std::exception& e) {
        fail(std::string("long_running_task 调用失败: ") + e.what());
    }

    // 3.7 使用 ToolCallParams 结构体调用
    std::cout << std::endl;
    info("--- 使用 ToolCallParams 结构体调用 ---");
    try {
        ToolCallParams params;
        params.name = "echo";
        params.arguments = {{"text", "通过 ToolCallParams 调用"}};
        auto result = client.callTool(params);
        ok("ToolCallParams 调用成功: " + result.content.dump());
    } catch (const std::exception& e) {
        fail(std::string("ToolCallParams 调用失败: ") + e.what());
    }
}

// 4. 资源操作
static void testResources(MCPClient& client) {
    section("4. 资源操作 (listResources / readResource)");

    // 4.1 列出资源
    std::vector<Resource> resources;
    try {
        resources = client.listResources().resources;
        ok("获取资源列表成功, 共 " + std::to_string(resources.size()) +
           " 个资源:");
        for (const auto& res : resources) {
            info("  - " + res.name + " (" + res.uri + ")");
            info("    描述: " + res.description);
            info("    MIME: " + res.mimeType);
        }
    } catch (const std::exception& e) {
        fail(std::string("获取资源列表失败: ") + e.what());
        return;
    }

    // 4.2 列出资源模板
    try {
        const auto templates = client.listResourceTemplates().resourceTemplates;
        if (templates.empty()) {
            fail("资源模板列表为空，期望至少 1 个模板");
            return;
        }
        ok("获取资源模板列表成功, 共 " + std::to_string(templates.size()) +
           " 个模板");
        for (const auto& tmpl : templates) {
            const std::string name =
                tmpl.name.empty() ? "<unnamed>" : tmpl.name;
            const std::string uri_template =
                tmpl.uriTemplate.empty() ? "<missing-uriTemplate>"
                                         : tmpl.uriTemplate;
            info("  - 模板: " + name + " (" + uri_template + ")");
        }
    } catch (const std::exception& e) {
        fail(std::string("获取资源模板列表失败: ") + e.what());
        return;
    }

    // 4.3 读取每个资源
    for (const auto& res : resources) {
        std::cout << std::endl;
        info("--- 读取资源: " + res.uri + " ---");
        try {
            auto content = client.readResource(res.uri);
            ok("读取成功:");
            info(content.toJson().dump(2));
        } catch (const std::exception& e) {
            fail(std::string("读取失败: ") + e.what());
        }
    }
}

static void testProtocol2025Methods(MCPClient& client) {
    section("5. 2025-11-25 扩展方法");

    try {
        client.subscribeResource("status://server");
        ok("resources/subscribe 调用成功");
    } catch (const std::exception& e) {
        fail(std::string("resources/subscribe 调用失败: ") + e.what());
    }

    try {
        client.unsubscribeResource("status://server");
        ok("resources/unsubscribe 调用成功");
    } catch (const std::exception& e) {
        fail(std::string("resources/unsubscribe 调用失败: ") + e.what());
    }

    try {
        auto tasks = client.listTasks();
        ok("tasks/list 调用成功, 条目数: " + std::to_string(tasks.tasks.size()));
    } catch (const std::exception& e) {
        fail(std::string("tasks/list 调用失败: ") + e.what());
    }

    try {
        auto task = client.getTask("task-demo-001");
        if (task.taskId == "task-demo-001") {
            ok("tasks/get 调用成功");
        } else {
            fail("tasks/get 返回结构不符合预期");
        }
    } catch (const std::exception& e) {
        fail(std::string("tasks/get 调用失败: ") + e.what());
    }

    try {
        auto task_result = client.getTaskResult("task-demo-completed");
        if (task_result.taskId == "task-demo-completed") {
            ok("tasks/result 调用成功");
        } else {
            fail("tasks/result 返回结构不符合预期");
        }
    } catch (const std::exception& e) {
        fail(std::string("tasks/result 调用失败: ") + e.what());
    }

    try {
        auto cancel_result = client.cancelTask("task-demo-001");
        if (cancel_result.status == TaskStatus::kCancelled) {
            ok("tasks/cancel 调用成功");
        } else {
            fail("tasks/cancel 返回结构不符合预期");
        }
    } catch (const std::exception& e) {
        fail(std::string("tasks/cancel 调用失败: ") + e.what());
    }

    ok("elicitation/create 为 server->client 请求，"
       "客户端已通过 setElicitationHandler 注册处理器");

    try {
        auto completion_result =
            client.complete({{"ref", json::object()},
                             {"argument", json{{"name", "tool"},
                                                 {"value", "ec"}}}});
        ok("completion/complete 调用成功, 建议数: " +
           std::to_string(completion_result.values.size()));
    } catch (const std::exception& e) {
        fail(std::string("completion/complete 调用失败: ") + e.what());
    }

    ok("sampling/createMessage 为 server->client 请求，"
       "客户端已通过 setSamplingHandler 注册处理器");
}

// 5. 提示操作
static void testPrompts(MCPClient& client) {
    section("5. 提示操作 (listPrompts / getPrompt)");

    // 5.1 列出提示
    std::vector<Prompt> prompts;
    try {
        prompts = client.listPrompts().prompts;
        ok("获取提示列表成功, 共 " + std::to_string(prompts.size()) +
           " 个提示:");
        for (const auto& p : prompts) {
            std::string args_str;
            for (size_t i = 0; i < p.arguments.size(); ++i) {
                if (i > 0) args_str += ", ";
                args_str += p.arguments[i];
            }
            info("  - " + p.name + ": " + p.description +
                 " (参数: " + args_str + ")");
        }
    } catch (const std::exception& e) {
        fail(std::string("获取提示列表失败: ") + e.what());
        return;
    }

    auto pick_prompt =
        [&prompts](const std::vector<std::string>& candidates) -> std::string {
        for (const auto& name : candidates) {
            auto it = std::find_if(
                prompts.begin(), prompts.end(),
                [&name](const Prompt& prompt) { return prompt.name == name; });
            if (it != prompts.end()) {
                return name;
            }
        }
        return "";
    };

    auto fetch_prompt = [&client, &pick_prompt](
                            const std::string& title,
                            const std::vector<std::string>& candidates,
                            const json& args) {
        std::cout << std::endl;
        info("--- " + title + " ---");

        const std::string prompt_name = pick_prompt(candidates);
        if (prompt_name.empty()) {
            info("跳过: 服务端未提供候选提示 " + json(candidates).dump());
            return;
        }

        try {
            auto result = client.getPrompt(prompt_name, args);
            ok("获取成功 (" + prompt_name + "): " + result.toJson().dump(2));
        } catch (const std::exception& e) {
            fail(std::string("获取失败 (") + prompt_name + "): " + e.what());
        }
    };

    // 5.2 获取 greeting 提示 — friendly 风格
    fetch_prompt("获取 greeting 提示 (friendly)", {"greeting", "greet"},
                 {{"name", "开发者"}, {"style", "friendly"}});

    // 5.3 获取 greeting 提示 — formal 风格
    fetch_prompt("获取 greeting 提示 (formal)", {"greeting", "greet"},
                 {{"name", "张总"}, {"style", "formal"}});

    // 5.4 获取 summarize 提示
    fetch_prompt("获取 summarize 提示", {"summarize", "summary"},
                 {{"text", "MCP 是一种用于 AI 模型与外部工具交互的协议。"},
                  {"max_length", 50}});

    // 5.5 获取 code_review 提示
    fetch_prompt("获取 code_review 提示", {"code_review", "review_code"},
                 {{"code", "int main() { return 0; }"}, {"language", "C++"}});
}

// 6. 通知处理
static void testNotifications(MCPClient& client) {
    section("6. 通知处理 (setNotificationHandler / sendNotification)");

    // 6.1 注册各种通知处理器
    client.setNotificationHandler(
        methods::NOTIFICATION_TOOLS_LIST_CHANGED,
        [](const std::string& method, const json& /*params*/) {
            ++g_notifications.tools_changed;
            std::cout << "  [通知] 工具列表已变更: " << method << std::endl;
        });
    ok("已注册 tools/list_changed 通知处理器");

    client.setNotificationHandler(
        methods::NOTIFICATION_RESOURCES_LIST_CHANGED,
        [](const std::string& method, const json& /*params*/) {
            ++g_notifications.resources_changed;
            std::cout << "  [通知] 资源列表已变更: " << method << std::endl;
        });
    ok("已注册 resources/list_changed 通知处理器");

    client.setNotificationHandler(
        methods::NOTIFICATION_PROMPTS_LIST_CHANGED,
        [](const std::string& method, const json& /*params*/) {
            ++g_notifications.prompts_changed;
            std::cout << "  [通知] 提示列表已变更: " << method << std::endl;
        });
    ok("已注册 prompts/list_changed 通知处理器");

    client.setNotificationHandler(
        methods::NOTIFICATION_PROGRESS,
        [](const std::string&, const json& params) {
            ++g_notifications.progress;
            double progress = params.value("progress", 0.0);
            double total = params.value("total", 0.0);
            std::string message = params.value("message", "");
            std::cout << "  [进度] " << message << " (" << progress << "/"
                      << total << ")" << std::endl;
        });
    ok("已注册 progress 通知处理器");

    client.setNotificationHandler(
        methods::NOTIFICATION_ROOTS_LIST_CHANGED,
        [](const std::string& method, const json& /*params*/) {
            ++g_notifications.roots_changed;
            std::cout << "  [通知] Roots 列表已变更: " << method << std::endl;
        });
    ok("已注册 roots/list_changed 通知处理器");

    client.setNotificationHandler(
        methods::NOTIFICATION_CANCELLED,
        [](const std::string& method, const json& params) {
            ++g_notifications.cancelled;
            std::cout << "  [通知] 收到取消通知: " << method
                      << " params=" << params.dump() << std::endl;
        });
    ok("已注册 cancelled 通知处理器");

    // 6.2 发送自定义通知
    std::cout << std::endl;
    info("--- 发送自定义通知 ---");
    try {
        client.sendNotification("custom/client_ready",
                                {{"client", "mcp-client-full-example"}});
        ok("自定义通知 custom/client_ready 已发送");
    } catch (const std::exception& e) {
        fail(std::string("发送通知失败: ") + e.what());
    }
}

static void testHandlerRegistration(MCPClient& client) {
    section("6.1 扩展处理器注册覆盖");

    client.setToolCallHandler(
        [](const ToolCallParams& params) -> ToolCallResult {
            ToolCallResult result;
            result.content = {
                {{"type", "text"},
                 {"text", "local handler placeholder: " + params.name}}};
            return result;
        });
    ok("setToolCallHandler 已设置");

    client.setResourceReadHandler([](const std::string& uri) -> ReadResourceResult {
        ReadResourceResult result;
        ResourceContent content;
        content.uri = uri;
        content.text = "local resource placeholder";
        result.contents.push_back(content);
        return result;
    });
    ok("setResourceReadHandler 已设置");

    client.setPromptGetHandler(
        [](const std::string& name, const json&) -> PromptResult {
            PromptResult result;
            PromptMessage message;
            message.role = Role::kAssistant;
            message.content = {{"type", "text"},
                               {"text", "local prompt placeholder: " + name}};
            result.messages.push_back(message);
            return result;
        });
    ok("setPromptGetHandler 已设置");

    client.setElicitationHandler([](const json& params) -> ElicitCreateResult {
        std::cout << "  [请求] 收到 server->client elicitation/create: "
                  << params.dump() << std::endl;
        ElicitCreateResult result;
        result.action = "accept";
        result.content = {{"handled_by", "mcp_client_full_example"},
                          {"mode", params.value("mode", std::string("form"))},
                          {"message", params.value("message", std::string(""))}};
        return result;
    });
    ok("setElicitationHandler 已设置");

    client.setSamplingHandler([](const json&) -> CreateMessageResult {
        std::cout << "  [请求] 收到 server->client sampling/createMessage"
                  << std::endl;
        CreateMessageResult result;
        result.role = Role::kAssistant;
        result.content = json::array(
            {json{{"type", "text"}, {"text", "sample ok from client"}}});
        result.model = "client-demo-model";
        result.stopReason = "endTurn";
        return result;
    });
    ok("setSamplingHandler 已设置");
}

// 7. 服务器信息查询
static void testServerInfo(MCPClient& client) {
    section("7. 服务器信息查询");

    try {
        auto server_info = client.getServerInfo();
        ok("服务器名称: " + server_info.name);
        ok("服务器版本: " + server_info.version);
    } catch (const std::exception& e) {
        fail(std::string("获取服务器信息失败: ") + e.what());
    }

    try {
        auto caps = client.getServerCapabilities();
        ok("服务器能力: " + caps.toJson().dump(2));
    } catch (const std::exception& e) {
        fail(std::string("获取服务器能力失败: ") + e.what());
    }
}

// 8. 传输状态查询
static void testTransportStatus(MCPClient& client) {
    section("8. 传输状态查询");

    auto type = client.getTransportType();
    auto status = client.getTransportStatus();
    bool connected = client.isConnected();

    info("传输类型: " + std::to_string(static_cast<int>(type)) +
         (type == TransportType::kSse
              ? " (SSE)"
              : type == TransportType::kIpc ? " (IPC)" : " (AUTO)"));
    info("传输状态: " + transport_status_str(status));
    info("连接状态: " + std::string(connected ? "已连接" : "未连接"));
}

// 9. 多端点管理
static void testEndpoints(MCPClient& client) {
    section("9. 多端点管理");

    // 显示当前端点
    auto current = client.getCurrentEndpoint();
    auto endpoints = client.getEndpoints();
    info("当前端点: " + current);
    info("端点列表 (" + std::to_string(endpoints.size()) + " 个):");
    for (const auto& ep : endpoints) {
        info("  - " + ep + (ep == current ? " (当前)" : ""));
    }

    // 设置多个端点
    std::cout << std::endl;
    info("--- 设置多端点 ---");
    std::vector<std::string> new_endpoints = {"/mcp", "/mcp/v2", "/mcp/backup"};
    client.setEndpoints(new_endpoints);
    ok("设置端点: " + std::to_string(new_endpoints.size()) + " 个");

    // 切换端点
    for (const auto& ep : new_endpoints) {
        client.setCurrentEndpoint(ep);
        info("切换到端点: " + client.getCurrentEndpoint());
    }

    // 恢复原始端点
    client.setCurrentEndpoint(current);
    ok("恢复到原始端点: " + current);
}

// 10. 等待服务器动态变更并重新查询
static void testDynamicChanges(MCPClient& client) {
    section("10. 等待并验证服务器动态变更");
    info("等待服务器进行动态注册/注销操作 (约 10 秒)...");
    info("期间可能收到变更通知");

    std::this_thread::sleep_for(std::chrono::seconds(10));

    // 重新查询
    info("--- 动态变更后重新查询 ---");
    try {
        auto tools = client.listTools();
        info("当前工具数: " + std::to_string(tools.tools.size()));
        for (const auto& t : tools.tools) info("  - " + t.name);
    } catch (...) {
    }

    try {
        auto resources = client.listResources();
        info("当前资源数: " + std::to_string(resources.resources.size()));
        for (const auto& r : resources.resources) info("  - " + r.uri);
    } catch (...) {
    }

    try {
        auto prompts = client.listPrompts();
        info("当前提示数: " + std::to_string(prompts.prompts.size()));
        for (const auto& p : prompts.prompts) info("  - " + p.name);
    } catch (...) {
    }
}

static void testNegativeCases(MCPClient& client) {
    section("11. 负向与异常路径测试");

    try {
        auto result = client.callTool("missing_tool");
        if (result.isError) {
            ok("未知工具返回预期错误: " + result.errorMessage);
        } else {
            fail("未知工具未返回错误");
        }
    } catch (const std::exception& e) {
        fail(std::string("未知工具调用抛出异常: ") + e.what());
    }

    try {
        (void)client.readResource("missing://resource");
        fail("未知资源读取未抛出异常");
    } catch (const std::exception& e) {
        ok(std::string("未知资源读取返回预期异常: ") + e.what());
    }

    try {
        (void)client.getPrompt("missing_prompt");
        fail("未知提示获取未抛出异常");
    } catch (const std::exception& e) {
        ok(std::string("未知提示获取返回预期异常: ") + e.what());
    }
}

static void testControlNotifications(MCPClient& client) {
    section("12. 控制通知闭环测试");

    try {
        client.sendNotification(methods::NOTIFICATION_CANCELLED,
                                {{"requestId", "manual-cancel-001"},
                                 {"reason", "regression-check"}});
        ok("已发送 notifications/cancelled 通知");
    } catch (const std::exception& e) {
        fail(std::string("发送 cancel 通知失败: ") + e.what());
    }

    try {
        client.sendNotification(methods::NOTIFICATION_ROOTS_LIST_CHANGED,
                                json::object());
        ok("已发送 notifications/roots/list_changed 通知");
    } catch (const std::exception& e) {
        fail(std::string("发送 roots/list_changed 通知失败: ") + e.what());
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    try {
        const auto tools = client.listTools().tools;
        const std::string system_info_tool = pickAvailableTool(
            tools, {"get_system_info", "system_info", "get_status"});

        if (system_info_tool.empty()) {
            info("跳过闭环状态校验: 当前服务端未提供系统状态工具");
            return;
        }

        auto result = client.callTool(system_info_tool);
        json payload = extract_system_info_payload(result);
        json state = payload.value("runtime_state", json::object());

        if (state.value("cancel_count", 0) >= 1) {
            ok("服务端已记录 cancel 通知");
        } else {
            fail("服务端未记录 cancel 通知");
        }

        if (state.value("roots_changed_count", 0) >= 1) {
            ok("服务端已记录 roots/list_changed 通知");
        } else {
            fail("服务端未记录 roots/list_changed 通知");
        }
    } catch (const std::exception& e) {
        fail(std::string("控制通知闭环校验失败: ") + e.what());
    }
}

static void testConnectOverload(const std::string& ipc_command) {
    section("13. connect(endpoint) 重载覆盖");

    if (ipc_command.empty()) {
        info("当前为 HTTP/SSE 模式，跳过 connect(endpoint) 重载测试");
        return;
    }

    MCPClient probe;
    probe.setTransportType(TransportType::kIpc);
    if (!probe.connect("cmd:" + ipc_command)) {
        fail("connect(endpoint) IPC 重载连接失败");
        return;
    }
    ok("connect(endpoint) IPC 重载连接成功");

    try {
        InitializeParams params;
        params.protocolVersion = PROTOCOL_VERSION;
        params.clientInfo.name = "mcp-client-connect-overload";
        params.clientInfo.version = "1.0.0";
        params.capabilities = mcp::utils::createDefaultClientCapabilities();

        auto result = probe.initialize(params);
        ok("connect(endpoint) 重载后的 initialize 成功: " +
           result.serverInfo.name);
    } catch (const std::exception& e) {
        fail(std::string("connect(endpoint) 重载后的 initialize 失败: ") +
             e.what());
    }

    probe.disconnect();
    ok("connect(endpoint) 重载测试已断开");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8080;
    std::string endpoint = "/mcp";
    std::string ipc_command;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc)
            host = argv[++i];
        else if (arg == "--port" && i + 1 < argc)
            port = std::stoi(argv[++i]);
        else if (arg == "--endpoint" && i + 1 < argc)
            endpoint = argv[++i];
        else if (arg == "--ipc" && i + 1 < argc)
            ipc_command = argv[++i];
        else if (arg == "--help") {
            std::cout << "用法: " << argv[0] << " [选项]\n"
                      << "  --host <host>       服务器地址 (默认 127.0.0.1)\n"
                      << "  --port <port>       服务器端口 (默认 8080)\n"
                      << "  --endpoint <path>   HTTP端点 (默认 /mcp)\n"
                      << "  --ipc <command>     使用IPC模式并指定服务器命令\n"
                      << "  --help              显示帮助\n";
            return 0;
        }
    }

    bool use_ipc = !ipc_command.empty();

    std::cout << "MCP Client 全能力展示示例" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "传输模式: " << (use_ipc ? "IPC" : "HTTP/SSE") << std::endl;
    if (use_ipc) {
        std::cout << "IPC命令: " << ipc_command << std::endl;
    } else {
        std::cout << "服务器: " << host << ":" << port << endpoint << std::endl;
    }

    try {
        MCPClient client;

        // ----- 建立连接 -----
        section("建立连接");

        TransportConfig config;
        if (use_ipc) {
            const auto command_parts = splitCommandLine(ipc_command);
            if (command_parts.empty()) {
                fail("IPC 命令为空");
                return 1;
            }

            const std::string& command = command_parts.front();
            std::vector<std::string> command_args;
            if (command_parts.size() > 1) {
                command_args.assign(command_parts.begin() + 1,
                                    command_parts.end());
            }
            config =
                mcp_transport::utils::createIpcConfig(command, command_args);
            config.ipc.read_timeout_ms = 2000;
            config.ipc.enable_logging = true;
            client.setTransportType(TransportType::kIpc);
        } else {
            config.type = TransportType::kSse;
            config.sse.host = host;
            config.sse.port = port;
            config.sse.protocol = TransportProtocol::kHttp;
            config.sse.timeout_seconds = 30;
            config.sse.max_retries = 3;
            config.sse.keep_alive = true;
            config.auto_reconnect = true;
            config.reconnect_interval = 5;
            config.sse.endpoint = endpoint;
            config.sse.headers["Content-Type"] =
                "application/json; charset=utf-8";
            config.sse.headers["Accept"] =
                "application/json, text/event-stream";
            client.setTransportType(TransportType::kSse);
        }
        config.enable_logging = true;

        // 设置端点
        client.setEndpoints({endpoint});
        client.setRoots(
            {Root{"file://" + std::filesystem::current_path().string(),
                  "workspace"},
             Root{"file://" +
                      (std::filesystem::current_path() / "examples").string(),
                  "examples"}});

        if (!client.connectWithConfig(config)) {
            fail("连接失败");
            return 1;
        }
        ok("连接成功");

        // Poll for readiness instead of relying on a fixed sleep.
        const auto ready_deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (!client.isConnected() &&
               std::chrono::steady_clock::now() < ready_deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!client.isConnected()) {
            fail("连接未就绪");
            return 1;
        }
        ok("连接就绪");

        // ----- 注册通知处理器 (需要在 initialize 之前) -----
        testNotifications(client);
        testHandlerRegistration(client);

        // ----- 执行所有测试 -----
        if (!testInitialize(client)) {
            fail("初始化失败，终止测试");
            client.disconnect();
            return 1;
        }

        testPing(client);
        testServerInfo(client);
        testTransportStatus(client);
        testTools(client);
        testResources(client);
        testProtocol2025Methods(client);
        testPrompts(client);
        testEndpoints(client);
        testDynamicChanges(client);
        testNegativeCases(client);
        testControlNotifications(client);

        // ----- 连接稳定性测试 -----
        section("14. 连接稳定性测试");
        for (int i = 1; i <= 5; ++i) {
            try {
                client.ping();
                ok("Ping " + std::to_string(i) + "/5 成功");
            } catch (const std::exception& e) {
                fail("Ping " + std::to_string(i) +
                     "/5 失败: " + std::string(e.what()));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        testConnectOverload(ipc_command);

        // ----- 断开连接 -----
        section("断开连接");
        client.disconnect();
        ok("已断开连接");
        info("连接状态: " +
             std::string(client.isConnected() ? "已连接" : "已断开"));

    } catch (const std::exception& e) {
        std::cerr << redTag("[ERROR]") << " " << e.what() << std::endl;
        return 1;
    }

    if (g_failures.load() > 0) {
        std::cout << "\n"
                  << redTag("[FAIL]")
                  << " 客户端回归失败数: " << g_failures.load() << std::endl;
        return 1;
    }

    std::cout << "\n[完成] MCP Client 全能力展示示例执行完毕" << std::endl;
    return 0;
}
