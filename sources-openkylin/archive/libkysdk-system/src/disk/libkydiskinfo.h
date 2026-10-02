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

#ifndef KYSDK_SYSTEM_DISK_INFO_H__
#define KYSDK_SYSTEM_DISK_INFO_H__

/**
 * @file libkydiskinfo.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 获取磁盘信息
 * @version 2.2
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-disk
 * @{
 * @since 1.0.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#define FWREV_SIZE    20

typedef enum _kdk_disk_type{
    DISK_TYPE_HDD,      // 机械
    DISK_TYPE_SSD,      // 固态
    DISK_TYPE_HHD,      // 混合
    DISK_TYPE_FLASH,    // 闪存，比如U盘啥的
    DISK_TYPE_NONE      // 分区盘
}kdk_disk_type;

typedef enum _kdk_disk_interface_type{
    DISK_INTERFACE_SATA,
    DISK_INTERFACE_SATA2,
    DISK_INTERFACE_SATA3,
    DISK_INTERFACE_mSATA,
    DISK_INTERFACE_SAS,
    DISK_INTERFACE_M2,
    DISK_INTERFACE_NVMe,
    DISK_INTERFACE_PCIE,
    DISK_INTERFACE_NONE
}kdk_disk_interface_type;

typedef enum _kdk_disk_format{
    DISK_FMT_UNKNOW,
    DISK_FMT_FAT16,
    DISK_FMT_FAT32,
    DISK_FMT_VFAT,
    DISK_FMT_NTFS,
    DISK_FMT_EXT2,
    DISK_FMT_EXT3,
    DISK_FMT_EXT4,
    DISK_FMT_NONE,
    DISK_FMT_SWAP
}kdk_disk_format;

typedef enum _kdk_disk_partition_type{
    DISK_PART_DISK,     // 基础硬盘
    DISK_PART_MASTER,   // 主分区
    DISK_PART_EXT,      // 扩展分区
    DISK_PART_LOGIC,    // 逻辑分区
    DISK_PART_RAID,     // RAID分区
    DISK_PART_UNKNOW
}kdk_disk_partition_type;

/**
 * @struct kdk_diskinfo
 * @brief 磁盘信息结构体
 */
typedef struct _kdk_diskinfo{
    char *name;     // 绝对路径
    char *vendor;   // 制造商
    char *model;    // 型号
    char *serial;   // 序列号

    kdk_disk_type disk_type;  // 磁盘类型，固态 or 机械 or 混合
    kdk_disk_interface_type inter_type;   // 接口类型
    unsigned int rpm;  // 转速，注意固态是没有转速概念的

    unsigned long long sectors_num;   // 扇区数量
    unsigned int sector_size;   // 每个扇区的字节数
    float total_size_MiB;  // 磁盘容量，MiB为单位
    
    unsigned int partition_nums; // 该磁盘/分区下的子分区数量
    char *uuid;     // UUID
    short mounted;   // 是否已挂载
    char *mount_path;   // 挂载路径
    kdk_disk_format format; // 格式化类型
    kdk_disk_partition_type part_type;  // 分区类型
    char *fwrev;    //固件版本信息

}kdk_diskinfo;

typedef enum 
{
    PARTITION_STYLE_UNKNOWN = 0,
    PARTITION_STYLE_MBR = 1,
    PARTITION_STYLE_GPT = 2
} kdk_partition_style;

typedef struct 
{
    kdk_partition_style partition_style;         // 分区样式（MBR 或 GPT）
    unsigned long long disk_size;                // 磁盘总大小（以字节为单位）
    unsigned int num_partitions;                 // 计划创建的分区数量
    unsigned long long *partition_sizes;         // 每个分区的大小（数组）
} DiskPartitionParams;

typedef struct 
{
    unsigned int cylinders;           // 磁盘的柱面数
    unsigned int tracks_per_cylinder; // 每个柱面的磁道数
    unsigned int sectors_per_track;   // 每个磁道的扇区数
    unsigned int bytes_per_sector;    // 每个扇区的字节数
} DiskGeometry;

