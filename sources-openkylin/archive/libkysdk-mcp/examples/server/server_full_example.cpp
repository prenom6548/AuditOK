/**
 * @file server_full_example.cpp
 * @brief MCP Server 全能力展示示例
 *
 * 本示例展示 MCPServer 的所有服务端能力，包括:
 *   1. 服务器信息与能力声明
 *   2. 工具(Tool)注册、注销、列表、变更通知
 *   3. 资源(Resource)注册、注销、列表、变更通知
 *   4. 提示(Prompt)注册、注销、列表、变更通知
 *   5. 进度通知(Progress)
 *   6. 取消处理(Cancel)
 *   7. 根目录查询(Roots: server->client)
 *   8. HTTP/SSE 与 IPC 双传输模式支持
 *
 * 配合 mcp_client_full_example 使用以验证完整交互流程。
 *
 * 用法:
 *   ./mcp_server_full_example [--port <port>] [--ipc]
 */

#include <signal.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include "logger.h"
#include "server.h"
#include "common/transport.h"

using namespace mcp;
using namespace mcp_transport;

// ---------------------------------------------------------------------------
// 全局停止标志
// ---------------------------------------------------------------------------
static std::atomic<bool> g_running{true};
static std::atomic<int> g_failures{0};
static constexpr const char* kColorGreen = "\033[32m";
static constexpr const char* kColorRed = "\033[31m";
static constexpr const char* kColorReset = "\033[0m";

struct RegressionState {
    std::mutex mutex;
    int cancel_count = 0;
    int roots_changed_count = 0;
    int last_roots_count = 0;
    std::string last_cancel_request_id;
    std::string last_cancel_reason;
};

static RegressionState g_state;

void signalHandler(int) { g_running = false; }

// ---------------------------------------------------------------------------
// 辅助: 打印分隔线
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

static void check(bool condition, const std::string& message) {
    if (condition) {
        std::cout << "  " << greenTag("[OK]") << " " << message << std::endl;
    } else {
        ++g_failures;
        std::cout << "  " << redTag("[FAIL]") << " " << message << std::endl;
    }
}

static void resetRegressionState() {
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.cancel_count = 0;
    g_state.roots_changed_count = 0;
    g_state.last_roots_count = 0;
    g_state.last_cancel_request_id.clear();
    g_state.last_cancel_reason.clear();
}

static ReadResourceResult makeTextResourceResult(const std::string& uri,
                                                 const std::string& text,
                                                 const std::string& mime_type =
                                                     "text/plain") {
    ReadResourceResult result;
    ResourceContent content;
    content.uri = uri;
    content.mimeType = mime_type;
    content.text = text;
    result.contents.push_back(content);
    return result;
}

static PromptResult makePromptTextResult(Role role, const std::string& text) {
    PromptResult result;
    PromptMessage message;
    message.role = role;
    message.content = {{"type", "text"}, {"text", text}};
    result.messages.push_back(message);
    return result;
}

