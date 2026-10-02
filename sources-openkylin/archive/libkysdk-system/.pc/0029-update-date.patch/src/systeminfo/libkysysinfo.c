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

#include "libkysysinfo.h"
#include <dbus-1.0/dbus/dbus.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <kysdk/kysdk-base/libkylog.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <X11/extensions/Xrandr.h>
#include <dirent.h>
#include <cjson/cJSON.h>
#include <locale.h>
#include <libintl.h>
#include <utmp.h>
#include <time.h>
#include <utime.h>
#include "pci/pci.h"
#include "sys/sysinfo.h"
#include <pthread.h>
#include <glib.h>
#include <sys/stat.h>
#include <locale.h>
#include <regex.h>
#include <libkypackages.h>
#include <sys/wait.h>
#include <libgen.h>
#include <systemd/sd-bus.h>
#include <ctype.h>
#include <stdbool.h>
#include <syslog.h>

#define SAFE_FREE(x) \
    if (x)           \
    {                \
        free(x);     \
        x = NULL;    \
    }
#define KYLIN_ACTIVATION_DBUS_ADDRESS "org.freedesktop.activation"

#define SYSTEM_VERSION_PATH "/usr/lib/system-info/kylin-system-version.conf"
#define UPDATE_VERSION_PATH "/usr/share/kylin-update-desktop-config/data/"
#define UPDATE_VERSION_PATH_REPLACE "/usr/share/kylin-update-desktop-config/config/"
#define OS_RELEASE_PATH "/etc/os-release"
#define KYLIN_RELEASE_ID "KYLIN_RELEASE_ID"
#define MAX_LINE 1024

#define BUF_SIZE 1024

#define LOCALEDIR "/usr/share/locale/"
#define GETTEXT_PACKAGE "kysdk-sysinfo"
#define MAX_LINE_LENGTH 1024

#define CONF_PATHS_USR(n)    \
    "/etc/" n,               \
        "/run/" n,           \
        "/usr/local/lib/" n, \
        "/usr/lib/" n

#define CONF_PATHS(n) \
    CONF_PATHS_USR(n)


#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

struct device
{
    struct device *next;
    struct pci_dev *dev;
    /* Bus topology calculated by grow_tree() */
    struct device *bus_next;
    struct bus *parent_bus;
    struct bridge *bridge;
    /* Cache */
    unsigned int config_cached, config_bufsize;
    unsigned char *config;  /* Cached configuration space data */
    unsigned char *present; /* Maps which configuration bytes are present */
};

typedef ServcieVendorPreset PresetAction;

typedef struct
{
    char *pattern;
    PresetAction action;
    char **instances;
} PresetRule;

typedef struct
{
    PresetRule *rules;
    size_t n_rules;
} Presets;

static inline char *package_lookup(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strnlen(key, STRNLEN_MAX) + 1;
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static char *delete_trailing_chars(char *s, const char *bad)
{
    char *p, *c = s;

    /* Drops all specified bad characters, at the end of the string */

    if (!s)
        return NULL;

    if (!bad)
        bad = " \t\n\r";

    for (p = s; *p; p++)
        if (!strchr(bad, *p))
            c = p + 1;

    *c = 0;

    return s;
}

static char *first_word(const char *s, const char *word)
{
    size_t sl, wl;
    const char *p;

    /* Checks if the string starts with the specified word, either
     * followed by NUL or by whitespace. Returns a pointer to the
     * NUL or the first character after the whitespace. */

    sl = strnlen(s, STRNLEN_MAX);
    wl = strnlen(word, STRNLEN_MAX);

    if (sl < wl)
        return NULL;

    if (wl == 0)
        return (char *)s;

    if (memcmp(s, word, wl) != 0)
        return NULL;

    p = s + wl;

    if (*p == 0)
        return (char *)p;

    if (!strchr(" \t\n\r", *p))
        return NULL;

    p += strspn(p, " \t\n\r");
    return (char *)p;
}

static inline void *mfree(void *memory)
{
    free(memory);
    return NULL;
}

static char **strv_free(char **l)
{
    char **k;

    if (!l)
        return NULL;

    for (k = l; *k; k++)
        free(*k);

    return mfree(l);
}

static int strv_contians_str(char **strv, char *str)
{
    if (!strv || !str)
        return;

    for (char **p = strv; p && *p; p++)
    {
        if (strcmp(*p, str))
            return 1;
    }
    return 0;
}

static char *endswith(const char *s, const char *postfix)
{
    size_t sl, pl;

    if (!s && !postfix)
        return NULL;

    sl = strnlen(s, STRNLEN_MAX);
    pl = strnlen(postfix, STRNLEN_MAX);

    if (pl == 0)
        return (char *)s + sl;

    if (sl < pl)
        return NULL;

    if (memcmp(s + sl - pl, postfix, pl) != 0)
        return NULL;

    return (char *)s + sl - pl;
}

static int conf_files_list_strv(char ***strv, const char *suffix, const char *const *dirs)
{
    if (!strv)
        return -1;

    char **p = NULL, **files = NULL;
    for (p = dirs; p && *p; p++)
    {
        DIR *dir = NULL;
        struct dirent *de = NULL;
        dir = opendir(*p);
        if (!dir)
        {
            if (errno == ENOENT)
                continue;
            return errno;
        }
        int count = 0;
        for (errno = 0, de = readdir(dir);; errno = 0, de = readdir(dir))
        {
            if (!de)
            {
                if (errno > 0)
                {
                    closedir(dir);
                    return -errno;
                }
                break;
            }
            else
            {
                /* Does this match the suffix? */
                if (suffix && !endswith(de->d_name, suffix))
                    continue;
                /* Has this file already been found in an earlier directory? */
                if (strv_contians_str(files, de->d_name))
                    continue;
                char **files = realloc(*strv, (count + 2) * sizeof(char *));
                if (!files)
                {
                    strv_free(files);
                    *strv = NULL;
                    closedir(dir);
                    return -1;
                }
                *strv = files;
                char *path = (char *)calloc(strnlen(*p, STRNLEN_MAX) + strnlen(de->d_name, STRNLEN_MAX) + 1, sizeof(char));
                if (!path)
                {
                    strv_free(files);
                    *strv = NULL;
                    closedir(dir);
                    return -1;
                }
                snprintf(path, (strnlen(*p, STRNLEN_MAX) + strnlen(de->d_name, STRNLEN_MAX) + 1) * sizeof(char), "%s/%s", *p, de->d_name);
                files[count++] = path;
            }
        }
        closedir(dir);
    }
}

static int presets_find_config(char ***files)
{
    const char *const *dirs[] = {CONF_PATHS("systemd/system-preset"), NULL};
    return conf_files_list_strv(files, ".preset", dirs);
}

static int read_presets(Presets *presets)
{
    Presets ps = {};
    size_t n_allocated = 0;

    char **files = NULL;
    int r;

    r = presets_find_config(&files);
    if (r < 0)
        return r;

    for (char **p = files; p && *p; p++)
    {
        FILE *fp = NULL;
        fp = fopen(*p, "re");
        if (!fp)
        {
            if (errno == ENOENT)
                continue;
            return -errno;
        }
        char line[1024];
        for (;;)
        {
            PresetRule rule = {};
            const char *parameter;
            char *l;

            if (feof(fp))
                break;

            l = fgets(line, 1024, fp);

            if (NULL == l || 0 == strnlen(l, STRNLEN_MAX))
                continue;

            delete_trailing_chars(l, NULL);

            if (strchr("#;", *l))
                continue;

            parameter = first_word(l, "enable");
            if (parameter)
            {
                // char *unit_name;
                // char **instances = NULL;
                char *q;

                /* Unit_name will remain the same as parameter when no instances are specified */
                for (q = parameter; !strchr(" \t\r\n\0", *q); q++)
                {
                    ;
                }
                *q = '\0';
                rule = (PresetRule){
                    .pattern = strdup(parameter),
                    .action = K_PRESET_ENABLE,
                };
            }

            parameter = first_word(l, "disable");
            if (parameter)
            {
                char *pattern;

                pattern = strdup(parameter);
                if (!pattern)
                    return -ENOMEM;
                rule = (PresetRule){
                    .pattern = pattern,
                    .action = K_PRESET_DISABLE,
                };
                free(pattern);
            }
            if (rule.action)
            {
                PresetRule *rules = realloc(ps.rules, sizeof(PresetRule) * (ps.n_rules + 1));
                if (NULL == rules)
                    return -ENOMEM;
                ps.rules = rules;
                ps.rules[ps.n_rules++] = rule;
                continue;
            }
        }
    }
    *presets = ps;
    ps = (Presets){};

    return 0;
}

static int query_presets(kdk_service_list *list, const Presets presets)
{
    PresetAction action = K_PRESET_UNKNOWN;
    size_t i;
    for (kdk_service_list *tmp = list; tmp; tmp = tmp->next)
    {
        tmp->vendorpreset = K_PRESET_ENABLE;
        for (i = 0; i < presets.n_rules; i++)
        {
            if (0 == strcmp(presets.rules[i].pattern, tmp->name))
            {
                tmp->vendorpreset = presets.rules[i].action;
                break;
            }
        }
    }
    return 0;
}

static int unit_file_query_preset(kdk_service_list *list)
{
    Presets presets = {};
    int r;

    r = read_presets(&presets);
    if (r < 0)
        return r;

    return query_presets(list, presets);
}

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/proc", strnlen("/proc", STRNLEN_MAX));
}

static char *get_val_from_file(FILE *fp, const char *key)
{
    if (!fp)
        return NULL;
    char *val = NULL;
    char buf[1024] = {0};
    while (fgets(buf, 1024, fp))
    {
        if (strncmp(buf, key, strnlen(key, STRNLEN_MAX)) == 0)
        {
            val = strdup((char *)buf + strnlen(key, STRNLEN_MAX) + 1);
            break;
        }
    }

    return val;
}

static kdk_service_list *list_unit_files()
{
    kdk_service_list *list = NULL;

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
        return NULL;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.systemd1",
                                           "/org/freedesktop/systemd1",
                                           "org.freedesktop.systemd1.Manager",
                                           "ListUnitFiles");
    if (message == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        return NULL;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        return NULL;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        return NULL;
    }

    DBusMessageIter tuples;
    dbus_message_iter_recurse(&iter, &tuples);
    while (dbus_message_iter_get_arg_type(&tuples) != DBUS_TYPE_INVALID)
    {
        DBusMessageIter value;
        dbus_message_iter_recurse(&tuples, &value);
        const char *name, *state;
        dbus_message_iter_get_basic(&value, &name);
        dbus_message_iter_next(&value);
        dbus_message_iter_get_basic(&value, &state);
        dbus_message_iter_next(&tuples);
        if (0 != strendwith(name, ".service"))
            continue;

        kdk_service_list *tmp = calloc(1, sizeof *tmp);
        if (NULL == tmp)
        {
            kdk_system_free_service_list(list);
            return NULL;
        }
        tmp->next = list;
        list = tmp;

        strncpy(tmp->name, basename(name), sizeof(tmp->name) - 1);
        if (0 == strcmp(state, "enabled"))
            tmp->status = K_UNIT_FILE_ENABLED;
        else if (0 == strcmp(state, "enabled-runtime"))
            tmp->status = K_UNIT_FILE_ENABLED_RUNTIME;
        else if (0 == strcmp(state, "linked"))
            tmp->status = K_UNIT_FILE_LINKED;
        else if (0 == strcmp(state, "linked-runtime"))
            tmp->status = K_UNIT_FILE_LINKED_RUNTIME;
        else if (0 == strcmp(state, "masked"))
            tmp->status = K_UNIT_FILE_MASKED;
        else if (0 == strcmp(state, "masked-runtime"))
            tmp->status = K_UNIT_FILE_MASKED_RUNTIME;
        else if (0 == strcmp(state, "static"))
            tmp->status = K_UNIT_FILE_STATIC;
        else if (0 == strcmp(state, "disabled"))
            tmp->status = K_UNIT_FILE_DISABLED;
        else if (0 == strcmp(state, "indirect"))
            tmp->status = K_UNIT_FILE_INDIRECT;
        else if (0 == strcmp(state, "generated"))
            tmp->status = K_UNIT_FILE_GENERATED;
        else if (0 == strcmp(state, "transient"))
            tmp->status = K_UNIT_FILE_TRANSIENT;
        else if (0 == strcmp(state, "bad"))
            tmp->status = K_UNIT_FILE_BAD;
        else
            tmp->status = K__UNIT_FILE_STATE_INVALID;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    return list;
}

char *kdk_system_get_architecture()
{
    char *architecture = NULL;
#ifdef __linux__
    FILE *fp = fopen("/proc/osinfo", "r");
    if (!fp)
        return NULL;

    architecture = get_val_from_file(fp, "Architecture");
    if (!architecture)
    {
        fclose(fp);
        return NULL;
    }
    strstrip(architecture, '\n');
    strstrip(architecture, '\t');

    fclose(fp);
#endif
    return architecture;
}

char *kdk_system_get_systemName()
{
    char *sysname = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
        return NULL;

    sysname = get_val_from_file(fp, "NAME");
    if (!sysname)
        return NULL;
    strstrip(sysname, '\n');
    strstrip(sysname, '\"');

    fclose(fp);
#endif
    return sysname;
}

static char* file_contains(const char *filepath, const char *keyword)
{
    char *scene = (char *)malloc(sizeof(char) * 50);
    ASSERT_NOT_NULL(scene, NULL);
#ifdef __linux__
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        strncpy(scene, "none", 49);  // 文件不存在
        return scene;
    }
    char *res = get_val_from_file(fp, keyword);
    if (!res)
    {   
        fclose(fp);
        strncpy(scene, "none", 49);
        return scene;
    }
    fclose(fp);
    strstrip(res, '\n');
    strstrip(res, '\"');
    strstripspace(res);
    strncpy(scene, res, 49);
    free(res);
#endif
    return scene;
}

static char *_kdk_system_get_appScene()
{
    char *osinfo = NULL;
    char *license = NULL;
    osinfo = file_contains("/etc/.kylin-osinfo", "APP_SCENE");
    if(strcmp(osinfo, "none") != 0)
    {
        return osinfo;
    }
    license = file_contains("/etc/LICENSE", "APP_SCENE");
    if((strcmp(license, "none") != 0) && (strcmp(osinfo, "none") == 0))
    {
        free(osinfo);
        return license;
        return osinfo;
    }
}

char *kdk_system_get_version(bool verbose)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);
    char *sysversion = NULL;
    if (verbose)
    {
#ifdef __linux__
        FILE *fp = fopen("/etc/.kyinfo", "r");
        if (!fp)
            return NULL;
        sysversion = get_val_from_file(fp, "milestone");
        if (!sysversion)
        {
            fclose(fp);
            return NULL;
        }
        strstrip(sysversion, '\n');
        strstrip(sysversion, '\"');
        strstripspace(sysversion);
        fclose(fp);
    }
    else
    {
        FILE *fp = NULL;

        // 首先尝试读取 /etc/.kylin-osinfo
        fp = fopen("/etc/.kylin-osinfo", "r");
        if (fp)
        {
            char *lang = getenv("LANG");
            if (lang && strstr(lang, "zh"))
            {
                sysversion = get_val_from_file(fp, "VERSION");
                if(sysversion)
                {
                    strstrip(sysversion, '\"');
                    strstrip(sysversion, '\n');
                    strstrip(sysversion, '\"');
                    strstripspace(sysversion);
                }
            }
            else
            {
                sysversion = get_val_from_file(fp, "VERSION_US");
                if (sysversion)
                {
                    strstrip(sysversion, '\"');
                    strstrip(sysversion, '\n');
                    strstrip(sysversion, '\"');
                    strstripspace(sysversion);
                    if (strcmp(sysversion, "Kylin Linux Desktop V10 (SP1)") == 0)
                    {
                        char *tmp = strdup(gettext("Kylin Linux Desktop V10 (SP1)"));
                        free(sysversion);
                        sysversion = tmp;
                    }
                    else if (strcmp(sysversion, "Kylin Linux Desktop V10 SP1") == 0)
                    {
                        char *tmp = strdup(gettext("Kylin Linux Desktop V10 SP1"));
                        free(sysversion);
                        sysversion = tmp;
                    }
                    else if (strcmp(sysversion, "Kylin Linux Desktop EDU V10") == 0)
                    {
                        char *tmp = strdup(gettext("Kylin Linux Desktop EDU V10"));
                        free(sysversion);
                        sysversion = tmp;
                    }
                    else if (strcmp(sysversion, "Kylin Linux Desktop EDU V11") == 0)
                    {
                        char *tmp = strdup(gettext("Kylin Linux Desktop EDU V11"));
                        free(sysversion);
                        sysversion = tmp;
                    }
                    else if (strcmp(sysversion, "Kylin Linux Desktop V11") == 0)
                    {
                        char *tmp = strdup(gettext("Kylin Linux Desktop V11"));
                        free(sysversion);
                        sysversion = tmp;
                    }
                }
            }
            fclose(fp);

            if (sysversion)
            {
                return sysversion;
            }
        }

        // 如果 /etc/.kylin-osinfo 中没有找到，尝试 /etc/os-release
        fp = fopen("/etc/os-release", "r");
        if (!fp)
        {
            return NULL;
        }

        char *lang = getenv("LANG");
        if (lang && strstr(lang, "zh"))
        {
            sysversion = get_val_from_file(fp, "VERSION");
            if(sysversion)
            {
                strstrip(sysversion, '\"');
                strstrip(sysversion, '\n');
                strstrip(sysversion, '\"');
                strstripspace(sysversion);
            }
        }
        else
        {
            sysversion = get_val_from_file(fp, "VERSION_US");
            if (sysversion)
            {
                strstrip(sysversion, '\"');
                strstrip(sysversion, '\n');
                strstrip(sysversion, '\"');
                strstripspace(sysversion);
                if (strcmp(sysversion, "Kylin Linux Desktop V10 (SP1)") == 0)
                {
                    char *tmp = strdup(gettext("Kylin Linux Desktop V10 (SP1)"));
                    free(sysversion);
                    sysversion = tmp;
                }
                else if (strcmp(sysversion, "Kylin Linux Desktop V10 SP1") == 0)
                {
                    char *tmp = strdup(gettext("Kylin Linux Desktop V10 SP1"));
                    free(sysversion);
                    sysversion = tmp;
                }
                else if (strcmp(sysversion, "Kylin Linux Desktop EDU V10") == 0)
                {
                    char *tmp = strdup(gettext("Kylin Linux Desktop EDU V10"));
                    free(sysversion);
                    sysversion = tmp;
                }
                else if (strcmp(sysversion, "Kylin Linux Desktop EDU V11") == 0)
                {
                    char *tmp = strdup(gettext("Kylin Linux Desktop EDU V11"));
                    free(sysversion);
                    sysversion = tmp;
                }
                else if (strcmp(sysversion, "Kylin Linux Desktop V11") == 0)
                {
                    char *tmp = strdup(gettext("Kylin Linux Desktop V11"));
                    free(sysversion);
                    sysversion = tmp;
                }
            }
        }
        fclose(fp);
    }
