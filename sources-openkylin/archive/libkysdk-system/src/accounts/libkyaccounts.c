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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "libkyaccounts.h"
#include <dbus-1.0/dbus/dbus.h>
#include <libkylog.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
// #include <polkit/polkit.h>
#include <syslog.h>
#include <cjson/cJSON.h>
#include <fcntl.h>
#include <libkylog.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

char *kdk_system_create_user(char *name, char *fullName, int accountType)
{
    if(!name || !fullName)
        return NULL;

    if (kdk_system_check_has_user(name) == 1)
    {
        klog_err("用户已存在 !\n");
        return NULL;
    }

    char *userpath = (char *)malloc(sizeof(char) * 256);
    if (!userpath)
    {
        return NULL;
    }
    memset(userpath, 0, 256);

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",  // target for the method call
                                            "/org/freedesktop/Accounts", // object to call on
                                            "org.freedesktop.Accounts",  // interface to call on
                                            "CreateUser");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
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
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    if (!strstr(ret, "/org/freedesktop/Accounts/User"))
    {
        klog_err("用户名命名不规范 !\n");
        goto err_out;
    }
    snprintf(userpath, 256, "/home/%s", name);
    if (!userpath)
    {
        goto err_out;
    }
    return userpath;

err_out:
    free(userpath);
    return NULL;
}

// int kdk_authority_check_by_polkit()
// {
//     PolkitAuthority *pAuthority = NULL;
//     GError *error = NULL;
//     pid_t pid_of_caller;
//     PolkitSubject *subject;
//     PolkitCheckAuthorizationFlags flags;
//     PolkitAuthorizationResult *polkit_ret = NULL;
//     pid_of_caller = getppid();
//     // if (pid_of_caller == 1)
//     // {
//     //     g_printerr("Refusing to render service to dead parents.\n");
//     //     return 0;
//     // }
//     subject = polkit_unix_process_new_for_owner(pid_of_caller,
//                                                 0,
//                                                 getuid());
//     gchar *actid = "org.kylin.kysdk.chpasswd";
//     flags = POLKIT_CHECK_AUTHORIZATION_FLAGS_NONE;
//     flags |= POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION;
//     pAuthority = polkit_authority_get_sync(NULL, &error);
//     polkit_ret = polkit_authority_check_authorization_sync(pAuthority,
//                                                            subject,
//                                                            actid,
//                                                            NULL,
//                                                            flags,
//                                                            NULL,
//                                                            NULL);

//     if (polkit_authorization_result_get_is_authorized(polkit_ret))
//     {
//         syslog(LOG_ERR, "polkit_ret is failed !");
//         g_object_unref(polkit_ret);
//         g_object_unref(subject);
//         return 1;
//     };
//     g_object_unref(polkit_ret);
//     g_object_unref(subject);

//     return 0;
// }

bool kdk_system_change_password(char *username, char *password, unsigned int *err_num)
{
    if (!username && !password)
    {
        *err_num = 0x04;
        return false;
    }

    if (kdk_system_check_has_user(username) != 1)
    {
        klog_err("用户不存在 !\n");
        syslog(LOG_ERR, "用户不存在！");
        *err_num = 0x02;
        return false;
    }

    char command[2048] = "\0";

    int len = strnlen(password, STRNLEN_MAX);
    if (len < 8)
    {
        klog_err("密码少于 8 个字符!\n");
        syslog(LOG_ERR, "密码少于 8 个字符！");
        *err_num = 0x03;
        return false;
    }

    if (len > 1024)
    {
        klog_err("密码大于 1024 个字符!\n");
        syslog(LOG_ERR, "密码大于 1024 个字符！");
        *err_num = 0x03;
        return false;
    }
    
    if(strstr(password, "$") || strstr(password, "\""))
    {
        char str[1024] = "\0";
        strncpy(str, password, sizeof(str));
        int i, j;
        int lenght = strnlen(str, STRNLEN_MAX);
        char tmp[1024] = "\0";

        for (i = 0, j = 0; i < lenght; i++) {
            if (str[i] == '$') {
                tmp[j] = '\\';
                j++;
                tmp[j] = '$';  
            }
            else if(str[i] == '\"') {
                tmp[j] = '\\';
                j++;
                tmp[j] = '\"';
            }
            else {
                tmp[j] = str[i];
            }
            j++;
        }
        tmp[j] = '\0';
        snprintf(command, sizeof(command), "echo \"%s:%s\" | chpasswd", username, tmp);
    }
    else{
        snprintf(command, sizeof(command), "echo \"%s:%s\" | chpasswd", username, password);
    }
    syslog(LOG_ERR,"command = %s\n", command);
    int ret = system(command);
    if (ret != 0)
    {
        klog_err("无效的密码，密码不规范!\n");
        syslog(LOG_ERR,"无效的密码，密码不规范！");
        *err_num = 0x03;
        return false;
    }
    *err_num = 0;
    syslog(LOG_ERR,"susscul\n");
    return true;
}

