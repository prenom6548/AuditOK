#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

#include "client.h"
#include "common/transport.h"

namespace {

bool has_python3() {
    const int rc = std::system("python3 --version > /dev/null 2>&1");
    return rc == 0;
}

std::filesystem::path create_mock_server_script() {
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() /
                      ("mcp_client_mock_server_" + std::to_string(now) + ".py");

    std::ofstream ofs(path);
    ofs << R"PY(
import json
import sys

handled = {"tool": False, "resource": False, "prompt": False}
modes = {
    "templates_invalid": False,
    "tools_list_invalid_payload": False,
    "resources_list_invalid_payload": False,
    "prompts_list_invalid_payload": False,
    "ping_invalid_result": False,
    "ping_null_result": False,
    "ping_id_mismatch": False,
    "task_direct_result": False,
    "task_invalid_result": False,
    "task_result_invalid_payload": False,
    "task_missing_status": False,
    "task_list_invalid_payload": False,
    "resource_read_invalid_payload": False,
    "prompt_get_invalid_payload": False,
    "tool_call_invalid_payload": False,
    "completion_invalid_payload": False,
    "create_message_invalid_payload": False
}
pending = {}

def send(obj):
    sys.stdout.write(json.dumps(obj, ensure_ascii=False) + "\n")
    sys.stdout.flush()

def send_result(req_id, result):
    send({"jsonrpc": "2.0", "id": req_id, "result": result})

def send_error(req_id, code, message):
    send({"jsonrpc": "2.0", "id": req_id, "error": {"code": code, "message": message}})

def read_json_line():
    line = sys.stdin.readline()
    if not line:
        return None
    line = line.strip()
    if not line:
        return {}
    try:
        return json.loads(line)
    except Exception:
        return {"_raw": line}

def handle_request(msg):
    method = msg.get("method", "")
    req_id = msg.get("id", None)
    params = msg.get("params", {})

    if req_id is None and method == "notifications/initialized":
        send({
            "jsonrpc": "2.0",
            "id": "svr-tools-call",
            "method": "tools/call",
            "params": {"name": "echo-tool", "arguments": {"x": 1}}
        })
        pending["svr-tools-call"] = "tool"

        send({
            "jsonrpc": "2.0",
            "id": "svr-res-read",
            "method": "resources/read",
            "params": {"uri": "file:///tmp/a.txt"}
        })
        pending["svr-res-read"] = "resource"

        send({
            "jsonrpc": "2.0",
            "id": "svr-prompts-get",
            "method": "prompts/get",
            "params": {"name": "demo", "arguments": {"y": 2}}
        })
        pending["svr-prompts-get"] = "prompt"

        send({
            "jsonrpc": "2.0",
            "method": "notifications/tasks/status",
            "params": {"taskId": "task-notify-1", "status": "working", "statusMessage": "tick"}
        })

        send({
            "jsonrpc": "2.0",
            "method": "notifications/elicitation/complete",
            "params": {"elicitationId": "elic-1", "result": {"action": "accept", "content": {"ok": True}}}
        })

        send({
            "jsonrpc": "2.0",
            "method": "notifications/progress",
            "params": {"progressToken": "tok-1", "progress": 42.5, "total": 100.0, "message": "halfway"}
        })

        send({
            "jsonrpc": "2.0",
            "method": "$/cancelRequest",
            "params": {"requestId": "req-42", "reason": "user_abort"}
        })
        return

    if req_id is None:
        return

    if method == "initialize":
        meta = params.get("_meta", {})
        require_fallback = isinstance(meta, dict) and meta.get("progressToken") == "require-fallback"
        if require_fallback:
            client_info = params.get("clientInfo", {})
            capabilities = params.get("capabilities", {})
            roots_caps = capabilities.get("roots", {}) if isinstance(capabilities, dict) else {}
            if (not isinstance(client_info, dict) or
                client_info.get("name", "") != "sdk-client" or
                client_info.get("version", "") != "9.9.9" or
                not isinstance(roots_caps, dict) or
                roots_caps.get("listChanged") is not True):
                send_error(req_id, -32602, "Client info/capabilities fallback is missing")
                return
        send_result(req_id, {
            "protocolVersion": "2025-11-25",
            "capabilities": {
                "tools": {"listChanged": True},
                "resources": {"subscribe": True}
            },
            "serverInfo": {"name": "mock-server", "version": "1.0.0"}
        })
    elif method == "tools/list":
        if modes.get("tools_list_invalid_payload", False):
            send_result(req_id, {"tools": {}, "nextCursor": 123})
        else:
            result = {
                "tools": [{
                    "name": "echo",
                    "description": "Echo tool",
                    "inputSchema": {"type": "object"}
                }]
            }
            if "cursor" in params:
                result["nextCursor"] = str(params.get("cursor"))
            send_result(req_id, result)
    elif method == "tools/call":
        name = params.get("name", "")
        if name == "force_error":
            send_error(req_id, -32001, "forced tool error")
        elif modes.get("tool_call_invalid_payload", False):
            send_result(req_id, {"isError": "nope"})
        else:
            send_result(req_id, {
                "isError": False,
                "content": {
                    "tool": name,
                    "arguments": params.get("arguments", {})
                }
            })
    elif method == "resources/list":
        if modes.get("resources_list_invalid_payload", False):
            send_result(req_id, {"resources": "bad"})
        else:
            result = {
                "resources": [{
                    "uri": "file:///tmp/a.txt",
                    "name": "a.txt",
                    "description": "demo resource",
                    "mimeType": "text/plain"
                }]
            }
            if "cursor" in params:
                result["nextCursor"] = str(params.get("cursor"))
            send_result(req_id, result)
    elif method == "resources/templates/list":
        if modes.get("templates_invalid", False):
            send_result(req_id, {"unexpected": []})
        else:
            result = {
                "resourceTemplates": [{
                    "uriTemplate": "file:///tmp/{name}.txt",
                    "name": "tmp-template",
                    "description": "tmp template",
                    "mimeType": "text/plain"
                }]
            }
            if "cursor" in params:
                result["nextCursor"] = str(params.get("cursor"))
            send_result(req_id, result)
    elif method == "resources/read":
        if modes.get("resource_read_invalid_payload", False):
            send_result(req_id, {"contents": {"uri": params.get("uri", "")}})
        else:
            send_result(req_id, {
                "contents": [{
                    "uri": params.get("uri", ""),
                    "mimeType": "text/plain",
                    "text": "hello"
                }]
            })
    elif method == "prompts/list":
        if modes.get("prompts_list_invalid_payload", False):
            send_result(req_id, {"prompts": [], "nextCursor": {}})
        else:
            result = {
                "prompts": [{
                    "name": "demo-prompt",
                    "description": "Demo prompt",
                    "arguments": [{"name": "topic"}]
                }]
            }
            if "cursor" in params:
                result["nextCursor"] = str(params.get("cursor"))
            send_result(req_id, result)
    elif method == "prompts/get":
        name = params.get("name", "")
        if name == "force_error":
            send_error(req_id, -32002, "forced prompt error")
        elif modes.get("prompt_get_invalid_payload", False):
            send_result(req_id, {
                "description": "Prompt " + name,
                "messages": {"role": "assistant"}
            })
        else:
            send_result(req_id, {
                "description": "Prompt " + name,
                "messages": [{
                    "role": "assistant",
                    "content": {"type": "text", "text": "hi"}
                }]
            })
    elif method == "logging/setLevel":
        if params.get("level", "") == "templates-invalid":
            modes["templates_invalid"] = True
        if params.get("level", "") == "tools-list-invalid":
            modes["tools_list_invalid_payload"] = True
        if params.get("level", "") == "resources-list-invalid":
            modes["resources_list_invalid_payload"] = True
        if params.get("level", "") == "prompts-list-invalid":
            modes["prompts_list_invalid_payload"] = True
        if params.get("level", "") == "ping-invalid-result":
            modes["ping_invalid_result"] = True
        if params.get("level", "") == "ping-null-result":
            modes["ping_null_result"] = True
        if params.get("level", "") == "ping-id-mismatch":
            modes["ping_id_mismatch"] = True
        if params.get("level", "") == "task-direct-result":
            modes["task_direct_result"] = True
        if params.get("level", "") == "task-invalid-result":
            modes["task_invalid_result"] = True
        if params.get("level", "") == "task-result-invalid":
            modes["task_result_invalid_payload"] = True
        if params.get("level", "") == "task-missing-status":
            modes["task_missing_status"] = True
        if params.get("level", "") == "task-list-invalid":
            modes["task_list_invalid_payload"] = True
        if params.get("level", "") == "resource-read-invalid":
            modes["resource_read_invalid_payload"] = True
        if params.get("level", "") == "prompt-get-invalid":
            modes["prompt_get_invalid_payload"] = True
        if params.get("level", "") == "tool-call-invalid":
            modes["tool_call_invalid_payload"] = True
        if params.get("level", "") == "completion-invalid":
            modes["completion_invalid_payload"] = True
        if params.get("level", "") == "create-message-invalid":
            modes["create_message_invalid_payload"] = True
        if params.get("level", "") == "force-server-error":
            send_error(req_id, -32602, "Unsupported logging level")
            return
        send_result(req_id, {"method": method, "level": params.get("level", "")})
    elif method == "tasks/get":
        if modes.get("task_invalid_result", False):
            send_result(req_id, {"task": None})
            return
        if modes.get("task_missing_status", False):
            send_result(req_id, {"taskId": params.get("taskId", "")})
            return
        task = {"taskId": params.get("taskId", ""), "status": "running", "statusMessage": ""}
        if modes.get("task_direct_result", False):
            send_result(req_id, task)
        else:
            send_result(req_id, {"task": task})
    elif method == "tasks/result":
        if modes.get("task_result_invalid_payload", False):
            send_result(req_id, {"task": params.get("taskId", "")})
        else:
            send_result(req_id, {"taskId": params.get("taskId", ""), "result": {}})
    elif method == "tasks/list":
        if modes.get("task_list_invalid_payload", False):
            send_result(req_id, {"tasks": {}, "nextCursor": 123})
        else:
            send_result(req_id, {"tasks": [], "nextCursor": params.get("cursor", "")})
    elif method == "tasks/cancel":
        if modes.get("task_invalid_result", False):
            send_result(req_id, {"task": []})
            return
        task = {"taskId": params.get("taskId", ""), "status": "cancelled", "statusMessage": "cancelled"}
        if modes.get("task_direct_result", False):
            send_result(req_id, task)
        else:
            send_result(req_id, {"task": task})
    elif method == "elicitation/create":
        send_result(req_id, {"action": "accept", "content": {}})
    elif method == "completion/complete":
        if modes.get("completion_invalid_payload", False):
            send_result(req_id, {"completion": {"values": {"bad": True}}})
        else:
            send_result(req_id, {"completion": {"values": ["abc"], "hasMore": False, "total": 1}})
    elif method == "sampling/createMessage":
        if modes.get("create_message_invalid_payload", False):
            send_result(req_id, {"role": "assistant", "model": "test"})
        else:
            send_result(req_id, {"role": "assistant", "content": {"type": "text", "text": "hi"}, "model": "test", "stopReason": "end"})
    elif method == "resources/subscribe":
        send_result(req_id, {})
    elif method == "resources/unsubscribe":
        send_result(req_id, {})
    elif method == "ping":
        if modes.get("ping_id_mismatch", False):
            send_result(str(req_id) + "-mismatch", {})
        elif modes.get("ping_null_result", False):
            send_result(req_id, None)
        elif modes.get("ping_invalid_result", False):
            send_result(req_id, {"unexpected": True})
        else:
            send_result(req_id, {})
    else:
        send_error(req_id, -32601, "Method not found")

def main():
    while True:
        msg = read_json_line()
        if msg is None:
            return
        if not isinstance(msg, dict):
            continue
        if "method" in msg:
            handle_request(msg)
            continue

        msg_id = msg.get("id", "")
        if msg_id in pending and "result" in msg:
            kind = pending.pop(msg_id)
            result = msg["result"]
            if kind == "tool":
                content = result.get("content", {})
                handled["tool"] = isinstance(content, dict) and content.get("echo") == "tool"
            elif kind == "resource":
                contents = result.get("contents", [])
                handled["resource"] = isinstance(contents, list) and len(contents) > 0
            elif kind == "prompt":
                messages = result.get("messages", [])
                handled["prompt"] = isinstance(messages, list) and len(messages) > 0

if __name__ == "__main__":
    main()
)PY";
    return path;
}