#endif
    return sysversion;
    
}

int kdk_system_get_activationStatus(int *status_error_num, int *date_error_num)
{
    if (NULL == status_error_num || NULL == date_error_num)
    {
        klog_err("激活状态接口调用失败：参数错误\n");
        return -1;
    }

    int res = 0;

// #define _KYLIN_ACTIVATION_H_
#ifdef __linux__
#ifdef _KYLIN_ACTIVATION_H_
    int err;
    int ret = -1;

    typedef int (*kylin_activation_activate_status)(int *);
    typedef int (*kylin_activation_trial_status)(int *);
    void *hwnd = dlopen("/usr/lib/libkylin-activation.so", RTLD_LAZY);

    if (!hwnd)
    {
        klog_err("加载libkylin-activation.so失败\n");
        return ret;
    }

    do
    {
        kylin_activation_activate_status pkylin_activation_activate_status = (kylin_activation_activate_status)dlsym(hwnd, "kylin_activation_activate_status");
        kylin_activation_trial_status pkylin_activation_trial_status = (kylin_activation_trial_status)dlsym(hwnd, "kylin_activation_trial_status");

        if (!pkylin_activation_activate_status || !pkylin_activation_trial_status)
        {
            klog_err("获取接口地址失败\n");
            break;
        }
        res = pkylin_activation_activate_status(&err);
        if (err != 0)
        {
            klog_err("激活状态获取失败：%d\n", err);
            ret = 0;
            break;
        }
        if (res == 1)
        {
            ret = 1;
            break;
        }

        res = pkylin_activation_trial_status(&err);
        if (err != 0)
        {
            klog_err("试用状态获取失败：%d\n", err);
            ret = 0;
            break;
        }

        if (res == 1)
        {
            ret = 0;
            break;
        }
    } while (false);

    dlclose(hwnd);
    return ret;
#else  // 修改dbus通信
    DBusConnection *conn;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return -1;
    }

    DBusMessage *status_msg = NULL;
    DBusMessage *status_pending_msg = NULL;
    DBusMessage *date_msg = NULL;
    DBusMessage *date_pending_msg = NULL;
    DBusMessageIter args;
    DBusPendingCall *status_pending = NULL;
    DBusPendingCall *date_pending = NULL;
    int stat = -1;

    // 获取激活状态
    status_msg = dbus_message_new_method_call("org.freedesktop.activation",           // target for the method call
                                              "/org/freedesktop/activation",          // object to call on
                                              "org.freedesktop.activation.interface", // interface to call on
                                              "status");                              // method name

    if (!status_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return -1;
    }

    if (!dbus_connection_send_with_reply(conn, status_msg, &status_pending, -1))
    { // -1 is default timeout
        klog_err("status_msg：dbus_connection_send_with_reply调用失败\n");
        return -1;
    }
    if (NULL == status_pending)
    {
        klog_err("status_pending：返回值为null\n");
        return -1;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(status_msg);

    // 接收返回值
    dbus_pending_call_block(status_pending);
    status_pending_msg = dbus_pending_call_steal_reply(status_pending);
    if (NULL == status_pending_msg)
    {
        klog_err("status_pending_msg：获取激活状态接口返回值异常\n");
        return -1;
    }

    // 释放status_pending
    dbus_pending_call_unref(status_pending);

    if (!dbus_message_iter_init(status_pending_msg, &args))
    {
        klog_err("status_pending_msg：dbus_message_iter_init获取激活状态接口返回值异常\n");
        return -1;
    }
    else
        dbus_message_iter_get_basic(&args, &stat);

    klog_debug("激活状态：%d\n", stat);
    if (!dbus_message_iter_next(&args))
    {
        klog_err("status_pending_msg：dbus_message_iter_next获取激活状态接口返回值异常\n");
        return -1;
    }
    else
        dbus_message_iter_get_basic(&args, status_error_num);
    klog_debug("激活状态：%d，错误码：%d\n", stat, *status_error_num);

    // 释放status_pending_msg
    dbus_message_unref(status_pending_msg);

    if (stat == 0)
    {
        char *date_stat;

        // 获取技术服务日期
        date_msg = dbus_message_new_method_call("org.freedesktop.activation",           // target for the method call
                                                "/org/freedesktop/activation",          // object to call on
                                                "org.freedesktop.activation.interface", // interface to call on
                                                "date");                                // method name
        if (!dbus_connection_send_with_reply(conn, date_msg, &date_pending, -1))
        { // -1 is default timeout
            klog_err("date_msg：dbus_connection_send_with_reply 接口调用失败\n");
            return -1;
        }
        if (NULL == date_pending)
        {
            klog_err("date_pending：返回值为null\n");
            return -1;
        }
        dbus_connection_flush(conn);
        dbus_message_unref(date_msg);
        // 接收返回值
        dbus_pending_call_block(date_pending);
        date_pending_msg = dbus_pending_call_steal_reply(date_pending);

        if (NULL == date_pending_msg)
        {
            klog_err("date_pending_msg：Reply Null\n");
            return -1;
        }
        if (!dbus_message_iter_init(date_pending_msg, &args))
        {
            dbus_message_unref(date_pending_msg);
            klog_err("date_pending_msg：dbus_message_iter_init获取技术服务日期接口返回值异常\n");
            return -1;
        }
        else
            dbus_message_iter_get_basic(&args, &date_stat);
        if (!dbus_message_iter_next(&args))
        {
            dbus_message_unref(date_pending_msg);
            klog_err("date_pending_msg：dbus_message_iter_next获取技术服务日期接口返回值异常\n");
            return -1;
        }
        else
            dbus_message_iter_get_basic(&args, date_error_num);
        klog_debug("技术服务日期：%s，错误码：%d\n", date_stat, *date_error_num);
        if (*date_error_num == 0)
        {
            if (strcmp(date_stat, ""))
            {
                dbus_message_unref(date_pending_msg);
                return 2;
            }
            else
            {
                dbus_message_unref(date_pending_msg);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (stat == 1 && *status_error_num == 0)
    {
        if (conn)
        {
            dbus_connection_close(conn);
        }

        return 1;
    }

    // #endif  // _KYLIN_ACTIVATION_H_
    // #endif  // __linux__
    if (status_msg)
    {
        dbus_message_unref(status_msg);
    }
    if (status_pending_msg)
    {
        dbus_message_unref(status_pending_msg);
    }
    if (date_msg)
    {
        dbus_message_unref(date_msg);
    }
    if (date_pending_msg)
    {
        dbus_message_unref(date_pending_msg);
    }
    if (status_pending)
    {
        dbus_pending_call_unref(status_pending);
    }
    if (date_pending)
    {
        dbus_pending_call_unref(date_pending);
    }
    if (conn)
    {
        dbus_connection_unref(conn);
    }
    return 0;
#endif // _KYLIN_ACTIVATION_H_
#endif // __linux__
}

char *kdk_system_get_serialNumber()
{
    char *serial = NULL;
#ifdef __linux__
#ifndef _KYLIN_ACTIVATION_H_ // 修改成dbus通信
    FILE *fp = fopen("/etc/.kyinfo", "r");
    if (!fp)
        return NULL;

    serial = get_val_from_file(fp, "key");
    fclose(fp);
    if (serial)
        strskipspace(serial);
#else
    int err;

    typedef char *(*kylin_activation_get_serial_number)(int *);
    void *hwnd = dlopen("/usr/lib/libkylin-activation.so", RTLD_LAZY);

    if (!hwnd)
    {
        klog_err("加载libkylin-activation.so失败\n");
        return serial;
    }
    kylin_activation_get_serial_number pkylin_activation_get_serial_number = (kylin_activation_get_serial_number)dlsym(hwnd, "kylin_activation_get_serial_number");
    if (!pkylin_activation_get_serial_number)
    {
        klog_err("加载接口kylin_activation_get_serial_number失败\n");
        return serial;
    }
    serial = pkylin_activation_get_serial_number(&err);
    if (!serial)
    {
        klog_err("序列号获取失败：%d\n", err);
    }
    else
    {
        strskipspace(serial);
    }
    dlclose(hwnd);
    return serial;
#endif // _KYLIN_ACTIVATION_H_
#endif // __linux__
    return serial;
}

char *kdk_system_get_kernelVersion()
{
    char *kernver = NULL;
#ifdef __linux__
    FILE *fp = fopen("/proc/version", "r");
    if (!fp)
        return NULL;

    char buf[1024];
    if (fgets(buf, 1024, fp) == NULL)
    {
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    char *p = buf;
    size_t pos = strfirstof(p, ' ');
    p += pos + 1;
    pos = strfirstof(p, ' ');
    p += pos + 1;
    size_t endpos = strfirstof(p, ' ');
    kernver = strndup(p, endpos);
#endif

    return kernver;
}

char *kdk_system_get_eUser()
{
    char tty[64] = {0};
    char buf[1024] = {0};
    FILE *fp = NULL;
    char tty_rel[64] = {0};
    char euser[1024] = {0};

    fp = fopen("/sys/class/tty/tty0/active", "r");
    if (!fp)
    {
        syslog(LOG_ERR, "打开文件失败\n");
        return NULL;
    }

    if (fgets(tty_rel, sizeof(tty_rel), fp))
    {
        strstripspace(tty_rel); // 去掉换行
    }
    fclose(fp);

    if (tty_rel[0] == '\0')
    {
        syslog(LOG_ERR, "tty_rel is empty\n");
        return NULL;
    }

    fp = popen("who", "r");
    if (fp)
    {
        while (fgets(buf, sizeof(buf), fp))
        {
            sscanf(buf, "%1023s %63s", euser, tty);

            if (strcmp(tty, tty_rel) == 0)
            {
                syslog(LOG_INFO, "euser: %s\n", euser);
                break;
            }
        }
        pclose(fp);
    }
    syslog(LOG_INFO, "ret euser: %s\n", euser);
    return euser[0] != '\0' ? strdup(euser) : NULL;
}

char *kdk_system_get_eUser_login_time()
{
    dbus_uint64_t time_stamp = 0;
    char *login_time = NULL;

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

    char *interface = "org.freedesktop.login1.User";
    char *state = "State";
    char *property = "Timestamp";

    for (int i = 0; i < element_count; i++)
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
            dbus_message_iter_get_basic(&iter, &time_stamp);

            break;
        }
    }

    if (0 == time_stamp)
        goto out;

    time_t t1 = time_stamp / 1000000;
    struct tm *timeDate = localtime(&t1);

    login_time = calloc(32, sizeof(char));
    snprintf(login_time, 32 * sizeof(char), "%04d-%02d-%02d %02d:%02d:%02d", timeDate->tm_year + 1900, timeDate->tm_mon + 1,
            timeDate->tm_mday, timeDate->tm_hour, timeDate->tm_min, timeDate->tm_sec);

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
    return login_time;
}

char *kdk_system_get_projectName()
{
    char *project_codename = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        project_codename = get_val_from_file(fp, "PROJECT_CODENAME");
        fclose(fp);
    }

    if (!project_codename)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, NULL);
        project_codename = get_val_from_file(fp, "PROJECT_CODENAME");
        fclose(fp);
    }
    if (project_codename)
        strstripspace(project_codename);
#endif
    return project_codename;
}

char *kdk_system_get_projectSubName()
{
    char *project_subcodename = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        project_subcodename = get_val_from_file(fp, "SUB_PROJECT_CODENAME");
        fclose(fp);
    }

    if (!project_subcodename)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, NULL);
        project_subcodename = get_val_from_file(fp, "SUB_PROJECT_CODENAME");
        fclose(fp);
    }
    if (project_subcodename)
        strstripspace(project_subcodename);
#endif
    return project_subcodename;
}

unsigned int kdk_system_get_productFeatures()
{
    char *product_features = NULL;
    unsigned int res = 0;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        product_features = get_val_from_file(fp, "PRODUCT_FEATURES");
        fclose(fp);
    }

    if (!product_features)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, 0);
        product_features = get_val_from_file(fp, "PRODUCT_FEATURES");
        fclose(fp);
    }
    if (product_features)
    {
        strstripspace(product_features);
        res = atoi(product_features);
    }
#endif

    free(product_features);
    return res;
}

char *kdk_system_get_hostVirtType()
{
    char *virtType = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(virtType, NULL);
#ifdef __linux__
    FILE *pipeLine = popen("systemd-detect-virt", "r"); // 建立流管道
    if (!pipeLine)
    {
        free(virtType);
        return NULL;
    }
    if (fgets(virtType, 64 * sizeof(char), pipeLine) == NULL)
    {
        SAFE_FREE(virtType);
        pclose(pipeLine);
        return NULL;
    }
    pclose(pipeLine);
    strstripspace(virtType);
    if (strcmp(virtType, "microsoft") == 0)
        strncpy(virtType, "hyper-v", 64);
    else if (strcmp(virtType, "oracle") == 0)
        strncpy(virtType, "orcale virtualbox", 64);
#endif
    return virtType;
}

char *kdk_system_get_hostCloudPlatform()
{
    char *cloudplat = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(cloudplat, NULL);
#ifdef __linux__
    cloudplat[0] = 0;
    char buf[256] = {0};
    bool res_flag = false;
    char *type = NULL;

    if (!access("/usr/local/ctyun/clink/Mirror/Registry/Default", F_OK))
    {
        strncpy(cloudplat, "ctyun", 64);
        return cloudplat;
    }

    FILE *fp = fopen("/etc/hw-cloud.conf", "rt");
    if (fp)
    {
        type = get_val_from_file(fp, "platform");
        fclose(fp);
    }
    if (type)
    {
        strstripspace(type);
        free(cloudplat);
        return type;
    }

    if (geteuid() == 0) // root 用户，可以用dmidecode
    {
        FILE *pipeLine = popen("dmidecode -s chassis-manufacturer", "r");
        if (__glibc_likely(pipeLine != NULL))
        {
            fgets(buf, 255 * sizeof(char), pipeLine);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strncpy(cloudplat, "huawei", 64);
                res_flag = true;
            }
            pclose(pipeLine);
        }
        if (!res_flag)
        {
            pipeLine = popen("dmidecode -s chassis-asset-tag", "r");
            if (__glibc_likely(pipeLine != NULL))
            {
                fgets(buf, 255 * sizeof(char), pipeLine);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strncpy(cloudplat, "huawei", 64);
                }
                pclose(pipeLine);
            }
        }
    }
    else // 普通用户，只能读取文件
    {
        FILE *fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "rt");
        if (__glibc_likely(fp != NULL))
        {
            fgets(buf, 255 * sizeof(char), fp);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strncpy(cloudplat, "huawei", 64);
                res_flag = true;
            }
            fclose(fp);
        }
        if (!res_flag)
        {
            fp = fopen("/sys/devices/virtual/dmi/id/chassis_asset_tag", "r");
            if (__glibc_likely(fp != NULL))
            {
                fgets(buf, 255 * sizeof(char), fp);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strncpy(cloudplat, "huawei", 64);
                }
                fclose(fp);
            }
        }
    }

    if (strnlen(cloudplat, STRNLEN_MAX) == 0)
        strncpy(cloudplat, "none", 64);
#endif
    return cloudplat;
}

bool kdk_system_is_zyj(void)
{
    bool bool_value = false;
    char *project_codename = kdk_system_get_projectName();

    if (project_codename && (strstr(project_codename, "zyj") || strstr(project_codename, "ZYJ")))
        bool_value = TRUE;

    SAFE_FREE(project_codename);
    return bool_value;
}

static char *readReleaseFile()
{
    char buf[MAX_LINE];
    FILE *fp;
    int len;
    static char *os_version = NULL;
    char version[32] = "\0";

    if ((fp = fopen(OS_RELEASE_PATH, "r")) == NULL)
    {
        return os_version;
    }

    os_version = get_val_from_file(fp, "KYLIN_RELEASE_ID");
    if (!os_version)
    {
        fclose(fp);
        return NULL;
    }
    strstrip(os_version, '\"');
    strstripspace(os_version);
    fclose(fp);
    return os_version;
}

static char *parseJsonText(char *text)
{
    char *out;
    cJSON *json;
    cJSON *childJson = NULL;
    char *version;

    // 获取补丁版本
    json = cJSON_Parse(text);
    if (!json)
    {
        version = NULL;
    }
    else
    {
        childJson = cJSON_GetObjectItem(json, "version");
        if (!childJson)
        {
            version = NULL;
        }
        else
        {
            version = strdup(childJson->valuestring);
        }
        out = cJSON_Print(json);
        cJSON_Delete(childJson);
        free(out);
    }
    return version;
}

static char *readJsonFile(char *json_file)
{
    FILE *fp;
    long len;
    char *data;
    static char *update_version;

    fp = fopen(json_file, "rb");
    if(!fp)
        return NULL;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = (char *)malloc(len + 1);
    fread(data, 1, len, fp);
    fclose(fp);
    update_version = parseJsonText(data);
    free(data);
    return update_version;
}

