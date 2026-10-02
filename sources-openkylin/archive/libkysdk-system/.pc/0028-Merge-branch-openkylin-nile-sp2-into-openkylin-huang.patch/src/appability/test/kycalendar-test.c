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

#include "../libkycalendar.h"
#include "stdio.h"

void test_kdk_calendar_open_calendar() 
{
    // 测试用例：2024年10月27日
    int day = 27;
    int month = 10;
    int year = 2024;

    // 调用 kdk_calendar_open_calendar() 函数并获取结果
    int result = kdk_calendar_open_calendar(day, month, year);

    // 检查结果并输出对应信息
    if (result == 1) 
    {
        printf("Test passed: kdk_calendar_open_calendar(%d, %d, %d) succeeded.\n", day, month, year);
    } 
    else 
    {
        printf("Test failed: kdk_calendar_open_calendar(%d, %d, %d) failed.\n", day, month, year);
    }
}

void test_kdk_calendar_edit_schedule() 
{
    // 测试用例：编辑一个日程
    int hour = 10;          // 小时
    int minute = 30;        // 分钟
    int day = 27;           // 日
    int month = 10;         // 月
    const char* descript = "重要会议"; // 新描述

    // 调用 kdk_calendar_edit_schedule() 函数并获取结果
    int result = kdk_calendar_edit_schedule(hour, minute, day, month, descript);

    // 检查结果并输出对应信息
    if (result == 1) 
    {
        printf("Test passed: kdk_calendar_edit_schedule() succeeded.\n");
    } 
    else if (result == 0) 
    {
        printf("Test failed: kdk_calendar_edit_schedule() failed (schedule not found or could not be edited).\n");
    } 
    else if (result == -1) 
    {
        printf("Test warning: kdk_calendar_edit_schedule() did not run normally (DBus service not running).\n");
    } 
    else 
    {
        printf("Test error: kdk_calendar_edit_schedule() returned unexpected value: %d\n", result);
    }
}

void test_kdk_calendar_delete_schedule() 
{
    // 测试用例：删除一个日程
    int hour = 10;          // 小时
    int minute = 30;        // 分钟
    int day = 27;           // 日
    int month = 10;         // 月
    const char* descript = "重要会议"; // 假设我们要删除的日程描述

    // 调用 kdk_calendar_delete_schedule() 函数并获取结果
    int result = kdk_calendar_delete_schedule(hour, minute, day, month, descript);

    // 检查结果并输出对应信息
    if (result == 1) 
    {
        printf("Test passed: kdk_calendar_delete_schedule() succeeded.\n");
    } 
    else if (result == 0) 
    {
        printf("Test failed: kdk_calendar_delete_schedule() failed (schedule not found or could not be deleted).\n");
    } 
    else if (result == -1) 
    {
        printf("Test warning: kdk_calendar_delete_schedule() did not run normally (DBus service not running).\n");
    } 
    else 
    {
        printf("Test error: kdk_calendar_delete_schedule() returned unexpected value: %d\n", result);
    }
}

void test_kdk_calendar_create_schedule() 
{
    // 测试用例：创建一个日程
    int hour = 10;
    int minute = 30;
    int day = 27;
    int month = 10;
    int year = 2024;
    int end_hour = 11;
    int end_minute = 30;
    const char* descript = "重要会议";

    // 调用 kdk_calendar_create_schedule() 函数并获取结果
    int result = kdk_calendar_create_schedule(hour, minute, day, month, year, end_minute, end_hour, descript);

    // 检查结果并输出对应信息
    if (result == 1) 
    {
        printf("Test passed: kdk_calendar_create_schedule() succeeded.\n");
    } 
    else 
    {
        printf("Test failed: kdk_calendar_create_schedule() failed.\n");
    }
}

int main() 
{
    // 执行测试函数
    test_kdk_calendar_open_calendar();
    // test_kdk_calendar_edit_schedule();
    // test_kdk_calendar_delete_schedule(); 
    // test_kdk_calendar_create_schedule();

    return 0;
}