std::filesystem::path create_tracking_server_script() {
    const auto now =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto path =
        std::filesystem::temp_directory_path() /
        ("mcp_client_tracking_server_" + std::to_string(now) + ".py");

    std::ofstream ofs(path);
    ofs << R"PY(
import json
import sys

state_path = sys.argv[1]
seen_ids = set()

def record(event):
    with open(state_path, "a", encoding="utf-8") as f:
        f.write(event + "\n")

def send(obj):
    sys.stdout.write(json.dumps(obj, ensure_ascii=False) + "\n")
    sys.stdout.flush()

def send_result(req_id, result):
    send({"jsonrpc": "2.0", "id": req_id, "result": result})

def send_error(req_id, code, message):
    send({"jsonrpc": "2.0", "id": req_id, "error": {"code": code, "message": message}})

def handle(msg):
    method = msg.get("method", "")
    req_id = msg.get("id", None)
    params = msg.get("params", {})
    if req_id is None:
        return
    should_record = req_id not in seen_ids
    seen_ids.add(req_id)
    if method == "initialize":
        if should_record:
            record("initialize")
        send_result(req_id, {
            "protocolVersion": "2025-11-25",
            "capabilities": {"resources": {"subscribe": True}},
            "serverInfo": {"name": "tracking-server", "version": "1.0.0"}
        })
    elif method == "resources/subscribe":
        uri = params.get("uri", "")
        if "fail-subscribe" in uri:
            send_error(req_id, -32000, "forced subscribe failure")
            return
        if should_record:
            record("subscribe " + uri)
        send_result(req_id, {})
    elif method == "resources/unsubscribe":
        uri = params.get("uri", "")
        if "fail-unsubscribe" in uri:
            send_error(req_id, -32001, "forced unsubscribe failure")
            return
        if should_record:
            record("unsubscribe " + uri)
        send_result(req_id, {})
    else:
        send_result(req_id, {})

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

void expect_invalid_params(const std::function<void()>& action,
                           const std::string& expected_message) {
    try {
        action();
        FAIL() << "Expected mcp_exception";
    } catch (const mcp::mcp_exception& e) {
        EXPECT_EQ(e.code(), mcp::ErrorCode::kInvalidParams);
        const std::string message = e.what();
        EXPECT_NE(message.find(expected_message), std::string::npos);
    } catch (const std::exception& e) {
        FAIL() << "Expected mcp_exception, got std::exception: " << e.what();
    } catch (...) {
        FAIL() << "Expected mcp_exception, got unknown exception";
    }
}

class MCPClientTest : public ::testing::Test {
protected:
    bool connect_with_fast_ipc(mcp::MCPClient& client) const {
        mcp_transport::TransportConfig config =
            mcp_transport::utils::createIpcConfig(
                "python3", {"-u", script_path_.string()});
        config.ipc.read_timeout_ms = 200;
        return client.connectWithConfig(config);
    }

    void SetUp() override {
        if (!has_python3()) {
            GTEST_SKIP() << "python3 not available";
        }
        script_path_ = create_mock_server_script();
        endpoint_ = "cmd:python3 -u \"" + script_path_.string() + "\"";
    }

    void TearDown() override {
        std::error_code ec;
        if (!script_path_.empty()) {
            std::filesystem::remove(script_path_, ec);
        }
    }

    std::filesystem::path script_path_;
    std::string endpoint_;
};

class MCPClientReconnectTest : public ::testing::Test {
protected:
    bool connect_tracking_server(mcp::MCPClient& client) const {
        mcp_transport::TransportConfig config =
            mcp_transport::utils::createIpcConfig(
                "python3", {"-u", script_path_.string(), state_path_.string()});
        config.ipc.read_timeout_ms = 200;
        return client.connectWithConfig(config);
    }

    void SetUp() override {
        if (!has_python3()) {
            GTEST_SKIP() << "python3 not available";
        }
        const auto now = std::chrono::high_resolution_clock::now()
                             .time_since_epoch()
                             .count();
        state_path_ =
            std::filesystem::temp_directory_path() /
            ("mcp_client_reconnect_state_" + std::to_string(now) + ".log");
        script_path_ = create_tracking_server_script();
    }

    void TearDown() override {
        std::error_code ec;
        if (!script_path_.empty()) {
            std::filesystem::remove(script_path_, ec);
        }
        if (!state_path_.empty()) {
            std::filesystem::remove(state_path_, ec);
        }
    }

    void initialize_client(mcp::MCPClient& client) const {
        mcp::InitializeParams params;
        params.protocolVersion = mcp::PROTOCOL_VERSION;
        params.clientInfo.name = "client-test";
        params.clientInfo.version = "1.0.0";
        ASSERT_NO_THROW(client.initialize(params));
    }

    std::filesystem::path script_path_;
    std::filesystem::path state_path_;
};

TEST_F(MCPClientTest, NewApisRoundTrip) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("info"));

    auto task_result = client.getTask("task-1");
    EXPECT_EQ(task_result.taskId, "task-1");

    auto task_output = client.getTaskResult("task-1");
    EXPECT_EQ(task_output.taskId, "task-1");
    EXPECT_TRUE(task_output.result.is_object());

    auto task_list = client.listTasks("cursor-1");
    EXPECT_EQ(task_list.nextCursor, "cursor-1");

    auto cancel_result = client.cancelTask("task-1");
    EXPECT_EQ(cancel_result.taskId, "task-1");

    auto completion_result = client.complete({{"query", "abc"}});
    ASSERT_EQ(completion_result.values.size(), 1u);
    EXPECT_EQ(completion_result.values[0], "abc");

    ASSERT_NO_THROW(client.subscribeResource("file:///tmp/a.txt"));
    ASSERT_NO_THROW(client.unsubscribeResource("file:///tmp/a.txt"));

    client.disconnect();
}

