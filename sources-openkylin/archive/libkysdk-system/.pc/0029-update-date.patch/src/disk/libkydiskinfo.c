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

#include "libkydiskinfo.h"
#include "diskmarcos.h"
#include <libkylog.h>
#include <kerr.h>
#include <kysdk/kysdk-base/sdkmarcos.h>
#include <blkid/blkid.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <libmount/libmount.h>
#include <unistd.h>
#include <libudev.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include "hd.h"
#include <dbus-1.0/dbus/dbus.h>
#include <sys/time.h>
#include <sys/statvfs.h> 
#include <parted/parted.h>
#include <libfdisk/libfdisk.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/hdreg.h>
#include <json-c/json.h>
#include <cjson/cJSON.h>
#include <libgen.h>

#define PATH_MAX_DISK 1024

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

// disk sector size shift
#define SECTOR_SHIFT 9
// sector size (512 bytes)
#define SECTOR_SIZE (1 << SECTOR_SHIFT)

typedef struct _kdk_disk_info{
    char *name;     // 绝对路径
    unsigned long long sectors_num;   // 扇区数量
    unsigned int sector_size;   // 每个扇区的字节数
    float total_size_MiB;  // 磁盘容量，MiB为单位
    unsigned int partition_nums; // 该磁盘/分区下的子分区数量
    kdk_disk_format format; // 格式化类型

}kdk_disk_info;

static int get_disk_partitions_info(kdk_disk_info *info);
static int kdk_get_disk_size(kdk_disk_info *info);
static kdk_disk_info* kdk_get_disk_info(const char *diskname);
void kdk_free_disk_info(kdk_disk_info *disk);

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
}

// 解析出磁盘设备路径，如从 "/dev/nvme0n1p1" 得到 "/dev/nvme0n1"
static int get_disk_path(const char *partition, char *disk_path, size_t len) {
    if (!partition || !disk_path || len == 0)
        return -1;

    // 处理常见命名规则：/dev/sda1 -> /dev/sda，/dev/nvme0n1p1 -> /dev/nvme0n1
    const char *base = strrchr(partition, '/');
    if (!base) return -1;

    // 检查是否已经是磁盘设备（非分区）
    int is_already_disk = 0;
    
    // NVMe磁盘设备判断（如/dev/nvme0n1）
    if (strstr(base, "nvme") != NULL) {
        // 如果路径中没有'p'且结尾是数字，说明已经是磁盘设备
        if (strchr(base, 'p') == NULL && isdigit(base[strnlen(base, STRNLEN_MAX)-1])) {
            is_already_disk = 1;
        }
    }
    // 普通磁盘设备判断（如/dev/sda）
    else {
        // 如果路径最后不是数字，说明已经是磁盘设备
        if (!isdigit(base[strnlen(base, STRNLEN_MAX)-1])) {
            is_already_disk = 1;
        }
    }

    if (is_already_disk) {
        strncpy(disk_path, partition, len);
        disk_path[len-1] = '\0';
        return 0;
    }

    // 处理分区路径情况
    if (strstr(base, "nvme") && strchr(base, 'p')) {
        // /dev/nvme0n1p1 -> 去掉最后一个'p'及其后内容
        strncpy(disk_path, partition, len);
        char *p = strrchr(disk_path, 'p');
        if (p) *p = '\0';
    } else {
        // 普通磁盘 /dev/sda1 -> /dev/sda
        strncpy(disk_path, partition, len);
        disk_path[len-1] = '\0';
        size_t i = strnlen(disk_path, STRNLEN_MAX);
        while (i > 0 && isdigit(disk_path[i-1])) {
            disk_path[i-1] = '\0';
            i--;
        }
    }

    return 0;
}

static int get_disk_fs(kdk_diskinfo *di)
{
    return 0;
}

static inline long test_valid_offset(int fd, off_t offset)
{
    char ch;

	if (lseek (fd, offset, 0) < 0)
		return 0;
	if (read (fd, &ch, 1) < 1)
		return 0;
	return 1;
}

static off_t find_disk_size(int fd)
{
    uintmax_t high, low = 0;

	for (high = 1024; test_valid_offset (fd, high); ) {
		if (high == UINTMAX_MAX)
			return -1;

		low = high;

		if (high >= UINTMAX_MAX/2)
			high = UINTMAX_MAX;
		else
			high *= 2;
	}

	while (low < high - 1)
	{
		uintmax_t mid = (low + high) / 2;

		if (test_valid_offset (fd, mid))
			low = mid;
		else
			high = mid;
	}
	test_valid_offset (fd, 0);
	return (low + 1);
}

static int get_disk_size(int fd, unsigned long long *bytes)
{
#ifdef BLKGETSIZE64
	if (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		return 0;
#elif defined BLKGETSIZE
	{
		unsigned long size;

		if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((unsigned long long)size << 9);
			return 0;
		}
	}
#endif // BLKGETSIZE
    {
		struct stat st;

		if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
			*bytes = st.st_size;
			return 0;
		}
		if (!S_ISBLK(st.st_mode))
			return -1;
	}
    *bytes   = find_disk_size(fd);
    return 0;
}

static int get_disk_space(int fd, kdk_diskinfo *di)
{
    blkid_loff_t size = -1; 
    size = blkid_get_dev_size(fd);
    if (size == (blkid_loff_t)-1) 
    {
        klog_err("无法获取磁盘总容量\n");
        return -1;
    }
    blkid_probe pr = blkid_new_probe_from_filename(di->name);
    if (! pr)
    {
        klog_err("无法获取%s分区表信息\n", di->name);
        return -1;
    }
    unsigned long sector_size = 0;
    sector_size = blkid_probe_get_sectorsize(pr);
    if (sector_size == 0) 
    {
        klog_err("无法获取设每个扇区的字节数\n");
        blkid_free_probe(pr);
        return -1;
    }
    di->total_size_MiB = (unsigned long long)size / 1024.00 /1024.00;
    
    di->sector_size = sector_size;
    di->sectors_num = (unsigned long long)size / sector_size;
    blkid_free_probe(pr);
    return 0;
}

static int get_disk_identifier(int fd, kdk_diskinfo *di)
{
    __uint16_t hd[256];

    if (ioctl(fd, KDK_HDIO_GET_IDENTITY, hd))
    {
        if (errno == -ENOMSG)
        {
            klog_warning("该磁盘[%s]未写入标识信息\n", di->name);
            return -1;
        }
        klog_warning("ioctl 执行失败：%s\n", strerror(errno));
        return -1;
    }
    
    char *model = strndup((char*)&hd[27], 40);
    char *fwrev = strndup((char*)&hd[23], 8);   // 固件版本
    char *serno = strndup((char*)&hd[10], 20);

    di->serial  = malloc(strnlen(serno, STRNLEN_MAX) + 1);
    di->model   = malloc(strnlen(model, STRNLEN_MAX) + 1);
    di->fwrev = malloc(strnlen(fwrev, STRNLEN_MAX) + 1);
    if (! di->serial || ! di->model || !di->fwrev)
    {
        klog_emerg("内存分配失败：%s\n", strerror(errno));
        SAFE_FREE(model);
        SAFE_FREE(serno);
        SAFE_FREE(fwrev);
        return KDK_ENOMEM;
    }

    strncpy(di->serial, serno, strnlen(serno, STRNLEN_MAX) + 1);
    strncpy(di->model, model, strnlen(model, STRNLEN_MAX) + 1);
    strncpy(di->fwrev, fwrev, strnlen(fwrev, STRNLEN_MAX) + 1);

    strstripspace(di->serial);
    strstripspace(di->model);
    strstripspace(di->fwrev);

    SAFE_FREE(model);
    SAFE_FREE(serno);
    SAFE_FREE(fwrev);
    return 0;
}

static int get_disk_partitions(kdk_diskinfo *di)
{
    blkid_probe pr = blkid_new_probe_from_filename(di->name);
    if (! pr)
    {
        klog_err("无法获取%s分区表信息\n", di->name);
        di->partition_nums  = 0;
        di->format  = DISK_FMT_NONE;
        goto out;
    }

    blkid_partlist ls;

    ls = blkid_probe_get_partitions(pr);
    if (! ls)
    {
        di->partition_nums  = 0;
    }
    else
    {
        di->partition_nums = blkid_partlist_numof_partitions(ls);
    }

    if (di->partition_nums < 0)
        goto out;
    
    if (di->partition_nums == 0)    // 子分区，只有子分区才有type、uuid、label属性
    {
        const char *type, *uuid;
        blkid_do_probe(pr);
        blkid_probe_lookup_value(pr, "UUID", &uuid, NULL);
        blkid_probe_lookup_value(pr, "TYPE", &type, NULL);

        if(type)
        {
            if (strcmp(type, "ext4") == 0)
                di->format   = DISK_FMT_EXT4;
            else if (strcmp(type, "fat32") == 0)
                di->format   = DISK_FMT_FAT32;
            else if (strcmp(type, "ntfs") == 0)
                di->format   = DISK_FMT_NTFS;
            else if (strcmp(type, "ext3") == 0)
                di->format   = DISK_FMT_EXT3;
            else if (strcmp(type, "vfat") == 0)
                di->format   = DISK_FMT_VFAT;
            else if (strcmp(type, "ext2") == 0)
                di->format   = DISK_FMT_EXT2;
            else if (strcmp(type, "fat16") == 0)
                di->format   = DISK_FMT_FAT16;
            else if (strcmp(type, "swap") == 0)
                di->format   = DISK_FMT_SWAP;
            else
                di->format   = DISK_FMT_UNKNOW;
        }
        else
            di->format   = DISK_FMT_UNKNOW;
        
        di->uuid    = malloc(strnlen(uuid, STRNLEN_MAX) + 1);
        if (! di->uuid)
        {
            di->uuid    = malloc(strnlen(uuid, STRNLEN_MAX) + 1);
            if (! di->uuid)
            {
                klog_emerg("内存分配失败：%s\n", strerror(errno));
                blkid_free_probe(pr);
                return KDK_ENOMEM;
            }
            strncpy(di->uuid, uuid, strnlen(uuid, STRNLEN_MAX) + 1);
        }    
    }
    else
        di->format   = DISK_FMT_NONE;

out:
    blkid_free_probe(pr);
    return 0;
}

static int get_disk_type(kdk_diskinfo *di)
{
    // 获取接口类型

    // 获取磁盘类型
    // if (isdigit(di->name[strlen(di->name)]))    // 分区盘
    // {
    //     di->disk_type   = DISK_TYPE_NONE;
    //     goto out;
    // }
    // else
    // {
#ifdef  __linux__
        const char *p_name = di->name;
        p_name += strlastof(p_name, '/');
        char syspath[PATH_MAX_DISK];
        snprintf(syspath, sizeof(syspath), "/sys/block/%s/queue/rotational", p_name + 1);
        // if(0 != access(syspath,0))
        // {
        //     di->disk_type = DISK_TYPE_NONE;
        //     return 0;
        // }
        FILE *tfd = fopen(syspath, "r");
        if (! tfd)
        {
            klog_err("rotational文件打开失败：%s\n", strerror(errno));
            // printf("Unable to open rotational. ");
            di->disk_type = DISK_TYPE_NONE;
            return 0;
        }
        char buf[2] = "\0";
        fgets(buf, sizeof(buf), tfd);
        if (strstr(buf,"0"))
            di->disk_type   = DISK_TYPE_SSD;
        else
            di->disk_type   = DISK_TYPE_HDD;
#endif  // __linux__

        if (di->disk_type == DISK_TYPE_SSD)
            goto out;
    // }

    // 获取转速信息（固态除外）
    
    

out:
    fclose(tfd);
    return 0;
}

