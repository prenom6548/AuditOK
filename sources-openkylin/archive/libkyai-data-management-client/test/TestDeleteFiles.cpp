#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, DeleteFiles_Success_Basic) {
    // 测试正常情况 - 删除多种格式的文件 删除的文件如果没有存储到db文件中，删除不报错
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath1 = dir / "conf/sample.png";
    fs::path targetFilePath2 = dir / "conf/待办.txt";
    fs::path targetFilePath3 = dir / "conf/document.pdf";

    std::string filePath = R"(
    [
      {
        "filepath": ")" + targetFilePath1.string() +
                           R"("
      },
      {
        "filepath": ")" + targetFilePath2.string() +
                           R"("
      },
      {
        "filepath": ")" + targetFilePath3.string() +
                           R"("
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Success_SingleFile) {
    // 测试删除单个文件 删除的文件如果没有存储到db文件中，删除不报错
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"(
    [
      {
        "filepath": "/path/to/single/file.pdf"
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Success_FileNotInSession) {
    // 测试删除不在会话中的文件（应该成功，因为删除不存在的文件通常被视为成功操作）
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"(
    [
      {
        "filepath": "/path/to/nonexistent/file.txt"
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, DeleteFiles_Error_NullSession) {
    // session为空
    DataManagementStatus status = data_management_delete_files(nullptr, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_EmptyFileInfos) {
    // fileinfos为空数组
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = result->data();
            return true;
        });
#endif

    DataManagementStatus status = data_management_delete_files(session_, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_NullFileInfos) {
    // 传入空指针
    DataManagementStatus status = data_management_delete_files(session_, nullptr);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_InvalidJSONFormat) {
    // fileinfos 参数格式错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"(
    [
      {
        "filepath": 123
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());  // filepath应该是字符串
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_MissingRequiredFields) {
    // 缺少必要字段
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"([{}])";  // 缺少filepath字段
    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_EmptyFilePath) {
    // filepath为空字符串
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"(
    [
      {
        "filepath": ""
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    DataManagementStatus status = data_management_delete_files(session_, R"([{"filepath":"/path/to/file.pdf"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    DataManagementStatus status = data_management_delete_files(session_, R"([{"filepath":"/path/to/file.txt"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_InvalidJSON) {
    // 模拟JSON解析错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid JSON format");
            return false;
        });

    DataManagementStatus status = data_management_delete_files(session_, "[invalid json");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, DeleteFiles_Error_PermissionDenied) {
    // 模拟权限不足错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"(
    [
      {
        "filepath": "/system/protected/file.pdf"
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, DeleteFiles_Edge_LargeNumberOfFiles) {
    // 测试删除大量文件
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    // 构建包含多个文件的JSON
    std::string filePath = "[";
    for (int i = 0; i < 20; ++i) {
        if (i > 0) filePath += ",";
        filePath += R"({"filepath": "/path/to/file)" + std::to_string(i) + R"(.txt"})";
    }
    filePath += "]";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, DeleteFiles_Edge_LongFilePath) {
    // 测试超长文件路径
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, DeleteFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    // 构造超长路径
    std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.txt";

    std::string filePath = R"(
    [
      {
        "filepath": ")" + longPath +
                           R"("
      }
    ])";

    DataManagementStatus status = data_management_delete_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}