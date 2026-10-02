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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#define LIBDPKG_VOLATILE_API
#include "libkypackages.h"
#include <libkylog.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <ctype.h>
#include <dbus-1.0/dbus/dbus.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <libgen.h>
#include <libkylog.h>
#include <libkysettings.h>
#include <syslog.h>
#include <pwd.h>
#include <pthread.h>
#include <glib.h>
#include <sys/utsname.h>

#define KDK_DEKSTOP_FILE_PATH "/usr/share/applications/"
#define KDK_LOCALE_DEKSTOP_FILE_PATH "/.local/share/applications/"
#define KDK_SNAPD_DEKSTOP_FILE_PATH "/var/lib/snapd/desktop/applications/"
#define KDK_FLATPAK_DEKSTOP_FILE_PATH "/var/lib/flatpak/exports/share/applications/"
#define PROGRAM_SIZE 1024

#ifdef __linux__
#define DPKG_DB "/var/lib/dpkg/status"
#endif

#define PACKAGE_INCREASE_STEP 50

#ifndef K_MAX_PATH
#define K_MAX_PATH 4096
#endif //K_MAX_PATH

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

typedef enum _kdk_boot_mode
{
    MODE_NONE = 0,          // 系统类型未知
    MODE_MAINTAIN,          // 运维模式
    MODE_NORMAL             // 用户模式
} kdk_boot_mode;

//文件到包的转换接口
#define MAX_PATH_LEN 1024

typedef struct {
    const char *kdk_real_path;  //全路径
    const char *kdk_path;       //转换后路径
} kdk_path_remap_rule;

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
}

static inline char *lookup(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strnlen(key, STRNLEN_MAX) + 1;
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static inline char *read_key(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strnlen(key, STRNLEN_MAX);
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static int get_desktop_info(char *dir_path, kdk_app_info *programlist, int start_app)
{
    char canonical_filename[4096] = {"\0"};
    if (!realpath(dir_path, canonical_filename) || !verify_file(canonical_filename))
    {
        return 0;
    }
    DIR *dir = opendir(canonical_filename);
    struct dirent *dir_t = NULL;
    if (dir != NULL)
    {
        while ((dir_t = readdir(dir)) != NULL)
        {
            if ((strcmp(dir_t->d_name, ".") == 0) || (strcmp(dir_t->d_name, "..") == 0))
                continue;
            if (DT_DIR == dir_t->d_type)
                continue;

            kdk_app_t *curinfo = NULL;
            if (strstr(dir_t->d_name, ".desktop"))
            {
                char fpath[PROGRAM_SIZE] = {0};
                snprintf(fpath, sizeof(fpath), "%s/%s", canonical_filename, dir_t->d_name);

                int is_start_menu_app = 1;
                char name[128] = {"None"};
                char version[128] = {"None"};
                char company[128] = {"None"};
                char cmd[128] = {"None"};
                char param[128] = {"None"};
                char icon[128] = {"None"};
                char buffer[PROGRAM_SIZE] = {0};

                char filename[4096] = {"\0"};
                if (!realpath(fpath, filename) || !verify_file(filename))
                {
                    return 0;
                }

                FILE *fp = fopen(filename, "r");
                if (NULL == fp)
                {
                    return 0;
                }

                while (fgets(buffer, PROGRAM_SIZE, fp))
                {
                    char *val = NULL;
                    if ((val = read_key(buffer, "Name=")))
                    {
                        strncpy(name, val, sizeof(name) - 1);
                    }

                    if ((val = read_key(buffer, "Version=")))
                    {
                        strncpy(version, val, sizeof(version) - 1);
                    }

                    if ((val = read_key(buffer, "Company=")))
                    {
                        strncpy(company, val, sizeof(company) - 1);
                    }

                    if ((val = read_key(buffer, "Exec=")))
                    {
                        char *p = val;
                        char tmp[128] = "\0";
                        strncpy(tmp, val, sizeof(tmp) - 1);
                        char *token = strsep(&p, " ");
                        if (strcmp(token, "sudo") == 0)
                        {
                            strncpy(cmd, tmp, sizeof(cmd) - 1);
                        }
                        else
                        {
                            strncpy(cmd, token, sizeof(cmd) - 1);
                            if (p != NULL)
                                strncpy(param, p, sizeof(param) - 1);
                        }
                    }

                    if ((val = read_key(buffer, "Icon=")))
                    {
                        strncpy(icon, val, sizeof(icon) - 1);
                    }

                    // 判断是否是显示在开始菜单的应用程序
                    if ((val = read_key(buffer, "NoDisplay=")))
                    {
                        if (0 == strcmp(val, "true") && start_app)
                        {
                            is_start_menu_app = 0;
                        }
                    }
                }
                fclose(fp);

                if (is_start_menu_app)
                {
                    kdk_app_t **tmp = (kdk_app_t **)realloc(programlist->list, PROGRAM_SIZE * sizeof(kdk_app_t *));
                    if (!tmp)
                        return 0;

                    curinfo = (kdk_app_t *)calloc(1, sizeof(kdk_app_t));
                    if (!curinfo)
                        return 0;

                    programlist->list = tmp;
                    programlist->list[programlist->nums] = curinfo;
                    programlist->nums++;

                    curinfo->name = strdup(name);
                    curinfo->version = strdup(version);
                    curinfo->company = strdup(company);
                    curinfo->cmd = strdup(cmd);
                    curinfo->param = strdup(param);
                    curinfo->icon = strdup(icon);
                }
            }
        }
        closedir(dir);
    }
    return 1;
}

static char *read_default_app(const char *filetype)
{
    char filename[K_MAX_PATH] = "\0";
    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    snprintf(filepath, sizeof(filepath), "%s/.config/mimeapps.list", homeDir);
    if(!realpath(filepath, filename) || !verify_file(filename))
    {
        ;
    }
    FILE *fp = fopen(filename, "r");
    char line[1024], *val = NULL;
    if (NULL != fp)
    {
        while (fgets(line, 1024, fp))
        {
            if ((val = lookup(line, filetype)))
                break;
        }
        fclose(fp);
    }

    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/ukui-mimeapps.list", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }
    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/mimeinfo.cache", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }

    if (NULL == val)
        return strdup("None");

    strstrip(val, ';');
    char desktop_file[1024] = "\0";
    snprintf(desktop_file, sizeof(desktop_file), "/usr/share/applications/%s", val);

    char canonical_filename[K_MAX_PATH] = {"\0"};
    if (!realpath(desktop_file, canonical_filename) || !verify_file(canonical_filename))
    {
        return NULL;
    }

    fp = fopen(canonical_filename, "r");
    if (NULL == fp)
        return NULL;

    while (fgets(line, 1024, fp))
    {
        val = line;
        if (strstartswith(val, "Name=") == 0)
        {
            val += strnlen("Name=", STRNLEN_MAX);
            strstripspace(val);
            break;
        }
    }
    fclose(fp);

    return val ? strdup(val) : NULL;
}

kdk_package_list *kdk_package_get_packagelist()
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    kdk_package_list *packagelist = (kdk_package_list *)calloc(1, sizeof *packagelist);
    if (!packagelist)
        goto err_out;

    char buffer[4097];
    kdk_package_t *curpackage = NULL;
    size_t maxpackage = 0;
    do
    {
        if (!fgets(buffer, 4096, fp))
            continue;
        char *val;
        if ((val = lookup(buffer, "Package")))
        {
            curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
            if (!curpackage)
                goto err_out;
            curpackage->name = strdup(val);
            if (packagelist->nums == maxpackage)
            {
                maxpackage += PACKAGE_INCREASE_STEP;
                kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                if (!tmp)
                {
                    free(curpackage);
                    goto err_out;
                }
                packagelist->list = tmp;
            }
            packagelist->list[packagelist->nums] = curpackage;
            packagelist->nums++;
        }
        else if ((val = lookup(buffer, "Status")))
        {
            if (strcmp(val, "install ok installed") == 0)
            {
                curpackage->status = strdup("ii");
            }
            else if (strcmp(val, "deinstall ok config-files") == 0)
            {
                curpackage->status = strdup("rc");
            }
        }
        else if ((val = lookup(buffer, "Section")))
        {
            curpackage->section = strdup(val);
        }
        else if ((val = lookup(buffer, "Version")))
        {
            curpackage->version = strdup(val);
        }
        else if ((val = lookup(buffer, "Installed-Size")))
        {
            curpackage->size_kb = atol(val);
        }
        else if ((val = lookup(buffer, "Depends")))
        {
        }
    } while (feof(fp) == 0);

    fclose(fp);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_info("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getAllLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        // 释放message
        dbus_message_unref(message);
        message = NULL;

        // 解析回复消息
        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        {
            klog_err("DBus reply parsing failed\n");
            goto out;
        }

        // 释放reply
        dbus_message_unref(reply);
        reply = NULL;

        DBusMessageIter array_iter;
        char *str = NULL;

        // 进入数组
        dbus_message_iter_recurse(&iter, &array_iter);

        // 遍历所有字符串
        while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&array_iter, &str);
            klog_info("Got string: %s\n", str);
            if(str)
            {

                curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
                if (!curpackage)
                    goto err_out;
                if (packagelist->nums == maxpackage)
                {
                    maxpackage += PACKAGE_INCREASE_STEP;
                    kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                    if (!tmp)
                    {
                        free(curpackage);
                        goto err_out;
                    }
                    packagelist->list = tmp;
                }
                packagelist->list[packagelist->nums] = curpackage;
                packagelist->nums++;

                cJSON *root, *format, *array;
                root = cJSON_Parse(str);

                cJSON *name = cJSON_GetObjectItem(root, "name");
                if(name)
                {
                    if(name->valuestring)
                    {
                        klog_info("name->valuestring = %s\n", name->valuestring);
                        curpackage->name = strdup(name->valuestring);
                    }
                }

                cJSON *version = cJSON_GetObjectItem(root, "version");
                if(version)
                {
                    if(version->valuestring)
                    {
                        klog_info("version->valuestring = %s\n", version->valuestring);
                        curpackage->version = strdup(version->valuestring);
                    }
                }

                cJSON *size = cJSON_GetObjectItem(root, "size");
                if(size)
                {
                    if(size->valuestring)
                    {
                        klog_info("size->valuestring = %s\n", size->valuestring);
                        char *endptr = NULL;
                        unsigned long size_b = strtoul(size->valuestring, &endptr, 10);
                        klog_info("size_b = %lu\n", size_b);

                        curpackage->size_kb = size_b / 1024;
                        klog_info("curpackage->size_kb = %lu\n", curpackage->size_kb);
                    }
                }
                dbus_message_iter_next(&array_iter);
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char name[256], version[64];
                int count = sscanf(line, "%s %s", name, version);
                if (count != 2)
                    continue;
                if (0 == strcmp(name, "软件包名")) // 第一行表头
                    continue;
                if (name[0] == '=')
                {
                    continue;
                }

                curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
                if (!curpackage)
                    goto err_out;
                if (packagelist->nums == maxpackage)
                {
                    maxpackage += PACKAGE_INCREASE_STEP;
                    kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                    if (!tmp)
                    {
                        free(curpackage);
                        goto err_out;
                    }
                    packagelist->list = tmp;
                }
                packagelist->list[packagelist->nums] = curpackage;
                packagelist->nums++;

                curpackage->name = strdup(name);
                curpackage->version = strdup(version);
            }
        }
        pclose(fp);
    }

    return packagelist;

err_out:
#ifdef __linux__
    fclose(fp);
#endif
    kdk_package_free_packagelist(packagelist);
    return NULL;
}

static void _free_package_t(kdk_package_t *package)
{
    if (package->name)
        free(package->name);
    if (package->section)
        free(package->section);
    if (package->version)
        free(package->version);
    if (package->status)
        free(package->status);
    if (package->depends)
        free(package->depends);
}

void kdk_package_free_packagelist(kdk_package_list *pl)
{
    if (!pl)
        return;

    if (pl->list)
    {
        for (int i = 0; i < pl->nums; i++)
        {
            _free_package_t(pl->list[i]);
            free(pl->list[i]);
        }
        free(pl->list);
    }
    free(pl);
}

