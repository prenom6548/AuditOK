#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Success_Basic) {
    // 测试正常情况 - 根据标签搜索文件
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 验证输入参数
            EXPECT_STREQ(R"({"tags":["文档","重要"]})", arg_input);

            auto *result = new std::string(
                R"({"err_no":0,"err_msg":"请求成功","data":["/path/to/document1.pdf","/path/to/report.docx","/path/to/notes.txt"]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status =
        data_management_search_files_about_tags(session_, R"({"tags":["文档","重要"]})", &result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ(R"(["/path/to/document1.pdf","/path/to/report.docx","/path/to/notes.txt"])", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Success_SingleTag) {
    // 测试单个标签的文件搜索
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            EXPECT_STREQ(R"({"tags":["图片"]})", arg_input);

            auto *result = new std::string(
                R"({"err_no":0,"err_msg":"请求成功","data":["/path/to/photo1.jpg","/path/to/screenshot.png","/path/to/image.gif"]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags":["图片"]})", &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_STREQ(R"(["/path/to/photo1.jpg","/path/to/screenshot.png","/path/to/image.gif"])", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Success_NoResults) {
    // 测试没有搜索结果的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            EXPECT_STREQ(R"({"tags":["不存在标签"]})", arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status =
        data_management_search_files_about_tags(session_, R"({"tags":["不存在标签"]})", &result);

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

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_NullSession) {
    // session为空

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(nullptr, R"({"tags":[]})", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_EmptyTagsArray) {
    // 测试空标签数组的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags":[]})", &result);

    EXPECT_FALSE(status.success);
    EXPECT_EQ(14, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_NullTagsInput) {
    // 测试传入空标签输入的情况
    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, nullptr, &result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_InvalidJSONFormat) {
    // 测试无效JSON格式的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags": invalid})", &result);

    EXPECT_FALSE(status.success);
    EXPECT_EQ(14, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_MissingTagsKey) {
    // 测试缺少tags键的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = g_strdup(result->c_str());
            return true;
        });
#endif

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"other_key":[]})", &result);

    EXPECT_FALSE(status.success);
    EXPECT_EQ(14, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags":["文档"]})", &result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
#endif
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags":["图片"]})", &result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
#endif
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Edge_LargeNumberOfTags) {
    // 测试大量标签的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 构建包含大量文件的响应
            std::string json = R"({"err_no":0,"err_msg":"请求成功","data":[)";
            for (int i = 0; i < 20; ++i) {
                if (i > 0) json += ",";
                json += "\"/path/to/file" + std::to_string(i) + ".txt\"";
            }
            json += "]}";
            auto *result = new std::string(json);
            *out_output = g_strdup(result->c_str());
            return true;
        });

    // 构建包含大量标签的输入
    std::string tagsJson = R"({"tags":[)";
    for (int i = 0; i < 15; ++i) {
        if (i > 0) tagsJson += ",";
        tagsJson += "\"标签" + std::to_string(i) + "\"";
    }
    tagsJson += "]}";

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, tagsJson.c_str(), &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, SearchFilesAboutTags_Edge_LongFilePaths) {
    // 测试超长文件路径的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, SearchFilesAboutTags(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.txt";
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[")" + longPath + R"("]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_search_files_about_tags(session_, R"({"tags":["长路径"]})", &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}