static void runRegistrySelfTests(MCPServer& server) {
    section("预检: 注册表与清理接口自检");

    Tool tool;
    tool.name = "preflight_tool";
    tool.description = "临时工具";
    tool.inputSchema = {{"type", "object"}, {"properties", json::object()}};
    check(server.registerTool(
              tool,
              [](const ToolCallParams&) -> ToolCallResult {
                  ToolCallResult result;
                  result.content = {{{"type", "text"}, {"text", "preflight"}}};
                  return result;
              }),
          "registerTool 可注册新工具");
    check(!server.registerTool(tool,
                               [](const ToolCallParams&) -> ToolCallResult {
                                   return ToolCallResult{};
                               }),
          "registerTool 会拒绝重复名称");
    check(server.unregisterTool("preflight_tool"),
          "unregisterTool 可移除已注册工具");
    check(!server.unregisterTool("preflight_tool"),
          "unregisterTool 对不存在工具返回 false");
    server.registerTool(tool, [](const ToolCallParams&) -> ToolCallResult {
        ToolCallResult result;
        result.content = {{{"type", "text"}, {"text", "preflight"}}};
        return result;
    });
    server.clearTools();
    check(server.listTools().empty(), "clearTools 会清空工具列表");

    Resource resource;
    resource.uri = "preflight://resource";
    resource.name = "临时资源";
    resource.description = "临时资源描述";
    resource.mimeType = "text/plain";
    check(
        server.registerResource(resource,
                                [](const std::string&) -> ReadResourceResult {
                                    return makeTextResourceResult(
                                        "preflight://resource", "preflight");
                                }),
        "registerResource 可注册新资源");
    check(!server.registerResource(
              resource,
              [](const std::string&) -> ReadResourceResult {
                  return ReadResourceResult{};
              }),
          "registerResource 会拒绝重复 URI");
    check(server.unregisterResource("preflight://resource"),
          "unregisterResource 可移除已注册资源");
    check(!server.unregisterResource("preflight://resource"),
          "unregisterResource 对不存在资源返回 false");
    server.registerResource(resource, [](const std::string&) -> ReadResourceResult {
        return makeTextResourceResult("preflight://resource", "preflight");
    });
    server.clearResources();
    check(server.listResources().empty(), "clearResources 会清空资源列表");

    ResourceTemplate resource_template;
    resource_template.uriTemplate = "preflight://template/{name}";
    resource_template.name = "preflight-template";
    resource_template.description = "临时模板";
    resource_template.mimeType = "text/plain";
    check(server.registerResourceTemplate(resource_template),
          "registerResourceTemplate 可注册新模板");
    check(!server.registerResourceTemplate(resource_template),
          "registerResourceTemplate 会拒绝重复 uriTemplate");
    check(server.unregisterResourceTemplate("preflight://template/{name}"),
          "unregisterResourceTemplate 可移除已注册模板");
    check(!server.unregisterResourceTemplate("preflight://template/{name}"),
          "unregisterResourceTemplate 对不存在模板返回 false");
    server.registerResourceTemplate(resource_template);
    {
        const auto templates = server.listResourceTemplates();
        check(templates.size() == 1,
              "resources/templates/list 能返回已注册模板");
    }
    server.clearResourceTemplates();
    {
        const auto templates = server.listResourceTemplates();
        check(templates.empty(), "clearResourceTemplates 会清空模板列表");
    }

    Prompt prompt;
    prompt.name = "preflight_prompt";
    prompt.description = "临时提示";
    prompt.arguments = {"input"};
    check(server.registerPrompt(
              prompt,
              [](const std::string&, const json& args) -> PromptResult {
                  return makePromptTextResult(Role::kUser,
                                              args.value("input", ""));
              }),
          "registerPrompt 可注册新提示");
    check(!server.registerPrompt(prompt,
                                 [](const std::string&, const json&) -> PromptResult {
                                     return PromptResult{};
                                 }),
          "registerPrompt 会拒绝重复名称");
    check(server.unregisterPrompt("preflight_prompt"),
          "unregisterPrompt 可移除已注册提示");
    check(!server.unregisterPrompt("preflight_prompt"),
          "unregisterPrompt 对不存在提示返回 false");
    server.registerPrompt(
        prompt, [](const std::string&, const json& args) -> PromptResult {
            return makePromptTextResult(Role::kUser, args.value("input", ""));
        });
    server.clearPrompts();
    check(server.listPrompts().empty(), "clearPrompts 会清空提示列表");
}

