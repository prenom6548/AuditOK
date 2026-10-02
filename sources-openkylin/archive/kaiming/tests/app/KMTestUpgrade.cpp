/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "app/upgrade/KMUpgrade.h"
 
 class KMTestUpgrade
 {
 public:
    KMTestUpgrade() = default;
     ~KMTestUpgrade() = default;
 
 public:
    KMUpgrade m_upgrade;
 };
 
 TEST(KMTestUpgrade, test_upgrade_depose)
 {
     KMTestUpgrade testUpgrade;
     const char *argv[] = {"upgrade","-y"};
     EXPECT_EQ(testUpgrade.m_upgrade.dispose(2, const_cast<char **>(argv)), 0);
 }
