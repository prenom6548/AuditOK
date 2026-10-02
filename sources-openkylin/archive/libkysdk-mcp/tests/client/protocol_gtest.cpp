/**
 * @file protocol_gtest.cpp
 * @brief Comprehensive GTest suite for MCP Protocol Core and Message interfaces
 */

#include <gtest/gtest.h>

#include "logger.h"
#include "common/message.h"
#include "common/protocol.h"

using namespace mcp;

// ============================================================================
// Test Fixture for common setup
// ============================================================================
class MCPProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger if needed
        auto logger = mcp_logger::getModuleLogger("mcp_protocol_gtest");
        logger->info("Setting up test fixture");
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

// ============================================================================
// Tests for message.h - ErrorCode enum
// ============================================================================
TEST_F(MCPProtocolTest, ErrorCodeValues) {
    EXPECT_EQ(static_cast<int>(ErrorCode::kParseError), -32700);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInvalidRequest), -32600);
    EXPECT_EQ(static_cast<int>(ErrorCode::kMethodNotFound), -32601);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInvalidParams), -32602);
    EXPECT_EQ(static_cast<int>(ErrorCode::kInternalError), -32603);
    EXPECT_EQ(static_cast<int>(ErrorCode::kServerErrorStart), -32000);
    EXPECT_EQ(static_cast<int>(ErrorCode::kServerErrorEnd), -32099);
}

// ============================================================================
// Tests for message.h - mcp_exception
// ============================================================================
TEST_F(MCPProtocolTest, McpException) {
    mcp_exception ex(ErrorCode::kInvalidRequest, "Test error message");
    EXPECT_EQ(ex.code(), ErrorCode::kInvalidRequest);
    EXPECT_STREQ(ex.what(), "Test error message");
}

// ============================================================================
// Tests for message.h - request structure
// ============================================================================
TEST_F(MCPProtocolTest, RequestCreate) {
    auto req = request::create("test/method", json{{"key", "value"}});
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.method, "test/method");
    EXPECT_FALSE(req.id.is_null());
    EXPECT_EQ(req.params["key"], "value");
}

TEST_F(MCPProtocolTest, RequestCreateWithId) {
    auto req = request::createWithId("custom-id", "test/method",
                                     json{{"key", "value"}});
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.method, "test/method");
    EXPECT_EQ(req.id.get<std::string>(), "custom-id");
    EXPECT_EQ(req.params["key"], "value");
}

TEST_F(MCPProtocolTest, RequestCreateNotification) {
    auto req = request::createNotification("notifications/test",
                                           json{{"key", "value"}});
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.method, "notifications/test");
    EXPECT_TRUE(req.id.is_null());
    EXPECT_TRUE(req.isNotification());
    EXPECT_EQ(req.params["key"], "value");
}

TEST_F(MCPProtocolTest, RequestIsNotification) {
    auto req1 = request::create("test/method");
    EXPECT_FALSE(req1.isNotification());

    auto req2 = request::createNotification("notifications/test");
    EXPECT_TRUE(req2.isNotification());
}

TEST_F(MCPProtocolTest, RequestToJson) {
    auto req = request::create("test/method", json{{"key", "value"}});
    auto j = req.toJson();

    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["method"], "test/method");
    EXPECT_TRUE(j.contains("id"));
    EXPECT_EQ(j["params"]["key"], "value");
}

TEST_F(MCPProtocolTest, RequestToJsonNotification) {
    auto req = request::createNotification("notifications/test",
                                           json{{"key", "value"}});
    auto j = req.toJson();

    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["method"], "notifications/test");
    EXPECT_FALSE(j.contains("id"));
    EXPECT_EQ(j["params"]["key"], "value");
}

TEST_F(MCPProtocolTest, RequestFromJson) {
    json j = {{"jsonrpc", "2.0"},
              {"id", 123},
              {"method", "test/method"},
              {"params", {{"key", "value"}}}};

    auto req = request::fromJson(j);
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.id, 123);
    EXPECT_EQ(req.method, "test/method");
    EXPECT_EQ(req.params["key"], "value");
}

