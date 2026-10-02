#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, CheckFileFormat_Success_SupportedFormat) {
    // 测试支持的文件格式
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/待办.txt";

    char *check_result = nullptr;
    DataManagementStatus status = data_management_check_file_format(session_, targetFilePath.c_str(), &check_result);

    EXPECT_TRUE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("", status.errorMsg);

    // 验证返回的JSON结果
    EXPECT_NE(nullptr, check_result);
    if (check_result) {
        // 这里可以添加JSON解析验证具体字段
        EXPECT_NE(nullptr, strstr(check_result, "\"supported\":true"));
        EXPECT_NE(nullptr, strstr(check_result, "\"format\":\"txt\""));
        data_management_destroy_result(check_result);
    }
}

TEST_F(DataManagementMockedTest, CheckFileFormat_Success_UnsupportedFormat) {
    // 测试不支持的文件格式

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path targetFilePath = dir / "conf/不支持的文件格式.123";

    char *check_result = nullptr;
    DataManagementStatus status = data_management_check_file_format(session_, targetFilePath.c_str(), &check_result);

    EXPECT_TRUE(status.success);
    EXPECT_EQ(0, status.errorCode);
    EXPECT_STREQ("", status.errorMsg);

    if (check_result) {
        EXPECT_NE(nullptr, strstr(check_result, "\"supported\":false"));
        EXPECT_NE(nullptr, strstr(check_result, "\"format\":\"123\""));
        data_management_destroy_result(check_result);
    }
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, CheckFileFormat_Error_FileNotExist) {
    // 测试文件不存在的情况

    char *check_result = nullptr;
    DataManagementStatus status =
        data_management_check_file_format(session_, "/path/to/nonexistent/file.xyz", &check_result);

    EXPECT_FALSE(status.success) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
    EXPECT_NE(0, status.errorCode);

    if (check_result) {
        data_management_destroy_result(check_result);
    }
}

TEST_F(DataManagementMockedTest, CheckFileFormat_Error_NullFilePath) {
    // 传入空文件路径
    char *check_result = nullptr;
    DataManagementStatus status = data_management_check_file_format(session_, nullptr, &check_result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, check_result);
}
TEST_F(DataManagementMockedTest, CheckFileFormat_Error_NullSession) {
    // 传入空Session
    char *check_result = nullptr;
    DataManagementStatus status = data_management_check_file_format(nullptr, nullptr, &check_result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);
    EXPECT_EQ(nullptr, check_result);
}

// ==================== 边界情况测试 (Edge Cases) ====================

TEST_F(DataManagementMockedTest, CheckFileFormat_Edge_EmptyFilePath) {
    // 测试空文件路径
    char *check_result = nullptr;
    DataManagementStatus status = data_management_check_file_format(session_, "", &check_result);

    EXPECT_FALSE(status.success);
    EXPECT_NE(0, status.errorCode);

    if (check_result) {
        data_management_destroy_result(check_result);
    }
}