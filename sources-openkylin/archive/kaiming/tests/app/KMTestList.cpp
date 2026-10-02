/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/list/KMList.h"

class KMTestList
{
public:
    KMTestList() = default;
    ~KMTestList() = default;

public:
    KMList m_list;
};

TEST(KMTestList, test_list)
{
    KMTestList testList;
    const char* args[]={"list", "--kind=base"};
    EXPECT_EQ(testList.m_list.dispose(1,const_cast<char**>(args)), 0);
    vector<string> vecList;
    EXPECT_EQ(testList.m_list.start(vecList), 0);
}
