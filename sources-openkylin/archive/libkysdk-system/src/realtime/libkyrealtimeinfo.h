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

#ifndef LIBKYREALTIMEINFO_H
#define LIBKYREALTIMEINFO_H

/**
 * @file libkyrealtimeinfo.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 瞬时信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2023
 * @defgroup libkysdk-realtime 
 * @{
 * @since 2.0.0.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define IO_ERROR ((unsigned long)(-1))

/**
 * @brief 获取上传的瞬时网速
 * 
 * @param nc 网卡名称，如eno1
 * @return float 成功返回上传的瞬时网速，失败返回-1.
 */
extern float kdk_real_get_net_speed(const char *nc);

/**
 * @brief 获取CPU温度
 * 
 * 
 * @return double 成功返回CPU温度，失败返回-1.
 */
extern double kdk_real_get_cpu_temperature();

/**
 * @brief 获取磁盘温度
 * 
 * @param name 磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 成功返回磁盘温度，失败返回-1.
 */
extern int kdk_real_get_disk_temperature(const char *name);

/**
 * @brief 获取磁盘转速
 * 
 * @param name 磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 成功返回磁盘转速，失败返回-1.
 */
extern int kdk_real_get_disk_rate(const char *name);

/**
 * @brief 获取下载的瞬时网速信息
 * 
 * @param nc 网卡名称，如eno1
 * @return float 成功返回下载的瞬时网速，失败返回-1.
 * @since 2.2.3.5
 */
extern float kdk_real_get_if_speed(const char *nc);

/**
 * @brief 获取指定磁盘的IO读取字节数
 * 
 * @param diskname 磁盘名
 * @return unsigned long long 返回读取的字节数，返回 0 表示失败
 * @since 3.0.1.0
 */
extern unsigned long long kdk_real_get_disk_read(const char *diskname);

/**
 * @brief 获取指定磁盘的IO写入字节数
 * 
 * @param diskname 磁盘名
 * @return unsigned long long 返回写入的字节数，返回 0 表示失败
 * @since 3.0.1.0
 */
extern unsigned long long kdk_real_get_disk_write(const char *diskname);

/**
 * @brief 获取指定进程的CPU执行的指令数
 * 
 * @param pid 进程ID
 * @param sleepTime 间隔时间，默认1秒
 * @return int 返回CPU执行的指令数，返回 -1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_instructions(int pid, int sleepTime);

/**
 * @brief 获取指定进程的CPU缓存未命中次数
 * 
 * @param pid 进程ID
 * @param sleepTime 间隔时间，默认1秒
 * @return int 返回CPU缓存未命中次数，返回 -1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_cache_misses(int pid, int sleepTime);

/**
 * @brief 获取指定进程的CPU分支预测未命中的次数
 * 
 * @param pid 进程ID
 * @param sleepTime 间隔时间，默认1秒
 * @return int 返回CPU分支预测未命中的次数，返回 -1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_branch_misses(int pid, int sleepTime);

/**
 * @brief 获取指定进程的CPU系统总线周期数
 * 
 * @param pid 进程ID
 * @param sleepTime 间隔时间，默认1秒
 * @return int 返回CPU系统总线周期数，返回 -1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_bus_cycles(int pid, int sleepTime);

/**
 * @brief 获取指定进程的CPU时钟周期数
 * 
 * @param pid 进程ID
 * @param sleepTime 间隔时间，默认1秒
 * @return int 返回CPU时钟周期数，返回 -1 表示失败
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_cycles(int pid, int sleepTime);

/**
 * @brief 获取等待磁盘 I/O 操作完成的请求数量
 * 
 * @param diskname 磁盘名
 * @return int 等待磁盘 I/O 操作完成的请求数量；失败返回IO_ERROR
 * @since 3.0.1.0
 */
extern unsigned long kdk_real_get_disk_queue_length(char *diskname);

/**
 * @brief 获取cpu执行内核态的时间比例
 * 
 * @return float cpu执行内核态的时间比例
 * @since 3.0.1.0
 */
extern float kdk_real_get_cpu_kernel_modes_per();

/**
 * @brief 获取cpu执行用户态的时间比例
 * 
 * @return float cpu执行用户态的时间比例
 * @since 3.0.1.0
 */
extern float kdk_real_get_cpu_user_modes_per();

/**
 * @brief 获取等待cpu时间的线程数
 * 
 * @return int 等待cpu时间的线程数
 * @since 3.0.1.0
 */
extern int kdk_real_get_cpu_queue_length();

/**
 * @brief 获取每秒交换到磁盘或从磁盘读回内存的页面数量
 * 
 * @return int 每秒交换到磁盘或从磁盘读回内存的页面数量
 * @since 3.0.1.0
 */
extern int kdk_real_get_memory_page();

/**
 * @brief 获取cpu的页错误率（硬页故障）
 * 
 * @return float cpu的硬页错误率
 * @since 3.0.1.0
 */
extern float kdk_real_get_cpu_hard_page_faults();

/**
 * @brief 获取cpu的页错误率（软页故障）
 * 
 * @return float cpu的软页错误率
 * @since 3.0.1.0
 */
extern float kdk_real_get_cpu_soft_page_faults();

#ifdef __cplusplus
}
#endif

/***********************************
 * 类已弃用 为之前已引用了c++接口的应用保留
 * 新应用获取瞬时信息请使用上面的c接口
************************************/
#ifdef __cplusplus

#define REAL_TIME_EXPORT     __attribute__((visibility("default")))
class REAL_TIME_EXPORT RealTimeInfo
{
public:
    RealTimeInfo();
public:
    float kdk_real_get_io_speed();
    float kdk_real_get_net_speed(const char *nc);
    float kdk_real_get_if_speed(const char *nc);
    float kdk_real_get_cpu_temperature();
    float kdk_real_get_disk_temperature(const char *name);
    int kdk_real_get_disk_rate(const char *name);
};
#endif

#endif // LIBKYREALTIMEINFO_H

/**
 * \example kysdk-system/src/realtime/test/kyrealtime-test.c
 * 
 */
