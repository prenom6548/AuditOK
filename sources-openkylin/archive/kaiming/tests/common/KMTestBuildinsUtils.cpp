/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */


#include <gtest/gtest.h>

#include "common/KMBuildinsUtils.h"

class KMBuildinsUtilsTest : public ::testing::Test {
protected:
    std::string errMsg;
};

// 测试合法APP ID格式
TEST_F(KMBuildinsUtilsTest, AcceptValidAppIds) {
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("top.openkylin.recorder", errMsg));
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("org.gnome.terminal", errMsg));
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("io.github.project-123", errMsg));
}

// 测试非法格式
TEST_F(KMBuildinsUtilsTest, RejectInvalidFormats) {
    // 元素不足3个
    EXPECT_FALSE(KMBuildinsUtils::isValidAppName("openkylin.recorder", errMsg));
    
    // 包含非法字符
    EXPECT_FALSE(KMBuildinsUtils::isValidAppName("top.openkylin.recorder!", errMsg));
    
    // 数字开头
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("top.1kylin.recorder", errMsg));
}

// 测试边界长度
TEST_F(KMBuildinsUtilsTest, HandleLengthBoundaries) {
    std::string maxValid(255, 'a');  // 构造255字符的合法ID
    maxValid[0] = 't'; maxValid[1] = '.'; maxValid[2] = 'e';
    
    EXPECT_FALSE(KMBuildinsUtils::isValidAppName(maxValid + "a", errMsg));
}

// 测试横线位置规则
TEST_F(KMBuildinsUtilsTest, ValidateDashPosition) {
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("top.openkylin.rec-order", errMsg));
    EXPECT_TRUE(KMBuildinsUtils::isValidAppName("top.open-kylin.recorder", errMsg));
}

// 测试字符级校验
TEST_F(KMBuildinsUtilsTest, CheckCharacterValidation) {
    EXPECT_TRUE(KMBuildinsUtils::isValidNameCharacter('a', false));
    EXPECT_FALSE(KMBuildinsUtils::isValidNameCharacter('@', false));
    EXPECT_TRUE(KMBuildinsUtils::isValidInitialNameCharacter('K', true));
}
