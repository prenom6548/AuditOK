#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, GetTagsOfFiles_Success_Basic) {
    // 测试正常情况 - 获取多个文件的标签
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetTagsOfFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 验证输入参数
            EXPECT_STREQ(R"({"files":["/path/to/file1.pdf","/path/to/file2.txt"]})", arg_input);

            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[)"
                                           R"({"filepath":"/path/to/file1.pdf","tags":["文档","技术","PDF"]},)"
                                           R"({"filepath":"/path/to/file2.txt","tags":["文本","配置","重要"]})"
                                           R"(]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_tags_of_files(
        session_, R"({"files":["/path/to/file1.pdf","/path/to/file2.txt"]})", &result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_NE(nullptr, result);
    EXPECT_STREQ(R"([{"filepath":"/path/to/file1.pdf","tags":["文档","技术","PDF"]},)"
                 R"({"filepath":"/path/to/file2.txt","tags":["文本","配置","重要"]}])",
                 result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, GetTagsOfFiles_Success_SingleFile) {
    // 测试单个文件的标签获取
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetTagsOfFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[)"
                                           R"({"filepath":"/path/to/single/file.jpg","tags":["图片","风景","旅游"]})"
                                           R"(]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status =
        data_management_get_tags_of_files(session_, R"({"files":["/path/to/single/file.jpg"]})", &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_STREQ(R"([{"filepath":"/path/to/single/file.jpg","tags":["图片","风景","旅游"]}])", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, GetTagsOfFiles_Success_EmptyTags) {
    // 测试文件没有标签的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetTagsOfFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":[)"
                                           R"({"filepath":"/path/to/untagged/file.txt","tags":[]})"
                                           R"(]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status =
        data_management_get_tags_of_files(session_, R"({"files":["/path/to/untagged/file.txt"]})", &result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
    EXPECT_STREQ(R"([{"filepath":"/path/to/untagged/file.txt","tags":[]}])", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, GGetTagsOfFiles_Error_NullSession) {
    // session为空

    char *result = nullptr;
    DataManagementStatus status =
        data_management_get_tags_of_files(nullptr, R"({"files":["/path/to/untagged/file.txt"]})", &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, GGetTagsOfFiles_Error_NullFiles) {
    // files为空
    char *result = nullptr;
    DataManagementStatus status = data_management_get_tags_of_files(session_, nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}