/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/update/KMUpdate.h"
#include "common/KMInstalledAppQuery.h"

class KMTestUpdate
{
public:
    KMTestUpdate() = default;
    ~KMTestUpdate() = default;

public:
    KMUpdate m_update;
};

TEST(KMTestUpdate, test_update_depose)
{
    KMTestUpdate testUpdate;

    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    std::string appid = refs.front().id;
    
    const char *argv[] = {"update", appid.c_str()};
    EXPECT_GE(testUpdate.m_update.dispose(2, const_cast<char **>(argv)), 0);
}

// 测试更新所有可更新的包（不带包名参数）
TEST(KMTestUpdate, test_update_all)
{
    KMTestUpdate testUpdate;
    
    // 执行 sudo kaiming update（不带包名，更新所有可更新的包）
    const char *argv[] = {"update"};
    int result = testUpdate.m_update.dispose(1, const_cast<char **>(argv));
    
    // 验证结果（返回0表示成功，或者返回其他代码表示没有可更新的包）
    EXPECT_GE(result, 0) << "Update all should succeed or return appropriate code";
    
    std::cout << "Update all result: " << result << std::endl;
}

// 测试查看可更新的包列表（kaiming update --upgradable）
TEST(KMTestUpdate, test_update_upgradable)
{
    KMTestUpdate testUpdate;
    
    // 执行 kaiming update --upgradable（查看可更新的包列表）
    const char *argv[] = {"update", "--upgradable"};
    int result = testUpdate.m_update.dispose(2, const_cast<char **>(argv));
    
    // 验证结果（返回0表示成功，或者返回其他代码表示没有可更新的包）
    EXPECT_GE(result, 0) << "Update --upgradable should succeed or return appropriate code";
    
    std::cout << "Update --upgradable result: " << result << std::endl;
}