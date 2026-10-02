#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, AddTextFiles_Success_Basic) {
// 测试正常情况 - 添加多种格式的文件
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/待办.txt";
    std::string filePath = R"(
    [
      {
        "filepath": ")" + targetFilePath.string() +
                           R"(",
        "fileformat": "txt"
      }
    ])";

    DataManagementStatus status = data_management_add_text_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Success_EmptyFileFormat) {
// 测试fileformat为空，现在服务端虽然没有直接使用fileformat，但是会检查fileformat键位是否存在，是否为字符串
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/待办.txt";
    std::string filePath = R"(
    [
      {
        "filepath": ")" + targetFilePath.string() +
                           R"(",
        "fileformat": ""
      }
    ])";

    DataManagementStatus status = data_management_add_text_files(session_, filePath.c_str());
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, AddTextFiles_Error_NullSession) {
    // session为空
    DataManagementStatus status = data_management_add_text_files(nullptr, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_EmptyFileInfos) {
// fileinfos为空数组
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":14,"err_msg":"解析参数失败"})");
            *out_output = result->data();
            return true;
        });
#endif

    DataManagementStatus status = data_management_add_text_files(session_, "[]");
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
    EXPECT_STREQ("解析参数失败", status.errorMsg);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_NullFileInfos) {
    // 传入空指针
    DataManagementStatus status = data_management_add_text_files(session_, nullptr);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_InvalidJSONFormat) {
// fileinfos 参数格式错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/待办.txt";
    std::string filePath = R"(
    [
      {
        "filepath": 123,
        "fileformat": "pdf"
      }
    ])";

    DataManagementStatus status = data_management_add_text_files(session_, filePath.c_str());  // filepath应该是字符串
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_MissingRequiredFields) {
// 缺少必要字段
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    std::string filePath = R"([{"fileformat":"pdf"}])";
    DataManagementStatus status = data_management_add_text_files(session_, filePath.c_str());  // 缺少filepath字段
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_ServiceUnavailable) {
// 模拟服务不可用错误 在打开mock宏时，才执行该条用例
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    DataManagementStatus status =
        data_management_add_text_files(session_, R"([{"filepath":"/path/to/file.pdf","fileformat":"pdf"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_Timeout) {
// 模拟超时错误 在打开mock宏时，才执行该条用例
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    DataManagementStatus status =
        data_management_add_text_files(session_, R"([{"filepath":"/path/to/file.pdf","fileformat":"pdf"}])");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, AddTextFiles_Error_InvalidJSON) {
// 模拟JSON解析错误 在打开mock宏时，才执行该条用例
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, AddTextFiles(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid JSON format");
            return false;
        });

    DataManagementStatus status = data_management_add_text_files(session_, "[invalid json");
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
#endif
}