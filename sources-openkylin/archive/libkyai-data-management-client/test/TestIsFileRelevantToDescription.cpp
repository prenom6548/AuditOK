#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Success_Relevant) {
    // 测试文件与描述相关的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 验证输入参数
            EXPECT_STREQ(R"({"file":"/path/to/document.pdf","description":"这是一个关于人工智能的文档"})", arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":{"is_relevant":true}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    bool isRelevant = false;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"/path/to/document.pdf","description":"这是一个关于人工智能的文档"})", &isRelevant);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_TRUE(isRelevant);
#endif
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Success_NotRelevant) {
    // 测试文件与描述不相关的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            EXPECT_STREQ(R"({"file":"/path/to/image.jpg","description":"这是一段关于编程的代码"})", arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":{"is_relevant":false}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    bool isRelevant = true;  // 初始化为true，测试是否会被修改为false
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"/path/to/image.jpg","description":"这是一段关于编程的代码"})", &isRelevant);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_FALSE(isRelevant);
#endif
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Success_EmptyDescription) {
    // 测试空描述的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            EXPECT_STREQ(R"({"file":"/path/to/file.txt","description":""})", arg_input);

            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败","data":{"is_relevant":false}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    bool isRelevant = true;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"/path/to/file.txt","description":""})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_NullSession) {
    // session为空

    bool isRelevant = false;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        nullptr, R"({"file":"","description":"这是一个描述"})", &isRelevant);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_EmptyFile) {
    // 测试空文件路径的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    bool isRelevant = false;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"","description":"这是一个描述"})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_NullFileInfos) {
    // 测试传入空文件信息的情况
    bool isRelevant = false;
    DataManagementStatus status = data_management_is_file_relevant_to_description(session_, nullptr, &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_MissingFileField) {
    // 测试缺少file字段的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    bool isRelevant = false;
    DataManagementStatus status =
        data_management_is_file_relevant_to_description(session_, R"({"description":"文档描述"})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_MissingDescriptionField) {
    // 测试缺少description字段的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    bool isRelevant = false;
    DataManagementStatus status =
        data_management_is_file_relevant_to_description(session_, R"({"file":"/path/to/file.pdf"})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_InvalidJSONFormat) {
    // 测试无效JSON格式的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    bool isRelevant = false;
    DataManagementStatus status =
        data_management_is_file_relevant_to_description(session_, R"({"file": invalid})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    bool isRelevant = false;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"/path/to/file.pdf","description":"文档描述"})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    // isRelevant 应该保持原值
    EXPECT_FALSE(isRelevant);
#endif
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    bool isRelevant = true;
    DataManagementStatus status = data_management_is_file_relevant_to_description(
        session_, R"({"file":"/path/to/file.pdf","description":"文档描述"})", &isRelevant);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    // isRelevant 应该保持原值
    EXPECT_TRUE(isRelevant);
#endif
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Edge_LongFilePath) {
    // 测试超长文件路径的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.txt";
            std::string expectedInput = R"({"file":")" + longPath + R"(","description":"长路径文件"})";
            EXPECT_STREQ(expectedInput.c_str(), arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":{"is_relevant":true}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.txt";
    std::string fileInfos = R"({"file":")" + longPath + R"(","description":"长路径文件"})";

    bool isRelevant = false;
    DataManagementStatus status =
        data_management_is_file_relevant_to_description(session_, fileInfos.c_str(), &isRelevant);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_TRUE(isRelevant);
#endif
}

TEST_F(DataManagementMockedTest, IsFileRelevantToDescription_Edge_LongDescription) {
    // 测试超长描述的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, IsFileRelevantToDescription(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string longDesc = "这是一个非常长的描述，" + std::string(500, 'x');
            std::string expectedInput = R"({"file":"/path/to/file.pdf","description":")" + longDesc + R"("})";
            EXPECT_STREQ(expectedInput.c_str(), arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":{"is_relevant":true}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    std::string longDesc = "这是一个非常长的描述，" + std::string(500, 'x');
    std::string fileInfos = R"({"file":"/path/to/file.pdf","description":")" + longDesc + R"("})";

    bool isRelevant = false;
    DataManagementStatus status =
        data_management_is_file_relevant_to_description(session_, fileInfos.c_str(), &isRelevant);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_TRUE(isRelevant);
#endif
}