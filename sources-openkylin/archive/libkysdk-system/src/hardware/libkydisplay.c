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

#include "libkydisplay.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstring-extension.h>
#include <errno.h>
#include <libkylog.h>
#include <dbus-1.0/dbus/dbus.h>

static char *display_get_info(char *str)
{
    char *display = (char*)malloc(sizeof(char) * 512);
    if (!display)
    {
        klog_err("内存申请失败：%s\n", strerror(errno));
        return NULL;
    }
    memset(display, 0, 512);
    char buf[1024] = {0};
    memset(buf, 0, sizeof(buf));
    int i = 0;
	char *buff[3], *p = NULL;
    FILE *pipeLine = popen("lshw -C display", "r");
    if (!pipeLine)
    {
        free(display);
        return NULL;
    }
    if (__glibc_likely(pipeLine != NULL))
    {
        while (fgets(buf, sizeof(buf), pipeLine))
        {
            if(strstr(buf, str))
            {
                p = strtok(buf, ":");
                while(p)
                {
                    buff[i] = p;
                    i++;
                    p = strtok(NULL,"");
                }
                strncpy(display, buff[1], 511); 
		        strstripspace(display);
                break; // 找到后立即退出，避免覆盖
            }
        }
    }
    pclose(pipeLine);
    
    // 根据C语言接口函数空值返回规范，在没有找到信息时释放内存并返回NULL
    if (display[0] == '\0') {
        free(display);
        return NULL;
    }
    
    return display;
}

char* _kdk_display_get_vendor()
{
    return display_get_info("vendor");
}

char *kdk_display_get_vendor()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *vendor = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayVendor");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        vendor = (char *)malloc(sizeof(char) * 512);
        if(!vendor)
        {
            vendor = NULL;
        }
        else
        {
            strncpy(vendor, ret, 511);
            vendor[511] = '\0';
            strstripspace(vendor);
            // 再次检查是否为空
            if (strlen(vendor) == 0) {
                free(vendor);
                vendor = NULL;
            }
        }
    }

    return vendor;
out:
    ret = _kdk_display_get_vendor();
    if(ret != NULL)
    {
        vendor = (char *)malloc(sizeof(char) * 512);
        if(!vendor)
        {
            vendor = NULL;
        }
        else
        {
            strncpy(vendor, ret, 511);
            vendor[511] = '\0';
            strstripspace(vendor);
            // 再次检查是否为空
            if (strlen(vendor) == 0) {
                free(vendor);
                vendor = NULL;
            }
        }
        free(ret);
    }
    return vendor;
}

char* _kdk_display_get_product()
{
    return display_get_info("product");
}

char* kdk_display_get_product()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *product = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayProduct");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        product = (char *)malloc(sizeof(char) * 512);
        if(!product)
        {
            product = NULL;
        }
        else
        {
            strncpy(product, ret, 511);
            product[511] = '\0';
            strstripspace(product);
            if (strlen(product) == 0) {
                free(product);
                product = NULL;
            }
        }
    }

    return product;
out:
    ret = _kdk_display_get_product();
    if(ret != NULL && strlen(ret) > 0)
    {
        product = (char *)malloc(sizeof(char) * 512);
        if(!product)
        {
            product = NULL;
        }
        else
        {
            strncpy(product, ret, 511);
            product[511] = '\0';
            strstripspace(product);
            if (strlen(product) == 0) {
                free(product);
                product = NULL;
            }
        }
        free(ret);
    }
    return product;
}

char* _kdk_display_get_description()
{
    return display_get_info("description");
}

char* kdk_display_get_description()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *description = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayDescription");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        description = (char *)malloc(sizeof(char) * 512);
        if(!description)
        {
            description = NULL;
        }
        else
        {
            strncpy(description, ret, 511);
            description[511] = '\0';
            strstripspace(description);
            if (strlen(description) == 0) {
                free(description);
                description = NULL;
            }
        }
    }

    return description;
