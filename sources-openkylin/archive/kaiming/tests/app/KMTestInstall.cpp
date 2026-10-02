/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "app/install/KMInstall.h"
#include "common/KMPackageManager.h"
#include "common/KMUtil.h"
#include "common/KMLogger.h"

class KMTestInstall
{
public:
    KMTestInstall() = default;
    ~KMTestInstall() = default;

public:
    KMInstall m_install;
    KMPackageManager m_pm;
};

TEST(KMTestInstall, test_install_depose)
{
    KMTestInstall testInstall;
    
    // 动态获取可用的包名
    std::vector<std::string> searchResults;
    PackageManagerOptions pmOptions;
    pmOptions.isPrint = false;
    pmOptions.kind = "app";
    
    kmlogger.info("begin to search");
    // 搜索所有可用的包
    int searchRet = testInstall.m_pm.searchInfo("", pmOptions, searchResults);
    kmlogger.info("search ret:", "%d", searchRet);
    
    // 获取第一个搜索结果并转换为KMRef
    std::string firstResult = searchResults.front();
    KMPMRef pmRef;
    try {
        pmRef = KMUtil::parseSummaryFuzzyKMPMRef(firstResult);
    } catch (const std::exception& e) {
        FAIL() << "Failed to parse package reference: " << e.what();
    }
    
    std::string packageId = pmRef.ref.id;
    
    // 使用动态获取的包名进行安装测试
    const char *argv[] = {"install", "-y", packageId.c_str()};
    EXPECT_EQ(testInstall.m_install.dispose(3, const_cast<char **>(argv)), 0);
}