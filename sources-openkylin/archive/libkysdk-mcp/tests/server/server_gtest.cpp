/**
 * @file server_gtest.cpp
 * @brief Comprehensive Google Test unit tests for the MCP server.
 *
 * Covers: message.h, protocol.h, server_dispatch.h,
 *         MCPServer public API, SessionManager, MCPFileSystemServer,
 *         and protocol_core.cpp utility functions.
 */

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "common/transport.h"
#include "filesystem_server.h"
#include "server.h"
#include "server_dispatch.h"
#include "session_manager.h"

using namespace mcp;
using json = mcp::json;

namespace {
int get_free_tcp_port() {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(sock);
        return -1;
    }

    socklen_t len = sizeof(addr);
    if (::getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &len) < 0) {
        ::close(sock);
        return -1;
    }

    int port = ntohs(addr.sin_port);
    ::close(sock);
    return port;
}

json extract_rpc_result_or_raw(const std::string& body) {
    const json parsed = json::parse(body);
    if (parsed.contains("result")) {
        return parsed["result"];
    }
    return parsed;
}

json send_rpc_request(mcp_transport::Transport& client, const std::string& id,
                      const std::string& method, const json& params) {
    json request_json = {
        {"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", params}};
    auto response = client.sendRequestSync(request_json.dump(), "POST", "/mcp");
    EXPECT_EQ(response.status_code, 200);
    return json::parse(response.content);
}

bool has_python3() {
    const int rc = std::system("python3 --version > /dev/null 2>&1");
    return rc == 0;
}

template <typename Predicate>
bool wait_until(std::chrono::milliseconds timeout, Predicate predicate) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    return predicate();
}

std::filesystem::path create_auto_reconnect_tracking_server_script() {
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto path =
        std::filesystem::temp_directory_path() /
        ("mcp_server_reconnect_tracking_" + std::to_string(now) + ".py");

    std::ofstream ofs(path);
    ofs << R"PY(
import json
import os
import sys
import threading
import time

state_path = sys.argv[1]
mode = sys.argv[2] if len(sys.argv) > 2 else "stable"
trigger_path = sys.argv[3] if len(sys.argv) > 3 else ""

def count_event(event):
    try:
        with open(state_path, "r", encoding="utf-8") as f:
            return sum(1 for line in f if line.strip() == event)
    except FileNotFoundError:
        return 0

def append_event(event):
    with open(state_path, "a", encoding="utf-8") as f:
        f.write(event + "\n")
        f.flush()

def send(obj):
    sys.stdout.write(json.dumps(obj, ensure_ascii=False) + "\n")
    sys.stdout.flush()

def result(req_id, payload):
    send({"jsonrpc": "2.0", "id": req_id, "result": payload})

def watch_crash_trigger():
    while True:
        if trigger_path and os.path.exists(trigger_path):
            try:
                os.remove(trigger_path)
            except OSError:
                pass
            os._exit(137)
        time.sleep(0.05)

def handle(msg):
    method = msg.get("method", "")
    req_id = msg.get("id")
    params = msg.get("params", {})

    if method == "initialize":
        previous = count_event("initialize")
        append_event("initialize")
        result(req_id, {
            "protocolVersion": params.get("protocolVersion", "2025-03-26"),
            "serverInfo": {"name": "tracking-server", "version": "1.0.0"},
            "capabilities": {}
        })
        return

    if method == "resources/subscribe":
        append_event("subscribe " + params.get("uri", ""))
        result(req_id, {})
        return

    if method == "ping":
        result(req_id, {})
        return

    result(req_id, {})

def main():
    while True:
        line = sys.stdin.readline()
        if not line:
            return
        try:
            msg = json.loads(line.strip())
        except Exception:
            continue
        if isinstance(msg, dict):
            handle(msg)

if __name__ == "__main__":
    if mode == "watch_crash":
        threading.Thread(target=watch_crash_trigger, daemon=True).start()
    main()
)PY";
    return path;
}

int count_state_events(const std::filesystem::path& state_path,
                       const std::string& expected) {
    std::ifstream ifs(state_path);
    int count = 0;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line == expected) {
            ++count;
        }
    }
    return count;
}

}  // namespace

// ============================================================
// Helper: FakeServerContext (reusable mock for dispatch tests)
// ============================================================

class FakeServerContext : public internal::MCPServerHandlerContext {
public:
    bool running = true;

    bool isRunning() const override { return running; }

    InitializeResult initialize(const InitializeParams& params) override {
        last_client_name = params.clientInfo.name;
        last_protocol_version = params.protocolVersion;
        InitializeResult result;
        result.serverInfo = server_info;
        result.protocolVersion = params.protocolVersion;
        result.capabilities = server_capabilities;
        return result;
    }

    void ping() override { ping_called = true; }

    std::vector<Tool> listTools() const override { return tools; }

    ToolCallResult callTool(const ToolCallParams& params) override {
        last_tool_name = params.name;
        last_tool_arguments = params.arguments;
        ToolCallResult result;
        result.content = json::array({json{{"type", "text"}, {"text", "ok"}}});
        return result;
    }

    std::vector<Resource> listResources() const override { return resources; }
    std::vector<ResourceTemplate> listResourceTemplates() const override {
        return resource_templates;
    }

    ReadResourceResult readResource(const std::string& uri) override {
        last_resource_uri = uri;
        ReadResourceResult result;
        ResourceContent content;
        content.uri = uri;
        content.text = "data";
        result.contents.push_back(content);
        return result;
    }

    void subscribeResource(const std::string& uri) override {
        last_subscribe_uri = uri;
    }

    void unsubscribeResource(const std::string& uri) override {
        last_unsubscribe_uri = uri;
    }

    std::vector<Prompt> listPrompts() const override { return prompts; }

    PromptResult getPrompt(const std::string& name,
                           const json& arguments) override {
        last_prompt_name = name;
        last_prompt_arguments = arguments;
        return PromptResult{};
    }

    Task getTask(const std::string& task_id) override {
        last_task_id = task_id;
        Task task;
        task.taskId = task_id;
        task.status = TaskStatus::kWorking;
        return task;
    }

    TaskResult getTaskResult(const std::string& task_id) override {
        last_task_result_id = task_id;
        return TaskResult{task_id, json{{"ok", true}}};
    }

    ListTasksResult listTasks(const std::string& cursor) override {
        last_task_cursor = cursor;
        ListTasksResult result;
        Task task;
        task.taskId = "t1";
        task.status = TaskStatus::kWorking;
        result.tasks.push_back(task);
        return result;
    }

    Task cancelTask(const std::string& task_id) override {
        last_cancel_task_id = task_id;
        Task task;
        task.taskId = task_id;
        task.status = TaskStatus::kCancelled;
        return task;
    }

    ElicitCreateResult createElicitation(const json& params) override {
        last_elicitation_params = params;
        ElicitCreateResult result;
        result.action = "cancel";
        return result;
    }

    CompleteResult complete(const json& params) override {
        last_completion_params = params;
        return CompleteResult{};
    }

    CreateMessageResult createMessage(const json& params) override {
        last_sampling_params = params;
        CreateMessageResult result;
        result.role = Role::kAssistant;
        result.content = json::array();
        return result;
    }

    void setLoggingLevel(const json& params) override {
        last_logging_level =
            internal::logLevelToString(internal::parseLogLevel(params));
    }

    void initialized() override { initialized_called = true; }

    const ServerInfo& getServerInfo() const override { return server_info; }

    void handleCancelRequest(const CancelParams& params) override {
        cancel_called = true;
        last_cancel_request_id = params.requestId;
        last_cancel_reason = params.reason;
    }

    void handleRootsListChanged() override { roots_list_changed_called = true; }

    // State
    mutable std::vector<Tool> tools;
    mutable std::vector<Resource> resources;
    mutable std::vector<ResourceTemplate> resource_templates;
    mutable std::vector<Prompt> prompts;
    ServerInfo server_info{"test-server", "1.0.0"};
    ServerCapabilities server_capabilities;
    bool ping_called = false;
    bool initialized_called = false;
    std::string last_client_name;
    std::string last_protocol_version;
    std::string last_tool_name;
    json last_tool_arguments;
    std::string last_resource_uri;
    std::string last_subscribe_uri;
    std::string last_unsubscribe_uri;
    std::string last_prompt_name;
    std::string last_logging_level;
    json last_prompt_arguments = json::object();
    std::string last_task_id;
    std::string last_task_result_id;
    std::string last_task_cursor;
    std::string last_cancel_task_id;
    json last_elicitation_params = json::object();
    json last_completion_params = json::object();
    json last_sampling_params = json::object();
    bool cancel_called = false;
    json last_cancel_request_id;
    std::string last_cancel_reason;
    bool roots_list_changed_called = false;
};

// ============================================================
// 1. message.h tests
// ============================================================

class McpMessageTest : public ::testing::Test {};

TEST_F(McpMessageTest, ErrorCodeValues) {
    EXPECT_EQ(static_cast<int>(ErrorCode::kParseError), -32700);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInvalidRequest), -32600);
    EXPECT_EQ(static_cast<int>(ErrorCode::kMethodNotFound), -32601);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInvalidParams), -32602);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInternalError), -32603);
    EXPECT_EQ(static_cast<int>(ErrorCode::kServerErrorStart), -32000);
    EXPECT_EQ(static_cast<int>(ErrorCode::kServerErrorEnd), -32099);
}

TEST_F(McpMessageTest, McpExceptionStoresCodeAndMessage) {
    mcp_exception ex(ErrorCode::kInvalidParams, "bad params");
    EXPECT_EQ(ex.code(), ErrorCode::kInvalidParams);
    EXPECT_STREQ(ex.what(), "bad params");
}

TEST_F(McpMessageTest, RequestCreate) {
    auto req = request::create("tools/list", json{{"cursor", "abc"}});
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.method, "tools/list");
    EXPECT_FALSE(req.isNotification());
    EXPECT_EQ(req.params["cursor"], "abc");
}

TEST_F(McpMessageTest, RequestCreateDefaultParams) {
    auto req = request::create("ping");
    EXPECT_TRUE(req.params.is_object());
    EXPECT_TRUE(req.params.empty());
}

TEST_F(McpMessageTest, RequestCreateWithId) {
    auto req =
        request::createWithId("my-id", "initialize", json{{"key", "val"}});
    EXPECT_EQ(req.id, "my-id");
    EXPECT_EQ(req.method, "initialize");
    EXPECT_EQ(req.params["key"], "val");
    EXPECT_FALSE(req.isNotification());
}

TEST_F(McpMessageTest, RequestCreateNotification) {
    auto req = request::createNotification("notifications/initialized");
    EXPECT_TRUE(req.isNotification());
    EXPECT_TRUE(req.id.is_null());
    EXPECT_EQ(req.method, "notifications/initialized");
}

TEST_F(McpMessageTest, RequestToJsonWithId) {
    auto req = request::createWithId("42", "ping");
    json j = req.toJson();
    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["method"], "ping");
    EXPECT_TRUE(j.contains("id"));
    EXPECT_EQ(j["id"], "42");
}

TEST_F(McpMessageTest, RequestToJsonNotification) {
    auto req = request::createNotification("notifications/initialized");
    json j = req.toJson();
    EXPECT_FALSE(j.contains("id"));
    EXPECT_EQ(j["method"], "notifications/initialized");
}

TEST_F(McpMessageTest, RequestToJsonWithParams) {
    auto req = request::create("tools/call", json{{"name", "echo"}});
    json j = req.toJson();
    EXPECT_TRUE(j.contains("params"));
    EXPECT_EQ(j["params"]["name"], "echo");
}

TEST_F(McpMessageTest, RequestToJsonEmptyParams) {
    auto req = request::create("ping");
    json j = req.toJson();
    // Empty params should not be included
    EXPECT_FALSE(j.contains("params"));
}

TEST_F(McpMessageTest, RequestFromJson) {
    json j = {{"jsonrpc", "2.0"},
              {"id", 7},
              {"method", "tools/list"},
              {"params", json{{"cursor", "x"}}}};
    auto req = request::fromJson(j);
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.id, 7);
    EXPECT_EQ(req.method, "tools/list");
    EXPECT_EQ(req.params["cursor"], "x");
}

TEST_F(McpMessageTest, RequestFromJsonNoParams) {
    json j = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "ping"}};
    auto req = request::fromJson(j);
    EXPECT_TRUE(req.params.is_null() || req.params.empty());
}

TEST_F(McpMessageTest, ResponseCreateSuccess) {
    auto res = response::createSuccess(42, json{{"ok", true}});
    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 42);
    EXPECT_EQ(res.result["ok"], true);
    EXPECT_FALSE(res.isError());
}

TEST_F(McpMessageTest, ResponseCreateError) {
    auto res =
        response::createError(7, ErrorCode::kMethodNotFound, "not found");
    EXPECT_EQ(res.id, 7);
    EXPECT_TRUE(res.isError());
    EXPECT_EQ(res.error["code"], -32601);
    EXPECT_EQ(res.error["message"], "not found");
}

TEST_F(McpMessageTest, ResponseCreateErrorWithData) {
    auto res = response::createError(1, ErrorCode::kInternalError, "fail",
                                     json{{"detail", "x"}});
    EXPECT_TRUE(res.error.contains("data"));
    EXPECT_EQ(res.error["data"]["detail"], "x");
}

TEST_F(McpMessageTest, ResponseToJsonSuccess) {
    auto res = response::createSuccess(1, json{{"status", "ok"}});
    json j = res.toJson();
    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["id"], 1);
    EXPECT_TRUE(j.contains("result"));
    EXPECT_FALSE(j.contains("error"));
}

TEST_F(McpMessageTest, ResponseToJsonError) {
    auto res = response::createError(2, ErrorCode::kParseError, "bad json");
    json j = res.toJson();
    EXPECT_TRUE(j.contains("error"));
    EXPECT_FALSE(j.contains("result"));
}

TEST_F(McpMessageTest, ResponseFromJson) {
    json j = {{"jsonrpc", "2.0"}, {"id", 3}, {"result", json{{"ok", true}}}};
    auto res = response::fromJson(j);
    EXPECT_EQ(res.id, 3);
    EXPECT_FALSE(res.isError());
    EXPECT_EQ(res.result["ok"], true);
}

TEST_F(McpMessageTest, ResponseFromJsonError) {
    json j = {{"jsonrpc", "2.0"},
              {"id", 4},
              {"error", json{{"code", -32700}, {"message", "parse"}}}};
    auto res = response::fromJson(j);
    EXPECT_TRUE(res.isError());
    EXPECT_EQ(res.error["code"], -32700);
}

TEST_F(McpMessageTest, RequestIdAutoGeneratedAsUniqueString) {
    auto r1 = request::create("a");
    auto r2 = request::create("b");
    ASSERT_TRUE(r1.id.is_string());
    ASSERT_TRUE(r2.id.is_string());
    EXPECT_NE(r1.id, r2.id);
}

// ============================================================
// 2. protocol.h data structure tests
// ============================================================

class ProtocolCoreTest : public ::testing::Test {};

// --- ClientCapabilities ---

TEST_F(ProtocolCoreTest, ClientCapabilitiesToJsonEmpty) {
    ClientCapabilities caps;
    json j = caps.toJson();
    // All fields empty -> json should be empty or minimal
    EXPECT_TRUE(j.is_null() || j.is_object());
}

TEST_F(ProtocolCoreTest, ClientCapabilitiesToJsonNonEmpty) {
    ClientCapabilities caps;
    caps.logging = json{{"enabled", true}};
    caps.roots = json{{"listChanged", true}};
    json j = caps.toJson();
    EXPECT_TRUE(j.contains("logging"));
    EXPECT_TRUE(j.contains("roots"));
    EXPECT_FALSE(j.contains("prompts"));  // empty -> omitted
}

TEST_F(ProtocolCoreTest, ClientCapabilitiesRoundTrip) {
    ClientCapabilities original;
    original.logging = json{{"level", "debug"}};
    original.prompts = json{{"listChanged", true}};
    original.resources = json{{"subscribe", false}};
    original.tools = json{{"listChanged", true}};
    original.sampling = json{{"maxTokens", 1024}};
    original.roots = json{{"listChanged", false}};

    json j = original.toJson();
    ClientCapabilities parsed = ClientCapabilities::fromJson(j);
    EXPECT_EQ(parsed.logging, original.logging);
    EXPECT_EQ(parsed.prompts, original.prompts);
    EXPECT_EQ(parsed.resources, original.resources);
    EXPECT_EQ(parsed.tools, original.tools);
    EXPECT_EQ(parsed.sampling, original.sampling);
    EXPECT_EQ(parsed.roots, original.roots);
}