// ---------------------------------------------------------------------------
// 1. 注册工具(Tool) — 展示各种工具类型
// ---------------------------------------------------------------------------
static void registerAllTools(MCPServer& server) {
    section("注册工具 (Tools)");

    // 工具1: echo — 简单回显
    {
        Tool tool;
        tool.name = "echo";
        tool.description = "回显输入文本，用于测试基本工具调用";
        tool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"text", {{"type", "string"}, {"description", "要回显的文本"}}}}},
            {"required", {"text"}}};
        server.registerTool(
            tool, [](const ToolCallParams& params) -> ToolCallResult {
                ToolCallResult result;
                std::string text = params.arguments.value("text", "");
                result.content = {
                    {{"type", "text"}, {"text", "echo: " + text}}};
                return result;
            });
        std::cout << "  " << greenTag("[OK]") << " 注册工具: echo" << std::endl;
    }

    // 工具2: calculate — 简单计算器
    {
        Tool tool;
        tool.name = "calculate";
        tool.description = "执行简单数学运算 (add/sub/mul/div)";
        tool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"op",
               {{"type", "string"},
                {"enum", {"add", "sub", "mul", "div"}},
                {"description", "运算符"}}},
              {"a", {{"type", "number"}, {"description", "操作数 A"}}},
              {"b", {{"type", "number"}, {"description", "操作数 B"}}}}},
            {"required", {"op", "a", "b"}}};
        server.registerTool(
            tool, [](const ToolCallParams& params) -> ToolCallResult {
                ToolCallResult result;
                std::string op = params.arguments.value("op", "");
                double a = params.arguments.value("a", 0.0);
                double b = params.arguments.value("b", 0.0);
                double val = 0;
                if (op == "add")
                    val = a + b;
                else if (op == "sub")
                    val = a - b;
                else if (op == "mul")
                    val = a * b;
                else if (op == "div") {
                    if (b == 0) {
                        result.isError = true;
                        result.errorMessage = "除数不能为零";
                        return result;
                    }
                    val = a / b;
                } else {
                    result.isError = true;
                    result.errorMessage = "不支持的运算符: " + op;
                    return result;
                }
                result.content = {{{"type", "text"},
                                   {"text", std::to_string(a) + " " + op + " " +
                                                std::to_string(b) + " = " +
                                                std::to_string(val)}}};
                return result;
            });
        std::cout << "  " << greenTag("[OK]") << " 注册工具: calculate"
                  << std::endl;
    }

    // 工具3: long_running_task — 支持进度通知的长时间任务
    {
        Tool tool;
        tool.name = "long_running_task";
        tool.description = "模拟长时间运行的任务 (支持进度通知)";
        tool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"steps",
               {{"type", "integer"}, {"description", "模拟步数 (默认5)"}}},
              {"progress_token",
               {{"type", "string"},
                {"description", "进度令牌，用于接收进度通知"}}}}}};
        // 注意: 进度通知需要在 handler 内通过 server 引用发送
        // 这里通过捕获 server 指针来实现
        server.registerTool(
            tool, [&server](const ToolCallParams& params) -> ToolCallResult {
                ToolCallResult result;
                int steps = params.arguments.value("steps", 5);
                std::string token =
                    params.arguments.value("progress_token", "");

                for (int i = 1; i <= steps; ++i) {
                    // 发送进度通知
                    if (!token.empty()) {
                        ProgressParams progress;
                        progress.progressToken = token;
                        progress.progress = static_cast<double>(i);
                        progress.total = static_cast<double>(steps);
                        progress.message = "步骤 " + std::to_string(i) + "/" +
                                           std::to_string(steps);
                        server.sendProgress(progress);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }

                result.content = {
                    {{"type", "text"},
                     {"text", "任务完成，共执行 " + std::to_string(steps) +
                                  " 个步骤"}}};
                return result;
            });
        std::cout << "  " << greenTag("[OK]")
                  << " 注册工具: long_running_task (支持进度通知)" << std::endl;
    }

    // 工具4: get_system_info — 返回结构化数据
    {
        Tool tool;
        tool.name = "get_system_info";
        tool.description = "获取服务器系统信息";
        tool.inputSchema = {{"type", "object"}, {"properties", json::object()}};
        server.registerTool(tool, [](const ToolCallParams&) -> ToolCallResult {
            ToolCallResult result;
            auto now = std::chrono::system_clock::now();
            auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                             now.time_since_epoch())
                             .count();
            json runtime_state;
            {
                std::lock_guard<std::mutex> lock(g_state.mutex);
                runtime_state = {
                    {"cancel_count", g_state.cancel_count},
                    {"last_cancel_request_id", g_state.last_cancel_request_id},
                    {"last_cancel_reason", g_state.last_cancel_reason},
                    {"roots_changed_count", g_state.roots_changed_count},
                    {"last_roots_count", g_state.last_roots_count}};
            }
            json info = {{"server_name", "mcp_server_full_example"},
                         {"version", "1.0.0"},
                         {"protocol_version", PROTOCOL_VERSION},
                         {"uptime_epoch", epoch},
                         {"logging_supported", true},
                         {"features",
                          {"tools", "resources", "prompts", "progress",
                           "cancel", "roots", "tasks", "elicitation",
                           "completion", "sampling", "subscribe"}},
                         {"runtime_state", runtime_state}};
            result.content = {{{"type", "text"}, {"text", info.dump(2)}}};
            return result;
        });
        std::cout << "  " << greenTag("[OK]") << " 注册工具: get_system_info"
                  << std::endl;
    }

    auto tools = server.listTools();
    std::cout << "  已注册工具总数: " << tools.size() << std::endl;
}

