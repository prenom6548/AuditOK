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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "libkyrealtimeinfo.h"
#include "fcntl.h"
#include "stdio.h"
#include "net/if.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "ifaddrs.h"
#include "math.h"
#include "sys/types.h"
#include <dbus-1.0/dbus/dbus.h>
#define _GNU_SOURCE
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <math.h>
#include <libkylog.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

struct if_data
{
    /*  generic interface information */
    unsigned int ifi_opackets; /*  packets sent on interface */
    unsigned int ifi_ipackets; /*  packets received on interface */
    unsigned int ifi_ibytes;   /*  total number of octets received */
    unsigned int ifi_obytes;   /*  total number of octets sent */
};

struct if_info
{
    char ifi_name[64];
    unsigned long ifi_ibytes;
    unsigned long ifi_obytes;
};
struct if_speed
{
    char ifs_name[64];
    unsigned long ifs_ispeed;
    unsigned long ifs_ospeed;
    unsigned long ifs_us;
};

static int get_if_dbytes(struct if_info *ndev)
{
    if (!ndev)
        return -1;

    struct ifaddrs *ifa_list = NULL;

    struct ifaddrs *ifa = NULL;

    struct if_data *ifd = NULL;

    int ret = 0;

    ret = getifaddrs(&ifa_list);

    if (ret < 0)
    {

        perror("Get Interface Address Fail:");

        goto end;
    }

    for (ifa = ifa_list; ifa; ifa = ifa->ifa_next)
    {

        if (!(ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_RUNNING))

            continue;

        if (ifa->ifa_data == 0)

            continue;

        ret = strcmp(ifa->ifa_name, ndev->ifi_name);

        if (ret == 0)
        {

            ifd = (struct if_data *)ifa->ifa_data;

            ndev->ifi_ibytes = ifd->ifi_ibytes;

            ndev->ifi_obytes = ifd->ifi_obytes;

            break;
        }
    }

    freeifaddrs(ifa_list);

end:
    return (ret ? -1 : 0);
}

static int get_if_speed(struct if_speed *ndev)
{
    if (!ndev)
        return -1;

    struct if_info *p1 = NULL, *p2 = NULL;

    p1 = (struct if_info *)malloc(sizeof(struct if_info));

    p2 = (struct if_info *)malloc(sizeof(struct if_info));

    bzero(p1, sizeof(struct if_info));

    bzero(p2, sizeof(struct if_info));

    strncpy(p1->ifi_name, ndev->ifs_name, sizeof(p1->ifi_name));

    strncpy(p2->ifi_name, ndev->ifs_name, sizeof(p2->ifi_name));

    int ret = 0;

    ret = get_if_dbytes(p1);

    if (ret < 0)
        goto end;

    usleep(ndev->ifs_us);

    ret = get_if_dbytes(p2);

    if (ret < 0)
        goto end;

        ndev->ifs_ispeed = p2->ifi_ibytes - p1->ifi_ibytes;

    ndev->ifs_ospeed = p2->ifi_obytes - p1->ifi_obytes;

end:

    free(p1);

    free(p2);

    return 0;
}

float kdk_real_get_net_speed(const char *nc)
{
    if(!nc)
        return -1;
    struct if_speed ndev;
    memset(&ndev, 0, sizeof(struct if_speed));

    int ret = 0;
    memset(&ndev, 0, sizeof(ndev));
    snprintf(ndev.ifs_name, sizeof(ndev.ifs_name), "%s", nc);
    ndev.ifs_us = 100000;
    ret = get_if_speed(&ndev);
    if (ret < 0)
        return -1;
    return ndev.ifs_ospeed * 1.0 / (ndev.ifs_us / 1000 * 0.001) / 1024.0f;
}

