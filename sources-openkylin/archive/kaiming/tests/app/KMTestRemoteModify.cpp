/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
 
#include "app/remoteModify/KMRemoteModify.h"
 
class KMTestRemoteModify
{
public:
   KMTestRemoteModify() = default;
    ~KMTestRemoteModify() = default;
 
public:
   KMRemoteModify m_remoteModify;
};
 
TEST(KMTestRemoteModify, test_remoteModify_addRemote)
{
   KMTestRemoteModify testRemoteModify;
   const char *argv[] = {"remote", "add", "test-repo", "--repo=http://172.30.60.201/repo"};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(4, const_cast<char **>(argv)), 0);
}

TEST(KMTestRemoteModify, test_remoteModify_modifyRemote)
{
   KMTestRemoteModify testRemoteModify;
   const char *argv[] = {"remote", "modify", "test-repo", "--repo=http://172.30.60.202/repo"};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(4, const_cast<char **>(argv)), 0);
}

TEST(KMTestRemoteModify, test_remoteModify_deleteRemote)
{
   KMTestRemoteModify testRemoteModify;
   const char *argv[] = {"remote", "remove", "test-repo"};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(3, const_cast<char **>(argv)), 0);
}

TEST(KMTestRemoteModify, test_remoteModify_listRemote)
{
   KMTestRemoteModify testRemoteModify;
   const char *argv[] = {"remote", "list"};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(2, const_cast<char **>(argv)), 0);
}

TEST(KMTestRemoteModify, test_remoteModify_showRemote)
{
   KMTestRemoteModify testRemoteModify;
   const char *argv[] = {"remote", "show"};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(2, const_cast<char **>(argv)), 0);
}