TEST_F(MCPProtocolTest, RequestFromJsonWithoutParams) {
    json j = {{"jsonrpc", "2.0"}, {"id", 456}, {"method", "test/method"}};

    auto req = request::fromJson(j);
    EXPECT_EQ(req.jsonrpc, "2.0");
    EXPECT_EQ(req.id, 456);
    EXPECT_EQ(req.method, "test/method");
    EXPECT_TRUE(req.params.empty());
}

// ============================================================================
// Tests for message.h - response structure
// ============================================================================
TEST_F(MCPProtocolTest, ResponseCreateSuccess) {
    json req_id = 123;
    json result_data = {{"key", "value"}};
    auto res = response::createSuccess(req_id, result_data);

    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 123);
    EXPECT_FALSE(res.isError());
    EXPECT_EQ(res.result["key"], "value");
}

TEST_F(MCPProtocolTest, ResponseCreateError) {
    json req_id = 456;
    auto res = response::createError(req_id, ErrorCode::kInvalidParams,
                                     "Invalid parameters");

    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 456);
    EXPECT_TRUE(res.isError());
    EXPECT_EQ(res.error["code"], static_cast<int>(ErrorCode::kInvalidParams));
    EXPECT_EQ(res.error["message"], "Invalid parameters");
}

TEST_F(MCPProtocolTest, ResponseCreateErrorWithData) {
    json req_id = 789;
    json error_data = {{"details", "More info"}};
    auto res = response::createError(req_id, ErrorCode::kMethodNotFound,
                                     "Method not found", error_data);

    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 789);
    EXPECT_TRUE(res.isError());
    EXPECT_EQ(res.error["code"], static_cast<int>(ErrorCode::kMethodNotFound));
    EXPECT_EQ(res.error["message"], "Method not found");
    EXPECT_EQ(res.error["data"]["details"], "More info");
}

TEST_F(MCPProtocolTest, ResponseIsError) {
    json req_id = 1;
    auto success_res = response::createSuccess(req_id);
    EXPECT_FALSE(success_res.isError());

    auto error_res =
        response::createError(req_id, ErrorCode::kInternalError, "Error");
    EXPECT_TRUE(error_res.isError());
}

TEST_F(MCPProtocolTest, ResponseToJsonSuccess) {
    json req_id = 123;
    json result_data = {{"key", "value"}};
    auto res = response::createSuccess(req_id, result_data);
    auto j = res.toJson();

    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["id"], 123);
    EXPECT_TRUE(j.contains("result"));
    EXPECT_FALSE(j.contains("error"));
    EXPECT_EQ(j["result"]["key"], "value");
}

TEST_F(MCPProtocolTest, ResponseToJsonError) {
    json req_id = 456;
    auto res =
        response::createError(req_id, ErrorCode::kParseError, "Parse error");
    auto j = res.toJson();

    EXPECT_EQ(j["jsonrpc"], "2.0");
    EXPECT_EQ(j["id"], 456);
    EXPECT_FALSE(j.contains("result"));
    EXPECT_TRUE(j.contains("error"));
    EXPECT_EQ(j["error"]["code"], static_cast<int>(ErrorCode::kParseError));
    EXPECT_EQ(j["error"]["message"], "Parse error");
}

TEST_F(MCPProtocolTest, ResponseFromJsonSuccess) {
    json j = {{"jsonrpc", "2.0"}, {"id", 123}, {"result", {{"key", "value"}}}};

    auto res = response::fromJson(j);
    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 123);
    EXPECT_FALSE(res.isError());
    EXPECT_EQ(res.result["key"], "value");
}

TEST_F(MCPProtocolTest, ResponseFromJsonError) {
    json j = {{"jsonrpc", "2.0"},
              {"id", 456},
              {"error",
               {{"code", static_cast<int>(ErrorCode::kInvalidRequest)},
                {"message", "Invalid request"}}}};

    auto res = response::fromJson(j);
    EXPECT_EQ(res.jsonrpc, "2.0");
    EXPECT_EQ(res.id, 456);
    EXPECT_TRUE(res.isError());
    EXPECT_EQ(res.error["code"], static_cast<int>(ErrorCode::kInvalidRequest));
    EXPECT_EQ(res.error["message"], "Invalid request");
}

