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
 * Authors: JinBao Wang <wangjinbao@kylinos.cn>
 *
 */

#include <dbus-1.0/dbus/dbus.h>
#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include <libkylog.h>

int open_video() 
{
    // 定义命令，直接打开音乐播放器
    // 主命令和备用命令
    const char *primary_command = "/usr/bin/kylin-video";
    const char *fallback_command = "kaiming";
    const char *arg1 = "run";
    const char *arg2 = "top.openkylin.kylin-video";

    // 使用 fork 创建子进程
    pid_t pid = fork();
    
    if (pid < 0) 
    {
        // fork 失败
        klog_err("Fork failed\n");
        return -1;  // 失败返回 -1
    } 
    else if (pid == 0) 
    {
        // 子进程
        // 创建新会话，成为新的会话领导
        if (setsid() < 0) 
        {
            klog_err("setsid failed\n");
            exit(1);  // 退出子进程
        }

        // 首先尝试执行主命令
        execlp(primary_command, primary_command, (char *)NULL);
        
        // 如果 execlp 返回，说明主命令执行失败
        klog_info("Primary command failed, trying fallback command...\n");
        
        // 尝试执行备用命令
        execlp(fallback_command, fallback_command, arg1, arg2, (char *)NULL);
        
        // 如果两个命令都失败
        klog_err("Both commands failed\n");
        exit(1);  // 退出子进程
    } 
    else 
    {
        // 父进程可以选择继续运行而不等待子进程
        return 1;  // 成功返回 1
    }
}

int kdk_video_next()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Next");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return ret; 
}

int kdk_video_add_file(const char* trackid)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "AddFile");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 添加参数到消息中
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &trackid, DBUS_TYPE_INVALID))
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

char* kdk_video_current_file()
{
    char* ret = NULL;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;  // 挂起的消息调用
    DBusMessage *replyMsg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = NULL;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = NULL;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "CurrentFile");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = NULL;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_exit()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Exit");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_full_screen()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "FullScreen");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_kv_play_pause()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "KvPlayPause");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_loop_mode()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "LoopMode");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_play_state()
{
    int ret = 2;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;  // 挂起的消息调用
    DBusMessage *replyMsg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "PlayState");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = -1;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        ret = -1;
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
        ret = -1;
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
        ret = -1;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        ret = -1;
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_previous()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Previous");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_volume_down()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "VolumeDown");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_volume_up()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "VolumeUp");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

char* kdk_video_get_play_back_state()
{
    char* ret = NULL;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;  // 挂起的消息调用
    DBusMessage *replyMsg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = NULL;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = NULL;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "getPlaybackState");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = NULL;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_pause()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Pause");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_play_pause()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "PlayPause");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_stop()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Stop");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_play()
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Play");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    return ret; 
}

int kdk_video_seek(int offset)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "Seek");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    int64_t offset64 = (int64_t)offset;

    // 添加参数到消息中
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT64, &offset64, DBUS_TYPE_INVALID))
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return ret; 
}

int kdk_video_set_position(const char* trackid, int position)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "SetPosition");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    int64_t position64 = (int64_t)position;
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &trackid, DBUS_TYPE_INT64, &position64, DBUS_TYPE_INVALID))
    {
        ret = -1;  // 如果参数添加失败，设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 发送消息，不需要等待返回值
    if (!dbus_connection_send(conn, info_msg, 0))  // 不等待回复
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return ret; 
}

int kdk_video_open_uri(const char* uri)
{
    int ret = 1;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);  // 尝试建立到系统总线的私有连接

    if (NULL == conn)  // 如果连接失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    if (!dbus_bus_name_has_owner(conn, "org.mpris.MediaPlayer2.KylinVideo", &err)) {
        if (dbus_error_is_set(&err)) {
            klog_err("Error checking name owner: %s\n", err.message);
            dbus_error_free(&err);
        }
        klog_err("Service org.mpris.MediaPlayer2.KylinMusic not running.\n");
        ret = -1;
        goto out;
    }

    // 创建 D-Bus 方法调用消息
    info_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.KylinVideo",  // 目标服务名称
                                            "/org/mpris/MediaPlayer2", // 目标对象路径
                                            "org.mpris.MediaPlayer2.Player",  // 接口名称
                                            "OpenUri");      // 方法名称
    if (!info_msg)  // 如果消息创建失败
    {
        ret = -1;  // 设置返回值为-1
        goto out;  // 跳转到清理阶段
    }

    // 添加参数到消息中
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID))
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
    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    if (conn)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return ret; 
}