/** 函数名：      GetIniKeyString
*** 入口参数：    section :配置文件中一组数据的标识
***              key :这组数据中要读出的值的标识
***             filename :要读取的文件路径
*** 返回值：      找到需要查的值则返回正确结果 0
***              否则返回-1     */
static int GetIniKeyString(char *section, char *key, char *filename, char *buf)
{
    FILE *fp;
    int flag = 0;
    char sTitle[64], *wTmp;
    char sLine[1024];
    snprintf(sTitle, sizeof(sTitle), "[%s]", section);

    if (NULL == (fp = fopen(filename, "r")))
    {
        klog_err("fopen failed \n");
        return -1;
    }
    while (NULL != fgets(sLine, 1024, fp))
    {
        // 这是注释行
        if (0 == strncmp("//", sLine, 2))
            continue;
        if ('#' == sLine[0])
            continue;
        wTmp = strchr(sLine, '=');
        if ((NULL != wTmp) && (1 == flag))
        {
            if (0 == strncmp(key, sLine, strnlen(key, STRNLEN_MAX)))
            { // 长度依文件读取的为准
                sLine[strnlen(sLine, STRNLEN_MAX) - 1] = '\0';
                fclose(fp);
                while (*(wTmp + 1) == ' ')
                {
                    wTmp++;
                }
                strncpy(buf, wTmp + 1, 100);
                return 0;
            }
        }
        else
        {
            if (0 == strncmp(sTitle, sLine, strnlen(sTitle, STRNLEN_MAX)))
            {             // 长度依文件读取的为准
                flag = 1; // 找到标题位置
            }
        }
    }
    fclose(fp);
    return -1;
}

// /** 函数名:         PutIniKeyString
// *** 入口参数:        section:配置文件中一组数据的标识
// ***                key:这组数据中要读出的值的标识
// ***                val:更改后的值
// ***                filename:要读取的文件路径
// *** 返回值:         成功返回  0
// ***                否则返回 -1      */
// int PutIniKeyString(char *section, char *key, char *val, char *filename)
// {
//     FILE *fpr, *fpw;
//     int flag = 0;
//     char sLine[1024], sTitle[32], *wTmp;
//     sprintf(sTitle, "[%s]", section);
//     if (NULL == (fpr = fopen(filename, "r")))
//         return -1; // 读取原文件
//     sprintf(sLine, "%s.tmp", filename);
//     if (NULL == (fpw = fopen(sLine, "w")))
//     {
//         fclose(fpr);
//         return -1; // 写入临时文件
//     }
//     while (NULL != fgets(sLine, 1024, fpr))
//     {
//         if (2 != flag)
//         { // 如果找到要修改的那一行，则不会执行内部的操作
//             wTmp = strchr(sLine, '=');
//             if ((NULL != wTmp) && (1 == flag))
//             {
//                 if (0 == strncmp(key, sLine, strlen(key)))
//                 {             // 长度依文件读取的为准
//                     flag = 2; // 更改值，方便写入文件
//                     sprintf(wTmp + 1, " %s\n", val);
//                 }
//             }
//             else
//             {
//                 if (0 == strncmp(sTitle, sLine, strlen(sTitle)))
//                 {             // 长度依文件读取的为准
//                     flag = 1; // 找到标题位置
//                 }
//             }
//         }
//         fputs(sLine, fpw); // 写入临时文件
//     }
//     fclose(fpr);
//     fclose(fpw);
//     sprintf(sLine, "%s.tmp", filename);
//     return rename(sLine, filename); // 将临时文件更新到原文件
// }

static char* version_info_trim(char *str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    
    char *end = str + strnlen(str, STRNLEN_MAX) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static bool getOsVersion(version_t *ver) {
    ver->os_version = NULL;
    ver->update_version = NULL;

    FILE *file = fopen(SYSTEM_VERSION_PATH, "r");
    if (!file) {
        klog_err("Failed to open file\n");
        return FALSE;
    }

    struct stat file_stat;
    if (lstat(SYSTEM_VERSION_PATH, &file_stat) == -1) {
        klog_err("lstat failed\n");
        return FALSE;
    }
    if (!S_ISREG(file_stat.st_mode)) {
        klog_err("Not a regular file\n");
        return FALSE;
    }
    if (file_stat.st_size == 0) {
        klog_err("File is empty\n");
        return FALSE;
    } else {
        klog_info("File is not empty (size: %lld bytes)\n", (long long)file_stat.st_size);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *contents = malloc(length + 1);
    if (!contents) {
        fclose(file);
        klog_err("Memory allocation failed\n");
        return FALSE;
    }
    
    size_t read = fread(contents, 1, length, file);
    if (read != (size_t)length) {
        free(contents);
        fclose(file);
        klog_err("File read error\n");
        return FALSE;
    }
    contents[length] = '\0'; 
    fclose(file);

    char *line = strtok(contents, "\n");
    while (line != NULL) {
        if (strnlen(line, STRNLEN_MAX) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }

        char *sep = strchr(line, '=');
        if (!sep) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        *sep = '\0'; 
        char *key = version_info_trim(line);
        char *value = version_info_trim(sep + 1);

        if (strcmp(key, "os_version") == 0) {
            ver->os_version = strdup(value);
        } else if (strcmp(key, "update_version") == 0) {
            ver->update_version = strdup(value);
        }
        line = strtok(NULL, "\n");
    }

    free(contents);

    if (!ver->os_version || !ver->update_version) {
        klog_info("The required version information was not found\n");
    }

    return TRUE;
}

version_t kdk_system_get_version_detaile()
{
    version_t version = {0};

    // get_os_version
    if(!getOsVersion(&version)) {
        return (version_t){0};
    }

    if (!strcmp(version.os_version, "") && strcmp(version.update_version, "")) {
        version.os_version = strdup(version.update_version);
    }
    else if (!strcmp(version.update_version, "") && strcmp(version.os_version, "")) {
        version.update_version = strdup(version.os_version);
    }
    else if (!strcmp(version.os_version, "") && !strcmp(version.update_version, "")) {
        version.os_version = strdup("");
        version.update_version = strdup("");
    }

    strstrip(version.os_version, '"');
    strstrip(version.update_version, '"');

    return version;
}

char **kdk_system_get_resolving_power()
{
    char **res = NULL;
    char **tmpres = NULL;
    int count = 0;
    char tmp[1024] = "\0";
    
    if (getenv("WAYLAND_DISPLAY")) {
        char resolv[32] = "\0";
        char resolv_all[512] = "\0";
        char resolv_temp[512] = "\0";
        char resolv_name[128] = "\0";
        FILE *fp = popen("kscreen-doctor --json", "r");
        if(!fp)
        {
            return NULL;
        }
        char *data = (char *)malloc(sizeof(char) * 230400);
        fread(data, 1, 230400, fp);
        fclose(fp);

        cJSON *json = cJSON_Parse(data);
        if (json == NULL) {
            free(data);
            return NULL;
        }
        // 获取 outputs 数组
        cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
        if (outputs == NULL || !cJSON_IsArray(outputs)) {
            cJSON_Delete(json);
            free(data);
            return NULL;
        }

        // 遍历 outputs 数组
        cJSON *output, *currentModeId;
        cJSON_ArrayForEach(output, outputs) {
            // 获取当前对象的 currentModeId
            currentModeId = cJSON_GetObjectItem(output, "currentModeId");
            if (strnlen(currentModeId->valuestring, STRNLEN_MAX) != 0) {
                 cJSON *modes = cJSON_GetObjectItem(output, "modes");
                if (modes == NULL || !cJSON_IsArray(modes)) {
                    cJSON_Delete(json);
                    free(data);
                    return NULL;
                }

                cJSON *name = cJSON_GetObjectItem(output, "name");
                if (name == NULL || !cJSON_IsString(name)) {
                    cJSON_Delete(json);
                    free(data);
                    return NULL;
                }

                if(strstr(name->valuestring, "未知"))
                {
                    char tmp[128] = "\0";
                    sscanf(name->valuestring, "%*[^a-zA-Z] %[a-zA-Z-0-9]", tmp);
                    int length = strnlen(tmp, STRNLEN_MAX);
                    int i = length - 1;

                    while (i >= 0 && tmp[i] == '-') {
                        i--;
                    }

                    strncpy(resolv_name, tmp, sizeof(resolv_name) - 1);
                    resolv_name[sizeof(resolv_name) - 1] = '\0';
                }
                else{
                    strncpy(resolv_name, name->valuestring, sizeof(resolv_name) - 1);
                }

                // 遍历 modes 数组
                cJSON *mode;
                cJSON_ArrayForEach(mode, modes) {
                    // 获取当前 mode 对象的 id
                    cJSON *id = cJSON_GetObjectItem(mode, "id");
                    cJSON *size = cJSON_GetObjectItem(mode, "size");
                    // if (id != NULL && cJSON_IsString(id)) {
                    // 获取 size 对象中的 height 和 width
                    cJSON *height = cJSON_GetObjectItem(size, "height");
                    cJSON *width = cJSON_GetObjectItem(size, "width");

                    snprintf(resolv_all, sizeof(resolv_all), "%dx%d", width->valueint, height->valueint);
                    if(!strstr(resolv_temp, resolv_all))
                    {
                        strncat(resolv_temp, resolv_all, sizeof(resolv_temp) - strnlen(resolv_temp, STRNLEN_MAX) - 1);
                        strncat(resolv_temp, " ", sizeof(resolv_temp) - strnlen(resolv_temp, STRNLEN_MAX) - 1);
                    }

                    if(strcmp(id->valuestring, currentModeId->valuestring) == 0)
                    {
                        snprintf(resolv, sizeof(resolv), "%dx%d", width->valueint, height->valueint);
                    }
                }

                snprintf(tmp, sizeof(tmp), "%s,%s,%s", resolv_name, resolv, resolv_temp); 
                tmpres = realloc(res, (count + 2) * sizeof(char *));
                if (!tmpres)
                {
                    free(data);
                    klog_err("内存申请失败：%s\n", strerror(errno));
                    goto err_wayland_out;
                    
                }
                res = tmpres;
                res[count] = strdup(tmp);
                count++;
            }
        }

        cJSON_Delete(json);
        free(data);
        res[count] = NULL;
        goto out;

    err_wayland_out:
        cJSON_Delete(json);
        while (count)
        {
            free(res[count - 1]);
            count--;
        }
        SAFE_FREE(res);
        return NULL;
    } 
    else if (getenv("DISPLAY")) {
    
        Display *disp = NULL;
        XRRScreenResources *screen = NULL;
        XRROutputInfo *info;
        XRRCrtcInfo *crtc_info;
        int iscres;
        int icrtc;
        char temp[512] = {0};

        disp = XOpenDisplay(0);
        screen = XRRGetScreenResources (disp, DefaultRootWindow(disp));
        for (iscres = screen->noutput; iscres > 0;)
        {
            --iscres;

            info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
            if (info->connection == RR_Connected)
            {
                memset(tmp, 0, sizeof(tmp));
                snprintf(tmp, sizeof(tmp), "%s,", info->name);

                crtc_info = XRRGetCrtcInfo(disp, screen, info->crtc);
                if (crtc_info->width != 0 || crtc_info->height != 0)
                {
                    snprintf(tmp, sizeof(tmp), "%s%dx%d,", tmp, crtc_info->width, crtc_info->height);

                    for (int m = 0; m < screen->nmode; m++)
                    {
                        XRRModeInfo *mode = &screen->modes[m];
                        if (!strstr(temp, mode->name))
                        {
                            strncat(temp, mode->name, sizeof(temp) - strnlen(temp, STRNLEN_MAX) - 1);
                            strncat(temp, " ", sizeof(temp) - strnlen(temp, STRNLEN_MAX) - 1);
                        }
                    }
                    strncat(tmp, temp, sizeof(tmp) - strnlen(tmp, STRNLEN_MAX) - 1);

                    tmpres = realloc(res, (count + 2) * sizeof(char *));
                    if (!tmpres)
                    {
                        klog_err("内存申请失败：%s\n", strerror(errno));
                        XRRFreeCrtcInfo(crtc_info);
                        XRRFreeOutputInfo(info);
                        goto err_out;
                    }
                    res = tmpres;
                    res[count] = strdup(tmp);
                    count++;
                }
                XRRFreeCrtcInfo(crtc_info);
            }
            XRRFreeOutputInfo(info);
        }

        XRRFreeScreenResources(screen);
        XCloseDisplay(disp);
        res[count] = NULL;
        goto out;

    err_out:
        while (count)
        {
            free(res[count - 1]);
            count--;
        }
        SAFE_FREE(res);
        if (screen)
            XRRFreeScreenResources(screen);
        if (disp)
            XCloseDisplay(disp);
        return NULL;
    
    }
out:
    return res;
}

char *kdk_system_get_systemCategory()
{
    char *vername = (char *)malloc(sizeof(char) * 50);
#ifdef __linux__
    FILE *fp = fopen("/etc/LICENSE", "r");
    if (!fp)
    {
        strncpy(vername, "none", 49);
        return vername;
    }

    char *tmp = get_val_from_file(fp, "HW_TYPE");
    if (!tmp)
    {
        fclose(fp);
        strncpy(vername, "none", 49);
        return vername;
    }
    strstrip(tmp, '\n');
    strstrip(tmp, '\"');
    strncpy(vername, tmp, 49);
    free(tmp);
    fclose(fp);
#endif
    return vername;
}

char **kdk_system_get_startup_time()
{
    struct utmp *utmp_buf;
    time_t boot_time, rawtime;
    struct tm *info, *res;
    char *result = (char *)malloc(128 * sizeof(char));
    char *tmpe = (char *)malloc(20 * sizeof(char));
    if (!tmpe || !result)
    {
        goto err_out;
    }
    char **ret = NULL;
    char **tt = realloc(ret, 520 * sizeof(char *));
    if (!tt)
    {
        goto err_out;
    }
    ret = tt;
    int count = 0;

    boot_time = 0;
    time(&rawtime);
    res = localtime(&rawtime);
    int l_year = res->tm_year;
    int l_mon = res->tm_mon;
    int l_day = res->tm_mday;
    utmpname(WTMP_FILE);

    setutent();

    while ((utmp_buf = getutent()) != NULL)
    {
        if (utmp_buf->ut_type == BOOT_TIME)
        {
            boot_time = utmp_buf->ut_time;
            info = localtime(&boot_time);

            if ((l_year == info->tm_year) && (l_day == info->tm_mday) && (l_mon == info->tm_mon))
            {
                snprintf(tmpe, 20 * sizeof(char), "%d/%d/%d,%d:%d:%d", info->tm_mon + 1, info->tm_mday,
                        info->tm_year + 1900, info->tm_hour, info->tm_min, info->tm_sec);

                strncpy(result, tmpe, 127);

                ret[count] = malloc(256 * sizeof(char) + 1);
                if (!ret[count])
                {
                    free(tt);
                    goto err_out;
                }
                strncpy(ret[count], result, 256);
                count++;
            }
        }
    }
    ret[count] = NULL;
    free(tmpe);
    free(result);
    endutent();
    return ret;
err_out:
    if (result)
        free(result);
    if (tmpe)
        free(tmpe);
    return NULL;
}

char **kdk_system_get_shutdown_time()
{
    time_t rawtime;
    struct utmp *u;
    struct tm *info, *res;
    char *result = (char *)malloc(128 * sizeof(char));
    char *tmpe = (char *)malloc(20 * sizeof(char));
    if (!tmpe || !result)
    {
        goto err_out;
    }
    char **ret = NULL;
    char **tt = realloc(ret, 520 * sizeof(char *));
    if (!tt)
    {
        goto err_out;
    }
    ret = tt;
    int count = 0;

    time(&rawtime);
    res = localtime(&rawtime);
    int l_year = res->tm_year;
    int l_mon = res->tm_mon;
    int l_day = res->tm_mday;

    utmpname("/var/log/wtmp");
    while ((u = getutent()))
    {
        if ((strncmp(u->ut_user, "shutdown", 8) == 0))
        {
#if __WORDSIZE_TIME64_COMPAT32
            time_t tmp = (time_t)(u->ut_tv.tv_sec);
            info = localtime(&tmp);
#else
                info = localtime((time_t *)(&u->ut_tv.tv_sec));
#endif
            if ((l_year == info->tm_year) && (l_day == info->tm_mday) && (l_mon == info->tm_mon))
            {
                snprintf(tmpe, 20 * sizeof(char), "%d/%d/%d,%d:%d:%d", info->tm_mon + 1, info->tm_mday,
                        info->tm_year + 1900, info->tm_hour, info->tm_min, info->tm_sec);

                strncpy(result, tmpe, 127);
                ret[count] = malloc(256 * sizeof(char) + 1);
                if (!ret[count])
                {
                    free(tt);
                    goto err_out;
                }
                strncpy(ret[count], result, 256);
                count++;
            }
        }
    }
    ret[count] = NULL;
    free(tmpe);
    free(result);
    endutent();
    return ret;

err_out:
    if (result)
        free(result);
    if (tmpe)
        free(tmpe);
    return NULL;
}

char *kdk_get_host_vendor()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *vendor = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/sys_vendor", "r");
    if (fp == NULL)
    {
        free(vendor);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strncpy(vendor, line, 63);
        strstripspace(vendor);
    }
    return vendor;
}

char *kdk_get_host_product()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *product = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/product_name", "r");
    if (fp == NULL)
    {
        free(product);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strncpy(product, line, 63);
        strstripspace(product);
    }
    return product;
}

char *_kdk_get_host_serial()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *serial = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/product_serial", "r");
    if (fp == NULL)
    {
        free(serial);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strncpy(serial, line, 63);
        strstripspace(serial);
    }
    return serial;
}

char *kdk_get_host_serial()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
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

    // 创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/sysinfo", // object to call on
                                            "com.kylin.kysdk.sysinfo",  // interface to call on
                                            "getHostSerial");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
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
        ret = NULL;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
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
        ret = NULL;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
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
out:
    if (ret == NULL)
    {
        ret = _kdk_get_host_serial();
    }

    char *serial = NULL;
    if (ret != NULL)
    {
        serial = (char *)malloc(sizeof(char) * 64);
        if (!serial)
        {
            serial = NULL;
        }
        strncpy(serial, ret, 63);
    }

    return serial;
}

char *kdk_system_get_hostName()
{
    char *hostName = NULL;
    FILE *fp = fopen("/etc/hostname", "r");
    if (!fp)
        return NULL;

    hostName = (char *)calloc(512, sizeof(char));
    if (hostName)
        fread(hostName, sizeof(char), 512, fp);
    fclose(fp);
    strstrip(hostName, '\n');
    strstrip(hostName, '\t');
    return hostName;
}