bool kdk_system_check_has_user(char *username)
{
    if (!username)
    {
        return false;
    }

    char path[256] = "\0";
    struct stat buff;
    snprintf(path, sizeof(path), "/home/%s", username);
    if (access(path, F_OK) != -1)
    {
        int result = stat(path, &buff);
        if (__S_IFDIR & buff.st_mode)
        {
            return true;
        }
    }
    return false;
}

char *kdk_login_get_public_encrypt()
{
    char *pencrypt = NULL;

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return NULL;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.ukui.UniauthBackend",  // target for the method call
                                            "/org/ukui/UniauthBackend", // object to call on
                                            "org.ukui.UniauthBackend",  // interface to call on
                                            "getPublicEncrypt");        // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return NULL;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID)) {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return NULL;
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
        klog_err("kdk : get reply message fail !\n");
        return NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &pencrypt);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    if (!pencrypt)
    {
        return NULL;
    }

    return pencrypt;
}

bool kdk_login_send_password(const char *username, unsigned char *password, int length)
{
    if (!username || !password)
    {
        return false;
    }
    unsigned char *passwd = password;

    bool result;

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return false;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessageIter datatoSendIter, subArrayIter;

    //创建用户s
    info_msg = dbus_message_new_method_call("org.ukui.UniauthBackend",  // target for the method call
                                            "/org/ukui/UniauthBackend", // object to call on
                                            "org.ukui.UniauthBackend",  // interface to call on
                                            "sendPassword");            // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return false;
    }

    dbus_message_iter_init_append(info_msg, &datatoSendIter);
    dbus_message_iter_append_basic(&datatoSendIter, DBUS_TYPE_STRING, &username);
    dbus_message_iter_open_container(&datatoSendIter,  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING,  &subArrayIter);
    dbus_message_iter_append_fixed_array(&subArrayIter, DBUS_TYPE_BYTE, &passwd, length + 1);
    dbus_message_iter_close_container(&datatoSendIter, &subArrayIter);

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return false;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return false;
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
        klog_err("kdk : get reply message fail !\n");
        return false;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return false;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &result);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    return result;
}

// static uint verify_file(char *pFileName)
// {
//     return !strncmp(pFileName, "/", strlen("/"));
// }

char* kdk_kylinid_check_login()
{
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.kylinID.service",   // target for the method call
                                            "/org/kylinID/path",     // object to call on
                                            "org.kylinID.interface", // interface to call on
                                            "checkLogin");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
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
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return NULL;
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    return ret;

err_out:
    return NULL;
}

char *kdk_kylinid_get_user_token()
{
    char *token = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.kylinID.service",   // target for the method call
                                            "/org/kylinID/path",     // object to call on
                                            "org.kylinID.interface", // interface to call on
                                            "getUserInfo");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
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
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return NULL;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &token);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    if(token != NULL)
    {
        return token;
    }

err_out:
    return NULL;
}

static pthread_t thread_logout = 0;
static volatile int running_logout = 0;
static CallBackLogout _callbacklogout = NULL;