// 简单的正则匹配函数（简化版，用于提取设备名）
char* regexp_label(const char* line, const char* pattern)
{
    // 这里简化实现，实际应该使用正则表达式库
    // 对于特定模式进行简单匹配
    
    // 模式1: 匹配没有数字结尾的设备名 (^([^0-9]+)$)
    if (strcmp(pattern, "^([^0-9]+)$") == 0) {
        int has_digit = 0;
        for (int i = 0; line[i] != '\0'; i++) {
            if (isdigit(line[i])) {
                has_digit = 1;
                break;
            }
        }
        if (!has_digit && strnlen(line, STRNLEN_MAX) > 0) {
            return strdup(line);
        }
    }
    
    // 模式2: 匹配md设备 (^(md[0-9]+)$)
    else if (strcmp(pattern, "^(md[0-9]+)$") == 0) {
        if (strncmp(line, "md", 2) == 0) {
            int all_digits = 1;
            for (int i = 2; line[i] != '\0'; i++) {
                if (!isdigit(line[i])) {
                    all_digits = 0;
                    break;
                }
            }
            if (all_digits && strnlen(line, STRNLEN_MAX) > 2) {
                return strdup(line);
            }
        }
    }
    
    // 模式3: 匹配mmcblk设备 (^(mmcblk[0-9]+)$)
    else if (strcmp(pattern, "^(mmcblk[0-9]+)$") == 0) {
        if (strncmp(line, "mmcblk", 6) == 0) {
            int all_digits = 1;
            for (int i = 6; line[i] != '\0'; i++) {
                if (!isdigit(line[i])) {
                    all_digits = 0;
                    break;
                }
            }
            if (all_digits && strnlen(line, STRNLEN_MAX) > 6) {
                return strdup(line);
            }
        }
    }
    
    // 模式4: 匹配nvme设备 (^(nvme[0-9]+n[0-9]+)$)
    else if (strcmp(pattern, "^(nvme[0-9]+n[0-9]+)$") == 0) {
        if (strncmp(line, "nvme", 4) == 0) {
            // 简化检查：包含'n'且长度足够
            if (strchr(line, 'n') != NULL && strnlen(line, STRNLEN_MAX) > 5) {
                return strdup(line);
            }
        }
    }
    
    // 模式5: 匹配控制器设备 (^([a-z]+/c[0-9]+d[0-9]+)$)
    else if (strcmp(pattern, "^([a-z]+/c[0-9]+d[0-9]+)$") == 0) {
        if (strchr(line, '/') != NULL && 
            strchr(line, 'c') != NULL && 
            strchr(line, 'd') != NULL) {
            return strdup(line);
        }
    }


    // 模式2: 匹配普通分区设备 (^([a-z]+[0-9]+)$) - 新增
    else if (strcmp(pattern, "^([a-z]+[0-9]+)$") == 0) {
        // 检查是否以小写字母开头，后跟数字结尾
        if (strnlen(line, STRNLEN_MAX) < 2) {
            return NULL;
        }
        
        // 检查第一个字符是否是小写字母
        if (!islower(line[0])) {
            return NULL;
        }
        
        // 检查是否包含数字
        int has_digit = 0;
        int has_alpha_after_digit = 0;
        int digit_started = 0;
        
        for (int i = 0; line[i] != '\0'; i++) {
            if (isdigit(line[i])) {
                has_digit = 1;
                digit_started = 1;
            } else if (digit_started && isalpha(line[i])) {
                // 如果数字后面还有字母，不符合模式（如 sda1a）
                has_alpha_after_digit = 1;
                break;
            }
        }
        
        if (has_digit && !has_alpha_after_digit) {
            return strdup(line);
        }
    }
    
    return NULL;
}

char** kdk_get_disklist()
{
    char **res = NULL;
    size_t counts = 0;
    int has_devices_from_proc = 0;
    FILE* proc_partitions = fopen("/proc/partitions", "r");
    if (proc_partitions) {
        char line[1024] = {0};
        
        while (fgets(line, sizeof(line), proc_partitions)) {
            char name[256] = {0};
            unsigned long maj = 0, min = 0;
            
            if (sscanf(line, "%lu %lu %*lu %255s", &maj, &min, name) == 3) {
                if (maj == 0 || strnlen(name, STRNLEN_MAX) == 0 || strcmp(name, "name") == 0 || strstr(name, "loop") || strstr(name, "sr")) {
                    continue;
                }
                
                char* device = NULL;
                const char* patterns[] = {
                    "^([^0-9]+)$",           // 普通磁盘设备
                    "^([a-z]+[0-9]+)$",      // 分区设备 sda1, sda2 (可选)
                    "^(md[0-9]+)$",          // MD RAID设备
                    "^(mmcblk[0-9]+)$",      // MMC设备
                    "^(nvme[0-9]+n[0-9]+)$", // NVMe设备
                    "^([a-z]+/c[0-9]+d[0-9]+)$" // 控制器设备
                };
                
                for (int i = 0; i < 5 && !device; i++) {
                    device = regexp_label(name, patterns[i]);
                }
                
                if (device) {
                    char full_device_path[256 + 6] = {0};
                    snprintf(full_device_path, sizeof(full_device_path), "/dev/%s", device);
                    
                    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
                    if (!tmp) {
                        free(device);
                        goto err_out;
                    }
                    res = tmp;
                    
                    res[counts] = strdup(full_device_path);
                    if (!res[counts]) {
                        free(device);
                        goto err_out;
                    }
                    
                    klog_info("Found device: %s\n", res[counts]);
                    counts ++;
                    has_devices_from_proc = 1;
                    free(device);
                }
            }
        }
        fclose(proc_partitions);
    } else {
        klog_err("Failed to open /proc/partitions\n");
    }

    if (!has_devices_from_proc) {
#ifdef  __linux__
#define DISKDIR "/dev/disk/by-path/"
        DIR *dir = opendir(DISKDIR);
        ASSERT_NOT_NULL(dir, NULL);
        struct dirent *dp;
        while ((dp = readdir(dir))) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }
            
            char linkpath[PATH_MAX] = {0};
            char targetpath[PATH_MAX] = {0};
            
            snprintf(linkpath, sizeof(linkpath), "%s%s", DISKDIR, dp->d_name);
            
            ssize_t len = readlink(linkpath, targetpath, sizeof(targetpath) - 1);
            if (len < 0) {
                continue;
            }
            targetpath[len] = '\0';

            // 调整出绝对路径
            char *pstr = targetpath;
            char prevpath[32];
            strncpy(prevpath, DISKDIR, sizeof(prevpath) - 1);
            char *ppp = &prevpath[strnlen(prevpath, STRNLEN_MAX)];
            ppp --;
            *ppp = 0;
            while (!strncmp(pstr, "..", 2))
            {
                while (*ppp != '/')
                    ppp --;
                *ppp = 0;
                pstr += 3;
            }
            
            char absolute_path[PATH_MAX] = {0};
            if (pstr) {
                snprintf(absolute_path, sizeof(absolute_path), "/dev/%s", pstr);
            } else {
                strncpy(absolute_path, pstr, sizeof(absolute_path) - 1);
            }
            
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) {
                closedir(dir);
                goto err_out;
            }
            res = tmp;
            
            res[counts] = strdup(absolute_path);
            if (!res[counts]) {
                closedir(dir);
                goto err_out;
            }
            
            klog_info("Found device from by-path: %s\n", res[counts]);
            counts++;
        }
        closedir(dir);
#elif __win32
#endif
    }

    if (counts > 0) {
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) {
            goto err_out;
        }
        res = tmp;
        res[counts] = NULL;
    } else {
        res = malloc(sizeof(char *));
        if (res) {
            res[0] = NULL;
        }
    }
    
    return res;

err_out:
    for (size_t i = 0; i < counts; i++) {
        free(res[i]);
    }
    free(res);
    return NULL;
}

void kdk_free_disklist(char** disklist)
{
    if (!disklist)
        return ;
    
    for (int i = 0; disklist[i]; i ++)
    {
        free(disklist[i]);
    }

    free(disklist);
}

kdk_diskinfo *_kdk_get_diskinfo(const char *diskname)
{
    if (!diskname)
    {
        klog_err("diskname is NULL\n");
        return NULL;
    }

    int fd = -1;
    kdk_diskinfo *res = NULL;
    struct udev *udev = NULL;
    struct udev_device *dev_block = NULL;
    struct udev_device *dev_nvme = NULL;
    struct udev_device *dev = NULL;
    FILE *tfd = NULL;

    fd = open(diskname, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        klog_err("%s文件打开失败：%s\n", diskname, strerror(errno));
        return NULL;
    }

    res = (kdk_diskinfo *)calloc(1, sizeof(kdk_diskinfo));
    if (!res)
    {
        klog_err("内存分配失败\n");
        goto cleanup;
    }

    res->name = strdup(diskname);

    if (get_disk_identifier(fd, res) != 0)
    {
        udev = udev_new();
        if (!udev)
        {
            klog_err("初始化udev失败\n");
            goto cleanup;
        }

        int pos = strlastof(diskname, '/');
        if (pos <= 0)
        {
            pos = 0;
        }
        const char *dn = diskname + pos + 1;

        dev_block = udev_device_new_from_subsystem_sysname(udev, "block", dn);
        dev_nvme = udev_device_new_from_subsystem_sysname(udev, "nvme", dn);

        if (dev_block)
        {
            dev = dev_block;
        }
        else if (dev_nvme)
        {
            dev = dev_nvme;
        }
        else
        {
            klog_err("未找到匹配的磁盘信息: %s\n", diskname);
            goto cleanup;
        }

        const char *model = NULL;
        const char *serial_val = NULL;

        struct udev_device *dev_P = udev_device_get_parent(dev);
        struct udev_device *dev_PP = dev_P ? udev_device_get_parent(dev_P) : NULL;

        if (udev_device_get_sysattr_value(dev, "model"))
        {
            model = udev_device_get_sysattr_value(dev, "model");
            serial_val = udev_device_get_sysattr_value(dev, "serial");
        }
        else if (dev_P && udev_device_get_sysattr_value(dev_P, "model"))
        {
            model = udev_device_get_sysattr_value(dev_P, "model");
            serial_val = udev_device_get_sysattr_value(dev_P, "serial");

            if (!serial_val && dev_PP)
            {
                struct udev_device *serial_dev = udev_device_get_parent(udev_device_get_parent(dev_PP));
                if (serial_dev)
                {
                    serial_val = udev_device_get_sysattr_value(serial_dev, "serial");
                    udev_device_unref(serial_dev);
                }
            }
        }
        else if (dev_PP && udev_device_get_sysattr_value(dev_PP, "model"))
        {
            model = udev_device_get_sysattr_value(dev_PP, "model");
            serial_val = udev_device_get_sysattr_value(dev_PP, "serial");

            if (!serial_val)
            {
                struct udev_device *serial_dev = udev_device_get_parent(udev_device_get_parent(udev_device_get_parent(dev_PP)));
                if (serial_dev)
                {
                    serial_val = udev_device_get_sysattr_value(serial_dev, "serial");
                    udev_device_unref(serial_dev);
                }
            }
        }
        else
        {
            klog_err("未找到磁盘模型信息: %s\n", diskname);
            goto cleanup;
        }

        if (model)
        {
            res->model = strdup(model);
            if (!res->model)
            {
                klog_err("内存分配失败\n");
                goto cleanup;
            }
        }
        else
        {
            res->model = strdup("Unknown");
        }

        if (serial_val)
        {
            res->serial = strdup(serial_val);
            if (!res->serial)
            {
                klog_err("内存分配失败\n");
                goto cleanup;
            }
        }
        else
        {
            res->serial = strdup("None");
        }

        char disk_path[256] = {0};
        if (get_disk_path(res->name, disk_path, sizeof(disk_path)) != 0)
        {
            klog_err("解析磁盘路径失败: %s\n", res->name);
            goto cleanup;
        }

        klog_info("res->name = %s, disk_path = %s\n", res->name, disk_path);

        const char *p_name = strrchr(disk_path, '/');
        if (!p_name)
        {
            p_name = disk_path;
        }
        else
        {
            p_name++; // 跳过 '/'
        }

        char syspath[PATH_MAX];
        int ret = snprintf(syspath, sizeof(syspath), "/sys/block/%s/device/firmware_rev", p_name);
        if (ret < 0 || ret >= (int)sizeof(syspath))
        {
            klog_err("固件版本路径过长: %s\n", p_name);
            res->fwrev = strdup("None");
        }
        else
        {
            tfd = fopen(syspath, "r");
            if (!tfd)
            {
                res->fwrev = strdup("None");
            }
            else
            {
                char buf[FWREV_SIZE] = {0};
                if (fgets(buf, sizeof(buf), tfd))
                {
                    strstripspace(buf);
                    res->fwrev = strndup(buf, 20);
                }
                else
                {
                    res->fwrev = strdup("None");
                }
                fclose(tfd);
                tfd = NULL;
            }
        }
    }

    // 获取其他磁盘信息
    if (get_disk_space(fd, res) != 0)
    {
        klog_err("获取磁盘空间信息失败: %s\n", diskname);
        goto cleanup;
    }

    if (get_disk_partitions(res) != 0)
    {
        klog_err("获取磁盘分区信息失败: %s\n", diskname);
        goto cleanup;
    }

    if (get_disk_type(res) != 0)
    {
        klog_err("获取磁盘类型失败: %s\n", diskname);
        goto cleanup;
    }

    // 成功，清理资源并返回
    udev_unref(udev);
    close(fd);
    return res;

cleanup:
    // 统一清理资源
    if (res)
    {
        kdk_free_diskinfo(res);
    }
    if (udev)
    {
        udev_unref(udev);
    }
    if (tfd)
    {
        fclose(tfd);
    }
    if (fd > 0)
    {
        close(fd);
    }
    return NULL;
}

