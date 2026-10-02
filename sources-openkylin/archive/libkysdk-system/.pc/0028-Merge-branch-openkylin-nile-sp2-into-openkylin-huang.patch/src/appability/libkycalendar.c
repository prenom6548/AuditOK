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

#include <dbus-1.0/dbus/dbus.h>
#include "stdlib.h"
#include "stdio.h"
#include <string.h>

int kdk_calendar_open_calendar(int day,int month,int year)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (dbus_error_is_set(&err))  // 检查是否有连接错误
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);  // 打印错误信息
        dbus_error_free(&err);  // 释放错误信息
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.ukui.calendar.Notifications",  // 目标服务名称
                                            "/org/ukui/calendar/Notifications", // 目标对象路径
                                            "org.ukui.calendar.Notifications",  // 接口名称
                                            "OpenSchedule");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 添加参数到消息中
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &day, DBUS_TYPE_INT32, &month, DBUS_TYPE_INT32, &year, DBUS_TYPE_INVALID))
    {
        ret = -1;  // 如果参数添加失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 发送消息，不需要等待返回值
    if (!dbus_connection_send(conn, info_msg, NULL))  // 不等待回复
    {
        ret = -1;  // 如果消息发送失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (info_msg)  // 如果消息对象不为空
    {
        dbus_message_unref(info_msg);  // 释放消息对象
    }

    dbus_connection_close(conn);  // 关闭 D-Bus 连接

    return ret;

out:
    return ret; 
}

int kdk_calendar_edit_schedule(int hour,int minute,int day,int month,const char* descript)
{
    int ret = 2;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (dbus_error_is_set(&err))  // 检查是否有连接错误
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);  // 打印错误信息
        dbus_error_free(&err);  // 释放错误信息
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;  // 挂起的消息调用
    DBusMessage *replyMsg = NULL;

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.ukui.calendar.Notifications",  // 目标服务名称
                                            "/org/ukui/calendar/Notifications", // 目标对象路径
                                            "org.ukui.calendar.Notifications",  // 接口名称
                                            "EditSchedule");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &hour, DBUS_TYPE_INT32, &minute, DBUS_TYPE_INT32, &day, DBUS_TYPE_INT32, &month, DBUS_TYPE_STRING, &descript, DBUS_TYPE_INVALID))
    {
        ret = -1;  // 如果参数添加失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = NULL;
        goto out;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        ret = NULL;
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        // klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        ret = NULL;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = NULL;
        goto out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    return ret; 

out:
    return ret; 
}

int kdk_calendar_delete_schedule(int hour,int minute,int day,int month,const char* descript)
{
    int ret = 2;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (dbus_error_is_set(&err))  // 检查是否有连接错误
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);  // 打印错误信息
        dbus_error_free(&err);  // 释放错误信息
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;  // 挂起的消息调用
    DBusMessage *replyMsg = NULL;

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.ukui.calendar.Notifications",  // 目标服务名称
                                            "/org/ukui/calendar/Notifications", // 目标对象路径
                                            "org.ukui.calendar.Notifications",  // 接口名称
                                            "DeleteSchedule");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &hour, DBUS_TYPE_INT32, &minute, DBUS_TYPE_INT32, &day, DBUS_TYPE_INT32, &month, DBUS_TYPE_STRING, &descript, DBUS_TYPE_INVALID))
    {
        ret = -1;  // 如果参数添加失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = NULL;
        goto out;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        ret = NULL;
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        // klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        ret = NULL;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = NULL;
        goto out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    return ret; 

out:
    return ret; 
}
   
int kdk_calendar_create_schedule(int hour,int minute,int day,int month,int year,int end_minute,int end_hour,const char* descript)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (dbus_error_is_set(&err))  // 检查是否有连接错误
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);  // 打印错误信息
        dbus_error_free(&err);  // 释放错误信息
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.ukui.calendar.Notifications",  // 目标服务名称
                                            "/org/ukui/calendar/Notifications", // 目标对象路径
                                            "org.ukui.calendar.Notifications",  // 接口名称
                                            "ScheduleNotification");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 添加参数到消息中
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &hour, DBUS_TYPE_INT32, &minute, DBUS_TYPE_INT32, &day, DBUS_TYPE_INT32, &month, DBUS_TYPE_INT32, &year, DBUS_TYPE_INT32, &end_minute, DBUS_TYPE_INT32, &end_hour, DBUS_TYPE_STRING, &descript, DBUS_TYPE_INVALID))
    {
        ret = -1;  // 如果参数添加失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 发送消息，不需要等待返回值
    if (!dbus_connection_send(conn, info_msg, NULL))  // 不等待回复
    {
        ret = -1;  // 如果消息发送失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (info_msg)  // 如果消息对象不为空
    {
        dbus_message_unref(info_msg);  // 释放消息对象
    }

    dbus_connection_close(conn);  // 关闭 D-Bus 连接

    return ret;

out:
    return ret; 
}