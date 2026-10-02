/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMSearch.h"
#include "KMError.h"

#include <gtest/gtest.h>

#define private public

class KMTestSearch
{
public:
    KMTestSearch() = default;
    ~KMTestSearch() = default;

public:
    KMSearch m_search;
};

TEST(KMTestSearch, test_search_depose)
{
    KMTestSearch testSearch;
    const char *argv[] = {"search", ""};
    EXPECT_EQ(testSearch.m_search.dispose(2, const_cast<char **>(argv)), 0);
}

TEST(KMTestSearch, test_search_dbsStart)
{
    KMTestSearch testSearch;
    vector<string> vecSearch;  
    vecSearch.push_back("");
    EXPECT_EQ(testSearch.m_search.dbusStart(vecSearch), 0);
}

TEST(KMTestSearch, test_search_getRemoteInfo)
{
    KMTestSearch testSearch;
    vector<string> vecSearch;  
    int errorCode = 0;
    std::string strRemoteInfoJson;
    EXPECT_EQ(testSearch.m_search.getRemoteInfo("top.openkylin.base",strRemoteInfoJson), KM_ERROR_NO);
}
