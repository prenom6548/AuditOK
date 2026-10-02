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

#include "libkypnp.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "libkylog.h"
#include "libudev.h"
#include "pthread.h"
#include "signal.h"

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

static KdkPnpCallback connect_callback = NULL;
static KdkPnpCallback disconnect_callback = NULL;
static pthread_t monitor_thread = 0;

static int _str_starts_with(char *s1, char *s2)
{
    return strncmp(s1, s2, strnlen(s2, STRNLEN_MAX));
}

static int check_thread_running()
{
    if (0 == monitor_thread)
        return 0;

    int res = pthread_kill(monitor_thread, 0);
    if (res == 0)
    {
        klog_info("Thread is already running\n");
        return 1;
    }
    else if (res == ESRCH)
    {
        klog_info("Thread is not running\n");
        return 0;
    }
    else
    {
        klog_info("Error checking thread status\n");
        return -1;
    }
}

static void *monitorThread(void *arg)
{
    struct udev *udev = (struct udev *)arg;
    struct udev_monitor *monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "pnp", NULL);
    // udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", NULL);
    udev_monitor_enable_receiving(monitor);

    while (1)
    {
        if ((NULL == connect_callback) && (NULL == disconnect_callback))
            break;

        struct udev_device *device = udev_monitor_receive_device(monitor);
        if (device)
        {
            char *action = udev_device_get_action(device);
            char *sys_path = udev_device_get_syspath(device);
            if ((0 == strcmp(action, "add")) && connect_callback)
            {
                connect_callback(action, sys_path);
            }
            if ((0 == strcmp(action, "remove")) && disconnect_callback)
            {
                disconnect_callback(action, sys_path);
            }
            udev_device_unref(device);
        }
    }
    udev_monitor_unref(monitor);
    return NULL;
}

kdk_pnp_info *kdk_pnp_get_info()
{
    kdk_pnp_info *result = NULL;

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;
    struct udev_device *device;

    // 创建一个udev实例
    udev = udev_new();
    if (!udev)
    {
        fprintf(stderr, "Failed to create udev context\n");
        return NULL;
    }

    // 创建一个udev枚举器并添加过滤条件
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "pnp");
    udev_enumerate_scan_devices(enumerate);

    // 获取设备列表
    devices = udev_enumerate_get_list_entry(enumerate);

    // 遍历PnP设备并输出关键信息
    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);
        device = udev_device_new_from_syspath(udev, path);

        char *sysname = strdup(udev_device_get_sysname(device));
        char *sysnum = strdup(udev_device_get_sysnum(device));
        char *id = strdup(udev_device_get_sysattr_value(device, "id"));
        char *resources = udev_device_get_sysattr_value(device, "resources");
        char *state = NULL, mem[1024];
        memset(mem, 0, 1024);
        char *token = strtok(resources, "\n");
        while (NULL != token)
        {
            if (0 == _str_starts_with(token, "state"))
            {
                char *tmp = strchr(token, '=');
                if (NULL != tmp)
                {
                    state = strdup(tmp + 2);
                }
            }

            if (0 == _str_starts_with(token, "mem"))
            {
                strcat(mem, "|");
                strcat(mem, token + 4);
            }
            token = strtok(NULL, "\n");
        }
        char *hid = strdup(udev_device_get_sysattr_value(device, "firmware_node/hid"));
        char *control = strdup(udev_device_get_sysattr_value(device, "power/control"));

        kdk_pnp_info *info = calloc(1, sizeof(kdk_pnp_info));
        if (NULL == info)
        {
            kdk_pnp_free_info(result);
            result = NULL;
            klog_err("Memory error\n");
            goto out;
        }
        info->next = result;
        result = info;

        info->sysname = sysname;
        info->sysnum = sysnum;
        info->id = id;
        info->mem = strdup(mem);
        info->hid = hid;
        info->control = control;

        udev_device_unref(device);
    }

out:
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return result;
}

void kdk_pnp_free_info(kdk_pnp_info *info)
{
    for (kdk_pnp_info *tmp = info; tmp; tmp = info)
    {
#define PNP_SAFE_FREE(x) \
    if ((x))             \
        free((x));
        PNP_SAFE_FREE(info->sysname)
        PNP_SAFE_FREE(info->sysnum)
        PNP_SAFE_FREE(info->id)
        PNP_SAFE_FREE(info->mem)
        PNP_SAFE_FREE(info->hid)
        PNP_SAFE_FREE(info->control)
#undef PNP_SAFE_FREE
        info = tmp->next;
        free(tmp);
    }
}

int kdk_pnp_register_switch_connect_handle(KdkPnpCallback call_back, void *user_data)
{
    if (1 != check_thread_running())
    {
        int r = pthread_create(&monitor_thread, NULL, monitorThread, NULL);
        if (0 != r)
        {
            klog_err("Creat monitor thread fail\n");
            return r;
        }
    }
    connect_callback = call_back;
    return 0;
}

int kdk_pnp_unregister_switch_connect_handle()
{
    connect_callback = NULL;
    return 0;
}

int kdk_pnp_register_switch_disconnect_handle(KdkPnpCallback call_back, void *user_data)
{
    if (1 != check_thread_running())
    {
        int r = pthread_create(&monitor_thread, NULL, monitorThread, NULL);
        if (0 != r)
        {
            klog_err("Creat monitor thread fail\n");
            return r;
        }
    }
    disconnect_callback = call_back;
    return 0;
}

int kdk_pnp_unregister_switch_disconnect_handle()
{
    disconnect_callback = NULL;
    return 0;
}
