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

#include "stdio.h"
#include "../libkybios.h"
#include <stdlib.h>
int main()
{
    char *vendor = kdk_bios_get_vendor();
    char *version = kdk_bios_get_version();
    printf("vendor: %s\n", vendor);
    printf("version: %s\n",version);
    kdk_bios_free(vendor);
    kdk_bios_free(version);

    char *smbios_version = kdk_bios_get_smbios_version();
    printf("SMBIOS Version: %s\n", smbios_version);
    kdk_bios_free(smbios_version);

    char *bios_date = kdk_bios_get_date();
    printf("date: %s\n", bios_date);
    kdk_bios_free(bios_date);

    char *type = kdk_bios_get_type();
    printf("type: %s\n", type);
    kdk_bios_free(type);
    return 0;
}