static kdk_diskinfo *getDiskInfo(char *diskname)
{
    char *ret = NULL;
    DBusConnection *conn = NULL;
    DBusError err;
    kdk_diskinfo *diskinfo = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        klog_err("Failed to get DBus connection\n");
        goto fallback;
    }

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        goto fallback;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getDiskInfo");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto cleanup;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto cleanup;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto cleanup;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto cleanup;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
        info_msg = NULL; // 设置为NULL以避免重复释放
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto cleanup;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
        sendMsgPending = NULL; // 设置为NULL以避免重复释放
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        goto cleanup;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        klog_err("kdk : d-bus reply message fail !\n");
        goto cleanup;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
        replyMsg = NULL; // 设置为NULL以避免重复释放
    }
    
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    conn = NULL; // 设置为NULL以避免重复释放

    if(ret && strlen(ret) != 0)
    {
        cJSON *root;
        root = cJSON_Parse(ret);
        if(!root)
        {
            klog_err("JSON Parse error\n");
            goto fallback;
        }

        diskinfo = (kdk_diskinfo*)calloc(1, sizeof(kdk_diskinfo));
        if (!diskinfo) {
            klog_err("Error: Memory allocation failed\n");
            cJSON_Delete(root);
            goto fallback;
        }

        diskinfo->name = strdup(diskname);
        if (!diskinfo->name) {
            klog_err("Error: Disk name allocation failed\n");
            free(diskinfo);
            cJSON_Delete(root);
            goto fallback;
        }

        cJSON *sector_size = cJSON_GetObjectItem(root, "SectorSize");
        if(cJSON_IsNumber(sector_size))
        {
            diskinfo->sector_size = sector_size->valueint;
        }
        
        cJSON *total_size_MiB = cJSON_GetObjectItem(root, "TotalSizeMiB");
        if(cJSON_IsNumber(total_size_MiB))
            diskinfo->total_size_MiB = total_size_MiB->valuedouble;

        cJSON *model = cJSON_GetObjectItem(root, "Model");
        if(cJSON_IsString(model) && model->valuestring)
            diskinfo->model = strdup(model->valuestring);

        cJSON *serial = cJSON_GetObjectItem(root, "Serial");
        if(cJSON_IsString(serial) && serial->valuestring)
            diskinfo->serial = strdup(serial->valuestring);

        cJSON *partition_nums = cJSON_GetObjectItem(root, "PartitionNums");
        if(cJSON_IsNumber(partition_nums))
            diskinfo->partition_nums = partition_nums->valueint;

        cJSON *type = cJSON_GetObjectItem(root, "Type");
        if(cJSON_IsNumber(type))
            diskinfo->disk_type = type->valueint;

        cJSON *version = cJSON_GetObjectItem(root, "Version");
        if(cJSON_IsString(version) && version->valuestring)
            diskinfo->fwrev = strdup(version->valuestring);

        cJSON *sector_num = cJSON_GetObjectItem(root, "SectorNum");
        if(cJSON_IsNumber(sector_num))
            diskinfo->sectors_num = (unsigned long long)sector_num->valuedouble;

        cJSON *format = cJSON_GetObjectItem(root, "Format");
        if(cJSON_IsNumber(format))
            diskinfo->format = format->valueint;

        cJSON_Delete(root);
    }
    else
    {
        goto fallback;
    }

    return diskinfo;

cleanup:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

fallback:
    if (diskinfo) {
        kdk_free_diskinfo(diskinfo);
        diskinfo = NULL;
    }
    diskinfo = _kdk_get_diskinfo(diskname);
    if(diskinfo)
    {
        return diskinfo;
    }

    return NULL;
}

kdk_diskinfo *kdk_get_diskinfo(const char *diskname)
{
    if(!diskname)
        return NULL;
    kdk_diskinfo *diskinfo = NULL;
    
    diskinfo = getDiskInfo(diskname);
    if(!diskinfo)
    {
        return NULL;
    }
    return diskinfo;
}

extern char** kdk_get_hard_disk()
{
    char **res = NULL;
    size_t counts = 0;

#ifdef  __linux__
#define DISKDIR "/sys/block/"
    DIR *dir = opendir(DISKDIR);
    ASSERT_NOT_NULL(dir, NULL);
    struct dirent *dp;
    while ((dp = readdir(dir)))
    {
        if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)) || strstr(dp->d_name, "loop") || strstr(dp->d_name, "sr"))
            continue;
        
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) {
            goto err_out;
        }
        res = tmp;
        res[counts] = malloc(PATH_MAX_DISK * sizeof(char) + 1 + PATH_MAX_DISK);
        res[counts][PATH_MAX_DISK + 1] = 0;

        // 调整出绝对路径
        
        strncpy(res[counts], "/dev", PATH_MAX_DISK * sizeof(char) + PATH_MAX_DISK);
        strncat(res[counts], "/", (PATH_MAX_DISK * sizeof(char) + PATH_MAX_DISK) - strnlen(res[counts], STRNLEN_MAX));
        strncat(res[counts], dp->d_name, (PATH_MAX_DISK * sizeof(char) + PATH_MAX_DISK) - strnlen(res[counts], STRNLEN_MAX));

        counts ++;
    }
    closedir(dir);
#elif __win32
#endif

    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    // FixMe: Memory Leak
    // ASSERT_NOT_NULL(tmp, NULL);
    if(!tmp)
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    return res;
err_out:
    closedir(dir);
    while (counts)
    {
        free(res[counts - 1]);
        counts--;
    }
    free(res);
    return NULL;
}

char* _kdk_get_hard_disk_size(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_size = (char *)malloc(128*sizeof(char));
    if (!hard_size)
    {
        return NULL;
    }
    memset(hard_size, 0, 128);
    kdk_disk_info *info = kdk_get_disk_info(hardname);
    if (!info)
    {
        free(hard_size);
        return NULL;
    }

    snprintf(hard_size, 128, "%f",info->total_size_MiB);
    kdk_free_disk_info(info);
    if(strnlen(hard_size, STRNLEN_MAX) == 0)
    {
        free(hard_size);
        return NULL;
    }
    return hard_size;
}

char* kdk_get_hard_disk_size(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_size = (char *)malloc(128*sizeof(char));
    if (!hard_size)
    {
        return NULL;
    }
    memset(hard_size, 0, 128);
    kdk_disk_info *info = kdk_get_disk_info(hardname);
    if (!info)
    {
        free(hard_size);
        return NULL;
    }

    snprintf(hard_size, 128, "%f",info->total_size_MiB);
    kdk_free_disk_info(info);
    if(strnlen(hard_size, STRNLEN_MAX) == 0)
    {
        free(hard_size);
        return NULL;
    }
    return hard_size;
}

static int is_block_device(const char *path) {
    struct stat st;
    
    if (stat(path, &st) == -1) {
        klog_err("stat failed");
        return 0;  // 不是块设备或出错
    }
    
    return S_ISBLK(st.st_mode);
}

static char* get_disk_fwrev(char *hardname) 
{
	const char *name = NULL;
	char *res = NULL;
    struct udev *udev = udev_new();
    if (!udev)
    {
        return NULL;
    }
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) 
    {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);

        // 过滤掉分区和虚拟设备
        const char *devtype = udev_device_get_devtype(dev);
        if (!devtype || strcmp(devtype, "disk") != 0) 
        {
            udev_device_unref(dev);
            continue;
        }

        name = udev_device_get_sysname(dev);
        const char *fwrev = udev_device_get_property_value(dev, "ID_REVISION");

        if (fwrev && strcmp(name, hardname + 5) == 0 ) 
        {
			res = strdup(fwrev);
			break;
        }
		else
			continue;

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
	return res;
}

char* _kdk_get_hard_fwrev(const char *hardname)
{
    klog_info("hardname = %s\n", hardname);
    if(!hardname)
    {
        return NULL;
    }

    int block = is_block_device(hardname);
    
    if((access(hardname, F_OK) == -1) || !block)
    {
        return NULL;
    }

    klog_info("hardname = %s, block = %d\n", hardname, block);
    
    char path[128] = "\0"; 
    char tmp[1024] = "\0";
    char fwrev[128] = "\0";
    char *hard_fwrev = (char *)malloc(128*sizeof(char));
    if (!hard_fwrev)
    {
        return NULL;
    }
    memset(hard_fwrev, 0, 128);

    kdk_diskinfo *sdainfo = _kdk_get_diskinfo(hardname);
    if (!sdainfo)
    {
        free(hard_fwrev);
        return NULL;
    }
    
    if(!strcmp(sdainfo->fwrev, "None"))
    {
        snprintf(path, sizeof(path), "smartctl -i %s", hardname);
        FILE *fp = popen(path, "r");
        if(!fp)
        {
            free(hard_fwrev);
            kdk_free_diskinfo(sdainfo);
            return NULL;
        }
        while (fgets(tmp, 1024, fp))
        {
            if (0 == strncmp(tmp, "Firmware Version:", 17))
            {
                sscanf(tmp, "%*s %*s %s", fwrev);
                break;
            }
        }
        pclose(fp);
        if(fwrev[0] != '\0')
        {
            strncpy(hard_fwrev, fwrev, 127);
        }
        else{
            kdk_free_diskinfo(sdainfo);
            char *res = NULL;
            res = get_disk_fwrev(hardname);
            if(res)
            {
                strncpy(hard_fwrev, res, 127);
                free(res);
                return hard_fwrev;
            }
            free(hard_fwrev);
            return NULL;
        }
    }else{
        strncpy(hard_fwrev, sdainfo->fwrev, 127);
        strstripspace(hard_fwrev);
    }
    kdk_free_diskinfo(sdainfo);
    return hard_fwrev;
}

char* kdk_get_hard_fwrev(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *fwrev = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
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
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getHardFwrev");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
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

    if(ret && strlen(ret) != 0)
    {
        fwrev = (char *)malloc(sizeof(char) * 128);
        if(!fwrev)
        {
            fwrev = NULL;
        }
        strncpy(fwrev, ret, 127);
    }

    return fwrev;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    ret = _kdk_get_hard_fwrev(hardname);
    if(ret != NULL)
    {
        fwrev = (char *)malloc(sizeof(char) * 128);
        if(!fwrev)
        {
            fwrev = NULL;
        }
        strncpy(fwrev, ret, 127);
        free(ret);
    }
    return fwrev;
}

char* _kdk_get_hard_type(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_type = (char *)malloc(128*sizeof(char));
    if (!hard_type)
    {
        return NULL;
    }
    
    kdk_diskinfo *sdainfo = _kdk_get_diskinfo(hardname);
    if (!sdainfo)
    {
        free(hard_type);
        return NULL;
    }
    if(sdainfo->disk_type == 0)
    {
        strncpy(hard_type, "机械", 127);
    }else{
        strncpy(hard_type, "固态", 127);
    }
    kdk_free_diskinfo(sdainfo);
    return hard_type;
}

char* kdk_get_hard_type(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *type = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
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
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getHardType");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
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

    if(ret && strlen(ret) != 0)
    {
        type = (char *)malloc(sizeof(char) * 128);
        if(!type)
        {
            type = NULL;
        }
        strncpy(type, ret, 127);
    }

    return type;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    ret = _kdk_get_hard_type(hardname);
    if(ret != NULL)
    {
        type = (char *)malloc(sizeof(char) * 128);
        if(!type)
        {
            type = NULL;
        }
        strncpy(type, ret, 127);
        free(ret);
    }

    return type;
}

static char* get_disk_models(char *hardname) 
{
	const char *name = NULL;
	const char *model = NULL;
	char *res = NULL;
    struct udev *udev = udev_new();
    if (!udev)
    {
        return NULL;
    }
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) 
    {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);

        // 过滤掉分区和虚拟设备
        const char *devtype = udev_device_get_devtype(dev);
        if (!devtype || strcmp(devtype, "disk") != 0) 
        {
            udev_device_unref(dev);
            continue;
        }

        name = udev_device_get_sysname(dev);
        const char *model = udev_device_get_property_value(dev, "ID_MODEL");

        if (model && strcmp(name, hardname + 5) == 0 ) 
        {
			res = strdup(model);
			break;
        }
		else
			continue;

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
	return res;
}

