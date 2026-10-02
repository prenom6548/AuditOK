#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, SimilaritySearch_Success_Basic) {
// 测试正常情况 在打开mock宏时，才执行该条用例
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            EXPECT_STREQ(R"({"text":"linux内核安全选项","similarity-threshold":0.8})", arg_input);
            auto *result = new std::string(
                R"({"err_no":0,"err_msg":"success","data":[{"filepath":"/path/to/file1.txt","similarity":0.92},{"filepath":"/path/to/file2.txt","similarity":0.85}]})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(
        session_, R"({"text":"linux内核安全选项","similarity-threshold":0.8})", &result);
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("success", status.errorMsg);
    EXPECT_STREQ(
        R"([{"filepath":"/path/to/file1.txt","similarity":0.92},{"filepath":"/path/to/file2.txt","similarity":0.85}])",
        result);
    delete result;
#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_ErrorJsonFormat_1) {
// 返回结果json格式有误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err":14,"err_msg":"解析参数失败","data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    delete result;
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_ErrorJsonFormat_2) {
// 返回结果json格式有误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":"string","err_msg":"解析参数失败","data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    delete result;
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_ErrorJsonFormat_3) {
// 返回结果json格式有误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":10,"err_msg":1,"data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    delete result;
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_EmptySearchConditions) {
// search_conditions为空
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败","data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
    delete result;
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_NullSession) {
    // 传入空指针
    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(nullptr, nullptr, &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_NullSearchConditions) {
    // 传入空指针
    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, nullptr, &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_ErrorParmaFormat) {
// search_conditions 参数格式错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败","data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status =
        data_management_similarity_search(session_, R"({"text":1,"similarity-threshold":"error"})", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_EmptySearchResult) {
// search_result为空
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":59,"err_msg":"搜索文件信息失败","data":""})");
            *out_output = result->data();
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(
        session_, R"({"text":"linux内核安全选项","similarity-threshold":0.8})", &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("搜索文件信息失败", status.errorMsg);
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_ServiceUnavailable) {
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 模拟服务不可用错误
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "test query", &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_InvalidJSON) {
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 模拟JSON解析错误
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid JSON format");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "{invalid json", &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, SimilaritySearch_Error_Timeout) {
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SimilaritySearch(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 模拟超时错误
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_similarity_search(session_, "test query", &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}