double kdk_real_get_cpu_temperature()
{
    char tmp[1024] = "\0";
    char path[64] = "\0";
    for (int i = 0;; i++)
    {
        memset(path, 0, 64);
        snprintf(path, 64, "/sys/class/hwmon/hwmon%d", i);
        if (0 != access(path, F_OK))
        {
            snprintf(path, 64, "/sys/class/hwmon/hwmon0/temp1_input");
            break;
        }
        snprintf(path, 64, "/sys/class/hwmon/hwmon%d/name", i);
        if (0 != access(path, F_OK))
            continue;
        snprintf(path, 64, "/sys/class/hwmon/hwmon%d/temp1_input", i);
        if (0 != access(path, F_OK))
            continue;
        break;
    }
    double temp = 0.0;
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        fgets(tmp, sizeof(tmp), fp);
        fclose(fp);
        temp = atof(tmp);
        return temp < 100 ? temp : temp / 1000;
    }
    if(fabs(temp - 0.0) < 0.000001) 
    {
        fp = popen("sensors","r");
        if(fp)
        {
            while (fgets(tmp, sizeof(tmp), fp))
            {
                if (0 == strncmp(tmp, "Core", 4))
                {
                    char *pos = tmp, *start = NULL;
                    while (*pos != '+' && *pos != '\0')
                    {
                        pos++;
                        continue;
                    }
                    start = pos++;
                    while (*pos != '\0')
                    {
                        if(*pos == 0xc2 && *(pos + 1) == 0xb0) //字符"°"
                            break;
                        pos++;
                    }
                    *pos = '\0';
                    temp = atof(start);
                    fclose(fp);
                    return temp;
                }
            }
            fclose(fp);
        }
    }
    return -1;
}

static int is_block_device(const char *path) {
    struct stat st;
    
    if (stat(path, &st) == -1) {
        return 0;  // 不是块设备或出错
    }
    
    return S_ISBLK(st.st_mode);
}

