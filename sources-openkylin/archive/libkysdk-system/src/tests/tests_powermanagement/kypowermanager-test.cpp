/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "stdio.h"
#include "../../powermanagement/libkypowermanager.h"
#include "../../powermanagement/libkylockscreen.h"
#include <gtest/gtest.h>

TEST(KdkPowerManagementTest, PowerManagementInfo)
{
    int timeout = kdk_power_get_screenidle_timeout();
    printf("timeout = %d\n", timeout);

    int active = kdk_power_is_active();
    printf("active = %d\n", active);

    int mode = kdk_power_get_mode();
    printf("mode = %d\n", mode);

    char *hibernate = kdk_power_is_hibernate();
    printf("hibernate = %s\n", hibernate);

    char* status = kdk_power_get_control_disk_status();
    printf("status = %s\n", status);

    char appName[] = "test-package";
    char reason[] = "test-reason";

    uint32_t flag;
    if ((flag = kdk_set_inhibit_lockscreen(appName , reason)) == 0) {
        printf("set inhibit lock screen fail !\n");
    }

    if (kdk_un_inhibit_lockscreen(flag)) {
        printf("un inhibit lock screen fail !\n");
    }
}
