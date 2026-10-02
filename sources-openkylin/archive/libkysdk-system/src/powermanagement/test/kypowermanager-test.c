/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
#include "../libkypowermanager.h"

int main()
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

    // test 7
    bool supports_hibernate = kdk_power_is_support_hibernate();
    if (supports_hibernate) {
        printf("系统支持休眠功能\n");
    } else {
        printf("系统不支持休眠功能\n");
    }

    // // test 8
    bool supports_suspend = kdk_power_is_support_suspend();
    if (supports_suspend) {
        printf("系统支持挂起功能\n");
    } else {
        printf("系统不支持挂起功能\n");
    }

    // test 9
    // int ret9 = kdk_power_set_hibernate();
    
    // if (ret9 == 0) {
    //     printf("Hibernate command sent successfully\n");
    // } else {
    //     printf("Failed to send hibernate command\n");
    // }

    // test 10
    int ret10 = kdk_power_set_suspend();

    if (ret10 == 0) {
        printf("Suspend command sent successfully\n");
    } else {
        printf("Failed to send suspend command\n");
    }
    
    return 0;
}