InstallMethod kdk_package_get_installation_method(const char *name, const char *version)
{
    const char *dpkg = "dpkg -l";
    FILE *fp = popen(dpkg, "r");
    if (NULL != fp)
    {
        char *line[1024];
        while (fgets(line, sizeof(line), fp))
        {
            char tmp_name[256], tmp_version[64];
            int count = sscanf(line, "%*s %s %s %*s %*s", tmp_name, tmp_version);
            if (count != 2)
                continue;
            if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
            {
                if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                {
                    pclose(fp);
                    if (strstr(name, "kwre"))
                        return KWRE_METHOD;
                    else if (strstr(name, "kmre"))
                        return KMRE_METHOD;
                    else
                        return DPKG_METHOD;
                }
            }
            else
            {
                if (0 == strcmp(tmp_name, name))
                {
                    pclose(fp);
                    if (strstr(name, "kwre"))
                        return KWRE_METHOD;
                    else if (strstr(name, "kmre"))
                        return KMRE_METHOD;
                    else
                        return DPKG_METHOD;
                }
            }
        }
    }

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {

        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("installation method DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getAllLocalInfo");
        if (message == NULL)
        {
            klog_err("installation method DBus message allocation failed\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("installation method DBus reply error: %s\n", error.message);
            goto out;
        }

        // 释放message
        dbus_message_unref(message);
        message = NULL;

        // 解析回复消息
        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        {
            klog_err("installation method DBus reply parsing failed\n");
            goto out;
        }

        // 释放reply
        dbus_message_unref(reply);
        reply = NULL;

        DBusMessageIter array_iter;
        char *str = NULL;

        // 进入数组
        dbus_message_iter_recurse(&iter, &array_iter);

        // 遍历所有字符串
        while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&array_iter, &str);
            klog_info("installation method Got string: %s\n", str);
            if(str)
            {
                cJSON *root;
                root = cJSON_Parse(str);

                cJSON *km_name = cJSON_GetObjectItem(root, "name");

                cJSON *km_version = cJSON_GetObjectItem(root, "version");
                if(km_name && km_version)
                {
                    klog_info("km_name->valuestring = %s\n", km_name->valuestring);
                    klog_info("km_version->valuestring = %s\n", km_version->valuestring);
                    

                    if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                    {
                        if (0 == strcmp(km_name->valuestring, name) && 0 == strcmp(km_version->valuestring, version))
                        {
                            if(connection)
                            {
                                dbus_connection_unref(connection);
                            }
                            return KM_METHOD;
                        }
                    }
                    else
                    {
                        if (0 == strcmp(km_name->valuestring, name))
                        {
                            if(connection)
                            {
                                dbus_connection_unref(connection);
                            }
                            return KM_METHOD;
                        }
                    }
                }

                dbus_message_iter_next(&array_iter);
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char *line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                {
                    if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                    {
                        pclose(fp);
                        return KARE_METHOD;
                    }
                }
                else
                {
                    if (0 == strcmp(tmp_name, name))
                    {
                        pclose(fp);
                        return KARE_METHOD;
                    }
                }
            }
            pclose(fp);
        }
    }

    return NONE_METHOD;
}

kdk_boot_mode kdk_get_boot_mode()
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
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return mode;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return mode;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        return mode;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return mode;
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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

static pthread_t thread_install = 0;
static volatile int running_install = 0;
InstallCallBack _callbackinstall = NULL;

static void *dbus_kaiming_online_install_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    char *info = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='progresschangesignal'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "progresschangesignal")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &info, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "info = %s\n", info);
                    cJSON *json = cJSON_Parse(info);
                    if (json == NULL) {
                        dbus_connection_unref(connection);
                        pthread_exit(NULL);
                    }

                    cJSON *progress = cJSON_GetObjectItem(json, "progress");
                    if (progress->valueint == 0) {
                        cJSON_Delete(json);
                        dbus_connection_unref(connection);
                        pthread_exit(NULL);
                    }
                    _callbackinstall(progress->valueint, 1, "");
                    errorCode = -1;
                }
            }
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "kaiming online install info name : %s, operateType : %d, errorCode : %d, errinfo : %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0)
                    {
                        _callbackinstall(100, 0, errinfo);
                        errorCode = -1;
                        mark = 1;
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming online install fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackinstall(0, -1, errinfo);
                        errorCode = -1;
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_install_online(const char *pkg_name, InstallCallBack call_back)
{
    int flag = 0;
    int mark = 0;

    DBusConnection *conn;
    DBusError err;

    DBusMessage *info_msg = NULL;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessage *replyMsgBranch = NULL;

    cJSON *branch_name = NULL;
    cJSON *json = NULL;

    running_install = 1;
    _callbackinstall = call_back;
    pthread_create(&thread_install, NULL, dbus_kaiming_online_install_listener_thread, NULL);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        syslog(LOG_INFO, "kaiming\n");
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            return 0;
        }        

        branch_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "getRemoteInfo");           // method name

        if (!branch_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            return 0;
        }
        if(strstr(pkg_name, "="))
        {
            char package_name[128] = "\0";
            char version[128] = "\0";
            char *name = (char *)malloc(sizeof(char) * 512);
            sscanf(pkg_name, "%[^=]= %s", package_name, version);
            strncpy(name, package_name, 511);
            syslog(LOG_DEBUG, "package_name = %s, version = %s, name = %s", package_name, version, name);
            if (!dbus_message_append_args(branch_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
            {
                free(name);
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                return 0;
            }
            free(name);
        }
        else
        {
            if (!dbus_message_append_args(branch_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                return 0;
            }
        }

        if (!dbus_connection_send_with_reply(conn, branch_msg, &sendMsgPendingBranch, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            return 0;
        }

        if (sendMsgPendingBranch == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            return 0;
        }

        dbus_connection_flush(conn);

        if (branch_msg)
        {
            dbus_message_unref(branch_msg);
        }

        dbus_pending_call_block(sendMsgPendingBranch);

        replyMsgBranch = dbus_pending_call_steal_reply(sendMsgPendingBranch);

        if (replyMsgBranch == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            return 0;
        }

        if (sendMsgPendingBranch)
        {
            dbus_pending_call_unref(sendMsgPendingBranch);
        }

        if (dbus_message_get_type(replyMsgBranch) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsgBranch));
            dbus_message_unref(replyMsgBranch);
            return 0;
        }

        DBusMessageIter args_branch;
        char *branch_res;
                
        if (!dbus_message_iter_init(replyMsgBranch, &args_branch))
        {
            dbus_message_unref(replyMsgBranch);
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            return 0;
        }
        else
        {
            dbus_message_iter_get_basic(&args_branch, &branch_res);
        }

        if (replyMsgBranch)
        {
            dbus_message_unref(replyMsgBranch);
        }
        // dbus_connection_close(conn);

        syslog(LOG_INFO, "kaiming online branch : %s\n", branch_res);

        json = cJSON_Parse(branch_res);
        if (json == NULL) {
            return 0;
        }

        branch_name = cJSON_GetObjectItem(json, "branch");
        if (branch_name == NULL || !cJSON_IsString(branch_name)) {
            cJSON_Delete(json);
            return 0;
        }

        if(strnlen(branch_name->valuestring, STRNLEN_MAX) == 0)
        {
            return 0;
        }
    // }

    // if(flag == 1)
    // {
        char *branch = "";
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            goto err_out;
        }        

        info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "install");           // method name

        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if(strstr(pkg_name, "="))
        {
            char package_name[128] = "\0";
            char version[128] = "\0";
            char *name = (char *)malloc(sizeof(char) * 512);
            sscanf(pkg_name, "%[^=]= %s", package_name, version);
            snprintf(name, sizeof(char) * 512, "%s/%s", package_name, version);
            syslog(LOG_DEBUG, "package_name = %s, version = %s, name = %s", package_name, version, name);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                free(name);
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }
            free(name);
        }
        else
        {
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);

        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            goto err_out;
        }

        DBusMessageIter args;
        bool *res = false;
                
        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &res);
        }

        dbus_message_unref(info_msg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_message_unref(replyMsg);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);
    }

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
        
   return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

typedef struct {
    char param_name[128];
    FILE *param_fp;
} ThreadArgs;

int kdk_package_is_installed(const char *name, const char *version);

