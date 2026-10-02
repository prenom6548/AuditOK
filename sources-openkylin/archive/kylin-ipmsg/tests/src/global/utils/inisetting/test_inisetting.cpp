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

#include "test_inisetting.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

void TestIniSettings::SetUp()
{
    // Set up test configuration directory
    QString configDir = tempDir.path() + "/.config/ipmsg";
    QDir().mkpath(configDir);
    
    // Set HOME environment variable to our temporary directory
    qputenv("HOME", tempDir.path().toUtf8());
}

void TestIniSettings::TearDown()
{
    // Clean up is handled by QTemporaryDir automatically
}

TEST_F(TestIniSettings, GetInstance)
{
    IniSettings* instance1 = IniSettings::getInstance();
    IniSettings* instance2 = IniSettings::getInstance();
    
    ASSERT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2); // Should be singleton
}

TEST_F(TestIniSettings, SetValueAndGetValue)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test setting and getting a value
    QString group = "TestGroup";
    QString key = "TestKey";
    QString value = "TestValue";
    
    settings->setValue(group, key, value);
    
    QVariant retrievedValue = settings->value(group, key);
    EXPECT_EQ(retrievedValue.toString(), value);
}

TEST_F(TestIniSettings, Contains)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test checking if a key exists
    QString group = "ContainsTestGroup";
    QString key = "ContainsTestKey";
    
    // Key should not exist initially in a new group
    EXPECT_FALSE(settings->containts(group, key)) << "Key should not exist in new group";
    
    // Set the value
    QString value = "ContainsTestValue";
    settings->setValue(group, key, value);
    
    // Key should now exist
    EXPECT_TRUE(settings->containts(group, key)) << "Key should exist after setting";
}

TEST_F(TestIniSettings, RemoveKey)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test removing a key
    QString group = "RemoveKeyTestGroup";
    QString key = "RemoveKeyTestKey";
    QString value = "RemoveKeyValue";
    
    // Set the value
    settings->setValue(group, key, value);
    EXPECT_TRUE(settings->containts(group, key)) << "Key should exist after setting";
    
    // Remove the key
    settings->removeKey(group, key);
    EXPECT_FALSE(settings->containts(group, key)) << "Key should not exist after removal";
}

TEST_F(TestIniSettings, RemoveNode)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test removing a node/section
    QString group1 = "RemoveNodeTestGroup1";
    QString group2 = "RemoveNodeTestGroup2";
    QString key = "RemoveNodeTestKey";
    QString value = "RemoveNodeValue";
    
    // Set values in both groups
    settings->setValue(group1, key, value);
    settings->setValue(group2, key, value);
    
    EXPECT_TRUE(settings->containts(group1, key)) << "Key should exist in group1";
    EXPECT_TRUE(settings->containts(group2, key)) << "Key should exist in group2";
    
    // Remove the first group
    settings->removeNode(group1);
    EXPECT_FALSE(settings->containts(group1, key)) << "Key should not exist in group1 after removal";
    EXPECT_TRUE(settings->containts(group2, key)) << "Key should still exist in group2";
}

TEST_F(TestIniSettings, LocalNickname)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test local nickname functions
    // Note: The nickname key already exists due to initSettings(), but it's empty
    EXPECT_TRUE(settings->isExistNickname());  // Key exists but is empty
    
    QString nickname = "TestNickname";
    
    // Set nickname
    settings->setLocalNickname(nickname);
    
    // Check the value
    EXPECT_EQ(settings->getLocalNickname(), nickname);
}

TEST_F(TestIniSettings, LocalTcpPort)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test local TCP port functions
    int port = 12345;
    
    // Set port
    settings->setLocalTcpPort(port);
    
    // Check the value
    EXPECT_EQ(settings->getLocalTcpPort(), port);
}

TEST_F(TestIniSettings, LocalUuid)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test local UUID functions
    QString uuid = "test-uuid-12345";
    
    // Set UUID
    settings->setLocalUuid(uuid);
    
    // Check the value
    EXPECT_EQ(settings->getLocalUuid(), uuid);
}

TEST_F(TestIniSettings, FilePath)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test file path functions
    QString filePath = "/test/path/to/file";
    
    // Set file path
    settings->setFilePath(filePath);
    
    // Check the value
    EXPECT_EQ(settings->getFilePath(), filePath);
}

TEST_F(TestIniSettings, Path)
{
    IniSettings* settings = IniSettings::getInstance();
    ASSERT_NE(settings, nullptr);
    
    // Test getting the configuration file path
    QString path = settings->path();
    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("Config.ini"));
}