// ---------------------------------------------------------------------------
// 2. 注册资源(Resource)
// ---------------------------------------------------------------------------
static void registerAllResources(MCPServer& server) {
    section("注册资源 (Resources)");

    // 资源1: 服务器状态
    {
        Resource res;
        res.uri = "status://server";
        res.name = "服务器状态";
        res.description = "当前服务器运行状态信息";
        res.mimeType = "application/json";
        server.registerResource(res, [](const std::string&) -> ReadResourceResult {
            auto now = std::chrono::system_clock::now();
            auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                             now.time_since_epoch())
                             .count();
            return makeTextResourceResult(
                "status://server",
                json({{"status", "running"},
                      {"timestamp", epoch},
                      {"tools_count", 4},
                      {"resources_count", 3},
                      {"prompts_count", 3}})
                    .dump(2),
                "application/json");
        });
        std::cout << "  " << greenTag("[OK]") << " 注册资源: status://server"
                  << std::endl;
    }

    // 资源2: 配置信息
    {
        Resource res;
        res.uri = "config://server";
        res.name = "服务器配置";
        res.description = "服务器配置参数";
        res.mimeType = "application/json";
        server.registerResource(res, [](const std::string&) -> ReadResourceResult {
            return makeTextResourceResult(
                "config://server",
                json({{"max_connections", 100},
                      {"timeout_ms", 30000},
                      {"log_level", "INFO"},
                      {"features_enabled",
                       {{"tools", true},
                        {"resources", true},
                        {"prompts", true},
                        {"progress", true},
                        {"roots", true},
                        {"tasks", true},
                        {"completion", true},
                        {"sampling", true},
                        {"elicitation", true},
                        {"subscribe", true}}}})
                    .dump(2),
                "application/json");
        });
        std::cout << "  " << greenTag("[OK]") << " 注册资源: config://server"
                  << std::endl;
    }

    // 资源3: 帮助文档
    {
        Resource res;
        res.uri = "doc://help";
        res.name = "帮助文档";
        res.description = "MCP Server 使用帮助";
        res.mimeType = "text/plain";
        server.registerResource(res, [](const std::string&) -> ReadResourceResult {
            std::string help_text =
                "MCP Server 全能力展示示例\n"
                "========================\n\n"
                "可用工具:\n"
                "  - echo: 回显文本\n"
                "  - calculate: 数学运算 (add/sub/mul/div)\n"
                "  - long_running_task: 长时间任务 (支持进度通知)\n"
                "  - get_system_info: 获取系统信息\n\n"
                "可用资源:\n"
                "  - status://server: 服务器状态\n"
                "  - config://server: 服务器配置\n"
                "  - doc://help: 本帮助文档\n\n"
                "可用提示:\n"
                "  - greeting: 生成问候语\n"
                "  - summarize: 生成摘要提示\n"
                "  - code_review: 生成代码审查提示\n";
            return makeTextResourceResult("doc://help", help_text);
        });
        std::cout << "  " << greenTag("[OK]") << " 注册资源: doc://help"
                  << std::endl;
    }

    // 模板1: 工作区文件模板
    {
        ResourceTemplate resource_template;
        resource_template.uriTemplate = "file:///workspace/{path}";
        resource_template.name = "workspace-file";
        resource_template.description = "按路径访问工作区文件";
        resource_template.mimeType = "text/plain";
        resource_template.annotations = {{"kind", "workspace"}};
        server.registerResourceTemplate(resource_template);
        std::cout << "  " << greenTag("[OK]")
                  << " 注册资源模板: file:///workspace/{path}" << std::endl;
    }

    // 模板2: 项目文档模板
    {
        ResourceTemplate resource_template;
        resource_template.uriTemplate = "doc://{topic}";
        resource_template.name = "topic-doc";
        resource_template.description = "按主题访问文档";
        resource_template.mimeType = "text/plain";
        resource_template.annotations = {{"kind", "documentation"}};
        server.registerResourceTemplate(resource_template);
        std::cout << "  " << greenTag("[OK]") << " 注册资源模板: doc://{topic}"
                  << std::endl;
    }

    auto resources = server.listResources();
    std::cout << "  已注册资源总数: " << resources.size() << std::endl;
    auto templates = server.listResourceTemplates();
    std::cout << "  已注册资源模板总数: " << templates.size() << std::endl;
}