char* _kdk_get_hard_model(const char *hardname)
{
    klog_info("hardname = %s\n", hardname);
    if(!hardname)
    {
        return NULL;
    }

    int block = is_block_device(hardname);
    
    if((access(hardname, F_OK) == -1) || !block)
    {
        return NULL;
    }

    klog_info("hardname = %s, block = %d\n", hardname, block);
    
    char path[256] = "\0";
    char tmp[1024] = "\0";
    char *str = NULL;

    snprintf(path, sizeof(path), "smartctl -i %s", hardname);
    FILE *fd = popen(path, "r");
    if(!fd)
    {
        return NULL;
    }
    while (fgets(tmp, 1024, fd))
    {
        if (0 == strncmp(tmp, "Model Number:", 13))
        {
            int tmp_pos = strlastof(tmp, ':');
            if (tmp_pos <= 0)
                tmp_pos = 0;
            str = tmp + tmp_pos + 1;
            break;
        }
        else if(0 == strncmp(tmp, "Device Model:", 13))
        {
            int tmp_pos = strlastof(tmp, ':');
            if (tmp_pos <= 0)
                tmp_pos = 0;
            str = tmp + tmp_pos + 1;
            break;
        }
    }
    pclose(fd);
	char *hard_model = NULL;
    
    if (str)
    {
		hard_model = (char *)malloc(128*sizeof(char));
    	if (!hard_model)
    	{
        	return NULL;
    	}
        strncpy(hard_model, str, 127); 
        strstripspace(hard_model);
    }
    else{
		char *model = NULL;
		model = get_disk_models(hardname);
		if(model)
		{
			hard_model = (char *)malloc(128*sizeof(char));
    		if (!hard_model)
    		{
        		return NULL;
    		}
        	strncpy(hard_model, model, 127); 
        	strstripspace(hard_model);
            free(model);
		}
    }
    return hard_model;
}

char* kdk_get_hard_model(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *model = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
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
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getHardModel");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
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

    if(ret && strlen(ret) != 0)
    {
        model = (char *)malloc(sizeof(char) * 128);
        if(!model)
        {
            model = NULL;
        }
        strncpy(model, ret, 127);
    }

    return model;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    ret = _kdk_get_hard_model(hardname);
    if(ret != NULL)
    {
        model = (char *)malloc(sizeof(char) * 128);
        if(!model)
        {
            model = NULL;
        }
        strncpy(model, ret, 127); 
        free(ret);
    }

    return model;
}

char* _kdk_get_hard_serial(const char *hardname)
{
    klog_info("hardname = %s\n", hardname);
    if(!hardname)
    {
        return NULL;
    }

    int block = is_block_device(hardname);
    
    if((access(hardname, F_OK) == -1) || !block)
    {
        return NULL;
    }

    klog_info("hardname = %s, block = %d\n", hardname, block);

    char *hard_serial = (char *)malloc(256*sizeof(char));
    if (!hard_serial)
    {
        return NULL;
    }
    memset(hard_serial, 0, 256);
    char line[256] = "\0";
    char path[128] = "\0";
    char tmp[1024] = "\0";
    char serial[128] = "\0";
    char *str = NULL;
    char name[128] = "\0";
    char hard_name[128] = "\0";
    strncpy(hard_name, hardname, sizeof(hard_name) - 1);

    int pos = strlastof(hardname, '/');
    if (pos <= 0)
        pos = 0;
    char *dn = hardname + pos + 1;

    snprintf(path, sizeof(path), "/sys/class/block/%s/device/serial", dn);

    FILE *fp = fopen(path, "r");
    if(fp)
    {
        fgets(line, sizeof(line), fp);
        fclose(fp);
        if(line[0] != '\0')
        {
            strncpy(hard_serial, line, 255);
            strstripspace(hard_serial);
        }
    }
    else{
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "smartctl -i %s", hardname);
        FILE *fp = popen(path, "r");
        if(!fp)
        {
            goto err_out;
        }
        while (fgets(tmp, 1024, fp))
        {
            if (0 == strncmp(tmp, "Serial Number:", 14))
            {
                int tmp_pos = strlastof(tmp, ':');
                if (tmp_pos <= 0)
                    tmp_pos = 0;
                str = tmp + tmp_pos + 1;
                break;
            }
        }
        pclose(fp);
        if (str)
        {
            strncpy(hard_serial, str, 255);
            strstripspace(hard_serial);
        }
        else{
            hd_data_t *hd_data;
            hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
            if (!hd_data)
            {
                goto err_out;
            }

            hd_data->progress = NULL;
            hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);
            int i = 0;
            hd_t *hd, *hd0;

            hd0 = hd_list(hd_data, hw_disk, 1, NULL);
            if(!hd0)
            {
                free(hd_data);
                goto err_out;
            }
            
            for (hd = hd0; hd; hd = hd->next)
            {
                snprintf(name, sizeof(name), "%s", hd->unix_dev_name);
                if(strcmp(name, hard_name) == 0)
                {
                    if(hd->serial)
                    {
                        strncpy(hard_serial, hd->serial, 255);
                    }
                    else{
                        hd_free_hd_data(hd_data);
                        free(hd_data);
                        hd_free_hd_list(hd0);
                        goto err_out;
                    }
                }
            }
            hd_free_hd_data(hd_data);
            free(hd_data);
            hd_free_hd_list(hd0);
        }
    }
    if(strnlen(hard_serial, STRNLEN_MAX) == 0)
    {
        goto err_out;
    }
    return hard_serial;
err_out:
    free(hard_serial);
    return NULL;
}

char* kdk_get_hard_serial(const char *hardname)
{
    char *ret = NULL;
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
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getHardSerial");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
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
    if(ret && strlen(ret) != 0)
    {
        serial = (char *)malloc(sizeof(char) * 128);
        if(!serial)
        {
            serial = NULL;
        }
        strncpy(serial, ret, 127);
    }

    return serial;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    ret = _kdk_get_hard_serial(hardname);
    if(ret != NULL)
    {
        serial = (char *)malloc(sizeof(char) * 128);
        if(!serial)
        {
            serial = NULL;
        }
        strncpy(serial, ret, 127);
        free(ret);
    }

    return serial;
}

// 从 bus info 中提取 SCSI 编号，例如从 "scsi@6:0.0.0" 提取 "6"
int extract_scsi_number_from_bus_info(const char *bus_info) {
    if (!bus_info) return -1;
    
    // 查找 "scsi@" 模式
    const char *scsi_pos = strstr(bus_info, "scsi@");
    if (scsi_pos) {
        scsi_pos += 5; // 跳过 "scsi@"
        int num = 0;
        while (*scsi_pos >= '0' && *scsi_pos <= '9') {
            num = num * 10 + (*scsi_pos - '0');
            scsi_pos++;
        }
        return num;
    }
    return -1;
}

// 从 logical name 中提取 SCSI 编号，例如从 "scsi6" 提取 "6"
int extract_scsi_number_from_logical_name(const char *logical_name) {
    if (!logical_name) return -1;
    
    // 查找 "scsi" 后面的数字
    const char *scsi_pos = strstr(logical_name, "scsi");
    if (scsi_pos) {
        scsi_pos += 4; // 跳过 "scsi"
        int num = 0;
        while (*scsi_pos >= '0' && *scsi_pos <= '9') {
            num = num * 10 + (*scsi_pos - '0');
            scsi_pos++;
        }
        return num;
    }
    
    return -1;
}

// 通过对比 SCSI 编号获取 vendor
char* get_vendor_by_scsi_match(const char *hardname) {
    char *vendor = NULL;
    char buf[1024] = {0};
    char target_vendor[256] = {0};
    char bus_info[256] = {0};
    int target_scsi_num = -1;
    int found_in_storage = 0;
    
    FILE *pipe_disk = NULL;
    FILE *pipe_storage = NULL;
    
    pipe_disk = popen("lshw -class disk 2>/dev/null", "r");
    if (!pipe_disk) {
        return NULL;
    }
    
    int in_disk_section = 0;
    char current_vendor[256] = {0};
    
    while (fgets(buf, sizeof(buf), pipe_disk)) {
        strstripspace(buf);
        
        if (strstr(buf, "*-disk")) {
            in_disk_section = 1;
            memset(current_vendor, 0, sizeof(current_vendor));
            memset(bus_info, 0, sizeof(bus_info));
            continue;
        }
        
        if (in_disk_section) {
            // 修复：添加 NULL 检查
            char *vendor_pos = strstr(buf, "vendor:");
            if (vendor_pos) {
                char *p = vendor_pos + 7;
                while (*p == ' ' || *p == '\t') p++;
                
                // 修复：空字符串检查
                if (strlen(p) > 0) {
                    char *end = p + strlen(p) - 1;
                    while (end > p && (*end == '\n' || *end == '\r')) end--;
                    *(end + 1) = '\0';
                    strncpy(current_vendor, p, sizeof(current_vendor) - 1);
                    current_vendor[sizeof(current_vendor) - 1] = '\0';  // 修复：强制终止
                }
            }
            
            char *bus_pos = strstr(buf, "bus info:");
            if (bus_pos) {
                char *p = bus_pos + 9;
                while (*p == ' ' || *p == '\t') p++;
                
                if (strlen(p) > 0) {
                    char *end = p + strlen(p) - 1;
                    while (end > p && (*end == '\n' || *end == '\r')) end--;
                    *(end + 1) = '\0';
                    strncpy(bus_info, p, sizeof(bus_info) - 1);
                    bus_info[sizeof(bus_info) - 1] = '\0';
                    
                    int scsi_num = extract_scsi_number_from_bus_info(bus_info);
                    if (scsi_num >= 0) {
                        target_scsi_num = scsi_num;
                        strncpy(target_vendor, current_vendor, sizeof(target_vendor) - 1);
                        target_vendor[sizeof(target_vendor) - 1] = '\0';
                        strstripspace(target_vendor);
                    }
                }
            }

            char *logical_pos = strstr(buf, "logical name:");
            if (logical_pos) {
                char *p = logical_pos + 13;
                while (*p == ' ' || *p == '\t') p++;
                
                char logical_name[128] = {0};
                char *end = p;
                while (*end && *end != '\n' && *end != '\r' && *end != ' ') end++;
                size_t len = end - p;
                if (len > 0 && len < sizeof(logical_name)) {
                    strncpy(logical_name, p, len);
                    logical_name[len] = '\0';
                    
                    if(strstr(hardname, logical_name)) {
                        if (strlen(target_vendor) > 0) {
                            break;
                        }
                    }
                }
            }
        }
        
        if (in_disk_section && (strstr(buf, "*-medium") || (buf[0] == '*' && buf[1] == '-'))) {
            in_disk_section = 0;
        }
    }
    
    pclose(pipe_disk);
    pipe_disk = NULL;
    
    if (target_scsi_num < 0) {
        return NULL;
    }
    
    pipe_storage = popen("lshw -C storage 2>/dev/null", "r");
    if (!pipe_storage) {
        return NULL;
    }
    
    while (fgets(buf, sizeof(buf), pipe_storage)) {
        strstripspace(buf);

        char *vendor_pos = strstr(buf, "vendor:");
        if (vendor_pos) {
            char *p = vendor_pos + 7;
            while (*p == ' ' || *p == '\t') p++;
            
            if (strlen(p) > 0) {
                char *end = p + strlen(p) - 1;
                while (end > p && (*end == '\n' || *end == '\r')) end--;
                *(end + 1) = '\0';
                strncpy(target_vendor, p, sizeof(target_vendor) - 1);
                target_vendor[sizeof(target_vendor) - 1] = '\0';
                strstripspace(target_vendor);
            }
        }
        
        char *logical_pos = strstr(buf, "logical name:");
        if (logical_pos) {
            char *p = logical_pos + 13;
            while (*p == ' ' || *p == '\t') p++;
            
            char logical_name[128] = {0};
            char *end = p;
            while (*end && *end != '\n' && *end != '\r' && *end != ' ') end++;
            size_t len = end - p;
            if (len > 0 && len < sizeof(logical_name)) {
                strncpy(logical_name, p, len);
                logical_name[len] = '\0';
                
                int storage_scsi = extract_scsi_number_from_logical_name(logical_name);
                
                if (storage_scsi == target_scsi_num) {
                    found_in_storage = 1;
                    break;
                }
            }
        }
    }
    
    pclose(pipe_storage);
    pipe_storage = NULL;
    
    if (found_in_storage && strlen(target_vendor) > 0) {
        vendor = malloc(strlen(target_vendor) + 1);
        if (vendor) {
            strcpy(vendor, target_vendor);
            strstripspace(vendor);
        }
        return vendor;
    }
    
    return NULL;
}