static void *dbus_debian_online_install_listener_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    char line[1024] = "\0";
    int success = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int mark = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    int extra_line = 0;
    size_t len = 0;
    if(running_install)
    {
        while (fgets(line, sizeof(line), args->param_fp))
        {
            syslog(LOG_INFO, "debain online install info : %s\n", line);

            if (strstr(line, "获取:") && !stage_flags[0]) {
                progress += 10;  // 下载阶段占 10%
                stage_flags[0] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "准备解压") && !stage_flags[1]) {
                progress += 20;  // 准备解压占 20%
                stage_flags[1] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "正在解压") && !stage_flags[2]) {
                progress += 30;  // 解压阶段占 30%
                stage_flags[2] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "正在设置") && !stage_flags[3]) {
                progress += 30;  // 设置阶段占 30%
                stage_flags[3] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "正在处理用于") && !stage_flags[4]) {
                progress += 5;  // 触发器处理占 10%
                stage_flags[4] = 1;
                _callbackinstall(progress, 1, line);
            }
            

            if (strstr(line, "下列软件包有未满足的依赖关系") || strstr(line, "E: 无法定位软件包") || strstr(line, "Failed to enable unit"))
            {
                capture = 1;  // 开始捕获
            }

            if (capture) 
            {
                len = strnlen(line, STRNLEN_MAX);
                if (total_len + len < sizeof(output) - 1) 
                {
                    strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                    total_len += len;
                } else {
                    break;  // 防止溢出
                }
            }

            if (strstr(line, "有未能满足的依赖关系")) 
            {
                extra_line = 1;  // 让下一行也加入输出
            }
            else if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错"))
            {
                if(strnlen(output, STRNLEN_MAX) == 0)
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }
                capture = 1;
                extra_line = 1;  // 让下一行也加入输出
            }
            else if (extra_line) 
            {
                capture = 0;
                extra_line = 0;  // 只多输出一行
            }

            if(strstr(line, "错误"))
            {
                flag = 1;
            }
            if(strstr(line, "E: 无法定位软件包"))
            {
                flag = 1;
            }
            if(strstr(line, "正在处理用于 man-db") || strstr(line, "正在处理用于 libc-bin"))
            {
                success = 1;
            }
        }
        pclose(args->param_fp);

        int ret =  kdk_package_is_installed(args->param_name, NULL);
        syslog(LOG_INFO, "%s package is installed : %d\n", args->param_name, ret);

        if(ret == 0)
        {
            _callbackinstall(100, 0, line);
            mark = 1;
            syslog(LOG_INFO, "debian online success");
        }
        else
        {
            _callbackinstall(0, -1, output);
            syslog(LOG_INFO, "debian online fail");
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_debian_install_online(const char *pkg_name, InstallCallBack call_back)
{
    int mark = 0;

    syslog(LOG_INFO, "debain\n");
    char cmd[512] = "\0";
    snprintf(cmd, 512, "apt install --allow-downgrades -y %s 2>&1", pkg_name);
    FILE *fp = popen(cmd, "r");
    if(!fp)
        return 0;

    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    strncpy(args->param_name, pkg_name, sizeof(args->param_name) - 1);
    args->param_fp = fp;

    running_install = 1;
    _callbackinstall = call_back;
    pthread_create(&thread_install, NULL, dbus_debian_online_install_listener_thread, (void *) args);
    
    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
    free(args);

    syslog(LOG_INFO, "debian online mark %d", mark);

    return mark;
}

static void *dbus_kare_build_base_env_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int progress = 0;
    int status = -1;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='buildStatusChanged'",
                    &error);

    while (true)
    {
        if(running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kare", "buildStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &progress, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, progress = %d, status = %d, errinfo = %s\n", name, progress, status, errinfo);

                    if(status == -1 || status == -2 || status == -3 || status == -4)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else
                    {
                        _callbackinstall(progress, 2, errinfo);
                        if(progress == 100 && status == 0)
                        {
                            _callbackinstall(progress, 2, "构建成功");
                            mark = 1;
                            break;
                        }
                        status = -5;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else{
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kare_build_base_env(char *env, InstallCallBack call_back)
{
    syslog(LOG_INFO, "build base env\n");
    int mark = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
        goto err_out;
    }

    if (dbus_error_is_set(&err))
    {
        // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                            "/org/kylin/kare", // object to call on
                                            "org.kylin.kare",  // interface to call on
                                            "buildBaseEnv");               // method name
    if (!info_msg)
    { // -1 is default timeout
        syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_INVALID))
    {
        syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        syslog(LOG_ERR, "kdk : get reply message fail !\n");
        goto err_out;
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        goto err_out;
    }

    DBusMessageIter args;
    bool *ret = false;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    dbus_message_unref(info_msg);
    dbus_message_unref(replyMsg);
    dbus_pending_call_unref(sendMsgPending);
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    dbus_error_free(&err);

    if(ret)
    {
        mark = 1;
    }
    

    if(mark)
    {
        mark = 0;
        running_install = 1;
        _callbackinstall = call_back;
        pthread_create(&thread_install, NULL, dbus_kare_build_base_env_listener_thread, NULL);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }
    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_kare_install_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int progress = 0;
    int status = -5;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='installStatusChanged'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kare", "installStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &progress, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, progress = %d, status = %d, errinfo = %s\n", name, progress, status, errinfo);
                    // if (status != -5)
                    // {
                    //     _callbackinstall(progress, status);
                        
                    //     if (status == 0)
                    //     {
                    //         mark = 1;
                    //         break;
                    //     }
                    //     status = -5;
                    // }

                    if(status == -1)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else if(status == -2)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else if(status == -3)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else if(status == -4)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else
                    {
                        _callbackinstall(progress, status, errinfo);
                        if(progress == 100 && status == 0)
                        {
                            mark = 1;
                            break;
                        }
                        status = -5;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else{
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static char *kdk_system_get_eUser()
{
    char *e_user = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        fprintf(stderr, "DBus connection error: %s\n", error.message);
        goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    int element_count = dbus_message_iter_get_element_count(&iter);
    char **users = calloc(element_count + 1, sizeof(char *));
    if (NULL == users)
        goto out;

    dbus_message_iter_recurse(&iter, &iter);

    // 解析返回值
    // 循环中过程中调用新的dbus接口会导致迭代器iter被清空，但是没细查原因
    // 先用一个字符串数组存储所有用户的object path
    for (int i = 0; i < element_count; i++)
    {
        char *path = NULL;

        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&iter, &struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &path);

        users[i] = strdup(path);

        dbus_message_iter_next(&iter);
    }

    for (int i = 0; i < element_count; i++)
    {
        char *interface = "org.freedesktop.login1.User";
        char *state = "State";
        char *property = "Name";

        message = dbus_message_new_method_call("org.freedesktop.login1",
                                               users[i],
                                               "org.freedesktop.DBus.Properties",
                                               "Get");
        if (message == NULL)
        {
            fprintf(stderr, "DBus message allocation failed\n");
            goto out;
        }

        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            fprintf(stderr, "DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_message_unref(message);
        message = NULL;

        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
        {
            fprintf(stderr, "DBus reply parsing failed\n");
            goto out;
        }

        dbus_message_unref(reply);
        reply = NULL;

        char *active = NULL;
        dbus_message_iter_recurse(&iter, &iter);
        dbus_message_iter_get_basic(&iter, &active);

        if (0 == strcmp(active, "active"))
        {
            message = dbus_message_new_method_call("org.freedesktop.login1",
                                                   users[i],
                                                   "org.freedesktop.DBus.Properties",
                                                   "Get");
            if (message == NULL)
            {
                fprintf(stderr, "DBus message allocation failed\n");
                goto out;
            }

            dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
            if (reply == NULL)
            {
                fprintf(stderr, "DBus reply error: %s\n", error.message);
                goto out;
            }

            dbus_message_unref(message);
            message = NULL;

            if (!dbus_message_iter_init(reply, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
            {
                fprintf(stderr, "DBus reply parsing failed\n");
                goto out;
            }

            dbus_message_unref(reply);
            reply = NULL;

            dbus_message_iter_recurse(&iter, &iter);
            dbus_message_iter_get_basic(&iter, &e_user);

            break;
        }
    }

    if (e_user)
        e_user = strdup(e_user);
out:
    if (users)
        g_strfreev(users);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);
    if (connection)
        dbus_connection_unref(connection);
    return e_user;
}

static char* kdk_get_home_dir()
{
    char *username = kdk_system_get_eUser();
    if(!username)
    {
        return NULL;
    }

    char *user = NULL;
    DBusConnection *conn = NULL;
    DBusError err;

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        free(username);
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            "/org/freedesktop/Accounts",     // object to call on
                                            "org.freedesktop.Accounts", // interface to call on
                                            "FindUserByName");           // method name
    if (!info_msg)
    { // -1 is default timeout
        free(username);
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &username, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        free(username);
        goto err_out;
    }
    free(username);

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
        dbus_message_iter_get_basic(&args, &user);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    DBusMessage *msg = NULL;
    DBusPendingCall *sendPending = NULL;
    DBusMessage *replyLangMsg = NULL;

    //创建用户
    msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            user,     // object to call on
                                            "org.freedesktop.DBus.Properties", // interface to call on
                                            "Get");           // method name
    if (!msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }
    char *interface = "org.freedesktop.Accounts.User";
    char *property = "HomeDirectory";

    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, msg, &sendPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    if (msg)
    {
        dbus_message_unref(msg);
    }

    dbus_pending_call_block(sendPending);

    replyLangMsg = dbus_pending_call_steal_reply(sendPending);

    if (replyLangMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendPending)
    {
        dbus_pending_call_unref(sendPending);
    }

    if (dbus_message_get_type(replyLangMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyLangMsg));
        dbus_message_unref(replyLangMsg);
        goto err_out;
    }

    DBusMessageIter iter;
    char *dir = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(replyLangMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("DBus reply parsing failed\n");
        dbus_message_unref(replyLangMsg);
        dbus_connection_unref(conn);
        goto err_out;
    }

    // 释放reply
    dbus_message_unref(replyLangMsg);

    dbus_message_iter_recurse(&iter, &iter);
    dbus_message_iter_get_basic(&iter, &dir);
    dbus_connection_unref(conn);
    return dir;

err_out:
    return NULL;
}

static int kdk_kare_install_online(const char *pkg_name, InstallCallBack call_back)
{
    syslog(LOG_DEBUG, "kare online");

    struct utsname utsbuf;
    if (uname(&utsbuf) == -1)
    {
        return 0;
    }

    char *arch = strdup(utsbuf.machine);
    if(strstr(arch, "sw"))
    {
         kdk_boot_mode mode = kdk_get_boot_mode();
        if(mode == MODE_MAINTAIN)
        {
            int installRet = kdk_debian_install_online(pkg_name, call_back);
            return installRet;
        }
        else
        {
            syslog(LOG_INFO, "kare arch sw\n");
            return 0;
        }
    }

    char cmd[1024]= "\0";
    char line[1024] = "\0";
    FILE *fp = NULL;
    char deb_path[1024] = "\0";
    char pkg_name_str[256] = "\0";
    char filename[512] = "\0";
    char fileNamePath[512] = "\0";
    char dir[512] = "\0";

    char *home_dir = kdk_get_home_dir();
    syslog(LOG_DEBUG, "home_dir = %s", home_dir);
    snprintf(dir, sizeof(dir), "%s/.cache/kysdk/wget_down/deb", home_dir);
    syslog(LOG_DEBUG, "dir = %s", dir);

    if (0 != access(dir, F_OK))
    {
        char temp_path[512] = "\0";
        char *p = NULL;
        size_t len;

        // 拷贝路径到临时缓冲区
        strncpy(temp_path, dir, sizeof(temp_path));
        len = strnlen(temp_path, STRNLEN_MAX);

        // 如果路径以 '/' 结尾，移除它
        if (temp_path[len - 1] == '/') {
            temp_path[len - 1] = '\0';
        }

        // 从头到尾逐层创建目录
        for (p = temp_path + 1; *p; p++) {
            if (*p == '/') {
                *p = '\0'; // 暂时中断路径
                strstripspace(temp_path);
                syslog(LOG_DEBUG, "temp_path = %s\n", temp_path);
                if(access(temp_path,0) != 0)
                {
                    if (mkdir(temp_path, 0775) != 0) 
                    {
                        syslog(LOG_ERR, "Directory created error");
                        return 0;
                    }
                }
                *p = '/'; // 恢复路径
            }
        }

        // 创建最后一层目录
        if (mkdir(temp_path, 0775) != 0) 
        {
            syslog(LOG_ERR, "Directory created error");
            return 0;
        }
    
        syslog(LOG_DEBUG, "Directory created: %s", dir);
    }

    if(strstr(pkg_name , "="))
    {
        char cwd[1024];  // 存储当前目录路径
        if (getcwd(cwd, sizeof(cwd)) != NULL) 
        {
            syslog(LOG_DEBUG, "Current directory: %s\n", cwd);

            if (chdir(dir) == 0) 
            {
                syslog(LOG_DEBUG, "Switched to directory: %s\n", dir);
                memset(cmd, 0, 1024);
                snprintf(cmd, sizeof(cmd), "apt download %s", pkg_name);
                syslog(LOG_DEBUG, "cmd = %s\n", cmd);
                system(cmd);

                if (chdir(cwd) != 0) 
                {
                    syslog(LOG_ERR, "Failed to change directory");
                    return 0;
                }
            } 
            else 
            {
                syslog(LOG_ERR, "Failed to change directory");
                return 0;
            }
        } else {
            syslog(LOG_ERR, "getcwd failed");
            return 0;
        }
        char version[128] = "\0";
        char package_name [128] = "\0";
        sscanf(pkg_name, "%[^=]= %s", package_name, version);
        syslog(LOG_DEBUG, "package_name = %s, version = %s", package_name, version);
        DIR *dp;
        struct dirent *entry;
        dp = opendir(dir);
        if (dp == NULL) 
        {
            return 0;
        }
        while ((entry = readdir(dp)) != NULL) 
        {
        // 跳过当前目录和父目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }

            if(strstr(entry->d_name, package_name))
            {
                strncpy(filename, entry->d_name, sizeof(filename) - 1);
            }
        }
        closedir(dp);
    }
    else
    {
        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "apt-get --print-uris install %s", pkg_name);
        syslog(LOG_DEBUG, "cmd = %s\n", cmd);
        fp = popen(cmd, "r");
        if(fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                strstripspace(line);
                syslog(LOG_DEBUG, "line = %s\n", line);
                sscanf(line, "'%[^']", deb_path);
                syslog(LOG_DEBUG, "deb_path = %s\n", deb_path);

                const char *last_slash = strrchr(deb_path, '/'); // 找到最后一个 '/'
                const char *underscore;

                if (last_slash) {
                    last_slash++; // 移到 '/' 后的第一个字符
                    underscore = strchr(last_slash, '_'); // 找到 '_' 的位置

                    if (underscore) {
                        size_t length = underscore - last_slash; // 计算 的长度
                        char result[length + 1];
                        strncpy(result, last_slash, length); // 复制到结果字符串
                        result[length] = '\0'; // 添加字符串终止符
                        if(result[0] != '\0')
                        {
                            strncpy(pkg_name_str, result, sizeof(pkg_name_str) - 1);
                            syslog(LOG_DEBUG, "pkg_name_str = %s", pkg_name_str);
                        }
                    }
                }
                
                if(strcmp(pkg_name, pkg_name_str) == 0)
                {
                    syslog(LOG_DEBUG, "pkg_name = %s, pkg_name_str = %s", pkg_name, pkg_name_str);
                    strncpy(fileNamePath, deb_path, sizeof(fileNamePath) - 1);
                    break;
                }
            }
            pclose(fp);
        }
        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "wget -P %s %s", dir, deb_path);
        syslog(LOG_DEBUG, "wget cmd = %s", cmd);
        system(cmd);

        DIR *dp;
        struct dirent *entry;
        dp = opendir(dir);
        if (dp == NULL) 
        {
            return 0;
        }
        while ((entry = readdir(dp)) != NULL) 
        {
        // 跳过当前目录和父目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }

            if(strstr(entry->d_name, pkg_name))
            {
                strncpy(filename, entry->d_name, sizeof(filename) - 1);
            }
        }
        closedir(dp);
    }

    memset(deb_path, 0, 1024);
    snprintf(deb_path, sizeof(deb_path), "%s/%s", dir, filename);
    syslog(LOG_DEBUG, "deb_path: %s\n", deb_path);

    char rm_dir[256] = "\n";
    snprintf(rm_dir, sizeof(rm_dir), "rm %s", deb_path);

    if (filename[0] == '\0')
    {
        _callbackinstall(0, -1, "软件包不存在");
        return 0;
    }

    memset(fileNamePath, 0, 512);
	strncpy(fileNamePath, filename, sizeof(fileNamePath) - 1);
    
    // 查找第一个下划线的位置
    char **underscore = strsplit(fileNamePath, '_');

    // 如果没有找到下划线，返回空指针
    if (strnlen(underscore[0], STRNLEN_MAX) == 0) {
        system(rm_dir);
        return 0;
    }

    fp = fopen(DPKG_DB, "rt");
    if (!fp)
    {
        system(rm_dir);
        return 0;
    }

    char buffer[4097];
    char *val;
    int same = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, underscore[0]) == 0)
            {
                same = 1;
                break;
            }
        }
    }
    fclose(fp);
    if (1 == same)
    {
		_callbackinstall = call_back;
        _callbackinstall(0, -1, "系统中有同名软件包");
        system(rm_dir);
        return 0;
    }
    
    int mark = 0;
    int flag = 0;
    char key[128] = "\0";
    char value[128] = "\0";
    char *runtime = NULL;
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "kare\n");
        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "ar -x --output \"%s\" \"%s\" _compat", dir, deb_path);
        syslog(LOG_DEBUG, "cmd = %s\n", cmd);
        system(cmd);
        // if(ret != 0)
        //     return 0;

        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "%s/_compat", dir);

        fp = fopen(cmd, "r");
        if(fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                strstripspace(line);
                sscanf(line, "%[^:]:%s", key, value);
                if(strcmp(key, "runtime_version") == 0)
                {
                    break;
                }
            }
            fclose(fp);
            char remove_path[1024] = "\0";
            snprintf(remove_path, sizeof(remove_path), "rm \"%s\"", cmd);
            int remove = system(remove_path);
            syslog(LOG_INFO, "value = %s\n", value);

            if(value[0] != '\0')
            {
                if(strstr(value, "v11"))
                {
                    order = 1;
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                    {
                        system(rm_dir);
                        return 0;
                    }

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v11") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v11", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "ok2") && flag != 1)
                {
                    if(order == 0)
                    {
                        order = 2;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                    {
                        system(rm_dir);
                        return 0;
                    }

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "openkylin2.0") == 0)
                        {
                            flag = 1;
                            strncpy(value, "openkylin2.0", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "v10sp1") && flag != 1)
                {
                    if(order == 0)
                    {
                        order = 3;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                    {
                        system(rm_dir);
                        return 0;
                    }

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v10sp1") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v10sp1", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "v10") && flag != 1 && !strstr(value, "sp1"))
                {
                    if(order == 0)
                    {
                        order = 4;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                    {
                        system(rm_dir);
                        return 0;
                    }

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v10") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v10", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }

                if(flag != 1)
                {
                    if(order == 1)
                    {
                        int base = kdk_kare_build_base_env("v11", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v11", sizeof(value) - 1);
                        }
                    }
                    else if(order == 2)
                    {
                        int base = kdk_kare_build_base_env("openkylin2.0", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "openkylin2.0", sizeof(value) - 1);
                        }
                    }
                    else if(order == 3)
                    {
                        int base = kdk_kare_build_base_env("v10sp1", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v10sp1", sizeof(value) - 1);
                        }
                    }
                    else if(order == 4)
                    {
                        int base = kdk_kare_build_base_env("v10", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v10", sizeof(value) - 1);
                        }
                    }
                }
            }
        }
        else
        {
            KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
            if (NULL == setting)
            {
                syslog(LOG_ERR, "get handle failed\n");
                system(rm_dir);
                return 0;
            }

            runtime = kdk_conf2_get_string(setting, "kare-runtime");
            if (NULL != runtime)
            {
                syslog(LOG_INFO, "kare-runtime value: %s\n", runtime);
                kdk_conf2_ksettings_destroy(setting);
            }
        }

        if(runtime)
        {
            fp = popen("kare -E", "r");
            if(!fp)
            {
                system(rm_dir);
                return 0;
            }

            memset(line, 0, 1024);
            while (fgets(line, sizeof(line), fp))
            {
                strstripspace(line);
                if(strcmp(line, runtime) == 0)
                {
                    env = 1;
                    break;
                }
            }
            pclose(fp);

            if(env)
            {
                strncpy(value, runtime, sizeof(value) - 1);
                flag = 1;
            }
            else
            {
                // InstallCallBack build_call_back = call_back;
                int base = kdk_kare_build_base_env(runtime, call_back);
                if(base == 1)
                {
                    strncpy(value, runtime, sizeof(value) - 1);
                    flag = 1;
                }
            }
            free(runtime);
        }

        syslog(LOG_INFO, "flag = %d, value = %s\n", flag, value);
        
        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            //创建用户
            info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                    "/org/kylin/kare", // object to call on
                                                    "org.kylin.kare",  // interface to call on
                                                    "install");               // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            char *env = strdup(value);
			char *uri = strdup(deb_path);
            syslog(LOG_INFO, "kdk : d-bus env : %s\n", env);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                free(env);
				free(uri);
                goto err_out;
            }
            free(env);
			free(uri);

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

            if(ret)
            {
                mark = 1;
            }
        }
    }

    if(mark)
    {
        mark = 0;
        running_install = 1;
        _callbackinstall = call_back;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, NULL);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }
    system(rm_dir);
    return mark;
