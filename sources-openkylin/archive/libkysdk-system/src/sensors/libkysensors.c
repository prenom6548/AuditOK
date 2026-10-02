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
 * Authors: JinBao Wang <wangjinbao@kylinos.cn>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libkylog.h>
#include <sensors/sensors.h> // 包含传感器库的头文件
#include "libkysensors.h"

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif
                                
// 获取系统连接的所有传感器芯片名称
char** kdk_sensor_get_name() 
{
    int err;
    int nr = 0;
    const sensors_chip_name *chip;
    char **sensor_names = NULL;
    int index = 0;
    int max_sensors = 100; // 预分配的传感器名称数组大小

    // 初始化传感器库
    err = sensors_init(NULL);
    if (err) 
    {
        klog_err("Failed to initialize sensors library\n");
        return NULL;
    }

    // 分配存储传感器名称的数组
    sensor_names = malloc((max_sensors + 1) * sizeof(char *)); // +1 用于 NULL 结束符
    if (sensor_names == NULL) 
    {
        klog_err("Failed to allocate memory");
        sensors_cleanup();
        return NULL;
    }

    // 获取所有检测到的芯片
    while ((chip = sensors_get_detected_chips(NULL, &nr)) != NULL) 
    {
        char chip_name[256];
        // 获取芯片名称
        if (sensors_snprintf_chip_name(chip_name, sizeof(chip_name), chip) < 0) 
        {
            klog_err("Failed to format chip name\n");
            continue;
        } 
        else 
        {
            // 如果数组已满，则扩展数组
            if (index >= max_sensors) {
                max_sensors *= 2;
                sensor_names = realloc(sensor_names, (max_sensors + 1) * sizeof(char *));
                if (sensor_names == NULL) {
                    klog_err("Failed to reallocate memory");
                    sensors_cleanup();
                    return NULL;
                }
            }
            // 将芯片名称添加到传感器名称数组中
            sensor_names[index] = strdup(chip_name);
            if (sensor_names[index] == NULL) {
                klog_err("Failed to duplicate string");
                for (int i = 0; i < index; i++) 
                {
                    free(sensor_names[i]);  // 释放已分配的字符串
                }
                free(sensor_names);  // 释放数组本身
                sensors_cleanup();
                return NULL;
            }
            index++;
        }
    }

    // 终止数组
    sensor_names[index] = NULL;

    // 清理资源
    sensors_cleanup();

    return sensor_names;
}

void kdk_free_name(char **name)
{
    if (!name)  // 如果传入的指针为空，直接返回
        return;

    for (int i = 0; name[i]; i++) 
    {
        free(name[i]);  // 释放每个传感器名称的内存
    }

    free(name);  // 释放存储指针数组的内存
}

// 获取传感器适配器名称
char* kdk_sensor_get_adapter_name(char *sensor) 
{
    if (sensor == NULL || strnlen(sensor, STRNLEN_MAX) == 0) 
    {
        klog_err("Invalid sensor name\n");
        return NULL;
    }

    int err;
    int nr = 0;
    const sensors_chip_name *chip = NULL;

    err = sensors_init(NULL);
    if (err) 
    {
        klog_err("Failed to initialize sensors library\n");
        return NULL;
    }

    // 遍历所有检测到的芯片
    while ((chip = sensors_get_detected_chips(NULL, &nr)) != NULL) 
    {
        char chip_name[256];

        // 获取芯片名称字符串
        if (sensors_snprintf_chip_name(chip_name, sizeof(chip_name), chip) >= 0) 
        {
            // 如果找到匹配的芯片名称
            if (strcmp(chip_name, sensor) == 0) 
            {
                // 使用找到的芯片结构体的 bus 字段获取适配器名称
                const char *adapter_name = sensors_get_adapter_name(&(chip->bus));
                if (adapter_name != NULL) 
                {
                    // 返回适配器名称的副本
                    sensors_cleanup();
                    return strdup(adapter_name);
                }
                break; // 找到匹配的芯片后退出循环
            }
        }
    }

    klog_err("Failed to get adapter name for sensor '%s'\n", sensor);

    // 清理资源
    sensors_cleanup();
    return NULL;
}

