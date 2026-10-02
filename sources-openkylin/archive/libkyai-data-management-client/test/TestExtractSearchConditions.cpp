#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, ExtractSearchConditions_Success_Basic) {
    // 测试正常情况 - 基本描述提取搜索条件
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, ExtractSearchConditions(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(
                R"({"data":{"0":{"condition type":3,"content":"查找标题包含测试的文档","orCondition":false},"conditions count":1,"labels":[]},"err_msg":"请求成功","err_no":0})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string description = R"({
        "description": "查找标题包含测试的文档"
    })";

    char *result = nullptr;
    DataManagementStatus status = data_management_extract_search_conditions(session_, description.c_str(), &result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

    // 检查返回的结果是否包含预期的内容
    if (result) {
        EXPECT_NE(nullptr, strstr(result, "conditions"));
        free(result);  // 记得释放内存
    }
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, ExtractSearchConditions_Error_NullSession) {
    // session为空
    std::string description = R"({
        "description": "查找标题包含测试的文档"
    })";

    char *result = nullptr;
    DataManagementStatus status = data_management_extract_search_conditions(nullptr, description.c_str(), &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, ExtractSearchConditions_Error_NullDescription) {
    // session为空
    char *result = nullptr;
    DataManagementStatus status = data_management_extract_search_conditions(session_, nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}