int _kdk_real_get_disk_temperature(const char *name)
{
    if(!name)
    {
        return -1;
    }

    int block = is_block_device(name);
    
    if((access(name, F_OK) == -1) || !block)
    {
        return -1;
    }

    char cmd[128] = {"smartctl -A "};
    char tmp[1024] = {"\0"};
    strncat(cmd, name, sizeof(cmd) - strnlen(cmd, STRNLEN_MAX) - 1);
    FILE *fp = popen(cmd,"r");
    if(fp)
    {
        int temp = 0;
        while (fgets(tmp, 1024, fp))
        {
            //sata接口返回数据格式
            if(0 == strncmp(tmp,"194",3))
            {
                sscanf(tmp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %d", &temp);
                break;
            }
            //nvme接口返回数据格式
            if (0 == strncmp(tmp, "Temperature:", 12))
            {
                sscanf(tmp, "%*s %d %*s", &temp);
                break;
            }
        }
        return temp;
    }
    return -1;
}

int kdk_real_get_disk_temperature(const char *name)
{
    double ret = 0.0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0.0;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getDiskTemperature");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = 0.0;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
    {
        ret = 0.0;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = 0.0;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = 0.0;
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
        ret = 0.0;
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        dbus_message_unref(replyMsg);
        ret = 0.0;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = 0.0;
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

out:

    if(ret == 0.0)
    {
        return _kdk_real_get_disk_temperature(name);
    }
    int temperature = (double)ret;
    return temperature;
}

int _kdk_real_get_disk_rate(const char *name)
{
    if(!name)
    {
        return -1;
    }

    int block = is_block_device(name);
    
    if((access(name, F_OK) == -1) || !block)
    {
        return -1;
    }

    char cmd[128] = {"smartctl -i "};
    char tmp[1024] = {"\0"};
    strncat(cmd, name, sizeof(cmd) - strnlen(cmd, STRNLEN_MAX) - 1);
    FILE *fp = popen(cmd, "r");
    if (fp)
    {
        int rate = 0;
        while (fgets(tmp, 1024, fp))
        {
            // sata接口返回数据格式
            if (0 == strncmp(tmp, "Rotation Rate:", 14))
            {
                sscanf(tmp, "%*s %*s %d %*s", &rate);
                break;
            }
        }
        return rate;
    }
    return -1;
}

int kdk_real_get_disk_rate(const char *name)
{
    double ret = 0.0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0.0;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getDiskRate");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = 0.0;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
    {
        ret = 0.0;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = 0.0;
    }

    if (sendMsgPending == NULL)
    {
        ret = 0.0;
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
        ret = 0.0;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = 0.0;
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

    if(ret == 0.0)
    {
        return _kdk_real_get_disk_rate(name);
    }
    int rate = (double)ret;
    return rate;
}

float kdk_real_get_if_speed(const char *nc)
{
    if (!nc)
        return -1;
    struct if_speed ndev;
    memset(&ndev, 0, sizeof(struct if_speed));

    int ret = 0;
    memset(&ndev, 0, sizeof(ndev));
    snprintf(ndev.ifs_name, sizeof(ndev.ifs_name), "%s", nc);
    ndev.ifs_us = 100000;
    ret = get_if_speed(&ndev);
    if (ret < 0)
        return -1;
    return ndev.ifs_ispeed * 1.0 / (ndev.ifs_us / 1000 * 0.001) / 1024.0f;
}

unsigned long long kdk_real_get_disk_read(const char *diskname) 
{
    if(!diskname || access(diskname, F_OK) == -1)
        return 0;
    char buffer[256] = "\0";
    unsigned long long sectors_read;
    
#ifdef __linux__
    FILE *fp;  
    fp = fopen("/proc/diskstats", "r");
    if (!fp)
    {
        return 0;
    }
    char disk[128] = "\0";
    sscanf(diskname, "/dev/%s", disk);
    // 逐行读取文件内容
    while (fgets(buffer, sizeof(buffer), fp)) 
    {
        char dev_name[32] = "\0";;
        unsigned long long read_sectors;
        // 解析行内容，提取设备名称和读取的扇区数
        if (sscanf(buffer, "%*d %*d %s %*d %*d %llu", dev_name, &read_sectors) == 2) 
        {
            // 如果设备名称匹配，保存读取扇区数
            if (strcmp(dev_name, disk) == 0) 
            {
                sectors_read = read_sectors;
                break;
            }
        }
    }
    fclose(fp);
#endif
    // 如果读取了数据，计算并返回字节数
    if (sectors_read > 0) 
    {
        return sectors_read * 512;
    } 
    else 
    {
        klog_err("Disk %s not found or no data\n", diskname);
        return 0;
    }
}

unsigned long long kdk_real_get_disk_write(const char *diskname) 
{
    if(!diskname || access(diskname, F_OK) == -1)
        return 0;
    char buffer[256] = "\0";
    unsigned long long sectors_written = 0;
    
#ifdef __linux__
    FILE *fp;  
    fp = fopen("/proc/diskstats", "r");
    if (!fp)
    {
        return 0;
    }

    char disk[128] = "\0";
    sscanf(diskname, "/dev/%s", disk);
    // 逐行读取文件内容
    while (fgets(buffer, sizeof(buffer), fp)) 
    {
        char dev_name[32] = "\0";
        unsigned long long write_sectors;
        // 解析行内容，提取设备名称和读取的扇区数
        if (sscanf(buffer, "%*d %*d %s %*d %*d %*d %*d %*d %*d %llu", dev_name, &write_sectors) == 2)  
        {
            // 如果设备名称匹配，保存读取扇区数
            if (strcmp(dev_name, disk) == 0) 
            {
                sectors_written = write_sectors;
                break;
            }
        }
    }
    fclose(fp);
#endif
    // 如果读取了数据，计算并返回字节数
    if (sectors_written > 0) 
    {
        return sectors_written * 512;
    } 
    else 
    {
        klog_err("Disk %s not found or no data\n", diskname);
        return 0;
    }
}

long long _kdk_real_get_cpu_instructions(int pid, int sleepTime) 
{
    struct perf_event_attr pe;
    long long cpu_instructions_count = 0;;  // 类型改为 long long
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;   // 配置要监控的具体硬件事件类型为CPU执行的指令数
    pe.disabled = 1;
    pe.inherit = 1;
    pe.exclude_kernel = 1;  // 排除内核级别的指令
    pe.exclude_hv = 1;      // 排除虚拟化层的指令

#ifdef __linux__
    // 使用 perf_event_open 监控指定进程
    fd = syscall(__NR_perf_event_open, &pe, pid, -1, -1, 0);
    if (fd == -1) 
    {
        return -1;
    }

    // 启用计数器
    if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1 ||
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
        close(fd);
        return -1;
    }

    sleep(sleepTime);

    // 禁用计数器
    if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
        close(fd);
        return -1;
    }

    // 读取计数器值（8 字节）
    if (read(fd, &cpu_instructions_count, sizeof(cpu_instructions_count)) == -1) {
        close(fd);
        return -1;
    }

    close(fd);
#endif

    return cpu_instructions_count;  // 返回在函数执行期间记录到的 CPU 指令数
}

int kdk_real_get_cpu_instructions(int pid, int sleepTime)
{
    if(sleepTime < 0)
        return -1;

    int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -1;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getCpuInstructions");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &sleepTime, DBUS_TYPE_INVALID))
    {
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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

out:
    if(ret == -1)
    {
        ret = _kdk_real_get_cpu_instructions(pid, sleepTime);
    }

    int cpu_instructions_count;
    cpu_instructions_count = ret;

    return cpu_instructions_count;
}

long long _kdk_real_get_cpu_cache_misses(int pid, int sleepTime) {
    struct perf_event_attr pe;
    long long cpu_cache_misses_count = 0;
    int fd;

     // 初始化 perf_event_attr 结构体
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE; 
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CACHE_MISSES; // 设置为计数缓存未命中事件
    pe.disabled = 1;    // 计数器初始时是禁用的
    pe.inherit = 1;     // 子进程继承
    pe.exclude_kernel = 1;  // 排除内核级别的事件
    pe.exclude_hv = 1;      // 排除虚拟化层的事件

    // 打开性能计数器，监控指定进程
    fd = syscall(__NR_perf_event_open, &pe, pid, -1, -1, 0);
    if (fd == -1) 
    {
        klog_err("perf_event_open");
        return -1;
    }

    // 重置并启用计数器（检查错误）
    if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1)
    {
        klog_err("PERF_EVENT_IOC_RESET");
        close(fd);
        return -1;
    }
    if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) 
    {
        klog_err("PERF_EVENT_IOC_ENABLE");
        close(fd);
        return -1;
    }

    // 假设有一段耗时操作或工作逻辑（此处可替换为实际任务）
    sleep(sleepTime);  // 模拟任务执行，100毫秒

    // 禁用计数器
    if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
        klog_err("PERF_EVENT_IOC_DISABLE");
        close(fd);
        return -1;
    }

    // 读取计数值
    if (read(fd, &cpu_cache_misses_count, sizeof(cpu_cache_misses_count)) == -1) {
        klog_err("read");
        close(fd);
        return -1;
    }

    close(fd);
    return cpu_cache_misses_count;
}