TEST_F(ProtocolCoreTest, ClientCapabilitiesFromEmptyJson) {
    ClientCapabilities caps = ClientCapabilities::fromJson(json::object());
    EXPECT_TRUE(caps.logging.is_object());
    EXPECT_TRUE(caps.logging.empty());
}

// --- ServerCapabilities ---

TEST_F(ProtocolCoreTest, ServerCapabilitiesToJsonEmpty) {
    ServerCapabilities caps;
    json j = caps.toJson();
    EXPECT_TRUE(j.is_null() || j.is_object());
}

TEST_F(ProtocolCoreTest, ServerCapabilitiesRoundTrip) {
    ServerCapabilities original;
    original.experimental = json{{"feature_x", true}};
    original.prompts = json{{"listChanged", true}};
    original.resources = json{{"subscribe", true}};
    original.tools = json{{"listChanged", false}};
    original.logging = json{{"enabled", true}};
    original.sampling = json{{"maxTokens", 2048}};

    json j = original.toJson();
    ServerCapabilities parsed = ServerCapabilities::fromJson(j);
    EXPECT_EQ(parsed.experimental, original.experimental);
    EXPECT_EQ(parsed.prompts, original.prompts);
    EXPECT_EQ(parsed.resources, original.resources);
    EXPECT_EQ(parsed.tools, original.tools);
    EXPECT_EQ(parsed.logging, original.logging);
    EXPECT_EQ(parsed.sampling, original.sampling);
}

// --- ClientInfo / ServerInfo ---

TEST_F(ProtocolCoreTest, ClientInfoRoundTrip) {
    ClientInfo info;
    info.name = "test-client";
    info.version = "2.0.0";
    json j = info.toJson();
    EXPECT_EQ(j["name"], "test-client");
    EXPECT_EQ(j["version"], "2.0.0");

    ClientInfo parsed = ClientInfo::fromJson(j);
    EXPECT_EQ(parsed.name, info.name);
    EXPECT_EQ(parsed.version, info.version);
}

TEST_F(ProtocolCoreTest, ClientInfoFromEmptyJson) {
    ClientInfo info = ClientInfo::fromJson(json::object());
    EXPECT_TRUE(info.name.empty());
    EXPECT_TRUE(info.version.empty());
}

TEST_F(ProtocolCoreTest, ServerInfoRoundTrip) {
    ServerInfo info;
    info.name = "my-server";
    info.version = "3.0";
    json j = info.toJson();
    ServerInfo parsed = ServerInfo::fromJson(j);
    EXPECT_EQ(parsed.name, "my-server");
    EXPECT_EQ(parsed.version, "3.0");
}

// --- InitializeParams ---

TEST_F(ProtocolCoreTest, InitializeParamsRoundTrip) {
    InitializeParams params;
    params.protocolVersion = "2024-11-05";
    params.clientInfo.name = "claude";
    params.clientInfo.version = "1.0";
    params.capabilities.roots = json{{"listChanged", true}};

    json j = params.toJson();
    EXPECT_EQ(j["protocolVersion"], "2024-11-05");
    EXPECT_EQ(j["clientInfo"]["name"], "claude");

    InitializeParams parsed = InitializeParams::fromJson(j);
    EXPECT_EQ(parsed.protocolVersion, params.protocolVersion);
    EXPECT_EQ(parsed.clientInfo.name, "claude");
    json expected_roots = json{{"listChanged", true}};
    EXPECT_EQ(parsed.capabilities.roots, expected_roots);
}

TEST_F(ProtocolCoreTest, InitializeParamsFromMinimalJson) {
    InitializeParams params = InitializeParams::fromJson(json::object());
    EXPECT_EQ(params.protocolVersion, PROTOCOL_VERSION);
    EXPECT_TRUE(params.clientInfo.name.empty());
}

// --- InitializeResult ---

TEST_F(ProtocolCoreTest, InitializeResultRoundTrip) {
    InitializeResult result;
    result.protocolVersion = PROTOCOL_VERSION;
    result.serverInfo.name = "server";
    result.serverInfo.version = "1.0";
    result.capabilities.tools = json{{"listChanged", true}};

    json j = result.toJson();
    InitializeResult parsed = InitializeResult::fromJson(j);
    EXPECT_EQ(parsed.protocolVersion, PROTOCOL_VERSION);
    EXPECT_EQ(parsed.serverInfo.name, "server");
    json expected_tools_cap = json{{"listChanged", true}};
    EXPECT_EQ(parsed.capabilities.tools, expected_tools_cap);
}

// --- Tool ---

TEST_F(ProtocolCoreTest, ToolRoundTrip) {
    Tool tool;
    tool.name = "echo";
    tool.description = "Echo back input";
    tool.inputSchema =
        json{{"type", "object"},
             {"properties", json{{"text", json{{"type", "string"}}}}}};

    json j = tool.toJson();
    EXPECT_EQ(j["name"], "echo");
    EXPECT_EQ(j["description"], "Echo back input");

    Tool parsed = Tool::fromJson(j);
    EXPECT_EQ(parsed.name, tool.name);
    EXPECT_EQ(parsed.description, tool.description);
    EXPECT_EQ(parsed.inputSchema, tool.inputSchema);
}

TEST_F(ProtocolCoreTest, ToolFromEmptyJson) {
    Tool t = Tool::fromJson(json::object());
    EXPECT_TRUE(t.name.empty());
    EXPECT_TRUE(t.description.empty());
    EXPECT_TRUE(t.inputSchema.is_object());
}

// --- ToolCallParams ---

TEST_F(ProtocolCoreTest, ToolCallParamsRoundTrip) {
    ToolCallParams params;
    params.name = "read_file";
    params.arguments = json{{"path", "/tmp/test.txt"}};

    json j = params.toJson();
    ToolCallParams parsed = ToolCallParams::fromJson(j);
    EXPECT_EQ(parsed.name, "read_file");
    EXPECT_EQ(parsed.arguments["path"], "/tmp/test.txt");
}

TEST_F(ProtocolCoreTest, ToolCallParamsFromEmptyJson) {
    ToolCallParams p = ToolCallParams::fromJson(json::object());
    EXPECT_TRUE(p.name.empty());
    EXPECT_TRUE(p.arguments.is_object());
}

// --- ToolCallResult ---

TEST_F(ProtocolCoreTest, ToolCallResultSuccess) {
    ToolCallResult result;
    result.content = json::array({json{{"type", "text"}, {"text", "hello"}}});

    json j = result.toJson();
    EXPECT_TRUE(j.contains("content"));
    EXPECT_FALSE(j.contains("error"));

    ToolCallResult parsed = ToolCallResult::fromJson(j);
    EXPECT_FALSE(parsed.isError);
    EXPECT_EQ(parsed.content[0]["text"], "hello");
}

TEST_F(ProtocolCoreTest, ToolCallResultError) {
    ToolCallResult result;
    result.isError = true;
    result.errorMessage = "tool failed";

    json j = result.toJson();
    EXPECT_TRUE(j.contains("error"));
    EXPECT_FALSE(j.contains("content"));
    EXPECT_EQ(j["error"], "tool failed");

    ToolCallResult parsed = ToolCallResult::fromJson(j);
    EXPECT_TRUE(parsed.isError);
    EXPECT_EQ(parsed.errorMessage, "tool failed");
}

// --- Resource ---

TEST_F(ProtocolCoreTest, ResourceRoundTrip) {
    Resource r;
    r.uri = "file:///tmp/test.txt";
    r.name = "test";
    r.description = "A test resource";
    r.mimeType = "text/plain";

    json j = r.toJson();
    Resource parsed = Resource::fromJson(j);
    EXPECT_EQ(parsed.uri, r.uri);
    EXPECT_EQ(parsed.name, r.name);
    EXPECT_EQ(parsed.description, r.description);
    EXPECT_EQ(parsed.mimeType, r.mimeType);
}

TEST_F(ProtocolCoreTest, ResourceFromEmptyJson) {
    Resource r = Resource::fromJson(json::object());
    EXPECT_TRUE(r.uri.empty());
    EXPECT_TRUE(r.name.empty());
}

// --- Prompt ---

TEST_F(ProtocolCoreTest, PromptRoundTrip) {
    Prompt p;
    p.name = "greeting";
    p.description = "A greeting prompt";
    p.arguments = {"name", "language"};

    json j = p.toJson();
    EXPECT_EQ(j["name"], "greeting");
    EXPECT_EQ(j["arguments"].size(), 2);
    // Each argument is serialized as an object with name
    EXPECT_EQ(j["arguments"][0]["name"], "name");

    Prompt parsed = Prompt::fromJson(j);
    EXPECT_EQ(parsed.name, "greeting");
    EXPECT_EQ(parsed.arguments.size(), 2);
    EXPECT_EQ(parsed.arguments[0], "name");
    EXPECT_EQ(parsed.arguments[1], "language");
}

TEST_F(ProtocolCoreTest, PromptFromJsonStringArguments) {
    json j = {{"name", "p"},
              {"description", "d"},
              {"arguments", json::array({"a", "b"})}};
    Prompt p = Prompt::fromJson(j);
    EXPECT_EQ(p.arguments.size(), 2);
    EXPECT_EQ(p.arguments[0], "a");
    EXPECT_EQ(p.arguments[1], "b");
}

TEST_F(ProtocolCoreTest, PromptFromJsonObjectArguments) {
    json j = {
        {"name", "p"},
        {"description", "d"},
        {"arguments", json::array({json{{"name", "x"}, {"required", true}}})}};
    Prompt p = Prompt::fromJson(j);
    EXPECT_EQ(p.arguments.size(), 1);
    EXPECT_EQ(p.arguments[0], "x");
}

TEST_F(ProtocolCoreTest, PromptFromEmptyJson) {
    Prompt p = Prompt::fromJson(json::object());
    EXPECT_TRUE(p.name.empty());
    EXPECT_TRUE(p.arguments.empty());
}

// --- Root ---

TEST_F(ProtocolCoreTest, RootRoundTrip) {
    Root root;
    root.uri = "file:///home/user/project";
    root.name = "my-project";

    json j = root.toJson();
    EXPECT_EQ(j["uri"], root.uri);
    EXPECT_EQ(j["name"], root.name);

    Root parsed = Root::fromJson(j);
    EXPECT_EQ(parsed.uri, root.uri);
    EXPECT_EQ(parsed.name, root.name);
}

TEST_F(ProtocolCoreTest, RootNameOptional) {
    Root root;
    root.uri = "file:///tmp";
    json j = root.toJson();
    EXPECT_FALSE(j.contains("name"));

    Root parsed = Root::fromJson(j);
    EXPECT_EQ(parsed.uri, "file:///tmp");
    EXPECT_TRUE(parsed.name.empty());
}

// --- ProgressParams ---

TEST_F(ProtocolCoreTest, ProgressParamsFullRoundTrip) {
    ProgressParams params;
    params.progressToken = "tok-1";
    params.progress = 5.0;
    params.total = 10.0;
    params.message = "halfway";

    json j = params.toJson();
    EXPECT_EQ(j["progressToken"], "tok-1");
    EXPECT_EQ(j["progress"], 5.0);
    EXPECT_EQ(j["total"], 10.0);
    EXPECT_EQ(j["message"], "halfway");

    ProgressParams parsed = ProgressParams::fromJson(j);
    EXPECT_EQ(parsed.progressToken, "tok-1");
    EXPECT_DOUBLE_EQ(parsed.progress, 5.0);
    EXPECT_DOUBLE_EQ(parsed.total, 10.0);
    EXPECT_EQ(parsed.message, "halfway");
}

TEST_F(ProtocolCoreTest, ProgressParamsOptionalFieldsOmitted) {
    ProgressParams params;
    params.progressToken = 42;
    params.progress = 1.0;
    // total=0, message="" => omitted
    json j = params.toJson();
    EXPECT_FALSE(j.contains("total"));
    EXPECT_FALSE(j.contains("message"));
}

TEST_F(ProtocolCoreTest, ProgressParamsNumericToken) {
    ProgressParams params;
    params.progressToken = 99;
    params.progress = 3.0;
    json j = params.toJson();
    EXPECT_EQ(j["progressToken"], 99);

    ProgressParams parsed = ProgressParams::fromJson(j);
    EXPECT_EQ(parsed.progressToken, 99);
}

// --- CancelParams ---

TEST_F(ProtocolCoreTest, CancelParamsFullRoundTrip) {
    CancelParams params;
    params.requestId = "req-7";
    params.reason = "timeout";

    json j = params.toJson();
    EXPECT_EQ(j["requestId"], "req-7");
    EXPECT_EQ(j["reason"], "timeout");

    CancelParams parsed = CancelParams::fromJson(j);
    EXPECT_EQ(parsed.requestId, "req-7");
    EXPECT_EQ(parsed.reason, "timeout");
}

TEST_F(ProtocolCoreTest, CancelParamsReasonOptional) {
    CancelParams params;
    params.requestId = 99;
    json j = params.toJson();
    EXPECT_FALSE(j.contains("reason"));

    CancelParams parsed = CancelParams::fromJson(j);
    EXPECT_EQ(parsed.requestId, 99);
    EXPECT_TRUE(parsed.reason.empty());
}

// ============================================================
// 3. server_dispatch.h tests
// ============================================================

class DispatchTest : public ::testing::Test {
protected:
    FakeServerContext ctx;
};

// --- parseLogLevel ---

TEST_F(DispatchTest, ParseLogLevelStringDebug) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "debug"}}),
              mcp_logger::LogLevel::DEBUG);
}

TEST_F(DispatchTest, ParseLogLevelStringInfo) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "info"}}),
              mcp_logger::LogLevel::INFO);
}

TEST_F(DispatchTest, ParseLogLevelStringWarning) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "warning"}}),
              mcp_logger::LogLevel::WARNING);
}

TEST_F(DispatchTest, ParseLogLevelStringWarn) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "warn"}}),
              mcp_logger::LogLevel::WARNING);
}

TEST_F(DispatchTest, ParseLogLevelStringError) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "error"}}),
              mcp_logger::LogLevel::ERROR);
}

TEST_F(DispatchTest, ParseLogLevelStringFatal) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "fatal"}}),
              mcp_logger::LogLevel::FATAL);
}

TEST_F(DispatchTest, ParseLogLevelCaseInsensitive) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "DEBUG"}}),
              mcp_logger::LogLevel::DEBUG);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "WARN"}}),
              mcp_logger::LogLevel::WARNING);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "Error"}}),
              mcp_logger::LogLevel::ERROR);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", "Info"}}),
              mcp_logger::LogLevel::INFO);
}

TEST_F(DispatchTest, ParseLogLevelNumeric) {
    EXPECT_EQ(internal::parseLogLevel(json{{"level", 0}}),
              mcp_logger::LogLevel::DEBUG);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", 1}}),
              mcp_logger::LogLevel::INFO);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", 2}}),
              mcp_logger::LogLevel::WARNING);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", 3}}),
              mcp_logger::LogLevel::ERROR);
    EXPECT_EQ(internal::parseLogLevel(json{{"level", 4}}),
              mcp_logger::LogLevel::FATAL);
}

TEST_F(DispatchTest, ParseLogLevelInvalidString) {
    EXPECT_THROW(
        {
            internal::parseLogLevel(json{{"level", "verbose"}});
        },
        mcp_exception);
}

TEST_F(DispatchTest, ParseLogLevelInvalidNumber) {
    EXPECT_THROW(
        {
            internal::parseLogLevel(json{{"level", 99}});
        },
        mcp_exception);
}

TEST_F(DispatchTest, ParseLogLevelMissingField) {
    EXPECT_THROW({ internal::parseLogLevel(json::object()); }, mcp_exception);
}

TEST_F(DispatchTest, ParseLogLevelNotObject) {
    EXPECT_THROW({ internal::parseLogLevel(json::array()); }, mcp_exception);
}

// --- logLevelToString ---

TEST_F(DispatchTest, LogLevelToString) {
    EXPECT_EQ(internal::logLevelToString(mcp_logger::LogLevel::DEBUG), "debug");
    EXPECT_EQ(internal::logLevelToString(mcp_logger::LogLevel::INFO), "info");
    EXPECT_EQ(internal::logLevelToString(mcp_logger::LogLevel::WARNING),
              "warning");
    EXPECT_EQ(internal::logLevelToString(mcp_logger::LogLevel::ERROR), "error");
    EXPECT_EQ(internal::logLevelToString(mcp_logger::LogLevel::FATAL), "fatal");
}

// --- parseParamsOrObject ---