err_out:
    system(rm_dir);
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);

    return 0;
}

static void *dbus_kaiming_offline_line_install_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    char *info = NULL;

    int timeout_sec = 180;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='progresschangesignal'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "progresschangesignal")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &info, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "info = %s\n", info);
                    cJSON *json = cJSON_Parse(info);
                    if (json == NULL) {
                        dbus_connection_unref(connection);
                        pthread_exit(NULL);
                    }

                    cJSON *progress = cJSON_GetObjectItem(json, "progress");
                    if (progress != NULL) {
                        _callbackinstall(progress->valueint, 1, "");
                        errorCode = -1;
                    }
                }
            }
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "kaiming online install info name : %s, operateType : %d, errorCode : %d, errinfo : %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0)
                    {
                        _callbackinstall(100, 0, errinfo);
                        errorCode = -1;
                        mark = 1;
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming offline install fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackinstall(0, -1, errinfo);
                        errorCode = -1;
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_install_offline(const char *uri, InstallCallBack call_back)
{
    int flag = 0;
    int mark = 0;

    running_install = 1;
    _callbackinstall = call_back;
    pthread_create(&thread_install, NULL, dbus_kaiming_offline_line_install_listener_thread, NULL);

    DBusConnection *conn;
    DBusError err;

    DBusMessage *info_msg = NULL;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessage *replyMsgBranch = NULL;

    cJSON *branch_name = NULL;
    cJSON *json = NULL;

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        syslog(LOG_ERR, "kaiming\n");
        
        char *branch = "";
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            goto err_out;
        }        

        info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "installOfflinePackage");           // method name

        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            goto err_out;
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);

        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            goto err_out;
        }

        DBusMessageIter args;
        bool *res = false;
                
        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &res);
        }

        dbus_message_unref(info_msg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_message_unref(replyMsg);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);
    }

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
    
    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_debian_offline_install_listener_thread(void *arg)
{
    FILE *fp = (FILE *)arg;
    char line[1024] = "\0";
    int count = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int mark = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    int extra_line = 0;
    if(running_install)
    {
        while (fgets(line, sizeof(line), fp))
        {
            syslog(LOG_INFO, "debian install offline info : %s\n", line);

            if (strstr(line, "准备解压") && !stage_flags[1]) {
                progress += 20;  // 准备解压占 20%
                stage_flags[1] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "正在解压") && !stage_flags[2]) {
                progress += 30;  // 解压阶段占 30%
                stage_flags[2] = 1;
                _callbackinstall(progress, 1, line);
            } else if (strstr(line, "正在设置") && !stage_flags[3]) {
                progress += 30;  // 设置阶段占 30%
                stage_flags[3] = 1;
                _callbackinstall(progress, 1, line);
            }

            if (strstr(line, "dpkg: 依赖关系问题使得") || strstr(line, "dpkg: 处理归档") || strstr(line, "dpkg: 错误: 另外一个进程已经为 dpkg frontend lock 加锁"))
            {
                capture = 1;  // 开始捕获
            }

            if (capture) 
            {
                size_t len = strnlen(line, STRNLEN_MAX);
                if (total_len + len < sizeof(output) - 1) 
                {
                    strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                    total_len += len;
                } else {
                    break;  // 防止溢出
                }
            }

            if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错")) 
            {
                extra_line = 1;  // 让下一行也加入输出
            }
            else if (strstr(line, "dpkg: 处理归档") && strstr(line, "时出错"))
            {
                extra_line = 1;  // 处理 "dpkg: 处理归档 ..." 也要多输出一行
            } 
            else if (extra_line) 
            {
                capture = 0;
                extra_line = 0;  // 只多输出一行
            }

            if(strstr(line, "在处理时有错误发生"))
            {
                flag = 1;
                syslog(LOG_ERR, "debian offline install fail\n");
                break;
            }
            count++;
        }
        pclose(fp);

        if((!flag) && (count > 3))
        {
            _callbackinstall(100, 0, line);
            mark = 1;
        }
        else
        {
            _callbackinstall(0, -1, output);
        }

    }

    return (void *)(intptr_t)mark;
}

static int kdk_debian_install_offline(const char *uri, InstallCallBack call_back)
{
    syslog(LOG_INFO, "debian\n");
    FILE *fp = NULL;
    char cmd[1024] = "\0";
    int mark = 0;
    snprintf(cmd, sizeof(cmd), "dpkg -i \"%s\" 2>&1", uri);

    fp = popen(cmd, "r");
    if(!fp)
        return 0;

    running_install = 1;
    _callbackinstall = call_back;
    pthread_create(&thread_install, NULL, dbus_debian_offline_install_listener_thread, (void *) fp);

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
    return mark;
}

static int kdk_kare_install_offline(const char *uri, InstallCallBack call_back)
{
    if(!uri)
    {
        return 0;
    }

    struct utsname utsbuf;
    if (uname(&utsbuf) == -1)
    {
        return 0;
    }

    char *arch = strdup(utsbuf.machine);
    if(strstr(arch, "sw"))
    {
         kdk_boot_mode mode = kdk_get_boot_mode();
        if(mode == MODE_MAINTAIN)
        {
            int installRet = kdk_debian_install_offline(uri, call_back);
            return installRet;
        }
        else
        {
            syslog(LOG_INFO, "kare arch sw\n");
            return 0;
        }
    }

	char fileNamePath[512] = "\0";
	strncpy(fileNamePath, uri, sizeof(fileNamePath) - 1);

    char *file_name = basename(fileNamePath);

    // 查找第一个下划线的位置
    char **underscore = strsplit(file_name, '_');

    // 如果没有找到下划线，返回空指针
    if (strnlen(underscore[0], STRNLEN_MAX) == 0) {
        return 0;
    }
    FILE *fp = NULL;
    fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return 0;

    char buffer[4097];
    char *val;
    int same = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, underscore[0]) == 0)
            {
                same = 1;
                break;
            }
        }
    }
    fclose(fp);
    if (1 == same)
    {
		_callbackinstall = call_back;
        _callbackinstall(0, -1, "系统中有同名软件包");
        return 0;
    }
    int mark = 0;
    int flag = 0;
    char cmd[1024]= "\0";

    char line[1024] = "\0";
    char key[128] = "\0";
    char value[128] = "\0";
    char *runtime = NULL;
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    char filepath[512] = "\0";
    strncpy(filepath, uri, sizeof(filepath) - 1); 
    char *dir = dirname(filepath);

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "kare\n");
        snprintf(cmd, sizeof(cmd), "ar -x --output \"%s\" \"%s\" _compat", dir, uri);
        int ret = system(cmd);
        if(ret != 0)
            return 0;

        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "%s/_compat", dir);
        
        fp = fopen(cmd, "r");
        if(fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                strstripspace(line);
                sscanf(line, "%[^:]:%s", key, value);
                if(strcmp(key, "runtime_version") == 0)
                {
                    break;
                }
            }
            fclose(fp);
            char remove_path[1024] = "\0";
            snprintf(remove_path, sizeof(remove_path), "rm \"%s\"", cmd);
            int remove = system(remove_path);
            syslog(LOG_INFO, "value = %s\n", value);

            if(value[0] != '\0')
            {
                if(strstr(value, "v11"))
                {
                    order = 1;
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                        return 0;

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v11") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v11", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "ok2") && flag != 1)
                {
                    if(order == 0)
                    {
                        order = 2;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                        return 0;

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "openkylin2.0") == 0)
                        {
                            flag = 1;
                            strncpy(value, "openkylin2.0", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "v10sp1") && flag != 1)
                {
                    if(order == 0)
                    {
                        order = 3;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                        return 0;

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v10sp1") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v10sp1", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }
                if(strstr(value, "v10") && flag != 1 && !strstr(value, "sp1"))
                {
                    if(order == 0)
                    {
                        order = 4;
                    }
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);
                    fp = popen("kare -E", "r");
                    if(!fp)
                        return 0;

                    memset(line, 0, 1024);
                    while (fgets(line, sizeof(line), fp))
                    {
                        strstripspace(line);
                        if(strcmp(line, "v10") == 0)
                        {
                            flag = 1;
                            strncpy(value, "v10", sizeof(value) - 1);
                            break;
                        }
                    }
                    pclose(fp);
                }

                if(flag != 1)
                {
                    if(order == 1)
                    {
                        int base = kdk_kare_build_base_env("v11", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v11", sizeof(value) - 1);
                        }
                    }
                    else if(order == 2)
                    {
                        int base = kdk_kare_build_base_env("openkylin2.0", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "openkylin2.0", sizeof(value) - 1);
                        }
                    }
                    else if(order == 3)
                    {
                        int base = kdk_kare_build_base_env("v10sp1", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v10sp1", sizeof(value) - 1);
                        }
                    }
                    else if(order == 4)
                    {
                        int base = kdk_kare_build_base_env("v10", call_back);
                        if(base == 1)
                        {
                            flag = 1;
                            strncpy(value, "v10", sizeof(value) - 1);
                        }
                    }
                }
            }
        }
        else
        {
            KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
            if (NULL == setting)
            {
                syslog(LOG_ERR, "get handle failed\n");
                return 0;
            }

            runtime = kdk_conf2_get_string(setting, "kare-runtime");
            if (NULL != runtime)
            {
                syslog(LOG_INFO, "kare-runtime value: %s\n", runtime);
                kdk_conf2_ksettings_destroy(setting);
            }
        }

        if(runtime)
        {
            fp = popen("kare -E", "r");
            if(!fp)
                return 0;

            memset(line, 0, 1024);
            while (fgets(line, sizeof(line), fp))
            {
                strstripspace(line);
                if(strcmp(line, runtime) == 0)
                {
                    env = 1;
                    break;
                }
            }
            pclose(fp);

            if(env)
            {
                strncpy(value, runtime, sizeof(value) - 1);
                flag = 1;
            }
            else
            {
                // InstallCallBack build_call_back = call_back;
                int base = kdk_kare_build_base_env(runtime, call_back);
                if(base == 1)
                {
                    strncpy(value, runtime, sizeof(value) - 1); 
                    flag = 1;
                }
            }
            free(runtime);
        }

        syslog(LOG_INFO, "flag = %d, value = %s\n", flag, value);
        
        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            //创建用户
            info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                    "/org/kylin/kare", // object to call on
                                                    "org.kylin.kare",  // interface to call on
                                                    "install");               // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            char *env = strdup(value);
            syslog(LOG_INFO, "kdk : d-bus env : %s\n", env);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                free(env);
                goto err_out;
            }
            free(env);

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

            if(ret)
            {
                mark = 1;
            }

        }
    }

    if(mark)
    {
        mark = 0;
        running_install = 1;
        _callbackinstall = call_back;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, NULL);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }

    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

