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

#include "../../proc/libkyprocess.h"
#include "../../proc/libkyrtinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>

TEST(KdkProcTest, ProcessInfo)
{
    size_t index = 0;
    char *run_time = kdk_get_process_running_time(10193);
    char *cpu_time = kdk_get_process_cpu_time(10193);
    char *cmd = kdk_get_process_command(10193);
    char *start_time = kdk_get_process_start_time(10193);
    char *status = kdk_get_process_status(10193);
    char *user = kdk_get_process_user(10193);
    int *port = kdk_get_process_port_nums(87963);
    printf("获取某一进程的CPU利用率：%0.1f\n", kdk_get_process_cpu_usage_percent(10193));
    printf("获取某一进程的内存占用率：%0.1f\n", kdk_get_process_mem_usage_percent(10193));
    printf("获取某一进程的进程状态：%s\n", status);
    printf("获取某一进程的进程端口号：%d\n", kdk_get_process_port(87963));
    printf("获取某一进程的启动时间：%s\n", start_time);
    printf("获取某一进程的运行时间：%s\n", run_time);
    printf("获取某一进程的CPU时间：%s\n", cpu_time);
    printf("获取某一进程的Command：%s\n", cmd);
    printf("获取某一进程的属主：%s\n", user);
    free(status);
    free(run_time);
    free(cpu_time);
    free(cmd);
    free(start_time);
    free(user);

    if (port)
    {
        while (port[index])
        {
            printf("获取某一进程的进程端口号：%d\n", port[index]);
            index++;
        }
        kdk_proc_freeid(port);
    }
    
    char** pid =  kdk_procname_get_process_infomation("systemd");
    if (NULL != pid)
    {
        index = 0;
        while (pid[index])
        {
            printf("pid %s\n", pid[index]);
            index++;
        }
        kdk_proc_freeall(pid);
    }

    char** info = kdk_get_process_all_information();
    size_t count = 0;
    while (info[count])
    {
        printf("No. %d\t %s\n", count + 1, info[count]);
        count ++;
    }
    // kdk_proc_freeall(info);

    char *name = kdk_get_process_name(10193);
    printf("name = %s\n", name);
    free(name);

    int id =  kdk_get_process_id("systemd");
    printf("pid %d\n", id);

    int *pid_nums = kdk_get_process_id_nums("systemd");
    index = 0;
    if (pid_nums)
    {
        while (pid_nums[index])
        {
            printf("获取某一进程的进程端口号：%d\n", pid_nums[index]);
            index++;
        }
        // kdk_proc_freeid(pid_nums);
    }
}

TEST(KdkProcTest, ProcRtInfo)
{
    printf("内存总大小：%lu KiB\n", kdk_rti_get_mem_res_total_KiB());
    printf("当前已用内存：%lu KiB，%f\n", kdk_rti_get_mem_res_usage_KiB(), kdk_rti_get_mem_res_usage_percent());
    printf("当前空闲内存：%lu KiB\n", kdk_rti_get_mem_res_free_KiB());
    printf("可用内存大小：%lu KiB\n", kdk_rti_get_mem_res_available_KiB());
    printf("应用总申请虚拟内存大小：%lu KiB\n", kdk_rti_get_mem_virt_alloc_KiB());
    printf("交换分区总量：%lu KiB\n", kdk_rti_get_mem_swap_total_KiB());
    printf("交换分区已用量：%lu KiB, %f\n", kdk_rti_get_mem_swap_usage_KiB(), kdk_rti_get_mem_swap_usage_percent());
    printf("交换分区剩余大小：%lu KiB\n", kdk_rti_get_mem_swap_free_KiB());
    printf("当前CPU使用率：%f\n", kdk_rti_get_cpu_current_usage());
    printf("共享内存大小：%lu KiB\n", kdk_rti_get_mem_shared_KiB());
    printf("高速缓存大小：%lu KiB\n", kdk_rti_get_mem_cached_KiB());
    printf("数据缓存大小：%lu KiB\n", kdk_rti_get_mem_buffers_KiB());
    printf("交换缓存区大小：%lu KiB\n", kdk_rti_get_mem_swap_cached_KiB());
    printf("活跃的缓冲文件大小：%lu KiB\n", kdk_rti_get_mem_active_KiB());
    printf("不活跃的缓冲文件大小：%lu KiB\n", kdk_rti_get_mem_inactive_KiB());
    printf("脏页大小：%lu KiB\n", kdk_rti_get_mem_dirty_KiB());
    printf("映射大小：%lu KiB\n", kdk_rti_get_mem_map_KiB());
    printf("内核数据结构缓存大小：%lu KiB\n", kdk_rti_get_mem_slab_KiB());
    unsigned int day, hour, min, sec;
    kdk_rti_get_uptime(&day, &hour, &min, &sec);
    printf("开机时长：%u天%u小时%u分钟%u秒\n", day, hour, min, sec);
}