TEST_F(MCPClientTest, ListApisSupportCursorAndNextCursor) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    auto tools = client.listTools("cursor-tools");
    ASSERT_EQ(tools.tools.size(), 1u);
    EXPECT_EQ(tools.nextCursor, "cursor-tools");

    auto resources = client.listResources("cursor-resources");
    ASSERT_EQ(resources.resources.size(), 1u);
    EXPECT_EQ(resources.nextCursor, "cursor-resources");

    auto templates = client.listResourceTemplates("cursor-templates");
    ASSERT_EQ(templates.resourceTemplates.size(), 1u);
    EXPECT_EQ(templates.nextCursor, "cursor-templates");

    auto prompts = client.listPrompts("cursor-prompts");
    ASSERT_EQ(prompts.prompts.size(), 1u);
    EXPECT_EQ(prompts.nextCursor, "cursor-prompts");

    client.disconnect();
}

TEST_F(MCPClientTest, TaskApisAcceptDirectTaskResultPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("task-direct-result"));

    auto task_result = client.getTask("task-direct-1");
    EXPECT_EQ(task_result.taskId, "task-direct-1");

    auto cancel_result = client.cancelTask("task-direct-1");
    EXPECT_EQ(cancel_result.taskId, "task-direct-1");

    client.disconnect();
}

