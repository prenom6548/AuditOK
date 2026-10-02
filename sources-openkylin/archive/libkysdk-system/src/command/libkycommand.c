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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <stdio.h>


#include <libkysysinfo.h>

static void Traverse_production()
{
    const char *production = kdk_system_get_production_line();
    printf("production line : %s\n", production);
}

static void Traverse_major()
{
    const char *major = kdk_system_get_major_version();
    printf("major version : %s\n", major);
}

static void Traverse_minor()
{
    const char *minor = kdk_system_get_minor_version();
    printf("minor version : %s\n", minor);
}

static void Traverse_alias()
{
    const char *alias = kdk_system_get_version_alias();
    printf("version alias : %s\n", alias);
}

static void Traverse_serial()
{
    const char *serial = kdk_system_get_serial_name();
    printf("serial name : %s\n", serial);
}

static void Traverse_inner()
{
   kdk_production_inner_info info = kdk_system_get_production_inner_version();
    printf("production line : %s\n", info.production_line);
    printf("major version : %s\n", info.major_ver);
    printf("minor version : %s\n", info.minor_ver);
    printf("version alias : %s\n", info.alias);
    printf("serial name : %s\n", info.serial_name);

    const kdk_custom_info custom = kdk_system_get_custom_version();
    if(strcmp(custom.version_type, "是") == 0  || strcmp(custom.version_type, "Unknown") == 0)
    {
        printf("is customized : %s\n", custom.version_type);
        printf("customized type : %s\n", custom.custom_type);
    }
    else{
        const char *minor = kdk_system_get_minor_version();
        printf("is customized : %s\n", custom.version_type);
        // printf("minor version : %s\n", minor);
    }
}

static void Traverse_custom()
{
    const kdk_custom_info custom = kdk_system_get_custom_version();
    if(strcmp(custom.version_type, "是") == 0  || strcmp(custom.version_type, "Unknown") == 0)
    {
        printf("is customized : %s\n", custom.version_type);
        printf("customized type : %s\n", custom.custom_type);
        printf("customized info : %s\n", custom.custom_info);
    }
    else{
        const char *minor = kdk_system_get_minor_version();
        printf("is customized : %s\n", custom.version_type);
        printf("minor version : %s\n", minor);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 3)
    {
        if(argc == 2)
        {
            if(0 == strcmp(argv[1], "--help"))
            {
                printf("Usage:\n"
                "    kylin-sysinfo <option> --[target]\n"
                "option:\n"
                "    version               System version information\n"
                "target:\n"
                "    help                  Show this information\n"
                "    production            Show system production line\n"
                "    major                 Show system major version\n"
                "    minor                 Show system minor version\n"
                "    alias                 Show system version alias\n"
                "    serial                Show system serial name\n"
                "    customization         Show system customization version information\n"
                "    all                   Show system version information\n"
                "\n");
                return 0;
            }
            else{
                printf("please use like >> kylin-sysinfo <option> --[target]\n");
                printf("option:\n"
                    "    version\n");

                printf("target:\n");
                printf("    production");
                printf("\tmajor");
                printf("\tminor");
                printf("\talias");
                printf("\tserial");
                printf("\tcustomization");
                printf("\tall");
                printf("\n");
                printf("Enter kylin-sysinfo --help to obtain relevant help for the version information command\n");
                return 0;
            }
        }
        else{
            printf("please use like >> kylin-sysinfo <option> --[target]\n");
            printf("option:\n"
                "    version\n");

            printf("target:\n");
            printf("    production");
            printf("\tmajor");
            printf("\tminor");
            printf("\talias");
            printf("\tserial");
            printf("\tcustomization");
            printf("\tall");
            printf("\n");
            printf("Enter kylin-sysinfo --help to obtain relevant help for the version information command\n");
            return 0;
        }
    }

    if(0 == strcmp(argv[1], "version"))
    {
        if (0 == strcmp(argv[2], "--production"))
            Traverse_production();
        else if (0 == strcmp(argv[2], "--major"))
            Traverse_major();
        else if (0 == strcmp(argv[2], "--minor"))
            Traverse_minor();
        else if (0 == strcmp(argv[2], "--alias"))
            Traverse_alias();
        else if (0 == strcmp(argv[2], "--serial"))
            Traverse_serial();
        else if (0 == strcmp(argv[2], "--all"))
            Traverse_inner();
        else if (0 == strcmp(argv[2], "--customization"))
            Traverse_custom();
        else
        {
            printf("please use like >> kylin-sysinfo <option> --[target]\n");
            printf("option:\n"
                "    version\n");

            printf("target:\n");
            printf("    production");
            printf("\tmajor");
            printf("\tminor");
            printf("\talias");
            printf("\tserial");
            printf("\tcustomization");
            printf("\tall");
            printf("\n");
            printf("Enter kylin-sysinfo -- help to obtain relevant help for the version information command\n");
            return 0;
        }
    }
    else
    {
        printf("please use like >> kylin-sysinfo <option> --[target]\n");
        printf("option:\n"
            "    version\n");

        printf("target:\n");
        printf("    production");
        printf("\tmajor");
        printf("\tminor");
        printf("\talias");
        printf("\tserial");
        printf("\tcustomization");
        printf("\tall");
        printf("\n");
        printf("Enter kylin-sysinfo -- help to obtain relevant help for the version information command\n");
        return 0;
    }
}