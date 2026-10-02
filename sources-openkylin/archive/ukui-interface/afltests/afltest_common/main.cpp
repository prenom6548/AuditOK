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

int main(int argc, char **argv)
{
    char value[VALUE_MAX_LEN] = {0};
    if (argc < 2) {
        puts("I need more args");
        return 0;
    }

    if (!strcmp(argv[1], "lsb")) {
        if (argc < 3) {
            puts("I need more args");
            return 0;
        }
        kdk_get_lsbrelease(argv[2], value, VALUE_MAX_LEN);
        KDKGetLSBRelease(argv[2]);
    } else if (!strcmp(argv[1], "os")){
        if (argc < 3) {
            puts("I need more args");
            return 0;
        }
        kdk_get_osrelease(argv[2], value, VALUE_MAX_LEN);
        KDKGetOSRelease(argv[2]);
    } else if (!strcmp(argv[1], "kyinfo")) {
        if (argc < 4) {
            puts("I need more args");
            return 0;
        }
        kdk_get_kyinfo(argv[2], argv[3], value, VALUE_MAX_LEN);
        KDKGetKYInfo(argv[2], argv[3]);
    } else if (!strcmp(argv[1], "prjname")) {
        kdk_get_prjcodename(value, VALUE_MAX_LEN);
        KDKGetPrjCodeName();
    } else if (!strcmp(argv[1], "cpu")) {
        kdk_get_cpumodelname(value, VALUE_MAX_LEN);
        KDKGetCpuModelName();
    } else if (!strcmp(argv[1], "spec")) {
        kdk_get_spechdplatform(value, VALUE_MAX_LEN);
        KDKGetSpecHDPlatform();
    } else if (!strcmp(argv[1], "virt")) {
        kdk_get_virtenv(value, VALUE_MAX_LEN);
        KDKGetVirtEnv();
    }
    return 0;
}
