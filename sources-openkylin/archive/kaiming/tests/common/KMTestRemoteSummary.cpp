/*
 * Copyright (c) KylinSoft Co., Ltd. 2024. All rights reserved.
 *
 * Test cases for KMRemoteSummary class
 */

 #include <gtest/gtest.h>
 #include <string>
 #include <vector>
 
 #define private public

#include "common/KMRemoteSummary.h"
 
 class KMTestRemoteSummary
 {
 public:
     KMTestRemoteSummary() = default;
     ~KMTestRemoteSummary() = default;
 
 public:
     KMRemoteSummary m_summary;
 };
 
 TEST(KMTestRemoteSummary, test_curlCmdDownload)
 {
     KMTestRemoteSummary testRemoteSummary;
 
     // 测试 curlCmdDownload 基本功能
     EXPECT_TRUE(KMRemoteSummary::curlCmdDownload());

 }