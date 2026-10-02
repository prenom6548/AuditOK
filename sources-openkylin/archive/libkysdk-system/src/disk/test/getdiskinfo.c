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

#include "../libkydiskinfo.h"
#include <stdio.h>
#include <stdlib.h>

static void display_disk_info(kdk_diskinfo *di)
{
    printf("---------------%s------------------\n", di->name);
    printf("Disk Sectors:\t%llu\n", di->sectors_num);
    printf("Disk Sector Size:\t%u\n", di->sector_size);
    printf("Disk Size:\t%f MiB\n", di->total_size_MiB);
    printf("Disk Model:\t%s\n", di->model ? di->model : "None");
    printf("Disk Serial:\t%s\n", di->serial ? di->serial : "None");
    printf("Disk Partition Number:\t%u\n", di->partition_nums);
    printf("Disk Type:\t%d\n", di->disk_type);
    printf("Disk fwrev:\t%s\n", di->fwrev);
    printf("available space: %.2f MiB\n", kdk_get_partition_available_space(di->name));
    printf("Disk Format: %d\n", di->format);
}

static void display_disk_list(char **disklist)
{
    int count = 0;
    while (disklist[count])
    {
        printf("No. %d\t %s\n", count + 1, disklist[count]);
        count ++;
    }
}

void test_delete_all_partitions(const char *device_path) 
{
    int result = kdk_disk_delete_all_partitions(device_path);
    if (result == 0) 
    {
        printf("成功删除所有分区：%s\n", device_path);
    } 
    else 
    {
        printf("删除分区失败：%s\n", device_path);
    }
}

void test_kdk_disk_get_volume_label(const char *partition) 
{
    // 调用 kdk_disk_get_volume_label 函数获取分区的卷标
    char *label = kdk_disk_get_volume_label(partition);

    // 检查返回值，判断是否成功获取到卷标
    if (label != NULL) 
    {
        printf("Volume label for partition %s: %s\n", partition, label);
        // 释放函数返回的内存
        free(label);
    } 
    else 
    {
        printf("Failed to get volume label for partition %s\n", partition);
    }
}

void test_kdk_disk_get_mount_point() 
{
    const char *partition = "/dev/nvme0n1p1"; 
    char *mount_point = kdk_disk_get_mount_point(partition);
    
    if (mount_point) 
    {
        printf("Mount point for partition %s: %s\n", partition, mount_point);
        free(mount_point); // 释放分配的内存
    } 
    else 
    {
        printf("No mount point found for partition: %s\n", partition);
    }
}

void test_get_partition_start_sector(const char *partition) 
{
    long long start_sector = kdk_disk_get_partition_start_sector(partition);
    if (start_sector == -1) 
    {
        printf("无法获取分区 %s 的首扇区\n", partition);
    } 
    else 
    {
        printf("分区 %s 的首扇区: %d\n", partition, start_sector);
    }
}

void test_get_partition_end_sector(const char *partition) 
{
    long long end_sector = kdk_disk_get_partition_end_sector(partition);
    if (end_sector == -1) 
    {
        printf("无法获取分区 %s 的末扇区\n", partition);
    } 
    else 
    {
        printf("分区 %s 的末扇区: %d\n", partition, end_sector);
    }
}

void test_mount_partition(const char *partition, const char *mount_point) 
{
    printf("尝试挂载分区 %s 到路径 %s...\n", partition, mount_point);

    int result = kdk_disk_mount_partition(partition, mount_point);

    if (result == 0) 
    {
        printf("挂载成功！\n");
    } 
    else 
    {
        printf("挂载失败！请检查分区和挂载点是否正确。\n");
    }
}

void test_kdk_disk_delete_partition(const char *partition) 
{
    // 调用删除分区函数
    int result = kdk_disk_delete_partition(partition);
    if (result == 0) 
    {
        printf("分区 %s 删除成功！\n", partition);
    }
    else 
    {
        printf("分区 %s 删除失败。\n", partition);
    }
}

void test_unmount_partition() 
{
    const char *partition = "/dev/sda5"; // 要卸载的分区

    printf("尝试卸载分区 %s...\n", partition);
    
    int result = kdk_disk_unmount_partition(partition);
    
    if (result == 0) 
    {
        printf("分区 %s 卸载成功！\n", partition);
    } 
    else 
    {
        printf("分区 %s 卸载失败！请检查分区是否正在被使用。\n", partition);
    }
}

void test_get_partition_table_type() 
{
    const char *device = "/dev/nvme0n1"; // 目标设备

    char *partition_type = kdk_disk_get_partition_table_type(device);
    
    if (partition_type) 
    {
        printf("分区表类型: %s\n", partition_type);
        free(partition_type);
    } 
    else 
    {
        printf("获取分区表类型失败");
    }
}

void test_kdk_disk_get_total_tracks() 
{
    const char *device = "/dev/sda";  // 可以根据实际设备修改
    int heads = kdk_disk_get_total_tracks(device);
    
    if (heads != -1) 
    {
        printf("Disk %s has %d heads\n", device, heads);
    } 
    else 
    {
        printf("Failed to get heads for disk %s\n", device);
    }
}

