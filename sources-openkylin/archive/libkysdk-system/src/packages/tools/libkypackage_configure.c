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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <dbus-1.0/dbus/dbus.h>

typedef enum _kdk_boot_mode
{
    MODE_NONE = 0,          // 系统类型未知
    MODE_MAINTAIN,          // 运维模式
    MODE_NORMAL             // 用户模式
} kdk_boot_mode;

typedef unsigned long long ns_mode_session_id_t;

#define OSTREE_MODE_LIB                "/usr/lib/ostreemode/libostreemode.so"
#define NS_MODE_ENTER_HANDLE         "ns_mode_enter"
#define NS_MODE_EXIT_HANDLE        "ns_mode_exit"

typedef ns_mode_session_id_t (*handle_ns_mode_enter)(void);
typedef int (*handle_ns_mode_exit)(ns_mode_session_id_t session_id);

void* libHandle = NULL;

static ns_mode_session_id_t kdk_ns_mode_enter()
{
    libHandle = dlopen(OSTREE_MODE_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        syslog(LOG_ERR, "load %s failed\n",OSTREE_MODE_LIB);
        return 0;
    }
    handle_ns_mode_enter enter_handle = (handle_ns_mode_enter)dlsym(libHandle, NS_MODE_ENTER_HANDLE);
    if(!enter_handle)
    {
        syslog(LOG_ERR, "undefined symbol:%s\n", NS_MODE_ENTER_HANDLE);
        return 0;
    }

    ns_mode_session_id_t session_id_t = enter_handle();
    
    if(session_id_t == 0)
    {
        syslog(LOG_ERR, "错误: ns_mode_enter() 失败, %s\n", strerror(errno));
        dlclose(libHandle);
        return 0;
    }
    syslog(LOG_INFO, "ns_mode_enter() 成功，session_id = %llu\n", session_id_t);

    return (int)session_id_t;
}

static kdk_boot_mode kdk_get_boot_mode()
{
    DBusConnection *conn;
    DBusError err;
    kdk_boot_mode mode = MODE_NONE;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        return mode;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    // 创建用户
    info_msg = dbus_message_new_method_call("com.kylin.MaintainMode",  // target for the method call
                                            "/com/kylin/MaintainMode/Object", // object to call on
                                            "com.kylin.MaintainMode.interface",  // interface to call on
                                            "GetBootMode");           // method name
    if (!info_msg)
    { // -1 is default timeout
        return mode;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        return mode;
    }

    if (sendMsgPending == NULL)
    {
        return mode;
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
        return mode;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        dbus_message_unref(replyMsg);
        return mode;
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        return mode;
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

   if(strcmp(ret, "maintain") == 0)
   {
        mode = MODE_MAINTAIN;
   }
   else if(strcmp(ret, "normal") == 0)
   {
        mode = MODE_NORMAL;
   }

    return mode;
}

static int kdk_ns_mode_exit(ns_mode_session_id_t session_id)
{
    libHandle = dlopen(OSTREE_MODE_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        syslog(LOG_ERR, "load %s failed\n",OSTREE_MODE_LIB);
        return 0;
    }
    handle_ns_mode_exit exit_handle = (handle_ns_mode_exit)dlsym(libHandle, NS_MODE_EXIT_HANDLE);
    if(!exit_handle)
    {
        syslog(LOG_ERR, "undefined symbol:%s\n", NS_MODE_EXIT_HANDLE);
        return 0;
    }

    int ret = exit_handle(session_id);
    
    if(ret != 0)
    {
        syslog(LOG_ERR, "错误: ns_mode_exit() 失败, %s\n", strerror(errno));
        dlclose(libHandle);
        return 0;
    }
    dlclose(libHandle);
    syslog(LOG_INFO, "ns_mode_exit() 成功，资源已清理");
    return 1;
}

static int kdk_dpkg_configure()
{
    syslog(LOG_INFO, "%s debian\n", __func__);
    int flag_enter = 0;
    int session_id = 0;
    if (kdk_get_boot_mode() == MODE_NORMAL)
    {
        session_id = kdk_ns_mode_enter();
        if (session_id != 0)
        {
            flag_enter = 1;
        }
        else
        {
            return 0;
        }
    }

    FILE *fp = NULL;
    char cmd[1024] = "\0";
    int mark = 0;
    char line[1024] = "\0";

    snprintf(cmd, sizeof(cmd), "dpkg.orig --configure -a");
    fp = popen(cmd, "r");
    if(fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            printf("%s", line);
        }

        pclose(fp);
    }

    if (session_id != 0 && flag_enter)
    {
        kdk_ns_mode_exit((ns_mode_session_id_t)session_id);
    }
    return mark;
}

int main(int argc, char *argv[])
{  
    if (geteuid() == 0) // root 用户，可以调用接口
    {
        if(argc > 1)
        {
            printf("不需要加其它参数\n");
            return 1;
        }
        int ret = kdk_dpkg_configure();
    }
    else
    {
        printf("所请求的操作需要超级用户权限\n");
    }
    return 0;
}