TEST_F(MCPClientTest, TaskApisThrowOnInvalidTaskPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("task-invalid-result"));

    EXPECT_THROW((void)client.getTask("task-invalid-1"), mcp::mcp_exception);
    EXPECT_THROW((void)client.cancelTask("task-invalid-1"), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, TaskResultInvalidPayloadFallsBackToRawResult) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("task-result-invalid"));

    const auto result = client.getTaskResult("task-invalid-result-1");
    EXPECT_EQ(result.taskId, "task-invalid-result-1");
    ASSERT_TRUE(result.result.is_object());
    EXPECT_EQ(result.result.value("task", ""), "task-invalid-result-1");

    client.disconnect();
}

TEST_F(MCPClientTest, TaskThrowsOnMissingStatusField) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("task-missing-status"));

    EXPECT_THROW((void)client.getTask("task-missing-status-1"),
                 mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, TaskListThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("task-list-invalid"));

    try {
        (void)client.listTasks("cursor-invalid");
        FAIL() << "Expected mcp_exception for invalid tasks/list payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid tasks/list payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, ListToolsThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("tools-list-invalid"));

    try {
        (void)client.listTools("cursor-invalid");
        FAIL() << "Expected mcp_exception for invalid tools/list payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid tools/list payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, ListResourcesThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("resources-list-invalid"));

    try {
        (void)client.listResources("cursor-invalid");
        FAIL() << "Expected mcp_exception for invalid resources/list payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid resources/list payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, ListPromptsThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("prompts-list-invalid"));

    try {
        (void)client.listPrompts("cursor-invalid");
        FAIL() << "Expected mcp_exception for invalid prompts/list payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid prompts/list payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, ReadResourceThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("resource-read-invalid"));

    try {
        (void)client.readResource("file:///tmp/a.txt");
        FAIL() << "Expected mcp_exception for invalid resources/read payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid resources/read payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, GetPromptThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("prompt-get-invalid"));

    try {
        (void)client.getPrompt("demo-prompt", {{"topic", "abc"}});
        FAIL() << "Expected mcp_exception for invalid prompts/get payload";
    } catch (const mcp::mcp_exception& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("Invalid prompts/get payload"),
                  std::string::npos);
        EXPECT_NE(message.find("payload="), std::string::npos);
    }

    client.disconnect();
}

TEST_F(MCPClientTest, CallToolThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("tool-call-invalid"));

    EXPECT_THROW((void)client.callTool("echo", {{"msg", "hello"}}),
                 mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, CompleteThrowsOnInvalidPayload) {
    mcp::MCPClient client;

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("completion-invalid"));

    EXPECT_THROW((void)client.complete({{"query", "abc"}}), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, InboundHandlersAreDispatched) {
    mcp::MCPClient client;

    bool tool_called = false;
    bool resource_called = false;
    bool prompt_called = false;

    client.setToolCallHandler([&](const mcp::ToolCallParams& params) {
        tool_called = true;
        mcp::ToolCallResult result;
        result.content = {{"echo", "tool"}, {"name", params.name}};
        return result;
    });

    client.setResourceReadHandler([&](const std::string& uri) {
        resource_called = true;
        mcp::ReadResourceResult result;
        mcp::ResourceContent content;
        content.uri = uri;
        content.text = "resource";
        result.contents.push_back(content);
        return result;
    });

    client.setPromptGetHandler(
        [&](const std::string& name, const mcp::json& arguments) {
            prompt_called = true;
            mcp::PromptResult result;
            result.description = "prompt";
            mcp::PromptMessage msg;
            msg.role = mcp::Role::kAssistant;
            msg.content = mcp::json{{"name", name}, {"arguments", arguments}};
            result.messages.push_back(msg);
            return result;
        });

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));
    client.initialized();

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(3000);
    while (std::chrono::steady_clock::now() < deadline) {
        if (tool_called && resource_called && prompt_called) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    ASSERT_NO_THROW(client.ping());

    EXPECT_TRUE(tool_called);
    EXPECT_TRUE(resource_called);
    EXPECT_TRUE(prompt_called);

    client.disconnect();
}

TEST_F(MCPClientTest, NotificationAndTypedHandlersReceiveServerSignals) {
    mcp::MCPClient client;

    std::atomic<bool> task_status_called{false};
    std::atomic<bool> elicitation_complete_called{false};
    std::atomic<bool> progress_called{false};
    std::atomic<bool> cancel_called{false};

    std::mutex capture_mutex;
    std::string task_id;
    std::string task_status;
    std::string elicitation_id;
    std::string elicitation_action;
    std::string progress_token;
    double progress_value = 0.0;
    std::string progress_message;
    std::string cancel_request_id;
    std::string cancel_reason;

    client.setNotificationHandler(
        mcp::methods::NOTIFICATION_TASKS_STATUS,
        [&](const std::string&, const mcp::json& params) {
            {
                std::lock_guard<std::mutex> lock(capture_mutex);
                task_id = params.value("taskId", "");
                task_status = params.value("status", "");
            }
            task_status_called = true;
        });

    client.setNotificationHandler(
        mcp::methods::NOTIFICATION_ELICITATION_COMPLETE,
        [&](const std::string&, const mcp::json& params) {
            {
                std::lock_guard<std::mutex> lock(capture_mutex);
                elicitation_id = params.value("elicitationId", "");
                if (params.contains("result") && params["result"].is_object()) {
                    elicitation_action = params["result"].value("action", "");
                }
            }
            elicitation_complete_called = true;
        });

    client.setProgressHandler([&](const mcp::ProgressParams& params) {
        {
            std::lock_guard<std::mutex> lock(capture_mutex);
            if (params.progressToken.is_string()) {
                progress_token = params.progressToken.get<std::string>();
            }
            progress_value = params.progress;
            progress_message = params.message;
        }
        progress_called = true;
    });

    client.setCancelRequestHandler([&](const mcp::CancelParams& params) {
        {
            std::lock_guard<std::mutex> lock(capture_mutex);
            if (params.requestId.is_string()) {
                cancel_request_id = params.requestId.get<std::string>();
            }
            cancel_reason = params.reason;
        }
        cancel_called = true;
    });

    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));
    client.initialized();

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(4000);
    while (std::chrono::steady_clock::now() < deadline) {
        if (task_status_called.load() && elicitation_complete_called.load() &&
            progress_called.load() && cancel_called.load()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(task_status_called.load());
    EXPECT_TRUE(elicitation_complete_called.load());
    EXPECT_TRUE(progress_called.load());
    EXPECT_TRUE(cancel_called.load());

    {
        std::lock_guard<std::mutex> lock(capture_mutex);
        EXPECT_EQ(task_id, "task-notify-1");
        EXPECT_EQ(task_status, "working");
        EXPECT_EQ(elicitation_id, "elic-1");
        EXPECT_EQ(elicitation_action, "accept");
        EXPECT_EQ(progress_token, "tok-1");
        EXPECT_DOUBLE_EQ(progress_value, 42.5);
        EXPECT_EQ(progress_message, "halfway");
        EXPECT_EQ(cancel_request_id, "req-42");
        EXPECT_EQ(cancel_reason, "user_abort");
    }

    client.disconnect();
}

TEST_F(MCPClientTest, CoreRequestApisRoundTripAndErrorSemantics) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    const auto tools = client.listTools();
    ASSERT_EQ(tools.tools.size(), 1u);
    EXPECT_EQ(tools.tools[0].name, "echo");

    auto tool_ok = client.callTool("echo", {{"msg", "hello"}});
    EXPECT_FALSE(tool_ok.isError);
    ASSERT_TRUE(tool_ok.content.is_object());
    EXPECT_EQ(tool_ok.content.value("tool", ""), "echo");

    auto tool_fail = client.callTool("force_error", mcp::json::object());
    EXPECT_TRUE(tool_fail.isError);
    EXPECT_FALSE(tool_fail.errorMessage.empty());

    const auto resources = client.listResources();
    ASSERT_EQ(resources.resources.size(), 1u);
    EXPECT_EQ(resources.resources[0].uri, "file:///tmp/a.txt");

    const auto templates = client.listResourceTemplates();
    ASSERT_EQ(templates.resourceTemplates.size(), 1u);
    EXPECT_EQ(templates.resourceTemplates[0].name, "tmp-template");

    const auto resource_content = client.readResource("file:///tmp/a.txt");
    ASSERT_EQ(resource_content.contents.size(), 1u);
    EXPECT_EQ(resource_content.contents[0].text, "hello");

    const auto prompts = client.listPrompts();
    ASSERT_EQ(prompts.prompts.size(), 1u);
    EXPECT_EQ(prompts.prompts[0].name, "demo-prompt");

    const auto prompt = client.getPrompt("demo-prompt", {{"topic", "abc"}});
    ASSERT_FALSE(prompt.messages.empty());

    client.disconnect();
}

TEST_F(MCPClientTest, StateAndMetadataApisAreObservable) {
    mcp::MCPClient client;

    client.setTransportType(mcp_transport::TransportType::kIpc);
    EXPECT_EQ(client.getTransportType(), mcp_transport::TransportType::kIpc);

    const std::vector<std::string> endpoints = {endpoint_,
                                                "http://127.0.0.1:8080/mcp"};
    client.setEndpoints(endpoints);
    EXPECT_EQ(client.getCurrentEndpoint(), endpoint_);
    const auto got_endpoints = client.getEndpoints();
    ASSERT_EQ(got_endpoints.size(), 2u);

    mcp::Root root_a;
    root_a.uri = "file:///workspace";
    root_a.name = "workspace";
    mcp::Root root_b;
    root_b.uri = "file:///tmp";
    root_b.name = "tmp";
    client.setRoots({root_a, root_b});
    const auto got_roots = client.getRoots();
    ASSERT_EQ(got_roots.size(), 2u);
    EXPECT_EQ(got_roots[0].name, "workspace");

    mcp::ClientCapabilities caps;
    caps.roots = {{"listChanged", true}};
    client.setCapabilities(caps);
    client.setClientInfo("sdk-client", "9.9.9");

    ASSERT_TRUE(client.connect(client.getCurrentEndpoint()));
    EXPECT_TRUE(client.isConnected());

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params._meta.progressToken = "require-fallback";
    ASSERT_NO_THROW(client.initialize(params));

    const auto server_info = client.getServerInfo();
    EXPECT_EQ(server_info.name, "mock-server");
    EXPECT_EQ(server_info.version, "1.0.0");

    const auto server_caps = client.getServerCapabilities();
    ASSERT_TRUE(server_caps.tools.is_object());
    EXPECT_TRUE(server_caps.tools.value("listChanged", false));
    ASSERT_TRUE(server_caps.resources.is_object());
    EXPECT_TRUE(server_caps.resources.value("subscribe", false));

    EXPECT_NE(client.getTransportStatus(),
              mcp_transport::TransportStatus::kDisconnected);

    client.disconnect();
    EXPECT_FALSE(client.isConnected());
}

TEST_F(MCPClientTest, DisconnectedRequestsFailFast) {
    mcp::MCPClient client;

    EXPECT_THROW(client.ping(), mcp::mcp_exception);
    EXPECT_THROW(client.listTools(), mcp::mcp_exception);
    EXPECT_THROW(client.listResources(), mcp::mcp_exception);

    const auto tool_result = client.callTool("echo", {{"x", 1}});
    EXPECT_TRUE(tool_result.isError);
    EXPECT_FALSE(tool_result.errorMessage.empty());

    EXPECT_NO_THROW(
        client.sendNotification("notifications/custom", {{"x", 1}}));
}

TEST_F(MCPClientTest, PingWithNonEmptyResultThrows) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("ping-invalid-result"));
    EXPECT_THROW(client.ping(), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, PingWithNullResultAccepted) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("ping-null-result"));
    ASSERT_NO_THROW(client.ping());

    client.disconnect();
}

