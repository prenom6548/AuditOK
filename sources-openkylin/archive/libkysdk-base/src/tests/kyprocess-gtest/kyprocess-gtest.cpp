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
#include "../../process/processdaemon.h"

int quit;

void func(void* data)
{
    printf("dog fake.\n");
    quit = 1;
    KYSDK_BASE::stopProcDaemon();
}

TEST(TestSuite1, kyprocess)
{
    KYSDK_BASE::startProcDaemon(5, func, NULL);
    int counts = 0;
    srand(time(NULL));
    while (!quit)
    {
        int stime = rand() % 7;
        printf("sleep %d\n", stime);
        sleep(stime);
        KYSDK_BASE::feedDog();
    }

    printf("quit.\n");
    KYSDK_BASE::stopProcDaemon();
}