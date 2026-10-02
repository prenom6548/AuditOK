#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, GetAllTags_Success_Basic) {
    // 测试正常情况 - 获取所有标签
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(
                R"({"err_no":0,"err_msg":"请求成功","data":["文档","图片","视频","音乐","代码","配置文件"]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ(R"(["文档","图片","视频","音乐","代码","配置文件"])", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, GetAllTags_Success_EmptyTags) {
    // 测试空标签列表的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_STREQ("[]", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, GetAllTags_Error_NullSession) {
    // session为空

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, GetAllTags_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
#endif
}

TEST_F(DataManagementMockedTest, GetAllTags_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
#endif
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, GetAllTags_Edge_LargeNumberOfTags) {
    // 测试大量标签的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 构建包含大量标签的JSON响应
            std::string json = R"({"err_no":0,"err_msg":"请求成功","data":[)";
            for (int i = 0; i < 100; ++i) {
                if (i > 0) json += ",";
                json += R"("标签)" + std::to_string(i) + R"(")";
            }
            json += "]}";
            auto *result = new std::string(json);
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, GetAllTags_Edge_VeryLongTagNames) {
    // 测试超长标签名称的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string longTag = "这是一个非常长的标签名称_" + std::string(100, 'x');
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[")" + longTag + R"("]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_tags(session_, &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}