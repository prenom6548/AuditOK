/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMInstall.h"

#include <gtest/gtest.h>

#define private public

class KMTestInstall
{
public:
    KMTestInstall() = default;
    ~KMTestInstall() = default;

public:
    KMInstall m_install;
};

TEST(KMTestInstall, test_install_depose)
{
    KMTestInstall testInstall;
    const char *argv[] = {"install","-y", "com.daidouji.oneko"};
    EXPECT_EQ(testInstall.m_install.dispose(3, const_cast<char **>(argv)), 0);
}