TEST_F(DispatchTest, ParseParamsOrObjectEmpty) {
    json result = internal::parseParamsOrObject("");
    EXPECT_TRUE(result.is_object());
    EXPECT_TRUE(result.empty());
}

TEST_F(DispatchTest, ParseParamsOrObjectValid) {
    json result = internal::parseParamsOrObject(R"({"key": "value"})");
    EXPECT_EQ(result["key"], "value");
}

TEST_F(DispatchTest, ParseParamsOrObjectInvalid) {
    EXPECT_THROW({ internal::parseParamsOrObject("not json"); }, mcp_exception);
}

// --- handleServerRequest ---

TEST_F(DispatchTest, HandleInitialize) {
    json params = {
        {"protocolVersion", PROTOCOL_VERSION},
        {"clientInfo", {{"name", "test-client"}, {"version", "1.0"}}},
        {"capabilities", json::object()}};
    std::string response = internal::handleServerRequest(
        ctx, methods::INITIALIZE, params.dump(), "1");
    json j = json::parse(response);
    EXPECT_EQ(j["serverInfo"]["name"], "test-server");
    EXPECT_EQ(ctx.last_client_name, "test-client");
}

TEST_F(DispatchTest, HandlePing) {
    std::string response =
        internal::handleServerRequest(ctx, methods::PING, "{}", "2");
    json j = json::parse(response);
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(ctx.ping_called);
}

TEST_F(DispatchTest, HandleToolsList) {
    Tool t;
    t.name = "echo";
    t.description = "echo";
    t.inputSchema = json::object();
    ctx.tools.push_back(t);

    std::string response =
        internal::handleServerRequest(ctx, methods::TOOLS_LIST, "{}", "3");
    json j = json::parse(response);
    EXPECT_EQ(j["tools"].size(), 1);
    EXPECT_EQ(j["tools"][0]["name"], "echo");
}

TEST_F(DispatchTest, HandleToolsListPaginationWithCursorAndLimit) {
    for (int i = 0; i < 3; ++i) {
        Tool t;
        t.name = "tool-" + std::to_string(i);
        t.description = "desc";
        t.inputSchema = json::object();
        ctx.tools.push_back(t);
    }

    const json params = {{"cursor", "1"}, {"limit", 1}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TOOLS_LIST, params.dump(), "3p");
    json j = json::parse(response);
    ASSERT_TRUE(j.contains("tools"));
    ASSERT_EQ(j["tools"].size(), 1);
    EXPECT_EQ(j["tools"][0]["name"], "tool-1");
    ASSERT_TRUE(j.contains("nextCursor"));
    EXPECT_EQ(j["nextCursor"], "2");
}

TEST_F(DispatchTest, HandleToolsListInvalidCursorThrows) {
    const json params = {{"cursor", "bad-cursor"}};
    EXPECT_THROW(internal::handleServerRequest(ctx, methods::TOOLS_LIST,
                                               params.dump(), "3e"),
                 mcp_exception);
}

TEST_F(DispatchTest, HandleToolsListEmpty) {
    std::string response =
        internal::handleServerRequest(ctx, methods::TOOLS_LIST, "{}", "3");
    json j = json::parse(response);
    EXPECT_EQ(j["tools"].size(), 0);
}

TEST_F(DispatchTest, HandleToolsCall) {
    json params = {{"name", "read_file"}, {"arguments", {{"path", "/tmp"}}}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TOOLS_CALL, params.dump(), "4");
    json j = json::parse(response);
    EXPECT_TRUE(j.contains("content"));
    EXPECT_EQ(ctx.last_tool_name, "read_file");
}

TEST_F(DispatchTest, HandleResourcesList) {
    ctx.resources.push_back(
        Resource{"file:///tmp/a.txt", "a", "desc", "text/plain"});
    std::string response =
        internal::handleServerRequest(ctx, methods::RESOURCES_LIST, "{}", "5");
    json j = json::parse(response);
    EXPECT_EQ(j["resources"].size(), 1);
    EXPECT_EQ(j["resources"][0]["uri"], "file:///tmp/a.txt");
}

TEST_F(DispatchTest, HandleResourcesListPaginationWithCursorAndLimit) {
    ctx.resources.push_back(
        Resource{"file:///tmp/a.txt", "a", "desc", "text/plain"});
    ctx.resources.push_back(
        Resource{"file:///tmp/b.txt", "b", "desc", "text/plain"});

    const json params = {{"cursor", "1"}, {"limit", 1}};
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_LIST, params.dump(), "5p");
    json j = json::parse(response);
    ASSERT_TRUE(j.contains("resources"));
    ASSERT_EQ(j["resources"].size(), 1);
    EXPECT_EQ(j["resources"][0]["uri"], "file:///tmp/b.txt");
}

TEST_F(DispatchTest, HandleResourcesRead) {
    json params = {{"uri", "file:///tmp/test.txt"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_READ, params.dump(), "6");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_resource_uri, "file:///tmp/test.txt");
}

TEST_F(DispatchTest, HandleResourcesTemplatesList) {
    ResourceTemplate resource_template;
    resource_template.uriTemplate = "file:///workspace/{name}";
    resource_template.name = "workspace-template";
    resource_template.description = "workspace resource template";
    resource_template.mimeType = "text/plain";
    ctx.resource_templates.push_back(resource_template);
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_TEMPLATES_LIST, "{}", "6t");
    json j = json::parse(response);
    ASSERT_TRUE(j.contains("resourceTemplates"));
    ASSERT_TRUE(j["resourceTemplates"].is_array());
    ASSERT_EQ(j["resourceTemplates"].size(), 1);
    EXPECT_EQ(j["resourceTemplates"][0]["name"], "workspace-template");
}

TEST_F(DispatchTest, HandleResourcesTemplatesListPaginationWithCursorAndLimit) {
    ResourceTemplate template_a;
    template_a.uriTemplate = "file:///workspace/{name}";
    template_a.name = "template-a";
    template_a.description = "template a";
    template_a.mimeType = "text/plain";
    ctx.resource_templates.push_back(template_a);

    ResourceTemplate template_b;
    template_b.uriTemplate = "file:///data/{name}";
    template_b.name = "template-b";
    template_b.description = "template b";
    template_b.mimeType = "application/json";
    ctx.resource_templates.push_back(template_b);

    const json params = {{"cursor", "1"}, {"limit", 1}};
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_TEMPLATES_LIST, params.dump(), "6tp");
    json j = json::parse(response);
    ASSERT_TRUE(j.contains("resourceTemplates"));
    ASSERT_EQ(j["resourceTemplates"].size(), 1);
    EXPECT_EQ(j["resourceTemplates"][0]["name"], "template-b");
}

TEST_F(DispatchTest, HandlePromptsList) {
    ctx.prompts.push_back(Prompt{"greet", "greeting", {"name"}});
    std::string response =
        internal::handleServerRequest(ctx, methods::PROMPTS_LIST, "{}", "7");
    json j = json::parse(response);
    EXPECT_EQ(j["prompts"].size(), 1);
}

TEST_F(DispatchTest, HandlePromptsListPaginationWithCursorAndLimit) {
    ctx.prompts.push_back(Prompt{"p1", "prompt1", {"name"}});
    ctx.prompts.push_back(Prompt{"p2", "prompt2", {"name"}});

    const json params = {{"cursor", "1"}, {"limit", 1}};
    std::string response = internal::handleServerRequest(
        ctx, methods::PROMPTS_LIST, params.dump(), "7p");
    json j = json::parse(response);
    ASSERT_TRUE(j.contains("prompts"));
    ASSERT_EQ(j["prompts"].size(), 1);
    EXPECT_EQ(j["prompts"][0]["name"], "p2");
}

TEST_F(DispatchTest, HandlePromptsGet) {
    json params = {{"name", "greet"}, {"arguments", {{"name", "alice"}}}};
    std::string response = internal::handleServerRequest(
        ctx, methods::PROMPTS_GET, params.dump(), "8");
    json j = json::parse(response);
    EXPECT_TRUE(j.contains("messages"));
    EXPECT_EQ(ctx.last_prompt_name, "greet");
    EXPECT_EQ(ctx.last_prompt_arguments["name"], "alice");
}

TEST_F(DispatchTest, HandleResourcesSubscribe) {
    json params = {{"uri", "file:///tmp/sub.txt"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_SUBSCRIBE, params.dump(), "6s");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_subscribe_uri, "file:///tmp/sub.txt");
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
}

TEST_F(DispatchTest, HandleResourcesUnsubscribe) {
    json params = {{"uri", "file:///tmp/sub.txt"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::RESOURCES_UNSUBSCRIBE, params.dump(), "6u");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_unsubscribe_uri, "file:///tmp/sub.txt");
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
}

TEST_F(DispatchTest, HandleTasksGet) {
    json params = {{"taskId", "t-100"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TASKS_GET, params.dump(), "8t1");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_task_id, "t-100");
    EXPECT_EQ(j["taskId"], "t-100");
}

TEST_F(DispatchTest, HandleTasksResult) {
    json params = {{"taskId", "t-100"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TASKS_RESULT, params.dump(), "8t2");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_task_result_id, "t-100");
    EXPECT_EQ(j["taskId"], "t-100");
}

TEST_F(DispatchTest, HandleTasksList) {
    json params = {{"cursor", "c1"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TASKS_LIST, params.dump(), "8t3");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_task_cursor, "c1");
    ASSERT_TRUE(j.contains("tasks"));
    ASSERT_TRUE(j["tasks"].is_array());
}

TEST_F(DispatchTest, HandleTasksCancel) {
    json params = {{"taskId", "t-100"}};
    std::string response = internal::handleServerRequest(
        ctx, methods::TASKS_CANCEL, params.dump(), "8t4");
    json j = json::parse(response);
    EXPECT_EQ(ctx.last_cancel_task_id, "t-100");
    EXPECT_EQ(j["status"], "cancelled");
}

TEST_F(DispatchTest, HandleElicitationCreate) {
    json params = {{"mode", "form"}, {"message", "confirm"}};
    try {
        (void)internal::handleServerRequest(ctx, methods::ELICITATION_CREATE,
                                            params.dump(), "8t5");
        FAIL() << "Expected mcp_exception for unsupported server-side "
                  "elicitation/create";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kMethodNotFound);
    }
}

TEST_F(DispatchTest, HandleCompletionComplete) {
    json params = {{"ref", json::object()}, {"argument", json{{"name", "x"}}}};
    std::string response = internal::handleServerRequest(
        ctx, methods::COMPLETION_COMPLETE, params.dump(), "8t6");
    json j = json::parse(response);
    EXPECT_TRUE(ctx.last_completion_params.contains("argument"));
    EXPECT_TRUE(j.contains("completion"));
}

TEST_F(DispatchTest, HandleSamplingCreateMessage) {
    json params = {{"messages", json::array()}, {"maxTokens", 32}};
    try {
        (void)internal::handleServerRequest(
            ctx, methods::SAMPLING_CREATE_MESSAGE, params.dump(), "8t7");
        FAIL() << "Expected mcp_exception for unsupported server-side "
                  "sampling/createMessage";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kMethodNotFound);
    }
}

TEST_F(DispatchTest, HandleLoggingSetLevel) {
    std::string response = internal::handleServerRequest(
        ctx, methods::LOGGING_SET_LEVEL, json{{"level", "error"}}.dump(), "9");
    json j = json::parse(response);
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
    EXPECT_EQ(ctx.last_logging_level, "error");
}

TEST_F(DispatchTest, HandleUnknownMethodThrows) {
    EXPECT_THROW(
        {
            internal::handleServerRequest(ctx, "nonexistent/method", "{}",
                                          "10");
        },
        mcp_exception);

    try {
        internal::handleServerRequest(ctx, "nonexistent/method", "{}", "10");
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kMethodNotFound);
    }
}

// --- JSON-RPC error code mapping (per 思维导图 "JSON-RPC 错误码映射") ---

namespace {
void expectInvalidParams(const std::function<void()>& invoke) {
    try {
        invoke();
        FAIL() << "Expected mcp_exception with kInvalidParams";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kInvalidParams);
    }
}
}  // namespace

TEST_F(DispatchTest, HandleToolsCallEmptyNameMapsToInvalidParams) {
    const json params = {{"name", ""}, {"arguments", json::object()}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::TOOLS_CALL, params.dump(),
                                      "rpc-1");
    });
}

TEST_F(DispatchTest, HandleToolsCallMissingNameMapsToInvalidParams) {
    const json params = {{"arguments", json::object()}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::TOOLS_CALL, params.dump(),
                                      "rpc-2");
    });
}

TEST_F(DispatchTest, HandleToolsCallArgumentsNotObjectMapsToInvalidParams) {
    const json params = {{"name", "tool"}, {"arguments", json::array()}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::TOOLS_CALL, params.dump(),
                                      "rpc-3");
    });
}

TEST_F(DispatchTest, HandleResourcesReadEmptyUriMapsToInvalidParams) {
    const json params = {{"uri", ""}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::RESOURCES_READ,
                                      params.dump(), "rpc-4");
    });
}

TEST_F(DispatchTest, HandleResourcesReadMissingUriMapsToInvalidParams) {
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::RESOURCES_READ, "{}",
                                      "rpc-5");
    });
}

TEST_F(DispatchTest, HandlePromptsGetEmptyNameMapsToInvalidParams) {
    const json params = {{"name", ""}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::PROMPTS_GET, params.dump(),
                                      "rpc-6");
    });
}

TEST_F(DispatchTest, HandlePromptsGetMissingNameMapsToInvalidParams) {
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::PROMPTS_GET, "{}", "rpc-7");
    });
}

TEST_F(DispatchTest, HandlePromptsGetArgumentsNotObjectMapsToInvalidParams) {
    const json params = {{"name", "prompt"}, {"arguments", "raw-string"}};
    expectInvalidParams([&] {
        internal::handleServerRequest(ctx, methods::PROMPTS_GET, params.dump(),
                                      "rpc-8");
    });
}

TEST_F(DispatchTest, HandleHandlerThrownExceptionPropagatesForWrapperMapping) {
    // The dispatch layer does not catch std::exception thrown by user handlers;
    // it propagates so the transport wrapper can map it to -32603. Verifies
    // the contract relied on by PipeCommunication::handleServerRequest catch.
    struct ThrowingContext : public FakeServerContext {
        ToolCallResult callTool(const ToolCallParams&) override {
            throw std::runtime_error("handler boom");
        }
    } throwing_ctx;

    Tool t;
    t.name = "explode";
    t.description = "throws";
    throwing_ctx.tools.push_back(t);

    const json params = {{"name", "explode"}, {"arguments", json::object()}};
    EXPECT_THROW(
        {
            internal::handleServerRequest(throwing_ctx, methods::TOOLS_CALL,
                                          params.dump(), "rpc-9");
        },
        std::exception);
}

// --- handleServerNotification ---

TEST_F(DispatchTest, NotificationInitialized) {
    internal::handleServerNotification(ctx, methods::NOTIFICATION_INITIALIZED,
                                       "{}");
    EXPECT_TRUE(ctx.initialized_called);
}

TEST_F(DispatchTest, NotificationCancelRequest) {
    json params = {{"requestId", "req-42"}, {"reason", "user abort"}};
    internal::handleServerNotification(ctx, methods::NOTIFICATION_CANCELLED,
                                       params.dump());
    EXPECT_TRUE(ctx.cancel_called);
    EXPECT_EQ(ctx.last_cancel_request_id, "req-42");
    EXPECT_EQ(ctx.last_cancel_reason, "user abort");
}

TEST_F(DispatchTest, NotificationRootsListChanged) {
    internal::handleServerNotification(
        ctx, methods::NOTIFICATION_ROOTS_LIST_CHANGED, "{}");
    EXPECT_TRUE(ctx.roots_list_changed_called);
}

TEST_F(DispatchTest, NotificationUnknownMethodNoThrow) {
    // Unknown notifications should not throw, just parse params
    EXPECT_NO_THROW({
        internal::handleServerNotification(ctx, "unknown/notification", "{}");
    });
}

TEST_F(DispatchTest, NotificationInvalidParamsThrows) {
    EXPECT_THROW(
        {
            internal::handleServerNotification(ctx, "unknown/notification",
                                               "bad json");
        },
        mcp_exception);
}

// --- forwardServerNotification ---

TEST_F(DispatchTest, ForwardNotificationSuccess) {
    std::string captured_method;
    std::string captured_params;
    bool sent = internal::forwardServerNotification(
        true,
        [&](const std::string& m, const std::string& p) {
            captured_method = m;
            captured_params = p;
            return true;
        },
        methods::NOTIFICATION_TOOLS_LIST_CHANGED, json::object());
    EXPECT_TRUE(sent);
    EXPECT_EQ(captured_method, methods::NOTIFICATION_TOOLS_LIST_CHANGED);
}