int kdk_system_get_word()
{
#if __WORDSIZE == 64
    return 64;
#else
        return 32;
#endif
}

char *kdk_system_get_buildTime()
{
    char *buildTime = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/kylin-build", "r");
    if (!fp)
        return NULL;

    buildTime = get_val_from_file(fp, "Build");
    if (!buildTime)
        return NULL;
    strstrip(buildTime, '\n');
    strstrip(buildTime, '\"');

    fclose(fp);
#endif
    return buildTime;
}

inline void kdk_resolving_freeall(char **list)
{
    if (!list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index++;
    }
    free(list);
}

#define NONRET __attribute__((noreturn))
static void NONRET
die(char *msg, ...)
{
    va_list args;

    char errinfo[1024] = "\0";

    va_start(args, msg);
    klog_err("%s: ", "setpci");
    vsprintf(errinfo, msg, args);
    klog_err("%s\n", errinfo);
    exit(1);
}

static void
check_conf_range(struct device *d, unsigned int pos, unsigned int len)
{
    while (len)
        if (!d->present[pos])
            die("Internal bug: Accessing non-read configuration byte at position %x", pos);
        else
            pos++, len--;
}

static unsigned char get_conf_byte(struct device *d, unsigned int pos)
{
    check_conf_range(d, pos, 1);
    return d->config[pos];
}

static u16 get_conf_word(struct device *d, unsigned int pos)
{
    check_conf_range(d, pos, 2);
    return d->config[pos] | (d->config[pos + 1] << 8);
}

static void get_subid(struct device *d, u16 *subvp, u16 *subdp)
{
    u8 htype = get_conf_byte(d, PCI_HEADER_TYPE) & 0x7f;

    if (htype == PCI_HEADER_TYPE_NORMAL)
    {
        *subvp = get_conf_word(d, PCI_SUBSYSTEM_VENDOR_ID);
        *subdp = get_conf_word(d, PCI_SUBSYSTEM_ID);
    }
    else if (htype == PCI_HEADER_TYPE_CARDBUS && d->config_cached >= 128)
    {
        *subvp = get_conf_word(d, PCI_CB_SUBSYSTEM_VENDOR_ID);
        *subdp = get_conf_word(d, PCI_CB_SUBSYSTEM_ID);
    }
    else
        *subvp = *subdp = 0xffff;
}

static void *
xmalloc(size_t howmuch)
{
    void *p = malloc(howmuch);
    if (!p)
        die("Unable to allocate %d bytes of memory", (int)howmuch);
    return p;
}

static void *
xrealloc(void *ptr, size_t howmuch)
{
    void *p = realloc(ptr, howmuch);
    if (!p)
        die("Unable to allocate %d bytes of memory", (int)howmuch);
    return p;
}

// char *
// xstrdup(const char *str)
// {
//     int len = strlen(str) + 1;
//     char *copy = xmalloc(len);
//     memcpy(copy, str, len);
//     return copy;
// }

static int config_fetch(struct device *d, unsigned int pos, unsigned int len)
{
    unsigned int end = pos + len;
    int result;

    while (pos < d->config_bufsize && len && d->present[pos])
        pos++, len--;
    while (pos + len <= d->config_bufsize && len && d->present[pos + len - 1])
        len--;
    if (!len)
        return 1;

    if (end > d->config_bufsize)
    {
        int orig_size = d->config_bufsize;
        while (end > d->config_bufsize)
            d->config_bufsize *= 2;
        d->config = xrealloc(d->config, d->config_bufsize);
        d->present = xrealloc(d->present, d->config_bufsize);
        memset(d->present + orig_size, 0, d->config_bufsize - orig_size);
    }
    result = pci_read_block(d->dev, pos, d->config + pos, len);
    if (result)
        memset(d->present + pos, 1, len);
    return result;
}

static struct device *
scan_device(struct pci_dev *p, struct pci_filter *filter)
{
    struct device *d;

    //  if (p->domain && !opt_domains)
    //    opt_domains = 1;
    if (!pci_filter_match(filter, p))
    {
        return NULL;
    }
    d = xmalloc(sizeof(struct device));
    memset(d, 0, sizeof(*d));
    d->dev = p;
    d->config_cached = d->config_bufsize = 64;
    d->config = xmalloc(64);
    d->present = xmalloc(64);
    memset(d->present, 1, 64);

    if (!pci_read_block(p, 0, d->config, 64))
    {
        klog_err("lspci: Unable to read the standard configuration space header of device %04x:%02x:%02x.%d\n",
                 p->domain, p->bus, p->dev, p->func);
        free(d);
        return NULL;
    }
    if ((d->config[PCI_HEADER_TYPE] & 0x7f) == PCI_HEADER_TYPE_CARDBUS)
    {
        /* For cardbus bridges, we need to fetch 64 bytes more to get the
         * full standard header... */
        if (config_fetch(d, 64, 64))
            d->config_cached += 64;
    }

    pci_setup_cache(p, d->config, d->config_cached);
    pci_fill_info(p, PCI_FILL_IDENT | PCI_FILL_CLASS);
    return d;
}

static void
scan_devices(struct pci_access *pacc, struct device **ret, struct pci_filter *filter)
{
    struct device *d, *first_dev = NULL;
    struct pci_dev *p;

    pci_scan_bus(pacc);
    for (p = pacc->devices; p; p = p->next)
    {
        if ((d = scan_device(p, filter)))
        {
            d->next = first_dev;
            first_dev = d;
        }
    }
    *ret = first_dev;
}

static int
compare_them(const void *A, const void *B)
{
    const struct pci_dev *a = (*(const struct device **)A)->dev;
    const struct pci_dev *b = (*(const struct device **)B)->dev;

    if (a->domain < b->domain)
        return -1;
    if (a->domain > b->domain)
        return 1;
    if (a->bus < b->bus)
        return -1;
    if (a->bus > b->bus)
        return 1;
    if (a->dev < b->dev)
        return -1;
    if (a->dev > b->dev)
        return 1;
    if (a->func < b->func)
        return -1;
    if (a->func > b->func)
        return 1;
    return 0;
}

static void
sort_them(struct device **devices)
{
    struct device **index = NULL, **h = NULL, **last_dev = NULL;
    int cnt;
    struct device *d = NULL;

    struct device *first_dev = *devices;

    // 计算个数
    cnt = 0;
    for (d = first_dev; d; d = d->next)
        cnt++;

    // 申请一个数组空间，存放所有元素
    //  h = index = alloca(sizeof(struct device *) * cnt);
    struct device *tmp[cnt];
    h = index = tmp;
    for (d = first_dev; d; d = d->next)
        *h++ = d;

    // 排序
    qsort(index, cnt, sizeof(struct device *), compare_them);

    // 重新组成链表
    last_dev = devices;
    h = index;
    while (cnt--)
    {
        *last_dev = *h;
        last_dev = &(*h)->next;
        h++;
    }
    *last_dev = NULL;
}

#ifdef PCI_OS_LINUX

#include <sys/utsname.h>

#ifdef PCI_USE_LIBKMOD

#include <libkmod.h>

static struct kmod_ctx *kmod_ctx;

static int
show_kernel_init(void)
{
    static int show_kernel_inited = -1;
    if (show_kernel_inited >= 0)
        return show_kernel_inited;

    kmod_ctx = kmod_new(NULL, NULL);
    if (!kmod_ctx)
    {
        klog_err("lspci: Unable to initialize libkmod context\n");
        goto failed;
    }

    int err;
    if ((err = kmod_load_resources(kmod_ctx)) < 0)
    {
        klog_err("lspci: Unable to load libkmod resources: error %d\n", err);
        goto failed;
    }

    show_kernel_inited = 1;
    return 1;

failed:
    show_kernel_inited = 0;
    return 0;
}

static void show_kernel_cleanup(void)
{
    if (kmod_ctx)
        kmod_unref(kmod_ctx);
}

static const char *next_module(struct device *d)
{
    static struct kmod_list *klist, *kcurrent;
    static struct kmod_module *kmodule;

    if (kmodule)
    {
        kmod_module_unref(kmodule);
        kmodule = NULL;
    }

    if (!klist)
    {
        pci_fill_info(d->dev, PCI_FILL_MODULE_ALIAS);
        if (!d->dev->module_alias)
            goto out;

        int err = kmod_module_new_from_lookup(kmod_ctx, d->dev->module_alias, &klist);
        if (err < 0)
        {
            fprintf(stderr, "lspci: libkmod lookup failed: error %d\n", err);
            goto out;
        }
        kcurrent = klist;
    }
    else
    {
        kcurrent = kmod_list_next(klist, kcurrent);
    }

    if (kcurrent)
    {
        kmodule = kmod_module_get_module(kcurrent);
        char *module = kmod_module_get_name(kmodule);
        return module;
    }
    kmod_module_unref_list(klist);
    klist = NULL;
out:
    return NULL;
}

#else // PCI_USE_LIBKMOD

struct pcimap_entry
{
    struct pcimap_entry *next;
    unsigned int vendor, device;
    unsigned int subvendor, subdevice;
    unsigned int class, class_mask;
    char module[1];
};

static struct pcimap_entry *pcimap_head;
static int
show_kernel_init(void)
{
    struct pcimap_entry *pcimap_head;
    static int tried_pcimap = 0;
    struct utsname uts;
    char *name, line[1024];
    FILE *f;

    if (tried_pcimap)
        return 1;
    tried_pcimap = 1;

    if (uname(&uts) < 0)
        die("uname() failed: %m");
    name = alloca(64 + strnlen(uts.release, STRNLEN_MAX));
    snprintf(name, 64 + strnlen(uts.release, STRNLEN_MAX), "/lib/modules/%s/modules.pcimap", uts.release);
    f = fopen(name, "r");
    if (!f)
        return 1;

    while (fgets(line, sizeof(line), f))
    {
        char *c = strchr(line, '\n');
        struct pcimap_entry *e;

        if (!c)
            die("Unterminated or too long line in %s", name);
        *c = 0;
        if (!line[0] || line[0] == '#')
            continue;

        c = line;
        while (*c && *c != ' ' && *c != '\t')
            c++;
        if (!*c)
            continue; /* FIXME: Emit warnings! */
        *c++ = 0;

        e = xmalloc(sizeof(*e) + strnlen(line, STRNLEN_MAX));
        if (sscanf(c, "%i%i%i%i%i%i",
                   &e->vendor, &e->device,
                   &e->subvendor, &e->subdevice,
                   &e->class, &e->class_mask) != 6)
            continue;
        e->next = pcimap_head;
        pcimap_head = e;
        strncpy(e->module, line, strnlen(line, STRNLEN_MAX) + 1);
        e->module[strnlen(line, STRNLEN_MAX)] = '\0';
    }
    fclose(f);

    return 1;
}

static int
match_pcimap(struct device *d, struct pcimap_entry *e)
{
    struct pci_dev *dev = d->dev;
    unsigned int class = get_conf_long(d, PCI_REVISION_ID) >> 8;
    u16 subv, subd;

#define MATCH(x, y) ((y) > 0xffff || (x) == (y))
    get_subid(d, &subv, &subd);
    return MATCH(dev->vendor_id, e->vendor) &&
           MATCH(dev->device_id, e->device) &&
           MATCH(subv, e->subvendor) &&
           MATCH(subd, e->subdevice) &&
           (class & e->class_mask) == e->class;
#undef MATCH
}

static const char *next_module(struct device *d)
{
    static struct pcimap_entry *current;

    if (!current)
        current = pcimap_head;
    else
        current = current->next;

    while (current)
    {
        if (match_pcimap(d, current))
            return current->module;
        current = current->next;
    }

    return NULL;
}

static void show_kernel_cleanup()
{
    struct pcimap_entry *tmp = NULL;
    if (pcimap_head)
    {
        tmp = pcimap_head;
        pcimap_head = pcimap_head->next;
        free(tmp);
    }
}

#endif // PCI_USE_LIBKMOD

static char *
find_driver(struct device *d, char *buf)
{
    struct pci_dev *dev = d->dev;
    char name[1024], *drv, *base = NULL;
    int n;

    if (dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
        return NULL;

    struct pci_param *p;
    for (p = dev->access->params; p; p = p->next)
        if (!strcmp(p->param, "sysfs.path"))
            base = p->value;

    if (!base || !base[0])
        return NULL;

    n = snprintf(name, sizeof(name), "%s/devices/%04x:%02x:%02x.%d/driver",
                 base, dev->domain, dev->bus, dev->dev, dev->func);
    if (n < 0 || n >= (int)sizeof(name))
        die("show_driver: sysfs device name too long, why?");

    n = readlink(name, buf, 1024);
    if (n < 0)
        return NULL;
    if (n >= 1024)
        return "<name-too-long>";
    buf[n] = 0;

    if ((drv = strrchr(buf, '/')))
        return drv + 1;
    else
        return buf;
}

static const char *
next_module_filtered(struct device *d)
{
    static char prev_module[256];
    const char *module;

    while ((module = next_module(d)))
    {
        if (strcmp(module, prev_module))
        {
            strncpy(prev_module, module, sizeof(prev_module));
            prev_module[sizeof(prev_module) - 1] = 0;
            return module;
        }
    }
    prev_module[0] = 0;
    return NULL;
}
#endif // PCI_OS_LINUX

struct KPci *kdk_hw_get_pci_info()
{

    struct KPci *result = NULL, *node = NULL;

    struct pci_access *pacc = NULL;
    struct device *first_dev = NULL;
    struct pci_filter filter; /* Device filter */

    pacc = pci_alloc();
    pacc->error = die;
    pci_filter_init(pacc, &filter);

    pci_init(pacc);
    scan_devices(pacc, &first_dev, &filter);
    sort_them(&first_dev);

    struct device *d;

    for (d = first_dev; d; d = d->next)
    {
        if (!pci_filter_match(&filter, d->dev))
        {
            continue;
        }
        struct pci_dev *p = d->dev;

        node = (struct KPci *)calloc(1, sizeof *node);
        if (!node)
        {
            klog_err("Failed to request memory %02x:%02x.%d", p->bus, p->dev, p->func);
            kdk_hw_free_pci_info(result);
            result = NULL;
            goto out;
        }
        node->next = result;
        result = node;

        snprintf(node->slot_path, sizeof(node->slot_path), "%02x:%02x.%d", p->bus, p->dev, p->func);

        pci_lookup_name(pacc, node->class_name, sizeof(node->class_name),
                        PCI_LOOKUP_CLASS,
                        p->device_class);

        pci_lookup_name(pacc, node->product_name, sizeof(node->product_name),
                        PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
                        p->vendor_id, p->device_id);

        node->rev = get_conf_byte(d, PCI_REVISION_ID);

        // Subsystem Name
        u16 subsys_v, subsys_d;

        //    pci_fill_info(p, PCI_FILL_LABEL);
        //    if (p->label)
        //      printf("\tDeviceName: %s", p->label);

        get_subid(d, &subsys_v, &subsys_d);
        if (subsys_v && subsys_v != 0xffff)
        {
            char *pp = pci_lookup_name(pacc, node->ss_name, sizeof(node->ss_name),
                                       PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE | PCI_LOOKUP_SUBSYSTEM,
                                       p->vendor_id, p->device_id, subsys_v, subsys_d);
        }
        else
            strncpy(node->ss_name, "None", sizeof(node->ss_name) - 1);

        // 获取dirver
        const char *driver, *module;

        char buf[1024];
        strncpy(node->driver_use, (driver = find_driver(d, buf)) ? driver : "None", sizeof(node->driver_use) - 1);

        // 获取Kernel Modules
        if (!show_kernel_init()) // Kernel Init失败,modules返回空，进入下一轮循环
            continue;

        while ((module = next_module_filtered(d)))
        {
            // node->module_count++;

            char **tmp = NULL;
            tmp = realloc(node->modules, (node->module_count + 1) * sizeof(char *));
            if (!tmp)
            {
                klog_err("Failed to request memory modules");
                kdk_hw_free_pci_info(result);
                result = NULL;
                show_kernel_cleanup();
                goto out;
            }
            node->module_count++;
            node->modules = tmp;
            node->modules[node->module_count - 1] = (char *)malloc(strnlen(module, STRNLEN_MAX) + 1);
            if (!node->modules[node->module_count - 1])
            {
                klog_err("Failed to request memory %d module", node->module_count);
                kdk_hw_free_pci_info(result);
                result = NULL;
                goto out;
            }

            strncpy(node->modules[node->module_count - 1], module, strnlen(module, STRNLEN_MAX));
            // node->modules[node->module_count] = NULL;
        }
    }
out:
    while (first_dev)
    {
        d = first_dev;
        first_dev = first_dev->next;
        free(d->config);
        free(d->present);
        free(d);
    }
    show_kernel_cleanup();
    pci_cleanup(pacc);
    return result;
}

void kdk_hw_free_pci_info(struct KPci *info)
{
    struct KPci *tmp = NULL;
    while (info)
    {
        tmp = info;
        info = info->next;
        for (int i = 0; i < tmp->module_count; i++)
            free(tmp->modules[i]);
        free(tmp->modules);
        free(tmp);
    }
}

char *kdk_system_get_appScene()
{
    char *osinfo = NULL;
    char *license = NULL;
    osinfo = file_contains("/etc/.kylin-osinfo", "APP_SCENE");
    if(strcmp(osinfo, "none") != 0)
    {
        return osinfo;
    }
    license = file_contains("/etc/LICENSE", "APP_SCENE");
    if((strcmp(license, "none") != 0) && (strcmp(osinfo, "none") == 0))
    {
        free(osinfo);
        return license;
    }
    else
    {
        free(license);
        return osinfo;
    }
}

static bool kdk_system_check_has_user(char *username)
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

bool kdk_system_user_logout_status(char *name)
{
    bool status = true;
    if (kdk_system_check_has_user(name) == false)
    {
        return true;
    }

    DBusConnection *connt = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    DBusMessageIter iter, args;

    // 初始化DBus连接
    dbus_error_init(&error);
    connt = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connt == NULL)
    {
        klog_err("DBus connection error: %s\n", error.message);
        // goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        klog_err("DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connt, message, -1, &error);
    if (reply == NULL)
    {
        klog_err("DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);

    // 解析回复消息

    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        klog_err("DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);

    // 获取数组长度  获取的长度不对 弃用
    dbus_message_iter_recurse(&iter, &iter);

    // 解析数组中的每个结构体元素
    while (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID)
    {
        DBusMessageIter struct_iter;

        char *iter_name = NULL;
        dbus_message_iter_recurse(&iter, &struct_iter);

        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &iter_name); // 获取name
        if (strcmp(iter_name, name) == 0)
        {
            status = false;
        }

        dbus_message_iter_next(&struct_iter);

        dbus_message_iter_next(&iter);
    }

out:
    dbus_error_free(&error);
    if (connt)
        dbus_connection_unref(connt);
    return status;
}

static pthread_t thread = 0;
static volatile int running = 0;
static char *curr_user = NULL;
static CallBack _callback = NULL;

static DBusHandlerResult signal_handler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    DBusError err;
    dbus_error_init(&err);

    if (dbus_message_is_signal(message, "org.freedesktop.DBus.Properties", "PropertiesChanged"))
    {
        // const char *interface_name;
        DBusMessageIter array_iter, dict_iter, struct_iter, value_iter;
        // const char *property_name;
        const char *session_id, *session_path;

        // 迭代到interfacename
        DBusMessageIter iter;
        dbus_message_iter_init(message, &iter);
        // dbus_message_iter_get_basic(&iter, &interface_name);

        // 迭代到propertiename
        dbus_message_iter_next(&iter);
        dbus_message_iter_recurse(&iter, &array_iter);
        dbus_message_iter_recurse(&array_iter, &dict_iter);
        // dbus_message_iter_get_basic(&dict_iter, &property_name);

        // 迭代到propertievalue
        dbus_message_iter_next(&dict_iter);
        dbus_message_iter_recurse(&dict_iter, &struct_iter);
        dbus_message_iter_recurse(&struct_iter, &value_iter);
        dbus_message_iter_get_basic(&value_iter, &session_id);
        dbus_message_iter_next(&value_iter);
        dbus_message_iter_get_basic(&value_iter, &session_path);

        // 切换tty会出发信号，切换到tty1 tty2等终端，但这些没有用户登录时，不相应
        if (0 == strcmp("/", session_path))
        {
            // free(curr_user);
            // curr_user = NULL;
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        // 创建DBus方法调用消息
        DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.login1",
                                                        session_path,
                                                        "org.freedesktop.DBus.Properties",
                                                        "Get");
        char *interface = "org.freedesktop.login1.Session";
        char *name = "Name";
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

        // 发送消息并等待响应
        DBusMessage *reply;
        reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &err);
        if (dbus_error_is_set(&err))
        {
            klog_err("Error sending message: %s\n", err.message);
            dbus_error_free(&err);
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        // 处理响应
        const char *response;
        DBusMessageIter reply_iter;
        dbus_message_iter_init(reply, &reply_iter);
        // 移动到variant参数的内部
        dbus_message_iter_recurse(&reply_iter, &reply_iter);
        dbus_message_iter_get_basic(&reply_iter, &response);

        // 释放消息和连接
        dbus_message_unref(msg);
        dbus_message_unref(reply);

        if (0 != strcmp(response, "lightdm"))
        {
            // struct passwd *pwd = getpwnam(response);
            _callback(curr_user, response, user_data);
            free(curr_user);
            curr_user = strdup(response);
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static void *dbus_listener_thread(void *data)
{
    int old_type;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type);

    DBusConnection *connection;
    DBusError error;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        fprintf(stderr, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        pthread_exit(NULL);
    }
    if (connection == NULL)
    {
        fprintf(stderr, "Connection Null\n");
        pthread_exit(NULL);
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                       "type='signal',path='/org/freedesktop/login1/seat/seat0',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'",
                       &error);
    dbus_connection_add_filter(connection, signal_handler, NULL, NULL);

    // 循环监听DBus消息
    while (true)
    {
        // if (running)
        //     dbus_connection_read_write_dispatch(connection, 1000);
        // else
        // {
        //     dbus_connection_unref(connection);
        //     pthread_exit(NULL);
        // }
        dbus_connection_read_write_dispatch(connection, -1);
    }

    return NULL;
}

int kdk_system_register_switch_user_handle(CallBack call_back, void *user_data)
{
    DBusConnection *connection = NULL;
    DBusMessage *msg = NULL, *reply = NULL;
    DBusError err;
    int ret = -1;

    // 初始化DBus错误结构
    dbus_error_init(&err);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        return 1;
    }
    if (!connection)
    {
        klog_err("Failed to connect to D-Bus system bus\n");
        return 1;
    }

    // 创建DBus方法调用消息
    msg = dbus_message_new_method_call("org.freedesktop.login1",
                                      "/org/freedesktop/login1",
                                      "org.freedesktop.login1.Manager",
                                      "GetSeat");
    if (!msg)
    {
        klog_err("Failed to create D-Bus message\n");
        goto out;
    }

    const char *arg = "seat1";
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &arg, DBUS_TYPE_INVALID))
    {
        klog_err("Failed to append arguments to D-Bus message\n");
        goto out;
    }

    // 发送消息并等待响应
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &err);
    if (dbus_error_is_set(&err))
    {
        klog_err("D-Bus error: %s\n", err.message);
        dbus_error_free(&err);
    }
    else
    {
        // 成功获取 seat 信息，可以继续
        dbus_message_unref(reply);
    }

    // 获取当前登录用户
    char *tmp = kdk_system_get_eUser();
    curr_user = strdup(tmp ? tmp : "");

    // 设置回调并启动监听线程
    running = 1;
    _callback = call_back;

    int thread_ret = pthread_create(&thread, NULL, dbus_listener_thread, user_data);
    if (thread_ret != 0)
    {
        klog_err("Failed to create D-Bus listener thread: %d\n", thread_ret);
        running = 0;
        free(curr_user);
        curr_user = NULL;
        goto out;
    }

    ret = 0; // 成功

out:
    if (msg)
        dbus_message_unref(msg);
    if (reply)
        dbus_message_unref(reply);
    if (connection)
        dbus_connection_unref(connection);

    return ret;
}

