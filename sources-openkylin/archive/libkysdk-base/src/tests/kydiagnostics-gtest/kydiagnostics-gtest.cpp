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
#include "../../diagnostics/libkydiagnostics.h"

TEST(TestSuite1, kydiagnostics)
{
    char appName[] = "kylin-font-viewer";
    char messageType[] = "FunctionType";

    KBuriedPoint pt[2];
    pt[0].key = "testkey";
    pt[0].value = "testvalue";
    pt[1].key = "testkey1";
    pt[1].value = "testvalue1";

    if (kdk_buried_point(appName, messageType, pt, 2))
    {
        printf("buried point fail !\n");
        return;
    }

    printf("buried point success !\n");
}