// 原始方法 + 备用匹配
char* get_storage_vendor(const char *hardname) {
    char *vendor = NULL;
    char buf[1024] = {0};
    char target_vendor[256] = {0};
    char target_name[256] = {0};
    int in_target_device = 0;
    char search_name[128] = {0};
    
    FILE *pipeLine = popen("lshw -C storage 2>/dev/null", "r");
    if (!pipeLine) {
        return get_vendor_by_scsi_match(hardname);
    }
    
    // 先扫描，找到目标设备并记录 vendor
    while (fgets(buf, sizeof(buf), pipeLine)) {
        strstripspace(buf);
        
        // 缓存当前设备的 vendor
        if (strstr(buf, "vendor:")) {
            char *p = strstr(buf, "vendor:") + 7;
            while (*p == ' ' || *p == '\t') p++;
            char *end = p + strlen(p) - 1;
            while (end > p && (*end == '\n' || *end == '\r')) end--;
            *(end + 1) = '\0';
            strncpy(target_vendor, p, sizeof(target_vendor) - 1);
        }

        if (strstr(buf, "logical name:")) {
            char *p = strstr(buf, "logical name:");
            if (p) {
                p += 13;
                while (*p == ' ' || *p == '\t') p++;
                char *end = p;
                while (*end && *end != '\n' && *end != '\r' && *end != ' ') end++;
                size_t len = end - p;
                if (len > 0 && len < sizeof(target_name)) {
                    strncpy(target_name, p, len);
                    target_name[len] = '\0';
                }
            }
            in_target_device = 1;
        }
        
        // 检查是否是目标设备
        if (in_target_device && strstr(hardname, target_name)) {
            in_target_device = 0;
            
            // 优先使用 vendor
            if (strlen(target_vendor) > 0) {
                vendor = malloc(strlen(target_vendor) + 1);
                if (vendor) {
                    strcpy(vendor, target_vendor);
                    strstripspace(vendor);
                }
                pclose(pipeLine);
                return vendor;
            }
        }
    }
    pclose(pipeLine);
    
    // 原始方法失败，使用备用方法
    return get_vendor_by_scsi_match(hardname);
}

char* kdk_get_hard_vendor(const char *hardname) 
{
    if(!hardname) {
        return NULL;
    }
    
    char name[64] = "\0";
    char vendor[256] = "\0";
    
    hd_data_t *hd_data = NULL;
    hd_t *hd0 = NULL;
    char *hard_vendor = NULL;
    
    hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
    if (!hd_data) {
        return NULL;
    }
    
    hd_data->progress = NULL;
    hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);
    
    hd0 = hd_list(hd_data, hw_disk, 1, NULL);
    if(!hd0) {
        free(hd_data);
        return NULL;
    }
    
    for (hd_t *hd = hd0; hd; hd = hd->next) {
        snprintf(name, sizeof(name), "%s", hd->unix_dev_name ? hd->unix_dev_name : "");
        if(strcmp(name, hardname) == 0) {
            if(hd->vendor.name) {
                strncpy(vendor, hd->vendor.name, sizeof(vendor) - 1);
                vendor[sizeof(vendor) - 1] = '\0';
            }
            else if(hd->sub_vendor.name) {
                strncpy(vendor, hd->sub_vendor.name, sizeof(vendor) - 1);
                vendor[sizeof(vendor) - 1] = '\0';
            }
            break;  // 找到后立即退出循环
        }
    }
    
    hd_free_hd_list(hd0);
    hd_free_hd_data(hd_data);
    free(hd_data);
    
    if(strstr(vendor, "ST")) {
        strncpy(vendor, "Seagate", sizeof(vendor) - 1);
        vendor[sizeof(vendor) - 1] = '\0';
    }
    
    if(strnlen(vendor, sizeof(vendor)) == 0) {
        char *target_vendor = get_storage_vendor(hardname);
        if (target_vendor) {
            strncpy(vendor, target_vendor, sizeof(vendor) - 1);
            vendor[sizeof(vendor) - 1] = '\0';
            free(target_vendor);
        }
    }
    
    if(strnlen(vendor, sizeof(vendor)) == 0) {
        return NULL;
    }
    
    hard_vendor = (char *)malloc(128 * sizeof(char));
    if (!hard_vendor) {
        return NULL;  // 此时所有资源已释放，安全返回
    }
    
    memset(hard_vendor, 0, 128);
    strncpy(hard_vendor, vendor, 127);
    hard_vendor[127] = '\0';
    return hard_vendor;
}

void kdk_free_diskinfo(kdk_diskinfo *disk)
{
    SAFE_FREE(disk->name);
    SAFE_FREE(disk->serial);
    SAFE_FREE(disk->uuid);
    SAFE_FREE(disk->vendor);
    SAFE_FREE(disk->model);
    SAFE_FREE(disk->mount_path);
    SAFE_FREE(disk->fwrev);
    SAFE_FREE(disk);
}