void kdk_system_unregister_switch_user_handle()
{
    if (pthread_kill(thread, 0) == 0)
    {
        pthread_cancel(thread);
        pthread_join(thread, NULL);
        thread = 0;
        _callback = NULL;
    }
}

char *kdk_system_get_startup_takeTime()
{
    char str[256] = {0};
    char *time = NULL;
    FILE *pipeLine = popen("systemd-analyze", "r"); // 建立流管道
    if (!pipeLine)
    {
        return NULL;
    }

    while (fgets(str, 256 * sizeof(char), pipeLine))
    {
        if (strstr(str, "reached after"))
        {
            time = (char *)malloc(sizeof(char) * 256);
            if (!time)
            {
                pclose(pipeLine);
                return NULL;
            }
            sscanf(str, "%*s %*s %*s %s %*s %*s", time);
        }
    }
    pclose(pipeLine);
    return time;
}

unsigned int kdk_system_get_file_descriptor()
{
    FILE *fp = NULL;
    unsigned int file_nr = 0;

    errno = 0;
    if ((fp = fopen("/proc/sys/fs/file-nr", "r")))
    {
        char buf[128] = {0};
        if (fgets(buf, sizeof(buf), fp))
        {
            int nm = sscanf(buf, "%u", &file_nr);
            if (nm == 1)
            {
                return file_nr;
            }
        }
    }
    fclose(fp);
    return 0;
}

unsigned int kdk_system_get_process_nums()
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    // 打开/proc目录
    dir = opendir("/proc");
    if (dir == NULL)
    {
        // klog_err("无法打开/proc目录");
        return 1;
    }

    // 遍历目录并计算进程总数
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            // 判断目录名是否为数字（代表进程）
            char *name = entry->d_name;
            int is_process = 1;
            for (int i = 0; name[i] != '\0'; i++)
            {
                if (!isdigit(name[i]))
                {
                    is_process = 0;
                    break;
                }
            }

            if (is_process)
            {
                count++;
            }
        }
    }

    closedir(dir);

    return count;
}

unsigned int kdk_system_get_thread_nums()
{
    DIR *dir;
    struct dirent *entry;
    char path[128] = {0};
    struct stat fileStat;
    int threadCount = 0;
    char buffer[1024] = {0};

    // 打开/proc目录
    dir = opendir("/proc");
    if (dir == NULL) {
        klog_err("无法打开/proc目录\n");
        return 1;
    }

    // 遍历目录并计算进程总数
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // 判断目录名是否为数字（代表进程）
            char *name = entry->d_name;
            int is_process = 1;
            for (int i = 0; name[i] != '\0'; i++) {
                if (!isdigit(name[i])) {
                    is_process = 0;
                    break;
                }
            }

            if (is_process) {
                // count++;
                snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
                char canonical_filename[4096] = {"\0"};
                FILE *fp = NULL;
                if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
                {
                    continue;
                }
                fp = fopen(canonical_filename, "r");
                if(!fp)
                {
                    continue;
                }
                char *threads = get_val_from_file(fp, "Threads");
                strstripspace(threads);
                threadCount += atoi(threads);
                fclose(fp);
                free(threads);
            }
        }
    }

    closedir(dir);

    return threadCount;
}

#define LOAD_AVG_INIT {0.0, 0.0, 0.0}
kdk_loadavg kdk_system_get_loadavg()
{
    struct sysinfo info;
    kdk_loadavg loadAvg = LOAD_AVG_INIT;
    int rc = sysinfo(&info);
    if (!rc)
    {
        loadAvg.loadavg_1m = (float)info.loads[0] / (1 << SI_LOAD_SHIFT);
        loadAvg.loadavg_5m = (float)info.loads[1] / (1 << SI_LOAD_SHIFT);
        loadAvg.loadavg_15m = (float)info.loads[2] / (1 << SI_LOAD_SHIFT);
    }
    return loadAvg;
}
#undef LOAD_AVG_INIT

char *_kdk_system_get_grub_menu(void)
{
    const char grub_cfg[] = "/boot/grub/grub.cfg";

    // 检查文件是否存在
    FILE *file = fopen(grub_cfg, "r");
    if (!file)
    {
        klog_err("Error opening file\n");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    int in_submenu = 0;        // 标记是否在submenu中
    int pre_line_endswith = 0; // 标记上一行是否以'}'结尾

    regex_t regex;
    // 编译正则表达式，匹配单引号中的内容
    if (regcomp(&regex, "'([^']*)'", REG_EXTENDED))
    {
        klog_err("Error compiling regex\n");
        fclose(file);
        return NULL;
    }

    cJSON *json = cJSON_CreateObject(); // 创建JSON对象

    while (fgets(line, sizeof(line), file))
    {
        // line[strcspn(line, "\r\n")] = 0; // 去掉行末的换行符和回车符

        strstripspace(line);

        // 计算行首空格数量
        size_t startIndex = 0;
        while (isspace((unsigned char)line[startIndex]))
        { // 使用isspace检查字符是否为空格，注意类型转换以确保对所有字符的正确处理
            startIndex++;
        }

        // 如果有空格，将非空格字符向前移动，覆盖掉空格
        if (startIndex > 0)
        {
            memmove(line, line + startIndex, strnlen(line + startIndex, STRNLEN_MAX) + 1); // 使用memmove避免源和目标区域重叠的问题
        }

        regmatch_t matches[2];
        // 检查行是否以"menuentry "或"submenu "开头
        if (strncmp(line, "menuentry ", 10) == 0 || strncmp(line, "submenu ", 8) == 0)
        {
            // 执行正则表达式匹配
            if (regexec(&regex, line, 2, matches, 0) == 0)
            {
                // 提取匹配的条目名称
                int match_length = matches[1].rm_eo - matches[1].rm_so;
                char *entry_name = malloc(match_length + 1);
                strncpy(entry_name, &line[matches[1].rm_so], match_length);
                entry_name[match_length] = '\0';

                // 判断是menuentry还是submenu
                if (strncmp(line, "submenu ", 8) == 0)
                {
                    cJSON_AddStringToObject(json, entry_name, "submenu");
                    in_submenu = 1; // 进入submenu
                }
                if (strncmp(line, "menuentry ", 10) == 0 && in_submenu == 0)
                {
                    cJSON_AddStringToObject(json, entry_name, "menuentry");
                }

                free(entry_name); // 释放分配的内存
            }
        }

        // 检查是否退出submenu,如果上一行和当前行都以}结尾，表示从submenu块中退出。
        if (pre_line_endswith && line[strnlen(line, STRNLEN_MAX) - 1] == '}')
        {
            in_submenu = 0;
            // printf("out submenu\n");
        }
        pre_line_endswith = (line[strnlen(line, STRNLEN_MAX) - 1] == '}');
    }

    fclose(file);
    regfree(&regex);

    char *json_string = cJSON_PrintUnformatted(json); // 转换JSON对象为字符串
    cJSON_Delete(json);                               // 删除JSON对象

    return json_string;
}

char *kdk_system_get_grub_menu(void)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
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

    // 创建消息
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/sysinfo", // object to call on
                                            "com.kylin.kysdk.sysinfo",  // interface to call on
                                            "getSystemGrubMenu");      // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
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
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
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
        ret = NULL;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
        goto out;
    }
    else
    {
        const char *menu_str = NULL;
        dbus_message_iter_get_basic(&args, &menu_str);
        if (menu_str)
        {
            ret = strdup(menu_str);
        }
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

out:
    if (ret == NULL)
    {
        ret = _kdk_system_get_grub_menu();
    }

    return ret;
}

SystemForm _kdk_system_get_basic_form(void)
{
    char buffer[128];
    FILE *pipe = popen("ostree admin status", "r");
    if (!pipe)
    {
        // 如果ostree命令不存在或无法执行，popen会返回NULL
        return K_SYSTEM_NONE;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        // 检查命令输出中是否包含特定标记（这里假设"*"作为示例）
        if (strstr(buffer, "*") != NULL)
        {
            pclose(pipe);
            return K_SYSTEM_IMMUTABLEOS;
        }
    }

    // 如果没有找到特定标记，则认为是NONE类型
    pclose(pipe);
    return K_SYSTEM_NONE;
}

SystemForm kdk_system_get_basic_form()
{
    int ret = K_SYSTEM_ERR;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = K_SYSTEM_ERR;
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

    // 创建用户
    info_msg = dbus_message_new_method_call("com.kylin.kysdk.service",  // target for the method call
                                            "/com/kylin/kysdk/sysinfo", // object to call on
                                            "com.kylin.kysdk.sysinfo",  // interface to call on
                                            "getSystemForm");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = K_SYSTEM_ERR;
        goto out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = K_SYSTEM_ERR;
        goto out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = K_SYSTEM_ERR;
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
        ret = K_SYSTEM_ERR;
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
        ret = K_SYSTEM_ERR;
        goto out;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = K_SYSTEM_ERR;
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

out:
    if (ret == K_SYSTEM_ERR)
    {
        ret = _kdk_system_get_basic_form();
    }

    return ret;
}

char *kdk_system_get_system_manufacturer()
{
    // 打开 /etc/lsb-release 文件
    FILE *file = fopen("/etc/lsb-release", "r");
    if (file == NULL)
    {
        klog_err("打开文件时出错\n"); // 错误信息输出到 stderr
        return NULL;
    }

    char line[256];            // 缓冲区存储读取的一行
    char *manufacturer = NULL; // 初始化制造商变量

    // 循环读取文件每一行
    while (fgets(line, sizeof(line), file))
    {
        // 检查是否是 DISTRIB_ID 行
        if (strncmp(line, "DISTRIB_ID=", 11) == 0)
        {
            // 跳过 "DISTRIB_ID=" 并获取制造商名
            manufacturer = strchr(line, '=');
            if (manufacturer != NULL)
            {
                // 移除等号
                manufacturer++;
                // 移除行尾的换行符
                char *end = strchr(manufacturer, '\n');
                if (end != NULL)
                    *end = '\0';
                // 使用 strdup 分配内存并复制制造商名
                manufacturer = strdup(manufacturer);
                if (manufacturer == NULL)
                {
                    klog_err("内存分配失败\n");
                    fclose(file);
                    return NULL;
                }
                break;
            }
        }
    }

    fclose(file); // 关闭文件
    // 返回制造商名
    return manufacturer;
}

char *kdk_system_get_system_locale()
{
    char *locale = getenv("LC_ADDRESS");
    if (NULL == locale)
        locale = getenv("LANG");
    return strdup(locale);
}

char **kdk_system_get_env()
{
    extern char **environ;
    int i = 0;
    while (environ[i])
        i++;
    i += 2;
    char **result = calloc(i, sizeof(char *));
    if (NULL == result)
        return NULL;
    i = 0;
    while (environ[i])
    {
        result[i] = strdup(environ[i]);
        i++;
    }
    return result;
}

kdk_service_list *kdk_system_get_all_service_list()
{
    kdk_service_list *result = NULL;
    result = list_unit_files();
    if (NULL == result)
        return result;

    int r = unit_file_query_preset(result);
    return result;
}

kdk_service_list *kdk_system_get_automatic_start_service_list()
{
    kdk_service_list *list = NULL, *result = NULL;
    list = list_unit_files();
    if (NULL == list)
        return list;

    int r = unit_file_query_preset(list);

    kdk_service_list *p = list, *q = NULL, *k = NULL;
    while (p)
    {
        if (p->status == K_UNIT_FILE_ENABLED)
        {
            // 符合要求的节点从原列表中删除
            k = p->next;
            if (NULL == q)
                list = k;
            else
                q->next = k;

            // 将符合要求的节点添加到新列表
            p->next = result;
            result = p;

            p = q ? q->next : list;
        }
        else
        {
            q = p;
            p = p->next;
        }
    }
    kdk_system_free_service_list(list);
    return result;
}

void kdk_system_free_service_list(kdk_service_list *list)
{
    for (kdk_service_list *tmp = list; tmp; tmp = list)
    {
        list = list->next;
        free(tmp);
    }
}