out:
    ret = _kdk_display_get_description();
    if(ret != NULL && strlen(ret) > 0)
    {
        description = (char *)malloc(sizeof(char) * 512);
        if(!description)
        {
            description = NULL;
        }
        else
        {
            strncpy(description, ret, 511);
            description[511] = '\0';
            strstripspace(description);
            if (strlen(description) == 0) {
                free(description);
                description = NULL;
            }
        }
        free(ret);
    }

    return description;
}

char* _kdk_display_get_physical_id()
{
    return display_get_info("physical id");
}

char* kdk_display_get_physical_id()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *id = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayPhysicalId");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        id = (char *)malloc(sizeof(char) * 512);
        if(!id)
        {
            id = NULL;
        }
        else
        {
            strncpy(id, ret, 511);
            id[511] = '\0';
            strstripspace(id);
            if (strlen(id) == 0) {
                free(id);
                id = NULL;
            }
        }
    }

    return id;
out:
    ret = _kdk_display_get_physical_id();
    if(ret != NULL && strlen(ret) > 0)
    {
        id = (char *)malloc(sizeof(char) * 512);
        if(!id)
        {
            id = NULL;
        }
        else
        {
            strncpy(id, ret, 511);
            id[511] = '\0';
            strstripspace(id);
            if (strlen(id) == 0) {
                free(id);
                id = NULL;
            }
        }
        free(ret);
    }

    return id;
}

char* _kdk_display_get_bus_info()
{
    return display_get_info("bus info");
}

char* kdk_display_get_bus_info()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *info = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayBusInfo");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        info = (char *)malloc(sizeof(char) * 512);
        if(!info)
        {
            info = NULL;
        }
        else
        {
            strncpy(info, ret, 511);
            info[511] = '\0';
            strstripspace(info);
            if (strlen(info) == 0) {
                free(info);
                info = NULL;
            }
        }
    }

    return info;
out:
    ret = _kdk_display_get_bus_info();
    if(ret != NULL && strlen(ret) > 0)
    {
        info = (char *)malloc(sizeof(char) * 512);
        if(!info)
        {
            info = NULL;
        }
        else
        {
            strncpy(info, ret, 511);
            info[511] = '\0';
            strstripspace(info);
            if (strlen(info) == 0) {
                free(info);
                info = NULL;
            }
        }
        free(ret);
    }

    return info;
}

char* _kdk_display_get_version()
{
    return display_get_info("version");
}

char* kdk_display_get_version()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *version = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayVersion");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        version = (char *)malloc(sizeof(char) * 512);
        if(!version)
        {
            version = NULL;
        }
        else
        {
            strncpy(version, ret, 511);
            version[511] = '\0';
            strstripspace(version);
            if (strlen(version) == 0) {
                free(version);
                version = NULL;
            }
        }
    }

    return version;
out:
    ret = _kdk_display_get_version();
    if(ret != NULL && strlen(ret) > 0)
    {
        version = (char *)malloc(sizeof(char) * 512);
        if(!version)
        {
            version = NULL;
        }
        else
        {
            strncpy(version, ret, 511);
            version[511] = '\0';
            strstripspace(version);
            if (strlen(version) == 0) {
                free(version);
                version = NULL;
            }
        }
        free(ret);
    }

    return version;
}

char* _kdk_display_get_width()
{
    return display_get_info("width");
}

char* kdk_display_get_width()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *width = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayWidth");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        width = (char *)malloc(sizeof(char) * 512);
        if(!width)
        {
            width = NULL;
        }
        else
        {
            strncpy(width, ret, 511);
            width[511] = '\0';
            strstripspace(width);
            if (strlen(width) == 0) {
                free(width);
                width = NULL;
            }
        }
    }

    return width;
out:
    ret = _kdk_display_get_width();
    if(ret != NULL && strlen(ret) > 0)
    {
        width = (char *)malloc(sizeof(char) * 512);
        if(!width)
        {
            width = NULL;
        }
        else
        {
            strncpy(width, ret, 511);
            width[511] = '\0';
            strstripspace(width);
            if (strlen(width) == 0) {
                free(width);
                width = NULL;
            }
        }
        free(ret);
    }

    return width;
}

char* _kdk_display_get_clock()
{
    return display_get_info("clock");
}

