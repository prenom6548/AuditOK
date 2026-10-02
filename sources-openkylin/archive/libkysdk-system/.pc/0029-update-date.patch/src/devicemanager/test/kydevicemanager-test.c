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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include <stdio.h>
#include "../libkydevicemanager.h"

int main(int argc, char *argv[])
{
    char ** paths = NULL;
    paths = kdk_dm_get_device_list();
    if (paths)
    {
        for (int i = 0; paths[i]; i++)
        {
            printf("%s\n", paths[i]);
            free(paths[i]);
        }
        free(paths);
    }

    printf("/dev/sda %s\n", kdk_dm_check_device_available("/dev/sda") == 0 ? "available" : "unavailable");

    kdk_device_number *info = kdk_dm_get_device_number("/sys/devices/cpu");
    if (info)
    {
        printf("/sys/devices/cpu :\n");
        printf("\tdevice_subsystem: %s\n", info->device_subsystem);
        printf("\tdevice_devtype: %s\n", info->device_devtype);
        printf("\tdevice_syspath: %s\n", info->device_syspath);
        printf("\tdevice_sysname: %s\n", info->device_sysname);
        printf("\tdevice_sysnum: %d\n", info->device_sysnum);
    }

    printf("hotplug: %d\n", kdk_dm_get_device_hotplug_info("/dev/vcsu6"));

    paths = kdk_dm_get_device_list_by_type(DEVICE_DEVTYPE_PARTITION);
    if (paths)
    {
        for (int i = 0; paths[i]; i++)
        {
            printf("%s\n", paths[i]);
            free(paths[i]);
        }
        free(paths);
    }

    paths = kdk_dm_get_device_list_by_vendor("SINO_WEALTH");
    if (paths)
    {
        for (int i = 0; paths[i]; i++)
        {
            printf("%s\n", paths[i]);
            free(paths[i]);
        }
        free(paths);
    }

    kdk_dm_refresh_device_list();
}