MachineType kdk_system_get_machine_type()
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/sys/class/power_supply/");
    if (dir == NULL)
    {
        klog_err("opendir failed \n");
        return 1;
    }

    int found_bat = 0; // 用于标记是否找到以BAT开头的文件

    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, "BAT", 3) == 0)
        {
            found_bat = 1;
            break;
        }
    }

    closedir(dir);

    if (found_bat)
        return K_TABLET;
    else
        return K_STATIONARY;
}

static int lookup(char *line, char *pattern, char **value)
{
	char *p, *v;
	int len = strnlen(pattern, STRNLEN_MAX);
    // klog_debug("%s\n",pattern);
	/* don't re-fill already found tags, first one wins */
    if (strcmp(pattern,"processor")){
        if (!*line || *value)
		    return 0;
    }

	/* pattern */
	if (strncmp(line, pattern, len))
		return 0;

	/* white spaces */
	for (p = line + len; isspace(*p); p++);

	/* separator */
	if (*p != ':')
		return 0;

	/* white spaces */
	for (++p; isspace(*p); p++);

	/* value */
	if (!*p)
		return 0;
	v = p;

	/* end of value */
	len = strnlen(line, STRNLEN_MAX) - 1;
	for (p = line + len; isspace(*(p-1)); p--);
	*p = '\0';

	*value = strdup(v);
	return 1;
}

// 执行 dpkg --compare-versions 命令
static int compare_versions(const char *version1, const char *operator, const char *version2) 
{    
    int pipefd[2];
    pid_t pid;
    int status;
    char command[256];

    // 构造 "dpkg --compare-versions %s %s %s; echo $?" 命令
    snprintf(command, sizeof(command), "dpkg --compare-versions %s %s %s; echo $?", version1, operator, version2);

    if (pipe(pipefd) == -1) {
        // klog_err("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        // klog_err("fork");
        return -1;
    }

    if (pid == 0) {  // Child process
        close(pipefd[0]);  // 关闭管道的读端
        dup2(pipefd[1], STDOUT_FILENO);  // 将标准输出重定向到管道
        close(pipefd[1]);

        // 使用 /bin/sh 来执行 snprintf 构造的命令
        char *args[] = {"/bin/sh", "-c", command, NULL};
        execve("/bin/sh", args, NULL);

        // klog_err("execve");  // 如果 execve 失败
        exit(EXIT_FAILURE);
    } else {  // Parent process
        close(pipefd[1]);  // 关闭管道的写端
        waitpid(pid, &status, 0);  // 等待子进程结束

        char result_buffer[16] = "\0";
        read(pipefd[0], result_buffer, sizeof(result_buffer));  // 从管道读取输出
        close(pipefd[0]);

        if (WIFEXITED(status)) {
            int result = atoi(result_buffer);  // 将输出的 $? 转换为整数
            // printf("Result: %d\n", result);  
            return result;
        } else {
            // klog_err("waitpid");
            return -1;
        }
    }
}

static int kdk_version_verify(const char *package, const char *version)
{
    if(strcmp(package, "ukui-control-center") == 0)
    {
        int min_check_320= compare_versions(version, "gt", "3.20");
        int max_check_320 = compare_versions(version, "lt", "3.21");
        if(min_check_320 == 0 && max_check_320 == 0)
            return 1;

        int min_check_324= compare_versions(version, "gt", "3.24");
        int max_check_324 = compare_versions(version, "lt", "3.25");
        if(min_check_324 == 0 && max_check_324 == 0)
            return 2;
    }

    if(strcmp(package, "ukui-search") == 0)
    {
        int max_check_40 = compare_versions(version, "lt", "4.0");
        if(max_check_40 == 0)
            return 3;
    }

    if(strcmp(package, "ukui-globaltheme-common") == 0)
    {
        int max_check_324 = compare_versions(version, "lt", "3.24");
        if(max_check_324 == 0)
            return 4;
    }

    if(strcmp(package, "mate-panel") == 0 || strcmp(package, "mate-menus") == 0 || strcmp(package, "mate-session-manager") == 0 
        || strcmp(package, "caja") == 0)
    {
        int max_check_324 = compare_versions(version, "lt", "1.24.0");
        if(max_check_324 == 0)
            return 5;
    }

    return 0;
}

static int check_cpu_mode(char *mode)
{
    if(mode)
    {
        if(strstr(mode, "phytium") || strstr(mode, "kunpeng") || strstr(mode, "ft-2000") || strstr(mode, "d2000"))
            return 1;
        else if(strstr(mode, "zhaoxin") || strstr(mode, "hygon"))
            return 1;
        else if(strstr(mode, "intel") || strstr(mode, "amd"))
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

static int check_kernel_version(char *release)
{
    if(strncmp(release, "5.4.18", 7) == 0 && strstr(release, "hw") == NULL)
        return 1;
    else if(strncmp(release, "5.10.0", 7) == 0)
        return 1;
    else if(strncmp(release, "5.15.0", 7) == 0)
        return 1;
    else
        return 0;
}

static int check_cpu_hw()
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(!fp)
        return 0;
    
    int ret = 0;    
    char line[8192] = "\0";
    while (fgets(line, 8192, fp))
    {
        str2lower(line);
        if(strstr(line, "kirin") && strstr(line, "990"))
            ret = 1;
        else if(strstr(line, "kirin") && strstr(line, "9006c"))
            ret = 1;
        else if(strstr(line, "pangu") && strstr(line, "m900"))
            ret = 1;
        else if(strstr(line, "kirin") && strstr(line, "9000c"))
            ret = 1;
        else
            ret = 0;
    }
    fclose(fp);
    return ret;
}

static const char* product = "Unknown";

static const char *kdk_arch_product_line_verify()
{
    if(strcmp(product, "Unknown") != 0)
        return product;
    
    struct utsname buffer;
    if(uname(&buffer) != 0)
    {
        return "Unknown";
    }

    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(!fp)
        return "Unknown";
    
    char line[8192] = "\0";
    char *mode = NULL;
    while (fgets(line, 8192, fp))
    {
        if (lookup(line, "model name", &mode));
    }
    fclose(fp);

    if(mode)
    {
        str2lower(mode);
    }

    if(strcmp(buffer.machine, "aarch64") == 0 && (check_cpu_mode(mode) || check_kernel_version(buffer.release)))
    {
        product = "XC";
    }

    if(strcmp(buffer.machine, "x86_64") == 0 && (check_cpu_mode(mode) || check_kernel_version(buffer.release)))
    {
        product = "XC";
    }

    if(strcmp(buffer.machine, "mips64") == 0)
    {
        product = "XC";
    }

    if(strcmp(buffer.machine, "loongarch64") == 0)
    {
        product = "XC";
    }

    if(strcmp(buffer.machine, "sw_64") == 0)
    {
        product = "XC";
    }

    if(strcmp(buffer.machine, "x86_64") == 0 && check_cpu_mode(mode) && check_kernel_version(buffer.release))
    {
        product = "HWE";
    }

    if(strcmp(buffer.machine, "aarch64") == 0 && check_cpu_hw())
        product = "Wayland";
    
    return product;
}

static const char *kdk_build_id_product_line_verify()
{
    if(strcmp(product, "Unknown") != 0)
        return product;
    
    char buffer[128] = "\0";
    char *val = NULL;
    const char *match_wayland[] = {
        "27647", "29861", "33071", "24188", "19730", "23518", "24334", "29039",
        "35063", "37257", "38601", "40222", "38540", "40198", "46149", "46147",
        "44401", "44409", "49400", "49399", "52314", "52313", "52312", "54852",
        "54854", "54853"
    };
    int wayland_size =  (sizeof(match_wayland) / sizeof(match_wayland[0]));

    const char *match_xc[] = {
        "18817", "20929", "31949", "38109", "41998", "44398", "48722", "52363", "54198", "57090",
        "18818", "20930", "31950", "38110", "41999", "44404", "48723", "52364", "54202", "57091",
        "18819", "20931", "31951", "38111", "42000", "44405", "48724", "52365",
        "18820", "21167", "31952", "38112", "42001", "44406", "48725", "52366", "55148", "57093",
        "42002", "44397", "48743", "52367", "54197", "57094"
    };
    int xc_size =  (sizeof(match_xc) / sizeof(match_xc[0]));

    const char *match_hwe[] = {
        "18817", "20929", "31949", "38109", "41998", "44398", "48722", "52363", "54198", "57090",
        "18818", "20930", "31950", "38110", "41999", "44404", "48723", "52364", "54202", "57091",
        "18819", "20931", "31951", "38111", "42000", "44405", "48724", "52365",
        "18820", "21167", "31952", "38112", "42001", "44406", "48725", "52366", "55148", "57093",
        "42002", "44397", "48743", "52367", "54197", "57094",
        "22092", "32941", "36197", "37285", "38114", "41996", "44395", "48740", "52368", "54195",
        "41997", "44396", "48741", "52369", "54196"
    };
    int hwe_size =  (sizeof(match_hwe) / sizeof(match_hwe[0]));

    FILE *fp = fopen("/etc/kylin-build", "r");
    if(!fp)
        return "Unknown";
    while (fgets(buffer, 128, fp))
    {
        if(strstr(buffer, "buildid"))
        {
            val = package_lookup(buffer, "buildid");
        }
    }

    fclose(fp);

    if(val)
    {
        for (int i = 0; i < wayland_size; i++) {
            if (strcmp(val, match_wayland[i]) == 0) {
                product = "Wayland";
                return product;
            }
        }

        for (int i = 0; i < xc_size; i++) {
            if (strcmp(val, match_xc[i]) == 0) {
                product = "XC";
                return product;
            }
        }

        for (int i = 0; i < hwe_size; i++) {
            if (strcmp(val, match_hwe[i]) == 0) {
                product = "HWE";
                return product;
            }
        }
    }
    
    return product;
}

static const char *kdk_package_product_line_verify()
{
    if(strcmp(product, "Unknown") != 0)
        return product;
    
    int hwe_ret = kdk_package_is_installed("linux-headers-generic-hwe-v10pro", NULL);
    if(hwe_ret == 1)
    {
        product = "HWE";
    }

    int wayland_ret_990 = kdk_package_is_installed("linux-headers-kr990", NULL);
    int wayland_ret_9a0 = kdk_package_is_installed("linux-headers-kr9a0", NULL);
    int wayland_ret_image = kdk_package_is_installed("linux-image-4.19.71-23-kr990", NULL);
    int wayland_ret_pangu = kdk_package_is_installed("linux-headers-pangu", NULL);
    int wayland_ret_pangux = kdk_package_is_installed("linux-image-pangux", NULL);

    if(wayland_ret_990 == 1 || wayland_ret_9a0 == 1 || wayland_ret_image == 1 || wayland_ret_pangu == 1 || wayland_ret_pangux == 1)
    {
        product = "Wayland";
    }

    int xc_ret_bj = kdk_package_is_installed("linux-image-5.4.18-28.23-bj-generic", NULL);
    int xc_ret_image = kdk_package_is_installed("linux-image-generic", NULL);
    int xc_ret_85 = kdk_package_is_installed("linux-image-5.4.18-85-generic", NULL);
    int xc_ret_91 = kdk_package_is_installed("linux-image-5.4.18-91-generic", NULL);
    
    if(xc_ret_bj == 1 || xc_ret_image == 1 || xc_ret_85 == 1 || xc_ret_91 == 1)
    {
        product = "XC";
    }

    return product;
}

static char productInfo[128] = "\0";

static const char *kdk_system_tag_product_line_verify()
{
    if(strcmp(product, "Unknown") != 0)
        return product;
    
    char line[128] = "\0";
    FILE *fp = fopen("/etc/.kylin-osinfo", "rt");
    if (!fp)
        return "Unknown";

    char *val = NULL;

    while (fgets(line, 128, fp))
    {
        if(strstr(line, "ProductInfo"))
        {
            val = package_lookup(line, "ProductInfo");
            break;
        }
    }
    fclose(fp);

    if(val)
    {
        strskipspace(val);
        strncpy(productInfo, val, sizeof(productInfo) - 1);
        product = productInfo;
    }

    return product;
}

const char *kdk_system_get_production_line()
{
    const char *line;

    line = kdk_system_tag_product_line_verify();

    if(strcmp(line, "Unknown") == 0)
    {
        line = kdk_build_id_product_line_verify();
    }

    if(strcmp(line, "Unknown") == 0)
    {
        line = kdk_package_product_line_verify();
    }

    if(strcmp(line, "Unknown") == 0)
    {
        line = kdk_arch_product_line_verify();
    }
    return line;
}

static const char *majorVer = "Unknown";

static const char *kdk_package_verfiy()
{
    if(strcmp(majorVer, "Unknown") != 0)
        return majorVer;
    
    int pan_ret = 0;
    int men_ret = 0;
    int man_ret = 0;
    int caja_ret = 0;
    
    int ret_panel = kdk_package_is_installed("mate-panel", NULL);
    if(ret_panel == 1)
    {
        char* panel_ver =  kdk_package_get_version("mate-panel");
        pan_ret = kdk_version_verify("mate-panel", panel_ver);
        free(panel_ver);
    }
    int ret_menus = kdk_package_is_installed("mate-menus", NULL);
    if(ret_menus == 1)
    {
        char *menus_ver = kdk_package_get_version("mate-menus");
        men_ret = kdk_version_verify("mate-menus", menus_ver);
        free(menus_ver);
    }
    int ret_manager = kdk_package_is_installed("mate-session-manager", NULL);
    if(ret_manager == 1)
    {
        char *manager_ver = kdk_package_get_version("mate-session-manager");
        man_ret = kdk_version_verify("mate-session-manager", manager_ver);
        free(manager_ver);
    }
    int ret_caja = kdk_package_is_installed("caja", NULL);
    if(ret_caja == 1)
    {
        char *caja_ver = kdk_package_get_version("caja");
        caja_ret = kdk_version_verify("caja", caja_ver);
        free(caja_ver);
    }

    if(pan_ret == 5 && men_ret == 5 && man_ret == 5 && caja_ret == 5)
        majorVer = "V10";
    else
        majorVer = "V10-SP1";
    
    return majorVer;
}

static char majorVersion[128] = "\0";
static const char *kdk_system_tag_major_version_verify()
{
    if(strcmp(majorVer, "Unknown") != 0)
        return majorVer;
    
    char line[128] = "\0";
    FILE *fp = fopen("/etc/.kylin-osinfo", "rt");
    if (!fp)
        return "Unknown";

    char *val = NULL;

    while (fgets(line, 128, fp))
    {
        if(strstr(line, "MajorVersion"))
        {
            val = package_lookup(line, "MajorVersion");
            break;
        }
    }
    fclose(fp);

    if(val)
    {
        strskipspace(val);
        strncpy(majorVersion, val, sizeof(majorVersion) - 1);
        majorVer = majorVersion;
    }

    return majorVer;
}

const char* kdk_system_get_major_version()
{
    const char *major;
    major = kdk_system_tag_major_version_verify();
    if(strcmp(major, "Unknown") == 0)
    {
        major = kdk_package_verfiy();
    }
    return major;
}

static const char *minorVer = "Unknown";

static const char *kdk_file_verify()
{
    if(strcmp(minorVer, "Unknown") != 0)
        return minorVer;

    char *release_id = NULL;
    FILE *fp = fopen("/etc/os-release", "r");
    if(!fp)
        return "Unknown";
    release_id = get_val_from_file(fp, "KYLIN_RELEASE_ID");
    if(!release_id)
    {
        minorVer = "2107";
        return minorVer;
    }
    fclose(fp);
    strstripspace(release_id);
    
    char* center_ver =  kdk_package_get_version("ukui-control-center");
    char *search_ver = kdk_package_get_version("ukui-search");
    char *globaltheme_ver = kdk_package_get_version("ukui-globaltheme-common");
    int ret =  kdk_package_is_installed("kylin-os-config-common", NULL);

    if(strcmp(release_id, "2203") == 0)
    {
        minorVer = "2203";
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return minorVer;
    }
       
    else if(strcmp(release_id, "2303") == 0 && kdk_version_verify("ukui-control-center", center_ver) == 1
            && kdk_version_verify("ukui-search", search_ver) == 3)
    {
        minorVer = "2303";
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return minorVer;
    }
       
    else if (strcmp(release_id, "2303") == 0 && kdk_version_verify("ukui-control-center", center_ver) == 2
            && kdk_version_verify("ukui-globaltheme-common", globaltheme_ver) == 4)
    {
        minorVer = "2303-update2";
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return minorVer;
    }
    else if (strcmp(release_id, "2403") == 0 && ret != 1)
    {
        minorVer = "2403";
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return minorVer;
    }
    else if (strcmp(release_id, "2403") == 0 && ret == 1)
    {
        minorVer = "2403-update1";
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return minorVer;
    }
    else
    {
        free(center_ver);
        free(search_ver);
        free(globaltheme_ver);
        free(release_id);
        return "Unknown";
    }
}

#define MAXSIZES 50

#define C_CTYPE_BIT(bit)	(1 << (bit))

enum c_ctype_bit {
	C_CTYPE_BLANK = C_CTYPE_BIT(0),
	C_CTYPE_WHITE = C_CTYPE_BIT(1),
	C_CTYPE_SPACE = C_CTYPE_BIT(2),
	C_CTYPE_UPPER = C_CTYPE_BIT(3),
	C_CTYPE_LOWER = C_CTYPE_BIT(4),
	C_CTYPE_DIGIT = C_CTYPE_BIT(5),

	C_CTYPE_ALPHA = C_CTYPE_UPPER | C_CTYPE_LOWER,
	C_CTYPE_ALNUM = C_CTYPE_ALPHA | C_CTYPE_DIGIT,
};

#undef S
#define S C_CTYPE_SPACE
#undef W
#define W C_CTYPE_WHITE
#undef B
#define B C_CTYPE_BLANK
#undef U
#define U C_CTYPE_UPPER
#undef L
#define L C_CTYPE_LOWER
#undef D
#define D C_CTYPE_DIGIT

static unsigned short int c_ctype[256] = {
/** 0 **/
	/* \0 */ 0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* \a */ 0,
	/* \b */ 0,
	/* \t */ S | W | B,
	/* \n */ S | W,
	/* \v */ S,
	/* \f */ S,
	/* \r */ S,
	0,
	0,
/** 16 **/
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
/** 32 **/
	/*   */ S | W | B,
	/* ! */ 0,
	/* " */ 0,
	/* # */ 0,
	/* $ */ 0,
	/* % */ 0,
	/* & */ 0,
	/* ' */ 0,
	/* ( */ 0,
	/* ) */ 0,
	/* * */ 0,
	/* + */ 0,
	/* , */ 0,
	/* - */ 0,
	/* . */ 0,
	/* / */ 0,
/** 48 **/
	/* 0 */ D,
	/* 1 */ D,
	/* 2 */ D,
	/* 3 */ D,
	/* 4 */ D,
	/* 5 */ D,
	/* 6 */ D,
	/* 7 */ D,
	/* 8 */ D,
	/* 9 */ D,
	/* : */ 0,
	/* ; */ 0,
	/* < */ 0,
	/* = */ 0,
	/* > */ 0,
	/* ? */ 0,
/* 64 */
	/* @ */ 0,
	/* A */ U,
	/* B */ U,
	/* C */ U,
	/* D */ U,
	/* E */ U,
	/* F */ U,
	/* G */ U,
	/* H */ U,
	/* I */ U,
	/* J */ U,
	/* K */ U,
	/* L */ U,
	/* M */ U,
	/* N */ U,
	/* O */ U,
/* 80 */
	/* P */ U,
	/* Q */ U,
	/* R */ U,
	/* S */ U,
	/* T */ U,
	/* U */ U,
	/* V */ U,
	/* W */ U,
	/* X */ U,
	/* Y */ U,
	/* Z */ U,
	/* [ */ 0,
	/* \ */ 0,
	/* ] */ 0,
	/* ^ */ 0,
	/* _ */ 0,
/* 96 */
	/* ` */ 0,
	/* a */ L,
	/* b */ L,
	/* c */ L,
	/* d */ L,
	/* e */ L,
	/* f */ L,
	/* g */ L,
	/* h */ L,
	/* i */ L,
	/* j */ L,
	/* k */ L,
	/* l */ L,
	/* m */ L,
	/* n */ L,
	/* o */ L,
/* 112 */
	/* p */ L,
	/* q */ L,
	/* r */ L,
	/* s */ L,
	/* t */ L,
	/* u */ L,
	/* v */ L,
	/* w */ L,
	/* x */ L,
	/* y */ L,
	/* z */ L,
	/* { */ 0,
	/* | */ 0,
	/* } */ 0,
	/* ~ */ 0,
	0,
/* 128 */
};

static void Substitute(const char *pInput, char *pOutput, char *pSrc, char *pDst)
{
    char    *pi, *po, *p;
    int     nSrcLen, nDstLen, nLen;
    po = pOutput;
    pi = pInput;
    nSrcLen = strnlen(pSrc, STRNLEN_MAX);
    nDstLen = strnlen(pDst, STRNLEN_MAX);
    p = strstr(pi, pSrc);
    if(p)
    {
        while(p)
        {
            nLen = (int)(p - pi);
            memcpy(po, pi, nLen);
            memcpy(po + nLen, pDst, nDstLen);
            pi = p + nSrcLen;
            po = po + nLen + nDstLen;
            p = strstr(pi, pSrc);
        }
        strncpy(po, pi, strnlen(pi, STRNLEN_MAX) + 1);
    }
    else
    {
        strncpy(po, pi, strnlen(pi, STRNLEN_MAX) + 1);
    }
}

static bool c_isbits(int c, enum c_ctype_bit bits)
{
	return ((c_ctype[(unsigned char)c] & bits) != 0);
}

static inline bool c_isdigit(int c)
{
	return c_isbits(c, C_CTYPE_DIGIT);
}

static inline bool c_isalpha(int c)
{
	return c_isbits(c, C_CTYPE_ALPHA);
}

static int order(int c)
{
	if (c_isdigit(c))
		return 0;
	else if (c_isalpha(c))
		return c;
	else if (c == '~')
		return -1;
	else if (c)
		return c + 256;
	else
		return 0;
}

static int verrevcmp(const char *a, const char *b)
{
	if (a == NULL)
		a = "";
	if (b == NULL)
		b = "";

	while (*a || *b) {
		int first_diff = 0;

		while ((*a && !c_isdigit(*a)) || (*b && !c_isdigit(*b))) {
			int ac = order(*a);
			int bc = order(*b);

			if (ac != bc)
				return ac - bc;

			a++;
			b++;
		}
		while (*a == '0')
			a++;
		while (*b == '0')
			b++;
		while (c_isdigit(*a) && c_isdigit(*b)) {
			if (!first_diff)
				first_diff = *a - *b;
			a++;
			b++;
		}

		if (c_isdigit(*a))
			return 1;
		if (c_isdigit(*b))
			return -1;
		if (first_diff)
			return first_diff;
	}

	return 0;
}

static int dpkg_version_compare(const char* verA, const char* verB)
{
	int rc;
    char a_version[MAXSIZES];
    char b_version[MAXSIZES];
    char src[MAXSIZES] = "kylin";
    char dst[MAXSIZES] = "ubuntu";

    if(verA != NULL)
    {
        Substitute(verA,a_version,src,dst);
        if(verB != NULL)
        {
            Substitute(verB,b_version,src,dst);
            rc = verrevcmp(a_version, b_version);
        }
        else
            rc = verrevcmp(a_version, verB);
    }
    else
        rc = verrevcmp(verA, verB);
    
    if (rc)
        return rc;
}

#define MAX_PACKAGES 3000
#define MAX_FILENAME_LEN 128
// 定义包信息结构
typedef struct {
    char name[128];
    char version[128];
} Package;

// 定义文件包信息结构
typedef struct {
    char filename[128];
    int package_count;
    int version_count;
} FilePackageInfo;

typedef struct {
    FilePackageInfo packInfo[64];
    char dir[64];
} PackageInfo;

static const char *serName = "Unknown";

static int read_packages_from_file(const char *filename, Package *packages)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    int count = 0;
    while (fscanf(file, "%127s %127s", packages[count].name, packages[count].version) == 2) {
        count++;
        if (count >= MAX_PACKAGES) {
            break;
        }
    }

    fclose(file);
    return count;
}