/**
 * @brief 获取所有磁盘的列表
 * 
 * @return char**， 每个字符串表示一个磁盘的绝对路径， 结尾以NULL字符表示结束
 */
extern char** kdk_get_disklist();

/**
 * @brief 释放由kdk_get_disklist返回的磁盘列表
 * 
 * @param disklist 由kdk_get_disk_list返回的字符串指针
 */
extern void kdk_free_disklist(char** disklist);

/**
 * @brief 获取指定磁盘的磁盘信息
 * 
 * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径，或者是disklist中的某个元素
 * @return kdk_diskinfo* 该磁盘的详细信息，具体信息自取
 */
extern kdk_diskinfo *kdk_get_diskinfo(const char *diskname);

/**
 * @brief 获取系统接入所有硬盘
 * 
 * @return char** 硬盘名称
 * @since 2.2.3.5
 */
extern char** kdk_get_hard_disk();

/**
 * @brief 获取硬盘大小
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘大小，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_disk_size(const char *hardname);

/**
 * @brief 获取硬盘固件版本
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘固件版本，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_fwrev(const char *hardname);

/**
 * @brief 获取硬盘类型
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘类型，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_type(const char *hardname);

/**
 * @brief 获取硬盘型号
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘型号，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_model(const char *hardname);

/**
 * @brief 获取硬盘序列号
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘序列号，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_serial(const char *hardname);

/**
 * @brief 获取硬盘厂商
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘厂商，失败返回NULL。返回的字符串需要被 free 释放
 * @since 2.2.3.5
 */
extern char* kdk_get_hard_vendor(const char *hardname);

/**
 * @brief 释放由kdk_get_diskinfo返回的磁盘信息结构体
 * 
 * @param disk 由kdk_get_diskinfo返回的结构体指针
 */
extern void kdk_free_diskinfo(kdk_diskinfo *disk);

/**
 * @brief 获取硬盘运行时长
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 硬盘从出厂到现在已运行时长总和
 * @since 2.4.1.0
 */
extern int kdk_get_hard_running_time(const char *hardname);

/**
 * @brief 获取磁盘读取速度
 * 
 * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 磁盘读取速度，单位KB/s
 * @since 2.5.1.2
 */
extern double kdk_get_disk_read_speed(const char *diskname);

/**
 * @brief 获取磁盘写入速度
 * 
 * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 磁盘写入速度，单位KB/s
 * @since 2.5.1.2
 */
extern double kdk_get_disk_write_speed(const char *diskname);

/**
 * @brief 获取分区剩余容量
 *
 * @param partitionname 指定分区名称，应当是例如/dev/sda1这种绝对路径，仅支持分区
 *
 * @return float 分区剩余容量，以MiB为单位，获取失败返回-1
 * @since 3.0.1.0
 */
extern double kdk_get_partition_available_space(const char *partitionname);

/**
 * @brief 删除指定磁盘的所有分区
 * 
 * @param device_path 设备的路径，例如 "/dev/sda"
 * @return int 成功返回0，失败返回-1
 * @since 3.0.1.0
 */
extern int kdk_disk_delete_all_partitions(const char *device_path);

/**
 * @brief 获取指定分区的卷标
 * 
 * @param partition 要查询的磁盘分区名称，如 /dev/sda1
 * @return char* 返回指定分区的卷标，失败返回NULL
 * @since 3.0.1.0
 */
extern char* kdk_disk_get_volume_label(const char *partition);

/**
 * @brief 获取指定分区的挂载点位置
 * 
 * @param partition 要查询的磁盘分区名称，如 /dev/sda1
 * @return char* 返回指定分区的挂载点的路径，失败返回NULL
 * @since 3.0.1.0
 */
extern char* kdk_disk_get_mount_point(const char *partition);

/**
 * @brief 获取指定分区的首扇区
 * 
 * @param partition 要查询的磁盘分区名称，如 /dev/sda1
 * @return long long 返回指定分区的首扇区，失败返回-1
 * @since 3.0.1.0
 */