void test_kdk_disk_is_disk_writable() 
{

    const char *device_path = "/dev/sda"; // 你可以根据需要修改这个设备路径

    // 调用函数检查设备是否可写
    if (kdk_disk_is_disk_writable(device_path) == 0) 
    {
        printf("设备 %s 可写\n", device_path);
    } 
    else 
    {
        printf("设备 %s 不可写\n", device_path);
    }
}

void test_get_disk_geometry() 
{
    const char *device_path = "/dev/sda";

    DiskGeometry *geometry = kdk_disk_get_disk_geometry(device_path);

    if (geometry != NULL) 
    {
        printf("Disk Geometry for %s:\n", device_path);
        printf("Cylinders: %u\n", geometry->cylinders);
        printf("Tracks per Cylinder: %u\n", geometry->tracks_per_cylinder);
        printf("Sectors per Track: %u\n", geometry->sectors_per_track);
        printf("Bytes per Sector: %u\n", geometry->bytes_per_sector);
        
        // 释放内存
        free(geometry);
    } 
    else 
    {
        printf("Failed to get disk geometry for %s\n", device_path);
    }
}

void test_kdk_disk_sync() 
{
    const char *device = "/dev/sda"; 

    printf("正在尝试刷新设备: %s\n", device);

    int result = kdk_disk_sync(device); // 调用刷新函数

    if (result == 0) 
    {
        printf("磁盘刷新成功\n");
    } 
    else 
    {
        printf("磁盘刷新失败\n");
    }
}

int main()
{
    // const char *device_path = "/dev/sdb"; 
    // test_delete_all_partitions(device_path);  //需要换电脑测试

    // const char *test_partition = "/dev/sda1";
    // test_kdk_disk_get_volume_label(test_partition);  //需要dbus
    
    // test_kdk_disk_get_mount_point(); 

    // const char *partition = "/dev/nvme0n1p1"; 
    // test_get_partition_start_sector(partition);  //需要dbus
    // test_get_partition_end_sector(partition);   //需要dbus

    // partition = "/dev/sda5";
    // const char *mount_point = "/mnt/data";
    // test_mount_partition(partition, mount_point);  //需要dbus

    // test_partition = "/dev/sdb2";
    // test_kdk_disk_delete_partition(test_partition);  //需要换电脑测试

    // test_unmount_partition();  //需要dbus

    // test_get_partition_table_type();  //需要dbus

    // test_kdk_disk_get_total_tracks();  //需要dbus   

    // test_kdk_disk_is_disk_writable();  //需要dbus 

    // test_kdk_disk_sync();  //需要dbus

    // test_get_disk_geometry();  //需要dbus

    char **disklist = kdk_get_disklist();
    // display_disk_list(disklist);
    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s]Get disk info error.\n", disklist[i]);
            continue;
        }

        display_disk_info(sdainfo);

        kdk_free_diskinfo(sdainfo);

        int time = kdk_get_hard_running_time(disklist[i]);
        printf("运行时长 = %d\n", time);

        sdainfo = kdk_get_diskinfo("/dev/abc");
        if (!sdainfo)
        {
            printf("[%s]Get disk info error.\n", disklist[i]);
            continue;
        }
    }
    kdk_free_disklist(disklist);

    char **di = kdk_get_hard_disk();
    size_t num = 0;
    while (di[num])
    {

        // double read_speed = kdk_get_disk_read_speed(di[num]);
        // printf("磁盘读取速度 = %0.2f\n", read_speed);

        // double write_speed = kdk_get_disk_write_speed(di[num]);
        // printf("磁盘写入速度 = %0.2f\n", write_speed);

        printf("No. %ld\t %s\n", num + 1, di[num]);
        char *res = kdk_get_hard_disk_size(di[num]);
        printf("容量 = %s\n", res);
        free(res);

        res = kdk_get_hard_fwrev(di[num]);
        printf("固态版本 = %s\n", res);
        free(res);

        res = kdk_get_hard_vendor(di[num]);
        printf("厂商 = %s\n", res);
        free(res);

        res = kdk_get_hard_type(di[num]);
        printf("类型 = %s\n", res);
        free(res);

        res = kdk_get_hard_model(di[num]);
        printf("型号 = %s\n", res);
        free(res);

        res =kdk_get_hard_serial(di[num]);
        printf("序列号 = %s\n", res);
        free(res);

        num ++;
    }
    kdk_free_disklist(di);

    printf("available space: %.2f MiB\n", kdk_get_partition_available_space("/dev/abc"));
    int time = kdk_get_hard_running_time("/dev/abc");
    printf("运行时长 = %d\n", time);

    char *res = kdk_get_hard_serial("/dev/abc");
    printf("序列号 = %s\n", res);
    free(res);

    res = kdk_get_hard_model("/dev/abc");
    printf("型号 = %s\n", res);
    free(res);

    res = kdk_get_hard_type("/dev/abc");
    printf("类型 = %s\n", res);
    free(res);

    res = kdk_get_hard_fwrev("/dev/abc");
    printf("固态版本 = %s\n", res);
    free(res);
    
    res = kdk_get_hard_disk_size("/dev/abc");
    printf("容量 = %s\n", res);
    free(res);

    res = kdk_get_hard_vendor("/dev/abc");
    printf("厂商 = %s\n", res);
    free(res);

    return 0;
}