static void count_matching_packages(PackageInfo *info, int num, int index, const Package *file_packages, int file_count, char *path) {
    int match_count = 0;
    kdk_package_list *list = kdk_package_get_packagelist();
    for (int j = 0; j < list->nums; j++) {
        for (int i = 0; i < file_count; i++) {

            if (strcmp(file_packages[i].name, list->list[j]->name) == 0)
            {
                int ver_com = dpkg_version_compare(list->list[j]->version, file_packages[i].version);
                if(ver_com == 0)
                {
                    info[num].packInfo[index].version_count++;
                }
                else if(ver_com < 0 && index > 0)
                {
                    Package packages[MAX_PACKAGES];
                    char file_path[256] = "\0";
                    snprintf(file_path, sizeof(file_path), "%s/%s", path, info[num].packInfo[index - 1].filename);
                    int count = read_packages_from_file(file_path, packages);
                    if(count < 0)
                    {
                        break;
                    }
                    for (int p = 0; p < count; p++)
                    {
                        if (strcmp(packages[p].name, list->list[j]->name) == 0)
                        {
                            ver_com = dpkg_version_compare(list->list[j]->version, packages[p].version);
                            if(ver_com > 0)
                            {
                                info[num].packInfo[index - 1].version_count++; 
                            }
                        }
                    }
                }
                info[num].packInfo[index].package_count++;
                break; // 如果只需要匹配包名，找到一个匹配就可以跳出内层循环
            }
        }
    }

    kdk_package_free_packagelist(list);
}

static char max_filename[MAX_FILENAME_LEN] = "\0";
static char max_dir[MAX_FILENAME_LEN] = "\0";
static const char *kdk_package_ver_info(bool verbose)
{
    if(verbose)
    {
        if(strcmp(serName, "Unknown") != 0)
            return serName;
    }
    else{
        if(strcmp(minorVer, "Unknown") != 0)
            return minorVer;
    }

    char *debug = NULL;
    debug = getenv("SYSINFO_VERSION_DEBUG");
    
    const char *major = kdk_system_get_major_version();
    const char *production = kdk_system_get_production_line();
    if (!major || !production) 
    {
        goto out;
    }
    PackageInfo info[64];
    
    char path[256] = "\0";
    snprintf(path, sizeof(path), "/var/lib/kysdk/kysdk-system/kysysinfo/General/%s/%s", major, production);
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    dirfd = opendir(path);
    if (!dirfd)
    {
        goto out;
    }
    
    char *folders[64];
    int count = 0;

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if(direfd->d_type != DT_DIR || (strcmp(direfd->d_name, ".") == 0) || (strcmp(direfd->d_name, "..") == 0))
            continue;

        folders[count] = (char *)malloc(sizeof(char) * 32);
        if(!folders[count])
        {
            closedir(dirfd);
            goto out;
        }
        strncpy(folders[count], direfd->d_name, 31);
        count++;
    }

    closedir(dirfd);

    for (size_t i = 0; i < count; i++) 
    {
        char file_path[512] = "\0";
        snprintf(file_path, sizeof(file_path), "%s/%s", path, folders[i]);

        strncpy(info[i].dir, folders[i], sizeof(info[i].dir) - 1);

        free(folders[i]);
        int index = 0;

        struct dirent **namelist;              
        int n;

        n = scandir(file_path, &namelist, NULL, alphasort);
        if (n < 0)
        {
            goto out;
        }
        else 
        {
            for (size_t j = 0; j < n; j++) 
            {
                if (namelist[j]->d_type == DT_REG)
                {
                    Package file_packages[MAX_PACKAGES];
                    char file_name[1024] = "\0";
                    snprintf(file_name, sizeof(file_name), "%s/%s", file_path, namelist[j]->d_name);

                    int file_count = read_packages_from_file(file_name, file_packages);
                    if (file_count < 0) 
                    {
                        continue;
                    }

                    strncpy(info[i].packInfo[index].filename, namelist[j]->d_name, sizeof(info[i].packInfo[index].filename) - 1);
                    
                    count_matching_packages(info, i, index, file_packages, file_count, file_path);
                    index++;
                }
                free(namelist[j]);
            }
            free(namelist);         // 访问完指定目录下所有目录或文件信息内容，释放malloc分配的用于索引的指针数组内存
        }
    }

    int num = 0;
    double max_ratio = 0.0;

    while (num < 64 && info[num].dir[0] != '\0') {
        for (int q = 0; q < 64 && info[num].packInfo[q].filename[0] != '\0'; q++)
        {
            if (info[num].packInfo[q].package_count > 0) 
            {
                double ratio = (double)info[num].packInfo[q].version_count / info[num].packInfo[q].package_count;
                if(debug)
                {
                    if(strcmp(debug, "1") == 0)
                    {
                        printf("[DEBUG] [%s] packages in %s : %d\n", info[num].dir, info[num].packInfo[q].filename, info[num].packInfo[q].package_count);
                        printf("[DEBUG] [%s] matched packages in %s : %d\n", info[num].dir, info[num].packInfo[q].filename, info[num].packInfo[q].version_count);
                        printf("[DEBUG] [%s] accuracy in %s : %f\n", info[num].dir, info[num].packInfo[q].filename, ratio);
                    }
                }
                if (ratio > max_ratio) 
                {
                    max_ratio = ratio;
                    strncpy(max_filename, info[num].packInfo[q].filename, sizeof(max_filename));
                    strncpy(max_dir, info[num].dir, sizeof(max_dir));
                }
            }
        }
        num++;
    }

    if(strnlen(max_filename, STRNLEN_MAX) != 0 && strnlen(max_dir, STRNLEN_MAX) != 0)
    {
        minorVer = max_filename;
        serName = max_dir;
    }

    if (verbose)
    {
        return serName;
    }
    else{
        return minorVer;
    }

out:
    serName = "Unknown";
    minorVer = "Unknown";
    if (verbose)
    {
        return serName;
    }
    else{
        return minorVer;
    }
}

static char minorVersion[128] = "\0";
static const char *kdk_system_tag_minor_version_verify()
{
    if(strcmp(minorVer, "Unknown") != 0)
        return minorVer;
    
    char line[128] = "\0";
    FILE *fp = fopen("/etc/.kylin-osinfo", "rt");
    if (!fp)
        return "Unknown";

    char *val = NULL;

    while (fgets(line, 128, fp))
    {
        if(strstr(line, "MinorVersion"))
        {
            val = package_lookup(line, "MinorVersion");
            break;
        }
    }

    fclose(fp);

    if(val)
    {
        strskipspace(val);
        strncpy(minorVersion, val, sizeof(minorVersion) - 1);
        minorVer = minorVersion;
    }

    return minorVer;
}

const char *kdk_system_get_minor_version()
{
    const char *minor_ver;
    const char *major = kdk_system_get_major_version();
    if(major && strcmp(major, "V10") == 0)
    {
        minor_ver = "v10";
        return minor_ver;
    }

    minor_ver = kdk_system_tag_minor_version_verify();

    if(strcmp(minor_ver, "Unknown") == 0)
    {
        minor_ver = kdk_package_ver_info(false);
    }

    if(strcmp(minor_ver, "Unknown") == 0)
    {
        minor_ver = kdk_file_verify();
    }
    return minor_ver;
}

static const char *alias = "Unknown";

const char *kdk_system_get_version_alias()
{
    const char *minor = kdk_system_get_minor_version();

    if(strcmp(minor, "v10") == 0)
    {
        alias = "acacia";
    }
    else if(strcmp(minor, "2107") == 0)
    {
        alias = "balsa";
    }
    else if(strcmp(minor, "2107-1020") == 0)
    {
        alias = "banyan";
    }
    else if(strcmp(minor, "2107-1228") == 0)
    {
        alias = "baobab";
    }
    else if(strcmp(minor, "2203") == 0)
    {
        alias = "cedar";
    }
    else if(strcmp(minor, "2303") == 0)
    {
        alias = "durian";
    }
    else if(strcmp(minor, "2303-update2") == 0)
    {
        alias = "ebony";
    }
    else if(strcmp(minor, "2403") == 0)
    {
        alias = "fir";
    }
    else if(strcmp(minor, "2403-update1") == 0)
    {
        alias = "gum";
    }
    else if(strcmp(minor, "2503") == 0)
    {
        alias = "ilex";
    }
    else if(strcmp(minor, "2603-preview") == 0)
    {
        alias = "kauri";
    }

    return alias;
}

static char serialInfo[128] = "\0";
static const char *kdk_system_tag_serial_name_verify()
{
    if(strcmp(serName, "Unknown") != 0)
        return serName;
    
    char line[128] = "\0";
    FILE *fp = fopen("/etc/.kylin-osinfo", "rt");
    if (!fp)
        return "Unknown";

    char *val = NULL;

    while (fgets(line, 128, fp))
    {
        if(strstr(line, "SerialInfo"))
        {
            val = package_lookup(line, "SerialInfo");
            break;
        }
    }
    fclose(fp);

    if(val)
    {
        strskipspace(val);
        strncpy(serialInfo, val, sizeof(serialInfo) - 1);
        serName = serialInfo;
    }

    return serName;
}

const char *kdk_system_get_serial_name()
{
    const char *serial_name = "\0";

    const char *major = kdk_system_get_major_version();
    if(major && strcmp(major, "V10") == 0)
    {
        serial_name = "xc";
        return serial_name;
    }
    
    serial_name = kdk_system_tag_serial_name_verify();

    if(strcmp(serial_name, "Unknown") == 0)
    {
        serial_name = kdk_package_ver_info(true);
    }
    return serial_name;
}

const kdk_production_inner_info kdk_system_get_production_inner_version()
{
    kdk_production_inner_info info;
    
    info.production_line = kdk_system_get_production_line();
    info.major_ver = kdk_system_get_major_version();
    const char *minor = kdk_system_get_minor_version();
    info.minor_ver = minor;
    info.alias = kdk_system_get_version_alias();
    info.serial_name = kdk_system_get_serial_name();
    return info;
}

static void extract_value(const char *line, const char *key, char *output, size_t output_size) {
    if (strstr(line, key)) {
        const char *colon = strchr(line, ':');
        if (colon) {
            strncpy(output, colon + 1, output_size - 1);
            output[output_size - 1] = '\0'; // 确保字符串结尾
            strstripspace(output); // 去除空格
        }
    }
}

static kdk_custom_info kdk_system_tag_custom_version_verify()
{
    kdk_custom_info info;
    char line[128];
    FILE *fp = fopen("/etc/.kylin-osinfo", "rt");
    if (!fp)
    {
        strncpy(info.version_type, "Unknown", sizeof(info.version_type) - 1);
        strncpy(info.custom_info, "Unknown", sizeof(info.custom_info) - 1);
        strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
        return info;
    }

    char type[128] = {0};
    char project[128] = {0};

    while (fgets(line, sizeof(line), fp))
    {
        extract_value(line, "CustomType", type, sizeof(type));
        extract_value(line, "ProjectInfo", project, sizeof(project));

        if (*type && *project)  // 两个都找到就停止
            break;
    }
    fclose(fp);

    if (*type)
    {
        if (*project)
        {
            strncpy(info.version_type, "是", sizeof(info.version_type) - 1);
            strncpy(info.custom_info, project, sizeof(info.custom_info) - 1);
            strncpy(info.custom_type, type, sizeof(info.custom_type) - 1);
        }
        else
        {
            strncpy(info.version_type, "否", sizeof(info.version_type) - 1);
            strncpy(info.custom_info, "Unknown", sizeof(info.custom_info) - 1);
            strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
        }
    }
    else
    {
        strncpy(info.version_type, "Unknown", sizeof(info.version_type) - 1);
        strncpy(info.custom_info, "Unknown", sizeof(info.custom_info) - 1);
        strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
    }
    return info;
}

