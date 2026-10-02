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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "libkybios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

const char *kdk_bios_get_vendor()
{
    char *vendor = (char *)calloc(32, sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_vendor", "r");
    if (fd)
    {
        fgets(vendor, 32, fd);
        fclose(fd);
    }
    int i = 0;
    while (vendor[i] && vendor[i] != '\n')
        i++;
    vendor[i] = '\0';

    return vendor;
}

const char *kdk_bios_get_version()
{
    char *version = (char *)calloc(128, sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_version", "r");
    if (fd)
    {
        fgets(version, 128, fd);
        fclose(fd);
    }
    int i = 0;
    while (version[i] && version[i] != '\n')
        i++;
    version[i] = '\0';
    return version;
}

void kdk_bios_free(char *info)
{
    free(info);
}

char *kdk_bios_get_smbios_version()
{
    int fd;
    char buffer[128];
    memset(buffer, 0, 128);
    char smbios_version[2];

    fd = open("/sys/firmware/dmi/tables/smbios_entry_point", O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        return NULL;
    }
    if (read(fd, buffer, 128) <= 0)
    {
        perror("Error reading file");
        close(fd);
        return NULL;
    }

    smbios_version[0] = buffer[6];
    smbios_version[1] = buffer[6 + 1];

    close(fd);

    snprintf(buffer, sizeof(buffer), "%u.%u", smbios_version[0], smbios_version[1]);

    return strdup(buffer);
}

char *kdk_bios_get_date()
{
    char *date = (char *)calloc(32, sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_date", "r");
    if (fd)
    {
        fgets(date, 32, fd);
        fclose(fd);
    }
    int i = 0;
    while (date[i] && date[i] != '\n')
        i++;
    date[i] = '\0';
    return date;
}

char *kdk_bios_get_type()
{
    struct stat sb;

    if (stat("/sys/firmware/efi", &sb) == 0)
    {
        if (S_ISDIR(sb.st_mode))
        {
            return strdup("UEFI");
        }
    }

    return "Legacy";
}