int kdk_real_get_cpu_cache_misses(int pid, int sleepTime)
{
    if(sleepTime < 0)
        return -1;
    int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -1;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getCpuCacheMisses");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &sleepTime, DBUS_TYPE_INVALID))
    {
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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

out:
    if(ret == -1)
    {
        ret = _kdk_real_get_cpu_cache_misses(pid, sleepTime);
    }

    int cpu_cache_misses_count;
    cpu_cache_misses_count = ret;

    return cpu_cache_misses_count;
}

long long _kdk_real_get_cpu_branch_misses(int pid, int sleepTime) 
{
    struct perf_event_attr pe;
    long long cpu_branch_miss_count = 0;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE; 
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_BRANCH_MISSES;  // 配置为分支预测未命中事件
    pe.disabled = 1;
    pe.inherit = 1;
    pe.exclude_kernel = 1;  // 排除内核级别的分支预测未命中
    pe.exclude_hv = 1;      // 排除虚拟化层的分支预测未命中

#ifdef __linux__
    // 使用 perf_event_open 监控指定进程
    fd = syscall(__NR_perf_event_open, &pe, pid, -1, -1, 0);
    if (fd == -1) 
    {
        klog_err("perf_event_open");
        return -1;
    }

    // 启用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1)      // 重置计数器
    {
        klog_err("PERF_EVENT_IOC_RESET");
        close(fd);
        return -1;
    }

    if(ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1)  // 启用计数器
    {
        klog_err("PERF_EVENT_IOC_ENABLE");
        close(fd);
        return -1;
    }

    // 假设函数执行逻辑在此模拟
    sleep(sleepTime);  // 模拟一些耗时操作

    // 禁用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1)
    {
        klog_err("PERF_EVENT_IOC_DISABLE");
        close(fd);
        return -1;
    }

    // 读取分支预测未命中次数
    if (read(fd, &cpu_branch_miss_count, sizeof(cpu_branch_miss_count)) == -1) 
    {
        close(fd);
        return -1;
    }

    // 关闭计数器文件描述符
    close(fd);
#endif

    return cpu_branch_miss_count;  // 返回分支预测未命中次数
}

int kdk_real_get_cpu_branch_misses(int pid, int sleepTime)
{
    if(sleepTime < 0)
        return -1;
    int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -1;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getCpuBranchMisses");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &sleepTime, DBUS_TYPE_INVALID))
    {
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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

out:
    if(ret == -1)
    {
        ret = _kdk_real_get_cpu_branch_misses(pid, sleepTime);
    }

    int cpu_branch_miss_count;
    cpu_branch_miss_count = ret;

    return cpu_branch_miss_count;
}

long long _kdk_real_get_cpu_bus_cycles(int pid, int sleepTime) 
{
    struct perf_event_attr pe;
    long long cpu_bus_cycles_count = 0;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE; 
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_BUS_CYCLES;  // 配置为系统总线周期数
    pe.disabled = 1;
    pe.inherit = 1;
    pe.exclude_kernel = 1;  // 排除内核级别的总线周期数
    pe.exclude_hv = 1;      // 排除虚拟化层的总线周期数

#ifdef __linux__
    // 使用 perf_event_open 监控指定进程
    fd = syscall(__NR_perf_event_open, &pe, pid, -1, -1, 0);
    if (fd == -1) 
    {
        return -1;
    }

    // 启用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1)  // 重置计数器
    {
        klog_err("PERF_EVENT_IOC_RESET");
        close(fd);
        return -1;
    }
    if(ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1)  // 启用计数器
    {
        klog_err("PERF_EVENT_IOC_ENABLE");
        close(fd);
        return -1;
    }

    // 假设函数执行逻辑在此模拟
    sleep(sleepTime);  // 模拟一些耗时操作

    // 禁用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1)
    {
        klog_err("PERF_EVENT_IOC_DISABLE");
        close(fd);
        return -1;
    }

    // 读取系统总线周期数
    if (read(fd, &cpu_bus_cycles_count, sizeof(cpu_bus_cycles_count)) == -1) 
    {
        close(fd);
        return -1;
    }

    // 关闭计数器文件描述符
    close(fd);
#endif

    return cpu_bus_cycles_count;  // 返回系统总线周期数
}

int kdk_real_get_cpu_bus_cycles(int pid, int sleepTime)
{
    if(sleepTime < 0)
        return -1;
    
    int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -1;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getCpuBusCycles");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &sleepTime, DBUS_TYPE_INVALID))
    {
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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

out:
    if(ret == -1)
    {
        ret = _kdk_real_get_cpu_bus_cycles(pid, sleepTime);
    }

    int cpu_bus_cycles_count;
    cpu_bus_cycles_count = ret;

    return cpu_bus_cycles_count;
}

long long _kdk_real_get_cpu_cycles(int pid, int sleepTime) 
{
    struct perf_event_attr pe;
    long long cpu_cycles_count = 0;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE; 
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CPU_CYCLES;  // 配置为 CPU 时钟周期数
    pe.disabled = 1;
    pe.inherit = 1;
    pe.exclude_kernel = 1;  // 排除内核级别的时钟周期数
    pe.exclude_hv = 1;      // 排除虚拟化层的时钟周期数

#ifdef __linux__
    // 使用 perf_event_open 监控指定进程
    fd = syscall(__NR_perf_event_open, &pe, pid, -1, -1, 0);
    if (fd == -1) 
    {
        return -1;
    }

    // 启用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1)  // 重置计数器
    {
        klog_err("PERF_EVENT_IOC_RESET");
        close(fd);
        return -1;
    }
    if(ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1)  // 启用计数器
    {
        klog_err("PERF_EVENT_IOC_ENABLE");
        close(fd);
        return -1;
    }

    // 假设函数执行逻辑在此模拟
    sleep(sleepTime);  // 模拟一些耗时操作

    // 禁用事件计数器
    if(ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1)
    {
        klog_err("PERF_EVENT_IOC_ENABLE");
        close(fd);
        return -1;
    }

    // 读取CPU时钟周期数
    if (read(fd, &cpu_cycles_count, sizeof(cpu_cycles_count)) == -1) 
    {
        close(fd);
        return -1;
    }

    // 关闭计数器文件描述符
    close(fd);
#endif

    return cpu_cycles_count;  // 返回CPU时钟周期数
}

int kdk_real_get_cpu_cycles(int pid, int sleepTime)
{
    if(sleepTime < 0)
        return -1;

    int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -1;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/runinfo", // object to call on
                                            "com.kylin.kysdk.runinfo",  // interface to call on
                                            "getCpuCycles");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &sleepTime, DBUS_TYPE_INVALID))
    {
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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

out:
    if(ret == -1)
    {
        ret = _kdk_real_get_cpu_cycles(pid, sleepTime);
    }

    int cpu_cycles_count;
    cpu_cycles_count = ret;

    return cpu_cycles_count;
}

unsigned long kdk_real_get_disk_queue_length(char *diskname)
{
    if(!diskname)
        return IO_ERROR;
    FILE *fp = NULL;
    char line[512] = "\0";
    char disk[128] = "\0";
    char disk_name[48] = "\0";
    sscanf(diskname, "/dev/%s", disk);
    unsigned long queue;
    fp = fopen("/proc/diskstats", "r");
    if(!fp)
        return IO_ERROR;

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%*d %*d %s %*d %*d %*d %*d %*d %*d %*d %*d %lu", disk_name, &queue);
        if(strcmp(disk_name, disk) == 0)
        {
            return queue;
        }
    }
    return IO_ERROR;
}

