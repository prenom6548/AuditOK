/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMUpdate.h"

#include <gtest/gtest.h>

#define private public

class KMTestUpdate
{
public:
    KMTestUpdate() = default;
    ~KMTestUpdate() = default;

public:
    KMUpdate m_update;
};

TEST(KMTestUpdate, test_update_depose)
{
    KMTestUpdate testUpdate;
    const char *argv[] = {"update", "org.gnome.Breakout"};
    EXPECT_GE(testUpdate.m_update.dispose(2, const_cast<char **>(argv)), 0);
}