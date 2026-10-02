#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, GetSummarysOfFiles_Success_Basic) {
    // 测试正常情况 - 获取单个文件的摘要
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetSummarysOfFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result =
                new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[{"filepath":"", "summary":""}]})");
            *out_output = result->data();
            return true;
        });

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/待办.txt";

    std::string files = R"({
        "files": [")" + targetFilePath.string() +
                        R"("]
    })";

    char *result = nullptr;
    DataManagementStatus status = data_management_get_summarys_of_files(session_, files.c_str(), &result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, GetSummarysOfFiles_Error_NullSession) {
    // session为空

    char *result = nullptr;
    DataManagementStatus status = data_management_get_summarys_of_files(nullptr, nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, GetSummarysOfFiles_Error_NullFiles) {
    // session为空
    char *result = nullptr;
    DataManagementStatus status = data_management_get_summarys_of_files(session_, nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}