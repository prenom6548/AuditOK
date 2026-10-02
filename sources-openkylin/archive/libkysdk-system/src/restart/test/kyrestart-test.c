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
 * Authors: JingWen Wang <wangjingwen@kylinos.cn>
 *
 */

#include "../libkyrestart.h"


int main()
{
    // test 1
    int res = kdk_restart_reboot(10);
    printf("res = %d \n", res);

    // test 2
    struct tm restart_time = {0};
    
    // 设置关机时间为2025年4月17日17:30:00
    restart_time.tm_year = 2025 - 1900;  // 年份从1900开始计算
    restart_time.tm_mon = 4 - 1;         // 月份0-11 (4月是3)
    restart_time.tm_mday = 17;           // 日期1-31
    restart_time.tm_hour = 17;           // 小时0-23
    restart_time.tm_min = 30;            // 分钟0-59
    restart_time.tm_sec = 0;             // 秒0-59
    restart_time.tm_isdst = -1;          // 自动判断夏令时
    
    // 验证时间设置是否正确
    time_t restart_timestamp = mktime(&restart_time);
    if (restart_timestamp == -1) {
        printf("Invalid time specified\n");
        return -1;
    }
    // 打印设置的时间
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &restart_time);

    int result = kdk_restart_schedule_reboot(restart_time);
    if (result == 0)
    {
        printf("系统将在 %s 重启\n", buf);
    }
    else
    {
        printf("设置重启失败\n");
    }

    // test 3
    bool result3 = kdk_restart_cancel_reboot();
    if (result3)
    {
        printf("成功取消了计划的重启\n");
    }
    else
    {
        printf("取消重启失败或没有计划的重启任务\n");
    }

    // test 4
    bool is_scheduled = kdk_restart_is_schedule_reboot();
    if (is_scheduled)
    {
        printf("当前有计划的重启任务\n");
    }
    else
    {
        printf("当前没有计划的重启任务\n");
    }
 
}
