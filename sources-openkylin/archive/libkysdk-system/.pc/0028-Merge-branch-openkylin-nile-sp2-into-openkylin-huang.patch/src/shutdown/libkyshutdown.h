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

#ifndef KYSDK_SYSTEM_SHUTDOWN_H_
#define KYSDK_SYSTEM_SHUTDOWN_H_

/**
 * @file libkyshutdown.h
 * @author wangjingwen (wangjingwen@kylinos.cn)
 * @brief 系统关闭模块
 * @version 3.0.1.0
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 * @defgroup libkysdk-shutdown
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
#include <stdint.h>

/**
 * @brief 设置指定时间后关闭当前系统
 * 
 * @param min 经过多少分钟关机
 * 
 * @return 0成功 -1失败
 * @since 3.0.1.0
 */
extern int kdk_shutdown_power_off(int min);

/**
 * @brief 设置在指定时间关闭当前系统，只能是未来的时间
 * 
 * @param shutdown_time 关机时间的 tm 结构体
 * @return int 0成功，-1失败
 * @since 3.0.1.0
 */
extern int kdk_shutdown_schedule_power_off(struct tm shutdown_time);

/**
 * @brief 取消当前系统上所有的关机任务
 * @param void
 * 
 * @return true成功 false失败
 * @since 3.0.1.0
 */
extern bool kdk_shutdown_cancel_power_off();

/**
 * @brief 系统上是否有关机任务
 * 
 * @param void
 * 
 * @return true成功 false失败
 */
extern bool kdk_shutdown_is_schedule_power_off();

/**
 * @brief 注销指定用户，返回登录界面
 * 
 * @param user 用户名
 * 
 * @return 0成功 -1失败 
 */
extern int kdk_shutdown_exit_window(char* user);

/**
 * @brief 锁定当前用户的显示（进入锁屏页面）
 * 
 * @param void
 * 
 * @return 0成功 -1失败
 */
extern int kdk_shutdown_lock_work_station();

#ifdef __cplusplus
}
#endif

#endif // KYSDK_SYSTEM_SHUTDOWN_H_