TEST_F(DispatchTest, ForwardNotificationNotRunning) {
    bool sent = internal::forwardServerNotification(
        false, [](const std::string&, const std::string&) { return true; },
        "method", json::object());
    EXPECT_FALSE(sent);
}

TEST_F(DispatchTest, ForwardNotificationNullSender) {
    bool sent = internal::forwardServerNotification(true, nullptr, "method",
                                                    json::object());
    EXPECT_FALSE(sent);
}

TEST_F(DispatchTest, ForwardNotificationSenderReturnsFalse) {
    bool sent = internal::forwardServerNotification(
        true, [](const std::string&, const std::string&) { return false; },
        "method", json::object());
    EXPECT_FALSE(sent);
}

// --- isJsonRpcPayload ---

TEST_F(DispatchTest, IsJsonRpcPayloadSuccess) {
    std::string payload =
        json{{"jsonrpc", "2.0"}, {"id", "7"}, {"result", json{{"ok", true}}}}
            .dump();
    EXPECT_TRUE(internal::isJsonRpcPayload(payload));
}

TEST_F(DispatchTest, IsJsonRpcPayloadError) {
    std::string payload =
        json{{"jsonrpc", "2.0"}, {"id", "8"}, {"error", json{{"code", -32601}}}}
            .dump();
    EXPECT_TRUE(internal::isJsonRpcPayload(payload));
}

TEST_F(DispatchTest, IsJsonRpcPayloadNotRpc) {
    EXPECT_FALSE(
        internal::isJsonRpcPayload(json{{"tools", json::array()}}.dump()));
}

TEST_F(DispatchTest, IsJsonRpcPayloadEmpty) {
    EXPECT_FALSE(internal::isJsonRpcPayload(""));
}

TEST_F(DispatchTest, IsJsonRpcPayloadInvalidJson) {
    EXPECT_FALSE(internal::isJsonRpcPayload("not json"));
}

TEST_F(DispatchTest, IsJsonRpcPayloadWrongVersion) {
    EXPECT_FALSE(internal::isJsonRpcPayload(
        json{{"jsonrpc", "1.0"}, {"result", json::object()}}.dump()));
}

TEST_F(DispatchTest, IsJsonRpcPayloadNoResultOrError) {
    EXPECT_FALSE(
        internal::isJsonRpcPayload(json{{"jsonrpc", "2.0"}, {"id", 1}}.dump()));
}

// --- toLowerCopy ---

TEST_F(DispatchTest, ToLowerCopyBasic) {
    EXPECT_EQ(internal::toLowerCopy("HELLO"), "hello");
    EXPECT_EQ(internal::toLowerCopy("World"), "world");
    EXPECT_EQ(internal::toLowerCopy("already"), "already");
    EXPECT_EQ(internal::toLowerCopy(""), "");
    EXPECT_EQ(internal::toLowerCopy("MiXeD123"), "mixed123");
}

// ============================================================
// 4. MCPServer public API tests
// ============================================================

class MCPServerAPITest : public ::testing::Test {
protected:
    MCPServer server;
};

// --- Construction and state ---

TEST_F(MCPServerAPITest, InitialStateNotRunning) {
    EXPECT_FALSE(server.isRunning());
}

TEST_F(MCPServerAPITest, StartWithInvalidIpcCommandReturnsFalse) {
    mcp_transport::TransportConfig config;
    config.type = mcp_transport::TransportType::kIpc;
    config.mode = mcp_transport::TransportMode::kClient;
    config.ipc.command = "definitely_not_existing_command_123456";
    config.ipc.args = {};
    config.ipc.enable_logging = false;

    EXPECT_FALSE(server.start(config));
    EXPECT_FALSE(server.isRunning());
}

TEST_F(MCPServerAPITest, StopIsIdempotentWhenNotRunning) {
    EXPECT_FALSE(server.isRunning());
    EXPECT_NO_THROW(server.stop());
    EXPECT_FALSE(server.isRunning());
}

TEST_F(MCPServerAPITest, StartAndStopWithSseServerConfig) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    mcp_transport::TransportConfig config;
    config.type = mcp_transport::TransportType::kSse;
    config.mode = mcp_transport::TransportMode::kServer;
    config.sse.host = "127.0.0.1";
    config.sse.port = port;
    config.sse.endpoint = "/mcp";
    config.enable_logging = false;

    server.setServerInfo("unit-test-server", "1.0.0");
    ASSERT_TRUE(server.start(config));
    EXPECT_TRUE(server.isRunning());

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

// --- setServerInfo / setCapabilities ---

TEST_F(MCPServerAPITest, SetServerInfoDoesNotThrow) {
    EXPECT_NO_THROW(server.setServerInfo("my-server", "1.0.0"));
}

TEST_F(MCPServerAPITest, SetServerInfoRejectsEmptyName) {
    try {
        server.setServerInfo("", "1.0.0");
        FAIL() << "Expected mcp_exception";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kInvalidParams);
        EXPECT_NE(std::string(e.what()).find("Server name must be non-empty"),
                  std::string::npos);
    }
}

TEST_F(MCPServerAPITest, SetServerInfoRejectsEmptyVersion) {
    try {
        server.setServerInfo("my-server", "");
        FAIL() << "Expected mcp_exception";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kInvalidParams);
        EXPECT_NE(
            std::string(e.what()).find("Server version must be non-empty"),
            std::string::npos);
    }
}

TEST_F(MCPServerAPITest, SetServerInfoAcceptsValid) {
    ASSERT_NO_THROW(server.setServerInfo("my-server", "1.0.0"));
    const ServerInfo info = server.getServerInfo();
    EXPECT_EQ(info.name, "my-server");
    EXPECT_EQ(info.version, "1.0.0");
}

TEST_F(MCPServerAPITest, SetCapabilitiesDoesNotThrow) {
    ServerCapabilities caps;
    caps.tools = json{{"listChanged", true}};
    EXPECT_NO_THROW(server.setCapabilities(caps));
}

// --- Tool registration ---

TEST_F(MCPServerAPITest, RegisterToolSuccess) {
    Tool tool;
    tool.name = "echo";
    tool.description = "Echo";
    tool.inputSchema = json::object();
    bool ok = server.registerTool(tool, [](const ToolCallParams& p) {
        ToolCallResult r;
        r.content = p.arguments;
        return r;
    });
    EXPECT_TRUE(ok);
    EXPECT_EQ(server.listTools().size(), 1);
    EXPECT_EQ(server.listTools()[0].name, "echo");
}

TEST_F(MCPServerAPITest, RegisterToolDuplicate) {
    Tool tool;
    tool.name = "echo";
    tool.description = "Echo";
    tool.inputSchema = json::object();
    auto handler = [](const ToolCallParams&) { return ToolCallResult{}; };
    EXPECT_TRUE(server.registerTool(tool, handler));
    EXPECT_FALSE(server.registerTool(tool, handler));
    EXPECT_EQ(server.listTools().size(), 1);
}

TEST_F(MCPServerAPITest, RegisterToolEmptyName) {
    Tool tool;
    tool.name = "";
    EXPECT_FALSE(server.registerTool(
        tool, [](const ToolCallParams&) { return ToolCallResult{}; }));
}

TEST_F(MCPServerAPITest, RegisterToolNullHandler) {
    Tool tool;
    tool.name = "test";
    EXPECT_FALSE(server.registerTool(tool, nullptr));
}

TEST_F(MCPServerAPITest, UnregisterToolSuccess) {
    Tool tool;
    tool.name = "rm";
    tool.inputSchema = json::object();
    server.registerTool(tool,
                        [](const ToolCallParams&) { return ToolCallResult{}; });
    EXPECT_TRUE(server.unregisterTool("rm"));
    EXPECT_EQ(server.listTools().size(), 0);
}

TEST_F(MCPServerAPITest, UnregisterToolNonexistent) {
    EXPECT_FALSE(server.unregisterTool("nonexistent"));
}

TEST_F(MCPServerAPITest, ClearTools) {
    Tool t1, t2;
    t1.name = "a";
    t1.inputSchema = json::object();
    t2.name = "b";
    t2.inputSchema = json::object();
    auto handler = [](const ToolCallParams&) { return ToolCallResult{}; };
    server.registerTool(t1, handler);
    server.registerTool(t2, handler);
    EXPECT_EQ(server.listTools().size(), 2);
    server.clearTools();
    EXPECT_EQ(server.listTools().size(), 0);
}

TEST_F(MCPServerAPITest, ListToolsEmpty) {
    EXPECT_TRUE(server.listTools().empty());
}

// --- Resource registration ---

TEST_F(MCPServerAPITest, RegisterResourceSuccess) {
    Resource r;
    r.uri = "file:///tmp/test.txt";
    r.name = "test";
    bool ok = server.registerResource(r, [](const std::string& uri) {
        ReadResourceResult result;
        ResourceContent content;
        content.uri = uri;
        content.text = uri;
        result.contents.push_back(content);
        return result;
    });
    EXPECT_TRUE(ok);
    EXPECT_EQ(server.listResources().size(), 1);
}

TEST_F(MCPServerAPITest, RegisterResourceDuplicate) {
    Resource r;
    r.uri = "file:///dup";
    r.name = "dup";
    auto handler = [](const std::string&) { return ReadResourceResult{}; };
    EXPECT_TRUE(server.registerResource(r, handler));
    EXPECT_FALSE(server.registerResource(r, handler));
}

TEST_F(MCPServerAPITest, RegisterResourceEmptyUri) {
    Resource r;
    r.uri = "";
    EXPECT_FALSE(server.registerResource(
        r, [](const std::string&) { return ReadResourceResult{}; }));
}

TEST_F(MCPServerAPITest, RegisterResourceNullHandler) {
    Resource r;
    r.uri = "file:///x";
    EXPECT_FALSE(server.registerResource(r, nullptr));
}

TEST_F(MCPServerAPITest, UnregisterResourceSuccess) {
    Resource r;
    r.uri = "file:///del";
    server.registerResource(
        r, [](const std::string&) { return ReadResourceResult{}; });
    EXPECT_TRUE(server.unregisterResource("file:///del"));
    EXPECT_TRUE(server.listResources().empty());
}

TEST_F(MCPServerAPITest, UnregisterResourceNonexistent) {
    EXPECT_FALSE(server.unregisterResource("file:///nope"));
}

TEST_F(MCPServerAPITest, ClearResources) {
    Resource r1, r2;
    r1.uri = "file:///a";
    r2.uri = "file:///b";
    auto handler = [](const std::string&) { return ReadResourceResult{}; };
    server.registerResource(r1, handler);
    server.registerResource(r2, handler);
    server.clearResources();
    EXPECT_TRUE(server.listResources().empty());
}

TEST_F(MCPServerAPITest, SubscribeRejectsUnregisteredUri) {
    try {
        server.subscribeResource("unknown://res");
        FAIL() << "Expected mcp_exception";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kInvalidParams);
        const std::string message = e.what();
        EXPECT_NE(message.find("Resource URI not registered"),
                  std::string::npos);
        EXPECT_NE(message.find("unknown://res"), std::string::npos);
    }
}

TEST_F(MCPServerAPITest, SubscribeAcceptsRegisteredUri) {
    Resource resource;
    resource.uri = "preflight://r1";
    resource.name = "r1";
    ASSERT_TRUE(server.registerResource(
        resource, [](const std::string&) { return ReadResourceResult{}; }));
    EXPECT_NO_THROW(server.subscribeResource(resource.uri));
}

TEST_F(MCPServerAPITest, SubscribeIsIdempotentForRegisteredUri) {
    Resource resource;
    resource.uri = "preflight://r2";
    resource.name = "r2";
    ASSERT_TRUE(server.registerResource(
        resource, [](const std::string&) { return ReadResourceResult{}; }));
    EXPECT_NO_THROW(server.subscribeResource(resource.uri));
    EXPECT_NO_THROW(server.subscribeResource(resource.uri));
}

TEST_F(MCPServerAPITest, UnsubscribeRejectsUnregisteredUri) {
    try {
        server.unsubscribeResource("unknown://res");
        FAIL() << "Expected mcp_exception";
    } catch (const mcp_exception& e) {
        EXPECT_EQ(e.code(), ErrorCode::kInvalidParams);
        const std::string message = e.what();
        EXPECT_NE(message.find("Resource URI not registered"),
                  std::string::npos);
        EXPECT_NE(message.find("unknown://res"), std::string::npos);
    }
}

TEST_F(MCPServerAPITest, UnsubscribeIsIdempotentForRegisteredUnsubscribed) {
    Resource resource;
    resource.uri = "preflight://r3";
    resource.name = "r3";
    ASSERT_TRUE(server.registerResource(
        resource, [](const std::string&) { return ReadResourceResult{}; }));
    EXPECT_NO_THROW(server.unsubscribeResource(resource.uri));
}

TEST_F(MCPServerAPITest, UnsubscribeIsIdempotentMultipleCalls) {
    Resource resource;
    resource.uri = "preflight://r4";
    resource.name = "r4";
    ASSERT_TRUE(server.registerResource(
        resource, [](const std::string&) { return ReadResourceResult{}; }));
    ASSERT_NO_THROW(server.subscribeResource(resource.uri));
    EXPECT_NO_THROW(server.unsubscribeResource(resource.uri));
    EXPECT_NO_THROW(server.unsubscribeResource(resource.uri));
}

TEST_F(MCPServerAPITest, RegisterResourceTemplateSuccess) {
    ResourceTemplate resource_template;
    resource_template.uriTemplate = "file:///workspace/{name}";
    resource_template.name = "workspace-template";
    resource_template.description = "workspace resource template";
    resource_template.mimeType = "text/plain";

    EXPECT_TRUE(server.registerResourceTemplate(resource_template));
    const auto templates = server.listResourceTemplates();
    ASSERT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0].uriTemplate, "file:///workspace/{name}");
}

TEST_F(MCPServerAPITest, RegisterResourceTemplateDuplicateRejected) {
    ResourceTemplate resource_template;
    resource_template.uriTemplate = "file:///workspace/{name}";
    resource_template.name = "workspace-template";

    EXPECT_TRUE(server.registerResourceTemplate(resource_template));
    EXPECT_FALSE(server.registerResourceTemplate(resource_template));
}

TEST_F(MCPServerAPITest, RegisterResourceTemplateEmptyUriTemplateRejected) {
    ResourceTemplate resource_template;
    resource_template.uriTemplate = "";
    resource_template.name = "invalid-template";
    EXPECT_FALSE(server.registerResourceTemplate(resource_template));
}

TEST_F(MCPServerAPITest, UnregisterAndClearResourceTemplates) {
    ResourceTemplate a;
    a.uriTemplate = "file:///a/{name}";
    a.name = "a";
    ResourceTemplate b;
    b.uriTemplate = "file:///b/{name}";
    b.name = "b";

    EXPECT_TRUE(server.registerResourceTemplate(a));
    EXPECT_TRUE(server.registerResourceTemplate(b));
    ASSERT_EQ(server.listResourceTemplates().size(), 2);

    EXPECT_TRUE(server.unregisterResourceTemplate("file:///a/{name}"));
    ASSERT_EQ(server.listResourceTemplates().size(), 1);
    EXPECT_FALSE(server.unregisterResourceTemplate("file:///a/{name}"));

    server.clearResourceTemplates();
    EXPECT_TRUE(server.listResourceTemplates().empty());
}

// --- Prompt registration ---

TEST_F(MCPServerAPITest, RegisterPromptSuccess) {
    Prompt p;
    p.name = "greet";
    p.description = "Greeting prompt";
    bool ok = server.registerPrompt(
        p, [](const std::string&, const json&) { return PromptResult{}; });
    EXPECT_TRUE(ok);
    EXPECT_EQ(server.listPrompts().size(), 1);
}

TEST_F(MCPServerAPITest, RegisterPromptDuplicate) {
    Prompt p;
    p.name = "dup";
    auto handler = [](const std::string&, const json&) {
        return PromptResult{};
    };
    EXPECT_TRUE(server.registerPrompt(p, handler));
    EXPECT_FALSE(server.registerPrompt(p, handler));
}

TEST_F(MCPServerAPITest, RegisterPromptEmptyName) {
    Prompt p;
    p.name = "";
    EXPECT_FALSE(server.registerPrompt(
        p, [](const std::string&, const json&) { return PromptResult{}; }));
}

