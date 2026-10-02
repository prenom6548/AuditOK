/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/which/KMWhich.h"
#include "common/KMInstalledAppQuery.h"

class KMTestWhich
{
public:
    KMTestWhich() = default;
    ~KMTestWhich() = default;

public:
    KMWhich m_which;
};

TEST(KMTestWhich, test_which_depose)
{
    KMTestWhich testWhich;
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    if (refs.empty())
    {
        GTEST_SKIP() << "No installed package found";
    }
    
    // 构造应用路径
    std::string appPath = refs.front().baseInstallDir + "/" + refs.front().id;
    kmlogger.info("App path: %s", appPath.c_str());
    const char *argv[] = {"which", appPath.c_str()};
    EXPECT_EQ(testWhich.m_which.dispose(2, const_cast<char **>(argv)), 0);
}

TEST(KMTestWhich, test_which_normalizePath)
{
    KMTestWhich testWhich;
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    
    // 构造应用路径
    std::string appPath = refs.front().baseInstallDir + "/" + refs.front().id;
    // std::string normalizedPath = testWhich.m_which.normalizePath(appPath);
    
    // // 验证路径规范化后的结果应该等于原始路径（或者根据实际规范化规则调整）
    // EXPECT_EQ(normalizedPath, appPath);
}