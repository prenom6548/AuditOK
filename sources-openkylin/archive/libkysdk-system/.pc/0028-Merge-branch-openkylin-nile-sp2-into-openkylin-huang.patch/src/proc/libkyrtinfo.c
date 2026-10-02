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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyrtinfo.h"
#include <cstring-extension.h>
#include <stdio.h>
#include <unistd.h>
#include <libkylog.h>
#include <dbus-1.0/dbus/dbus.h>

#ifdef __linux__
#define MEMINFO_FILE    "/proc/meminfo"
#endif

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

static unsigned long lookup(char* buffer, const char* key, unsigned long *res)
{
    if (strstartswith(buffer, key) == 0)
    {
        char *val = buffer;
        val += strnlen(key, STRNLEN_MAX) + 1;
        *res = atol(strskipblank(val));
        return 1;
    }
    return 0;
}

unsigned long _kdk_rti_get_mem_res_total_KiB()
{
    unsigned long memTotal = 0;
    FILE *fp = NULL;
    char buffer[256];
    char *line = NULL;
    size_t len = 0;

    fp = popen("dmidecode -t memory | grep Size", "r");
    if (fp == NULL) {
        klog_err("popen failed");
        return 0.0f;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *p = buffer;
        while (*p && isspace((unsigned char)*p)) p++;
        
        if (strncmp(p, "Size:", 5) == 0) {
            char *value = p + 5;
            while (*value && isspace((unsigned char)*value)) value++;
            
            char *num_end;
            float size = strtof(value, &num_end);
            if (value != num_end) {
                while (*num_end && isspace((unsigned char)*num_end)) num_end++;
                
                if (strncmp(num_end, "MB", 2) == 0) {
                    memTotal += (unsigned long)(size * 1024);
                } else if (strncmp(num_end, "GB", 2) == 0) {
                    memTotal += (unsigned long)(size * 1024 * 1024);
                } else if (strncmp(num_end, "KB", 2) == 0) {
                    memTotal = size;
                }
            }
        }
    }
    
    pclose(fp);
    
    return memTotal;
}

unsigned long kdk_rti_get_mem_res_total_KiB()
{
    unsigned long memTotal = 0;
    DBusConnection *conn;
    DBusError err;
    char *serial = NULL;
    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }
    if (dbus_error_is_set(&err))
    {
        klog_err(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/resource", // object to call on
                                            "com.kylin.kysdk.resource",  // interface to call on
                                            "getMemTotalKiB");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }
    
    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }
    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto out;
    }
    dbus_connection_flush(conn);
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }
    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }
    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }
    DBusMessageIter args;
    double ret = 0;
    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }
    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    memTotal = (unsigned long)ret;
    
    return memTotal;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    memTotal = _kdk_rti_get_mem_res_total_KiB();
    
    return memTotal;
}

unsigned long kdk_rti_get_mem_res_available_KiB()
{
    unsigned long memAvailable = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];

    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "MemAvailable", &memAvailable))
            break;
    }

    fclose(fp);
#endif

    return memAvailable;
}

float kdk_rti_get_mem_res_usage_percent()
{
    float percent = 0.00;
#ifdef  __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0.00;
    
    unsigned long memTotal = 0, memAvailable = 0;
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "MemTotal", &memTotal));
        else if (lookup(buffer, "MemAvailable", &memAvailable));

        if (memTotal && memAvailable)
            break;
    }
    fclose(fp);
    if (memTotal)
        percent = (float)(memTotal - memAvailable) / (float)memTotal;
#endif

    return percent;
}

unsigned long kdk_rti_get_mem_res_usage_KiB()
{
    unsigned long memResUsage = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    unsigned long memTotal = 0, memAvailable = 0;
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "MemTotal", &memTotal));
        else if (lookup(buffer, "MemAvailable", &memAvailable));

        if (memTotal && memAvailable)
            break;
    }
    fclose(fp);
    memResUsage = memTotal - memAvailable;
#endif

    return memResUsage;
}

unsigned long kdk_rti_get_mem_res_free_KiB()
{
    unsigned long memFree = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "MemFree", &memFree))
            break;
    }
    fclose(fp);
#endif

    return memFree;
}

unsigned long kdk_rti_get_mem_virt_alloc_KiB()
{
    unsigned long virtTotal = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "VmallocTotal", &virtTotal))
            break;
    }
    fclose(fp);
#endif

    return virtTotal;
}

unsigned long kdk_rti_get_mem_swap_total_KiB()
{
    unsigned long swapTotal = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "SwapTotal", &swapTotal))
            break;
    }
    fclose(fp);
#endif

    return swapTotal;
}

unsigned long kdk_rti_get_mem_swap_usage_KiB()
{
    unsigned long swapUsage = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    unsigned long swapTotal = 0, swapFree = 0;
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "SwapTotal", &swapTotal));
        else if (lookup(buffer, "SwapFree", &swapFree));

        if (swapTotal && swapFree)
            break;
    }
    fclose(fp);

    swapUsage = swapTotal - swapFree;
#endif
    return swapUsage;
}

float kdk_rti_get_mem_swap_usage_percent()
{
    float percent = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    unsigned long swapTotal = 0, swapFree = 0;
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "SwapTotal", &swapTotal));
        else if (lookup(buffer, "SwapFree", &swapFree));

        if (swapTotal && swapFree)
            break;
    }
    fclose(fp);

    if (swapTotal)
        percent = (float)(swapTotal - swapFree) / (float)swapTotal;
