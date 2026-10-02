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
#include <set>

#include "app/uninstall/KMUninstall.h"
#include "common/KMPackageManager.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMUtil.h"

class KMTestUninstall
{
public:
    KMTestUninstall() = default;
    ~KMTestUninstall() = default;

public:
    KMUninstall m_uninstall;
    KMPackageManager m_pm;
};

TEST(KMTestUninstall, test_uninstall_dispose)
{
    KMTestUninstall testUninstall;

    // 不能卸载的系统核心包列表
    std::set<std::string> protectedPackages = {
        "top.openkylin.base",
        "top.openkylin.ukui",
        "cn.kylin.v10sp1-base"};

    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";

    // 查找第一个非保护包的应用
    std::string appid;
    for (const auto &ref : refs)
    {
        if (protectedPackages.find(ref.id) == protectedPackages.end())
        {
            appid = ref.id;
            break;
        }
    }

    // 如果没有找到非保护包，跳过测试
    if (appid.empty())
    {
        GTEST_SKIP() << "No non-protected package found for uninstall test";
    }

    std::cout << "Testing uninstall for package: " << appid << std::endl;

    // 使用动态获取的包名进行卸载测试
    const char *argv[] = {"uninstall", "-y", appid.c_str()};
    EXPECT_EQ(testUninstall.m_uninstall.dispose(3, const_cast<char **>(argv)), 0);
}