// ============================================================================
// Tests for protocol.h - ClientCapabilities
// ============================================================================
TEST_F(MCPProtocolTest, ClientCapabilitiesToJson) {
    ClientCapabilities caps;
    caps.logging = json{{"level", "info"}};
    caps.prompts = json{{"listChanged", true}};
    caps.resources = json{{"subscribe", true}};
    caps.tools = json{{"listChanged", true}};
    caps.sampling = json{{"maxTokens", 4096}};

    auto j = caps.toJson();
    EXPECT_EQ(j["logging"]["level"], "info");
    EXPECT_EQ(j["prompts"]["listChanged"], true);
    EXPECT_EQ(j["resources"]["subscribe"], true);
    EXPECT_EQ(j["tools"]["listChanged"], true);
    EXPECT_EQ(j["sampling"]["maxTokens"], 4096);
}

TEST_F(MCPProtocolTest, ClientCapabilitiesFromJson) {
    json j = {{"logging", {{"level", "debug"}}},
              {"prompts", {{"listChanged", false}}},
              {"resources", {{"subscribe", true}}},
              {"tools", {{"listChanged", true}}},
              {"sampling", {{"maxTokens", 2048}}}};

    auto caps = ClientCapabilities::fromJson(j);
    EXPECT_EQ(caps.logging["level"], "debug");
    EXPECT_EQ(caps.prompts["listChanged"], false);
    EXPECT_EQ(caps.resources["subscribe"], true);
    EXPECT_EQ(caps.tools["listChanged"], true);
    EXPECT_EQ(caps.sampling["maxTokens"], 2048);
}

TEST_F(MCPProtocolTest, ClientCapabilitiesEmptyFields) {
    ClientCapabilities caps;
    auto j = caps.toJson();
    // Empty fields should not appear in JSON
    EXPECT_FALSE(j.contains("logging"));
    EXPECT_FALSE(j.contains("prompts"));
    EXPECT_FALSE(j.contains("resources"));
    EXPECT_FALSE(j.contains("tools"));
    EXPECT_FALSE(j.contains("sampling"));
}

// ============================================================================
// Tests for protocol.h - ServerCapabilities
// ============================================================================
TEST_F(MCPProtocolTest, ServerCapabilitiesToJson) {
    ServerCapabilities caps;
    caps.experimental = json{{"feature", true}};
    caps.prompts = json{{"listChanged", false}};
    caps.resources = json{{"subscribe", false}};
    caps.tools = json{{"listChanged", false}};
    caps.logging = json{{"level", "info"}};
    caps.sampling = json{{"maxTokens", 2048}};

    auto j = caps.toJson();
    EXPECT_EQ(j["experimental"]["feature"], true);
    EXPECT_EQ(j["prompts"]["listChanged"], false);
    EXPECT_EQ(j["resources"]["subscribe"], false);
    EXPECT_EQ(j["tools"]["listChanged"], false);
    EXPECT_EQ(j["logging"]["level"], "info");
    EXPECT_EQ(j["sampling"]["maxTokens"], 2048);
}

TEST_F(MCPProtocolTest, ServerCapabilitiesFromJson) {
    json j = {{"experimental", {{"feature", true}}},
              {"prompts", {{"listChanged", true}}},
              {"resources", {{"subscribe", true}}},
              {"tools", {{"listChanged", true}}},
              {"logging", {{"level", "debug"}}},
              {"sampling", {{"maxTokens", 4096}}}};

    auto caps = ServerCapabilities::fromJson(j);
    EXPECT_EQ(caps.experimental["feature"], true);
    EXPECT_EQ(caps.prompts["listChanged"], true);
    EXPECT_EQ(caps.resources["subscribe"], true);
    EXPECT_EQ(caps.tools["listChanged"], true);
    EXPECT_EQ(caps.logging["level"], "debug");
    EXPECT_EQ(caps.sampling["maxTokens"], 4096);
}

// ============================================================================
// Tests for protocol.h - ClientInfo
// ============================================================================
TEST_F(MCPProtocolTest, ClientInfoToJson) {
    ClientInfo info;
    info.name = "TestClient";
    info.version = "1.0.0";

    auto j = info.toJson();
    EXPECT_EQ(j["name"], "TestClient");
    EXPECT_EQ(j["version"], "1.0.0");
}