// 获取传感器芯片上的功能和对应数值
kdk_sensor_info* kdk_sensor_get_feature_value(char *sensor) 
{
    int err;
    kdk_sensor_info *info = (kdk_sensor_info *)malloc(sizeof(kdk_sensor_info));
    if (info == NULL) 
    {
        klog_err("Failed to allocate memory for kdk_sensor_info\n");
        return NULL;  // 在内存分配失败时，返回 NULL
    }
    memset(info, 0, sizeof(info));  // 初始化结构体

    err = sensors_init(NULL);
    if (err) 
    {
        klog_err("Failed to initialize sensors library\n");
        free(info);  // 在初始化传感器库失败时释放已分配的内存
        return NULL;
    }

    const sensors_chip_name *chip = NULL;
    int nr = 0;

    // 遍历所有检测到的芯片，查找匹配的芯片
    while ((chip = sensors_get_detected_chips(NULL, &nr)) != NULL) 
    {
        char chip_name[256];
        if (sensors_snprintf_chip_name(chip_name, sizeof(chip_name), chip) >= 0) 
        {
            if (strcmp(chip_name, sensor) == 0) 
            {
                // 如果匹配，开始获取功能信息
                const sensors_feature *feature;
                int feature_nr = 0;
                
                // 初始化计数器
                int subfeature_count = 0;
                
                // 统计子功能数量
                while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) 
                {
                    int subfeature_nr = 0;
                    const sensors_subfeature *subfeature;
                    while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL) 
                    {
                        if (subfeature->flags & SENSORS_MODE_R) 
                        {
                            subfeature_count++;
                        }
                    }
                }

                // 重新遍历，填充子功能数据
                feature_nr = 0;
                int current_index = 0;
                int feature_index = 0;

                // 遍历每个主功能
                while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) 
                {
                    // 复制功能的标签名称到info结构体
                    strncpy(info->feature[feature_index], sensors_get_label(chip, feature), sizeof(info->feature[feature_index]) - 1);
                    info->feature[1][sizeof(info->feature[feature_index]) - 1] = '\0';  // 确保字符串以NULL结尾
                    
                    int subfeature_nr = 0;
                    const sensors_subfeature *subfeature;
                    
                    // 遍历功能的所有子功能
                    while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL) 
                    {
                        if (subfeature->flags & SENSORS_MODE_R) // 如果子功能可读
                        {
                            // 复制子功能的名称到info结构体
                            strncpy(info->subfeatures[current_index].name, subfeature->name, sizeof(info->subfeatures[current_index].name) - 1);
                            info->subfeatures[current_index].name[sizeof(info->subfeatures[current_index].name) - 1] = '\0';  // 确保字符串以NULL结尾
                            
                            double value;
                            if (sensors_get_value(chip, subfeature->number, &value) == 0) 
                            {
                                 // 将子功能的值格式化为字符串存储到info结构体中
                                snprintf(info->subfeatures[current_index].value, sizeof(info->subfeatures[current_index].value), "%f", value);
                            }  
                            else 
                            {
                                // 处理获取子功能值失败的情况
                                klog_err("Failed to get value for subfeature '%s'\n", subfeature->name);
                                strncpy(info->subfeatures[current_index].value, "N/A", sizeof(info->subfeatures[current_index].value));
                            }
                            
                            // 根据不同的主功能类型设置单位
                            switch (feature->type) 
                            {
                                case SENSORS_FEATURE_TEMP:
                                    strncpy(info->subfeatures[current_index].unit, "°C", sizeof(info->subfeatures[current_index].unit) - 1);
                                    break;
                                case SENSORS_FEATURE_IN:
                                    strncpy(info->subfeatures[current_index].unit, "V", sizeof(info->subfeatures[current_index].unit) - 1);
                                    break;
                                case SENSORS_FEATURE_FAN:
                                    strncpy(info->subfeatures[current_index].unit, "RPM", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                case SENSORS_FEATURE_VID:
                                    strncpy(info->subfeatures[current_index].unit, "V", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                case SENSORS_FEATURE_POWER:
                                    strncpy(info->subfeatures[current_index].unit, "W", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                case SENSORS_FEATURE_ENERGY:
                                    strncpy(info->subfeatures[current_index].unit, "J", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                case SENSORS_FEATURE_CURR:
                                    strncpy(info->subfeatures[current_index].unit, "A", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                case SENSORS_FEATURE_HUMIDITY:
                                    strncpy(info->subfeatures[current_index].unit, "RH", sizeof(info->subfeatures[current_index].unit) - 1);    
                                    break;
                                default:
                                continue;
                            }
                        }
                        current_index++;
                    }
                    feature_index++;
                }
                info->subfeature_count = subfeature_count;
                break;  // 找到匹配的芯片后，退出循环
            }
        }
    }
    if (info->subfeature_count == 0) 
    {
        klog_err("Failed to get feature values for sensor '%s'\n", sensor);
        free(info);  
        sensors_cleanup(); 
        return NULL;
    }

    sensors_cleanup(); 
    return info;
}





