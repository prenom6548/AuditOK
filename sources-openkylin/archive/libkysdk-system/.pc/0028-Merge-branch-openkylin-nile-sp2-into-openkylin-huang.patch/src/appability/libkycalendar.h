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

#ifndef LIBKYCALENDAR_H
#define LIBKYCALENDAR_H

/**
 * @file        libkycalendar.h
 * @brief       应用能力模块
 * @author      wangjinbao (wangjinbao@kylinos.cn)
 * @version     0.1
 * @date        2024-10-28
 * @copyright   Copyright (c) 2025
 * @defgroup    libkysdk-appability
 * @since       3.0.1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 打开日历
 *
 * @param day 日
 * @param month 月
 * @param year 年
 * @return int 成功返回 1，失败返回 -1
 */
extern int kdk_calendar_open_calendar(int day,int month,int year);

/**
 * @brief 编辑日程
 *
 * @param hour 小时
 * @param minute 分钟
 * @param day 日
 * @param month 月
 * @param descript 描述
 * @return int 成功返回1，失败返回0，Dbus服务未正常运行返回-1
 */
extern int kdk_calendar_edit_schedule(int hour,int minute,int day,int month,const char* descript);

/**
 * @brief 删除日程
 * 
 * @param hour 小时
 * @param minute 分钟
 * @param day 日
 * @param month 月
 * @param descript 描述
 * @return int 成功返回1，失败返回0，Dbus服务未正常运行返回-1
 */
extern int kdk_calendar_delete_schedule(int hour,int minute,int day,int month,const char* descript);

/**
 * @brief 创建日程
 * 
 * @param hour 小时
 * @param minute 分钟
 * @param day 日
 * @param month 月
 * @param year 月
 * @param end_minute 结束分钟
 * @param end_hour 结束小时
 * @param descript 描述
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_calendar_create_schedule(int hour,int minute,int day,int month,int year,int end_minute,int end_hour,const char* descript);

#ifdef __cplusplus
}
#endif

#endif // LIBKYSENSOR_H