float kdk_real_get_cpu_kernel_modes_per()
{
    FILE *file = NULL;
    char buf[1024] = "\0";
    float ratio = 0.0;
    unsigned long long total_time;   /* jiffies used by this process */
    unsigned long long user;  // user time
    unsigned long long system;  // kernel time

    // Read process stats
    file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return ratio;
    }
    if (fgets(buf, sizeof(buf) - 1, file) == NULL) {
        fclose(file);
        return ratio;
    }
    fclose(file);
    strstripspace(buf);
    
    // Parse the required fields
    sscanf(buf, "%*s %llu %*d %llu", &user, &system);
    total_time = user + system;
    ratio = (float) system / total_time;
    return ratio;
}

float kdk_real_get_cpu_user_modes_per()
{
    FILE *file = NULL;
    char buf[1024] = "\0";
    float ratio = 0.0;
    unsigned long long total_time;   /* jiffies used by this process */
    unsigned long long user;  // user time
    unsigned long long system;  // kernel time

    // Read process stats
    file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return ratio;
    }
    if (fgets(buf, sizeof(buf) - 1, file) == NULL) {
        fclose(file);
        return ratio;
    }
    fclose(file);
    strstripspace(buf);
    
    // Parse the required fields
    sscanf(buf, "%*s %llu %*d %llu", &user, &system);
    total_time = user + system;
    ratio = (float) user / total_time;
    return ratio;
}