TEST_F(MCPClientTest, PingWithMismatchedIdThrows) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("ping-id-mismatch"));
    EXPECT_THROW(client.ping(), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, ConnectWithConfigWorksAndInvalidEndpointThrows) {
    mcp::MCPClient endpoint_client;
    EXPECT_THROW(endpoint_client.connect(""), mcp::mcp_exception);

    mcp::MCPClient client;

    mcp_transport::TransportConfig invalid_config;
    invalid_config.type = mcp_transport::TransportType::kAuto;
    invalid_config.ipc.command.clear();
    invalid_config.sse.host.clear();
    EXPECT_FALSE(client.connectWithConfig(invalid_config));
    EXPECT_FALSE(client.isConnected());

    mcp_transport::TransportConfig config =
        mcp_transport::utils::createIpcConfig("python3",
                                              {"-u", script_path_.string()});
    config.ipc.read_timeout_ms = 1000;
    EXPECT_TRUE(client.connectWithConfig(config));
    EXPECT_TRUE(client.isConnected());

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    client.disconnect();
}

TEST_F(MCPClientTest, ConnectRejectsPortOutOfRange) {
    mcp::MCPClient client;
    expect_invalid_params(
        [&]() { (void)client.connect("http://127.0.0.1:99999/mcp"); },
        "Invalid port number");
    EXPECT_FALSE(client.isConnected());
}

