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


#ifndef LIBKYSENSOR_H
#define LIBKYSENSOR_H

/**
 * @file libkysensor.h
 * @author wangjinbao (wangjinbao@kylinos.cn)
 * @brief 传感器模块
 * @version 0.1
 * @date 2025-4-11
 * 
 * @copyright Copyright (c) 2025
 * @defgroup libkysdk-sensors
 * @{
 * @since 3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

// 定义 SensorSubFeature 结构体
typedef struct {
    char name[256];   // 子功能名称
    char value[256];  // 子功能值
    char unit[64];    // 单位
} SensorSubFeature;

// 定义 kdk_sensor_info 结构体
typedef struct {
    char feature[10][256];           // 功能名称
    SensorSubFeature subfeatures[256]; // 子功能结构体信息
    int subfeature_count;        // 子模块数量
} kdk_sensor_info;

/**
 * @brief 获取系统连接的所有传感器芯片名称
 * 
 * @return char** 返回指向传感器芯片名称的字符串数组的指针，数组的最后一个元素为 NULL。如果操作失败，返回 NULL。
 */
extern char** kdk_sensor_get_name();

/**
 * @brief 释放由kdk_sensor_get_name返回的传感器名称列表
 * 
 * @param name 由kdk_sensor_get_name返回的字符串指针数组
 */
extern void kdk_free_name(char** name);

/**
 * @brief 获取传感器适配器名称
 * 
 * @param sensor 传感器芯片名称
 * @return char* 传感器适配器名称的字符串。调用者负责释放该字符串的内存。
 */
extern char* kdk_sensor_get_adapter_name(char *sensor);

/**
 * @brief 获取传感器芯片上的功能和对应数值
 * 
 * @param sensor 传感器芯片名称
 * @return kdk_senser_info 包含传感器芯片功能和对应数值的结构体
 */
extern kdk_sensor_info* kdk_sensor_get_feature_value(char *sensor);

#ifdef __cplusplus
}
#endif

#endif // LIBKYSENSOR_H

/**
 * \example kysdk-system/src/realtime/test/kyrealtime-test.c
 * 
 */