TEST_F(MCPProtocolTest, ClientInfoFromJson) {
    json j = {{"name", "TestClient"}, {"version", "1.0.0"}};

    auto info = ClientInfo::fromJson(j);
    EXPECT_EQ(info.name, "TestClient");
    EXPECT_EQ(info.version, "1.0.0");
}

TEST_F(MCPProtocolTest, ClientInfoFromJsonPartial) {
    json j = {{"name", "TestClient"}};

    auto info = ClientInfo::fromJson(j);
    EXPECT_EQ(info.name, "TestClient");
    EXPECT_EQ(info.version, "");  // Default value
}

// ============================================================================
// Tests for protocol.h - ServerInfo
// ============================================================================
TEST_F(MCPProtocolTest, ServerInfoToJson) {
    ServerInfo info;
    info.name = "TestServer";
    info.version = "2.0.0";

    auto j = info.toJson();
    EXPECT_EQ(j["name"], "TestServer");
    EXPECT_EQ(j["version"], "2.0.0");
}

TEST_F(MCPProtocolTest, ServerInfoFromJson) {
    json j = {{"name", "TestServer"}, {"version", "2.0.0"}};

    auto info = ServerInfo::fromJson(j);
    EXPECT_EQ(info.name, "TestServer");
    EXPECT_EQ(info.version, "2.0.0");
}

// ============================================================================
// Tests for protocol.h - InitializeParams
// ============================================================================
TEST_F(MCPProtocolTest, InitializeParamsToJson) {
    InitializeParams params;
    params.protocolVersion = "2024-11-05";
    params.capabilities.logging = json{{"level", "info"}};
    params.clientInfo.name = "TestClient";
    params.clientInfo.version = "1.0.0";

    auto j = params.toJson();
    EXPECT_EQ(j["protocolVersion"], "2024-11-05");
    EXPECT_EQ(j["capabilities"]["logging"]["level"], "info");
    EXPECT_EQ(j["clientInfo"]["name"], "TestClient");
    EXPECT_EQ(j["clientInfo"]["version"], "1.0.0");
}

TEST_F(MCPProtocolTest, InitializeParamsFromJson) {
    json j = {{"protocolVersion", "2024-11-05"},
              {"capabilities", {{"logging", {{"level", "debug"}}}}},
              {"clientInfo", {{"name", "TestClient"}, {"version", "1.0.0"}}}};

    auto params = InitializeParams::fromJson(j);
    EXPECT_EQ(params.protocolVersion, "2024-11-05");
    EXPECT_EQ(params.capabilities.logging["level"], "debug");
    EXPECT_EQ(params.clientInfo.name, "TestClient");
    EXPECT_EQ(params.clientInfo.version, "1.0.0");
}

TEST_F(MCPProtocolTest, InitializeParamsDefaultVersion) {
    json j = {{"capabilities", json::object()}, {"clientInfo", json::object()}};

    auto params = InitializeParams::fromJson(j);
    EXPECT_EQ(params.protocolVersion, PROTOCOL_VERSION);
}

// ============================================================================
// Tests for protocol.h - InitializeResult
// ============================================================================
TEST_F(MCPProtocolTest, InitializeResultToJson) {
    InitializeResult result;
    result.protocolVersion = "2024-11-05";
    result.capabilities.prompts = json{{"listChanged", false}};
    result.serverInfo.name = "TestServer";
    result.serverInfo.version = "2.0.0";

    auto j = result.toJson();
    EXPECT_EQ(j["protocolVersion"], "2024-11-05");
    EXPECT_EQ(j["capabilities"]["prompts"]["listChanged"], false);
    EXPECT_EQ(j["serverInfo"]["name"], "TestServer");
    EXPECT_EQ(j["serverInfo"]["version"], "2.0.0");
}