int _kdk_real_get_cpu_queue_length()
{
    char path[128] = "\0";
    char thread_path[256] = "\0";
    char line[512] = "\0";
    char status[12] = "\0";
    int num = 0;
    // sprintf(path, "proc/%d/task", pid);
    FILE *fp = NULL;

    DIR *dir = NULL, *dirfd = NULL;
    dir = opendir("/proc");
    if(!dir)
    {
        return -1;
    }
    struct dirent *direfd;
    struct dirent *pthfd;
    while ((direfd = readdir(dir)) != NULL) 
    {
        if ((!strncmp(direfd->d_name, ".", 1)) || (!strncmp(direfd->d_name, "..", 2)))
            continue;
        if (DT_DIR != direfd->d_type)
            continue;
        
        snprintf(path, sizeof(path), "/proc/%s/task", direfd->d_name);
        
        dirfd = opendir(path);
        if(!dirfd)
        {
            continue;
        }
        while ((pthfd = readdir(dirfd)) != NULL)
        {
            if ((!strncmp(pthfd->d_name, ".", 1)) || (!strncmp(pthfd->d_name, "..", 2)))
                continue;
            if (DT_DIR != pthfd->d_type)
                continue;

            snprintf(thread_path, sizeof(thread_path), "%s/%s/stat", path, pthfd->d_name);
            fp = fopen(thread_path, "r");
            if(!fp)
            {
                continue;
            }
            fgets(line, sizeof(line), fp);
            fclose(fp);
            sscanf(line, "%*d (%*[^)]) %s", status);
            if(strcmp(status, "R") == 0)
            {
                num++;
            }
            else{
                continue;
            }
        }
        closedir(dirfd);
    }
    closedir(dir);
    return num;
}