static void *dbus_logout_listener_thread(void *user_data)
{
    int old_type;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type);

    DBusConnection *connection_logout;
    DBusError error;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection_logout = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error))
    {
        klog_err("Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        pthread_exit(NULL);
    }
    if (connection_logout == NULL)
    {
        klog_err("Connection Null\n");
        pthread_exit(NULL);
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection_logout,
                       "type='signal',path='/org/kylinID/path',interface='org.kylinID.interface',member='finishedLogout'",
                       &error);
    // dbus_connection_flush(connection_logout);
    // dbus_connection_add_filter(connection_logout, logout_signal_handler, NULL, NULL);
    int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_logout)
        {
            // dbus_connection_read_write_dispatch(connection_logout, 20);
            dbus_connection_read_write(connection_logout, -1);
            DBusMessage *message = dbus_connection_pop_message(connection_logout);
            if(message == NULL){
                continue;
            }
        
            if(dbus_message_is_signal(message, "org.kylinID.interface", "finishedLogout")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_INT32, &ret, DBUS_TYPE_INVALID))
                {
                    if (ret != -1)
                    {
                        _callbacklogout(ret, user_data);
                        ret = -1;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_unref(connection_logout);
            pthread_exit(NULL);
        }
    }
}

int kdk_kylinid_register_switch_logout_handle(CallBackLogout call_back, void *user_data)
{
    running_logout = 1;
    _callbacklogout = call_back;
    pthread_create(&thread_logout, NULL, dbus_logout_listener_thread, user_data);
    return 0;
}

void kdk_kylinid_unregister_switch_logout_handle()
{
    if (pthread_kill(thread_logout, 0) == 0)
    {
        pthread_cancel(thread_logout);
        pthread_join(thread_logout, NULL);
        thread_logout = 0;
        _callbacklogout = NULL;
    }
}

static pthread_t thread_login = 0;
static volatile int running_login = 0;
static CallBackLogin _callbacklogin = NULL;

static void *dbus_login_listener_thread(void *user_data)
{
    int old_type;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type);

    DBusConnection *connection_login;
    DBusError error;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection_login = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error))
    {
        klog_err("Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        pthread_exit(NULL);
    }
    if (connection_login == NULL)
    {
        klog_err("Connection Null\n");
        pthread_exit(NULL);
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection_login,
                       "type='signal',path='/org/kylinID/path',interface='org.kylinID.interface',member='userInfo'",
                       &error);
    // dbus_connection_add_filter(connection_login, login_signal_handler, NULL, NULL);
    // dbus_connection_flush(connection_login);
    char *info = NULL;
    kdk_userinfo *userInfo = NULL;

    // 循环监听DBus消息
    while (true)
    {
        if (running_login)
        {
            // dbus_connection_read_write_dispatch(connection_login, 20);
            dbus_connection_read_write(connection_login, -1);
            DBusMessage *msg = dbus_connection_pop_message(connection_login);
            if(msg == NULL){
                // sleep(1);
                continue;
            }
        
            if(dbus_message_is_signal(msg, "org.kylinID.interface", "userInfo")){
                if(dbus_message_get_args(msg, &error, DBUS_TYPE_STRING, &info, DBUS_TYPE_INVALID))
                {
                    if (info != NULL)
                    {
                        userInfo = (kdk_userinfo *)calloc(1, sizeof(kdk_userinfo));
                        if(userInfo)
                        {
                            cJSON *root, *format, *array;
                            root = cJSON_Parse(info);
                            format = cJSON_GetObjectItem(root, "data");
                            array = cJSON_GetObjectItem(format, "entity");

                            cJSON *kylinID = cJSON_GetObjectItem(array, "kylinID");
                            if(kylinID)
                            {
                                if(kylinID->valuestring)
                                    strncpy(userInfo->kylinID, kylinID->valuestring, sizeof(userInfo->kylinID) - 1);
                            }

                            cJSON *userName = cJSON_GetObjectItem(array, "userName");
                            if(userName)
                            {
                                if(userName->valuestring)
                                    strncpy(userInfo->userName, userName->valuestring, sizeof(userInfo->userName) - 1);
                            }

                            cJSON *token = cJSON_GetObjectItem(array, "token");
                            if(token)
                            {
                                if(token->valuestring)
                                    strncpy(userInfo->token, token->valuestring, sizeof(userInfo->token) - 1);
                            }

                            cJSON *uid = cJSON_GetObjectItem(array, "uid");
                            if(uid)
                            {
                                if(uid->valuestring)
                                    strncpy(userInfo->uid, uid->valuestring, sizeof(userInfo->uid) - 1);
                            }

                            cJSON *phone = cJSON_GetObjectItem(array, "phone");
                            if(phone)
                            {
                                if(phone->valuestring)
                                    strncpy(userInfo->phone, phone->valuestring, sizeof(userInfo->phone) - 1);
                            }

                            cJSON *email = cJSON_GetObjectItem(array, "email");
                            if(email)
                            {
                                if(email->valuestring)
                                    strncpy(userInfo->email, email->valuestring, sizeof(userInfo->email) - 1);
                            }

                            cJSON *status = cJSON_GetObjectItem(array, "status");
                            if(status)
                            {
                                if(status->valuestring)
                                    strncpy(userInfo->status, status->valuestring, sizeof(userInfo->status) - 1);
                            }
                        }
                        _callbacklogin(userInfo, user_data);
                        userInfo = NULL;
                    }
                }
            }
            dbus_message_unref(msg);
        }
        else
        {
            dbus_connection_unref(connection_login);
            pthread_exit(NULL);
        }
    }
}