int kdk_package_install_package_offline(const char *uri, int list_mode, InstallCallBack call_back)
{
    if (0 != access(uri, F_OK))
    {
        return 0;
    }
    syslog(LOG_INFO, "offline\n");
    int ret = 0;
    char filename[512] = "\0";
    switch (list_mode)
    {
    case KDK_PACKAGE_KAIMING:
        ret = kdk_kaiming_install_offline(uri, call_back);
        break;
    case KDK_PACKAGE_KARE:
        ret = kdk_kare_install_offline(uri, call_back);
        break;
    case KDK_PACKAGE_DEBIAN:
        ret = kdk_debian_install_offline(uri, call_back);
        break;
    case KDK_PACKAGE_AUTO:
        strncpy(filename, uri, 511); 
        const char *dot = strrchr(filename, '.');
        // 如果没有找到点，或者点是文件名的第一个字符，则返回 false
        if (dot == NULL || dot == filename) {
            return 0;
        }
        // 检查从点开始的子字符串是否与后缀匹配
        if(strcmp(dot, ".ok") == 0)
        {
            ret = kdk_kaiming_install_offline(uri, call_back);
        }
        else if(strcmp(dot, ".deb") == 0)
        {
            int flag = 0;
            memset(filename, 0, 512);
            strncpy(filename, uri, 511);

            char *file_name = basename(filename);

            // 查找第一个下划线的位置
            char *underscore_pos = strchr(file_name, '_');

            // 如果没有找到下划线，返回空指针
            if (underscore_pos == NULL) {
                return 0;
            }

            // 将下划线及其后的部分替换为终止符
            *underscore_pos = '\0';
            FILE *fp = fopen("", "r");
            if(fp)
            {
                char line[1024] = "\0";
                while (fgets(line, 1024, fp))
                {
                    if(strcmp(line, file_name))
                    {
                        flag = 1;
                        break;
                    }
                }

                if(flag)
                    ret = kdk_debian_install_offline(uri, call_back);
                else
                    ret = kdk_kare_install_offline(uri, call_back);
            }
            else
                ret = kdk_kare_install_offline(uri, call_back);
        }
        else
            break;

        break;
    default:
        break;
    }

    return ret;
}

int kdk_package_install_package_online(const char *pkg_name, int list_mode, InstallCallBack call_back)
{
    if(!pkg_name)
        return 0;

    int ret = 0;
    syslog(LOG_ERR, "online\n");
    int deb = 0;
    char package_name[128] = "\0";
    char version[128] = "\0";
    switch (list_mode)
    {
    case KDK_PACKAGE_KAIMING:
        ret = kdk_kaiming_install_online(pkg_name, call_back);
        break;
    case KDK_PACKAGE_DEBIAN:
        ret = kdk_debian_install_online(pkg_name, call_back);
        break;
    case KDK_PACKAGE_KARE:
        ret = kdk_kare_install_online(pkg_name, call_back);
        break;
    case KDK_PACKAGE_AUTO:
        ret = kdk_kaiming_install_online(pkg_name, call_back);
        if(!ret)
        {
            KSettings *setting = kdk_conf2_new("kylin-debs-whiteList", NULL);
            if (NULL == setting)
            {
                syslog(LOG_ERR, "get handle failed\n");
            }

            char **sv = kdk_conf2_get_strv(setting, "debs-list");
            if (sv)
            {
                for (int i = 0; sv[i]; i++)
                {
                    if(strstr(pkg_name, "="))
                    {
                        sscanf(pkg_name, "%[^=]= %s", package_name, version);
                        syslog(LOG_DEBUG, "package_name = %s, version = %s", package_name, version);
                        if(strcmp(sv[i], package_name) == 0)
                        {
                            deb = 1;
                            break;
                        }
                    }
                    else
                    {
                        if(strcmp(sv[i], pkg_name) == 0)
                        {
                            deb = 1;
                            break;
                        }
                    }
                }
                g_strfreev(sv);
                // syslog(LOG_INFO, "kare-runtime value: %s\n", runtime);
                kdk_conf2_ksettings_destroy(setting);
            }
            if (deb)
            {
                ret = kdk_debian_install_online(pkg_name, call_back);
            }
            else
            {
                ret = kdk_kare_install_online(pkg_name, call_back);
            }
        }
        break;
    default:
        break;
    }
    return ret;   
}

int kdk_package_install_package(const char *uri, int list_mode, InstallCallBack call_back)
{
    if(!uri)
        return 0;
    int ret = 0;
    char *dir = NULL;
    char filepath[256] = "\0";
    strncpy(filepath, uri, 255);
    dir = dirname(filepath);
    if(dir && strcmp(dir, ".") != 0)
    {
        ret = kdk_package_install_package_offline(uri, list_mode, call_back);
    }
    else
        ret = kdk_package_install_package_online(uri, list_mode, call_back);

    return ret;
}

static pthread_t thread_remove = 0;
static volatile int running_remove = 0;
RemoveCallBack _callbackremove = NULL;

static void *dbus_kare_remove_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int status = -1;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;
    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='removeStatusChanged'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_remove)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                continue;
            }

            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kare", "removeStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, status = %d, errinfo = %s\n", name, status, errinfo);
                    // if (status == 0)
                    // {
                    //     mark = 1;
                    //     break;
                    // }
                    // else if(status == -1)
                    // {
                    //     syslog(LOG_ERR, "kare remove fail! status : %s, errinfo = %s\n", status, errinfo);
                    //     break;
                    // }
                    // else
                    //     continue;

                    if (status != -1)
                    {
                        if(status == 0)
                        {
                            _callbackremove(100, status, "卸载成功");
                            mark = 1;
                            break;
                        }
                        // status = -1;
                    }
                    else
                    {
                        _callbackremove(0, status, errinfo);
                        status = -1;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kare_remove(const char *pkg_name, RemoveCallBack call_back)
{
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;
    // _callbackremove = call_back;

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusConnection *conn;
    DBusError err;

    if (0 == access("/usr/bin/kare", F_OK))
    {
        // syslog(LOG_INFO, "kare\n");
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    // pclose(fp);
                    flag = 1;
                    break;
                }
            }
            pclose(fp);
        }
    }

    if (1 == flag)
    {
        syslog(LOG_INFO, "kare\n");

        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (NULL == conn)
        {
            goto err_out;
        }

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        //创建用户
        info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                "/org/kylin/kare", // object to call on
                                                "org.kylin.kare",  // interface to call on
                                                "remove");               // method name
        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            goto err_out;
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);
        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            goto err_out;
        }

        DBusMessageIter args;
        bool *ret = false;

        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &ret);
        }

        dbus_message_unref(info_msg);
        dbus_message_unref(replyMsg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);

        if(ret)
        {
            running_remove = 1;
            pthread_create(&thread_remove, NULL, dbus_kare_remove_listener_thread, NULL);

            void *thread_result;
            pthread_join(thread_remove, &thread_result);

            if(thread_result)
            {
                mark = (int)(intptr_t)thread_result;
            }
        }
    }
    else
    {
        _callbackremove(0, 1, "应用未安装");
    }

    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_kaiming_remove_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    int mark = 0;

    int timeout_sec = 180;
    int poll_interval = 1;
    int elapsed_time = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if(running_remove)
        {
            int result = dbus_connection_read_write(connection, poll_interval * 1000);
            
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }

            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, operateType = %d, errorCode = %d, errinfo = %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0)
                    {
                        mark = 1;
                        _callbackremove(100, 0, errinfo);
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming remove fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackremove(0, -1, errinfo);
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_close(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_remove(const char *pkg_name, RemoveCallBack call_back)
{
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;

    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessage *replyMsgBranch = NULL;

    running_remove = 1;
    // _callbackremove = call_back;
    pthread_create(&thread_remove, NULL, dbus_kaiming_remove_listener_thread, NULL);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        const char *kaiming = "/opt/kaiming-tools/bin/kaiming list";
        fp = popen(kaiming, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %*s %*s %*s %s %*s %*s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    pclose(fp);
                    flag = 1;
                    break;
                }
            }
        }
    }

    if (1 == flag)
    {
        syslog(LOG_INFO, "kaiming\n");
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            return 0;
        }     

        // branch_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
        //                                         "/org/kylin/kaiming",     // object to call on
        //                                         "org.kylin.kaiming.interface", // interface to call on
        //                                         "getLocalInfo");           // method name

        // if (!branch_msg)
        // { // -1 is default timeout
        //     syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
        //     return 0;
        // }

        // if (!dbus_message_append_args(branch_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
        // {
        //     syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
        //     return 0;
        // }

        // if (!dbus_connection_send_with_reply(conn, branch_msg, &sendMsgPendingBranch, -1))
        // {
        //     syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
        //     return 0;
        // }

        // if (sendMsgPendingBranch == NULL)
        // {
        //     syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
        //     return 0;
        // }

        // dbus_connection_flush(conn);

        // if (branch_msg)
        // {
        //     dbus_message_unref(branch_msg);
        // }

        // dbus_pending_call_block(sendMsgPendingBranch);

        // replyMsgBranch = dbus_pending_call_steal_reply(sendMsgPendingBranch);

        // if (replyMsgBranch == NULL)
        // {
        //     syslog(LOG_ERR, "kdk : get reply message fail !\n");
        //     return 0;
        // }

        // if (sendMsgPendingBranch)
        // {
        //     dbus_pending_call_unref(sendMsgPendingBranch);
        // }

        // if (dbus_message_get_type(replyMsgBranch) == DBUS_MESSAGE_TYPE_ERROR) 
        // {
        //     syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        //     dbus_message_unref(replyMsgBranch);
        //     return 0;
        // }

        // DBusMessageIter args_branch;
        // char *branch = NULL;
                
        // if (!dbus_message_iter_init(replyMsgBranch, &args_branch))
        // {
        //     dbus_message_unref(replyMsgBranch);
        //     syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
        //     return 0;
        // }
        // else
        // {
        //     dbus_message_iter_get_basic(&args_branch, &branch);
        // }

        // if (replyMsgBranch)
        // {
        //     dbus_message_unref(replyMsgBranch);
        // }

        // cJSON *json = cJSON_Parse(branch);
        // if (json == NULL) {
        //     return 0;
        // }

        // cJSON *branch_name = cJSON_GetObjectItem(json, "branch");
        // if (branch_name == NULL || !cJSON_IsString(branch_name)) {
        //     cJSON_Delete(json);
        //     return 0;
        // }

        // if(strlen(branch_name->valuestring) != 0)
        // {
            char *branch = "";
            //
            info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                    "/org/kylin/kaiming",     // object to call on
                                                    "org.kylin.kaiming.interface", // interface to call on
                                                    "uninstall");           // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);

            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }

            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                goto err_out;
            }

            DBusMessageIter args;
            bool remove = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &remove);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            // dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

    }
    else
    {
        _callbackremove(0, 1, "应用未安装");
        return 0;
    }

    void *thread_result;
    pthread_join(thread_remove, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }

    return (int)(intptr_t)mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_debian_remove_listener_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int mark = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    int extra_line = 0;
    size_t len = 0;

    if(running_remove)
    {
        char line[1024] = "\0";
        while (fgets(line, sizeof(line), args->param_fp))
        {
            syslog(LOG_INFO, "debian remove info : %s\n", line);

            if (strstr(line, "正在卸载") && !stage_flags[1]) {
                progress += 20;  // 准备解压占 20%
                stage_flags[1] = 1;
                _callbackremove(progress, 1, line);
            } else if (strstr(line, "正在清除") && !stage_flags[2]) {
                progress += 30;  // 解压阶段占 30%
                stage_flags[2] = 1;
                _callbackremove(progress, 1, line);
            } else if (strstr(line, "正在处理用于") && !stage_flags[4]) {
                progress += 30;  // 触发器处理占 10%
                stage_flags[4] = 1;
                _callbackremove(progress, 1, line);
            }

            if (strstr(line, "dpkg: 依赖问题阻止了卸载") || (strstr(line, "dpkg: 警告:") && strstr(line, "不允许的操作")))
            {
                capture = 1;  // 开始捕获
            }

            if (capture) 
            {
                len = strnlen(line, STRNLEN_MAX);
                if (total_len + len < sizeof(output) - 1) 
                {
                    strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                    total_len += len;
                } else {
                    break;  // 防止溢出
                }
            }

            if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错")) 
            {
                if(strnlen(output, STRNLEN_MAX) == 0)
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }
                extra_line = 1;  // 让下一行也加入输出
                capture = 1;
            }
            else if (strstr(line, "dpkg: 处理归档") && strstr(line, "时出错"))
            {
                if(strnlen(output, STRNLEN_MAX) == 0)
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }
                extra_line = 1;  // 处理 "dpkg: 处理归档 ..." 也要多输出一行
                capture = 1;
            } 
            else if (extra_line) 
            {
                capture = 0;
                extra_line = 0;  // 只多输出一行
            }

            // if(!strstr(line, "错误") && !strstr(line, "警告"))
            // {
            //     flag = 1;
            // }
            // else
            // {
            //     if(strstr(line, "现假定该软件包目前没有任何文件被安装在系统里"))
            //     {
            //         flag = 1;
            //     }
            //     else
            //     {
            //         flag = 0;
            //         break;
            //     }
            // }
        }
        pclose(args->param_fp);

        int ret =  kdk_package_is_installed(args->param_name, NULL);
        syslog(LOG_INFO, "%s package is installed : %d\n", args->param_name, ret);

        if(ret != 0)
        {
            _callbackremove(100, 0, line);
            mark = 1;
            syslog(LOG_INFO, "debian remove success");
        }
        else
        {
            _callbackremove(0, -1, output);
            syslog(LOG_INFO, "debian remove fail");
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_debian_remove(const char *pkg_name, RemoveCallBack call_back)
{
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;
    // _callbackremove = call_back;

    fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return 0;

    char buffer[4097] = "\0";
    char *val;
    
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, pkg_name) == 0)
            {
                flag = 1;
                break;
            }
        }
    }
    fclose(fp);
    if (1 == flag)
    {
        syslog(LOG_INFO, "debian\n");
        char command[256] = "\0";
        snprintf(command, sizeof(command), "dpkg -P %s 2>&1", pkg_name);
        fp = popen(command, "r");
        if (!fp)
            return 0;

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        strncpy(args->param_name, pkg_name, sizeof(args->param_name) - 1);
        args->param_fp = fp;

        running_remove = 1;
        pthread_create(&thread_remove, NULL, dbus_debian_remove_listener_thread, (void *) args);

        void *thread_result;
        pthread_join(thread_remove, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }
    else
    {
        _callbackremove(0, 1, "应用未安装");
    }

    return mark;
}