int kdk_get_hard_running_time(const char *hardname)
{
    klog_info("hardname = %s\n", hardname);
    if(!hardname)
    {
        return -1;
    }

    int block = is_block_device(hardname);
    
    if((access(hardname, F_OK) == -1) || !block)
    {
        return -1;
    }

    klog_info("hardname = %s, block = %d\n", hardname, block);

    char cmd[128] = {"smartctl -A "};
    char tmp[1024] = {"\0"};
    strncat(cmd, hardname, sizeof(cmd) - strnlen(cmd, STRNLEN_MAX));
    FILE *fp = popen(cmd,"r");
    if(fp)
    {
        int temp = -1;
        while (fgets(tmp, 1024, fp))
        {
            //sata接口返回数据格式
            if(0 == strncmp(tmp,"  9",3))
            {
                sscanf(tmp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %d", &temp);
                break;
            }
            //nvme接口返回数据格式
            if (0 == strncmp(tmp, "Power On Hours:", 15))
            {
                char hours[128] = "\0";
                sscanf(tmp, "%*s %*s %*s %s", hours);

                int len = strnlen(hours, STRNLEN_MAX);
                char tmp_h[len+1];
                int j = 0;
                for (int i = 0; i < len; i++) {
                    if (hours[i] == ',') {
                        continue;
                    } else if (hours[i] != '\0') {
                        tmp_h[j++] = hours[i];
                    }
                }
                tmp_h[j] = '\0';

                temp = atoi(tmp_h);
                break;
            }
        }
        return temp;
    }
    return -1;
}

#define MAX_DISKS 100  // 假设最多有100个磁盘

// 模拟系统时间函数
static struct timeval get_uptime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

// 定义一个结构体存储每个磁盘的读信息
typedef struct {
    char disk_read_name[128];
    unsigned long long blk_read;
    int read_sec;
    struct timeval timevalList[2];
} disk_info_read;

static disk_info_read disk_info_read_list[MAX_DISKS];
static int disk_read_count = 0;

typedef struct {
    char disk_write_name[128];
    unsigned long long blk_wrtn;
    unsigned long long blk_discard_sector; // 放弃的扇区
    int write_sec;
    struct timeval timevalwrList[2];
} disk_info_write;

static disk_info_write disk_info_write_list[MAX_DISKS];
static int disk_write_count = 0;

static int get_uptime_seconds() 
{
    FILE *file = fopen("/proc/uptime", "r");
    if (file == NULL) {
        perror("Failed to open /proc/uptime");
        return -1;
    }

    int uptime_seconds;
    fscanf(file, "%d", &uptime_seconds); // 从文件中读取 uptime（第一个值表示从启动到现在的秒数）

    fclose(file); // 关闭文件
    return uptime_seconds;
}

// 查找或添加磁盘信息
static disk_info_read *get_disk_info_read(const char *diskname) {
    // 查找是否已有该磁盘
    for (int i = 0; i < disk_read_count; i++) {
        if (strcmp(disk_info_read_list[i].disk_read_name, diskname) == 0) {
            disk_info_read_list[i].read_sec = 0;
            disk_info_read_list[i].timevalList[0] = disk_info_read_list[i].timevalList[1];
            disk_info_read_list[i].timevalList[1] = get_uptime();
            return &disk_info_read_list[i];
        }
    }

    // 如果没有找到并且未超过最大磁盘数量，则添加新磁盘
    if (disk_read_count < MAX_DISKS) {
        disk_info_read *new_disk = &disk_info_read_list[disk_read_count++];
        strncpy(new_disk->disk_read_name, diskname, sizeof(new_disk->disk_read_name));
        new_disk->blk_read = 0;
        new_disk->read_sec = get_uptime_seconds();

        new_disk->timevalList[0].tv_sec = 0;
        new_disk->timevalList[0].tv_usec = 0;
        new_disk->timevalList[1].tv_sec = 0;
        new_disk->timevalList[1].tv_usec = 0;
        new_disk->timevalList[0] = new_disk->timevalList[1];
        new_disk->timevalList[1] = get_uptime();
        return new_disk;
    }

    return NULL;
}

static double calc_disk_io_read(unsigned long long read_sectors, disk_info_read *disk_info)
{
    unsigned long long curr_read_sector = read_sectors;
    
    struct timeval cur_time = disk_info->timevalList[1];
    struct timeval prev_time = disk_info->timevalList[0];

    // 读增量
    unsigned long long rdiff = (curr_read_sector > disk_info->blk_read) ? (curr_read_sector - disk_info->blk_read) : 0;
    
    // 计算实际大小
    unsigned long long rsize = (rdiff * SECTOR_SIZE) / 1024;
    // 计算时间间隔
    double ltime = prev_time.tv_sec + prev_time.tv_usec / 1000000.0;
    double rtime = cur_time.tv_sec + cur_time.tv_usec / 1000000.0;
    double interval = (rtime > ltime) ? (rtime - ltime) : 1;

    // 计算读写速度
    double read_speed = rsize / interval;

    if(disk_info->read_sec)
    {
        read_speed = rsize / disk_info->read_sec;
    }
    return read_speed;
}

static double read_speed_info(const char *diskname) 
{
    disk_info_read *disk_info = get_disk_info_read(diskname);
    if (!disk_info) {
        return 0;
    }

    FILE* file = fopen("/proc/diskstats", "r");
    if (!file) {
        return 0;
    }
    char disk[128] = "\0";
    sscanf(diskname, "/dev/%s", disk);

    char line[256];
    char dev_name[32] = "\0";
    unsigned long long read_sectors;

    while (fgets(line, sizeof(line), file)) {
        
        sscanf(line, "%*d %*d %s %*d %*d %llu %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d", dev_name, &read_sectors);
        if (strcmp(dev_name, disk) == 0) 
        {
            break;
        }
    }
    fclose(file);

    double speed = calc_disk_io_read(read_sectors, disk_info);

    disk_info->blk_read = read_sectors;
    return speed;
}

double kdk_get_disk_read_speed(const char *diskname)
{
    if(!diskname)
        return 0;
    
    double read_speed = read_speed_info(diskname);
    return read_speed;
}

// 查找或添加磁盘信息
static disk_info_write *get_disk_info_write(const char *diskname) {
    // 查找是否已有该磁盘
    for (int i = 0; i < disk_write_count; i++) {
        if (strcmp(disk_info_write_list[i].disk_write_name, diskname) == 0) {
            disk_info_write_list[i].write_sec = 0;
            disk_info_write_list[i].timevalwrList[0] = disk_info_write_list[i].timevalwrList[1];
            disk_info_write_list[i].timevalwrList[1] = get_uptime();
            return &disk_info_write_list[i];
        }
    }

    // 如果没有找到并且未超过最大磁盘数量，则添加新磁盘
    if (disk_write_count < MAX_DISKS) {
        disk_info_write *new_disk = &disk_info_write_list[disk_write_count++];
        strncpy(new_disk->disk_write_name, diskname, sizeof(new_disk->disk_write_name));
        new_disk->blk_wrtn = 0;
        new_disk->blk_discard_sector = 0;
        new_disk->write_sec = get_uptime_seconds();

        new_disk->timevalwrList[0].tv_sec = 0;
        new_disk->timevalwrList[0].tv_usec = 0;
        new_disk->timevalwrList[1].tv_sec = 0;
        new_disk->timevalwrList[1].tv_usec = 0;
        new_disk->timevalwrList[0] = new_disk->timevalwrList[1];
        new_disk->timevalwrList[1] = get_uptime();
        return new_disk;
    }

    // 如果超过最大磁盘数量，返回 NULL
    return NULL;
}

static double calc_disk_io_write(unsigned long long write_sectors, unsigned long long discard_sector, disk_info_write *disk_info)
{
    unsigned long long curr_write_sector = write_sectors;
    unsigned long long curr_discard_sector = discard_sector;
    
    struct timeval cur_time = disk_info->timevalwrList[1];
    struct timeval prev_time = disk_info->timevalwrList[0];

    // 写增量
    unsigned long long wdiff = (curr_write_sector > disk_info->blk_wrtn) ? (curr_write_sector - disk_info->blk_wrtn) : 0;
    // 丢弃的增量
    unsigned long long ddiff = (curr_discard_sector > disk_info->blk_discard_sector) ? (curr_discard_sector - disk_info->blk_discard_sector) : 0;
    
    // 计算实际大小
    unsigned long long wsize = ((wdiff + ddiff) * SECTOR_SIZE) / 1024;

    // 计算时间间隔
    double ltime = prev_time.tv_sec + prev_time.tv_usec / 1000000.0;
    double rtime = cur_time.tv_sec + cur_time.tv_usec / 1000000.0;
    double interval = (rtime > ltime) ? (rtime - ltime) : 1;
    // 计算读写速度
    double write_speed = wsize / interval;

    if(disk_info->write_sec)
    {
        write_speed = wsize / disk_info->write_sec;
    }
    
    return write_speed;
}  

static double write_speed_info(const char *diskname) 
{
    disk_info_write *disk_info = get_disk_info_write(diskname);
    if (!disk_info) {
        return 0;
    }

    FILE* file = fopen("/proc/diskstats", "r");
    if (!file) {
        return 0;
    }
    char disk[128] = "\0";
    sscanf(diskname, "/dev/%s", disk);

    char line[256];
    char dev_name[32] = "\0";
    unsigned long long write_sectors;
    unsigned long long discard_sector;

    while (fgets(line, sizeof(line), file)) {
        
        sscanf(line, "%*d %*d %s %*d %*d %*d %*d %*d %*d %lld %*d %*d %*d %*d %*d %*d %lld", dev_name, &write_sectors, &discard_sector);
        if (strcmp(dev_name, disk) == 0) 
        {
            break;
        }
    }
    fclose(file);

    double speed = calc_disk_io_write(write_sectors, discard_sector, disk_info);

    disk_info->blk_wrtn = write_sectors;

    disk_info->blk_discard_sector = discard_sector;
    return speed;
}

double kdk_get_disk_write_speed(const char *diskname)
{
    if(!diskname)
        return 0;
    
    double write_speed = write_speed_info(diskname);
    return write_speed;
}
/**
 * 获取交换分区的可用空间（字节）
 *
 * @param partition 分区设备路径，如 "/dev/sda1"
 * @return 分区的可用空间（字节），如果无法获取则返回 -1
 */
static double kdk_get_swap_partition_space(const char *partition) 
{

    FILE *fp = NULL;
    char path[256] = "\0";

    fp = fopen("/proc/swaps", "r");
    if(!fp)
        return -1;

    char line[1024] = "\0";
    unsigned long count = 0;
    unsigned long size = 0;
    unsigned long long N = 0;

    while (fgets(line, 1024, fp))
    {
        if (strstr(line, partition))
        {
            sscanf(line, "%*s %*s %*d %lld", &N );
            klog_info("line = %s\n", line);
            break;
        }
    }
    fclose(fp);

    
    unsigned long long used_space = -1;

    kdk_diskinfo *info = kdk_get_diskinfo(partition);
    if(!info)
        return -1;

    klog_info("N = %lld\n", N);
    N = (N * (1024 / info->sector_size ) );
    klog_info("info->sector_size = %d\n", info->sector_size);
    klog_info("sectors_num = %lld\n", info->sectors_num);
    used_space = (info->sectors_num - N) * info->sector_size;

    kdk_free_diskinfo(info);

    if (used_space != (unsigned long long)-1) {
        klog_info("%s 已用空间:\n", partition);
        klog_info("  字节: %llu\n", used_space);
        klog_info("  KiB: %.2f\n", (double)used_space / 1024.0);
        klog_info("  MiB: %.2f\n", (double)used_space / (1024.0 * 1024.0));
        klog_info("  GiB: %.2f\n", (double)used_space / (1024.0 * 1024.0 * 1024.0));
    }
    return (double)used_space / (1024.0 * 1024.0);
}

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

/**
 * @brief 临时挂载磁盘并获取磁盘剩余空间
 *
 * @param partition 分区设备路径，如 "/dev/sda1"
 * @return 分区的可用空间，如果无法获取则返回 -1
 */
double _kdk_get_unmounted_partition_space(const char* partition) 
{
    int block = is_block_device(partition);
    
    if((access(partition, F_OK) == -1) || !block)
    {
        return -1;
    }

    klog_info("partition = %s, block = %d\n", partition, block);
    
    unsigned long long used_space = -1;
    char diskname[128] = "\0";
    char canonical_filename[PATH_MAX] = "\0";

    int pos = strlastof(partition, '/');
    char *p_name = partition + pos + 1;
    klog_info("p_name = %s, partition = %s\n", p_name, partition);
    if(strstr(p_name, "dm"))
    {
        char syspath[PATH_MAX];
        snprintf(syspath, sizeof(syspath), "/sys/block/%s/slaves", p_name);

        DIR *dir = opendir(syspath);
        struct dirent *dp;
        while ((dp = readdir(dir)))
        {
            if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
                continue; 

            if(dp->d_name)
                snprintf(diskname, sizeof(diskname), "/dev/%s", dp->d_name);
            klog_info("diskname = %s\n", diskname);
        }
        closedir(dir);
    }
    else
        strncpy(diskname, partition, sizeof(diskname) - 1);

    if(!realpath(diskname, canonical_filename) || !verify_file(canonical_filename))
    {
        return -1;
    }

    FILE *fp = NULL;

    // sprintf(cmd, "dumpe2fs %s", canonical_filename);
    // klog_info("cmd = %s\n", cmd);

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);  // 修复泄漏
        close(fd_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        close(fd_pipe[0]); // 关闭读取端
        dup2(fd_pipe[1], STDOUT_FILENO); // 将管道写入端连接到标准输出
        close(fd_pipe[1]);

        // 使用execve执行所需的命令，例如：
        char* args[] = {"dumpe2fs", canonical_filename, NULL};
        execve("/sbin/dumpe2fs", args, NULL);

        // 如果execve执行失败，需要打印错误信息并退出
        perror("Failed to execute command");
        exit(1);
    } else {
        close(fd_pipe[1]); // 关闭写入端
        fp = fdopen(fd_pipe[0], "r"); // 将读取端转换为文件指针
        if(!fp)
        {
            close(fd_pipe[0]);  // 关闭剩余描述符
            waitpid(pid, NULL, 0);  // 等待子进程
            return -1;
        }
    }

    char line[1024] = "\0";
    unsigned long count = 0;
    unsigned long size = 0;

    while (fgets(line, 1024, fp))
    {
        if (lookup(line, "Block count", &count))
        {
            klog_info("line = %s\n", line);
            continue;
        }
            
        if (lookup(line, "Block size", &size))
        {
            klog_info("line = %s\n", line);
            break;
        }
    }
    pclose(fp);
    int status;
    waitpid(pid, &status, 0);
    if (WEXITSTATUS(status) != 0) {
        return -1;
    }
    klog_info("count = %ld, size = %ld\n", count, size);

    // memset(cmd, 0, 256);
    // sprintf(cmd, "resize2fs -P %s", canonical_filename);

    int fd_pipe_cmd[2];
    if (pipe(fd_pipe_cmd) == -1) {
        return -1;
    }

    pid_t pid_cmd = fork();
    if (pid_cmd == -1) {
        close(fd_pipe_cmd[0]);  // 修复泄漏
        close(fd_pipe_cmd[1]);
        return -1;
    }

    if (pid_cmd == 0) {
        close(fd_pipe_cmd[0]); // 关闭读取端
        dup2(fd_pipe_cmd[1], STDOUT_FILENO); // 将管道写入端连接到标准输出
        close(fd_pipe_cmd[1]);

        // 使用execve执行所需的命令，例如：
        char* args[] = {"resize2fs", "-P", canonical_filename, NULL};
        execve("/sbin/resize2fs", args, NULL);

        // 如果execve执行失败，需要打印错误信息并退出
        perror("Failed to execute command");
        exit(1);
    } else {
        close(fd_pipe_cmd[1]); // 关闭写入端
        fp = fdopen(fd_pipe_cmd[0], "r"); // 将读取端转换为文件指针
        if(!fp)
        {
            close(fd_pipe_cmd[0]);  // 关闭剩余描述符
            waitpid(pid_cmd, NULL, 0);  // 等待子进程
            return -1;
        }
    }
    unsigned long N = 0;

    // fp = popen(cmd, "r");
    // if(!fp)
    //     return -1;
    while (fgets(line, 1024, fp))
    {
        if (strstr(line, "Estimated minimum size of the filesystem: "))
        {
            sscanf(line, "Estimated minimum size of the filesystem: %ld", &N);
            klog_info("line = %s\n", line);
            klog_info("N = %ld\n", N);
            break;
        }
    }
    pclose(fp);
    int status_cmd;
    waitpid(pid_cmd, &status_cmd, 0);
    if (WEXITSTATUS(status_cmd) != 0) {
        return -1;
    }
    klog_info("N = %ld\n", N);

    N = count - N;

    kdk_disk_info *info = kdk_get_disk_info(partition);
    if (!info)
    {
        return -1;
    }

    N = (N * (size / info->sector_size));

    used_space = N * info->sector_size;

    // kdk_free_diskinfo(info);

    if (used_space != (unsigned long long)-1) {
        klog_info("%s 已用空间:\n", canonical_filename);
        klog_info("  字节: %llu\n", used_space);
        klog_info("  KiB: %.2f\n", (double)used_space / 1024.0);
        klog_info("  MiB: %.2f\n", (double)used_space / (1024.0 * 1024.0));
        klog_info("  GiB: %.2f\n", (double)used_space / (1024.0 * 1024.0 * 1024.0));
    }
    kdk_free_disk_info(info);
    
    return (double)used_space / (1024.0 * 1024.0);
}

static double kdk_get_unmounted_partition_space(const char* partition)
{
    double ret = -1;
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
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        ret = -1;
        goto out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getUnmountedPartitionSpace");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &partition, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = -1;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = -1;
        goto out;
    }

    if (!sendMsgPending)
    {
        ret = -1;
        goto out;
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = -1;
        goto out;
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        klog_err("kdk : d-bus reply message fail !\n");
        ret = -1;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

out:
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }
    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    
    // 移除错误的fallback逻辑，如果DBus调用失败直接返回-1
    if(ret == -1 || !ret)
    {
        // 记录错误日志，表示无法获取未挂载分区的空间
        klog_err("无法通过DBus获取未挂载分区的空间: %s\n", partition);
        ret = _kdk_get_unmounted_partition_space(partition);
    }

    return ret;
}

/**
 * 通过设备路径获取其挂载点
 *
 * @param device 设备路径，如 "/dev/sda1"
 * @return 挂载点的字符串，如果未找到则返回 NULL
 */
static char* kdk_get_mount_point(const char* device) {
    FILE* mtab = setmntent("/proc/mounts", "r");
    if (!mtab) {
        perror("setmntent");
        return NULL;
    }

    struct mntent* entry;
    char* mount_point = NULL;
    
    while ((entry = getmntent(mtab)) != NULL) {
        // 比较设备路径
        if (strcmp(basename(entry->mnt_fsname), basename((char*)device)) == 0) {
            mount_point = strdup(entry->mnt_dir);
            break;
        }
    }
    
    endmntent(mtab);
    return mount_point;
}

/**
 * 获取已挂载分区的可用空间（以 MiB 为单位）
 *
 * @param partition 分区设备路径，如 "/dev/sda1"
 * @return 分区的可用空间，如果无法获取则返回 -1
 */