// ---------------------------------------------------------------------------
// 3. 注册提示(Prompt)
// ---------------------------------------------------------------------------
static void registerAllPrompts(MCPServer& server) {
    section("注册提示 (Prompts)");

    // 提示1: greeting
    {
        Prompt prompt;
        prompt.name = "greeting";
        prompt.description = "生成个性化问候语";
        prompt.arguments = {"name", "style"};
        server.registerPrompt(
            prompt, [](const std::string&, const json& args) -> PromptResult {
                std::string name = args.value("name", "用户");
                std::string style = args.value("style", "friendly");
                std::string greeting;
                if (style == "formal") {
                    greeting = "尊敬的 " + name + "，您好！欢迎使用 MCP 服务。";
                } else if (style == "casual") {
                    greeting = "嗨 " + name + "！欢迎来到 MCP！";
                } else {
                    greeting = "你好，" + name + "！很高兴为您服务。";
                }
                return makePromptTextResult(Role::kAssistant, greeting);
            });
        std::cout << "  " << greenTag("[OK]")
                  << " 注册提示: greeting (参数: name, style)" << std::endl;
    }

    // 提示2: summarize
    {
        Prompt prompt;
        prompt.name = "summarize";
        prompt.description = "生成文本摘要提示";
        prompt.arguments = {"text", "max_length"};
        server.registerPrompt(
            prompt, [](const std::string&, const json& args) -> PromptResult {
                std::string text = args.value("text", "");
                int max_len = args.value("max_length", 100);
                return makePromptTextResult(
                    Role::kUser,
                    "请将以下文本总结为不超过 " + std::to_string(max_len) +
                        " 字的摘要:\n\n" + text);
            });
        std::cout << "  " << greenTag("[OK]")
                  << " 注册提示: summarize (参数: text, max_length)"
                  << std::endl;
    }

    // 提示3: code_review
    {
        Prompt prompt;
        prompt.name = "code_review";
        prompt.description = "生成代码审查提示";
        prompt.arguments = {"code", "language"};
        server.registerPrompt(
            prompt, [](const std::string&, const json& args) -> PromptResult {
                std::string code = args.value("code", "");
                std::string lang = args.value("language", "C++");
                return makePromptTextResult(
                    Role::kUser,
                    "请审查以下 " + lang +
                        " 代码，指出潜在的问题和改进建议:\n\n```" + lang +
                        "\n" + code + "\n```");
            });
        std::cout << "  " << greenTag("[OK]")
                  << " 注册提示: code_review (参数: code, language)"
                  << std::endl;
    }

    auto prompts = server.listPrompts();
    std::cout << "  已注册提示总数: " << prompts.size() << std::endl;
}