extern long long kdk_disk_get_partition_start_sector(const char *partition);

/**
 * @brief 获取指定分区的末扇区
 * 
 * @param partition 要查询的磁盘分区名称，如 /dev/sda1
 * @return long long 返回指定分区的末扇区，失败返回-1
 * @since 3.0.1.0
 */
extern long long kdk_disk_get_partition_end_sector(const char *partition);

/**
 * @brief 将指定的分区挂载到指定路径
 * 
 * @param partition 要挂载的磁盘分区名称，如 /dev/sda1
 * @param mount_point 指定分区挂载到的目标路径
 * @return int 返回0表示成功，-1表示失败
 * @since 3.0.1.0
 */
extern int kdk_disk_mount_partition(const char *partition, const char *mount_point);

/**
 * @brief 删除指定的分区
 * 
 * @param partition 要删除的磁盘分区名称，如 /dev/sda1
 * @return int 返回0表示成功，-1表示失败
 * @since 3.0.1.0
 */
extern int kdk_disk_delete_partition(const char *partition);

/**
 * @brief 卸载指定的分区
 * 
 * @param partition 要卸载的磁盘分区名称，如 /dev/sda1
 * @return int 返回0表示成功，-1表示失败
 * @since 3.0.1.0
 */
extern int kdk_disk_unmount_partition(const char *partition);

/**
 * @brief 获取磁盘分区表类型
 * 
 * @param device_path 要查询的磁盘设备的路径，例如 /dev/sda
 * @return char* 返回分区表的类型，失败返回NULL
 * @since 3.0.1.0
 */
extern char* kdk_disk_get_partition_table_type(const char *device_path);

/**
 * @brief 获取磁盘的磁头数量
 * 
 * @param device_path 磁盘设备的路径，例如 /dev/sda
 * @return int 成功返回磁头数量；失败返回 -1
 * @since 3.0.1.0
 */
extern int kdk_disk_get_total_tracks(const char *device_path);

/**
 * @brief 获取指定的磁盘是否可写
 * 
 * @param device_path 设备的路径，例如 "/dev/sda"
 * @return int 0-是，-1-否，-2调用服务失败
 * @since 3.0.1.0
 */
extern int kdk_disk_is_disk_writable(const char *device_path);

/**
 * @brief 获取物理磁盘几何的信息
 * 
 * @param device 磁盘设备路径，例如 /dev/sda
 * @return DiskGeometry* 获取成功返回指向 DiskGeometry 结构体的指针，获取失败返回 NULL
 * @since 3.0.1.0
 */
extern DiskGeometry* kdk_disk_get_disk_geometry(const char *device_path);

/**
 * @brief 刷新磁盘
 * 
 * @param device 磁盘设备路径，例如 /dev/sda
 * @return int 返回 0 表示刷新成功，-1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_disk_sync(const char *device_path);

/**
 * @brief 获取CDROM的介质类型
 * 
 * @param device_path CDROM设备路径，例如 /dev/sr0
 * @return char* 返回CDROM的类型，失败返回NULL
 * @since 3.0.1.0
 */
char *kdk_udf_get_cdrom_type(const char *device_path);

/**
 * @brief 获取CDROM的容量
 * 
 * @param device_path CDROM设备路径，例如 /dev/sr0
 * @return long 返回CDROM的容量
 * @since 3.0.1.0
 */
unsigned long kdk_udf_get_cdrom_capacity(const char *device_path);

/**
 * @brief 获取CDROM已使用的容量
 * 
 * @param device_path CDROM设备路径，例如 /dev/sr0
 * @return long 获取CDROM已使用的容量
 * @since 3.0.1.0
 */
unsigned long kdk_udf_get_cdrom_used_capacity(const char *device_path);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example kysdk-system/src/disk/test/getdiskinfo.c
 * \example kysdk-system/src/disk/test/getdisklist.c
 * 
 */
