/**
 * @file client_complete_precheck_gtest.cpp
 * @brief 单元测试 —— MCPClient::complete() 的本地最小预检
 *
 * 校验目标：
 *   即使客户端尚未建立连接，传入明显非法的 params（null / 非 object / 空 object /
 *   缺 ref 或 argument）也应当立即抛 mcp_exception(kInvalidParams)，
 *   而不是先经历一次 IPC/网络 round-trip 才报错。
 *
 * 设计要点：
 *   - 不真实连接：预检发生在 send_request_internal 之前；
 *   - 仅检查必填字段是否存在，不复刻 server 端 ref.type / argument.value 等详细规则；
 *     更深层的字段校验由 validate_complete_params_gtest.cpp 覆盖；
 *   - 一旦客户端预检通过最低门槛（params 是非空 object 且含 ref 与 argument），
 *     若无连接，complete() 应继续走 send_request_internal 路径并因 transport 失败
 *     而抛出（这里只关心"不再是 kInvalidParams"——意味着预检已放行）。
 */

#include <gtest/gtest.h>

#include "client.h"
#include "message.h"

using namespace mcp;
// 显式使用 mcp::json（ordered_json 别名），避免与 nlohmann::json 同名冲突。

namespace {

// 辅助宏：要求抛 mcp_exception 且 code == kInvalidParams
#define EXPECT_INVALID_PARAMS(stmt)                                       \
    do {                                                                  \
        try {                                                             \
            (void)(stmt);                                                 \
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

// ------------------------- 必拒：null / 非 object / 空 -------------------------

TEST(ClientCompletePreCheck, RejectsNullParams) {
    MCPClient client;
    EXPECT_INVALID_PARAMS(client.complete(json(nullptr)));
}

TEST(ClientCompletePreCheck, RejectsArrayParams) {
    MCPClient client;
    EXPECT_INVALID_PARAMS(client.complete(json::array()));
}

TEST(ClientCompletePreCheck, RejectsStringParams) {
    MCPClient client;
    EXPECT_INVALID_PARAMS(client.complete(json("foo")));
}

TEST(ClientCompletePreCheck, RejectsNumberParams) {
    MCPClient client;
    EXPECT_INVALID_PARAMS(client.complete(json(7)));
}

TEST(ClientCompletePreCheck, RejectsEmptyObjectParams) {
    MCPClient client;
    EXPECT_INVALID_PARAMS(client.complete(json::object()));
}

// ------------------------- 必拒：缺必填字段 -------------------------

TEST(ClientCompletePreCheck, RejectsMissingRef) {
    MCPClient client;
    json p = {{"argument", {{"name", "a"}, {"value", "v"}}}};
    EXPECT_INVALID_PARAMS(client.complete(p));
}

TEST(ClientCompletePreCheck, RejectsMissingArgument) {
    MCPClient client;
    json p = {{"ref", {{"type", "ref/prompt"}, {"name", "demo"}}}};
    EXPECT_INVALID_PARAMS(client.complete(p));
}

// -------- 放行后：未连接情况下应抛出非 kInvalidParams 的错误 --------
//
// 这条断言反向证明：预检确实只做"必填字段存在"这一最低门槛，
// 不会越权去校验 ref.type / argument.value 等深层细节；
// 只要预检通过，complete 就把详细校验权交还给 server 端。

TEST(ClientCompletePreCheck, PassesPreCheckButFailsOnTransportWhenNotConnected) {
    MCPClient client;
    json p = {
        {"ref", {{"type", "ref/prompt"}, {"name", "demo"}}},
        {"argument", {{"name", "arg1"}, {"value", "v"}}},
    };
    try {
        (void)client.complete(p);
        FAIL() << "expected complete() to throw when client is not connected";
    } catch (const mcp_exception& e) {
        // 任何非 kInvalidParams 都说明预检已放行，进入了 transport 层后才失败。
        EXPECT_NE(e.code(), ErrorCode::kInvalidParams)
            << "pre-check should not reject a well-formed request; "
               "message="
            << e.what();
    } catch (const std::exception& e) {
        // 也允许是 transport 层抛出的 std::exception 派生类——同样说明已越过预检。
        SUCCEED() << "transport-layer failure as expected: " << e.what();
    }
}
