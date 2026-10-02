#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================
TEST_F(DataManagementMockedTest, GetAllFileinfos_Success_Basic) {
    // 先添加文件到数据库中，在获取所有文件信息
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

    std::string fileInfos = R"(
    [
      {
        "filepath": ")" + filePath1.string() +
                            R"(",
        "fileformat": "png"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);

    // 测试正常情况 - 获取所有文件信息
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillOnce([=](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                      gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string response = R"({"err_no":0,"err_msg":"success","data":)" + fileInfos + "}";
            // 使用g_strdup确保内存正确分配
            *out_output = g_strdup(response.c_str());
            return true;
        });
#endif

    char *result = nullptr;
    status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);

    // 验证返回结果
    EXPECT_NE(nullptr, result);
    // 清理资源
    if (result != nullptr) {
        data_management_destroy_result(result);
    }
}

TEST_F(DataManagementMockedTest, GetAllFileinfos_Success_MultipleFiles) {
    // 先添加文件到数据库中，在获取所有文件信息
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
              "filepath": ")" +
                            filePath2.string() +
                            R"(",
        "fileformat": "txt"
      },
      {
              "filepath": ")" +
                            filePath3.string() +
                            R"(",
        "fileformat": "png"
      }
    ])";

    DataManagementStatus status = data_management_update_files_content(session_, fileInfos.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);

    // 测试正常情况 - 获取所有文件信息
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillOnce([=](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                      gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string response = R"({"err_no":0,"err_msg":"success","data":)" + fileInfos + "}";
            // 使用g_strdup确保内存正确分配
            *out_output = g_strdup(response.c_str());
            return true;
        });
#endif

    char *result = nullptr;
    status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);

    // 验证返回结果
    EXPECT_NE(nullptr, result);
    // 清理资源
    if (result != nullptr) {
        data_management_destroy_result(result);
    }
}

TEST_F(DataManagementMockedTest, GetAllFileinfos_Success_EmptyList) {
    // 测试空文件列表的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"success","data":[]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("success", status.errorMsg);
    EXPECT_STREQ("[]", result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, GetAllFileinfos_Error_NullSession) {
    // session为空

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(nullptr, &result);
    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, GetAllFileinfos_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);  // 确保在错误情况下result没有被修改
#endif
}

TEST_F(DataManagementMockedTest, GetAllFileinfos_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, result);
#endif
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, GetAllFileinfos_Edge_LargeNumberOfFiles) {
    // 测试大量文件的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            // 构建包含大量文件的JSON响应
            std::string json = R"({"err_no":0,"err_msg":"success","data":[)";
            for (int i = 0; i < 50; ++i) {
                if (i > 0) json += ",";
                json += R"({"filepath":"/path/to/file)" + std::to_string(i) + R"(.txt","timestamp":)" +
                        std::to_string(1609459200 + i) + "}";
            }
            json += "]}";
            auto *result = new std::string(json);
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("success", status.errorMsg);

    // 检查结果是否包含预期的文件数量
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}

TEST_F(DataManagementMockedTest, GetAllFileinfos_Edge_LongFilePaths) {
    // 测试超长文件路径的情况
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetAllFilesInfos(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            std::string longPath = "/very/long/" + std::string(200, 'a') + "/file.txt";
            auto *result = new std::string(R"({"err_no":0,"err_msg":"success","data":[)"
                                           R"({"filepath":")" +
                                           longPath +
                                           R"(","timestamp":1234567890})"
                                           R"(]})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    char *result = nullptr;
    DataManagementStatus status = data_management_get_all_fileinfos(session_, &result);
    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("success", status.errorMsg);
    EXPECT_NE(nullptr, result);

    if (result != nullptr) {
        data_management_destroy_result(result);
    }
#endif
}