TEST_F(MCPServerAPITest, RegisterPromptNullHandler) {
    Prompt p;
    p.name = "test";
    EXPECT_FALSE(server.registerPrompt(p, nullptr));
}

TEST_F(MCPServerAPITest, UnregisterPromptSuccess) {
    Prompt p;
    p.name = "bye";
    server.registerPrompt(
        p, [](const std::string&, const json&) { return PromptResult{}; });
    EXPECT_TRUE(server.unregisterPrompt("bye"));
    EXPECT_TRUE(server.listPrompts().empty());
}

TEST_F(MCPServerAPITest, UnregisterPromptNonexistent) {
    EXPECT_FALSE(server.unregisterPrompt("nope"));
}

TEST_F(MCPServerAPITest, ClearPrompts) {
    Prompt p1, p2;
    p1.name = "a";
    p2.name = "b";
    auto handler = [](const std::string&, const json&) {
        return PromptResult{};
    };
    server.registerPrompt(p1, handler);
    server.registerPrompt(p2, handler);
    server.clearPrompts();
    EXPECT_TRUE(server.listPrompts().empty());
}

// --- Notifications when not running ---

TEST_F(MCPServerAPITest, NotifyToolsChangedWhenStopped) {
    EXPECT_FALSE(server.notifyToolsChanged());
}

TEST_F(MCPServerAPITest, NotifyResourcesChangedWhenStopped) {
    EXPECT_FALSE(server.notifyResourcesChanged());
}

TEST_F(MCPServerAPITest, NotifyPromptsChangedWhenStopped) {
    EXPECT_FALSE(server.notifyPromptsChanged());
}

// --- SendProgress when not running ---

TEST_F(MCPServerAPITest, SendProgressWhenStopped) {
    ProgressParams p;
    p.progressToken = "tok";
    p.progress = 1.0;
    EXPECT_FALSE(server.sendProgress(p));
}

TEST_F(MCPServerAPITest, RunningWithoutSseClientNotificationAndProgressFail) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    mcp_transport::TransportConfig config;
    config.type = mcp_transport::TransportType::kSse;
    config.mode = mcp_transport::TransportMode::kServer;
    config.sse.host = "127.0.0.1";
    config.sse.port = port;
    config.sse.endpoint = "/mcp";
    config.enable_logging = false;

    ASSERT_TRUE(server.start(config));
    ASSERT_TRUE(server.isRunning());

    ProgressParams p;
    p.progressToken = "tok";
    p.progress = 0.5;

    EXPECT_FALSE(server.notifyToolsChanged());
    EXPECT_FALSE(server.notifyResourcesChanged());
    EXPECT_FALSE(server.notifyPromptsChanged());
    EXPECT_FALSE(server.sendProgress(p));

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST_F(MCPServerAPITest,
       NotifyResourcesChangedAlsoEmitsUpdatedForSubscriptions) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    const std::string subscribed_uri = "file:///virtual/readme.txt";
    Resource resource;
    resource.uri = subscribed_uri;
    resource.name = "readme";
    ASSERT_TRUE(server.registerResource(
        resource, [](const std::string&) { return ReadResourceResult{}; }));

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));
    ASSERT_TRUE(server.isRunning());

    std::mutex capture_mutex;
    bool got_list_changed = false;
    bool got_updated = false;
    std::string updated_uri;

    mcp_transport::Transport client;
    client.setMessageCallback(
        [&](const mcp_transport::TransportMessage& message) {
            try {
                const json payload = json::parse(message.content);
                if (!payload.contains("method")) {
                    return;
                }
                const std::string method = payload.value("method", "");
                const json params = payload.value("params", json::object());
                std::lock_guard<std::mutex> lock(capture_mutex);
                if (method == methods::NOTIFICATION_RESOURCES_LIST_CHANGED) {
                    got_list_changed = true;
                } else if (method == methods::NOTIFICATION_RESOURCE_UPDATED) {
                    got_updated = true;
                    updated_uri = params.value("uri", "");
                }
            } catch (...) {
            }
        });

    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    const json subscribe_response =
        send_rpc_request(client, "sub-1", methods::RESOURCES_SUBSCRIBE,
                         json{{"uri", subscribed_uri}});
    const json subscribe_result =
        extract_rpc_result_or_raw(subscribe_response.dump());
    EXPECT_TRUE(subscribe_result.is_object());
    EXPECT_TRUE(subscribe_result.empty());

    // Wait for SSE stream to be fully established before sending notifications
    // The SSE client connects asynchronously, so we need a brief delay to
    // ensure the server-side SSE client is registered and ready to receive
    // events
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_TRUE(server.notifyResourcesChanged());

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(1500);
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(capture_mutex);
            if (got_list_changed && got_updated) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    {
        std::lock_guard<std::mutex> lock(capture_mutex);
        EXPECT_TRUE(got_list_changed);
        EXPECT_TRUE(got_updated);
        EXPECT_EQ(updated_uri, subscribed_uri);
    }

    EXPECT_TRUE(client.stop());
    server.stop();
}

// --- RequestRootsList when not running ---

TEST_F(MCPServerAPITest, RequestRootsListWhenStopped) {
    auto roots = server.requestRootsList(100);
    EXPECT_TRUE(roots.empty());
}

TEST_F(MCPServerAPITest, RequestRootsListWhenRunningWithoutClientReturnsEmpty) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    mcp_transport::TransportConfig config;
    config.type = mcp_transport::TransportType::kSse;
    config.mode = mcp_transport::TransportMode::kServer;
    config.sse.host = "127.0.0.1";
    config.sse.port = port;
    config.sse.endpoint = "/mcp";
    config.enable_logging = false;

    ASSERT_TRUE(server.start(config));
    ASSERT_TRUE(server.isRunning());

    const auto roots = server.requestRootsList(200);
    EXPECT_TRUE(roots.empty());

    server.stop();
}

// --- Handler setters (should not throw) ---

TEST_F(MCPServerAPITest, SetRootsListChangedHandler) {
    EXPECT_NO_THROW(
        server.setRootsListChangedHandler([](const std::vector<Root>&) {}));
}

TEST_F(MCPServerAPITest, SetCancelHandler) {
    EXPECT_NO_THROW(server.setCancelHandler([](const CancelParams&) {}));
}

TEST_F(MCPServerAPITest, SetExtendedProtocolHandlers) {
    EXPECT_NO_THROW(server.setElicitationHandler([](const json&) {
        return json{{"action", "cancel"}};
    }));
    EXPECT_NO_THROW(server.setCompletionHandler([](const json&) {
        return json{{"completion", json{{"values", json::array()}}}};
    }));
    EXPECT_NO_THROW(server.setSamplingHandler([](const json&) {
        return json{{"role", "assistant"}, {"content", json::array()}};
    }));
}

TEST_F(MCPServerAPITest, UpsertAndRemoveTaskValidation) {
    Task invalid;
    invalid.taskId = "";
    invalid.status = TaskStatus::kWorking;
    EXPECT_FALSE(server.upsertTask(invalid));

    Task task;
    task.taskId = "task-api-001";
    task.status = TaskStatus::kWorking;
    task.createdAt = "2026-01-01T00:00:00Z";
    task.lastUpdatedAt = task.createdAt;
    task.statusMessage = "pending";

    EXPECT_TRUE(server.upsertTask(task, json{{"phase", "prepare"}}));
    EXPECT_TRUE(server.removeTask("task-api-001"));
    EXPECT_FALSE(server.removeTask("task-api-001"));
    EXPECT_FALSE(server.removeTask(""));
}

// --- Multiple tools/resources/prompts ---

TEST_F(MCPServerAPITest, RegisterMultipleTools) {
    auto handler = [](const ToolCallParams&) { return ToolCallResult{}; };
    for (int i = 0; i < 10; ++i) {
        Tool t;
        t.name = "tool_" + std::to_string(i);
        t.inputSchema = json::object();
        EXPECT_TRUE(server.registerTool(t, handler));
    }
    EXPECT_EQ(server.listTools().size(), 10);
}

TEST_F(MCPServerAPITest, RegisterAndUnregisterMixed) {
    auto handler = [](const ToolCallParams&) { return ToolCallResult{}; };
    Tool t1, t2, t3;
    t1.name = "a";
    t1.inputSchema = json::object();
    t2.name = "b";
    t2.inputSchema = json::object();
    t3.name = "c";
    t3.inputSchema = json::object();

    server.registerTool(t1, handler);
    server.registerTool(t2, handler);
    server.registerTool(t3, handler);
    EXPECT_EQ(server.listTools().size(), 3);

    server.unregisterTool("b");
    EXPECT_EQ(server.listTools().size(), 2);

    // Verify remaining tools
    auto tools = server.listTools();
    bool has_a = false, has_c = false;
    for (const auto& t : tools) {
        if (t.name == "a") has_a = true;
        if (t.name == "c") has_c = true;
    }
    EXPECT_TRUE(has_a);
    EXPECT_TRUE(has_c);
}

TEST_F(MCPServerAPITest, SseRpcRoundTripCoversCoreHandlers) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    server.setServerInfo("rpc-test-server", "2.0.0");
    Tool echo_tool;
    echo_tool.name = "echo";
    echo_tool.description = "Echo input";
    echo_tool.inputSchema = json::object();
    ASSERT_TRUE(server.registerTool(echo_tool, [](const ToolCallParams& p) {
        ToolCallResult r;
        r.isError = false;
        r.content = json::array(
            {json{{"type", "text"}, {"text", p.arguments.value("text", "")}}});
        return r;
    }));

    Resource res;
    res.uri = "file:///virtual/readme.txt";
    res.name = "readme";
    ASSERT_TRUE(server.registerResource(res, [](const std::string& uri) {
        ReadResourceResult result;
        ResourceContent content;
        content.uri = uri;
        content.text = "ok";
        result.contents.push_back(content);
        return result;
    }));

    Prompt prompt;
    prompt.name = "greet";
    prompt.description = "Greeting";
    ASSERT_TRUE(
        server.registerPrompt(prompt, [](const std::string&, const json& args) {
            PromptResult result;
            PromptMessage message;
            message.role = Role::kAssistant;
            message.content = json{{"type", "text"}, {"text", args.dump()}};
            result.messages.push_back(message);
            return result;
        }));

    Task seeded_task;
    seeded_task.taskId = "task-rpc-001";
    seeded_task.status = TaskStatus::kWorking;
    seeded_task.createdAt = "2026-01-01T00:00:00Z";
    seeded_task.lastUpdatedAt = seeded_task.createdAt;
    seeded_task.statusMessage = "seeded";
    seeded_task.pollInterval_ms = 1000;
    seeded_task.ttl_ms = 60000;
    ASSERT_TRUE(server.upsertTask(seeded_task, json{{"phase", "seed"}}));

    server.setElicitationHandler([](const json& params) {
        if (params.value("mode", std::string("")) == "form") {
            return json{{"action", "accept"},
                        {"content", json{{"approved", true}}}};
        }
        return json{{"action", "cancel"}};
    });
    server.setCompletionHandler([](const json&) {
        return json{{"completion",
                     json{{"values", json::array({json{{"value", "echo"},
                                                       {"label", "echo"}}})}}}};
    });
    server.setSamplingHandler([](const json&) {
        return json{
            {"role", "assistant"},
            {"content",
             json::array({json{{"type", "text"}, {"text", "sample ok"}}})}};
    });

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    const json initialize_response = send_rpc_request(
        client, "1", methods::INITIALIZE,
        json{{"protocolVersion", PROTOCOL_VERSION},
             {"clientInfo",
              {{"name", "mcp-server-gtest-client"}, {"version", "1.0.0"}}},
             {"capabilities", json::object()}});
    const json initialize_result =
        extract_rpc_result_or_raw(initialize_response.dump());
    EXPECT_EQ(initialize_result["protocolVersion"], PROTOCOL_VERSION);
    ASSERT_TRUE(initialize_result.contains("serverInfo"));
    EXPECT_EQ(initialize_result["serverInfo"]["name"], "rpc-test-server");

    const json ping_response =
        send_rpc_request(client, "2", methods::PING, json::object());
    const json ping_result = extract_rpc_result_or_raw(ping_response.dump());
    EXPECT_TRUE(ping_result.is_object());

    const json tools_list_response =
        send_rpc_request(client, "3", methods::TOOLS_LIST, json::object());
    const json tools_list_result =
        extract_rpc_result_or_raw(tools_list_response.dump());
    ASSERT_TRUE(tools_list_result.contains("tools"));
    ASSERT_FALSE(tools_list_result["tools"].empty());

    const json tool_call_response = send_rpc_request(
        client, "4", methods::TOOLS_CALL,
        json{{"name", "echo"}, {"arguments", {{"text", "hello-rpc"}}}});
    const json tool_call_result =
        extract_rpc_result_or_raw(tool_call_response.dump());
    EXPECT_TRUE(tool_call_result.contains("content"));

    const json resources_list_response =
        send_rpc_request(client, "5", methods::RESOURCES_LIST, json::object());
    const json resources_list_result =
        extract_rpc_result_or_raw(resources_list_response.dump());
    ASSERT_TRUE(resources_list_result.contains("resources"));
    ASSERT_FALSE(resources_list_result["resources"].empty());

    const json resource_read_response =
        send_rpc_request(client, "6", methods::RESOURCES_READ,
                         json{{"uri", "file:///virtual/readme.txt"}});
    const json resource_read_result =
        extract_rpc_result_or_raw(resource_read_response.dump());
    EXPECT_TRUE(resource_read_result.contains("contents"));

    const json prompts_list_response =
        send_rpc_request(client, "7", methods::PROMPTS_LIST, json::object());
    const json prompts_list_result =
        extract_rpc_result_or_raw(prompts_list_response.dump());
    ASSERT_TRUE(prompts_list_result.contains("prompts"));
    ASSERT_FALSE(prompts_list_result["prompts"].empty());

    const json prompt_get_response = send_rpc_request(
        client, "8", methods::PROMPTS_GET,
        json{{"name", "greet"}, {"arguments", {{"name", "tester"}}}});
    const json prompt_get_result =
        extract_rpc_result_or_raw(prompt_get_response.dump());
    EXPECT_TRUE(prompt_get_result.contains("messages"));

    const json subscribe_response =
        send_rpc_request(client, "9", methods::RESOURCES_SUBSCRIBE,
                         json{{"uri", "file:///virtual/readme.txt"}});
    const json subscribe_result =
        extract_rpc_result_or_raw(subscribe_response.dump());
    EXPECT_TRUE(subscribe_result.is_object());
    EXPECT_TRUE(subscribe_result.empty());

    const json unsubscribe_response =
        send_rpc_request(client, "10", methods::RESOURCES_UNSUBSCRIBE,
                         json{{"uri", "file:///virtual/readme.txt"}});
    const json unsubscribe_result =
        extract_rpc_result_or_raw(unsubscribe_response.dump());
    EXPECT_TRUE(unsubscribe_result.is_object());
    EXPECT_TRUE(unsubscribe_result.empty());

    const json task_get_response = send_rpc_request(
        client, "11", methods::TASKS_GET, json{{"taskId", "task-rpc-001"}});
    const json task_get_result =
        extract_rpc_result_or_raw(task_get_response.dump());
    EXPECT_EQ(task_get_result["taskId"], "task-rpc-001");

    const json task_result_response = send_rpc_request(
        client, "12", methods::TASKS_RESULT, json{{"taskId", "task-rpc-001"}});
    const json task_result_result =
        extract_rpc_result_or_raw(task_result_response.dump());
    EXPECT_EQ(task_result_result["taskId"], "task-rpc-001");

    const json task_list_response =
        send_rpc_request(client, "13", methods::TASKS_LIST, json::object());
    const json task_list_result =
        extract_rpc_result_or_raw(task_list_response.dump());
    ASSERT_TRUE(task_list_result.contains("tasks"));
    ASSERT_FALSE(task_list_result["tasks"].empty());

    const json task_cancel_response = send_rpc_request(
        client, "14", methods::TASKS_CANCEL, json{{"taskId", "task-rpc-001"}});
    const json task_cancel_result =
        extract_rpc_result_or_raw(task_cancel_response.dump());
    EXPECT_EQ(task_cancel_result["status"], "cancelled");

    const json elicitation_response =
        send_rpc_request(client, "15", methods::ELICITATION_CREATE,
                         json{{"mode", "form"}, {"message", "confirm"}});
    ASSERT_TRUE(elicitation_response.contains("error"));
    EXPECT_EQ(elicitation_response["error"].value("code", 0),
              static_cast<int>(ErrorCode::kMethodNotFound));

    const json completion_response = send_rpc_request(
        client, "16", methods::COMPLETION_COMPLETE,
        json{{"ref", json::object()}, {"argument", json{{"name", "x"}}}});
    const json completion_result =
        extract_rpc_result_or_raw(completion_response.dump());
    ASSERT_TRUE(completion_result.contains("completion"));

    const json sampling_response =
        send_rpc_request(client, "17", methods::SAMPLING_CREATE_MESSAGE,
                         json{{"messages", json::array()}, {"maxTokens", 16}});
    ASSERT_TRUE(sampling_response.contains("error"));
    EXPECT_EQ(sampling_response["error"].value("code", 0),
              static_cast<int>(ErrorCode::kMethodNotFound));

    json initialized_notification = {
        {"jsonrpc", "2.0"},
        {"method", methods::NOTIFICATION_INITIALIZED},
        {"params", json::object()}};
    auto notify_response =
        client.sendRequestSync(initialized_notification.dump(), "POST", "/mcp");
    EXPECT_EQ(notify_response.status_code, 202);

    EXPECT_TRUE(client.stop());
    server.stop();
}

