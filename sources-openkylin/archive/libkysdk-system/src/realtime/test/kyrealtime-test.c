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

#include "stdio.h"
#include "../libkyrealtimeinfo.h"

int main()
{
    // if(argc < 3)
    // {
    //     printf("./kyrealtime-test <网卡名> <磁盘名>\n");
    //     return 0;
    // }
    int pid = 15055;
    int sleepTime = 1;
    printf("netSpeed : %f\n", kdk_real_get_net_speed("enaftgm1i0"));
    printf("cpuTemp: %f\n", kdk_real_get_cpu_temperature());
    printf("diskTemp : %d\n", kdk_real_get_disk_temperature("/dev/nvme0n1"));
    printf("diskRate : %d\n", kdk_real_get_disk_rate("/dev/nvme0n1"));
    // printf("ifnetSpeed : %f\n", kdk_real_get_if_speed("enaftgm1i0"));
    // printf("磁盘 %s 读取字节数：%llu\n", "nvme0n1", kdk_real_get_disk_read("/dev/nvme0n1"));
    // printf("磁盘 %s 写入字节数：%llu\n", "nvme0n1", kdk_real_get_disk_write("/dev/nvme0n1"));
    // int cpu_instructions_count = kdk_real_get_cpu_instructions(pid, sleepTime);
    // int cpu_cache_misses_count = kdk_real_get_cpu_cache_misses(pid, sleepTime);
    // int cpu_branch_miss_count = kdk_real_get_cpu_branch_misses(pid, sleepTime);
    // int cpu_bus_cycles_count = kdk_real_get_cpu_bus_cycles(pid, sleepTime);
    // int cpu_cycles_count = kdk_real_get_cpu_cycles(pid, sleepTime);
    // printf("Process %d executed %d cpu_instructions_count.\n", pid, cpu_instructions_count);
    // printf("Process %d executed %d cpu_cache_misses_count.\n", pid, cpu_cache_misses_count);
    // printf("Process %d executed %d cpu_branch_miss_count.\n", pid, cpu_branch_miss_count);
    // printf("Process %d executed %d cpu_bus_cycles_count.\n", pid, cpu_bus_cycles_count);
    // printf("Process %d executed %d cpu_cycles_count.\n", pid, cpu_cycles_count);

    unsigned long queue_length = kdk_real_get_disk_queue_length("/dev/nvme0n1");
    if(queue_length != IO_ERROR)
    {
        printf("queue_length = %lu\n", queue_length);
    }

    float kernel_ratio =  kdk_real_get_cpu_kernel_modes_per();
    printf("kernel_ratio = %.2f\n", kernel_ratio);

    float user_ratio =  kdk_real_get_cpu_user_modes_per();
    printf("user_ratio = %.2f\n", user_ratio);

    int length = kdk_real_get_cpu_queue_length();
    printf("length = %d\n", length);
    
    int page =  kdk_real_get_memory_page();
    printf("page = %d\n", page);

    float hard_faults = kdk_real_get_cpu_hard_page_faults();
    printf("hard_faults = %.2f\n", hard_faults);

    float soft_faults = kdk_real_get_cpu_soft_page_faults();
    printf("soft_faults = %.2f\n", soft_faults);
    
    return 0;
}
