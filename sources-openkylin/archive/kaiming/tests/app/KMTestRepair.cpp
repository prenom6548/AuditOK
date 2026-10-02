/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/repair/KMRepair.h"
#include "common/KMInstalledAppQuery.h"

class KMTestRepair
{
public:
    KMTestRepair() = default;
    ~KMTestRepair() = default;

public:
    KMRepair m_repair;
};

TEST(KMTestRepair, test_repair)
{
    KMTestRepair testRepair;
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    if (refs.empty())
    {
        GTEST_SKIP() << "No installed package found";
    }

    std::string appid = refs.front().id;
    
    const char* args[]={"repair", appid.c_str()};
    EXPECT_GE(testRepair.m_repair.dispose(2,const_cast<char**>(args)), 0);
    const vector<string> &vectorIds = {appid};
    EXPECT_EQ(testRepair.m_repair.start(vectorIds), 0); // 需要用sudo权限，不然failed
}