TEST_F(MCPProtocolTest, InitializeResultFromJson) {
    json j = {{"protocolVersion", "2024-11-05"},
              {"capabilities", {{"prompts", {{"listChanged", true}}}}},
              {"serverInfo", {{"name", "TestServer"}, {"version", "2.0.0"}}}};

    auto result = InitializeResult::fromJson(j);
    EXPECT_EQ(result.protocolVersion, "2024-11-05");
    EXPECT_EQ(result.capabilities.prompts["listChanged"], true);
    EXPECT_EQ(result.serverInfo.name, "TestServer");
    EXPECT_EQ(result.serverInfo.version, "2.0.0");
}

// ============================================================================
// Tests for protocol.h - Tool
// ============================================================================
TEST_F(MCPProtocolTest, ToolToJson) {
    Tool tool;
    tool.name = "test_tool";
    tool.description = "A test tool";
    tool.inputSchema = json{{"type", "object"}};

    auto j = tool.toJson();
    EXPECT_EQ(j["name"], "test_tool");
    EXPECT_EQ(j["description"], "A test tool");
    EXPECT_EQ(j["inputSchema"]["type"], "object");
}

TEST_F(MCPProtocolTest, ToolFromJson) {
    json j = {{"name", "test_tool"},
              {"description", "A test tool"},
              {"inputSchema", {{"type", "object"}}}};

    auto tool = Tool::fromJson(j);
    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_EQ(tool.description, "A test tool");
    EXPECT_EQ(tool.inputSchema["type"], "object");
}

TEST_F(MCPProtocolTest, ToolFromJsonPartial) {
    json j = {{"name", "test_tool"}};

    auto tool = Tool::fromJson(j);
    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_EQ(tool.description, "");
    EXPECT_TRUE(tool.inputSchema.empty());
}

// ============================================================================
// Tests for protocol.h - ToolCallParams
// ============================================================================
TEST_F(MCPProtocolTest, ToolCallParamsToJson) {
    ToolCallParams params;
    params.name = "test_tool";
    params.arguments = json{{"arg1", "value1"}};

    auto j = params.toJson();
    EXPECT_EQ(j["name"], "test_tool");
    EXPECT_EQ(j["arguments"]["arg1"], "value1");
}

TEST_F(MCPProtocolTest, ToolCallParamsFromJson) {
    json j = {{"name", "test_tool"}, {"arguments", {{"arg1", "value1"}}}};

    auto params = ToolCallParams::fromJson(j);
    EXPECT_EQ(params.name, "test_tool");
    EXPECT_EQ(params.arguments["arg1"], "value1");
}

// ============================================================================
// Tests for protocol.h - ToolCallResult
// ============================================================================
TEST_F(MCPProtocolTest, ToolCallResultToJsonSuccess) {
    ToolCallResult result;
    result.isError = false;
    result.content = json{{"output", "success"}};

    auto j = result.toJson();
    EXPECT_TRUE(j.contains("content"));
    EXPECT_FALSE(j.contains("error"));
    EXPECT_EQ(j["content"]["output"], "success");
}

TEST_F(MCPProtocolTest, ToolCallResultToJsonError) {
    ToolCallResult result;
    result.isError = true;
    result.errorMessage = "Tool execution failed";

    auto j = result.toJson();
    EXPECT_FALSE(j.contains("content"));
    EXPECT_TRUE(j.contains("error"));
    EXPECT_EQ(j["error"], "Tool execution failed");
}

TEST_F(MCPProtocolTest, ToolCallResultFromJsonSuccess) {
    json j = {{"content", {{"output", "success"}}}};

    auto result = ToolCallResult::fromJson(j);
    EXPECT_FALSE(result.isError);
    EXPECT_EQ(result.content["output"], "success");
}

TEST_F(MCPProtocolTest, ToolCallResultFromJsonError) {
    json j = {{"error", "Tool execution failed"}};

    auto result = ToolCallResult::fromJson(j);
    EXPECT_TRUE(result.isError);
    EXPECT_EQ(result.errorMessage, "Tool execution failed");
}

// ============================================================================
// Tests for protocol.h - Resource
// ============================================================================
TEST_F(MCPProtocolTest, ResourceToJson) {
    Resource resource;
    resource.uri = "file:///test.txt";
    resource.name = "Test Resource";
    resource.description = "A test resource";
    resource.mimeType = "text/plain";

    auto j = resource.toJson();
    EXPECT_EQ(j["uri"], "file:///test.txt");
    EXPECT_EQ(j["name"], "Test Resource");
    EXPECT_EQ(j["description"], "A test resource");
    EXPECT_EQ(j["mimeType"], "text/plain");
}