double kdk_get_partition_available_space(const char* partition) {

    if(!partition)
        return -1;
    
    kdk_disk_info *info = kdk_get_disk_info(partition);
    if(info)
    {
        if(info->format == DISK_FMT_SWAP)
        {
            kdk_free_disk_info(info);
            return kdk_get_swap_partition_space(partition);
        }
        else if(info->format == DISK_FMT_NONE)
        {
            kdk_free_disk_info(info);
            return -1;
        }
        else
            kdk_free_disk_info(info);
    }
    
    // 1. 首先获取分区的挂载点
    char* mount_point = kdk_get_mount_point(partition);
    if (!mount_point) {
        // klog_info("Partition %s is not mounted\n", partition);
        return kdk_get_unmounted_partition_space(partition);
    }

    // 2. 使用挂载点获取空间信息
    struct statvfs vfs;
    double available_space = -1;
    unsigned long long available = -1;
    
    if (statvfs(mount_point, &vfs) == 0) {
        available = (unsigned long long)vfs.f_bfree * (unsigned long long)vfs.f_frsize;
        available_space = available / (1024.0 * 1024.0); // 转换为 MiB
    }

    free(mount_point);

    if (available != (unsigned long long)-1) {
        klog_info("%s 已用空间:\n", partition);
        klog_info("  字节: %llu\n", available);
        klog_info("  KiB: %.2f\n", (double)available / 1024.0);
        klog_info("  MiB: %.2f\n", (double)available / (1024.0 * 1024.0));
        klog_info("  GiB: %.2f\n", (double)available / (1024.0 * 1024.0 * 1024.0));
    }
    return available_space;
}

int kdk_disk_delete_all_partitions(const char *device_path) 
{
    struct fdisk_context *cxt;

    // 创建 fdisk 上下文
    cxt = fdisk_new_context();
    if (fdisk_assign_device(cxt, device_path, 0))  // 0 表示可写
    {
        klog_err("打开磁盘设备失败");
        fdisk_unref_context(cxt);
        return -1;
    }

    // 删除所有分区
    if (fdisk_delete_all_partitions(cxt)) 
    {
        klog_err("删除所有分区失败\n");
        fdisk_unref_context(cxt);
        return -1;
    }

    // 写入分区表更改
    if (fdisk_write_disklabel(cxt)) 
    {
        klog_err("写入分区表失败");
        fdisk_unref_context(cxt);
        return -1;
    }

    klog_info("所有分区删除成功！\n");
    fdisk_unref_context(cxt);
    return 0;
}

char* kdk_disk_get_volume_label(const char *partition) 
{
    const char *label;
    
    // 直接使用 blkid 函数获取卷标
    label = blkid_get_tag_value(NULL, "LABEL", partition);
    if (label == NULL) 
    {
        klog_info("未找到卷标: %s\n", partition);
        return NULL;
    }

    return strdup(label);  // 返回卷标的副本
}

char* kdk_disk_get_mount_point(const char *partition) 
{
    FILE *file = fopen("/proc/mounts", "r");
    if (!file) 
    {
        klog_err("Error: Failed to open file '/proc/mounts' ");
        return NULL;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) 
    {
        char *dev = strtok(line, " ");
        char *mount_point = strtok(NULL, " ");
        
        // 如果当前行的设备名称和输入的分区相匹配
        if (strcmp(dev, partition) == 0) 
        {
            fclose(file);
            // 复制挂载点路径并返回
            char *result = malloc(strnlen(mount_point, STRNLEN_MAX) + 1);
            if (result) 
            {
                strncpy(result, mount_point, strnlen(mount_point, STRNLEN_MAX) + 1);
            }
            return result;
        }
    }

    fclose(file);
    return NULL; // 如果没有找到挂载点，则返回NULL
}

long long _kdk_disk_get_partition_start_sector(const char *partition) 
{
    // 1. 参数验证
    if (partition == NULL || *partition == '\0') {
        klog_err("Invalid partition parameter: %s", 
                partition ? partition : "NULL");
        errno = EINVAL;
        return -1;
    }
    long long start_sector = -1;

    char disk_path[256] = {0};
    if (get_disk_path(partition, disk_path, sizeof(disk_path)) != 0) {
        klog_err("Failed to parse disk path from %s\n", partition);
        return -1;
    }

    klog_info("disk_path = %s\n", disk_path);

    PedDevice *dev = ped_device_get(disk_path);
    if (!dev) {
        klog_err("Failed to get device for %s\n", disk_path);
        return -1;
    }

    PedDisk *disk = ped_disk_new(dev);
    if (!disk) {
        klog_err("Failed to read partition table from %s\n", disk_path);
        ped_device_destroy(dev);
        return -1;
    }

    PedPartition *part = NULL;
    part = ped_disk_next_partition(disk, NULL);
    while (part) {
        const char *device_part_path = ped_partition_get_path(part);
        klog_info("device_part_path = %s\n", device_part_path);
        if (device_part_path && strcmp(device_part_path, partition) == 0) {
            start_sector = part->geom.start;

            klog_info("part->geom.start = %lld, start_sector = %lld\n", part->geom.start, start_sector);
        }
        part = ped_disk_next_partition(disk, part);
    }
    ped_disk_destroy(disk);
    ped_device_destroy(dev);

    return start_sector;
}

long long kdk_disk_get_partition_start_sector(const char *partition)
{
    long long ret = 0;
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
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getPartitionStartSector"); // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &partition, DBUS_TYPE_INVALID))
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
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    if(ret == -1)
    {
        ret = _kdk_disk_get_partition_start_sector(partition);
    }

    long long start_sector;
    start_sector = ret;

    return start_sector;
}

long long _kdk_disk_get_partition_end_sector(const char *partition) 
{
    // 1. 参数验证
    if (partition == NULL || *partition == '\0') {
        klog_err("Invalid partition parameter: %s", 
                partition ? partition : "NULL");
        return -1;
    }
    long long end_sector = -1;

   char disk_path[256] = {0};
    if (get_disk_path(partition, disk_path, sizeof(disk_path)) != 0) {
        klog_err("Failed to parse disk path from %s\n", partition);
        return -1;
    }

    klog_info("disk_path = %s\n", disk_path);

    PedDevice *dev = ped_device_get(disk_path);
    if (!dev) {
        klog_err("Failed to get device for %s\n", disk_path);
        return -1;
    }

    PedDisk *disk = ped_disk_new(dev);
    if (!disk) {
        klog_err("Failed to read partition table from %s\n", disk_path);
        ped_device_destroy(dev);
        return -1;
    }

    PedPartition *part = NULL;
    part = ped_disk_next_partition(disk, NULL);
    while (part) {
        const char *device_part_path = ped_partition_get_path(part);
        klog_info("device_part_path = %s\n", device_part_path);
        if (device_part_path && strcmp(device_part_path, partition) == 0) {
            end_sector = part->geom.end;

            klog_info("part->geom.end = %lld, end_sector = %lld\n", part->geom.end, end_sector);
        }
        part = ped_disk_next_partition(disk, part);
    }
    ped_disk_destroy(disk);
    ped_device_destroy(dev);

    return end_sector;
}

long long kdk_disk_get_partition_end_sector(const char *partition)
{
    long long ret = 0;
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
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getPartitionEndSector"); // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &partition, DBUS_TYPE_INVALID))
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
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    if(ret == -1)
    {
        ret = _kdk_disk_get_partition_end_sector(partition);
    }

    long long end_sector;
    end_sector = ret;

    return end_sector;
}

int kdk_disk_mount_partition(const char *partition, const char *mount_point) 
{
    if (partition == NULL || mount_point == NULL) 
    {
        return -1;
    }

    // 使用 libblkid 获取分区的文件系统类型
    const char *fs_type = blkid_get_tag_value(NULL, "TYPE", partition);
    if (fs_type == NULL) 
    {
        klog_err("无法获取分区 %s 的文件系统类型\n", partition);
        return -1;
    }

    // 尝试挂载分区
    if (mount(partition, mount_point, fs_type, 0, NULL) == -1) 
    {
        klog_err("挂载失败");
        return -1;
    }

    return 0; // 成功
}

int kdk_disk_delete_partition(const char *partition) 
{
    struct fdisk_context *cxt;
    int part_num;
    char disk[128];

    // 找到分区号的起始位置（最后一个非数字字符之后）
    const char *num_start = partition + strnlen(partition, STRNLEN_MAX);
    while (num_start > partition && *(num_start - 1) >= '0' && *(num_start - 1) <= '9') 
    {
        num_start--;
    }

    // 提取磁盘路径
    size_t disk_len = num_start - partition;
    strncpy(disk, partition, disk_len);
    disk[disk_len] = '\0';

    // 转换分区号
    part_num = atoi(num_start) - 1;  // 将分区号减1，调整为系统实际的分区编号
    if (part_num < 0) 
    {
        klog_err("无效的分区号：%s\n", num_start);
        return -1;
    }

    // 打开磁盘设备，设置为可写模式
    cxt = fdisk_new_context();
    if (fdisk_assign_device(cxt, disk, 0)) 
    {  
        klog_err("打开磁盘设备失败");
        fdisk_unref_context(cxt);
        return -1;
    }

    // 删除指定分区
    if (fdisk_delete_partition(cxt, part_num)) 
    {
        klog_err("删除分区失败");
        fdisk_unref_context(cxt);
        return -1;
    }

    // 写入更改
    if (fdisk_write_disklabel(cxt)) 
    {
        klog_err("写入分区表失败");
        fdisk_unref_context(cxt);
        return -1;
    }

    klog_info("分区 %s 删除成功！\n", partition);
    fdisk_unref_context(cxt);
    return 0;
}

int kdk_disk_unmount_partition(const char *partition) 
{
    struct libmnt_context *cxt;

    // 创建挂载上下文
    cxt = mnt_new_context();
    if (cxt == NULL) 
    {
        klog_err("创建 libmount 上下文失败\n");
        return -1;
    }

    // 设置要卸载的目标
    mnt_context_set_target(cxt, partition);

    // 尝试卸载分区
    if (mnt_context_umount(cxt) != 0) 
    {
        mnt_free_context(cxt);
        return -1;
    }

    klog_info("分区 %s 成功卸载\n", partition);

    // 释放上下文
    mnt_free_context(cxt);
    return 0;
}

char* _kdk_disk_get_partition_table_type(const char *device_path) 
{
    int fd;
    unsigned char buffer[512]; // 磁盘的一个扇区大小通常为 512 字节
    char *partition_type = NULL;

    // 打开磁盘设备
    fd = open(device_path, O_RDONLY);
    if (fd < 0) 
    {
        klog_err("Failed to open device");
        return NULL;
    }

    // 读取磁盘的第一个扇区
    if (read(fd, buffer, sizeof(buffer)) != sizeof(buffer)) 
    {
        klog_err("Failed to read disk sector");
        close(fd);
        return NULL;
    }

    // 检查分区表类型
    // 检查是否存在保护性 MBR 的签名 0x55AA
    if (buffer[0x1FE] == 0x55 && buffer[0x1FF] == 0xAA) 
    {
        // 检测到 0x55AA 签名，继续检查 GPT 签名。
        // 移动到第二个扇区 (偏移 512 字节)
        if (lseek(fd, 512, SEEK_SET) == -1) 
        {
            klog_err("Failed to seek to second sector");
            close(fd);
            return NULL;
        }

        // 读取第二个扇区
        if (read(fd, buffer, sizeof(buffer)) != sizeof(buffer)) 
        {
            klog_err("Failed to read second sector");
            close(fd);
            return NULL;
        }

        // 检查 GPT 签名 "EFI PART"
        if (memcmp(buffer, "EFI PART", 8) == 0) 
        {
            partition_type = "GPT"; // 确认是 GPT
        } 
        else 
        {
            partition_type = "MBR"; // 如果没有 GPT 签名，则确认是 MBR
        }
    } 
    else 
    {
        // 如果第一个扇区没有保护性 MBR 的签名，继续检查第二个扇区
        // 移动到第二个扇区 (偏移 512 字节)
        if (lseek(fd, 512, SEEK_SET) == -1) 
        {
            klog_err("Failed to seek to second sector");
            close(fd);
            return NULL;
        }

        // 读取第二个扇区
        if (read(fd, buffer, sizeof(buffer)) != sizeof(buffer)) 
        {
            klog_err("Failed to read second sector");
            close(fd);
            return NULL;
        }

        // 检查 GPT 签名 "EFI PART"
        if (memcmp(buffer, "EFI PART", 8) == 0) 
        {
            partition_type = "GPT"; // 确认是 GPT
        } 
        else 
        {
            partition_type = "Unknown"; // 不能识别的类型
        }
    }

    // 复制分区表类型到动态分配的内存
    char *result = malloc(strnlen(partition_type, STRNLEN_MAX) + 1);
    if (result != NULL) 
    {
        strncpy(result, partition_type, strnlen(partition_type, STRNLEN_MAX) + 1);
    }

    // 关闭设备
    close(fd);

    return result;
}

