/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "kylin-com4cxx.h"
#include "kylin-ini.h"
#include "kylin-com4c.h"
#include <QObject>

class CommonCxxInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite() {}

    static void TearDownTestSuite() {}
};

TEST_F(CommonCxxInterfaceTest, getLsbRelease)
{
    QString strvalue = QString(KDKGetLSBRelease("DISTRIB_ID").c_str());
    EXPECT_EQ(strvalue, "Kylin");
}

TEST_F(CommonCxxInterfaceTest, getOsRelease)
{
    QString strvalue = QString(KDKGetOSRelease("NAME").c_str());
    EXPECT_EQ(strvalue, "Kylin");
}

TEST_F(CommonCxxInterfaceTest, getKyInfo)
{
    QString strvalue = QString(KDKGetKYInfo("dist", "name").c_str());
    EXPECT_EQ(strvalue, "");
}

TEST_F(CommonCxxInterfaceTest, getPrjCodeName)
{
    QString strvalue = QString(KDKGetPrjCodeName().c_str());
    EXPECT_EQ(strvalue.contains("V1"), true);
}

TEST_F(CommonCxxInterfaceTest, getCpuModelName)
{
    QString strvalue = QString(KDKGetCpuModelName().c_str());
    EXPECT_EQ(strvalue.isEmpty(), false);
}

TEST_F(CommonCxxInterfaceTest, getSpecHDPlatform)
{
    QString strvalue = QString(KDKGetSpecHDPlatform().c_str());
    EXPECT_EQ(strvalue, "default");
}

TEST_F(CommonCxxInterfaceTest, getVirtEnv)
{
    QString strvalue = QString(KDKGetVirtEnv().c_str());
    EXPECT_EQ(strvalue.contains("none"), true);
}

// 辅助函数，用于比较两个字符串是否相等（忽略结尾的空字符）
bool StringsAreEqual(const char *str1, const char *str2)
{
    if (str1 == nullptr && str2 == nullptr)
        return true;
    if (str1 == nullptr || str2 == nullptr)
        return false;
    return strcmp(str1, str2) == 0;
}

// 测试strstripr函数
TEST(StrStripTest, StrStriprTest_NoSpaces)
{
    char str[] = "hello";
    char *result = strstripr(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriprTest_TrailingSpaces)
{
    char str[] = "hello   ";
    char *result = strstripr(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriprTest_TrailingNewline)
{
    char str[] = "hello\n";
    char *result = strstripr(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriprTest_TrailingQuote)
{
    char str[] = "hello\"";
    char *result = strstripr(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriprTest_TrailingQuoteAndSpaces)
{
    char str[] = "hello \" ";
    char *result = strstripr(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, "hello "));
    result = strstripr(result, 0); // 再次去除尾部空格
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

// 测试strstripl函数
TEST(StrStripTest, StrStriplTest_NoSpaces)
{
    char str[] = "hello";
    char *result = strstripl(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriplTest_LeadingSpaces)
{
    char str[] = "   hello";
    char *result = strstripl(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriplTest_LeadingNewline)
{
    char str[] = "\nhello";
    char *result = strstripl(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriplTest_LeadingQuote)
{
    char str[] = "\"hello";
    char *result = strstripl(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStriplTest_LeadingQuoteAndSpaces)
{
    char str[] = " \" hello";
    char *result = strstripl(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, " hello"));
    result = strstripl(result, 0); // 再次去除首部空格
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

// 测试strstrip函数
TEST(StrStripTest, StrStripTest_NoSpaces)
{
    char str[] = "hello";
    char *result = strstrip(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStripTest_LeadingAndTrailingSpaces)
{
    char str[] = "   hello   ";
    char *result = strstrip(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStripTest_LeadingNewlineTrailingSpaces)
{
    char str[] = "\nhello   ";
    char *result = strstrip(str, 0);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStripTest_LeadingAndTrailingQuote)
{
    char str[] = "\"hello\"";
    char *result = strstrip(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

TEST(StrStripTest, StrStripTest_Complex)
{
    char str[] = " \" hello \" \t\n";
    char *result = strstrip(str, 1);
    EXPECT_TRUE(StringsAreEqual(result, "hello"));
}

class KylinCom4cTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite() {}

    static void TearDownTestSuite() {}
};

TEST_F(KylinCom4cTest, GetLsbRelease)
{
    const char *key = "DISTRIB_ID";
    char value[256];
    int result = kdk_get_lsbrelease(key, value, sizeof(value));
    EXPECT_GT(result, 0) << "kdk_get_lsbrelease() failed to get value";
    EXPECT_NE(std::string(value).find("Ubuntu"), std::string::npos)
        << "Unexpected LSB release value: " << value; // 以Ubuntu为例，根据实际情况修改
}

TEST_F(KylinCom4cTest, GetOsRelease)
{
    const char *key = "PRETTY_NAME";
    char value[256];
    int result = kdk_get_osrelease(key, value, sizeof(value));
    EXPECT_GT(result, 0) << "kdk_get_osrelease() failed to get value";
    EXPECT_NE(std::string(value).find("Ubuntu"), std::string::npos)
        << "Unexpected OS release value: " << value; // 以Ubuntu为例，根据实际情况修改
}

TEST_F(KylinCom4cTest, GetPrjCodename)
{
    char value[256];
    int result = kdk_get_prjcodename(value, sizeof(value));
    EXPECT_GT(result, 0) << "kdk_get_prjcodename() failed to get value";
    // 根据实际情况验证PROJECT_CODENAME的值
}

TEST_F(KylinCom4cTest, GetCpuModelName)
{
    char modelName[128];
    int result = kdk_get_cpumodelname(modelName, sizeof(modelName));
    EXPECT_GT(result, 0) << "kdk_get_cpumodelname() failed to get value";
    // 根据实际情况验证CPU型号的值
}

TEST_F(KylinCom4cTest, GetSpechdPlatform)
{
    char platformName[128];
    int result = kdk_get_spechdplatform(platformName, sizeof(platformName));
    EXPECT_GT(result, 0) << "kdk_get_spechdplatform() failed to get value";
    // 根据实际情况验证特定硬件平台信息的值，可能默认为"default"
}

TEST_F(KylinCom4cTest, GetVirtEnv)
{
    char virtenv[128];
    int result = kdk_get_virtenv(virtenv, sizeof(virtenv));
    EXPECT_GT(result, 0) << "kdk_get_virtenv() failed to get value";
    // 根据实际情况验证虚拟化环境标志的值，可能为"none"、"kvm"等
}