#endif
    return percent;
}

unsigned long kdk_rti_get_mem_swap_free_KiB()
{
    unsigned long swapFree = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "SwapFree", &swapFree))
            break;
    }
    fclose(fp);
#endif

    return swapFree;
}

static int _get_cpu_data(unsigned long *user, unsigned long *sys, unsigned long *idle, unsigned long *iowait)
{
#ifdef __linux__
    FILE *fp = fopen("/proc/stat", "rt");
    if (!fp)
        return 1;
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (strstartswith(buffer, "cpu") == 0)
        {
            // char **list = strsplit(buffer, ' ');
            // if (!list)
            // {
            //     fclose(fp);
            //     return 1;
            // }
            char list1[32] = {0};
            char list3[32] = {0};
            char list4[32] = {0};
            char list5[32] = {0};
            sscanf(buffer, "%*s %s %*s %s %s %s %*s", list1, list3, list4, list5);
            klog_info("list1 = %s, list3 = %s, list4 = %s, list5 = %s\n", list1, list3, list4, list5);
            if(strnlen(list1, STRNLEN_MAX) == 0)
            {
                return 1;
            }
            *user = atol(list1);

            if(strnlen(list3, STRNLEN_MAX) == 0)
            {
                return 1;
            }
            *sys = atol(list3);

            if(strnlen(list4, STRNLEN_MAX) == 0)
            {
                return 1;
            }
            *idle = atol(list4);

            if(strnlen(list5, STRNLEN_MAX) == 0)
            {
                return 1;
            }
            *iowait = atol(list5);
            // free(list);
            break;
        }
        else
            break;
    }

    fclose(fp);
#endif
    return 0;
}

float kdk_rti_get_cpu_current_usage()
{
    float usage = 0.00;
#ifdef __linux__
    unsigned long user_1, user_2;
    unsigned long sys_1, sys_2;
    unsigned long idle_1, idle_2;
    unsigned long iowait_1, iowait_2;
    if (_get_cpu_data(&user_1, &sys_1, &idle_1, &iowait_1))
        return 0.00;
    usleep(100 * 1000);
    if (_get_cpu_data(&user_2, &sys_2, &idle_2, &iowait_2))
        return 0.00;
    
    usage = (float)(user_2 - user_1 + sys_2 - sys_1 + iowait_2 - iowait_1)/ \
            (float)(user_2 - user_1 + sys_2 - sys_1 + idle_2 - idle_1 + iowait_2 - iowait_1);
#endif
    return usage;
}

// int kdk_rti_test_load_isok()
// {

// }

int kdk_rti_get_uptime(unsigned int *day, unsigned int *hour, unsigned int *min, unsigned int *sec)
{
    if (!day || !hour || !min || !sec)
        return -1;
#ifdef __linux__
    FILE *fp = fopen("/proc/uptime", "rt");
    if (!fp)
        return -1;
    
    char buffer[513];
    unsigned long upsecs = 0;
    if (fgets(buffer, 512, fp))
    {
        upsecs = atol(buffer);
    }
    fclose(fp);
    if (upsecs)
    {
        *sec = upsecs % 60;
        *min = (upsecs / 60) % 60;
        *hour = (upsecs / 60 / 60) % 24;
        *day = upsecs / 60 / 60 / 24;
    }
#endif
    return 0;
}

unsigned long kdk_rti_get_mem_shared_KiB()
{
    unsigned long shmem = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Shmem", &shmem))
            break;
    }
    fclose(fp);
#endif

    return shmem;
}

unsigned long kdk_rti_get_mem_cached_KiB()
{
    unsigned long cached = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Cached", &cached))
            break;
    }
    fclose(fp);
#endif

    return cached;
}

unsigned long kdk_rti_get_mem_buffers_KiB()
{
    unsigned long buffers = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Buffers", &buffers))
            break;
    }
    fclose(fp);
#endif

    return buffers;
}

unsigned long kdk_rti_get_mem_swap_cached_KiB()
{
    unsigned long swapCached = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "SwapCached", &swapCached))
            break;
    }
    fclose(fp);
#endif

    return swapCached;
}

unsigned long kdk_rti_get_mem_active_KiB()
{
    unsigned long active = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Active", &active))
            break;
    }
    fclose(fp);
#endif

    return active;
}

unsigned long kdk_rti_get_mem_inactive_KiB()
{
    unsigned long inactive = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Inactive", &inactive))
            break;
    }
    fclose(fp);
#endif

    return inactive;
}

unsigned long kdk_rti_get_mem_dirty_KiB()
{
    unsigned long dirty = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Dirty", &dirty))
            break;
    }
    fclose(fp);
#endif

    return dirty;
}

unsigned long kdk_rti_get_mem_map_KiB()
{
    unsigned long map = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Mapped", &map))
            break;
    }
    fclose(fp);
#endif

    return map;
}

unsigned long kdk_rti_get_mem_slab_KiB()
{
    unsigned long slab = 0;
#ifdef __linux__
    FILE *fp = fopen(MEMINFO_FILE, "rt");
    if (!fp)
        return 0;
    
    char buffer[1025];
    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "Slab", &slab))
            break;
    }
    fclose(fp);
#endif

    return slab;
}