int kdk_package_remove_package(const char *pkg_name, RemoveCallBack call_back)
{
    if(!pkg_name)
        return 0;

    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;
    int ret = 0;
    _callbackremove = call_back;

    ret = kdk_kare_remove(pkg_name, call_back);

    if(ret)
        return ret;
    
    ret = kdk_kaiming_remove(pkg_name, call_back);

    if(ret)
        return ret;

    ret = kdk_debian_remove(pkg_name, call_back);

    if(ret)
        return ret;

    _callbackremove(100, 0, "应用未安装");
    return ret;
}

char *kdk_package_get_version(const char *name)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Version")))
            break;
    }
    fclose(fp);

    if (val)
        return strdup(val);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        char *str = NULL;

        dbus_connection_flush(connection);

        if (message)
        {
            // 释放message
            dbus_message_unref(message);
            message = NULL;
        }

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;

        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &str);
        }

        if (reply)
        {
            // 释放reply
            dbus_message_unref(reply);
            reply = NULL;
        }
        dbus_connection_unref(connection);
        connection = NULL;

        klog_info("get version Got string: %s\n", str);
        if(str)
        {
            cJSON *root;
            root = cJSON_Parse(str);

            cJSON *km_version = cJSON_GetObjectItem(root, "version");
            if(km_version)
            {
                klog_info("km_version->valuestring = %s\n", km_version->valuestring);
                
                if (km_version->valuestring && strnlen(km_version->valuestring, STRNLEN_MAX) != 0)
                {
                    return strdup(km_version->valuestring);
                }
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char *line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if (0 == strcmp(tmp_name, name))
                {
                    pclose(fp);
                    return strdup(tmp_version);
                }
            }
            pclose(fp);
        }
    }

    return NULL;
}

int kdk_package_is_installed(const char *name, const char *version)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return -1;

    char buffer[4097];
    char *val;
    int flag = -1;
    int mark = 0;
    char *ver = NULL;
    while (fgets(buffer, 4096, fp))
    {
        if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
        {
            if ((val = lookup(buffer, "Package")))
            {
                if (strcmp(val, name) == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    mark = 1;
                }
            }
            if(mark == 1)
            {
                if ((ver = lookup(buffer, "Version")))
                {
                    mark++;
                    if (strcmp(ver, version) == 0)
                    {
                        syslog(LOG_DEBUG, "ver = %s, version = %s", ver, version);
                        flag = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            if ((val = lookup(buffer, "Package")))
            {
                if (strcmp(val, name) == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    flag = 0;
                    break;
                }
            }
        }
    }
    fclose(fp);
    if (0 == flag)
        return flag;
    
    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char *line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                {
                    if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                    {
                        syslog(LOG_DEBUG, "tmp_name = %s, tmp_version = %s", tmp_name, tmp_version);
                        flag = 0;
                        break;
                    }
                }
                else
                {
                    if (0 == strcmp(tmp_name, name))
                    {
                        syslog(LOG_DEBUG, "tmp_name = %s", tmp_name);
                        flag = 0;
                        break;
                    }
                }
            }
            pclose(fp);
        }
    }
    
    if (0 == flag)
        return flag;

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *conn;
        DBusError err;
        bool ret = false;
        char *pkgname = NULL;

        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (NULL == conn)
        {
            goto out;
        }

        if (dbus_error_is_set(&err))
        {
            klog_err("Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        DBusMessage *info_msg = NULL;
        DBusPendingCall *sendMsgPending = NULL;
        DBusMessage *replyMsg = NULL;

        //创建用户
        info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "isInstalled");           // method name
        if (!info_msg)
        { // -1 is default timeout
            klog_err("status_msg：dbus_message_new_method_call调用失败\n");
            goto out;
        }

        if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
        {
            char res[256] = "\0";
            snprintf(res, sizeof(res), "%s/%s", name, version);
            pkgname = strdup(res);
        }
        else
        {
            pkgname = strdup(name);
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkgname, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            free(pkgname);
            goto out;
        }
        free(pkgname);

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
        if(ret)
        {
            flag = 0;
        }
    out:
        if (conn)
        {
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
        }
    }
    return flag;
}

char *kdk_package_get_description(const char *name)
{
    if (NULL == name)
        return NULL;

    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097], result[4096];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Description")))
        {
            strncat(result, val, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            // strcat(result,"\n");
            // while (fgets(buffer,4096, fp))
            // {
            //     if ((val = lookup(buffer, "Homepage")))
            //         break;
            //     strcat(result, buffer);
            // }
            break;
        }
    }
    fclose(fp);

    if (val)
        return strdup(val);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        char *str = NULL;

        dbus_connection_flush(connection);

        if (message)
        {
            // 释放message
            dbus_message_unref(message);
            message = NULL;
        }

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;

        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &str);
        }

        if (reply)
        {
            // 释放reply
            dbus_message_unref(reply);
            reply = NULL;
        }
        dbus_connection_unref(connection);
        connection = NULL;

        klog_info("get version Got string: %s\n", str);
        if(str)
        {
            cJSON *root;
            root = cJSON_Parse(str);

            cJSON *km_des = cJSON_GetObjectItem(root, "description");
            if(km_des)
            {
                klog_info("km_des->valuestring = %s\n", km_des->valuestring);
                
                if (km_des->valuestring && strnlen(km_des->valuestring, STRNLEN_MAX) != 0)
                {
                    strstripspace(km_des->valuestring);
                    return strdup(km_des->valuestring);
                }
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    return NULL;
}

char **kdk_package_get_code_path(const char *name)
{
    if (NULL == name)
        return NULL;

    char **result = calloc(1, sizeof(char *));
    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
    {
        free(result);
        return 0;
    }

    int i = 0;
    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            free(result);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
        {
            char *dir_name = dirname(line);
            if ((i > 0) && (0 == strcmp(result[i - 1], dir_name)))
                continue;
            char **tmp = realloc(result, sizeof(char *) * (i + 2));
            if (NULL == tmp)
            {
                for (; i; i--)
                {
                    free(result[i]);
                }
                free(result);
                result = NULL;
                break;
            }
            result = tmp;
            result[i++] = strdup(dir_name);
            result[i] = NULL;
        }
    }
    fclose(fp);
    return result;
}

int kdk_package_get_file_count(const char *name)
{
    if (NULL == name)
        return 0;

    int result = 0;
    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
            result++;
    }
    fclose(fp);
    return result;
} 

bool kdk_package_is_removable(const char *pkg_name)
{
    kdk_boot_mode mode = kdk_get_boot_mode();

    if(mode == MODE_NORMAL)
    {
        const char *dpkg = "dpkg -l";
        FILE *fp = popen(dpkg, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%*s %s %*s %*s %*s", tmp_name);
                if (count != 1)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    return false;
                }
            }
        }

        if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
        {
            const char *kaiming = "/opt/kaiming-tools/bin/kaiming list";
            fp = popen(kaiming, "r");
            if (NULL != fp)
            {
                char line[1024] = "\0";
                while (fgets(line, sizeof(line), fp))
                {
                    char tmp_name[256], tmp_version[64];
                    int count = sscanf(line, "%s %*s %*s %*s %*s %*s %*s", tmp_name);
                    if (count != 1)
                        continue;
                    if (0 == strcmp(tmp_name, pkg_name))
                    {
                        pclose(fp);
                        return true;
                    }
                }
                pclose(fp);
            }
        }

        if (0 == access("/usr/bin/kare", F_OK))
        {
            const char *kare = "kare -l";
            fp = popen(kare, "r");
            if (NULL != fp)
            {
                char line[1024] = "\0";
                while (fgets(line, sizeof(line), fp))
                {
                    char tmp_name[256], tmp_version[64];
                    int count = sscanf(line, "%s %*s", tmp_name);
                    if (count != 1)
                        continue;
                    if (0 == strcmp(tmp_name, pkg_name))
                    {
                        pclose(fp);
                        return true;
                    }
                }
                pclose(fp);
            }
        }
        return false;
    }
    else
    {
        return true;
    }
}

int kdk_package_verify_disk_space(const char *path)
{
    if (NULL == path)
        return 0;

    int result = 0;

    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -I %s", path);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    char *val = NULL;
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        if ((val = lookup(line, "Installed-Size")))
        {
            break;
        }
    }
    fclose(fp);

    if (NULL == val)
        return 0;

    FILE *mounts = fopen("/proc/mounts", "r");

    char partition[100], mount_point[100];
    while (fscanf(mounts, "%s %s", partition, mount_point) != EOF)
    {
        if (strcmp(mount_point, "/") == 0)
        {
            break;
        }
    }
    fclose(mounts);

    struct statfs fs_stat;
    if (statfs(partition, &fs_stat) < 0)
    {
        return 0;
    }
    long install_size = atol(val) * 1024;
    long avail_space = fs_stat.f_bavail * fs_stat.f_bsize;

    return avail_space > install_size;
}

char *kdk_package_get_default_browser(void)
{
    return read_default_app("x-scheme-handler/http");
}

char *kdk_package_get_default_image_viewer(void)
{
    return read_default_app("image/bmp");
}

char *kdk_package_get_default_audio_player(void)
{
    return read_default_app("audio/mpeg");
}

char *kdk_package_get_default_video_player(void)
{
    return read_default_app("video/mp4");
}

char *kdk_package_get_default_pdf_viewer(void)
{
    return read_default_app("application/pdf");
}

char *kdk_package_get_default_word_viewer(void)
{
    return read_default_app("application/wps-office.docx");
}

char *kdk_package_get_default_excel_viewer(void)
{
    return read_default_app("application/wps-office.xls");
}

char *kdk_package_get_default_ppt_viewer(void)
{
    return read_default_app("application/wps-office.ppt");
}

