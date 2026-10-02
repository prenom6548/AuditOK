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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "libkydiskinfo.h"
#include "cdrom/datacdrom.h"
#include <string.h>

char *kdk_udf_get_cdrom_type(const char *device_path)
{

    KYSDKUdfBurn::DataCDROM cdrom(device_path);
    cdrom.getCDROMInfo();

    char *result = strdup(cdrom.getCDROMType().c_str());

    return result;
}

unsigned long kdk_udf_get_cdrom_capacity(const char *device_path)
{
    KYSDKUdfBurn::DataCDROM cdrom(device_path);
    cdrom.getCDROMInfo();

    unsigned long long result = cdrom.getCDROMCapacity();

    return result;
}

unsigned long kdk_udf_get_cdrom_used_capacity(const char *device_path)
{
    KYSDKUdfBurn::DataCDROM cdrom(device_path);
    cdrom.getCDROMInfo();

    unsigned long long result = cdrom.getCDROMUsedCapacity();

    return result;
}