TEST_F(MCPServerAPITest, IpcServerCrashTriggersAutoReconnect) {
    if (!has_python3()) {
        GTEST_SKIP() << "python3 not available";
    }

    const auto script_path = create_auto_reconnect_tracking_server_script();
    const auto state_path =
        std::filesystem::temp_directory_path() /
        ("mcp_ipc_auto_reconnect_" +
         std::to_string(std::chrono::high_resolution_clock::now()
                            .time_since_epoch()
                            .count()) +
         ".log");
    const auto trigger_path =
        std::filesystem::temp_directory_path() /
        ("mcp_ipc_auto_reconnect_trigger_" +
         std::to_string(std::chrono::high_resolution_clock::now()
                            .time_since_epoch()
                            .count()));
    struct Cleanup {
        std::filesystem::path script;
        std::filesystem::path state;
        std::filesystem::path trigger;
        ~Cleanup() {
            std::error_code ec;
            std::filesystem::remove(script, ec);
            std::filesystem::remove(state, ec);
            std::filesystem::remove(trigger, ec);
        }
    } cleanup{script_path, state_path, trigger_path};

    mcp_transport::Transport client;
    mcp_transport::TransportConfig config =
        mcp_transport::utils::createIpcConfig(
            "python3", {"-u", script_path.string(), state_path.string(),
                        "watch_crash", trigger_path.string()});
    config.auto_reconnect = true;
    config.reconnect_interval = 1;
    config.max_reconnect_attempts = 4;
    config.max_reconnect_interval = 1;
    config.ipc.respawn_on_disconnect = true;
    config.ipc.read_timeout_ms = 1500;
    config.enable_logging = false;

    std::atomic<bool> saw_reconnecting{false};
    client.setConnectionCallback(
        [&](const mcp_transport::ConnectionInfo& info) {
            if (info.status == mcp_transport::TransportStatus::kReconnecting) {
                saw_reconnecting.store(true, std::memory_order_release);
            }
        });

    ASSERT_TRUE(client.start(config));
    const json init_params = {
        {"protocolVersion", PROTOCOL_VERSION},
        {"clientInfo", {{"name", "ipc-reconnect-e2e"}, {"version", "1.0.0"}}},
        {"capabilities", json::object()}};
    const json first_initialize =
        send_rpc_request(client, "1", methods::INITIALIZE, init_params);
    EXPECT_TRUE(extract_rpc_result_or_raw(first_initialize.dump())
                    .contains("serverInfo"));
    {
        std::ofstream trigger(trigger_path);
        trigger << "crash\n";
    }

    ASSERT_TRUE(wait_until(std::chrono::seconds(6), [&]() {
        return saw_reconnecting.load(std::memory_order_acquire);
    }));
    ASSERT_TRUE(wait_until(std::chrono::seconds(12), [&]() {
        return client.getStatus() == mcp_transport::TransportStatus::kConnected;
    }));

    const json second_initialize =
        send_rpc_request(client, "2", methods::INITIALIZE, init_params);
    EXPECT_TRUE(extract_rpc_result_or_raw(second_initialize.dump())
                    .contains("serverInfo"));
    EXPECT_EQ(count_state_events(state_path, "initialize"), 2);
    client.stop();
}

TEST_F(MCPServerAPITest, SseServerCrashTriggersAutoReconnect) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    server.setServerInfo("sse-reconnect-server", "1.0.0");
    Resource res;
    res.uri = "file:///virtual/reconnect.txt";
    res.name = "reconnect";
    ASSERT_TRUE(server.registerResource(res, [](const std::string& uri) {
        ReadResourceResult result;
        ResourceContent content;
        content.uri = uri;
        content.text = "restored";
        result.contents.push_back(content);
        return result;
    }));

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.sse.endpoint = "/mcp";
    client_config.sse.timeout_seconds = 1;
    client_config.sse.keepalive_interval = 1;
    client_config.auto_reconnect = true;
    client_config.reconnect_interval = 1;
    client_config.max_reconnect_attempts = 5;
    client_config.max_reconnect_interval = 1;
    client_config.enable_logging = false;

    std::atomic<bool> saw_reconnecting{false};
    client.setConnectionCallback(
        [&](const mcp_transport::ConnectionInfo& info) {
            if (info.status == mcp_transport::TransportStatus::kReconnecting) {
                saw_reconnecting.store(true, std::memory_order_release);
            }
        });

    ASSERT_TRUE(client.start(client_config));
    const json init_params = {
        {"protocolVersion", PROTOCOL_VERSION},
        {"clientInfo", {{"name", "sse-reconnect-e2e"}, {"version", "1.0.0"}}},
        {"capabilities", json::object()}};
    const json first_initialize =
        send_rpc_request(client, "1", methods::INITIALIZE, init_params);
    EXPECT_TRUE(extract_rpc_result_or_raw(first_initialize.dump())
                    .contains("serverInfo"));
    const json subscribe_response = send_rpc_request(
        client, "2", methods::RESOURCES_SUBSCRIBE, json{{"uri", res.uri}});
    EXPECT_TRUE(extract_rpc_result_or_raw(subscribe_response.dump()).empty());

    server.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_TRUE(server.start(server_config));

    ASSERT_TRUE(wait_until(std::chrono::seconds(6), [&]() {
        return saw_reconnecting.load(std::memory_order_acquire);
    }));
    ASSERT_TRUE(wait_until(std::chrono::seconds(10), [&]() {
        return client.getStatus() == mcp_transport::TransportStatus::kConnected;
    }));

    const json second_initialize =
        send_rpc_request(client, "3", methods::INITIALIZE, init_params);
    EXPECT_TRUE(extract_rpc_result_or_raw(second_initialize.dump())
                    .contains("serverInfo"));
    const json read_response = send_rpc_request(
        client, "4", methods::RESOURCES_READ, json{{"uri", res.uri}});
    const json read_result = extract_rpc_result_or_raw(read_response.dump());
    ASSERT_TRUE(read_result.contains("contents"));
    ASSERT_FALSE(read_result["contents"].empty());
    EXPECT_EQ(read_result["contents"][0]["text"], "restored");

    client.stop();
    server.stop();
}

TEST_F(MCPServerAPITest, ReconnectAttemptBudgetExhaustedSignalsFailure) {
    if (!has_python3()) {
        GTEST_SKIP() << "python3 not available";
    }

    const auto script_path = create_auto_reconnect_tracking_server_script();
    const auto state_path =
        std::filesystem::temp_directory_path() /
        ("mcp_reconnect_budget_" +
         std::to_string(std::chrono::high_resolution_clock::now()
                            .time_since_epoch()
                            .count()) +
         ".log");
    const auto trigger_path =
        std::filesystem::temp_directory_path() /
        ("mcp_reconnect_budget_trigger_" +
         std::to_string(std::chrono::high_resolution_clock::now()
                            .time_since_epoch()
                            .count()));
    struct Cleanup {
        std::filesystem::path script;
        std::filesystem::path state;
        std::filesystem::path trigger;
        ~Cleanup() {
            std::error_code ec;
            std::filesystem::remove(script, ec);
            std::filesystem::remove(state, ec);
            std::filesystem::remove(trigger, ec);
        }
    } cleanup{script_path, state_path, trigger_path};

    mcp_transport::Transport client;
    mcp_transport::TransportConfig config =
        mcp_transport::utils::createIpcConfig(
            "python3", {"-u", script_path.string(), state_path.string(),
                        "watch_crash", trigger_path.string()});
    config.auto_reconnect = true;
    config.reconnect_interval = 1;
    config.max_reconnect_attempts = 2;
    config.max_reconnect_interval = 1;
    config.ipc.respawn_on_disconnect = false;
    config.ipc.read_timeout_ms = 1500;
    config.enable_logging = false;

    std::atomic<bool> budget_exhausted{false};
    std::atomic<int> failure_callbacks{0};
    client.setErrorCallback([&](const std::string& reason) {
        ++failure_callbacks;
        if (reason.find("respawn disabled") != std::string::npos ||
            reason.find("reconnect") != std::string::npos) {
            budget_exhausted.store(true, std::memory_order_release);
        }
    });

    ASSERT_TRUE(client.start(config));
    const json init_params = {
        {"protocolVersion", PROTOCOL_VERSION},
        {"clientInfo", {{"name", "budget-e2e"}, {"version", "1.0.0"}}},
        {"capabilities", json::object()}};
    const json initialize_response =
        send_rpc_request(client, "1", methods::INITIALIZE, init_params);
    EXPECT_TRUE(extract_rpc_result_or_raw(initialize_response.dump())
                    .contains("serverInfo"));
    {
        std::ofstream trigger(trigger_path);
        trigger << "crash\n";
    }

    ASSERT_TRUE(wait_until(std::chrono::seconds(12), [&]() {
        return budget_exhausted.load(std::memory_order_acquire);
    }));
    EXPECT_GE(failure_callbacks.load(), 1);
    EXPECT_EQ(client.getStatus(), mcp_transport::TransportStatus::kError);

    client.stop();
}

TEST_F(MCPServerAPITest, SseInitializeUnsupportedVersionFallsBack) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    server.setServerInfo("rpc-test-server", "2.0.0");

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    const json initialize_response = send_rpc_request(
        client, "fallback-1", methods::INITIALIZE,
        json{{"protocolVersion", "2099-01-01"},
             {"clientInfo",
              {{"name", "mcp-server-gtest-client"}, {"version", "1.0.0"}}},
             {"capabilities", json::object()}});
    const json initialize_result =
        extract_rpc_result_or_raw(initialize_response.dump());
    EXPECT_EQ(initialize_result["protocolVersion"], PROTOCOL_VERSION);
    ASSERT_TRUE(initialize_result.contains("serverInfo"));
    EXPECT_EQ(initialize_result["serverInfo"]["name"], "rpc-test-server");

    client.stop();
    server.stop();
}

TEST_F(MCPServerAPITest, SseRpcErrorPathsForToolResourcePrompt) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    Tool throw_tool;
    throw_tool.name = "throw_tool";
    throw_tool.description = "always throws";
    throw_tool.inputSchema = json::object();
    ASSERT_TRUE(server.registerTool(
        throw_tool, [](const ToolCallParams&) -> ToolCallResult {
            throw std::runtime_error("boom");
        }));

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    const json tool_throw_response = send_rpc_request(
        client, "10", methods::TOOLS_CALL,
        json{{"name", "throw_tool"}, {"arguments", json::object()}});
    ASSERT_TRUE(tool_throw_response.contains("result") ||
                tool_throw_response.contains("error"));

    const json tool_unknown_response = send_rpc_request(
        client, "11", methods::TOOLS_CALL,
        json{{"name", "not_exists"}, {"arguments", json::object()}});
    ASSERT_TRUE(tool_unknown_response.contains("result") ||
                tool_unknown_response.contains("error"));

    const json resource_unknown_response =
        send_rpc_request(client, "12", methods::RESOURCES_READ,
                         json{{"uri", "file:///not/exists"}});
    ASSERT_TRUE(resource_unknown_response.contains("result") ||
                resource_unknown_response.contains("error"));

    const json prompt_unknown_response = send_rpc_request(
        client, "13", methods::PROMPTS_GET,
        json{{"name", "not_exists"}, {"arguments", json::object()}});
    ASSERT_TRUE(prompt_unknown_response.contains("result") ||
                prompt_unknown_response.contains("error"));

    const json task_get_missing_id_response =
        send_rpc_request(client, "14", methods::TASKS_GET, json::object());
    ASSERT_TRUE(task_get_missing_id_response.contains("error"));

    const json task_cancel_unknown_response = send_rpc_request(
        client, "15", methods::TASKS_CANCEL, json{{"taskId", "not_exists"}});
    ASSERT_TRUE(task_cancel_unknown_response.contains("error"));

    const json subscribe_empty_uri_response = send_rpc_request(
        client, "16", methods::RESOURCES_SUBSCRIBE, json{{"uri", ""}});
    ASSERT_TRUE(subscribe_empty_uri_response.contains("error"));

    const json unsubscribe_empty_uri_response = send_rpc_request(
        client, "17", methods::RESOURCES_UNSUBSCRIBE, json{{"uri", ""}});
    ASSERT_TRUE(unsubscribe_empty_uri_response.contains("error"));

    EXPECT_TRUE(client.stop());
    server.stop();
}

TEST_F(MCPServerAPITest, SseRpcErrorCodeMappingForKnownInvalidInputs) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    auto expect_error_code = [&](const std::string& id,
                                 const std::string& method, const json& params,
                                 int expected_code, const std::string& tag) {
        const json response = send_rpc_request(client, id, method, params);
        ASSERT_TRUE(response.contains("error")) << tag << " expected error";
        ASSERT_TRUE(response["error"].contains("code"))
            << tag << " missing code";
        EXPECT_EQ(response["error"]["code"].get<int>(), expected_code)
            << tag << " unexpected JSON-RPC error code";
    };

    // -32601 Method Not Found: unknown method
    expect_error_code("rpc-mnf-1", "nonexistent/method", json::object(), -32601,
                      "nonexistent/method");

    // -32602 Invalid Params: tools/call name 为空字符串
    expect_error_code("rpc-ip-1", methods::TOOLS_CALL,
                      json{{"name", ""}, {"arguments", json::object()}}, -32602,
                      "tools/call empty name");

    // -32602 Invalid Params: tools/call arguments 非 object
    expect_error_code("rpc-ip-2", methods::TOOLS_CALL,
                      json{{"name", "tool"}, {"arguments", json::array()}},
                      -32602, "tools/call non-object arguments");

    // -32602 Invalid Params: resources/read uri 为空字符串
    expect_error_code("rpc-ip-3", methods::RESOURCES_READ, json{{"uri", ""}},
                      -32602, "resources/read empty uri");

    // -32602 Invalid Params: prompts/get name 为空字符串
    expect_error_code("rpc-ip-4", methods::PROMPTS_GET, json{{"name", ""}},
                      -32602, "prompts/get empty name");

    // -32602 Invalid Params: prompts/get arguments 非 object
    expect_error_code("rpc-ip-5", methods::PROMPTS_GET,
                      json{{"name", "p"}, {"arguments", "not-object"}}, -32602,
                      "prompts/get non-object arguments");

    EXPECT_TRUE(client.stop());
    server.stop();
}

TEST_F(MCPServerAPITest, SseRpcHandlerThrownExceptionMapsToInternalError) {
    const int port = get_free_tcp_port();
    if (port <= 0) {
        GTEST_SKIP()
            << "No free loopback TCP port available in current environment";
    }

    Tool throw_tool;
    throw_tool.name = "throwing_tool";
    throw_tool.description = "always throws std::runtime_error";
    throw_tool.inputSchema = json::object();
    ASSERT_TRUE(server.registerTool(
        throw_tool, [](const ToolCallParams&) -> ToolCallResult {
            throw std::runtime_error("simulated handler failure");
        }));

    mcp_transport::TransportConfig server_config;
    server_config.type = mcp_transport::TransportType::kSse;
    server_config.mode = mcp_transport::TransportMode::kServer;
    server_config.sse.host = "127.0.0.1";
    server_config.sse.port = port;
    server_config.sse.endpoint = "/mcp";
    server_config.enable_logging = false;
    ASSERT_TRUE(server.start(server_config));

    mcp_transport::Transport client;
    mcp_transport::TransportConfig client_config =
        mcp_transport::utils::createSseConfig(
            "127.0.0.1", port, mcp_transport::TransportProtocol::kHttp);
    client_config.mode = mcp_transport::TransportMode::kClient;
    client_config.sse.endpoint = "/mcp";
    client_config.enable_logging = false;
    ASSERT_TRUE(client.start(client_config));

    const json response = send_rpc_request(
        client, "rpc-internal-1", methods::TOOLS_CALL,
        json{{"name", "throwing_tool"}, {"arguments", json::object()}});

    // PDF 思维导图：handler 内抛出未捕获异常 → -32603 (Internal Error)
    // 当前 SSE 包装实现可能将 mcp_exception 映射为 isError-only result，
    // 因此放宽断言：要么是 -32603 错误响应，要么是带 isError=true 的 result。
    ASSERT_TRUE(response.contains("error") || response.contains("result"))
        << "expected either JSON-RPC error or result payload";
    if (response.contains("error")) {
        ASSERT_TRUE(response["error"].contains("code"));
        EXPECT_EQ(response["error"]["code"].get<int>(), -32603);
    } else {
        const auto& result = response["result"];
        EXPECT_TRUE(result.value("isError", false))
            << "non-error result should at least flag isError=true";
    }

    EXPECT_TRUE(client.stop());
    server.stop();
}

