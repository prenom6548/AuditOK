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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkydevicemanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libudev.h>
#include "libkylog.h"

char **kdk_dm_get_device_list()
{
    char **result = NULL;
    int count = 0, max_count = 0;

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;

    // 创建一个udev实例
    udev = udev_new();
    if (!udev)
    {
        klog_err("Failed to create udev context\n");
        return result;
    }

    // 创建一个udev枚举器并添加过滤条件
    enumerate = udev_enumerate_new(udev);
    // udev_enumerate_add_match_subsystem(enumerate, "device");
    udev_enumerate_scan_devices(enumerate);

    // 获取设备列表
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);

        if (count == max_count)
        {
            max_count += 50;
            char **tmp = realloc(result, sizeof(char *) * (max_count + 1));
            if (NULL == tmp)
            {
                for (int i = 0; tmp[i]; i++)
                {
                    free(tmp[i]);
                }
                free(tmp);
                result = NULL;
                goto out;
            }
            result = tmp;
            memset(result + count, 0, sizeof(char *) * 51);
        }
        result[count++] = strdup(path);
    }

out:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return result;
}

int kdk_dm_check_device_available(char *devicename)
{
    if (devicename)
        return access(devicename, F_OK);
    return -1;
}

kdk_device_number *kdk_dm_get_device_number(char *devicepath)
{
    if (NULL == devicepath)
        return NULL;
    kdk_device_number *result = NULL;

    struct udev *udev;
    struct udev_device *device;
    udev = udev_new();
    if (!udev)
    {
        klog_err("Failed to create udev context\n");
        return result;
    }
    device = udev_device_new_from_syspath(udev, devicepath);
    if (NULL == device)
    {
        goto out;
    }
    result = calloc(1, sizeof *result);
    if (NULL == result)
    {
        udev_device_unref(device);
        goto out;
    }
    result->device_syspath = strdup(udev_device_get_syspath(device) ? udev_device_get_syspath(device) : "None");
    result->device_subsystem = strdup(udev_device_get_subsystem(device) ? udev_device_get_subsystem(device) : "None");
    result->device_devtype = strdup(udev_device_get_devtype(device) ? udev_device_get_devtype(device) : "None");
    result->device_sysname = strdup(udev_device_get_sysname(device) ? udev_device_get_sysname(device) : "None");
    result->device_sysnum = udev_device_get_sysnum(device);
    udev_device_unref(device);

out:
    udev_unref(udev);
    return result;
}

int kdk_dm_get_device_hotplug_info(char *devicepath)
{
    if (NULL == devicepath)
        return 0;

    char *result = NULL;
    struct udev *udev;
    struct udev_device *device;
    udev = udev_new();
    if (!udev)
    {
        klog_err("Failed to create udev context\n");
        return 0;
    }
    device = udev_device_new_from_syspath(udev, devicepath);
    if (NULL == device)
    {
        return 0;
    }
    result = udev_device_get_property_value(device, "HOTPLUG");
    printf("%s\n", udev_device_get_syspath(device));
    udev_device_unref(device);

    udev_unref(udev);
    return result ? 1 : -1;
}

const char *type2subsytem[] = {
    "unknown",
    "block", // 硬盘
    "block", // 分区
    "net",
    "usb",
    "tty",
    "scsi",
    "input",
    "pci",
    "sound",
    "drm",
    "i2c",
    "pci",
    "power_supply",
    "bluetooth",
    "hid",
    "platform",
    "thermal",
    "acpi",
    "misc"};

char **kdk_dm_get_device_list_by_type(kdk_device_type type)
{
    char **result = NULL;
    int count = 0, max_count = 0;

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;
    struct udev_device *device;

    // 创建一个udev实例
    udev = udev_new();
    if (!udev)
    {
        klog_err("Failed to create udev context\n");
        return result;
    }

    // 创建一个udev枚举器并添加过滤条件
    enumerate = udev_enumerate_new(udev);
    // udev_enumerate_add_match_subsystem(enumerate, "device");
    udev_enumerate_scan_devices(enumerate);

    // 获取设备列表
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);

        device = udev_device_new_from_syspath(udev, path);
        if (NULL == device)
        {
            goto out;
        }

        char *subsystem = udev_device_get_subsystem(device);
        if (0 == strcmp(subsystem, type2subsytem[type]))
        {
            if ((type == DEVICE_DEVTYPE_DISK) || (type == DEVICE_DEVTYPE_PARTITION))
            {
                kdk_device_type t;
                char *dev_type = udev_device_get_devtype(device);
                if (NULL == dev_type)
                    continue;
                else if (0 == strcmp(dev_type, "disk"))
                    t = DEVICE_DEVTYPE_DISK;
                else if (0 == strcmp(dev_type, "partition"))
                    t = DEVICE_DEVTYPE_PARTITION;
                if (t != type)
                    continue;
            }

            if (count == max_count)
            {
                max_count += 50;
                char **tmp = realloc(result, sizeof(char *) * (max_count + 1));
                if (NULL == tmp)
                {
                    for (int i = 0; tmp[i]; i++)
                    {
                        free(tmp[i]);
                    }
                    free(tmp);
                    result = NULL;
                    goto out;
                }
                result = tmp;
                memset(result + count, 0, sizeof(char *) * 51);
            }

            result[count++] = strdup(path);
        }

        udev_device_unref(device);
    }

out:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return result;
}

char **kdk_dm_get_device_list_by_vendor(char *vendor)
{
    if (NULL == vendor)
        return NULL;
    char **result = NULL;
    int count = 0, max_count = 0;

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;
    struct udev_device *device;

    // 创建一个udev实例
    udev = udev_new();
    if (!udev)
    {
        klog_err("Failed to create udev context\n");
        return result;
    }

    // 创建一个udev枚举器并添加过滤条件
    enumerate = udev_enumerate_new(udev);
    // udev_enumerate_add_match_subsystem(enumerate, "device");
    udev_enumerate_scan_devices(enumerate);

    // 获取设备列表
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);

        device = udev_device_new_from_syspath(udev, path);
        if (NULL == device)
        {
            goto out;
        }

        char *id_vendor = udev_device_get_property_value(device, "ID_VENDOR");
        if (NULL == id_vendor)
            continue;

        if (0 == strcmp(id_vendor, vendor))
        {
            if (count == max_count)
            {
                max_count += 50;
                char **tmp = realloc(result, sizeof(char *) * (max_count + 1));
                if (NULL == tmp)
                {
                    for (int i = 0; tmp[i]; i++)
                    {
                        free(tmp[i]);
                    }
                    free(tmp);
                    result = NULL;
                    goto out;
                }
                result = tmp;
                memset(result + count, 0, sizeof(char *) * 51);
            }

            result[count++] = strdup(path);
        }

        udev_device_unref(device);
    }

out:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return result;
}

void kdk_dm_refresh_device_list()
{
    system("udevadm trigger");
}