int kdk_kylinid_register_switch_login_handle(CallBackLogin call_back, void *user_data)
{
    running_login = 1;
    _callbacklogin = call_back;
    pthread_create(&thread_login, NULL, dbus_login_listener_thread, user_data);
    return 0;
}

void kdk_kylinid_unregister_switch_login_handle()
{
    if (pthread_kill(thread_login, 0) == 0)
    {
        pthread_cancel(thread_login);
        pthread_join(thread_login, NULL);
        thread_login = 0;
        _callbacklogin = NULL;
    }
}

int kdk_login_set_process_name(char *processName)
{
    if(!processName)
        return 0;
    
    DBusConnection *conn;
    DBusError err;
    int ret = 0;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return ret;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.ukui.UniauthBackend",  // target for the method call
                                            "/org/ukui/UniauthBackend", // object to call on
                                            "org.ukui.UniauthBackend",  // interface to call on
                                            "setProcessName");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return ret;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &processName, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        return ret;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return ret;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return ret;
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
        klog_err("kdk : get reply message fail !\n");
        return ret;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return ret;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    return ret;
}

int kdk_login_send_auth_token(int id, char *username, unsigned char *token, int length, int authType)
{
    if (!username)
    {
        return -7;
    }

    int result;

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return false;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessageIter datatoSendIter, subArrayIter;

    //创建用户s
    info_msg = dbus_message_new_method_call("org.ukui.UniauthBackend",  // target for the method call
                                            "/org/ukui/UniauthBackend", // object to call on
                                            "org.ukui.UniauthBackend",  // interface to call on
                                            "sendAuthToken");            // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return -7;
    }

    dbus_message_iter_init_append(info_msg, &datatoSendIter);
    dbus_message_iter_append_basic(&datatoSendIter, DBUS_TYPE_INT32, &id);
    dbus_message_iter_append_basic(&datatoSendIter, DBUS_TYPE_STRING, &username);
    if(token)
    {
        dbus_message_iter_open_container(&datatoSendIter,  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING,  &subArrayIter);
        dbus_message_iter_append_fixed_array(&subArrayIter, DBUS_TYPE_BYTE, &token, length + 1);
        dbus_message_iter_close_container(&datatoSendIter, &subArrayIter);
    }
    else{
        dbus_message_iter_open_container(&datatoSendIter,  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING,  &subArrayIter);
        dbus_message_iter_append_fixed_array(&subArrayIter, DBUS_TYPE_BYTE, "", 0);
        dbus_message_iter_close_container(&datatoSendIter, &subArrayIter);
    }
    
    dbus_message_iter_append_basic(&datatoSendIter, DBUS_TYPE_INT32, &authType);

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return -7;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return -7;
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
        klog_err("kdk : get reply message fail !\n");
        return -7;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return -7;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &result);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    return result;
}
