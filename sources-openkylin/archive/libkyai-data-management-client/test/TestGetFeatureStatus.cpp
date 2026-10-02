#include <filesystem>

#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, GetFeatureStatus_Success_Available) {
    // 测试功能可用状态
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetFeatureStatus(_, _, _, _, _, _, _))
        .WillOnce([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                     gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            auto *result = new std::string(R"({"err_no":0,"err_msg":"请求成功","data":{"result":0}})");
            *out_output = g_strdup(result->c_str());
            return true;
        });

    FeatureStatus status = FeatureStatus::UNKNOWN;
    DataManagementStatus dm_status = data_management_get_feature_status(session_, &status);
    EXPECT_TRUE(dm_status.success) << "errno: " << dm_status.errorCode << ", errmsg: " << dm_status.errorMsg;
    EXPECT_EQ(0, dm_status.errorCode);
    EXPECT_STREQ("请求成功", dm_status.errorMsg);
    EXPECT_EQ(FeatureStatus::AVAILABLE, status);
#endif
}

// ==================== 错误场景测试 (Error Scenarios) ====================

TEST_F(DataManagementMockedTest, GetFeatureStatus_Error_NullSession) {
    // session为空

    FeatureStatus result = FeatureStatus::UNKNOWN;
    DataManagementStatus DmStatus = data_management_get_feature_status(nullptr, &result);
    EXPECT_FALSE(DmStatus.success) << "errno: " << DmStatus.errorCode << ", errmsg: " << DmStatus.errorMsg;
    EXPECT_NE(0, DmStatus.errorCode);
}

TEST_F(DataManagementMockedTest, GetFeatureStatus_Error_ServiceUnavailable) {
    // 模拟服务不可用错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetFeatureStatus(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_SERVICE_UNKNOWN, "Service unavailable");
            return false;
        });

    FeatureStatus status = FeatureStatus::UNKNOWN;
    DataManagementStatus dm_status = data_management_get_feature_status(session_, &status);
    EXPECT_FALSE(dm_status.success);
    EXPECT_NE(0, dm_status.errorCode);
#endif
}

TEST_F(DataManagementMockedTest, GetFeatureStatus_Error_Timeout) {
    // 模拟超时错误
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    EXPECT_CALL(mock_, GetFeatureStatus(_, _, _, _, _, _, _))
        .WillRepeatedly([](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_TIMED_OUT, "Operation timed out");
            return false;
        });

    FeatureStatus status = FeatureStatus::AVAILABLE;
    DataManagementStatus dm_status = data_management_get_feature_status(session_, &status);
    EXPECT_FALSE(dm_status.success);
    EXPECT_NE(0, dm_status.errorCode);
#endif
}