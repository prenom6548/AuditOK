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

#include "../libkydiskinfo.h"
#include <stdio.h>
#include <stdlib.h>

#define TOGB(x) ((double)x) / (1024.0 * 1024.0 * 1024.0)
int main(int argc, char *argv[])
{
    
    char *media_type = kdk_udf_get_cdrom_type("/dev/sr1");
    unsigned long long capacity = kdk_udf_get_cdrom_capacity("/dev/sr1");
    unsigned long long used_capacity = kdk_udf_get_cdrom_used_capacity("/dev/sr1");
    printf("Media type: %s\n", media_type);
    printf("Capacity: %.2fGB\n", TOGB(capacity));
    printf("Used capacity: %.2fGB\n", TOGB(used_capacity));
    printf("Free capacity: %.2fGB\n", TOGB(capacity - used_capacity));
}
