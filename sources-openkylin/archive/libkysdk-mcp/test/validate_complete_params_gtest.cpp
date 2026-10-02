/**
 * @file validate_complete_params_gtest.cpp
 * @brief 单元测试 —— server 端 validateCompleteParams 的参数校验
 *
 * 覆盖：
 *   1. 顶层 params 必须是非空 object
 *   2. ref 必须存在、为 object，含非空 string 字段 type
 *   3. ref.type 必须是 "ref/prompt" 或 "ref/resource"
 *   4. ref/prompt 必须含非空 ref.name
 *   5. ref/resource 必须含非空 ref.uri
 *   6. argument 必须存在、为 object，含非空 string name 与 string value
 *   7. argument.value 允许空串（请求所有候选）
 *   8. （新增）可选 context.arguments 字段：若提供必须是 map<string,string>
 *   9. 合法请求正常通过
 */

#include <gtest/gtest.h>

#include "message.h"
#include "server_dispatch.h"

using namespace mcp;
using mcp::internal::validateCompleteParams;
// 注意：mcp::json 是 nlohmann::ordered_json 的别名，必须显式 using mcp::json，
// 否则会与 nlohmann::basic_json<> 起的同名别名冲突。

namespace {

// 辅助：构造一份最小可通过的 ref/prompt 请求
json makePromptParams() {
    return json{
        {"ref", {{"type", "ref/prompt"}, {"name", "demo"}}},
        {"argument", {{"name", "arg1"}, {"value", "v"}}},
    };
}

// 辅助：构造一份最小可通过的 ref/resource 请求
json makeResourceParams() {
    return json{
        {"ref", {{"type", "ref/resource"}, {"uri", "file:///tmp/x"}}},
        {"argument", {{"name", "arg1"}, {"value", "v"}}},
    };
}

// 辅助：断言抛出 mcp_exception 且 code == kInvalidParams
#define EXPECT_INVALID_PARAMS(stmt)                                       \
    do {                                                                  \
        try {                                                             \
            (stmt);                                                       \
            FAIL() << "expected mcp_exception(kInvalidParams), got none"; \
        } catch (const mcp_exception& e) {                                \
            EXPECT_EQ(e.code(), ErrorCode::kInvalidParams)                \
                << "wrong code, message=" << e.what();                    \
        } catch (const std::exception& e) {                               \
            FAIL() << "expected mcp_exception, got std::exception: "      \
                   << e.what();                                           \
        }                                                                 \
    } while (0)

}  // namespace

// ------------------------- 顶层 params -------------------------

TEST(ValidateCompleteParams, RejectsNonObjectRoot) {
    EXPECT_INVALID_PARAMS(validateCompleteParams(json(nullptr)));
    EXPECT_INVALID_PARAMS(validateCompleteParams(json::array()));
    EXPECT_INVALID_PARAMS(validateCompleteParams(json("string")));
    EXPECT_INVALID_PARAMS(validateCompleteParams(json(123)));
}

TEST(ValidateCompleteParams, RejectsEmptyObject) {
    // 空对象 → 缺 ref
    EXPECT_INVALID_PARAMS(validateCompleteParams(json::object()));
}

// ------------------------- ref 字段 -------------------------

TEST(ValidateCompleteParams, RejectsMissingRef) {
    json p = {{"argument", {{"name", "a"}, {"value", "v"}}}};
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsNonObjectRef) {
    json p = makePromptParams();
    p["ref"] = "ref/prompt";  // 字符串而非 object
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsMissingRefType) {
    json p = makePromptParams();
    p["ref"].erase("type");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsEmptyRefType) {
    json p = makePromptParams();
    p["ref"]["type"] = "";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsNonStringRefType) {
    json p = makePromptParams();
    p["ref"]["type"] = 42;
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsUnknownRefType) {
    json p = makePromptParams();
    p["ref"]["type"] = "ref/unknown";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

// ------------------------- ref/prompt -------------------------

TEST(ValidateCompleteParams, AcceptsValidRefPrompt) {
    EXPECT_NO_THROW(validateCompleteParams(makePromptParams()));
}

TEST(ValidateCompleteParams, RefPromptRejectsMissingName) {
    json p = makePromptParams();
    p["ref"].erase("name");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RefPromptRejectsEmptyName) {
    json p = makePromptParams();
    p["ref"]["name"] = "";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RefPromptRejectsNonStringName) {
    json p = makePromptParams();
    p["ref"]["name"] = 1;
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

// ------------------------- ref/resource -------------------------

TEST(ValidateCompleteParams, AcceptsValidRefResource) {
    EXPECT_NO_THROW(validateCompleteParams(makeResourceParams()));
}

TEST(ValidateCompleteParams, RefResourceRejectsMissingUri) {
    json p = makeResourceParams();
    p["ref"].erase("uri");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RefResourceRejectsEmptyUri) {
    json p = makeResourceParams();
    p["ref"]["uri"] = "";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RefResourceRejectsNonStringUri) {
    json p = makeResourceParams();
    p["ref"]["uri"] = json::array();
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

// ------------------------- argument 字段 -------------------------

TEST(ValidateCompleteParams, RejectsMissingArgument) {
    json p = makePromptParams();
    p.erase("argument");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsNonObjectArgument) {
    json p = makePromptParams();
    p["argument"] = "v";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsMissingArgumentName) {
    json p = makePromptParams();
    p["argument"].erase("name");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsEmptyArgumentName) {
    json p = makePromptParams();
    p["argument"]["name"] = "";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsMissingArgumentValue) {
    json p = makePromptParams();
    p["argument"].erase("value");
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsNonStringArgumentValue) {
    json p = makePromptParams();
    p["argument"]["value"] = 1;
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

// MCP 规范允许 value 为空串，表示请求"所有可能的候选"
TEST(ValidateCompleteParams, AcceptsEmptyArgumentValue) {
    json p = makePromptParams();
    p["argument"]["value"] = "";
    EXPECT_NO_THROW(validateCompleteParams(p));
}

// ------------------------- 新增字段 context.arguments -------------------------

TEST(ValidateCompleteParams, AcceptsParamsWithoutContext) {
    // 兼容 2025-06-18 之前的客户端
    EXPECT_NO_THROW(validateCompleteParams(makePromptParams()));
}

TEST(ValidateCompleteParams, RejectsNonObjectContext) {
    json p = makePromptParams();
    p["context"] = "oops";
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, AcceptsContextWithoutArguments) {
    json p = makePromptParams();
    p["context"] = json::object();
    EXPECT_NO_THROW(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, AcceptsEmptyContextArguments) {
    json p = makePromptParams();
    p["context"] = {{"arguments", json::object()}};
    EXPECT_NO_THROW(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, AcceptsValidContextArguments) {
    json p = makePromptParams();
    p["context"] = {
        {"arguments", {{"key1", "v1"}, {"key2", "v2"}}},
    };
    EXPECT_NO_THROW(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsNonObjectContextArguments) {
    json p = makePromptParams();
    p["context"] = {{"arguments", json::array()}};
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsContextArgumentsWithNonStringValue) {
    json p = makePromptParams();
    p["context"] = {{"arguments", {{"k1", "ok"}, {"k2", 123}}}};
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}

TEST(ValidateCompleteParams, RejectsContextArgumentsWithObjectValue) {
    json p = makePromptParams();
    p["context"] = {{"arguments", {{"k1", json::object()}}}};
    EXPECT_INVALID_PARAMS(validateCompleteParams(p));
}
