/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/info/KMInfoCommand.h"
#include "common/KMInstalledAppQuery.h"

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
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    std::string appid = refs.front().id;
    
    const char* args[]={"info", appid.c_str()};
    EXPECT_EQ(testInfo.m_info.dispose(2,const_cast<char**>(args)), 0);
    
    // info -L listFilesDirectory(),listFilesDirectory()
    const char* args2[]={"info","-L", appid.c_str()};
    EXPECT_EQ(testInfo.m_info.dispose(3,const_cast<char**>(args2)), 0);
    
    EXPECT_EQ(testInfo.m_info.start(appid.c_str()), 0);
    
    int arg=0;
    std::string result = testInfo.m_info.getLocalInfo(appid.c_str(), arg);
    EXPECT_FALSE(result.empty());
}

// 测试获取本地deb信息（给软件商店调用的api）
TEST(KMTestInfo, test_getLocalDebInfo)
{
    KMTestInfo testInfo;
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    std::string appid = refs.front().id;
    
    int errorCode = -1;
    
    // 测试获取本地deb信息
    std::string debInfoJson = testInfo.m_info.getLocalDebInfo(appid, errorCode);
    
    // 验证结果
    EXPECT_EQ(errorCode, 0) << "getLocalDebInfo should return success error code";
}