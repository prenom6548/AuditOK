/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include "globalutils.h"
#include <QDir>
#include <QUuid>

class TestGlobalUtils : public ::testing::Test {
};

// 测试时间戳格式
TEST_F(TestGlobalUtils, GetTimeFormatTest) {
    QString format = GlobalUtils::getTimeFormat(true);
    EXPECT_EQ(format, "yyyy-MM-dd HH:mm:ss.zzz");
    
    format = GlobalUtils::getTimeFormat(false);
    EXPECT_EQ(format, "yyyy-MM-dd HH:mm:ss");
}

// 测试 UUID 生成 (至少保证生成的是有效的 UUID)
TEST_F(TestGlobalUtils, GetUserUuidTest) {
    QString uuid = GlobalUtils::getUserUuid();
    EXPECT_FALSE(uuid.isEmpty());
    // 检查去掉了 {} 的 UUID
    EXPECT_FALSE(uuid.contains("{"));
    EXPECT_FALSE(uuid.contains("}"));
}

// 测试加密解密 (对称性)
TEST_F(TestGlobalUtils, EncryptUncryptTest) {
    QString original = "KylinTestMessage";
    QString encrypted = GlobalUtils::encryptData(original);
    QString decrypted = GlobalUtils::uncryptData(encrypted);
    
    EXPECT_NE(original, encrypted); // 加密后应不同
    EXPECT_EQ(original, decrypted); // 解密后应相同
}

// 测试获取平台名称
TEST_F(TestGlobalUtils, GetPlatformNameTest) {
    QString platform = GlobalUtils::getPlatformName();
    EXPECT_FALSE(platform.isEmpty());
    // 在 Linux 环境下运行
    EXPECT_EQ(platform, "Linux");
}

// 测试获取系统用户名
TEST_F(TestGlobalUtils, GetUsernameTest) {
    QString username = GlobalUtils::getUsername();
    EXPECT_FALSE(username.isEmpty());
    EXPECT_NE(username, "Unknown");
}

// 测试头像生成逻辑
TEST_F(TestGlobalUtils, GetAvatarByNameTest) {
    EXPECT_EQ(GlobalUtils::getAvatarByName("alice"), "A");
    EXPECT_EQ(GlobalUtils::getAvatarByName("Bob"), "B");
    EXPECT_EQ(GlobalUtils::getAvatarByName("张三"), "张");
    EXPECT_EQ(GlobalUtils::getAvatarByName(""), "");
}

// 测试可用端口获取
TEST_F(TestGlobalUtils, GetAvailTcpPortTest) {
    int port = GlobalUtils::getAvailTcpPort();
    EXPECT_GE(port, 39901); // DEFAULT_TCP_PORT 为 39901
}

// 测试默认路径生成
TEST_F(TestGlobalUtils, GetDefaultPathTest) {
    QString receivePath = GlobalUtils::getDefaultPath(false);
    QString sendPath = GlobalUtils::getDefaultPath(true);
    
    EXPECT_TRUE(receivePath.endsWith("/ipmsgReceivedFiles/"));
    EXPECT_TRUE(sendPath.endsWith("/ipmsgReceivedFiles/sendFiles/"));
    
    EXPECT_TRUE(QDir(receivePath).exists());
    EXPECT_TRUE(QDir(sendPath).exists());
}

// 测试文件夹大小计算
TEST_F(TestGlobalUtils, GetDirSizeTest) {
    // 创建临时测试目录和文件
    QString tempPath = QDir::tempPath() + "/kylin_test_dir_size";
    QDir().mkpath(tempPath);
    
    QFile file(tempPath + "/test.txt");
    if (file.open(QIODevice::WriteOnly)) {
        file.write("Hello World"); // 11 bytes
        file.close();
    }
    
    EXPECT_EQ(GlobalUtils::getDirSize(tempPath), 11);
    
    // 清理
    QDir(tempPath).removeRecursively();
}

