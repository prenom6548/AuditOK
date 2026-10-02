/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

 #include "KMRemoteModify.h"

 #include <gtest/gtest.h>
 
 #define private public
 
 class KMTestRemoteModify
 {
 public:
    KMTestRemoteModify() = default;
     ~KMTestRemoteModify() = default;
 
 public:
    KMRemoteModify m_remoteModify;
 };
 
 TEST(KMTestRemoteModify, test_remoteModify_depose) {
   KMTestRemoteModify testRemoteModify;
   std::string repoArg = "--repo=http://172.30.60.202/repo";
   repoArg.erase(std::remove_if(repoArg.begin(), repoArg.end(), ::isspace), repoArg.end()); // 去除空白字符
   const char *argv[] = {"remote", "kaiming-repo", repoArg.c_str()};
   EXPECT_EQ(testRemoteModify.m_remoteModify.dispose(3, const_cast<char **>(argv)), 0);
}
 