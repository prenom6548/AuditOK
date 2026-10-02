/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

 #include "KMWhich.h"

 #include <gtest/gtest.h>
 
 #define private public
 
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
     const char *argv[] = {"which","/opt/kaiming/layers/stable/x86_64/app/top.openkylin.Recorder"};
     EXPECT_EQ(testWhich.m_which.dispose(2, const_cast<char **>(argv)), 0);
 }

 TEST(KMTestWhich, test_which_normalizePath)
 {
     KMTestWhich testWhich;
     EXPECT_EQ(testWhich.m_which.normalizePath("/opt/kaiming/layers/stable/x86_64/app/top.openkylin.Recorder"), "/opt/kaiming/layers/stable/x86_64/app/top.openkylin.Recorder");
 }
 