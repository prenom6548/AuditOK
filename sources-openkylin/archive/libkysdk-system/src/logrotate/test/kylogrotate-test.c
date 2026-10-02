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

#include <stdio.h>
#include <libkylog.h>
#include <unistd.h>
#include "../libkylogrotate.h"

int main()
{
    int ret = 0;
    // while(1)
    // {
    //     klog_err("logrotate 测试程序\n");
    //     usleep(500);
    // }
    
    // KdkLogrotateConfig *config = calloc(1, sizeof(KdkLogrotateConfig));
    // kdk_logrotate_config_init(config);
    
    // char *files[2] = {"/home/kylin/.log/test.log", NULL};
    // ret = kdk_logrotate_config_create(files, config);
    // printf("ret = %d\n", ret);
}