char* kdk_disk_get_partition_table_type(const char *device_path)
{

    char* ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = NULL;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getPartitionTableType");   // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = NULL;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &device_path, DBUS_TYPE_INVALID))
    {
        ret = NULL;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = NULL;
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
        ret = NULL;
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
        ret = NULL;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = NULL;
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
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    if(ret == NULL)
    {
        ret = _kdk_disk_get_partition_table_type(device_path);
    }

    char* partition_type;
    partition_type = ret;

    return partition_type;
}

int _kdk_disk_get_total_tracks(const char *device_path) 
{
    int fd;
    struct hd_geometry geo;

    // 打开磁盘设备
    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        klog_err("Failed to open device");
        return -1;
    }

    // 获取磁盘几何信息
    if (ioctl(fd, HDIO_GETGEO, &geo) < 0) 
    {
        klog_err("Failed to get disk geometry");
        close(fd);
        return -1;
    }

    // 如果几何信息不合法，则返回错误
    if (geo.heads <= 0 || geo.cylinders <= 0) 
    {
        klog_err("Invalid disk geometry values: heads=%d, cylinders=%d\n", 
                geo.heads, geo.cylinders);
        close(fd);
        return -1;
    }

    // 计算总磁道数
    int total_tracks = geo.heads * geo.cylinders;

    // 关闭设备
    close(fd);

    // 返回磁盘的总磁道数
    return total_tracks;
}

int kdk_disk_get_total_tracks(const char *device_path)
{
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
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "getDiskHeads"); // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -1;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &device_path, DBUS_TYPE_INVALID))
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
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    if(ret == -1)
    {
        ret = _kdk_disk_get_total_tracks(device_path);
    }

    int total_tracks;
    total_tracks = ret;

    return total_tracks;
}

int _kdk_disk_is_disk_writable(const char *device_path) 
{
    int fd = open(device_path, O_WRONLY);  // 尝试以写模式打开设备
    if (fd != -1) 
    {
        close(fd);  // 成功打开，表示可写，关闭文件描述符
        return 0; 
    } 
    else 
    {
        klog_err("Open error");  // 打开失败，打印错误信息
        return -1; 
    }
}

int kdk_disk_is_disk_writable(const char *device_path) 
{
    int ret = 2;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
   
    if (NULL == conn)
    {
        ret = -2;
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/disk", // object to call on
                                            "com.kylin.kysdk.disk",  // interface to call on
                                            "isDiskWritable"); // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = -2;
        goto out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &device_path, DBUS_TYPE_INVALID))
    {
        ret = -2;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -2;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -2;
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
        ret = -2;
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
        ret = -2;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -2;
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
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    if(ret == -2)
    {
        ret = _kdk_disk_is_disk_writable(device_path);
    }

    int heads;
    heads = ret;

    return heads;
}

DiskGeometry* _kdk_disk_get_disk_geometry(const char *device_path) 
{
    int fd;
    struct hd_geometry geo;
    DiskGeometry *geometry = malloc(sizeof(DiskGeometry)); // 分配内存

    if (geometry == NULL) 
    {
        klog_err("Failed to allocate memory for DiskGeometry");
        return NULL;
    }

    // 打开设备文件
    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) 
    {
        klog_err("Failed to open device");
        free(geometry);
        return NULL;
    }

    // 获取磁盘几何信息
    if (ioctl(fd, HDIO_GETGEO, &geo) == -1) 
    {
        klog_err("Failed to get disk geometry");
        close(fd);
        free(geometry);
        return NULL;
    }

    // 获取每个扇区的字节数
    int sector_size;
    if (ioctl(fd, BLKSSZGET, &sector_size) == -1) 
    {
        klog_err("Failed to get sector size");
        close(fd);
        free(geometry);
        return NULL;
    }

    // 填充 DiskGeometry 结构体
    geometry->cylinders = geo.cylinders;
    geometry->tracks_per_cylinder = geo.heads; // tracks_per_cylinder 用 heads 表示
    geometry->sectors_per_track = geo.sectors;
    geometry->bytes_per_sector = sector_size; // 使用获取的实际扇区大小

    // 关闭设备文件
    close(fd);

    return geometry; // 返回结构体指针
}

DiskGeometry* kdk_disk_get_disk_geometry(const char *device_path)
{
    DiskGeometry* geometry = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

    if (NULL == conn)
    {
        return NULL;
    }

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    // 创建方法调用消息
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service", 
                                            "/com/kylin/kysdk/disk", 
                                            "com.kylin.kysdk.disk",  
                                            "getDiskGeometry");   
    
    if (!info_msg)
    { // -1 is default timeout
        return NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &device_path, DBUS_TYPE_INVALID))
    {
        return NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        return NULL;
    }

    if (sendMsgPending == NULL)
    {
        return NULL;
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
        return NULL;
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
 
    // 提取 JSON 字符串
    const char *json_str = NULL;
    DBusMessageIter args;
    if (dbus_message_iter_init(replyMsg, &args) && DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) 
    {
        dbus_message_iter_get_basic(&args, &json_str);
    }

    dbus_message_unref(replyMsg);
    dbus_connection_close(conn);

    if (json_str == NULL) 
    {
        return NULL;
    }

    // 解析 JSON 数据
    struct json_object *parsed_json = json_tokener_parse(json_str);
    if (parsed_json == NULL) 
    {
        fprintf(stderr, "Failed to parse JSON\n");
        return NULL;
    }

    geometry = malloc(sizeof(DiskGeometry));
    if (geometry == NULL) 
    {
        json_object_put(parsed_json); // 释放 JSON 对象
        return NULL;
    }

    struct json_object *cylinders, *tracks_per_cylinder, *sectors_per_track, *bytes_per_sector;

    // 提取字段并赋值
    if (json_object_object_get_ex(parsed_json, "cylinders", &cylinders))
        geometry->cylinders = json_object_get_int(cylinders);

    if (json_object_object_get_ex(parsed_json, "tracks_per_cylinder", &tracks_per_cylinder))
        geometry->tracks_per_cylinder = json_object_get_int(tracks_per_cylinder);

    if (json_object_object_get_ex(parsed_json, "sectors_per_track", &sectors_per_track))
        geometry->sectors_per_track = json_object_get_int(sectors_per_track);

    if (json_object_object_get_ex(parsed_json, "bytes_per_sector", &bytes_per_sector))
        geometry->bytes_per_sector = json_object_get_int(bytes_per_sector);

    // 释放 JSON 对象
    json_object_put(parsed_json);

    return geometry;
out:
    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    geometry = _kdk_disk_get_disk_geometry(device_path);
    return geometry;
}

int kdk_disk_sync(const char *device_path) 
{
    int fd = open(device_path, O_RDWR); // 以读写模式打开磁盘设备
    if (fd < 0) 
    {
        klog_err("打开设备失败"); // 打印错误信息
        return -1; // 打开失败
    }

    if (fsync(fd) < 0) 
    { 
        // 刷新磁盘
        klog_err("刷新设备失败");
        close(fd); // 关闭文件描述符
        return -1; // 刷新失败
    }

    close(fd); // 关闭文件描述符
    return 0; // 刷新成功
}

static kdk_disk_info* kdk_get_disk_info(const char *diskname)
{
    if (!diskname)
    {
        klog_err("diskname is NULL\n");
        return NULL;
    }

    kdk_disk_info *res = NULL;

    res = (kdk_disk_info *)calloc(1, sizeof(kdk_disk_info));
    if (!res)
    {
        klog_err("内存分配失败\n");
        goto cleanup;
    }

    res->name = strdup(diskname);

    // 获取其他磁盘信息
    if (kdk_get_disk_size(res) != 0)
    {
        klog_err("获取磁盘容量信息失败: %s\n", diskname);
        goto cleanup;
    }

    if (get_disk_partitions_info(res) != 0)
    {
        klog_err("获取磁盘容量信息失败: %s\n", diskname);
        goto cleanup;
    }

    return res;

cleanup:
    // 统一清理资源
    kdk_free_disk_info(res);
    return NULL;
}   

static int kdk_get_disk_size(kdk_disk_info *info)
{
    char devname[64] = {0};
    char path[256];
    FILE *fp;

    unsigned long long sectors = 0;
    unsigned int logical_block_size = 512; // 默认512

    char disk_path[256] = {0};
    if (get_disk_path(info->name, disk_path, sizeof(disk_path)) != 0)
    {
        klog_err("解析磁盘路径失败: %s\n", info->name);
        return -1;
    }

    strcpy(devname, basename((char *)info->name));

    snprintf(path, sizeof(path), "/sys/class/block/%s/size", devname);
    fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    if (fscanf(fp, "%llu", &sectors) != 1) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    sectors <<= 9;

    strncpy(devname, basename((char *)disk_path), sizeof(devname) - 1);
    devname[sizeof(devname) - 1] = '\0';
    snprintf(path, sizeof(path), "/sys/block/%s/queue/logical_block_size", devname);

    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%u", &logical_block_size);
        fclose(fp);
    }
    info->sector_size = logical_block_size;

    info->total_size_MiB = (double)sectors / 1024.0 / 1024.0;

    info->sectors_num = sectors / logical_block_size;
     
    return 0;
}

static kdk_disk_format string_to_format(const char *type)
{
    if (!type) return DISK_FMT_UNKNOW;

    if(type)
    {
        if (strcmp(type, "ext4") == 0)
            return DISK_FMT_EXT4;
        else if (strcmp(type, "fat32") == 0)
            return DISK_FMT_FAT32;
        else if (strcmp(type, "ntfs") == 0)
            return DISK_FMT_NTFS;
        else if (strcmp(type, "ext3") == 0)
            return DISK_FMT_EXT3;
        else if (strcmp(type, "vfat") == 0)
            return DISK_FMT_VFAT;
        else if (strcmp(type, "ext2") == 0)
            return DISK_FMT_EXT2;
        else if (strcmp(type, "fat16") == 0)
            return DISK_FMT_FAT16;
        else if (strcmp(type, "swap") == 0)
            return DISK_FMT_SWAP;
        else
            return DISK_FMT_UNKNOW;
    }
}

static int get_disk_partitions_info(kdk_disk_info *info)
{
    char sys_path[512];
    DIR *dir = NULL;
    struct dirent *entry;
    int part_count = 0;
    char devname[64] = {0};

    strcpy(devname, basename((char *)info->name));
    
    // 构建 /sys/block/xxx 路径
    snprintf(sys_path, sizeof(sys_path), "/sys/block/%s", devname);

    dir = opendir(sys_path);
    if (!dir) {
        klog_err("无法打开 %s\n", sys_path);
        info->partition_nums  = 0;
    }    

    if (info->partition_nums != 0)
    {
        // 遍历所有分区
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;
            
            // 检查是否是分区（设备名以磁盘名开头，且后面有数字）
            if (strncmp(entry->d_name, info->name, strlen(info->name)) == 0) {
                const char *suffix = entry->d_name + strlen(info->name);
                // 确保是分区：后缀以数字开头
                if (isdigit(suffix[0])) {
                    part_count++;
                }
            }
        }
        info->partition_nums  = part_count;
    }

    if (dir)
    {
        closedir(dir);
    }

    if (info->partition_nums < 0)
        goto out;
    
    if (info->partition_nums == 0)    // 子分区，只有子分区才有type
    {
        FILE *fp;
        char line[512];
        char device[256];
        int is_swap = 0;
        
        fp = fopen("/proc/swaps", "r");
        if (!fp) {
            return 0;
        }
        
        // 跳过标题行
        fgets(line, sizeof(line), fp);
        
        // 格式：Filename Type Size Used Priority
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "%255s", device) == 1) {
                if (strcmp(device, info->name) == 0) {
                    is_swap = 1;
                    break;
                }
            }
        }
        
        fclose(fp);
        if(is_swap)
        {
            info->format  = DISK_FMT_SWAP;
            return 0;
        }

        FILE *mounts;
        struct mntent *entry;
        
        // 打开 /proc/mounts（优先）或 /etc/mtab
        mounts = setmntent("/proc/mounts", "r");
        if (!mounts) {
            mounts = setmntent("/etc/mtab", "r");
            if (!mounts) {
                info->format  = DISK_FMT_NONE;
                return 0;
            }
        }
        
        // 遍历所有挂载点，查找匹配的设备
        while ((entry = getmntent(mounts)) != NULL) {
            // 使用basename比较，以处理不同格式的路径
            if (strcmp(basename(entry->mnt_fsname), basename((char*)info->name)) == 0) {
                klog_debug("找到匹配的挂载点: %s\n", entry->mnt_fsname);
                info->format = string_to_format(entry->mnt_type);
                break;
            }
        }
        
        endmntent(mounts);
    }
    else
        info->format   = DISK_FMT_NONE;
out:
    return 0;
}

void kdk_free_disk_info(kdk_disk_info *disk)
{
    SAFE_FREE(disk->name);
    SAFE_FREE(disk);
}