// ---------------------------------------------------------------------------
// 4. 设置取消处理器
// ---------------------------------------------------------------------------
static void setupCancelHandler(MCPServer& server) {
    section("设置取消处理器 (Cancel)");

    server.setCancelHandler([](const CancelParams& params) {
        std::lock_guard<std::mutex> lock(g_state.mutex);
        ++g_state.cancel_count;
        g_state.last_cancel_request_id = params.requestId.dump();
        g_state.last_cancel_reason = params.reason;
        std::cout << "  [Cancel] 收到取消请求: requestId="
                  << params.requestId.dump() << ", reason=" << params.reason
                  << std::endl;
        // 实际应用中在此处取消对应的正在执行的任务
    });
    std::cout << "  " << greenTag("[OK]") << " 取消处理器已注册" << std::endl;
}

// ---------------------------------------------------------------------------
// 5. 设置根目录变更处理器
// ---------------------------------------------------------------------------
static void setupRootsHandler(MCPServer& server) {
    section("设置根目录变更处理器 (Roots)");

    server.setRootsListChangedHandler([](const std::vector<Root>& roots) {
        std::lock_guard<std::mutex> lock(g_state.mutex);
        ++g_state.roots_changed_count;
        g_state.last_roots_count = static_cast<int>(roots.size());
        std::cout << "  [Roots] 客户端根目录列表已更新 (" << roots.size()
                  << " 项):" << std::endl;
        for (const auto& root : roots) {
            std::cout << "    - " << root.uri;
            if (!root.name.empty()) std::cout << " (" << root.name << ")";
            std::cout << std::endl;
        }
    });
    std::cout << "  " << greenTag("[OK]") << " 根目录变更处理器已注册"
              << std::endl;
}

static void setup2025ProtocolHandlers(MCPServer& server) {
    section("设置 2025-11-25 扩展能力");

    Task pending;
    pending.taskId = "task-demo-001";
    pending.status = TaskStatus::kWorking;
    pending.createdAt = "2026-01-01T00:00:00Z";
    pending.lastUpdatedAt = pending.createdAt;
    pending.statusMessage = "pending";
    pending.ttl_ms = 600000;
    pending.pollInterval_ms = 1000;
    check(server.upsertTask(pending, json{{"phase", "queued"}}),
          "upsertTask 可注册待处理任务 task-demo-001");

    Task completed;
    completed.taskId = "task-demo-completed";
    completed.status = TaskStatus::kCompleted;
    completed.createdAt = "2026-01-01T00:00:00Z";
    completed.lastUpdatedAt = "2026-01-01T00:00:05Z";
    completed.statusMessage = "done";
    completed.ttl_ms = 600000;
    completed.pollInterval_ms = 1000;
    check(server.upsertTask(completed, json{{"summary", "demo result"}}),
          "upsertTask 可注册已完成任务 task-demo-completed");

    server.setCompletionHandler([](const json&) {
        return json{{"completion",
                     json{{"values",
                           json::array({json{{"value", "echo"},
                                             {"label", "echo"},
                                             {"description", "Echo tool"}}})}}}};
    });
    check(true, "setCompletionHandler 已设置");
}

