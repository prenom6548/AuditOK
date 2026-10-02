/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMRepair.h"

#include <gtest/gtest.h>

#define private public

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
    const char* args[]={"repair","org.gnome.baobab"};
    EXPECT_GE(testRepair.m_repair.dispose(2,const_cast<char**>(args)), 0);
    // const vector<string> &vectorIds = {"org.gnome.baobab"};
    // EXPECT_EQ(testRepair.m_repair.start(vectorIds), 0); // 需要用sudo权限，不然failed
}
