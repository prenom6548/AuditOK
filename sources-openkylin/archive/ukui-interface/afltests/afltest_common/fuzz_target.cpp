/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kylin-com4c.h"
#include "kylin-com4cxx.h"

#define VALUE_MAX_LEN 128

// 这是 libFuzzer 需要的入口函数
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char value[VALUE_MAX_LEN] = {0};
    // 处理lsb-release读取
    kdk_get_lsbrelease((const char*)(data), value, VALUE_MAX_LEN);
    KDKGetLSBRelease((const char*)(data));
    // 处理os-release读取
    kdk_get_osrelease((const char*)(data), value, VALUE_MAX_LEN);
    KDKGetOSRelease((const char*)(data));
    // 处理kyinfo读取
    if (size >= 2) {
        size_t mid = (size_t)(size/2);
        char *str_data = (char *)malloc(mid + 1); // 分配额外的空间用于字符串终止符
        if (str_data == NULL) return 0; // 内存分配失败处理
        memcpy(str_data, data, mid);
        str_data[mid] = '\0'; // 确保字符串以 null 结尾
        kdk_get_kyinfo(str_data, (const char*)(&data[mid]), value, VALUE_MAX_LEN);
        KDKGetKYInfo(str_data, (const char*)(&data[mid]));
        free(str_data); // 释放分配的内存
    }
    // 处理prjname读取
    kdk_get_prjcodename(value, VALUE_MAX_LEN);
    KDKGetPrjCodeName();
    // 处理cpumode读取
    kdk_get_cpumodelname(value, VALUE_MAX_LEN);
    KDKGetCpuModelName();
    // 处理specplatform读取
    kdk_get_spechdplatform(value, VALUE_MAX_LEN);
    KDKGetSpecHDPlatform();
    // 处理virtual mode读取
    kdk_get_virtenv(value, VALUE_MAX_LEN);
    KDKGetVirtEnv();
    return 0;
}