TEST_F(MCPClientTest, ConnectRejectsNonNumericPort) {
    mcp::MCPClient client;
    expect_invalid_params(
        [&]() { (void)client.connect("http://127.0.0.1:abc/mcp"); },
        "Invalid port number");
    EXPECT_FALSE(client.isConnected());
}

TEST_F(MCPClientTest, ConnectRejectsUnsupportedScheme) {
    mcp::MCPClient client;
    expect_invalid_params([&]() { (void)client.connect("invalid://path"); },
                          "Unsupported endpoint scheme");
    expect_invalid_params([&]() { (void)client.connect("ftp://host/file"); },
                          "Unsupported endpoint scheme");
    EXPECT_FALSE(client.isConnected());
}

TEST_F(MCPClientTest, SetClientInfoRejectsEmptyName) {
    mcp::MCPClient client;
    expect_invalid_params([&]() { client.setClientInfo("", "1.0.0"); },
                          "Client name must be non-empty");
}

TEST_F(MCPClientTest, SetClientInfoRejectsEmptyVersion) {
    mcp::MCPClient client;
    expect_invalid_params([&]() { client.setClientInfo("my-client", ""); },
                          "Client version must be non-empty");
}

TEST_F(MCPClientTest, SetClientInfoRejectsBothEmpty) {
    mcp::MCPClient client;
    expect_invalid_params([&]() { client.setClientInfo("", ""); },
                          "Client name must be non-empty");
}

