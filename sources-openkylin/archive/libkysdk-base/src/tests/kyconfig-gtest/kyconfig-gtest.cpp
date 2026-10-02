/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include "../cpp_stub/src/stub.h"
#include "../../config/libkyconf.h"

static int id = 0;

TEST(TestSuite1, kdk_conf_init)
{
    char **keylist = NULL;
    id = kdk_conf_init("../src/tests/kyconfig-gtest/struct.conf");
    EXPECT_GT(id, 0);
}

TEST(TestSuite2, kdk_conf_list_group)
{
    char **grouplist = kdk_conf_list_group(id);
    EXPECT_FALSE(grouplist == NULL);
    kdk_config_freeall(grouplist);
}

TEST(TestSuite3, kdk_conf_list_key)
{
    char **keylist = NULL;
    keylist = kdk_conf_list_key(id, "System");
    // EXPECT_FALSE(keylist == NULL);
    kdk_config_freeall(keylist);
}

TEST(TestSuite4, kdk_conf_get_value)
{
    const char *tmpval = kdk_conf_get_value(id, "Testmode", "key1");
    EXPECT_FALSE(tmpval == NULL);
}

TEST(TestSuite5, kdk_conf_reload)
{
    // kdk_conf_reload(id);
}

TEST(TestSuite6, kdk_conf_enable_autoreload)
{
    kdk_conf_enable_autoreload(id);
}

TEST(TestSuite7, kdk_conf_disable_autoreload)
{
    kdk_conf_disable_autoreload(id);
}

TEST(TestSuite8, kdk_conf_destroy)
{
    kdk_conf_destroy(id);
}