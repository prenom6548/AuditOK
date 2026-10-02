#include "mocks/DataManagementMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(DataManagementMockedTest, IsFeatureSupported_Success_Basic) {
    // 测试正常情况
    // 这个接口只有在x86和arm架构下会返回true
    bool isTrue = data_management_is_feature_supported();

    // 测试一些分支情况 获取session并销毁
    DataManagementSession* session = data_management_create_session();
    data_management_destroy_session(session);
    data_management_destroy_session(nullptr);
}