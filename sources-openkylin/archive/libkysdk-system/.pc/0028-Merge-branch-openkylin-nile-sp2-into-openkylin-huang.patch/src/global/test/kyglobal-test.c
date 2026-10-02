/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
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
#include "../libkyglobal.h"
#include <stdlib.h>
#include <libkysysinfo.h>
int main()
{
    int index = 0;
    char **language = kdk_global_get_system_support_language();
    if (language)
    {
        while (language[index])
        {
            printf("index = %d, language = %s\n", index, language[index]);
            index++;
        }

        kdk_free_langlist(language);
    }

    bool match = kdk_global_get_region_match_language();
    printf("match = %d\n", match);

    bool rtl = kdk_global_get_rtl();
    printf("rtl = %d\n", rtl);

    int offset = kdk_global_get_raw_offset();
    printf("offset = %d\n", offset);

    char *eUser = kdk_system_get_eUser();
    char* system_language = kdk_global_get_system_language(eUser);
    printf("system_language = %s\n", system_language);
    return 0;
}
