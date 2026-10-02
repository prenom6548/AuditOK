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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "../libkysensors.h"
#include <sensors/sensors.h>

void test_get_sensor_names() 
{
    // 调用函数获取传感器名称
    char **sensor_names = kdk_sensor_get_name();
    if (sensor_names == NULL) 
    {
        printf("Failed to get sensor names\n");
        return;
    }

    // 输出传感器名称
    printf("Detected sensors:\n");
    for (int i = 0; sensor_names[i] != NULL; i++) 
    {
        printf("%s\n", sensor_names[i]);
        free(sensor_names[i]); // 释放每个传感器名称的内存
    }
    free(sensor_names); // 释放传感器名称数组的内存
}

void test_get_adapter_name() 
{
    // 指定一个测试的传感器芯片名称
    char *sensor_name = "amdgpu-pci-0200";  // 你可以替换为实际存在的芯片名称
    char *adapter_name = kdk_sensor_get_adapter_name(sensor_name);

    // 检查是否成功获取适配器名称
    if (adapter_name)
    {
        printf("Adapter name for sensor '%s': %s\n", sensor_name, adapter_name);
        free(adapter_name);  // 释放适配器名称的内存
    }
    else
    {
        printf("Failed to get adapter name for sensor '%s'\n", sensor_name);
    }
}

void test_get_feature_value() 
{
    char *sensor_name = "amdgpu-pci-0200"; // 替换为你想测试的传感器名称
    kdk_sensor_info* info = kdk_sensor_get_feature_value(sensor_name);
    
    if (info) 
    {
        int num_of_features = 2;

        for (int i = 0; i < num_of_features; i++) 
        {
            printf("Feature %d: %s\n", i + 1, info->feature[i]);
        }

        printf("Subfeature count: %d\n", info->subfeature_count);

        if (info->subfeature_count > 0) 
        {
            for (int i = 0; i < info->subfeature_count; i++) 
            {
                printf("Subfeature %d:\n", i + 1);
                printf("  Name: %s\n", info->subfeatures[i].name);
                printf("  Value: %s\n", info->subfeatures[i].value);
                printf("  Unit: %s\n",info->subfeatures[i].unit);
            }
        } 
        else 
        {
            printf("No subfeatures found for sensor: %s\n", sensor_name);
        }

        free(info);
    }
    else 
    {
        printf("Failed to get sensor information for sensor: %s\n", sensor_name);
    }
}

int main() 
{
    // 执行并打印三个测试函数的结果
    printf("Running test_get_sensor_names()...\n");
    test_get_sensor_names();
    
    printf("\nRunning test_get_adapter_name()...\n");
    test_get_adapter_name();
    
    printf("\nRunning test_get_feature_value()...\n");
    test_get_feature_value();

    return 0;
}