TEST_F(MCPProtocolTest, ResourceFromJson) {
    json j = {{"uri", "file:///test.txt"},
              {"name", "Test Resource"},
              {"description", "A test resource"},
              {"mimeType", "text/plain"}};

    auto resource = Resource::fromJson(j);
    EXPECT_EQ(resource.uri, "file:///test.txt");
    EXPECT_EQ(resource.name, "Test Resource");
    EXPECT_EQ(resource.description, "A test resource");
    EXPECT_EQ(resource.mimeType, "text/plain");
}

TEST_F(MCPProtocolTest, ResourceFromJsonPartial) {
    json j = {{"uri", "file:///test.txt"}};

    auto resource = Resource::fromJson(j);
    EXPECT_EQ(resource.uri, "file:///test.txt");
    EXPECT_EQ(resource.name, "");
    EXPECT_EQ(resource.description, "");
    EXPECT_EQ(resource.mimeType, "");
}

// ============================================================================
// Tests for protocol.h - Prompt
// ============================================================================
TEST_F(MCPProtocolTest, PromptToJson) {
    Prompt prompt;
    prompt.name = "test_prompt";
    prompt.description = "A test prompt";
    prompt.arguments = {"arg1", "arg2", "arg3"};

    auto j = prompt.toJson();
    EXPECT_EQ(j["name"], "test_prompt");
    EXPECT_EQ(j["description"], "A test prompt");
    EXPECT_TRUE(j["arguments"].is_array());
    EXPECT_EQ(j["arguments"].size(), 3);
    EXPECT_EQ(j["arguments"][0]["name"], "arg1");
    EXPECT_EQ(j["arguments"][0]["description"], "");
    EXPECT_EQ(j["arguments"][0]["required"], false);
    EXPECT_EQ(j["arguments"][1]["name"], "arg2");
    EXPECT_EQ(j["arguments"][2]["name"], "arg3");
}

TEST_F(MCPProtocolTest, PromptFromJson) {
    json j = {{"name", "test_prompt"},
              {"description", "A test prompt"},
              {"arguments", json::array({json{{"name", "arg1"},
                                              {"description", "first"},
                                              {"required", true}},
                                         json{{"name", "arg2"},
                                              {"description", "second"},
                                              {"required", false}},
                                         json{{"name", "arg3"}}})}};

    auto prompt = Prompt::fromJson(j);
    EXPECT_EQ(prompt.name, "test_prompt");
    EXPECT_EQ(prompt.description, "A test prompt");
    EXPECT_EQ(prompt.arguments.size(), 3);
    EXPECT_EQ(prompt.arguments[0], "arg1");
    EXPECT_EQ(prompt.arguments[1], "arg2");
    EXPECT_EQ(prompt.arguments[2], "arg3");
}

TEST_F(MCPProtocolTest, PromptFromJsonLegacyStringArguments) {
    json j = {{"name", "test_prompt"},
              {"description", "A test prompt"},
              {"arguments", {"arg1", "arg2", "arg3"}}};

    auto prompt = Prompt::fromJson(j);
    EXPECT_EQ(prompt.name, "test_prompt");
    EXPECT_EQ(prompt.description, "A test prompt");
    EXPECT_EQ(prompt.arguments.size(), 3);
    EXPECT_EQ(prompt.arguments[0], "arg1");
    EXPECT_EQ(prompt.arguments[1], "arg2");
    EXPECT_EQ(prompt.arguments[2], "arg3");
}

TEST_F(MCPProtocolTest, PromptFromJsonNoArguments) {
    json j = {{"name", "test_prompt"}, {"description", "A test prompt"}};

    auto prompt = Prompt::fromJson(j);
    EXPECT_EQ(prompt.name, "test_prompt");
    EXPECT_EQ(prompt.description, "A test prompt");
    EXPECT_TRUE(prompt.arguments.empty());
}