TEST_F(MCPClientTest, SetClientInfoAcceptsValid) {
    mcp::MCPClient client;
    EXPECT_NO_THROW(client.setClientInfo("my-client", "1.0.0"));
}

TEST_F(MCPClientTest, ReconnectHandlerInvokedOnSuccess) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    std::atomic<int> calls{0};
    client.setReconnectHandler([&](const mcp::ReconnectEvent& event) {
        ++calls;
        EXPECT_TRUE(event.success);
        EXPECT_EQ(event.status, mcp_transport::TransportStatus::kConnected);
        return true;
    });

    EXPECT_TRUE(client.reconnect());
    EXPECT_EQ(calls.load(), 1);
    client.disconnect();
}

TEST_F(MCPClientTest, ReconnectHandlerInvokedOnFailure) {
    mcp::MCPClient client;

    std::atomic<int> calls{0};
    client.setReconnectHandler([&](const mcp::ReconnectEvent& event) {
        ++calls;
        EXPECT_FALSE(event.success);
        EXPECT_FALSE(event.reason.empty());
        return false;
    });

    EXPECT_FALSE(client.reconnect());
    EXPECT_EQ(calls.load(), 1);
}

TEST_F(MCPClientReconnectTest, SubscribedUrisTrackedCorrectly) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_tracking_server(client));
    initialize_client(client);

    const std::string kept_uri = "file:///tmp/kept.txt";
    const std::string removed_uri = "file:///tmp/removed.txt";
    const std::string failed_subscribe_uri = "file:///tmp/fail-subscribe.txt";
    const std::string failed_unsubscribe_uri =
        "file:///tmp/fail-unsubscribe.txt";

    ASSERT_NO_THROW(client.subscribeResource(kept_uri));
    ASSERT_NO_THROW(client.subscribeResource(removed_uri));
    EXPECT_THROW(client.subscribeResource(failed_subscribe_uri),
                 mcp::mcp_exception);

    ASSERT_NO_THROW(client.subscribeResource(failed_unsubscribe_uri));
    EXPECT_THROW(client.unsubscribeResource(failed_unsubscribe_uri),
                 mcp::mcp_exception);
    ASSERT_NO_THROW(client.unsubscribeResource(removed_uri));

    EXPECT_TRUE(client.reconnect());

    EXPECT_EQ(count_state_events(state_path_, "initialize"), 2);
    EXPECT_EQ(count_state_events(state_path_, "subscribe " + kept_uri), 2);
    EXPECT_EQ(count_state_events(state_path_, "subscribe " + removed_uri), 1);
    EXPECT_EQ(
        count_state_events(state_path_, "subscribe " + failed_subscribe_uri),
        0);
    EXPECT_EQ(
        count_state_events(state_path_, "subscribe " + failed_unsubscribe_uri),
        2);
    EXPECT_EQ(count_state_events(state_path_, "unsubscribe " + removed_uri), 1);

    client.disconnect();
}