kdk_app_info* kdk_package_get_startmenu_list(void)
{
    kdk_app_info *programlist = (kdk_app_info *)calloc(1, sizeof(kdk_app_info));
    if (!programlist)
        return NULL;

    get_desktop_info(KDK_DEKSTOP_FILE_PATH, programlist, 1);

    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    snprintf(filepath, sizeof(filepath), "%s%s", homeDir, KDK_LOCALE_DEKSTOP_FILE_PATH);
    get_desktop_info(filepath, programlist, 1);

    get_desktop_info(KDK_SNAPD_DEKSTOP_FILE_PATH, programlist, 1);

    get_desktop_info(KDK_FLATPAK_DEKSTOP_FILE_PATH, programlist, 1);

    return programlist;
}

kdk_app_info *kdk_package_free_startmenu_list(kdk_app_info *list)
{
    if (NULL == list)
        return;
    for (int i = 0; i < list->nums; i++)
        free(list->list[i]);
    free(list);
}

kdk_app_info *kdk_package_get_application_list(void)
{
    kdk_app_info *programlist = (kdk_app_info *)calloc(1, sizeof(kdk_app_info));
    if (!programlist)
        return NULL;

    get_desktop_info(KDK_DEKSTOP_FILE_PATH, programlist, 0);

    return programlist;
}

void kdk_package_free_app_info(kdk_app_info *list)
{
    if (NULL == list)
        return;
    for (int i = 0; i < list->nums; i++)
        free(list->list[i]);
    free(list);
}

void kdk_package_unregister_install_package_handle()
{
    if (NULL != _callbackinstall)
    {
        running_install = 0;
        _callbackinstall = NULL;
        // pthread_exit(thread_install);
    }
}

void kdk_package_unregister_remove_package_handle()
{
    if (NULL != _callbackremove)
    {
        running_remove = 0;
        _callbackremove = NULL;
        pthread_join(thread_remove, NULL);
    }
}

char* kdk_package_get_name(char *desktop)
{
    if(!desktop)
        return NULL;
    if (0 == access(desktop, F_OK))
    {
        char cmd[512] = "\0";
        FILE *fp = NULL;
        char package_name[256] = "\0";
        char file_path[256] = "\0";
        char line[1024] = "\0";
        char *name = NULL;

        snprintf(cmd, sizeof(cmd), "dpkg-query -S %s", desktop);
        fp = popen(cmd, "r");
        if (!fp) {
            return NULL;
        }

        while(fgets(line, sizeof(line), fp) != NULL) {
            sscanf(line, "%[^:]: %s", package_name, file_path);
            if(strcmp(desktop, file_path) == 0)
            {
                break;
            }
        }

        pclose(fp);

        if(package_name[0] != '\0')
        {
            name = (char *)malloc(sizeof(package_name) + 1);
            if(!name)
                return NULL;
            
            strncpy(name, package_name, sizeof(package_name));
            return name;
        }

        memset(line, 0, 1024);
        memset(package_name, 0, 256);
        memset(file_path, 0, 256);
        memset(cmd, 0, 512);
        snprintf(cmd, sizeof(cmd), "kare -S %s", desktop);
        fp = popen(cmd, "r");
        if (!fp) {
            return NULL;
        }

        while(fgets(line, sizeof(line), fp) != NULL) {
            sscanf(line, "%[^:]: %s", package_name, file_path);
            if(strcmp(desktop, file_path) == 0)
            {
                break;
            }
        }

        pclose(fp);

        if(package_name[0] != '\0')
        {
            name = (char *)malloc(sizeof(package_name) + 1);
            if(!name)
                return NULL;
            
            strncpy(name, package_name, sizeof(package_name));
            return name;
        }

        memset(line, 0, 1024);
        memset(package_name, 0, 256);
        memset(file_path, 0, 256);
        fp = fopen(desktop, "r");
        if(!fp)
            return NULL;

        while (fgets(line, 1024, fp))
        {
            if(strstr(line, "X-Kaiming"))
            {
                sscanf(line, "%*[^=]=%s", package_name);
            }
        }

        fclose(fp);
        if(package_name[0] != '\0')
        {
            name = (char *)malloc(sizeof(package_name) + 1);
            if(!name)
                return NULL;
            
            strncpy(name, package_name, sizeof(package_name));
            return name;
        }
    }        
    return NULL;
}

bool kdk_package_is_removable_by_desktop(const char *desktop)
{
    if (!desktop)
    {
        return false;
    }

    if (access(desktop, F_OK) != 0)
    {
        return false;
    }

    if (kdk_get_boot_mode() == MODE_MAINTAIN)
    {
        return true;
    }

    // 定义系统关键路径前缀数组
    static const char *prefixes[] = {
            "/usr",
            "/lib64",
            "/lib",
            "/sbin",
            "/bin",
            "/ostree",
            "/sysroot"
        };

    // 检查desktop文件是否位于系统关键路径
    size_t count = sizeof(prefixes) / sizeof(prefixes[0]);

    for (size_t i = 0; i < count; ++i) {
        size_t len = strnlen(prefixes[i], STRNLEN_MAX);
        if (strncmp(desktop, prefixes[i], len) == 0 &&
            (desktop[len] == '/' || desktop[len] == '\0')) {
            return false;
        }
    }
    return true;
}

kdk_cmd_node* kdk_package_cmd_init(const char *cmd)
{
    if(!cmd)
    {
        return NULL;
    }
    kdk_cmd_node *cmd_node = NULL;
    cmd_node = (kdk_cmd_node *)calloc(1, sizeof(kdk_cmd_node));
    if (cmd_node == NULL) 
    {
        return NULL;
    }
    strncpy(cmd_node->cmd, cmd, sizeof(cmd_node->cmd) - 1); 
    cmd_node->finished = NULL;
    cmd_node->update = NULL;
    cmd_node->stat = -1;
    return cmd_node;
}

static pthread_t thread_cmd = 0;
static volatile int running_cmd = 0;
FinishedCallBack _finished = NULL;
UpdateCallBack _update = NULL;

static void dbus_debian_cmd_listener_thread(void *arg)
{
    FILE *fp = (FILE *)arg;
    char cmd_line[512] = "\0";

    char *home_dir = kdk_get_home_dir();
    char log_path[512] = "\0";
    snprintf(log_path, sizeof(log_path), "%s/.log/kysdk_redirect.log", home_dir);
    syslog(LOG_DEBUG, "log_path = %s\n", log_path);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("open");
        return;
    }
    while (fgets(cmd_line, 512, fp))
    {
        // 写入内容
        if (write(fd, cmd_line, strnlen(cmd_line, STRNLEN_MAX)) < 0) {
            perror("write");
        }

        strskipspace(cmd_line);
        _update(cmd_line);
    }
    pclose(fp);
    close(fd);

    fp = fopen(log_path, "r");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (file_size == -1) {
        fclose(fp);
        return;
    }

    rewind(fp);
    char *content = (char *)malloc(file_size + 1);
    if (!content) {
        perror("Failed to allocate memory");
        fclose(fp);
        return;
    }

    size_t read_size = fread(content, 1, file_size, fp);
    if (read_size != file_size) {
        free(content);
        fclose(fp);
        return;
    }

    content[read_size] = '\0';  // Ensure null-termination
    syslog(LOG_DEBUG, "File content: %s", content);
    _finished(0, content, NULL);
    free(content);
    fclose(fp);
    }
}

static void dbus_kare_cmd_listener_thread(void *arg)
{
    char *package_pid = (char *)arg;
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    char *standard_err = NULL;
    char *pkg_pid = NULL;
    char *standard_log = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return ;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return ;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/dockerAgent',interface='com.kylin.dockerAgent',member='kareStandardLogUpdate'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/dockerAgent',interface='com.kylin.dockerAgent',member='kareErrorLogUpdate'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/dockerAgent',interface='com.kylin.dockerAgent',member='kareLogFinished'",
                    &error);
    // 循环监听DBus消息
    while (true)
    {
        if (running_cmd)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "com.kylin.dockerAgent", "kareStandardLogUpdate")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_log, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_log = %s", package_pid, pkg_pid, standard_log);
                    
                    _update(standard_log);
                }
            }

            if(dbus_message_is_signal(message, "com.kylin.dockerAgent", "kareErrorLogUpdate")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_err, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_err = %s", package_pid, pkg_pid, standard_err);
                    
                    _update(standard_err);
                }
            }
        
            if(dbus_message_is_signal(message, "com.kylin.dockerAgent", "kareLogFinished")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_log, DBUS_TYPE_STRING, &standard_err, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_log = %s, standard_err = %s", package_pid, pkg_pid, standard_log, standard_err);

                    if(standard_err && standard_log)
                    {
                        _finished(2, standard_log, standard_err);
                    }
                    else if(standard_log)
                    {
                        _finished(0, standard_log, NULL);
                    }
                    else if (standard_err)
                    {
                        _finished(1, NULL, standard_err);
                    }
                }
            }
            dbus_message_unref(message);
        }
    }
    dbus_connection_close(connection);
    pthread_exit(NULL);
}

int kdk_package_launch_cmd(kdk_cmd_node* node)
{
    if(!node)
    {
        return -1;
    }

    FILE *fp = NULL;
    char command[64] = "\0";
    sscanf(node->cmd, "%s %*s", command);
    syslog(LOG_DEBUG, "command = %s\n", command);
    char type_cmd[128] = "\0";
    char line[128] = "\0";
    char path_cmd[64] = "\0";
    char pkg_name[128] = "\0";
    char deb_cmd[64] = "\0";
    char resolved_path[256] = "\0";
    snprintf(type_cmd, sizeof(type_cmd), "bash -c 'type -a %s'", command);
    syslog(LOG_DEBUG, "type_cmd = %s\n", type_cmd);
    fp = popen(type_cmd, "r");
    if(fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            syslog(LOG_DEBUG, "line = %s\n", line);
            sscanf(line, "%s %*s %s", deb_cmd, path_cmd);
            syslog(LOG_DEBUG, "path_cmd = %s\n", path_cmd);

            if(strcmp(command, deb_cmd) == 0)
            {
                char dpkg_cmd[128] = "\0";
                snprintf(dpkg_cmd, sizeof(dpkg_cmd), "dpkg -S %s", path_cmd);
                syslog(LOG_DEBUG, "dpkg_cmd = %s\n", dpkg_cmd);
                FILE *path_fp = popen(dpkg_cmd, "r");
                if(path_fp)
                {
                    char pkg_line[256] = "\0";
                    fgets(pkg_line, sizeof(line), path_fp);
                    if(pkg_line[0] != "\0")
                    {
                        sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                        syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);

                        if(pkg_name[0] != 0 && strcmp(pkg_name, "dpkg-query") != 0)
                        {
                            pclose(path_fp);
                            break;
                        }
                        else
                        {
                            realpath(path_cmd, resolved_path);
                            if (0 == access(resolved_path, F_OK))
                            {
                                snprintf(dpkg_cmd, sizeof(dpkg_cmd), "dpkg -S %s", resolved_path);
                                syslog(LOG_DEBUG, "dpkg_cmd = %s\n", dpkg_cmd);
                                FILE *dpkg_fp = popen(dpkg_cmd, "r");
                                if(dpkg_fp)
                                {
                                    char pkg_line[256] = "\0";
                                    fgets(pkg_line, sizeof(line), dpkg_fp);
                                    if(pkg_line[0] != "\0")
                                    {
                                        sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                                        syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
                                        if(pkg_name[0] != 0 && strcmp(pkg_name, "dpkg-query") != 0)
                                        {
                                            pclose(dpkg_fp);
                                            break;
                                        }
                                    }
                                    pclose(dpkg_fp);
                                }
                            }
                        }
                    }
                    pclose(path_fp);
                }
            }
            else
            {
                break;
            }
        }
        pclose(fp);
    }

    if(pkg_name[0] != 0 && strcmp(pkg_name, "dpkg-query") != 0)
    {
        fp = popen(node->cmd, "r");
        if(fp)
        {
            node->stat = 1;
            _update = node->update;
            _finished = node->finished;
            pthread_create(&thread_cmd, NULL, dbus_debian_cmd_listener_thread, (void *) fp);
            return 0;
        }
    }
    else
    {
        syslog(LOG_DEBUG, "kare\n");
        char dpkg_cmd[128] = "\0";
        snprintf(dpkg_cmd, sizeof(dpkg_cmd), "kare -S %s", command);
        FILE *path_fp = popen(dpkg_cmd, "r");
        if(path_fp)
        {
            char pkg_line[256] = "\0";
            fgets(pkg_line, sizeof(line), path_fp);
            if(pkg_line[0] != "\0")
            {
                sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
                pclose(path_fp);
            }
            else
            {
                pclose(path_fp);
            }
        }
    }

    if(pkg_name[0] != 0 && strcmp(pkg_name, "kare-query") != 0)
    {
        char cmd_all[1024] = "\0";
        pid_t pid = getpid();

        char pkg_pid[256] = "\0";
        snprintf(pkg_pid, sizeof(pkg_pid), "%s_%d", pkg_name, pid);

        snprintf(cmd_all, sizeof(cmd_all), "kare -p %s run %s %s", pkg_pid, pkg_name, node->cmd);
        syslog(LOG_DEBUG, "cmd_all = %s\n", cmd_all);
        int ret = system(cmd_all);
        if(ret == 0)
        {
            node->stat = 0;
            _finished = node->finished;
            _update= node->update;
            running_cmd = 1;
            pthread_create(&thread_cmd, NULL, dbus_kare_cmd_listener_thread, (void*)pkg_pid);
            return 0;
        }
    }
    return -1;
}