// ============================================================================
// Tests for protocol.h - utils namespace functions
// ============================================================================
TEST_F(MCPProtocolTest, CreateDefaultClientCapabilities) {
    auto caps = utils::createDefaultClientCapabilities();

    // logging is set to empty json object, so it's empty but exists
    EXPECT_TRUE(caps.logging.is_object());
    EXPECT_FALSE(caps.prompts.empty());
    EXPECT_FALSE(caps.resources.empty());
    EXPECT_FALSE(caps.tools.empty());
    EXPECT_FALSE(caps.sampling.empty());

    EXPECT_EQ(caps.prompts["listChanged"], true);
    EXPECT_EQ(caps.resources["subscribe"], true);
    EXPECT_EQ(caps.tools["listChanged"], true);
    EXPECT_EQ(caps.sampling["maxTokens"], 4096);
    EXPECT_EQ(caps.sampling["temperature"], 0.7);
}

TEST_F(MCPProtocolTest, CreateDefaultServerCapabilities) {
    auto caps = utils::createDefaultServerCapabilities();

    EXPECT_FALSE(caps.prompts.empty());
    EXPECT_FALSE(caps.resources.empty());
    EXPECT_FALSE(caps.tools.empty());
    EXPECT_FALSE(caps.sampling.empty());

    EXPECT_EQ(caps.prompts["listChanged"], false);
    EXPECT_EQ(caps.resources["listChanged"], false);
    EXPECT_EQ(caps.resources["subscribe"], false);
    EXPECT_EQ(caps.tools["listChanged"], false);
    EXPECT_EQ(caps.sampling["maxTokens"], 2048);
    EXPECT_EQ(caps.sampling["temperature"], 0.5);
}

TEST_F(MCPProtocolTest, IsValidProtocolVersion) {
    EXPECT_TRUE(utils::isValidProtocolVersion("2024-11-05"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-03-26"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-06-18"));
    EXPECT_TRUE(utils::isValidProtocolVersion("2025-11-25"));
    EXPECT_FALSE(utils::isValidProtocolVersion("2024-01-01"));
    EXPECT_FALSE(utils::isValidProtocolVersion("invalid"));
    EXPECT_FALSE(utils::isValidProtocolVersion(""));
}

TEST_F(MCPProtocolTest, CreateToolFromSchema) {
    json schema = {{"type", "object"},
                   {"properties", {{"param1", {{"type", "string"}}}}}};

    auto tool = utils::createToolFromSchema("test_tool", "A test tool", schema);
    EXPECT_EQ(tool.name, "test_tool");
    EXPECT_EQ(tool.description, "A test tool");
    EXPECT_EQ(tool.inputSchema["type"], "object");
    EXPECT_TRUE(tool.inputSchema.contains("properties"));
}

TEST_F(MCPProtocolTest, CreateResourceFromUri) {
    auto resource = utils::createResourceFromUri(
        "file:///test.txt", "Test Resource", "A test resource");
    EXPECT_EQ(resource.uri, "file:///test.txt");
    EXPECT_EQ(resource.name, "Test Resource");
    EXPECT_EQ(resource.description, "A test resource");
    EXPECT_EQ(resource.mimeType, "text/plain");
}

TEST_F(MCPProtocolTest, CreateResourceFromUriDefaultName) {
    auto resource = utils::createResourceFromUri("file:///test.txt");
    EXPECT_EQ(resource.uri, "file:///test.txt");
    EXPECT_EQ(resource.name,
              "file:///test.txt");  // Uses URI as name if not provided
    EXPECT_EQ(resource.description, "");
    EXPECT_EQ(resource.mimeType, "text/plain");
}

TEST_F(MCPProtocolTest, CreateResourceFromUriWithNameOnly) {
    auto resource =
        utils::createResourceFromUri("file:///test.txt", "Test Resource");
    EXPECT_EQ(resource.uri, "file:///test.txt");
    EXPECT_EQ(resource.name, "Test Resource");
    EXPECT_EQ(resource.description, "");
    EXPECT_EQ(resource.mimeType, "text/plain");
}

