/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include "common/KMStorageDir.h"

// 测试 getUserDevelDataDirLocation()
TEST(KMStorageDirTest, GetUserDevelDataDirLocation) {
    // 只验证函数能执行且返回非空字符串
    std::string path = KMStorageDir::getUserDevelDataDirLocation();
    EXPECT_FALSE(path.empty());
}

// 测试 getLoginUser()
TEST(KMStorageDirTest, GetLoginUser) {
    // 只验证函数能执行（返回值可能为空，取决于环境）
    std::string user = KMStorageDir::getLoginUser();
    EXPECT_NO_THROW();  // 仅确保不抛异常
}