void kdk_package_free_cmd(kdk_cmd_node* node)
{
    if(node)
    {
        free(node);
    }
}

void kdk_package_cmd_close(kdk_cmd_node* node)
{
    if(node)
    {
        if(node->stat == 1)
        {
            syslog(LOG_DEBUG, "node->cmd = %s\n", node->cmd);
            char line[115200] = "\0";
            char pid_cmd[512] = "\0";
            char pid[32] = "\0";
            char command[11520] = "\0";
            snprintf(pid_cmd, sizeof(pid_cmd), "ps -ef | grep \"%s\"", node->cmd);
            FILE *fp = popen(pid_cmd, "r");
            if(fp)
            {
                while (fgets(line, sizeof(line), fp))
                {
                    sscanf(line, "%*s %s %*s %*s %*s %*s %*s %11520[^\n]", pid, command);
                    syslog(LOG_DEBUG, "command = %s, pid = %s\n", command, pid);
                    if(strcmp(command, node->cmd) == 0)
                    {
                        break;
                    }
                    
                }
                pclose(fp);
            }
            char kill_cmd[512] = "\0";
            snprintf(kill_cmd, sizeof(kill_cmd), "kill -9 %s", pid);
            syslog(LOG_DEBUG, "kill_cmd = %s\n", kill_cmd);
            system(kill_cmd);
        }
        else
        {
            syslog(LOG_DEBUG, "kare\n");
            char command[64] = "\0";
            sscanf(node->cmd, "%s %*s", command);
            syslog(LOG_DEBUG, "command = %s\n", command);
            char pkg_name[128] = "\0";
            char dpkg_cmd[128] = "\0";
            char line[512] = "\0";
            snprintf(dpkg_cmd, sizeof(dpkg_cmd), "kare -S %s", command);
            FILE *path_fp = popen(dpkg_cmd, "r");
            if(path_fp)
            {
                char pkg_line[256] = "\0";
                fgets(pkg_line, sizeof(line), path_fp);
                if(pkg_line[0] != "\0")
                {
                    sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                    syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
                    pclose(path_fp);
                }
                else
                {
                    pclose(path_fp);
                }
            }

            if(pkg_name[0] != 0 && strcmp(pkg_name, "kare-query") != 0)
            {
                char cmd_all[1024] = "\0";
                pid_t pid = getpid();

                char pkg_pid[256] = "\0";
                snprintf(pkg_pid, sizeof(pkg_pid), "%s_%d", pkg_name, pid);

                DBusMessage *info_msg = NULL;
                DBusPendingCall *sendMsgPending = NULL;
                DBusMessage *replyMsg = NULL;
                DBusConnection *conn;
                DBusError err;
                dbus_error_init(&err);
                conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
                if (NULL == conn)
                {
                    syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                    return ;
                }

                if (dbus_error_is_set(&err))
                {
                    // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                    dbus_error_free(&err);
                }

                //创建用户
                info_msg = dbus_message_new_method_call("com.kylin.dockerAgent",  // target for the method call
                                                        "/com/kylin/dockerAgent", // object to call on
                                                        "com.kylin.dockerAgent",  // interface to call on
                                                        "killApplication");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    return ;
                }

                char *package_pid = strdup(pkg_pid);
                syslog(LOG_INFO, "package_pid : %s\n", package_pid);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &package_pid, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(package_pid);
                    return ;
                }
                free(package_pid);

                if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
                {
                    syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                    return ;
                }

                if (sendMsgPending == NULL)
                {
                    syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                    return ;
                }

                dbus_connection_flush(conn);

                dbus_pending_call_block(sendMsgPending);
                replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
                if (replyMsg == NULL)
                {
                    syslog(LOG_ERR, "kdk : get reply message fail !\n");
                    return ;
                }


                if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
                {
                    syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                    return ;
                }

                DBusMessageIter args;
                bool *ret = false;

                if (!dbus_message_iter_init(replyMsg, &args))
                {
                    syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                    return ;
                }
                else
                {
                    dbus_message_iter_get_basic(&args, &ret);
                }

                dbus_message_unref(info_msg);
                dbus_message_unref(replyMsg);
                dbus_pending_call_unref(sendMsgPending);
                dbus_connection_close(conn);
                dbus_connection_unref(conn);
                dbus_error_free(&err);
            }
        }
    }
}

char **kdk_package_get_standard_path(const char *name)
{
    if (!name) 
        return NULL;

    FILE *fp = fopen("/etc/ostree/compat-paths.conf", "r");
    if (!fp) 
        return NULL;

    char line[1024];
    char **res = NULL;
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char *eq_pos = strchr(line, '=');
        if (!eq_pos) 
            continue;

        *eq_pos = '\0'; // 分割键值
        char *current_key = line;
        char *current_value = eq_pos + 1;

        // 去除换行符
        current_value[strcspn(current_value, "\n")] = '\0';

        if (strcmp(name, current_key) != 0)
            continue;

        // 分割路径
        char *saveptr;
        char *token = strtok_r(current_value, ":", &saveptr);
        while (token) 
        {
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) 
                goto err_out;
            res = tmp;
            res[counts] = strdup(token);
            if (!res[counts]) 
                goto err_out;
            counts++;
            token = strtok_r(NULL, ":", &saveptr);
        }

        // 添加 NULL 结尾
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        res[counts] = NULL;
        fclose(fp);
        return res;
    }
    fclose(fp);
    return NULL;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

char **kdk_package_get_standard_path_lists()
{
    FILE *fp = fopen("/etc/ostree/compat-paths.conf", "r");
    if (!fp) 
        return NULL;

    char line[1024];
    char **res = NULL;
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char *eq_pos = strchr(line, '=');
        if (!eq_pos) 
            continue;

        *eq_pos = '\0'; // 分割键值
        char *current_key = line;
        current_key[strcspn(current_key, "\n")] = '\0';

        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        res[counts] = strdup(current_key);
        if (!res[counts]) 
            goto err_out;
        counts++;
    }
    // 添加 NULL 结尾
    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    if (!tmp) 
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    fclose(fp);
    return res;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_dpkg_list_files(const char *pkg_name)
{
    char **res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "dpkg -L %s", pkg_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024];
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        line[strcspn(line, "\n")] = '\0';
        res[counts] = strdup(line);
        if (!res[counts]) 
            goto err_out;
        counts++;
    }
     // 添加 NULL 结尾
    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    if (!tmp) 
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    fclose(fp);
    return res;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_kare_list_files(const char *pkg_name)
{
    char **res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "kare -L %s", pkg_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024];
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        line[strcspn(line, "\n")] = '\0';
        res[counts] = strdup(line);
        if (!res[counts]) 
            goto err_out;
        counts++;
    }
     // 添加 NULL 结尾
    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    if (!tmp) 
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    fclose(fp);
    return res;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_kaiming_list_files(const char *pkg_name)
{
    char **res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "/opt/kaiming-tools/bin/kaiming info -L %s", pkg_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024];
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        line[strcspn(line, "\n")] = '\0';
        res[counts] = strdup(line);
        if (!res[counts]) 
            goto err_out;
        counts++;
    }
     // 添加 NULL 结尾
    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    if (!tmp) 
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    fclose(fp);
    return res;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

char **kdk_package_list_files(const char *pkg_name)
{
    if(!pkg_name)
        return NULL;
    char **res = NULL;

    InstallMethod ret = kdk_package_get_installation_method(pkg_name, NULL);
    switch (ret)
    {
    case DPKG_METHOD:
        res = kdk_package_dpkg_list_files(pkg_name);
        break;
    case KM_METHOD:
        res = kdk_package_kaiming_list_files(pkg_name);
        break;
    case KARE_METHOD:
        res = kdk_package_kare_list_files(pkg_name);
        break;
    default:
        break;
    }
    
   return res;
}

static char* kdk_package_dpkg_search_by_file(const char *file_name)
{
    char *res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "dpkg -S %s 2>/dev/null", file_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024] = "\0";

    fgets(line, sizeof(line), fp);
    strstripspace(line);
    if(line[0] != '\0')
    {
        char *eq_pos = strchr(line, ':');
        if (!eq_pos)
        {
            pclose(fp);
            return NULL;
        }

        *eq_pos = '\0'; // 分割键值
        res = strdup(line);
    }
    pclose(fp);
    return res;
}

static char* kdk_package_kare_search_by_file(const char *file_name)
{
    char *res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "kare -S %s 2>/dev/null", file_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024];
    int counts = 0;

    fgets(line, sizeof(line), fp);
    strstripspace(line);
    if(line[0] != '\0')
    {
        char errinfo[256] = "\0";
        snprintf(errinfo, sizeof(errinfo), "没有找到与 *%s* 相匹配的路径", file_name);
        if(strstr(line, errinfo))
        {
            pclose(fp);
            return NULL;
        }
        char *eq_pos = strchr(line, ':');
        if (!eq_pos)
        {
            pclose(fp);
            return NULL;
        }

        *eq_pos = '\0'; // 分割键值
        res = strdup(line);
    }
    pclose(fp);
    return res;
}

static char* kdk_package_kaiming_search_by_file(const char *file_name)
{
    char *res = NULL;
    char command[256] = "\0";
    snprintf(command, sizeof(command), "/opt/kaiming-tools/bin/kaiming which %s 2>/dev/null", file_name);

    // 执行命令并读取输出
    FILE *fp = popen(command, "r");
    if (!fp) 
    {
        return NULL;
    }
    char line[1024];
    int counts = 0;

    fgets(line, sizeof(line), fp);
    strstripspace(line);
    if(line[0] != '\0')
    {
        char errinfo[256] = "\0";
        snprintf(errinfo, sizeof(errinfo), "No package contains the file: %s", file_name);
        if(strstr(line, errinfo))
        {
            pclose(fp);
            return NULL;
        }
        char *eq_pos = strchr(line, ':');
        if (!eq_pos)
        {
            pclose(fp);
            return NULL;
        }

        *eq_pos = '\0'; // 分割键值
        res = strdup(line);
    }
    pclose(fp);
    return res;
}

// 路径替换规则列表
static const kdk_path_remap_rule remap_rules[] = {
    {"/var/opt", "/opt"},
    {"/var/mnt", "/mnt"},
    {"/var/srv", "/srv"},
    {"/var/tmp", "/tmp"},
    {"/run/media", "/media"}
    // 可添加更多规则
};

// 替换路径前缀：in_path 是输入路径，out_path 是输出路径缓冲区，必须至少 MAX_PATH_LEN 大
// 返回值：0 成功，-1 无需替换或错误
int remap_path_prefix(const char *in_path, char *out_path, size_t out_size)
{
    if (!in_path || !out_path || out_size == 0)
        return -1;

    size_t rule_count = sizeof(remap_rules) / sizeof(remap_rules[0]);

    for (size_t i = 0; i < rule_count; ++i) {
        const char *real_path = remap_rules[i].kdk_real_path;
        const char *path = remap_rules[i].kdk_path;
        size_t len = strnlen(real_path, STRNLEN_MAX);

        // 匹配前缀，并保证 "/" 或结束符之后不误匹配
        if (strncmp(in_path, real_path, len) == 0 &&
            (in_path[len] == '/' || in_path[len] == '\0')) {

            if (snprintf(out_path, out_size, "%s%s", path, in_path + len) >= out_size)
                return -1;  // buffer too small

            return 0; // 替换成功
        }
    }

    // 没有任何替换规则匹配
    strncpy(out_path, in_path, out_size - 1);
    out_path[out_size - 1] = '\0';
    return -1;
}

char *kdk_package_search_by_file(const char *file_name)
{
    if(!file_name)
        return NULL;
    char *res = NULL;
    char new_path[MAX_PATH_LEN] = "\0";
    remap_path_prefix(file_name, new_path, sizeof(new_path));
    klog_info("new_path = %s\n", new_path);

    res = kdk_package_dpkg_search_by_file(new_path);
    if(res)
    {
        return res;
    }

    res = kdk_package_kare_search_by_file(new_path);
    if(res)
    {
        return res;
    }

    res = kdk_package_kaiming_search_by_file(new_path);
    if(res)
    {
        return res;
    }
    
    return NULL;
}

void kdk_package_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}