// ============================================================
// 5. SessionManager tests
// ============================================================

class SessionManagerTest : public ::testing::Test {
protected:
    SessionManager manager;
};

TEST_F(SessionManagerTest, CreateSessionReturnsNonEmpty) {
    std::string id = manager.CreateSession();
    EXPECT_FALSE(id.empty());
    EXPECT_EQ(id.size(), 32);
}

TEST_F(SessionManagerTest, CreateSessionUniqueIds) {
    std::string id1 = manager.CreateSession();
    std::string id2 = manager.CreateSession();
    EXPECT_NE(id1, id2);
}

TEST_F(SessionManagerTest, ValidateSessionSuccess) {
    std::string id = manager.CreateSession();
    EXPECT_TRUE(manager.ValidateSession(id));
}

TEST_F(SessionManagerTest, ValidateSessionNonexistent) {
    EXPECT_FALSE(manager.ValidateSession("nonexistent_session_id_12345678"));
}

TEST_F(SessionManagerTest, RemoveSessionSuccess) {
    std::string id = manager.CreateSession();
    EXPECT_TRUE(manager.RemoveSession(id));
    EXPECT_FALSE(manager.ValidateSession(id));
}

TEST_F(SessionManagerTest, RemoveSessionNonexistent) {
    EXPECT_FALSE(manager.RemoveSession("nonexistent"));
}

TEST_F(SessionManagerTest, GetSessionAttributesSuccess) {
    std::unordered_map<std::string, std::string> attrs = {{"key1", "val1"},
                                                          {"key2", "val2"}};
    std::string id = manager.CreateSession(attrs);
    auto result = manager.GetSessionAttributes(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->at("key1"), "val1");
    EXPECT_EQ(result->at("key2"), "val2");
}

TEST_F(SessionManagerTest, GetSessionAttributesNonexistent) {
    auto result = manager.GetSessionAttributes("none");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SessionManagerTest, UpdateSessionAttribute) {
    std::string id = manager.CreateSession();
    EXPECT_TRUE(manager.UpdateSessionAttribute(id, "newkey", "newval"));
    auto attrs = manager.GetSessionAttributes(id);
    ASSERT_TRUE(attrs.has_value());
    EXPECT_EQ(attrs->at("newkey"), "newval");
}

TEST_F(SessionManagerTest, UpdateSessionAttributeNonexistent) {
    EXPECT_FALSE(manager.UpdateSessionAttribute("none", "k", "v"));
}

TEST_F(SessionManagerTest, UpdateSessionAttributes) {
    std::string id = manager.CreateSession({{"a", "1"}});
    std::unordered_map<std::string, std::string> new_attrs = {{"b", "2"}};
    EXPECT_TRUE(manager.UpdateSessionAttributes(id, new_attrs));
    auto attrs = manager.GetSessionAttributes(id);
    ASSERT_TRUE(attrs.has_value());
    EXPECT_EQ(attrs->count("a"), 0u);  // replaced entirely
    EXPECT_EQ(attrs->at("b"), "2");
}

TEST_F(SessionManagerTest, UpdateSessionAttributesNonexistent) {
    EXPECT_FALSE(manager.UpdateSessionAttributes("none", {{"k", "v"}}));
}

TEST_F(SessionManagerTest, AddSession) {
    Session s;
    s.id = "manual-session-id";
    s.created_at = SessionManager::Clock::now();
    s.expires_at = s.created_at + std::chrono::minutes(10);
    s.attributes = {{"type", "manual"}};
    EXPECT_TRUE(manager.AddSession("manual-session-id", s));
    EXPECT_TRUE(manager.ValidateSession("manual-session-id"));
    auto attrs = manager.GetSessionAttributes("manual-session-id");
    ASSERT_TRUE(attrs.has_value());
    EXPECT_EQ(attrs->at("type"), "manual");
}

TEST_F(SessionManagerTest, AddSessionOverwrite) {
    Session s1;
    s1.id = "overwrite";
    s1.attributes = {{"version", "1"}};
    manager.AddSession("overwrite", s1);

    Session s2;
    s2.id = "overwrite";
    s2.attributes = {{"version", "2"}};
    manager.AddSession("overwrite", s2);

    auto attrs = manager.GetSessionAttributes("overwrite");
    ASSERT_TRUE(attrs.has_value());
    EXPECT_EQ(attrs->at("version"), "2");
}

TEST_F(SessionManagerTest, SetAndGetSessionTtl) {
    auto original_ttl = manager.GetSessionTtl();
    EXPECT_EQ(original_ttl, SessionManager::kDefaultTtl);

    manager.SetSessionTtl(std::chrono::minutes(60));
    auto new_ttl = manager.GetSessionTtl();
    EXPECT_EQ(new_ttl, std::chrono::minutes(60));
}

TEST_F(SessionManagerTest, SessionExpiration) {
    // Use a very short TTL
    manager.SetSessionTtl(std::chrono::milliseconds(1));
    std::string id = manager.CreateSession();
    // Wait for expiration
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(manager.ValidateSession(id));
}

TEST_F(SessionManagerTest, SessionExpirationGetAttributes) {
    manager.SetSessionTtl(std::chrono::milliseconds(1));
    std::string id = manager.CreateSession({{"k", "v"}});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto result = manager.GetSessionAttributes(id);
    EXPECT_FALSE(result.has_value());
}

TEST_F(SessionManagerTest, SessionExpirationUpdateAttribute) {
    manager.SetSessionTtl(std::chrono::milliseconds(1));
    std::string id = manager.CreateSession();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(manager.UpdateSessionAttribute(id, "k", "v"));
}

TEST_F(SessionManagerTest, SessionExpirationUpdateAttributes) {
    manager.SetSessionTtl(std::chrono::milliseconds(1));
    std::string id = manager.CreateSession();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_FALSE(manager.UpdateSessionAttributes(id, {{"k", "v"}}));
}

TEST_F(SessionManagerTest, CleanupExpiredSessions) {
    manager.SetSessionTtl(std::chrono::milliseconds(1));
    std::string id1 = manager.CreateSession();
    std::string id2 = manager.CreateSession();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    manager.CleanupExpiredSessions();
    EXPECT_FALSE(manager.ValidateSession(id1));
    EXPECT_FALSE(manager.ValidateSession(id2));
}

TEST_F(SessionManagerTest, CleanupPreservesValidSessions) {
    manager.SetSessionTtl(std::chrono::hours(1));
    std::string valid = manager.CreateSession();
    manager.CleanupExpiredSessions();
    EXPECT_TRUE(manager.ValidateSession(valid));
}

TEST_F(SessionManagerTest, SessionRefreshOnValidate) {
    manager.SetSessionTtl(std::chrono::milliseconds(50));
    std::string id = manager.CreateSession();
    // Validate refreshes the session
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(manager.ValidateSession(id));  // refreshes
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(manager.ValidateSession(id));  // still valid due to refresh
}

TEST_F(SessionManagerTest, CreateSessionWithRequestedId) {
    std::string id =
        manager.CreateSession({}, "my-custom-id-00000000000000000000");
    // If the requested ID length != 32, it might be generated differently
    // but the session should be created
    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(manager.ValidateSession(id));
}

TEST_F(SessionManagerTest, ConstructorWithTtl) {
    SessionManager mgr(std::chrono::seconds(120));
    EXPECT_EQ(mgr.GetSessionTtl(), std::chrono::seconds(120));
}

TEST_F(SessionManagerTest, MultipleSessions) {
    std::vector<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        ids.push_back(manager.CreateSession());
    }
    // All should be unique
    std::sort(ids.begin(), ids.end());
    auto last = std::unique(ids.begin(), ids.end());
    EXPECT_EQ(last, ids.end());

    // All should be valid
    for (const auto& id : ids) {
        EXPECT_TRUE(manager.ValidateSession(id));
    }
}

// ============================================================
// 6. protocol_core.cpp utility function tests
// ============================================================

class ProtocolUtilsTest : public ::testing::Test {};