int kdk_real_get_cpu_queue_length()
{
    DBusConnection *conn;
    DBusError err;
    int num = 0;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        num = -1;
        goto out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",   // target for the method call
                                            "/com/kylin/kysdk/runinfo",     // object to call on
                                            "com.kylin.kysdk.runinfo", // interface to call on
                                            "getCpuQueueLength");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        num = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        num = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        num = -1;
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
        num = -1;
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
        num = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        num = -1;
        goto out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &num);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    return num;
out:
    if(num == -1)
    {
        num = _kdk_real_get_cpu_queue_length();
    }
    return num;
}

static unsigned long lookup(char* buffer, const char* key, int *res)
{
    if (strstartswith(buffer, key) == 0)
    {
        char *val = buffer;
        val += strnlen(key, STRNLEN_MAX) + 1;
        *res = atoi(strskipblank(val));
        return 1;
    }
    return 0;
}

int kdk_real_get_memory_page()
{
    FILE *fp = NULL;
    char line[128] = "\0";
    int pswin = 0;
    int pswpout = 0;
    int pages_per_sec = 0;
    fp = fopen("/proc/vmstat", "r");
    if(!fp)
    {
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if(strstr(line, "pswpin"))
        {
            if(lookup(line, "pswpin", &pswin))
            {
                continue;
            }
        }

        if(strstr(line, "pswpout"))
        {
            if(lookup(line, "pswpout", &pswpout))
            {
                continue;
            }
        }
    }
    fclose(fp);
    
    pages_per_sec = pswin + pswpout;
    return pages_per_sec;
}

float kdk_real_get_cpu_hard_page_faults()
{
    FILE *fp = NULL;
    char line[128] = "\0";
    int pgmajfault = 0;
    float faults = 0.00;
    fp = fopen("/proc/vmstat", "r");
    if(!fp)
    {
        return faults;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if(strstr(line, "pgmajfault"))
        {
            if(lookup(line, "pgmajfault", &pgmajfault))
            {
                break;
            }
        }
    }
    fclose(fp);

    float user = 0.00, sys = 0.00, idle = 0.00, iowait = 0.00;
    float total = 0.00;
    
    FILE *fb = fopen("/proc/stat", "rt");
    if (!fb)
        return faults;
    char buffer[1025];
    while (fgets(buffer, sizeof(buffer), fb))
    {
        if (strstartswith(buffer, "cpu") == 0)
        {
            char **list = strsplit(buffer, ' ');
            if (!list)
            {
                fclose(fb);
                return faults;
            }
            user = atol(list[1]);
            sys = atol(list[3]);
            idle = atol(list[4]);
            iowait = atol(list[5]);
            free(list);
            break;
        }
        else
            break;
    }
    total = user + sys + idle + iowait;
    fclose(fb);
    faults = (float)pgmajfault / total;

    return faults;
}

float kdk_real_get_cpu_soft_page_faults()
{
    FILE *fp = NULL;
    char line[128] = "\0";
    int pgfault = 0;
    float faults = 0.00;
    fp = fopen("/proc/vmstat", "r");
    if(!fp)
    {
        return faults;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if(strstr(line, "pgfault"))
        {
            if(lookup(line, "pgfault", &pgfault))
            {
                break;
            }
        }
    }
    fclose(fp);

    float user = 0.00, sys = 0.00, idle = 0.00, iowait = 0.00;
    float total = 0.00;
    
    FILE *fb = fopen("/proc/stat", "rt");
    if (!fb)
        return faults;
    char buffer[1025];
    while (fgets(buffer, sizeof(buffer), fb))
    {
        if (strstartswith(buffer, "cpu") == 0)
        {
            char **list = strsplit(buffer, ' ');
            if (!list)
            {
                fclose(fb);
                return faults;
            }
            user = atol(list[1]);
            sys = atol(list[3]);
            idle = atol(list[4]);
            iowait = atol(list[5]);
            free(list);
            break;
        }
        else
            break;
    }
    total = user + sys + idle + iowait;
    fclose(fb);
    faults = (float)pgfault / total;

    return faults;
}
