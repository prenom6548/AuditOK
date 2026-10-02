/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMInfoCommand.h"

#include <gtest/gtest.h>

#define private public
// #define public private

class KMTestInfo
{
public:
    KMTestInfo() = default;
    ~KMTestInfo() = default;

public:
    KMInfoCommand m_info;
};

TEST(KMTestInfo, test_info)
{
    KMTestInfo testInfo;
    const char* args[]={"info","top.openkylin.Recorder"};
    EXPECT_EQ(testInfo.m_info.dispose(2,const_cast<char**>(args)), 0);
    // info -L listFilesDirectory(),listFilesDirectory()
    const char* args2[]={"info","-L","top.openkylin.Recorder"};
    EXPECT_EQ(testInfo.m_info.dispose(3,const_cast<char**>(args2)), 0);
    EXPECT_EQ(testInfo.m_info.start("top.openkylin.Recorder"), 0);
    int arg=0;
    EXPECT_EQ(testInfo.m_info.getLocalInfo("top.openkylin.Recorder",arg), R"({"branch":"stable","name":"top.openkylin.Recorder","size":"4537373","version":""})");
}
