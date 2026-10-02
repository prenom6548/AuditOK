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

#ifndef KYSDK_SYSTEM_DEVICE_MANAGER_H_
#define KYSDK_SYSTEM_DEVICE_MANAGER_H_

/**
 * @file        libkydevicemanager.h
 * @brief       devicemanager设备
 * @author      tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @version     0.1
 * @date        2024-10-28
 * @copyright   Copyright (c) 2025
 * @defgroup    libkysdk-devicemanager
 * @since       3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _kdk_device_number
{
    char *device_subsystem; // 设备的类型
    char *device_devtype;   // 设备在其子系统中的具体角色
    char *device_syspath;   // 设备在内核空间中的路径
    char *device_sysname;   // 内核分配的设备名称
    int device_sysnum;      // 设备的编号
} kdk_device_number;

typedef enum _kdk_device_type
{
    DEVICE_DEVTYPE_UNKNOWN,      // 未知设备类型
    DEVICE_DEVTYPE_DISK,              // 硬盘
    DEVICE_DEVTYPE_PARTITION,         // 分区
    DEVICE_DEVTYPE_NET,               // 网络设备
    DEVICE_DEVTYPE_USB,               // USB 设备
    DEVICE_DEVTYPE_SERIAL,            // 串口设备
    DEVICE_DEVTYPE_SCSI,              // 串口设备
    DEVICE_DEVTYPE_INPUT,             // 输入设备
    DEVICE_DEVTYPE_PCI_CARD,          // PCI 卡
    DEVICE_DEVTYPE_SOUND_CARD,        // 声卡
    DEVICE_DEVTYPE_GRAPHIC_CARD,      // 显卡
    DEVICE_DEVTYPE_I2C_SENSOR,        // I²C 传感器
    DEVICE_DEVTYPE_SPI_SENSOR,        // SPI 传感器
    DEVICE_DEVTYPE_BATTERY,           // 电池
    DEVICE_DEVTYPE_BLUETOOTH_ADAPTER, // 蓝牙适配器
    DEVICE_DEVTYPE_HID_DEVICE,        // 人机接口设备
    DEVICE_DEVTYPE_EMBEDDED,          // 嵌入式设备
    DEVICE_DEVTYPE_THERMAL_SENSOR,    // 温度传感器
    DEVICE_DEVTYPE_ACPI_DEVICE,       // ACPI 设备
    DEVICE_DEVTYPE_MISC_DEVICE        // ACPI 设备
} kdk_device_type;

/**
 * @brief 获取系统中所有已连接的设备
 *
 * @return char** 已连接设备的名称列表，失败返回NULL
 * @since 3.0.1.0
 */
extern char **kdk_dm_get_device_list();

/**
 * @brief 检查指定设备是否支持访问
 *
 * @param devicename 设备名称，比如/dev/sda
 * @return int 设备是否支持访问 0 支持 -1不支持
 * @since 3.0.1.0
 */
extern int kdk_dm_check_device_available(char *devicename);

/**
 * @brief 获取指定设备的设备信息
 *
 * @param devicepath kdk_dm_get_device_list获取的设备名称
 * @return kdk_device_number* 设备类型、设备编号及相关的结构体，失败返回NULL
 * @since 3.0.1.0
 */
extern kdk_device_number *kdk_dm_get_device_number(char *devicepath);

/**
 * @brief 获取指定设备的热插拔支持情况
 *
 * @param devicepath kdk_dm_get_device_list获取的设备名称
 * @return int 热插拔支持情况，1为支持，-1为不支持，0为未知
 * @since 3.0.1.0
 */
extern int kdk_dm_get_device_hotplug_info(char *devicepath);

/**
 * @brief 获取系统上已连接的，指定类型的，设备列表
 *
 * @param type 设备类型
 * @return char** 设备的名称列表，失败返回NULL
 * @since 3.0.1.0
 */
extern char **kdk_dm_get_device_list_by_type(kdk_device_type type);

/**
 * @brief 获取系统上已连接的，指定厂商的，设备列表
 *
 * @param vendor 设备厂商
 * @return char** 设备的名称列表，失败返回NULL
 * @since 3.0.1.0
 */
extern char **kdk_dm_get_device_list_by_vendor(char *vendor);

/**
 * @brief 刷新系统上连接的设备列表
 * @since 3.0.1.0
 *
 */
extern void kdk_dm_refresh_device_list();

#ifdef __cplusplus
}
#endif

#endif // KYSDK_SYSTEM_DEVICE_MANAGER_H_