// ---------------------------------------------------------------------------
// 6. 演示动态变更 (注册/注销 + 通知)
// ---------------------------------------------------------------------------
static void demonstrateDynamicChanges(MCPServer& server) {
    section("演示动态变更与通知");

    // 动态添加一个工具
    Tool dynamic_tool;
    dynamic_tool.name = "dynamic_tool";
    dynamic_tool.description = "动态添加的工具 (稍后将被移除)";
    dynamic_tool.inputSchema = {{"type", "object"},
                                {"properties", json::object()}};
    server.registerTool(
        dynamic_tool, [](const ToolCallParams&) -> ToolCallResult {
            ToolCallResult r;
            r.content = {{{"type", "text"}, {"text", "动态工具响应"}}};
            return r;
        });
    std::cout << "  [+] 动态添加工具: dynamic_tool" << std::endl;

    // 发送工具变更通知
    server.notifyToolsChanged();
    std::cout << "  [>] 发送 tools/list_changed 通知" << std::endl;

    // 动态添加一个资源
    Resource dynamic_res;
    dynamic_res.uri = "dynamic://resource";
    dynamic_res.name = "动态资源";
    dynamic_res.description = "动态添加的资源 (稍后将被移除)";
    dynamic_res.mimeType = "text/plain";
    server.registerResource(dynamic_res, [](const std::string&) -> ReadResourceResult {
        return makeTextResourceResult("dynamic://resource", "动态资源内容");
    });
    std::cout << "  [+] 动态添加资源: dynamic://resource" << std::endl;
    server.notifyResourcesChanged();
    std::cout << "  [>] 发送 resources/list_changed 通知" << std::endl;

    // 动态添加一个提示
    Prompt dynamic_prompt;
    dynamic_prompt.name = "dynamic_prompt";
    dynamic_prompt.description = "动态添加的提示";
    dynamic_prompt.arguments = {"input"};
    server.registerPrompt(
        dynamic_prompt, [](const std::string&, const json& args) -> PromptResult {
            return makePromptTextResult(Role::kUser,
                                        "动态提示: " + args.value("input", ""));
        });
    std::cout << "  [+] 动态添加提示: dynamic_prompt" << std::endl;
    server.notifyPromptsChanged();
    std::cout << "  [>] 发送 prompts/list_changed 通知" << std::endl;

    std::cout << "  等待 3 秒后移除动态注册项..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 移除动态项
    server.unregisterTool("dynamic_tool");
    server.notifyToolsChanged();
    std::cout << "  [-] 移除工具: dynamic_tool + 发送通知" << std::endl;

    server.unregisterResource("dynamic://resource");
    server.notifyResourcesChanged();
    std::cout << "  [-] 移除资源: dynamic://resource + 发送通知" << std::endl;

    server.unregisterPrompt("dynamic_prompt");
    server.notifyPromptsChanged();
    std::cout << "  [-] 移除提示: dynamic_prompt + 发送通知" << std::endl;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 解析参数
    int port = 8080;
    std::string host = "127.0.0.1";
    std::string endpoint = "/mcp";
    bool use_ipc = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc)
            port = std::stoi(argv[++i]);
        else if (arg == "--host" && i + 1 < argc)
            host = argv[++i];
        else if (arg == "--endpoint" && i + 1 < argc)
            endpoint = argv[++i];
        else if (arg == "--ipc")
            use_ipc = true;
        else if (arg == "--help") {
            std::cout << "用法: " << argv[0] << " [选项]\n"
                      << "  --host <host>       监听地址 (默认 127.0.0.1)\n"
                      << "  --port <port>       监听端口 (默认 8080)\n"
                      << "  --endpoint <path>   HTTP端点 (默认 /mcp)\n"
                      << "  --ipc               使用IPC传输模式\n"
                      << "  --help              显示帮助\n";
            return 0;
        }
    }

    if (use_ipc) {
        // Keep stdout clean for JSON-RPC over stdio in IPC mode.
        std::cout.rdbuf(std::cerr.rdbuf());
        mcp_logger::Logger::getInstance().setTarget(
            mcp_logger::LogTarget::FILE);
        mcp_logger::Logger::getInstance().setLogFile(
            "/tmp/mcp_server_full_example_ipc.log");
    }

    std::cout << "MCP Server 全能力展示示例" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "传输模式: " << (use_ipc ? "IPC" : "HTTP/SSE") << std::endl;
    if (!use_ipc) {
        std::cout << "监听地址: " << host << ":" << port << endpoint
                  << std::endl;
    }

    // ----- 创建服务器 -----
    MCPServer server;
    resetRegressionState();
    runRegistrySelfTests(server);

    // 设置服务器信息
    section("服务器信息与能力");
    server.setServerInfo("mcp-server-full-example", "1.0.0");

    ServerCapabilities caps;
    caps.tools = {{"listChanged", true}};
    caps.resources = {{"subscribe", true}, {"listChanged", true}};
    caps.prompts = {{"listChanged", true}};
    caps.logging = {{"setLevel", true}};
    server.setCapabilities(caps);
    std::cout << "  服务器名称: mcp-server-full-example v1.0.0" << std::endl;
    std::cout << "  能力声明: " << caps.toJson().dump(2) << std::endl;

    // 注册所有能力
    registerAllTools(server);
    registerAllResources(server);
    registerAllPrompts(server);
    setupCancelHandler(server);
    setupRootsHandler(server);
    setup2025ProtocolHandlers(server);

    // ----- 配置传输层并启动 -----
    section("启动服务器");

    TransportConfig config;
    if (use_ipc) {
        config = mcp_transport::utils::createIpcServerConfig(
            "mcp-server-full-example", "1.0.0");
        config.ipc.enable_logging = false;
        config.ipc.log_level = mcp_transport::TransportLogLevel::kInfo;
    } else {
        config = mcp_transport::utils::createSseServerConfig(
            host, port, TransportProtocol::kHttp);
        config.sse.endpoint = endpoint;
        config.sse.timeout_seconds = 30;
        config.sse.keep_alive = true;
        config.sse.headers["Content-Type"] = "application/json; charset=utf-8";
    }
    config.name = "mcp-server-full-example";
    config.enable_logging = !use_ipc;
    config.server.server_name = "mcp-server-full-example";
    config.server.server_version = "1.0.0";
    config.server.description = "MCP Server 全能力展示";

    if (!server.start(config)) {
        std::cerr << redTag("[ERROR]") << " 服务器启动失败" << std::endl;
        return 1;
    }

    std::cout << greenTag("[OK]") << " 服务器启动成功" << std::endl;
    if (!use_ipc) {
        std::cout << "服务地址: http://" << host << ":" << port << endpoint
                  << std::endl;
    }
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;

    // ----- 启动后演示动态变更 -----
    // 在单独线程中延迟执行，以便客户端有时间连接
    std::thread demo_thread([&server]() {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (g_running && server.isRunning()) {
            demonstrateDynamicChanges(server);
        }

        std::this_thread::sleep_for(std::chrono::seconds(12));
        if (g_running && server.isRunning()) {
            section("验证 server->client 请求 (Elicitation/Sampling)");
            try {
                const auto elicitation = server.requestElicitation(
                    {{"mode", "form"},
                     {"message", "Please confirm from server demo"},
                     {"fields", json::object()}},
                    5000);
                if (!elicitation.action.empty()) {
                    check(true, "requestElicitation 收到客户端响应");
                    std::cout << "  action: " << elicitation.action
                              << std::endl;
                } else {
                    std::cout << "  [WARN] requestElicitation 返回空 action"
                              << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "  [WARN] requestElicitation 失败: " << e.what()
                          << std::endl;
            }

            try {
                const auto sampling = server.requestCreateMessage(
                    {{"messages", json::array()},
                     {"maxTokens", 16},
                     {"temperature", 0.2}},
                    5000);
                if (sampling.role == Role::kAssistant) {
                    check(true, "requestCreateMessage 收到客户端响应");
                    std::cout << "  model: " << sampling.model << std::endl;
                } else {
                    std::cout
                        << "  [WARN] requestCreateMessage 返回角色非 assistant"
                        << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "  [WARN] requestCreateMessage 失败: " << e.what()
                          << std::endl;
            }
        }

        // 演示: 查询客户端根目录
        if (g_running && server.isRunning()) {
            section("查询客户端根目录 (Roots)");
            auto roots = server.requestRootsList(5000);
            if (roots.empty()) {
                std::cout << "  未获取到客户端根目录 (客户端可能不支持或超时)"
                          << std::endl;
            } else {
                std::cout << "  客户端根目录 (" << roots.size()
                          << " 项):" << std::endl;
                for (const auto& root : roots) {
                    std::cout << "    - " << root.uri;
                    if (!root.name.empty())
                        std::cout << " (" << root.name << ")";
                    std::cout << std::endl;
                }
            }
        }
    });
    demo_thread.detach();

    // ----- 主循环 -----
    while (g_running && server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 停止服务器
    section("关闭服务器");
    server.stop();
    std::cout << greenTag("[OK]") << " 服务器已关闭" << std::endl;

    if (g_failures.load() > 0) {
        std::cout << redTag("[FAIL]")
                  << " 服务端自检失败数: " << g_failures.load() << std::endl;
        return 1;
    }

    std::cout << "[完成] 服务端回归示例执行完毕" << std::endl;
    return 0;
}
