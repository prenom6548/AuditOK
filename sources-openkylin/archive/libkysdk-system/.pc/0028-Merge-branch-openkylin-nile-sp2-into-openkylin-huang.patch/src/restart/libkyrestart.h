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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef KYSDK_SYSTEM_RESTART_H_
#define KYSDK_SYSTEM_RESTART_H_

/**
 * @file libkyrestart.h
 * @author wangjingwen (wangjingwen@kylinos.cn)
 * @brief 系统重启模块
 * @version 2.5
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 * @defgroup libkysdk-restart
 * @since 3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _kdk_timeinfo{
    char *time;   // 时间
    char *timesec;  // 带秒钟数时间
}kdk_timeinfo;


#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief 设置指定时间后重启当前系统
 * 
 * @param min 经过多少分钟关机
 * 
 * @return 0成功 -1失败
 */
extern int kdk_restart_reboot(int min);

/**
 * @brief 设置在指定时间重启当前系统
 * 
 * @param time 关机时间
 * 
 * @return 0成功 -1失败
 */
extern int kdk_restart_schedule_reboot(struct tm restart_time);

/**
 * @brief 取消当前系统上所有的重启任务
 * 
 * @param void
 * 
 * @return true成功 false失败
 */
extern bool kdk_restart_cancel_reboot();

/**
 * @brief 系统上是否有重启任务
 * 
 * @param void
 * 
 * @return true成功 false失败
 */
extern bool kdk_restart_is_schedule_reboot();

 

#ifdef __cplusplus
}
#endif

#endif // KYSDK_SYSTEM_RESTART_H_