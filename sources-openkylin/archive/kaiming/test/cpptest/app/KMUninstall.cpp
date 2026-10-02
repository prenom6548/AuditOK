/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMUninstall.h"

#include <gtest/gtest.h>

#define private public

class KMTestUninstall
{
public:
    KMTestUninstall() = default;
    ~KMTestUninstall() = default;

public:
    KMUninstall m_uninstall;
};

TEST(KMTestUninstall, test_uninstall_depose)
{
    KMTestUninstall testUninstall;
    const char *argv[] = {"uninstall", "-y","com.daidouji.oneko"};
    EXPECT_EQ(testUninstall.m_uninstall.dispose(3, const_cast<char **>(argv)), 0);
}