// ============================================================================
// Tests for protocol constants
// ============================================================================
TEST_F(MCPProtocolTest, ProtocolVersionConstant) {
    EXPECT_STREQ(PROTOCOL_VERSION, "2025-11-25");
    EXPECT_STREQ(MCP_VERSION, "2025-11-25");
    EXPECT_STREQ(PROTOCOL_VERSION_2024_11_05, "2024-11-05");
    EXPECT_STREQ(PROTOCOL_VERSION_2025_11_25, "2025-11-25");
    EXPECT_STREQ(MCP_VERSION_2024_11_05, "2024-11-05");
    EXPECT_STREQ(MCP_VERSION_2025_11_25, "2025-11-25");
}

TEST_F(MCPProtocolTest, ProtocolMethodsConstants) {
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
    EXPECT_STREQ(methods::TASKS_GET, "tasks/get");
    EXPECT_STREQ(methods::TASKS_RESULT, "tasks/result");
    EXPECT_STREQ(methods::TASKS_LIST, "tasks/list");
    EXPECT_STREQ(methods::TASKS_CANCEL, "tasks/cancel");
    EXPECT_STREQ(methods::ELICITATION_CREATE, "elicitation/create");
    EXPECT_STREQ(methods::COMPLETION_COMPLETE, "completion/complete");
    EXPECT_STREQ(methods::SAMPLING_CREATE_MESSAGE, "sampling/createMessage");
    EXPECT_STREQ(methods::RESOURCES_SUBSCRIBE, "resources/subscribe");
    EXPECT_STREQ(methods::RESOURCES_UNSUBSCRIBE, "resources/unsubscribe");
    EXPECT_STREQ(methods::ROOTS_LIST, "roots/list");
    EXPECT_STREQ(methods::NOTIFICATION_INITIALIZED,
                 "notifications/initialized");
    EXPECT_STREQ(methods::NOTIFICATION_CANCELLED, "notifications/cancelled");
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
    EXPECT_STREQ(methods::NOTIFICATION_MESSAGE, "notifications/message");
    EXPECT_STREQ(methods::NOTIFICATION_PROGRESS, "notifications/progress");
    EXPECT_STREQ(methods::NOTIFICATION_TASKS_STATUS,
                 "notifications/tasks/status");
    EXPECT_STREQ(methods::NOTIFICATION_ELICITATION_COMPLETE,
                 "notifications/elicitation/complete");
}

// ============================================================================
// Integration tests - Round-trip JSON serialization
// ============================================================================
TEST_F(MCPProtocolTest, ClientCapabilitiesRoundTrip) {
    ClientCapabilities original;
    original.logging = json{{"level", "info"}};
    original.prompts = json{{"listChanged", true}};

    auto j = original.toJson();
    auto restored = ClientCapabilities::fromJson(j);

    EXPECT_EQ(restored.logging["level"], "info");
    EXPECT_EQ(restored.prompts["listChanged"], true);
}

TEST_F(MCPProtocolTest, RequestResponseRoundTrip) {
    // Create request
    auto req = request::create("test/method", json{{"key", "value"}});
    auto req_json = req.toJson();
    auto req_restored = request::fromJson(req_json);

    EXPECT_EQ(req_restored.method, "test/method");
    EXPECT_EQ(req_restored.params["key"], "value");

    // Create response
    auto res = response::createSuccess(req_restored.id, json{{"result", "ok"}});
    auto res_json = res.toJson();
    auto res_restored = response::fromJson(res_json);

    EXPECT_FALSE(res_restored.isError());
    EXPECT_EQ(res_restored.result["result"], "ok");
}

TEST_F(MCPProtocolTest, InitializeParamsRoundTrip) {
    InitializeParams original;
    original.protocolVersion = "2024-11-05";
    original.capabilities.logging = json{{"level", "debug"}};
    original.clientInfo.name = "TestClient";
    original.clientInfo.version = "1.0.0";

    auto j = original.toJson();
    auto restored = InitializeParams::fromJson(j);

    EXPECT_EQ(restored.protocolVersion, "2024-11-05");
    EXPECT_EQ(restored.capabilities.logging["level"], "debug");
    EXPECT_EQ(restored.clientInfo.name, "TestClient");
    EXPECT_EQ(restored.clientInfo.version, "1.0.0");
}

// ============================================================================
// Main function for running tests
// ============================================================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