TEST_F(MCPClientReconnectTest, DefaultReconnectHandlerRestoresSession) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_tracking_server(client));
    initialize_client(client);

    const std::string uri = "file:///tmp/restored.txt";
    ASSERT_NO_THROW(client.subscribeResource(uri));

    EXPECT_TRUE(client.reconnect());

    EXPECT_EQ(count_state_events(state_path_, "initialize"), 2);
    EXPECT_EQ(count_state_events(state_path_, "subscribe " + uri), 2);

    client.disconnect();
}

TEST_F(MCPClientReconnectTest, ExplicitReconnectRecoversSession) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_tracking_server(client));
    initialize_client(client);

    const std::string first_uri = "file:///tmp/first.txt";
    const std::string second_uri = "file:///tmp/second.txt";
    ASSERT_NO_THROW(client.subscribeResource(first_uri));
    ASSERT_NO_THROW(client.subscribeResource(second_uri));

    EXPECT_TRUE(client.reconnect());

    EXPECT_EQ(count_state_events(state_path_, "initialize"), 2);
    EXPECT_EQ(count_state_events(state_path_, "subscribe " + first_uri), 2);
    EXPECT_EQ(count_state_events(state_path_, "subscribe " + second_uri), 2);

    client.disconnect();
}

TEST_F(MCPClientTest, ExplicitSseTransportRejectsNonHttpEndpoint) {
    mcp::MCPClient client;
    client.setTransportType(mcp_transport::TransportType::kSse);

    // Use a non-cmd, non-HTTP endpoint to trigger the SSE validation rejection.
    // "cmd:" endpoints are parsed as IPC regardless of preferred transport
    // type.
    try {
        (void)client.connect("not-a-valid-url");
        FAIL() << "Expected mcp_exception for non-http endpoint when "
                  "transport type is SSE";
    } catch (const mcp::mcp_exception& e) {
        EXPECT_EQ(e.code(), mcp::ErrorCode::kInvalidParams);
        const std::string message = e.what();
        EXPECT_NE(message.find("SSE transport requires HTTP(S) endpoint"),
                  std::string::npos);
    }
}

TEST_F(MCPClientTest, InitializeFallbackMissingDataThrows) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params._meta.progressToken = "require-fallback";
    EXPECT_THROW(client.initialize(params), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, PromptGetRemoteErrorThrows) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    EXPECT_THROW(client.getPrompt("force_error", mcp::json::object()),
                 mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, InvalidProtocolVersionThrowsBeforeRequestRoundTrip) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = "2099-01-01";
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    EXPECT_THROW(client.initialize(params), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, ListResourceTemplatesInvalidPayloadThrows) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    ASSERT_NO_THROW(client.setLoggingLevel("templates-invalid"));
    EXPECT_THROW(client.listResourceTemplates(), mcp::mcp_exception);

    client.disconnect();
}

TEST_F(MCPClientTest, EndpointOverrideTakesEffect) {
    mcp::MCPClient client;
    client.setEndpoints({"cmd:first", "cmd:second"});
    EXPECT_EQ(client.getCurrentEndpoint(), "cmd:first");

    client.setCurrentEndpoint(endpoint_);
    EXPECT_EQ(client.getCurrentEndpoint(), endpoint_);

    const auto endpoints = client.getEndpoints();
    ASSERT_EQ(endpoints.size(), 2u);
    EXPECT_EQ(endpoints[0], "cmd:first");
    EXPECT_EQ(endpoints[1], "cmd:second");
}

TEST_F(MCPClientTest, LoggingLevelAcceptsAllStandardLevels) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    for (const auto& level : {"debug", "info", "warning", "error"}) {
        EXPECT_NO_THROW(client.setLoggingLevel(level))
            << "setLoggingLevel(\"" << level << "\") should not throw";
    }

    client.disconnect();
}

TEST_F(MCPClientTest, LoggingLevelThrowsWhenServerReturnsError) {
    mcp::MCPClient client;
    ASSERT_TRUE(connect_with_fast_ipc(client));

    mcp::InitializeParams params;
    params.protocolVersion = mcp::PROTOCOL_VERSION;
    params.clientInfo.name = "client-test";
    params.clientInfo.version = "1.0.0";
    ASSERT_NO_THROW(client.initialize(params));

    EXPECT_THROW(client.setLoggingLevel("force-server-error"),
                 mcp::mcp_exception);

    client.disconnect();
}