static kdk_custom_info kdk_buildid_ver_info()
{
    kdk_custom_info info;
    FILE *fp = NULL, *build_fp = NULL;
    
    char kylin_build[512] = "\0";
    char line[1024] = "\0";
    char *kybuildid = NULL;
    char *kybuild = NULL;
    build_fp = fopen("/etc/kylin-build", "r");
    if(!build_fp)
    {
        goto out;
    }
    while (fgets(kylin_build, sizeof(kylin_build), build_fp))
    {
        if(strstr(kylin_build, "buildid"))
        {
            kybuildid = package_lookup(kylin_build, "buildid");
        }
        if(strstr(kylin_build, "Build"))
        {
            kybuild = package_lookup(kylin_build, "Build");
        }
    }
    fclose(build_fp);

    char buildid[64] = "\0";
    char version_type[128] = "\0";
    char custom_type[128] = "\0";
    bool is_custom = false;
    char version_tmp[128] = "\0";
    char custom_tmp[128] = "\0";
    bool is_version_type = false;
    bool is_custom_type = false;

    if(!kybuildid)
    {
        if(!kybuild)
        {
            goto out;
        }
        
        fp = fopen("/var/lib/kysdk/kysdk-system/kysysinfo/customizedDateInfo", "r");
        if(!fp)
        {
            goto out;
        }
        // 标志是否为第一次迭代
        int first_iteration = 1;

        while (fgets(line, sizeof(line), fp))
        {
            sscanf(line, "%s %s %s", buildid, version_type, custom_type);
            if(strcmp(buildid, kybuild) == 0)
            {
                is_custom = true;
                if(!first_iteration)
                {
                    if(strncmp(version_type, version_tmp, strnlen(version_type, STRNLEN_MAX)) != 0)
                    {
                        is_version_type = true;
                    }
                    if(strncmp(custom_type, custom_tmp, strnlen(custom_type, STRNLEN_MAX)) != 0)
                    {
                        is_custom_type = true;
                    }
                }
                first_iteration = 0;
                strncpy(version_tmp, version_type, sizeof(version_tmp) - 1);
                strncpy(custom_tmp, custom_type, sizeof(custom_tmp) - 1);
            }
        }
        fclose(fp);

        if(is_custom)
        {
            if(!is_version_type)
            {
                if(strcmp(version_tmp, "定制版本") == 0)
                {
                    strncpy(info.version_type, "是", sizeof(info.version_type) - 1);
                }
                else{
                    strncpy(info.version_type, "否", sizeof(info.version_type) - 1);
                }

                if(!is_custom_type)
                {
                    if(strcmp(custom_tmp, "不涉及") == 0 || strcmp(custom_tmp, "无") == 0)
                    {
                        strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
                    }
                    else{
                        strncpy(info.custom_type, custom_tmp, sizeof(info.custom_type) - 1);
                    }
                }
                else
                {
                    strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
                }
            }
            else
            {
                strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
                strncpy(info.version_type, "Unknown", sizeof(info.version_type) - 1);
            }
        }
        else{
            goto out;
        }
        return info;
    }
    
    fp = fopen("/var/lib/kysdk/kysdk-system/kysysinfo/customizedInfo", "r");
    if(!fp)
    {
        goto out;
    }

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s %s %s", buildid, version_type, custom_type);
        if(strcmp(buildid, kybuildid) == 0)
        {
            is_custom = true;
            if(strcmp(version_type, "定制版本") == 0)
            {
                strncpy(info.version_type, "是", sizeof(info.version_type) - 1);
            }
            else{
                strncpy(info.version_type, "否", sizeof(info.version_type) - 1);
            }
            
            if(strcmp(custom_type, "不涉及") == 0 || strcmp(custom_type, "无") == 0)
            {
                strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
            }
            else{
                strncpy(info.custom_type, custom_type, sizeof(info.custom_type) - 1);
            }
        }
    }
    fclose(fp);

    if(!is_custom)
    {
        int id = atoi(buildid);
        int kyid = atoi(kybuildid);
        if(kyid > id)
        {
            strncpy(info.custom_type, "未收录", sizeof(info.custom_type) - 1);
            strncpy(info.version_type, "未收录", sizeof(info.version_type) - 1);
        }
        else
            goto out;
    }
    strncpy(info.custom_info, "Unknown", sizeof(info.custom_info) - 1);
    return info;

out:
    strncpy(info.custom_type, "Unknown", sizeof(info.custom_type) - 1);
    strncpy(info.version_type, "Unknown", sizeof(info.version_type) - 1);
    strncpy(info.custom_info, "Unknown", sizeof(info.custom_info) - 1);
    return info;
}

const kdk_custom_info kdk_system_get_custom_version()
{
    kdk_custom_info info;

    info = kdk_system_tag_custom_version_verify();
    if(strcmp(info.custom_info, "Unknown") != 0 || strcmp(info.custom_type, "Unknown") != 0 || strcmp(info.version_type, "Unknown") != 0)
    {
        return info;
    }

    info = kdk_buildid_ver_info();
    return info;
}

void kdk_system_set_service_shutdown(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    char service_path[256];

    /* 参数检查 */
    if (!service) {
        fprintf(stderr, "Invalid service name\n");
        return;
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    /* 构造服务路径 */
    snprintf(service_path, sizeof(service_path), "org.freedesktop.systemd1.unit.%s", service);

    /* 调用systemd的StopUnit方法 */
    r = sd_bus_call_method(bus,
                          "org.freedesktop.systemd1",           /* 服务 */
                          "/org/freedesktop/systemd1",          /* 对象路径 */
                          "org.freedesktop.systemd1.Manager",   /* 接口 */
                          "StopUnit",                           /* 方法 */
                          &error,                               /* 错误对象 */
                          &m,                                   /* 返回消息 */
                          "ss",                                 /* 参数签名 */
                          service,                              /* 服务名称 */
                          "replace");                           /* 替换模式 */

    if (r < 0) {
        fprintf(stderr, "Failed to stop service: %s\n", error.message);
        goto finish;
    }

finish:
    /* 清理资源 */
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
}

void kdk_system_set_service_restart(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    char service_path[256];

    /* 参数检查 */
    if (!service) {
        fprintf(stderr, "Invalid service name\n");
        return;
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    /* 构造服务路径 */
    snprintf(service_path, sizeof(service_path), "org.freedesktop.systemd1.unit.%s", service);

    /* 调用systemd的RestartUnit方法 */
    r = sd_bus_call_method(bus,
                          "org.freedesktop.systemd1",           /* 服务 */
                          "/org/freedesktop/systemd1",          /* 对象路径 */
                          "org.freedesktop.systemd1.Manager",   /* 接口 */
                          "RestartUnit",                        /* 方法 */
                          &error,                               /* 错误对象 */
                          &m,                                   /* 返回消息 */
                          "ss",                                 /* 参数签名 */
                          service,                              /* 服务名称 */
                          "replace");                           /* 替换模式 */

    if (r < 0) {
        fprintf(stderr, "Failed to restart service: %s\n", error.message);
        goto finish;
    }

finish:
    /* 清理资源 */
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
}

void kdk_system_set_service_reload(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    int r;
    char service_path[256];

    /* 参数检查 */
    if (!service) {
        fprintf(stderr, "Invalid service name\n");
        return;
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    /* 构造服务路径 */
    snprintf(service_path, sizeof(service_path), "org.freedesktop.systemd1.unit.%s", service);

    /* 调用systemd的ReloadUnit方法 */
    r = sd_bus_call_method(bus,
                          "org.freedesktop.systemd1",           /* 服务 */
                          "/org/freedesktop/systemd1",          /* 对象路径 */
                          "org.freedesktop.systemd1.Manager",   /* 接口 */
                          "ReloadUnit",                         /* 方法 */
                          &error,                               /* 错误对象 */
                          &m,                                   /* 返回消息 */
                          "ss",                                 /* 参数签名 */
                          service,                              /* 服务名称 */
                          "replace");                           /* 替换模式 */

    if (r < 0) {
        fprintf(stderr, "Failed to reload service: %s\n", error.message);
        goto finish;
    }

finish:
    /* 清理资源 */
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
}

bool kdk_system_is_service_active(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    const char *active_state = NULL;
    int r;
    bool is_active = false;

    /* 参数检查 */
    if (!service) {
        fprintf(stderr, "Invalid service name\n");
        return false;
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    /* 获取服务的ActiveState属性 */
    r = sd_bus_call_method(bus,
                                 "org.freedesktop.systemd1",          /* 服务 */
                                 "/org/freedesktop/systemd1",         /* 对象路径 */
                                 "org.freedesktop.systemd1.Manager",  /* 接口 */
                                 "LoadUnit",                          /* 方法 */
                                 &error,                              /* 错误对象 */
                                 &m, 
                                 "s",                                /* 返回消息 */
                                 service);                            /* 服务名称 */

    if (r < 0) {
        fprintf(stderr, "Failed to get unit path: %s\n", error.message);
        goto finish;
    }

    const char *unit_path = NULL;
    r = sd_bus_message_read(m, "o", &unit_path);
    if (r < 0) {
        fprintf(stderr, "Failed to read unit path: %s\n", strerror(-r));
        goto finish;
    }
    klog_info("unit_path = %s\n",unit_path);

    /* 释放之前的消息对象 */
    sd_bus_message_unref(m);
    m = NULL;

    /* 获取服务状态 */
    r = sd_bus_get_property_string(bus,
                                 "org.freedesktop.systemd1",          /* 服务 */
                                 unit_path,                           /* 对象路径 */
                                 "org.freedesktop.systemd1.Unit",     /* 接口 */
                                 "ActiveState",                       /* 属性 */
                                 &error,                              /* 错误对象 */
                                 &active_state);                      /* 返回值 */

    if (r < 0) {
        fprintf(stderr, "Failed to get service status: %s\n", error.message);
        goto finish;
    }

    /* 检查状态是否为"active" */
    is_active = (active_state != NULL && strcmp(active_state, "active") == 0);
    klog_info("active_state = %s\n",active_state);

finish:
    /* 清理资源 */
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
    
    return is_active;
}

int kdk_system_set_service_startup(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus_message *reply = NULL;
    sd_bus *bus = NULL;
    int r;
    int ret = -1;
    char service_unit[256];

    /* 参数检查 */
    if (!service)
    {
        fprintf(stderr, "Invalid service name\n");
        return -1;
    }

    /* 检查root权限 */
    if (geteuid() != 0)
    {
        fprintf(stderr, "This operation requires root privileges\n");
        return -1;
    }

    /* 规范化服务名称 */
    if (strstr(service, ".service") == NULL)
    {
        snprintf(service_unit, sizeof(service_unit), "%s.service", service);
    }
    else
    {
        snprintf(service_unit, sizeof(service_unit), "%s", service);
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    /* 创建方法调用消息 */
    r = sd_bus_message_new_method_call(bus,
                                       &m,
                                       "org.freedesktop.systemd1",         /* 服务 */
                                       "/org/freedesktop/systemd1",        /* 对象路径 */
                                       "org.freedesktop.systemd1.Manager", /* 接口 */
                                       "EnableUnitFiles");                 /* 方法 */
    if (r < 0)
    {
        fprintf(stderr, "Failed to create method call: %s\n", strerror(-r));
        goto finish;
    }

    /* 打开数组容器 */
    r = sd_bus_message_open_container(m, 'a', "s");
    if (r < 0)
    {
        fprintf(stderr, "Failed to open array container: %s\n", strerror(-r));
        goto finish;
    }

    /* 添加服务名称到数组 */
    r = sd_bus_message_append_basic(m, 's', service_unit);
    if (r < 0)
    {
        fprintf(stderr, "Failed to append service name: %s\n", strerror(-r));
        goto finish;
    }

    /* 关闭数组容器 */
    r = sd_bus_message_close_container(m);
    if (r < 0)
    {
        fprintf(stderr, "Failed to close array container: %s\n", strerror(-r));
        goto finish;
    }

    /* 添加剩余参数：runtime和force */
    r = sd_bus_message_append(m, "bb", false, false); /* runtime=false, force=false */
    if (r < 0)
    {
        fprintf(stderr, "Failed to append boolean arguments: %s\n", strerror(-r));
        goto finish;
    }

    /* 发送消息并等待回复 */
    r = sd_bus_call(bus, m, 0, &error, &reply);
    if (r < 0)
    {
        fprintf(stderr, "Failed to enable service: %s\n", error.message);
        goto finish;
    }

    /* 重新加载 systemd 配置 */
    sd_bus_message_unref(m);
    m = NULL;
    r = sd_bus_message_new_method_call(bus,
                                       &m,
                                       "org.freedesktop.systemd1",
                                       "/org/freedesktop/systemd1",
                                       "org.freedesktop.systemd1.Manager",
                                       "Reload");
    if (r < 0)
    {
        fprintf(stderr, "Failed to create reload message: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call(bus, m, 0, &error, NULL);
    if (r < 0)
    {
        fprintf(stderr, "Failed to reload systemd: %s\n", error.message);
        goto finish;
    }

    ret = 0; /* 成功 */

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return ret;
}

int kdk_system_check_service_startup(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus_message *reply = NULL;
    sd_bus *bus = NULL;
    int r;
    int ret = -1;
    char service_unit[256];

    /* 参数检查 */
    if (!service)
    {
        fprintf(stderr, "Invalid service name\n");
        return -1;
    }

    /* 规范化服务名称 */
    if (strstr(service, ".service") == NULL)
    {
        snprintf(service_unit, sizeof(service_unit), "%s.service", service);
    }
    else
    {
        snprintf(service_unit, sizeof(service_unit), "%s", service);
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    /* 创建方法调用消息 - 获取服务状态 */
    r = sd_bus_message_new_method_call(bus,
                                       &m,
                                       "org.freedesktop.systemd1",         /* 服务 */
                                       "/org/freedesktop/systemd1",        /* 对象路径 */
                                       "org.freedesktop.systemd1.Manager", /* 接口 */
                                       "GetUnitFileState");                /* 方法 */
    if (r < 0)
    {
        fprintf(stderr, "Failed to create method call: %s\n", strerror(-r));
        goto finish;
    }

    /* 添加服务名称参数 */
    r = sd_bus_message_append(m, "s", service_unit);
    if (r < 0)
    {
        fprintf(stderr, "Failed to append arguments: %s\n", strerror(-r));
        goto finish;
    }

    /* 发送消息并等待回复 */
    r = sd_bus_call(bus, m, 0, &error, &reply);
    if (r < 0)
    {
        /* 特殊处理：如果服务不存在，返回0（不支持自启动） */
        if (sd_bus_error_has_name(&error, "org.freedesktop.systemd1.NoSuchUnit"))
        {
            ret = 0;
            goto finish;
        }
        fprintf(stderr, "Failed to get unit state: %s\n", error.message);
        goto finish;
    }

    /* 读取返回的状态字符串 */
    const char *state;
    r = sd_bus_message_read(reply, "s", &state);
    if (r < 0)
    {
        fprintf(stderr, "Failed to parse reply: %s\n", strerror(-r));
        goto finish;
    }

    /* 解析状态字符串
     * enabled: 已启用
     * enabled-runtime: 运行时启用
     * linked: 链接启用
     * linked-runtime: 运行时链接
     * static: 静态启用
     * indirect: 间接启用
     */
    if (strcmp(state, "enabled") == 0 ||
        strcmp(state, "enabled-runtime") == 0 ||
        strcmp(state, "linked") == 0 ||
        strcmp(state, "linked-runtime") == 0 ||
        strcmp(state, "static") == 0 ||
        strcmp(state, "indirect") == 0)
    {
        ret = 1; /* 支持自启动 */
    }
    else if (strcmp(state, "disabled") == 0 ||
             strcmp(state, "masked") == 0 ||
             strcmp(state, "masked-runtime") == 0)
    {
        ret = 0; /* 不支持自启动 */
    }
    else
    {
        /* 未知状态，默认为不支持 */
        ret = 0;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return ret;
}

int kdk_system_disable_service_startup(char *service)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    char service_path[256];
    int r;

    /* 参数检查 */
    if (!service) {
        return -1;
    }

    /* 构造完整的服务名 */
    if (strstr(service, ".service") == NULL) {
        snprintf(service_path, sizeof(service_path), "%s.service", service);
    } else {
        strncpy(service_path, service, sizeof(service_path) - 1);
    }

    /* 连接到系统总线 */
    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    /* 创建方法调用消息 */
    r = sd_bus_message_new_method_call(bus,
                                      &m,
                                      "org.freedesktop.systemd1",           /* 目标服务 */
                                      "/org/freedesktop/systemd1",          /* 对象路径 */
                                      "org.freedesktop.systemd1.Manager",   /* 接口 */
                                      "DisableUnitFiles");                  /* 方法 */
    if (r < 0) {
        fprintf(stderr, "Failed to create method call: %s\n", strerror(-r));
        goto cleanup;
    }

    /* 构造参数 */
    r = sd_bus_message_open_container(m, 'a', "s");
    if (r < 0) {
        fprintf(stderr, "Failed to open container: %s\n", strerror(-r));
        goto cleanup;
    }

    r = sd_bus_message_append_basic(m, 's', service_path);
    if (r < 0) {
        fprintf(stderr, "Failed to append service name: %s\n", strerror(-r));
        goto cleanup;
    }

    r = sd_bus_message_close_container(m);
    if (r < 0) {
        fprintf(stderr, "Failed to close container: %s\n", strerror(-r));
        goto cleanup;
    }

    /* 添加runtime参数(false)和force参数(false) */
    r = sd_bus_message_append(m, "b", false);
    if (r < 0) {
        fprintf(stderr, "Failed to append parameters: %s\n", strerror(-r));
        goto cleanup;
    }

    /* 发送消息并等待响应 */
    r = sd_bus_call(bus, m, 0, &error, NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to call method: %s\n", error.message);
        goto cleanup;
    }

    /* 执行daemon-reload以应用更改 */
    sd_bus_message_unref(m);
    m = NULL;
    
    r = sd_bus_message_new_method_call(bus,
                                      &m,
                                      "org.freedesktop.systemd1",
                                      "/org/freedesktop/systemd1",
                                      "org.freedesktop.systemd1.Manager",
                                      "Reload");
    if (r < 0) {
        fprintf(stderr, "Failed to create reload method call: %s\n", strerror(-r));
        goto cleanup;
    }

    r = sd_bus_call(bus, m, 0, &error, NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to reload daemon: %s\n", error.message);
        goto cleanup;
    }

cleanup:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return (r < 0) ? -1 : 0;
}