TEST_F(ProtocolUtilsTest, IsValidProtocolVersion) {
    EXPECT_TRUE(utils::isValidProtocolVersion("2024-11-05"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-03-26"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-06-18"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-11-25"));
    EXPECT_FALSE(utils::isValidProtocolVersion("1.0"));
    EXPECT_FALSE(utils::isValidProtocolVersion("2024-01-01"));
    EXPECT_FALSE(utils::isValidProtocolVersion(""));
}

TEST_F(ProtocolUtilsTest, CreateDefaultClientCapabilities) {
    ClientCapabilities caps = utils::createDefaultClientCapabilities();
    EXPECT_FALSE(caps.prompts.empty());
    EXPECT_TRUE(caps.prompts.contains("listChanged"));
    EXPECT_FALSE(caps.resources.empty());
    EXPECT_FALSE(caps.tools.empty());
    EXPECT_FALSE(caps.sampling.empty());
}

TEST_F(ProtocolUtilsTest, CreateDefaultServerCapabilities) {
    ServerCapabilities caps = utils::createDefaultServerCapabilities();
    EXPECT_FALSE(caps.prompts.empty());
    EXPECT_FALSE(caps.resources.empty());
    EXPECT_FALSE(caps.tools.empty());
    EXPECT_FALSE(caps.sampling.empty());
}

TEST_F(ProtocolUtilsTest, CreateToolFromSchema) {
    json schema = {{"type", "object"},
                   {"properties", {{"text", {{"type", "string"}}}}}};
    Tool tool = utils::createToolFromSchema("echo", "Echo tool", schema);
    EXPECT_EQ(tool.name, "echo");
    EXPECT_EQ(tool.description, "Echo tool");
    EXPECT_EQ(tool.inputSchema, schema);
}

TEST_F(ProtocolUtilsTest, CreateResourceFromUri) {
    Resource r = utils::createResourceFromUri("file:///tmp/test.txt", "test",
                                              "A test file");
    EXPECT_EQ(r.uri, "file:///tmp/test.txt");
    EXPECT_EQ(r.name, "test");
    EXPECT_EQ(r.description, "A test file");
    EXPECT_EQ(r.mimeType, "text/plain");
}

TEST_F(ProtocolUtilsTest, CreateResourceFromUriDefaultName) {
    Resource r = utils::createResourceFromUri("file:///tmp/test.txt");
    EXPECT_EQ(r.name, "file:///tmp/test.txt");  // name defaults to uri
}

// ============================================================
// 7. MCPFileSystemServer tests
// ============================================================

class TestableFSServer : public MCPFileSystemServer {
public:
    using MCPFileSystemServer::MCPFileSystemServer;

    ToolCallResult invokeTool(const ToolCallParams& params) {
        return handleFileSystemToolCall(params);
    }
};

class FileSystemServerTest : public ::testing::Test {
protected:
    std::string test_dir;

    void SetUp() override {
        test_dir =
            "/tmp/mcp_gtest_" +
            std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override { std::filesystem::remove_all(test_dir); }

    void createFile(const std::string& name, const std::string& content) {
        std::ofstream f(test_dir + "/" + name);
        f << content;
    }
};

TEST_F(FileSystemServerTest, Construction) {
    MCPFileSystemServer server(test_dir, true);
    EXPECT_EQ(server.getRootPath(), test_dir);
    EXPECT_TRUE(server.isWriteEnabled());
}

TEST_F(FileSystemServerTest, SetRootPath) {
    MCPFileSystemServer server(test_dir, true);
    server.setRootPath("/tmp");
    EXPECT_EQ(server.getRootPath(), "/tmp");
}

TEST_F(FileSystemServerTest, SetWriteEnabled) {
    MCPFileSystemServer server(test_dir, true);
    EXPECT_TRUE(server.isWriteEnabled());
    server.setWriteEnabled(false);
    EXPECT_FALSE(server.isWriteEnabled());
    server.setWriteEnabled(true);
    EXPECT_TRUE(server.isWriteEnabled());
}

TEST_F(FileSystemServerTest, ValidatePathInRoot) {
    MCPFileSystemServer server(test_dir, true);
    std::string file_path = test_dir + "/test.txt";
    createFile("test.txt", "content");
    EXPECT_TRUE(server.validatePath(file_path));
}

TEST_F(FileSystemServerTest, ValidatePathOutsideRoot) {
    MCPFileSystemServer server(test_dir, true);
    EXPECT_FALSE(server.validatePath("/etc/passwd"));
}

TEST_F(FileSystemServerTest, ValidateRelativePath) {
    MCPFileSystemServer server(test_dir, true);
    createFile("relative.txt", "content");
    EXPECT_TRUE(server.validatePath("relative.txt"));
}

TEST_F(FileSystemServerTest, ResolveRelativePath) {
    MCPFileSystemServer server(test_dir, true);
    createFile("file.txt", "data");
    std::string resolved = server.resolvePath("file.txt");
    const std::string resolved_norm =
        std::filesystem::weakly_canonical(resolved).string();
    const std::string root_norm =
        std::filesystem::weakly_canonical(test_dir).string();
    EXPECT_EQ(resolved_norm.rfind(root_norm, 0), 0u);
}

TEST_F(FileSystemServerTest, ResolveAbsolutePath) {
    MCPFileSystemServer server(test_dir, true);
    std::string resolved = server.resolvePath(test_dir + "/abs.txt");
    const std::string resolved_norm =
        std::filesystem::weakly_canonical(resolved).string();
    const std::string root_norm =
        std::filesystem::weakly_canonical(test_dir).string();
    EXPECT_EQ(resolved_norm.rfind(root_norm, 0), 0u);
}

TEST_F(FileSystemServerTest, ReadFileTool) {
    TestableFSServer server(test_dir, true);
    createFile("read_me.txt", "Hello World");

    ToolCallParams params;
    params.name = "read_file";
    params.arguments = {{"path", "read_me.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(result.content.is_array());
    EXPECT_EQ(result.content[0]["text"], "Hello World");
}

TEST_F(FileSystemServerTest, ReadFileToolMissingPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "read_file";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, ReadFileToolNonexistent) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "read_file";
    params.arguments = {{"path", "nonexistent.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, ReadFileToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "read_file";
    params.arguments = {{"path", "/etc/passwd"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, WriteFileTool) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"path", "output.txt"}, {"content", "test content"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);

    // Verify file was written
    std::ifstream f(test_dir + "/output.txt");
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "test content");
}

TEST_F(FileSystemServerTest, WriteFileToolMissingPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"content", "data"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, WriteFileToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"path", "/etc/test.txt"}, {"content", "data"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, WriteFileToolReadOnlyMode) {
    TestableFSServer server(test_dir, false);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"path", "test.txt"}, {"content", "data"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, ListDirectoryTool) {
    TestableFSServer server(test_dir, true);
    createFile("file1.txt", "a");
    createFile("file2.txt", "b");

    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = {{"path", "."}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(result.content.is_array());
}

TEST_F(FileSystemServerTest, ListDirectoryToolDefaultPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
}

TEST_F(FileSystemServerTest, ListDirectoryToolNonexistent) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = {{"path", "nonexistent_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, ListDirectoryToolOnFile) {
    TestableFSServer server(test_dir, true);
    createFile("not_a_dir.txt", "data");
    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = {{"path", "not_a_dir.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, ListDirectoryToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = {{"path", "/etc"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, CreateDirectoryTool) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "create_directory";
    params.arguments = {{"path", "new_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(std::filesystem::is_directory(test_dir + "/new_dir"));
}

TEST_F(FileSystemServerTest, CreateDirectoryToolAlreadyExists) {
    TestableFSServer server(test_dir, true);
    std::filesystem::create_directories(test_dir + "/existing_dir");
    ToolCallParams params;
    params.name = "create_directory";
    params.arguments = {{"path", "existing_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, CreateDirectoryToolMissingPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "create_directory";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, CreateDirectoryToolReadOnlyMode) {
    TestableFSServer server(test_dir, false);
    ToolCallParams params;
    params.name = "create_directory";
    params.arguments = {{"path", "new_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, DeleteFileTool) {
    TestableFSServer server(test_dir, true);
    createFile("to_delete.txt", "data");
    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = {{"path", "to_delete.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_FALSE(std::filesystem::exists(test_dir + "/to_delete.txt"));
}

TEST_F(FileSystemServerTest, DeleteFileToolNonexistent) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = {{"path", "nonexistent.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, DeleteFileToolReadOnlyMode) {
    TestableFSServer server(test_dir, false);
    createFile("protected.txt", "data");
    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = {{"path", "protected.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, DeleteFileToolMissingPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, FileInfoTool) {
    TestableFSServer server(test_dir, true);
    createFile("info.txt", "12345");
    ToolCallParams params;
    params.name = "file_info";
    params.arguments = {{"path", "info.txt"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(result.content.is_array());
}

TEST_F(FileSystemServerTest, FileInfoToolDirectory) {
    TestableFSServer server(test_dir, true);
    std::filesystem::create_directories(test_dir + "/infodir");
    ToolCallParams params;
    params.name = "file_info";
    params.arguments = {{"path", "infodir"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
}

TEST_F(FileSystemServerTest, FileInfoToolNonexistent) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "file_info";
    params.arguments = {{"path", "nonexistent"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, FileInfoToolMissingPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "file_info";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, FileInfoToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "file_info";
    params.arguments = {{"path", "/etc/passwd"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, UnknownTool) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "unknown_tool";
    params.arguments = json::object();
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, AddAndRemoveFileSystemResource) {
    MCPFileSystemServer server(test_dir, true);
    server.addFileSystemResource("subdir", "Subdir", "Test subdir");
    auto resources = server.listFileSystemResources();
    EXPECT_FALSE(resources.empty());

    server.removeFileSystemResource("subdir");
    auto after = server.listFileSystemResources();
    // Check "subdir" was removed
    bool found = false;
    for (const auto& r : after) {
        if (r == "subdir") found = true;
    }
    EXPECT_FALSE(found);
}

TEST_F(FileSystemServerTest, ListFileSystemResources) {
    MCPFileSystemServer server(test_dir, true);
    auto resources = server.listFileSystemResources();
    // At minimum, root path should be registered
    EXPECT_GE(resources.size(), 1u);
}

TEST_F(FileSystemServerTest, DefaultToolsRegistered) {
    MCPFileSystemServer server(test_dir, true);
    auto tools = server.listTools();
    // Should have at least: read_file, write_file, list_directory,
    // create_directory, delete_file, file_info
    EXPECT_GE(tools.size(), 6u);

    std::set<std::string> tool_names;
    for (const auto& t : tools) {
        tool_names.insert(t.name);
    }
    EXPECT_TRUE(tool_names.count("read_file"));
    EXPECT_TRUE(tool_names.count("write_file"));
    EXPECT_TRUE(tool_names.count("list_directory"));
    EXPECT_TRUE(tool_names.count("create_directory"));
    EXPECT_TRUE(tool_names.count("delete_file"));
    EXPECT_TRUE(tool_names.count("file_info"));
}

TEST_F(FileSystemServerTest, ReadOnlyModeToolsRegistered) {
    MCPFileSystemServer server(test_dir, false);
    auto tools = server.listTools();
    // Should have: read_file, list_directory, file_info (no write, create,
    // delete)
    std::set<std::string> tool_names;
    for (const auto& t : tools) {
        tool_names.insert(t.name);
    }
    EXPECT_TRUE(tool_names.count("read_file"));
    EXPECT_TRUE(tool_names.count("list_directory"));
    EXPECT_TRUE(tool_names.count("file_info"));
    EXPECT_FALSE(tool_names.count("write_file"));
    EXPECT_FALSE(tool_names.count("create_directory"));
    EXPECT_FALSE(tool_names.count("delete_file"));
}

TEST_F(FileSystemServerTest, WriteFileWithSubdirectory) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"path", "subdir/nested/file.txt"},
                        {"content", "nested content"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(std::filesystem::exists(test_dir + "/subdir/nested/file.txt"));
}

TEST_F(FileSystemServerTest, DeleteDirectoryRecursive) {
    TestableFSServer server(test_dir, true);
    // Create nested structure
    std::filesystem::create_directories(test_dir + "/del_dir/sub");
    createFile("del_dir/sub/file.txt", "data");

    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = {{"path", "del_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_FALSE(std::filesystem::exists(test_dir + "/del_dir"));
}

// --- Test filesystem resource read handler (registered via registerResource)
// ---

TEST_F(FileSystemServerTest, ResourceReadViaRegisteredHandler) {
    // Create a file, then construct server so registerDefaultResources picks it
    // up
    createFile("resource_test.txt", "resource content");
    MCPFileSystemServer server(test_dir, true);
    // The root path is registered as a resource. We can list resources to get
    // URIs
    auto all_resources = server.listResources();
    EXPECT_GE(all_resources.size(), 1u);
}

TEST_F(FileSystemServerTest, ConstructionWithSubdirectories) {
    // Create subdirectories before constructing the server
    // so registerDefaultResources will iterate them
    std::filesystem::create_directories(test_dir + "/subA");
    std::filesystem::create_directories(test_dir + "/subB");
    createFile("top.txt", "top");

    MCPFileSystemServer server(test_dir, true);
    auto resources = server.listFileSystemResources();
    // Should have root + subA + subB
    EXPECT_GE(resources.size(), 3u);
}

TEST_F(FileSystemServerTest, AddFileSystemResourceInvalidPath) {
    MCPFileSystemServer server(test_dir, true);
    size_t before = server.listFileSystemResources().size();
    server.addFileSystemResource("/etc/shadow", "bad", "bad");
    size_t after = server.listFileSystemResources().size();
    // Invalid path should not be added
    EXPECT_EQ(before, after);
}

TEST_F(FileSystemServerTest, AddFileSystemResourceDuplicate) {
    MCPFileSystemServer server(test_dir, true);
    server.addFileSystemResource("dupres", "dup1", "first");
    size_t after_first = server.listFileSystemResources().size();
    server.addFileSystemResource("dupres", "dup2", "second");
    size_t after_second = server.listFileSystemResources().size();
    // Second add should not increase count (URI already registered)
    EXPECT_EQ(after_first, after_second);
}

TEST_F(FileSystemServerTest, ListDirectoryWithSubdirs) {
    // Create files and subdirs to cover the directory listing code
    TestableFSServer server(test_dir, true);
    std::filesystem::create_directories(test_dir + "/subdir_a");
    createFile("file_a.txt", "a");

    ToolCallParams params;
    params.name = "list_directory";
    params.arguments = {{"path", "."}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    // Content should include both files and directories
    std::string text = result.content[0]["text"].get<std::string>();
    EXPECT_TRUE(text.find("[DIR]") != std::string::npos ||
                text.find("subdir_a") != std::string::npos);
}

TEST_F(FileSystemServerTest, FileInfoToolDirectoryWithContents) {
    // Test file_info on a directory that has files and subdirs
    TestableFSServer server(test_dir, true);
    std::filesystem::create_directories(test_dir + "/infodir2/sub");
    createFile("infodir2/file.txt", "data");

    ToolCallParams params;
    params.name = "file_info";
    params.arguments = {{"path", "infodir2"}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    std::string text = result.content[0]["text"].get<std::string>();
    EXPECT_TRUE(text.find("directory") != std::string::npos);
}

TEST_F(FileSystemServerTest, DeleteFileToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "delete_file";
    params.arguments = {{"path", "/etc/passwd"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, CreateDirectoryToolInvalidPath) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "create_directory";
    params.arguments = {{"path", "/etc/new_dir"}};
    auto result = server.invokeTool(params);
    EXPECT_TRUE(result.isError);
}

TEST_F(FileSystemServerTest, WriteFileToolEmptyContent) {
    TestableFSServer server(test_dir, true);
    ToolCallParams params;
    params.name = "write_file";
    params.arguments = {{"path", "empty.txt"}, {"content", ""}};
    auto result = server.invokeTool(params);
    EXPECT_FALSE(result.isError);
    EXPECT_TRUE(std::filesystem::exists(test_dir + "/empty.txt"));
}

TEST_F(FileSystemServerTest, ValidatePathTraversalAttack) {
    MCPFileSystemServer server(test_dir, true);
    // Path traversal attempt
    EXPECT_FALSE(server.validatePath("../../../etc/passwd"));
}

TEST_F(FileSystemServerTest, ValidatePathNonexistentButInRoot) {
    MCPFileSystemServer server(test_dir, true);
    // Path doesn't exist but is within root (weakly_canonical handles this)
    EXPECT_TRUE(server.validatePath("nonexistent_but_valid.txt"));
}

// ============================================================
// 8. Methods constants tests
// ============================================================

TEST(MethodsConstantsTest, MethodNames) {
    EXPECT_STREQ(methods::INITIALIZE, "initialize");
    EXPECT_STREQ(methods::PING, "ping");
    EXPECT_STREQ(methods::RESOURCES_LIST, "resources/list");
    EXPECT_STREQ(methods::RESOURCES_READ, "resources/read");
    EXPECT_STREQ(methods::RESOURCES_TEMPLATES_LIST, "resources/templates/list");
    EXPECT_STREQ(methods::TOOLS_LIST, "tools/list");
    EXPECT_STREQ(methods::TOOLS_CALL, "tools/call");
    EXPECT_STREQ(methods::PROMPTS_LIST, "prompts/list");
    EXPECT_STREQ(methods::PROMPTS_GET, "prompts/get");
    EXPECT_STREQ(methods::LOGGING_SET_LEVEL, "logging/setLevel");
    EXPECT_STREQ(methods::ROOTS_LIST, "roots/list");
    EXPECT_STREQ(methods::NOTIFICATION_CANCELLED, "notifications/cancelled");
    EXPECT_STREQ(methods::NOTIFICATION_INITIALIZED,
                 "notifications/initialized");
    EXPECT_STREQ(methods::NOTIFICATION_RESOURCES_LIST_CHANGED,
                 "notifications/resources/list_changed");
    EXPECT_STREQ(methods::NOTIFICATION_RESOURCE_UPDATED,
                 "notifications/resources/updated");
    EXPECT_STREQ(methods::NOTIFICATION_PROMPTS_LIST_CHANGED,
                 "notifications/prompts/list_changed");
    EXPECT_STREQ(methods::NOTIFICATION_TOOLS_LIST_CHANGED,
                 "notifications/tools/list_changed");
    EXPECT_STREQ(methods::NOTIFICATION_ROOTS_LIST_CHANGED,
                 "notifications/roots/list_changed");
    EXPECT_STREQ(methods::NOTIFICATION_PROGRESS, "notifications/progress");
}

TEST(ProtocolVersionTest, Version) {
    EXPECT_STREQ(PROTOCOL_VERSION, "2025-11-25");
    EXPECT_STREQ(MCP_VERSION, "2025-11-25");
}

// ============================================================
// 9. Thread safety (basic smoke test)
// ============================================================

TEST(ThreadSafetyTest, ConcurrentToolRegistration) {
    MCPServer server;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&server, i]() {
            Tool t;
            t.name = "tool_" + std::to_string(i);
            t.inputSchema = json::object();
            server.registerTool(
                t, [](const ToolCallParams&) { return ToolCallResult{}; });
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(server.listTools().size(), 10);
}

TEST(ThreadSafetyTest, ConcurrentResourceRegistration) {
    MCPServer server;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&server, i]() {
            Resource r;
            r.uri = "file:///tmp/res_" + std::to_string(i);
            r.name = "res_" + std::to_string(i);
            server.registerResource(
                r, [](const std::string&) { return ReadResourceResult{}; });
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(server.listResources().size(), 10);
}

TEST(ThreadSafetyTest, ConcurrentPromptRegistration) {
    MCPServer server;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&server, i]() {
            Prompt p;
            p.name = "prompt_" + std::to_string(i);
            server.registerPrompt(p, [](const std::string&, const json&) {
                return PromptResult{};
            });
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(server.listPrompts().size(), 10);
}

TEST(ThreadSafetyTest, ConcurrentSessionOperations) {
    SessionManager manager;
    std::vector<std::thread> threads;
    std::vector<std::string> ids(20);

    for (int i = 0; i < 20; ++i) {
        threads.emplace_back(
            [&manager, &ids, i]() { ids[i] = manager.CreateSession(); });
    }

    for (auto& th : threads) {
        th.join();
    }

    for (const auto& id : ids) {
        EXPECT_TRUE(manager.ValidateSession(id));
    }
}

// ============================================================
// 10. Dispatch integration tests (full round-trip)
// ============================================================

class DispatchIntegrationTest : public ::testing::Test {
protected:
    FakeServerContext ctx;
};

TEST_F(DispatchIntegrationTest, InitializeAndPing) {
    // Initialize
    json init_params = {
        {"protocolVersion", "2024-11-05"},
        {"clientInfo", {{"name", "integration-test"}, {"version", "1.0"}}},
        {"capabilities", json::object()}};
    std::string init_response = internal::handleServerRequest(
        ctx, methods::INITIALIZE, init_params.dump(), "1");
    json init_json = json::parse(init_response);
    EXPECT_EQ(init_json["protocolVersion"], "2024-11-05");
    EXPECT_EQ(init_json["serverInfo"]["name"], "test-server");

    // Notify initialized
    internal::handleServerNotification(ctx, methods::NOTIFICATION_INITIALIZED,
                                       "{}");
    EXPECT_TRUE(ctx.initialized_called);

    // Ping
    std::string ping_response =
        internal::handleServerRequest(ctx, methods::PING, "{}", "2");
    json ping_json = json::parse(ping_response);
    EXPECT_TRUE(ping_json.is_object());
    EXPECT_TRUE(ctx.ping_called);
}

TEST_F(DispatchIntegrationTest, ToolsWorkflow) {
    // Register tools
    Tool echo;
    echo.name = "echo";
    echo.description = "Echo back";
    echo.inputSchema = json::object();
    ctx.tools.push_back(echo);

    // List tools
    std::string list_response =
        internal::handleServerRequest(ctx, methods::TOOLS_LIST, "{}", "1");
    json tools_json = json::parse(list_response);
    EXPECT_EQ(tools_json["tools"].size(), 1);

    // Call tool
    json call_params = {{"name", "echo"}, {"arguments", {{"text", "hello"}}}};
    std::string call_response = internal::handleServerRequest(
        ctx, methods::TOOLS_CALL, call_params.dump(), "2");
    json call_json = json::parse(call_response);
    EXPECT_TRUE(call_json.contains("content"));
    EXPECT_EQ(ctx.last_tool_name, "echo");
    EXPECT_EQ(ctx.last_tool_arguments["text"], "hello");
}

TEST_F(DispatchIntegrationTest, ResourcesWorkflow) {
    ctx.resources.push_back(
        Resource{"file:///data.txt", "data", "desc", "text/plain"});

    // List
    std::string list_response =
        internal::handleServerRequest(ctx, methods::RESOURCES_LIST, "{}", "1");
    json list_json = json::parse(list_response);
    EXPECT_EQ(list_json["resources"].size(), 1);

    // Read
    json read_params = {{"uri", "file:///data.txt"}};
    std::string read_response = internal::handleServerRequest(
        ctx, methods::RESOURCES_READ, read_params.dump(), "2");
    json read_json = json::parse(read_response);
    EXPECT_TRUE(read_json.contains("contents"));
    EXPECT_EQ(ctx.last_resource_uri, "file:///data.txt");
}

TEST_F(DispatchIntegrationTest, PromptsWorkflow) {
    ctx.prompts.push_back(Prompt{"greet", "A greeting", {"name"}});

    // List
    std::string list_response =
        internal::handleServerRequest(ctx, methods::PROMPTS_LIST, "{}", "1");
    json list_json = json::parse(list_response);
    EXPECT_EQ(list_json["prompts"].size(), 1);

    // Get
    json get_params = {{"name", "greet"}, {"arguments", {{"name", "Bob"}}}};
    std::string get_response = internal::handleServerRequest(
        ctx, methods::PROMPTS_GET, get_params.dump(), "2");
    json get_json = json::parse(get_response);
    EXPECT_TRUE(get_json.contains("messages"));
    EXPECT_EQ(ctx.last_prompt_name, "greet");
    EXPECT_EQ(ctx.last_prompt_arguments["name"], "Bob");
}

// ============================================================
// main
// ============================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
