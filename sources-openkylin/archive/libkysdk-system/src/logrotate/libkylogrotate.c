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

#include "libkylogrotate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

// 获取单位枚举对应的字符串表示
static const char *unit_to_str(KdkLogrotateUnit unit)
{
    switch (unit)
    {
    case KDK_KB:
        return "k";
    case KDK_MB:
        return "m";
    case KDK_GB:
        return "g";
    default:
        return "";
    }
}

// 构建压缩命令字符串
static void build_compress_cmd(const KdkLogrotateConfig *cfg, char *buf, size_t size)
{
    if (!cfg->compress)
    {
        buf[0] = '\0';
        return;
    }

    const char *mode = (cfg->compress_ext == KDK_COMPRESS_BZIP2) ? "bzip2" : "xz";
    snprintf(buf, size, "compresscmd /usr/bin/%s", mode);
}
void kdk_logrotate_config_init(KdkLogrotateConfig *config)
{
    memset(config, 0, sizeof(KdkLogrotateConfig));
    config->frequency = KDK_DAILY;
    config->rotate = 7;
    config->compress = true;
    config->missingok = true;
    config->notifempty = true;
    config->copytruncate = true;
}

// 主要实现函数
int kdk_logrotate_config_create(const char **config_path, const KdkLogrotateConfig *config)
{
    if (!config_path || !config)
    {
        return LOGROTATE_INVALID_PATH;
    }

    if ( ((config->frequency == KDK_NONE) && (config->by_size == false)) ||
            (config->rotate < 0) ||
            ((config->by_size == true) && (config->max_size <= 0)))
    {
        return LOGROTATE_INVALID_CONFIG;
    }

    const char *basename = strrchr(config_path[0], '/');

    char path[4096];
    snprintf(path, sizeof(path), "/etc/logrotate.d/%s", basename + 1);

    // printf("%s\n", path);

    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        // printf("open %s failed: %s\n", path, strerror(errno));
        return LOGROTATE_PERM_DENIED;
    }

    // 开始写入配置
    for (int i = 0; config_path[i]; i++)
    {
        fprintf(fp, "%s", config_path[i]);
    }
    
    fprintf(fp, "{\n");

    const char *frequency = NULL;
    switch (config->frequency)
    {
    case KDK_DAILY:
        frequency = "daily";
        break;
    case KDK_WEEKLY:
        frequency = "weekly";
        break;
    case KDK_MONTHLY:
        frequency = "monthly";
        break;
    case KDK_YEARLY:
        frequency = "yearly";
        break;
    default:
        frequency = NULL;
    }

    if(frequency)
    {
        fprintf(fp, "    %s\n", frequency);
        fprintf(fp, "    rotate %d\n", config->rotate);
    }

    if (config->by_size)
    { 
        fprintf(fp, "    size %d%s\n", config->max_size, unit_to_str(config->unit));
    }

    if (config->compress)
    {
        fprintf(fp, "    compress\n");
        if (config->delaycompress)
        {
            fprintf(fp, "    delaycompress\n");
        }
    }

    if (config->missingok)
    {
        fprintf(fp, "    missingok\n");
    }

    if (config->notifempty)
    {
        fprintf(fp, "    notifempty\n");
    }

    if (config->copytruncate)
    {
        fprintf(fp, "    copytruncate\n");
    }

    if (config->dateext)
    {
        fprintf(fp, "    dateext\n");
        if (strnlen(config->dateformat, STRNLEN_MAX) > 0)
        {
            fprintf(fp, "    dateformat %s\n", config->dateformat);
        }
    }

    if (strnlen(config->extension, STRNLEN_MAX) > 0)
    {
        fprintf(fp, "    extension %s\n", config->extension);
    }

    if (config->nocreate)
    {
        fprintf(fp, "    nocreate\n");
    }
    
    if (!config->nocreate && !config->copytruncate)
    {
        fprintf(fp, "    create\n");
        if (strnlen(config->create_mode, STRNLEN_MAX) > 0)
        {
            fprintf(fp, "    create_mode %s\n", config->create_mode);
        }
        if (strnlen(config->create_owner, STRNLEN_MAX) > 0)
        {
            fprintf(fp, "    create_owner %s\n", config->create_owner);
        }
    }

    if (strnlen(config->postrotate, STRNLEN_MAX) > 0)
    {
        fprintf(fp, "    postrotate\n        %s\n    endscript\n", config->postrotate);
    }

    if (config->sharedscripts)
    {
        fprintf(fp, "    sharedscripts\n");
    }

    fprintf(fp, "}\n");
    fclose(fp);

    return LOGROTATE_OK;
}