char* kdk_display_get_clock()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *clock = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayClock");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        clock = (char *)malloc(sizeof(char) * 512);
        if(!clock)
        {
            clock = NULL;
        }
        else
        {
            strncpy(clock, ret, 511);
            clock[511] = '\0';
            strstripspace(clock);
            if (strlen(clock) == 0) {
                free(clock);
                clock = NULL;
            }
        }
    }

    return clock;
out:
    ret = _kdk_display_get_clock();
    if(ret != NULL && strlen(ret) > 0)
    {
        clock = (char *)malloc(sizeof(char) * 512);
        if(!clock)
        {
            clock = NULL;
        }
        else
        {
            strncpy(clock, ret, 511);
            clock[511] = '\0';
            strstripspace(clock);
            if (strlen(clock) == 0) {
                free(clock);
                clock = NULL;
            }
        }
        free(ret);
    }

    return clock;
}

char* _kdk_display_get_capabilities()
{
    return display_get_info("capabilities");
}

char* kdk_display_get_capabilities()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *capabilities = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayCapabilities");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        capabilities = (char *)malloc(sizeof(char) * 512);
        if(!capabilities)
        {
            capabilities = NULL;
        }
        else
        {
            strncpy(capabilities, ret, 511);
            capabilities[511] = '\0';
            strstripspace(capabilities);
            if (strlen(capabilities) == 0) {
                free(capabilities);
                capabilities = NULL;
            }
        }
    }

    return capabilities;
out:
    ret = _kdk_display_get_capabilities();
    if(ret != NULL && strlen(ret) > 0)
    {
        capabilities = (char *)malloc(sizeof(char) * 512);
        if(!capabilities)
        {
            capabilities = NULL;
        }
        else
        {
            strncpy(capabilities, ret, 511);
            capabilities[511] = '\0';
            strstripspace(capabilities);
            if (strlen(capabilities) == 0) {
                free(capabilities);
                capabilities = NULL;
            }
        }
        free(ret);
    }

    return capabilities;
}

char* _kdk_display_get_configuration()
{
    return display_get_info("configuration");
}

char* kdk_display_get_configuration()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *configuration = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayConfiguration");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        configuration = (char *)malloc(sizeof(char) * 512);
        if(!configuration)
        {
            configuration = NULL;
        }
        else
        {
            strncpy(configuration, ret, 511);
            configuration[511] = '\0';
            strstripspace(configuration);
            if (strlen(configuration) == 0) {
                free(configuration);
                configuration = NULL;
            }
        }
    }

    return configuration;
out:
    ret = _kdk_display_get_configuration();
    if(ret != NULL && strlen(ret) > 0)
    {
        configuration = (char *)malloc(sizeof(char) * 512);
        if(!configuration)
        {
            configuration = NULL;
        }
        else
        {
            strncpy(configuration, ret, 511);
            configuration[511] = '\0';
            strstripspace(configuration);
            if (strlen(configuration) == 0) {
                free(configuration);
                configuration = NULL;
            }
        }
        free(ret);
    }
    return configuration;
}

char* _kdk_display_get_resources()
{
    return display_get_info("resources");
}

char* kdk_display_get_resources()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;
    char *resources = NULL;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        goto out;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/display", // object to call on
                                            "com.kylin.kysdk.display",  // interface to call on
                                            "getDisplayResources");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto out;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        goto out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

    if(ret != NULL && strlen(ret) > 0)
    {
        resources = (char *)malloc(sizeof(char) * 512);
        if(!resources)
        {
            resources = NULL;
        }
        else
        {
            strncpy(resources, ret, 511);
            resources[511] = '\0';
            strstripspace(resources);
            if (strlen(resources) == 0) {
                free(resources);
                resources = NULL;
            }
        }
    }

    return resources;
out:
    ret = _kdk_display_get_resources();
    if(ret != NULL)
    {
        resources = (char *)malloc(sizeof(char) * 512);
        if(!resources)
        {
            resources = NULL;
        }
        else
        {
            strncpy(resources, ret, 511);
            resources[511] = '\0';
            strstripspace(resources);
            if (strlen(resources) == 0) {
                free(resources);
                resources = NULL;
            }
        }
        free(ret);
    }
    return resources;
}
