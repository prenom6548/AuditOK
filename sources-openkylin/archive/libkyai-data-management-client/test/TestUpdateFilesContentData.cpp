#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, UpdateFilesContent_Success_Basic) {
    // 测试正常情况 - 更新多种格式文件的向量数据
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath1 = dir / "conf/aiassistant_20240709_191804.png";
    fs::path filePath2 = dir / "conf/待办.txt";
    fs::path filePath3 = dir / "conf/aiassistant_20240709_191810.png";

    std::string fileInfos = R"(
    [
      {
        "filepath": ")" + filePath1.string() +
                            R"(",
        "fileformat": "png"
      },
      {
        "filepath": ")" + filePath2.string() +
                            R"(",
        "fileformat": "txt"
      },
      {
        "filepath": ")" + filePath3.string() +
                            R"(",
        "fileformat": "png"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Success_EmptyFileFormat) {
    // 测试fileformat为空的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/aiassistant_20240709_191804.png";

    std::string fileInfos = R"(
    [
      {
        "filepath": ")" + filePath.string() +
                            R"(",
        "fileformat": ""
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_NullSession) {
    // 传入空指针
    DataManagementStatus status = data_management_update_files_content(nullptr, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_EmptyFileInfos) {
    // fileinfos为空数组
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = result->data();
            return true;
        });
#endif

    DataManagementStatus status = data_management_update_files_content(session_, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_NullFileInfos) {
    // 传入空指针
    DataManagementStatus status = data_management_update_files_content(session_, nullptr);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_InvalidJSONFormat) {
    // fileinfos 参数格式错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string fileInfos = R"(
    [
      {
        "filepath": 123,
        "fileformat": "pdf"
      }
    ])";

    DataManagementStatus status =
        data_management_update_files_content(session_, fileInfos.c_str());  // filepath应该是字符串
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_MissingRequiredFields) {
    // 缺少必要字段
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string fileInfos = R"([{"fileformat":"pdf"}])";  // 缺少filepath字段
    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_EmptyFilePath) {
    // filepath为空字符串
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string fileInfos = R"(
    [
      {
        "filepath": "",
        "fileformat": "pdf"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_UnsupportedFileFormat) {
    // 测试不支持的格式
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string fileInfos = R"(
    [
      {
        "filepath": "/path/to/file.xyz",
        "fileformat": "xyz"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_FileNotExists) {
    // 文件不存在
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string fileInfos = R"(
    [
      {
        "filepath": "/nonexistent/path/file.pdf",
        "fileformat": "pdf"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    DataManagementStatus status =
        data_management_update_files_content(session_, R"([{"filepath":"/path/to/file.pdf","fileformat":"pdf"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    DataManagementStatus status =
        data_management_update_files_content(session_, R"([{"filepath":"/path/to/file.txt","fileformat":"txt"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Error_InvalidJSON) {
    // 模拟JSON解析错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid JSON format");
            return false;
        });

    DataManagementStatus status = data_management_update_files_content(session_, "[invalid json");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, UpdateFilesContent_Edge_LargeNumberOfFiles) {
    // 测试更新大量文件的向量数据
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    // 构建包含多个文件的JSON
    std::string fileInfos = "[";
    for (int i = 0; i < 20; ++i) {
        if (i > 0) fileInfos += ",";
        fileInfos += R"({"filepath": "/path/to/file)" + std::to_string(i) + R"(.txt", "fileformat": "txt"})";
    }
    fileInfos += "]";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, UpdateFilesContent_Edge_LongFilePath) {
    // 测试超长文件路径
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesContentData(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    // 构造超长路径
    std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.pdf";

    std::string fileInfos = R"(
    [
      {
        "filepath": ")" + longPath +
                            R"(",
        "fileformat": "pdf"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}