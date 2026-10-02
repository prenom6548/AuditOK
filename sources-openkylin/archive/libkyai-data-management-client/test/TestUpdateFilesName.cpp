#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, UpdateFilesName_Success_Basic) {
    // 测试正常情况 - 更新多种格式的文件名
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, UpdateFilesName(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功"})");
            *out_output = result->data();
            return true;
        });
#endif

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path oldFilePath1 = dir / "conf/aiassistant_20240709_191804.png";
    fs::path newFilePath1 = dir / "conf/aiassistant_20240709_191810.png";

    std::string filePath = R"(
    [
      {
        "old_filepath": ")" +
                           oldFilePath1.string() + R"(",
        "new_filepath": ")" +
                           newFilePath1.string() + R"("
      }
    ])";

    DataManagementStatus status = data_management_update_files_name(session_, filePath.c_str());
    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("请求成功", status.errorMsg);
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, UpdateFilesName_Error_NullSession) {
    // 传入空指针
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path oldFilePath1 = dir / "conf/aiassistant_20240709_191804.png";
    fs::path newFilePath1 = dir / "conf/aiassistant_20240709_191810.png";

    std::string filePath = R"(
    [
      {
        "old_filepath": ")" +
                           oldFilePath1.string() + R"(",
        "new_filepath": ")" +
                           newFilePath1.string() + R"("
      }
    ])";

    DataManagementStatus status = data_management_update_files_name(nullptr, filePath.c_str());
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}

TEST_F(DataManagementMockedTest, UpdateFilesName_Error_NullSearchConditions) {
    // 传入空指针
    DataManagementStatus status = data_management_update_files_name(session_, nullptr);
    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
}