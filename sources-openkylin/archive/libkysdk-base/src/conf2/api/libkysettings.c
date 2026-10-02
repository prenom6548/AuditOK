/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <pwd.h>

#include "libkysettings.h"
#include "ksettingsschema.h"

#ifndef K_MAX_PATH
#define K_MAX_PATH 4096
#endif // K_MAX_PATH

static GMainLoop *dbus_signal_loop = NULL; // glib主循环，用于处理dbus消息事件，主循环不开启无法接收dbus信号
static volatile guint ref_count = 0;                // 记录dbus_signal_loop的引用计数
static pthread_mutex_t mutex;                       // 互斥锁

/*
 *句柄继承GObject
 *继承GObject，以使用glib框架的信号机制
 */
typedef struct _KSettingsPrivate
{
    GMainContext *main_context;
    KSettingsSchema *schema;
    GDBusConnection *conn;
    guint sub_id[2];
} KSettingsPrivate;

struct _KSettings
{
    GObject parent_instance;
    KSettingsPrivate *priv;
};

typedef struct _KSettingsClass
{
    GObjectClass parent_class;

    /* Signals */
    void (*writable_changed)(KSettings *settings,
                             const gchar *key);
    void (*changed)(KSettings *settings,
                    const gchar *key);
    void (*reload)(KSettings *settings);
    gboolean (*writable_change_event)(KSettings *settings,
                                      GQuark key);
    gboolean (*change_event)(KSettings *settings,
                             const GQuark *keys,
                             gint n_keys);

    gpointer padding[20];
} KSettingsClass;

enum
{
    SIGNAL_CHANGED,
    SIGNAL_RELOAD,
    N_SIGNALS
};

static guint k_settings_signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE(KSettings, k_settings, G_TYPE_OBJECT)

#define K_TYPE_SETTINGS (k_settings_get_type())
#define K_SETTINGS(inst) (G_TYPE_CHECK_INSTANCE_CAST((inst), \
                                                     K_TYPE_SETTINGS, KSettings))
#define K_SETTINGS_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), \
                                                         K_TYPE_SETTINGS, KSettingsClass))
#define K_IS_SETTINGS(inst) (G_TYPE_CHECK_INSTANCE_TYPE((inst), K_TYPE_SETTINGS))
#define K_IS_SETTINGS_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE((class), K_TYPE_SETTINGS))
#define K_SETTINGS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS((inst), \
                                                              K_TYPE_SETTINGS, KSettingsClass))

static void k_settings_finalize(GObject *object)
{
    KSettings *ksettings = K_SETTINGS(object);

    g_main_context_unref(ksettings->priv->main_context);
    //    kdk_conf2_schema_destroy(ksettings->priv->schema);
    if (ksettings->priv->conn)
    {
        g_dbus_connection_signal_unsubscribe(ksettings->priv->conn, ksettings->priv->sub_id[SIGNAL_CHANGED]);
        g_dbus_connection_signal_unsubscribe(ksettings->priv->conn, ksettings->priv->sub_id[SIGNAL_RELOAD]);
        g_object_unref(ksettings->priv->conn);
    }

    G_OBJECT_CLASS(k_settings_parent_class)->finalize(object);
}

static void k_settings_init(KSettings *settings)
{
    settings->priv = k_settings_get_instance_private(settings);
    settings->priv->main_context = g_main_context_ref_thread_default();
}

static void k_settings_class_init(KSettingsClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->finalize = k_settings_finalize;

    // 创建一个新的信号
    k_settings_signals[SIGNAL_CHANGED] =
        g_signal_new("changed",
                     K_TYPE_SETTINGS,
                     G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                     G_STRUCT_OFFSET(KSettingsClass, changed),
                     NULL,
                     NULL,
                     NULL,
                     G_TYPE_NONE,
                     1, G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    k_settings_signals[SIGNAL_RELOAD] =
        g_signal_new("reload",
                     K_TYPE_SETTINGS,
                     G_SIGNAL_RUN_LAST,
                     G_STRUCT_OFFSET(KSettingsClass, reload),
                     NULL,
                     NULL,
                     NULL,
                     G_TYPE_NONE,
                     0);
}

/**
 * @brief _g_main_loop_thread 开启main_loop获取消息循环
 * @param data
 * @return
 */
static void *_g_main_loop_thread(void *data)
{
    // 启动主循环
    g_main_loop_run(dbus_signal_loop);
}

/**
 * @brief _on_key_changed DBus服务中key_changed信号响应函数
 * @param connection
 * @param sender_name
 * @param object_path
 * @param interface_name
 * @param signal_name
 * @param parameters
 * @param user_data
 */
static void _on_key_changed(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data)
{
    pthread_mutex_lock(&mutex);

    KSettings *ksettings = (KSettings *)user_data;

    char *id, *version, *key;
    g_variant_get(parameters, "(sss)", &id, &version, &key);

    char *schema_id = kdk_conf2_schema_get_id(ksettings->priv->schema);
    char *schema_version = kdk_conf2_schema_get_version(ksettings->priv->schema);
    KSettingsSchema *schema = kdk_conf2_schema_reload(id, version);

    gboolean emit_siganl = FALSE;
    if ((0 == strcmp(schema_id, id)) && (0 == strcmp(schema_version, version)))
    {
        ksettings->priv->schema = schema;
        emit_siganl = TRUE;
    }
    g_free(schema_id);
    g_free(schema_version);

    pthread_mutex_unlock(&mutex);

    if (emit_siganl)
    {
        g_signal_emit(ksettings, k_settings_signals[SIGNAL_CHANGED], g_quark_from_string(key), key);
        syslog(LOG_INFO, "[kysdk-conf2] %s changed signal\n", key);
    }
}

/**
 * @brief _on_updated   DBus服务中update信号的响应函数
 * @param connection
 * @param sender_name
 * @param object_path
 * @param interface_name
 * @param signal_name
 * @param parameters
 * @param user_data
 */
static void _on_updated(GDBusConnection *connection,
                        const gchar *sender_name,
                        const gchar *object_path,
                        const gchar *interface_name,
                        const gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
    pthread_mutex_lock(&mutex);
    kdk_conf2_schema_update_schemas_table();
    pthread_mutex_unlock(&mutex);

    KSettings *ksettings = (KSettings *)user_data;
    g_signal_emit(ksettings, k_settings_signals[SIGNAL_RELOAD], 0);
}

/**
 * @brief _type_check   检查vaule的值是否符合key的数据类型标签
 * @param ksettings 句柄
 * @param key   键名
 * @param value 目标值的字符串。
 * @return 布尔类型 1：True 0：False
 */
static int _type_check(KSettings *ksettings, const char *key, const char *value)
{
    const gchar *endptr = NULL;
    GError *error = NULL;
    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    if (NULL == key_hash)
        return FALSE;

    char *type = kdk_conf2_schema_key_value_type(key_hash);
    if ((0 != strcmp(type, "enum")) && (0 != strcmp(type, "s")))
    {
        GVariant *variant = g_variant_parse(G_VARIANT_TYPE(type), value, NULL, &endptr, &error);
        if (NULL == variant)
        {
            syslog(LOG_ERR, "[kysdk-conf2] % -> parser error : %s\n", __func__, error->message);
            g_error_free(error);
            g_free(type);
            return FALSE;
        }
        g_variant_unref(variant);
    }
    g_free(type);
    return TRUE;
}

/**
 * @brief _call_method 调用dbus接口
 * @param connection   dbus链接
 * @param method    接口名 set/reset/reset_recursively/reload
 * @param id        配置id，可为NULL，根据method传参。
 * @param version   配置版本，可为NULL，根据method传参。
 * @param key       键名，可为NULL，根据method传参。
 * @param value     键值，可为NULL，根据method传参。
 * @return  boolean类型 成功返回1 失败返回0
 */
static int _call_method(const char *method, const char *id, const char *version, const char *key, const char *value)
{
    GError *error = NULL;
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error != NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> Connection Error: %s\n", __func__, error->message);
        g_error_free(error);
        return FALSE;
    }

    GDBusProxy *proxy;
    GVariant *result;
    int response = FALSE;

    // 创建代理对象
    proxy = g_dbus_proxy_new_sync(connection,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  NULL,
                                  "com.kylin.kysdk.conf2",
                                  "/com/kylin/kysdk/conf2",
                                  "com.kylin.kysdk.conf2",
                                  NULL,
                                  &error);
    if (error != NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> Proxy Error: %s\n", __func__, error->message);
        g_error_free(error);
        return response;
    }

    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s -> Get user name failed! use root config\n", __func__);
        strncpy(user_name, "root", sizeof(user_name));
    }
    else
    {
        strncpy(user_name, pw->pw_name, sizeof(user_name));
        endpwent();
    }

    // 调用代理对象的方法
    GVariant *variant = NULL;
    if (0 == strcmp(method, "set"))
        variant = g_variant_new("(sssss)", user_name, id, version, key, value);
    if (0 == strcmp(method, "reset"))
        variant = g_variant_new("(ssss)", user_name, id, version, key);
    if (0 == strcmp(method, "reset_recursively"))
        variant = g_variant_new("(sss)", user_name, id, version);

    result = g_dbus_proxy_call_sync(proxy,
                                    method,
                                    variant,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
    if (error != NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> %s call fialed: %s\n", __func__, method, error->message);
        g_error_free(error);
        return response;
    }

    // 处理方法调用的返回值
    g_variant_get(result, "(i)", &response);

    // 释放资源
    g_variant_unref(result);
    g_object_unref(proxy);
    g_object_unref(connection);
    return response;
}

static gboolean _is_just_read()
{
    char process_name[256];
    FILE *fp = fopen("/proc/self/comm", "r");
    if (fp != NULL)
    {
        fgets(process_name, sizeof(process_name), fp);
        fclose(fp);
        // 去除末尾换行符
        process_name[strcspn(process_name, "\n")] = 0;
        if (0 == strcmp(process_name, "dbus-daemon") ||
            0 == strcmp(process_name, "lightdm") ||
            0 == strcmp(process_name, "systemd-logind"))
        {
            // syslog(LOG_DEBUG, "kysdk-conf2 -> %s: %s is just read config. Dont start loop", __func__, process_name);
            return TRUE;
        }
    }
    return FALSE;
}

KSettings *kdk_conf2_new(const char *id, const char *version)
{
    if (NULL == id)
    {
        syslog(LOG_INFO, "[kysdk-conf2] id is nullptr\n");
        return NULL;
    }

    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();

    if (NULL == schemas_table)
    {
        syslog(LOG_INFO, "[kysdk-conf2] load user.db failed\n");
        return NULL;
    }

    KSettingsSchema *schema = kdk_conf2_schema_table_lookup(id, version);
    if (NULL == schema)
    {
        syslog(LOG_ERR, "[kysdk-conf2] invalied schema id\n");
        return NULL;
    }

    KSettings *ksettings = g_object_new(K_TYPE_SETTINGS, NULL);
    if (!K_IS_SETTINGS(ksettings))
    {
        syslog(LOG_INFO, "[kysdk-conf2] init KSettings failed\n");
        return NULL;
    }

    ksettings->priv->schema = schema;

    if (!_is_just_read())
    {
        uid_t uid = getuid(); // 获取当前进程的用户ID
        char user_name[128] = {0};
        struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
        if (NULL == pw)
        {
            syslog(LOG_INFO, "[kysdk-conf2] %s ->  :Get user name failed! use root config\n", __func__);
            strncpy(user_name, "root", sizeof(user_name));
        }
        else
        {
            strncpy(user_name, pw->pw_name, sizeof(user_name));
            endpwent();
        }

        // 创建dbus服务的链接，用于调用接口及接收信号
        GError *error = NULL;
        if (0 == strcmp(user_name, "root"))
            ksettings->priv->conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        else
            ksettings->priv->conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
        if (error != NULL)
        {
            syslog(LOG_INFO, "[kysdk-conf2] Connection Error: %s\n", error->message);
            g_object_unref(ksettings);
            ksettings = NULL;
            g_error_free(error);
            return NULL;
        }

        // 监听key_changed信号
        ksettings->priv->sub_id[0] = g_dbus_connection_signal_subscribe(ksettings->priv->conn,
                                                                        "com.kylin.kysdk.conf2",  // 消息来源 (NULL 表示任意)
                                                                        "com.kylin.kysdk.conf2",  // 接口名称
                                                                        "key_changed",            // 信号名称
                                                                        "/com/kylin/kysdk/conf2", // 对象路径 (NULL 表示任意)
                                                                        NULL,
                                                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                                                        _on_key_changed,
                                                                        ksettings,
                                                                        NULL);
        // 监听重载信号
        ksettings->priv->sub_id[1] = g_dbus_connection_signal_subscribe(ksettings->priv->conn,
                                                                        "com.kylin.kysdk.conf2",  // 消息来源 (NULL 表示任意)
                                                                        "com.kylin.kysdk.conf2",  // 接口名称
                                                                        "updated",                // 信号名称
                                                                        "/com/kylin/kysdk/conf2", // 对象路径 (NULL 表示任意)
                                                                        NULL,
                                                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                                                        _on_updated,
                                                                        ksettings,
                                                                        NULL);

        // 创建glib的事件处理循环，单独起线程。
        // 如果循环已经启动，增加dbus_signal_loop的引用计数
        if (NULL == dbus_signal_loop)
        {
            dbus_signal_loop = g_main_loop_new(NULL, FALSE);
        }

        if (!g_main_loop_is_running(dbus_signal_loop))
        {
            pthread_t thread;
            pthread_create(&thread, NULL, _g_main_loop_thread, dbus_signal_loop);
        }
        ref_count++;
    }
    return ksettings;
}

void kdk_conf2_ksettings_destroy(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return;

    g_object_unref(ksettings);

    if (!_is_just_read())
    {
        ref_count -= 1;
        if (0 == ref_count)
        {
            if (g_main_loop_is_running(dbus_signal_loop))
                g_main_loop_quit(dbus_signal_loop);

            g_hash_table_destroy(schemas_table);
            schemas_table = NULL;
        }
    }
}

char *kdk_conf2_get_id(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;
    return kdk_conf2_schema_get_id(ksettings->priv->schema);
}

char *kdk_conf2_get_version(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;
    return strdup(kdk_conf2_schema_get_version(ksettings->priv->schema));
}

KSettings *kdk_conf2_get_child(KSettings *ksettings, const char *name)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == name)
        return NULL;

    if (!kdk_conf2_schema_has_child(ksettings->priv->schema, name))
        return NULL;
    char child_id[PATH_MAX];
    char *id = kdk_conf2_schema_get_id(ksettings->priv->schema);
    snprintf(child_id, sizeof(child_id), "%s.%s", id, name);
    g_free(id);
    KSettings *result = kdk_conf2_new(child_id, kdk_conf2_schema_get_version(ksettings->priv->schema));
    return result;
}

char **kdk_conf2_list_children(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;

    return kdk_conf2_schema_list_children(ksettings->priv->schema);
}

char **kdk_conf2_list_keys(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;

    return kdk_conf2_schema_list_keys(ksettings->priv->schema);
}

char *kdk_conf2_get_range(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    return kdk_conf2_schema_key_get_range(key_hash);
}

int kdk_conf2_range_check(KSettings *ksettings, const char *key, const char *value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return FALSE;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    if (NULL == key_hash)
        return FALSE;
    return kdk_conf2_schema_key_range_check(key_hash, value);
}

int kdk_conf2_set_value(KSettings *ksettings, const char *key, const char *value)
{
    pthread_mutex_lock(&mutex);

    if (!K_IS_SETTINGS(ksettings) || NULL == key)
    {
        pthread_mutex_unlock(&mutex);
        syslog(LOG_INFO, "[kysdk-conf2] %s -> assert ksettings is KSettings failed\n", __func__);
        return FALSE;
    }
    syslog(LOG_INFO, "[kysdk-conf2] %s -> Set the key '%s' of %s to %s\n", __func__, key, ksettings->priv->schema->id, value);

    if (0 == kdk_conf2_is_writable(ksettings, key))
    {
        pthread_mutex_unlock(&mutex);
        syslog(LOG_INFO, "[kysdk-conf2] %s -> %s is unwritable\n", __func__, key);
        return FALSE;
    }

    if (!_type_check(ksettings, key, value))
    {
        pthread_mutex_unlock(&mutex);
        syslog(LOG_INFO, "[kysdk-conf2] %s -> %s type invalid\n", __func__, key);
        return FALSE;
    }
    if (!kdk_conf2_range_check(ksettings, key, value))
    {
        pthread_mutex_unlock(&mutex);
        syslog(LOG_INFO, "[kysdk-conf2] %s -> %s vlaue is not in range\n", __func__, key);
        return FALSE;
    }

    int result = FALSE;
    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    // key不存在
    if (NULL == key)
    {
        pthread_mutex_unlock(&mutex);
        syslog(LOG_INFO, "[kysdk-conf2] %s -> %s key is not exists\n", __func__, key);
        return FALSE;
    }

    char *curr_value = kdk_conf2_schema_key_get_value(key_hash);

    char *id = kdk_conf2_schema_get_id(ksettings->priv->schema);
    char *version = kdk_conf2_schema_get_version(ksettings->priv->schema);
    if (NULL == curr_value || 0 != strcmp(curr_value, value))
    {
        result = _call_method("set",
                              id,
                              version,
                              key, value);
        if (result)
        {
            key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
            kdk_conf2_schema_key_set_value(key_hash, value);
        }
    }
    else
        result = TRUE;
    g_free(id);
    g_free(version);
    g_free(curr_value);
    syslog(LOG_INFO, "[kysdk-conf2] %s -> Set value finished. Return %s\n", __func__, result ? "true" : "false");

    pthread_mutex_unlock(&mutex);
    return result;
}

char *kdk_conf2_get_value(KSettings *ksettings, const char *key)
{
    pthread_mutex_lock(&mutex);

    syslog(LOG_INFO, "[kysdk-conf2] %s -> Get value start\n", __func__);

    if (!K_IS_SETTINGS(ksettings) || NULL == key)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    if (NULL == key_hash)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    char *value = kdk_conf2_schema_key_get_value(key_hash);
    if ((0 == kdk_conf2_is_writable(ksettings, key)) || NULL == value)
    {
        value = kdk_conf2_schema_key_get_default_value(key_hash);
    }
    syslog(LOG_INFO, "[kysdk-conf2] %s -> Get value end\n", __func__);

    pthread_mutex_unlock(&mutex);
    return value;
}

char *kdk_conf2_get_default_value(KSettings *ksettings, const char *key)
{
    pthread_mutex_lock(&mutex);

    if (!K_IS_SETTINGS(ksettings) || NULL == key)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    GHashTable *node = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    char *default_value = NULL;
    if (node)
        default_value = kdk_conf2_schema_key_get_default_value(node);

    pthread_mutex_unlock(&mutex);
    return default_value;
}

int kdk_conf2_set_boolean(KSettings *ksettings, const char *key, int value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return FALSE;

    GVariant *variant = g_variant_new_boolean(value);
    if (NULL == variant)
        return FALSE;

    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);
    return result;
}

int kdk_conf2_get_boolean(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = FALSE;

    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        result = (0 == strcmp(value, "true")) ? 1 : 0;
        g_free(value);
    }
    return result;
}

int kdk_conf2_get_default_boolean(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = FALSE;

    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        result = (0 == strcmp(value, "true")) ? 1 : 0;
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_double(KSettings *ksettings, const char *key, double value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant = g_variant_new_double(value);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);
    return result;
}

double kdk_conf2_get_double(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    double result = 0.0;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%lf", &result);
        g_free(value);
    }
    return result;
}

double kdk_conf2_get_default_double(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    double result = 0.0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%lf", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_enum(KSettings *ksettings, const char *key, int value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = 0;
    char *range = kdk_conf2_get_range(ksettings, key);
    if (NULL == range)
        goto out;

    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *variant = g_variant_parse(G_VARIANT_TYPE("a{si}"), range, NULL, &endptr, &error);
    if (variant == NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> 解析失败: %s\n", __func__, error->message);
        g_error_free(error);
        goto out;
    }

    GVariantIter *iter = NULL;
    g_variant_get(variant, "a{si}", &iter);

    char *nick = NULL;
    gint32 tmp = 0;
    while (g_variant_iter_loop(iter, "{si}", &nick, &tmp))
    {
        if (tmp == value)
        {
            char set_value[1024];
            snprintf(set_value, sizeof(set_value), "%d", value);
            result = kdk_conf2_set_value(ksettings, key, nick);
            break;
        }
    }
    g_variant_iter_free(iter);
    g_variant_unref(variant);
out:
    g_free(range);
    return result;
}

int kdk_conf2_get_enum(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = -1;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL == value)
        return result;
    char *range = kdk_conf2_get_range(ksettings, key);
    if (NULL == range)
        return result;

    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *variant = g_variant_parse(G_VARIANT_TYPE("a{si}"), range, NULL, &endptr, &error);
    if (variant == NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> 解析失败: %s\n", __func__, error->message);
        g_error_free(error);
        goto out;
    }
    GVariantIter *iter = NULL;
    g_variant_get(variant, "a{si}", &iter);

    char *nick = NULL;
    gint32 tmp = 0;
    while (g_variant_iter_loop(iter, "{si}", &nick, &tmp))
    {
        if (0 == g_strcmp0(nick, value))
        {
            result = tmp;
            break;
        }
    }
    g_variant_iter_free(iter);
    g_variant_unref(variant);

out:
    g_free(value);
    g_free(range);
    return result;
}

int kdk_conf2_get_default_enum(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = 0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL == value)
        return result;
    char *range = kdk_conf2_get_range(ksettings, key);
    if (NULL == range)
        return result;

    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *variant = g_variant_parse(G_VARIANT_TYPE("a{si}"), range, NULL, &endptr, &error);
    if (variant == NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> 解析失败: %s\n", __func__, error->message);
        g_error_free(error);
        goto out;
    }
    GVariantIter *iter = NULL;
    g_variant_get(variant, "a{si}", &iter);

    char *nick = NULL;
    gint32 tmp = 0;
    while (g_variant_iter_loop(iter, "{si}", &nick, &tmp))
    {
        if (0 == g_strcmp0(nick, value))
        {
            result = tmp;
            break;
        }
    }
    g_variant_iter_free(iter);
    g_variant_unref(variant);

out:
    g_free(value);
    g_free(range);
    return result;
}

int kdk_conf2_set_int(KSettings *ksettings, const char *key, int value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant = g_variant_new_int32(value);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);

    return result;
}

int kdk_conf2_get_int(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = 0;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%d", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_get_default_int(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = 0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%d", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_int64(KSettings *ksettings, const char *key, long value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant = g_variant_new_int64(value);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);

    return result;
}

long kdk_conf2_get_int64(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    long result = 0;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%ld", &result);
        g_free(value);
    }
    return result;
}

long kdk_conf2_get_default_int64(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    long result = 0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%ld", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_uint(KSettings *ksettings, const char *key, unsigned int value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant = g_variant_new_uint32(value);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);

    return result;
}

unsigned int kdk_conf2_get_uint(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    unsigned int result = 0;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%u", &result);
        g_free(value);
    }
    return result;
}

unsigned int kdk_conf2_get_default_uint(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    unsigned int result = 0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%u", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_uint64(KSettings *ksettings, const char *key, unsigned long value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant = g_variant_new_uint64(value);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);

    return result;
}

unsigned long kdk_conf2_get_uint64(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    unsigned long result = 0;
    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%lu", &result);
        g_free(value);
    }
    return result;
}

unsigned long kdk_conf2_get_default_uint64(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    unsigned long result = 0;
    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL != value)
    {
        sscanf(value, "%lu", &result);
        g_free(value);
    }
    return result;
}

int kdk_conf2_set_string(KSettings *ksettings, const char *key, const char *value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    int result = kdk_conf2_set_value(ksettings, key, value);

    return result;
}

char *kdk_conf2_get_string(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    return kdk_conf2_get_value(ksettings, key);
}

char *kdk_conf2_get_default_string(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    return kdk_conf2_get_default_value(ksettings, key);
}

int kdk_conf2_set_strv(KSettings *ksettings, const char *key, const char *const *value)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return 0;

    GVariant *variant;
    if (value != NULL)
        variant = g_variant_new_strv(value, -1);
    else
        variant = g_variant_new_strv(NULL, 0);
    int result = kdk_conf2_set_value(ksettings, key, g_variant_print(variant, FALSE));
    g_variant_unref(variant);

    return result;
}

char **kdk_conf2_get_strv(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    char **result = NULL;

    char *value = kdk_conf2_get_value(ksettings, key);
    if (NULL == value)
        return result;

    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *variant = g_variant_parse(G_VARIANT_TYPE("as"), value, NULL, &endptr, &error);
    if (NULL == variant)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> 解析失败: %s\n", __func__, error->message);
        g_free(value);
        g_error_free(error);
        return NULL;
    }
    result = g_variant_dup_strv(variant, NULL);
    g_free(value);
    g_variant_unref(variant);

    return result;
}

char **kdk_conf2_get_default_strv(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    char **result = NULL;

    char *value = kdk_conf2_get_default_value(ksettings, key);
    if (NULL == value)
        return result;

    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *variant = g_variant_parse(G_VARIANT_TYPE("as"), value, NULL, &endptr, &error);
    if (NULL == variant)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> 解析失败: %s\n", __func__, error->message);
        g_free(value);
        g_error_free(error);
        return NULL;
    }
    result = g_variant_dup_strv(variant, NULL);
    g_free(value);
    g_variant_unref(variant);

    return result;
}

char *kdk_conf2_get_descrition(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    return kdk_conf2_schema_key_get_description(key_hash);
}

char *kdk_conf2_get_summary(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    return kdk_conf2_schema_key_get_summary(key_hash);
}

char *kdk_conf2_get_type(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return NULL;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    return kdk_conf2_schema_key_value_type(key_hash);
}

void kdk_conf2_reset(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return;

    if (NULL == kdk_conf2_schema_get_key(ksettings->priv->schema, key))
        return;

    syslog(LOG_INFO, "[kysdk-conf2] %s ->  :reset %s value\n", __func__, key);
    char *id = kdk_conf2_schema_get_id(ksettings->priv->schema);
    char *version = kdk_conf2_schema_get_version(ksettings->priv->schema);
    int result = _call_method("reset",
                              id,
                              version,
                              key, NULL);
    if (result)
    {
        GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
        kdk_conf2_schema_key_reset(key_hash);
    }
    g_free(id);
    g_free(version);
}

int kdk_conf2_is_writable(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return FALSE;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    char *permission = kdk_conf2_schema_key_get_permission(key_hash);
    if ((NULL == permission) || (0 == strcmp(permission, "public")))
        return TRUE;
    return FALSE;
}

int kdk_conf2_has_key(KSettings *ksettings, const char *key)
{
    if (!K_IS_SETTINGS(ksettings) || NULL == key)
        return FALSE;

    GHashTable *key_hash = kdk_conf2_schema_get_key(ksettings->priv->schema, key);
    if (NULL == key_hash)
        return 0;
    else
        return 1;
}

unsigned long kdk_conf2_connect_signal(KSettings *ksettings, const char *signal_name, KCallBack handler, void *user_data)
{
    if (!K_IS_SETTINGS(ksettings))
        return 0;

    return g_signal_connect(ksettings, signal_name, G_CALLBACK(handler), user_data);
}

void kdk_conf2_reload()
{
    _call_method("reload", NULL, NULL, NULL, NULL);
}

char **kdk_conf2_list_schemas(const char *app, const char *version)
{
    if (NULL == app)
        return NULL;

    char *default_version = NULL;
    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();

    if (NULL == schemas_table)
    {
        syslog(LOG_INFO, "[kysdk-conf2] load user.db failed\n");
        return NULL;
    }

    GHashTableIter iter;
    AppData *app_data = NULL;
    GHashTable *app_hash = NULL;
    g_hash_table_iter_init(&iter, schemas_table);
    while (g_hash_table_iter_next(&iter, (gpointer*)&app_data, (gpointer*)&app_hash))
    {
        if (0 == strcmp(app_data->name, app))
        {
            default_version = app_data->default_version;
            break;
        }
        app_hash = NULL;
    }
    if (NULL == app_hash)
        return NULL;

    VersionData version_data;
    version_data.name = version ? version : default_version;
    GHashTable *version_hash = g_hash_table_lookup(app_hash, &version_data);
    if (NULL == version_hash)
        return NULL;

    GList *schemas = g_hash_table_get_keys(version_hash);
    guint len = g_list_length(schemas);

    GList *tmp = schemas;
    char **result = calloc(len + 1, sizeof(char *));
    for (guint i = 0; i < len; i++)
    {
        result[i] = strdup(tmp->data);
        tmp = tmp->next;
    }

    if (0 == ref_count)
    {
        g_hash_table_destroy(schemas_table);
        schemas_table = NULL;
    }
    return result;
}

int kdk_conf2_save_user_configure(const char *path)
{
    syslog(LOG_INFO, "[kysdk-conf2] %s -> save user config start\n", __func__);
    char real_path[K_MAX_PATH];
    memset(real_path, 0, sizeof(real_path));
    if (!realpath(path, real_path))
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s -> path not exists\n", __func__);
        return FALSE;
    }

    GDBusConnection *connection;
    GError *error = NULL;
    GDBusProxy *proxy;
    GVariant *result;
    int response = FALSE;

    // 获取 DBus 会话总线
    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (NULL == connection)
    {
        connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        if (error != NULL)
        {
            syslog(LOG_ERR, "[kysdk-conf2] %s -> Connection Error: %s\n", __func__, error->message);
            g_error_free(error);
            return response;
        }
    }

    // 创建代理对象
    proxy = g_dbus_proxy_new_sync(connection,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  NULL,
                                  "com.kylin.kysdk.conf2",
                                  "/com/kylin/kysdk/conf2",
                                  "com.kylin.kysdk.conf2",
                                  NULL,
                                  &error);
    if (error != NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> Proxy Error: %s\n", error->message);
        g_error_free(error);
        return response;
    }

    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_INFO, "【kysdk-conf2】 Get user name failed! use root config\n");
        strncpy(user_name, "root", sizeof(user_name));
    }
    else
    {
        strncpy(user_name, pw->pw_name, sizeof(user_name));
        endpwent();
    }

    GVariant *variant = g_variant_new("(ss)", user_name, real_path);
    result = g_dbus_proxy_call_sync(proxy,
                                    "save",
                                    variant,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
    if (error != NULL)
    {
        syslog(LOG_ERR, "[kysdk-conf2] %s -> Method Call Error: %s\n", __func__, error->message);
        g_error_free(error);
        return response;
    }

    // 处理方法调用的返回值
    g_variant_get(result, "(b)", &response);

    // 释放资源
    g_variant_unref(result);
    g_object_unref(proxy);
    syslog(LOG_INFO, "[kysdk-conf2] %s -> save user config end\n", __func__);
    return response;
}

KSettings *kdk_conf2_new_extends_id(const char *old_id, const char *new_id, const char *version)
{
    if (NULL == old_id || NULL == new_id)
        return NULL;

    KSettings *setting = kdk_conf2_new(new_id, version);
    if (NULL != setting)
        return setting;

    syslog(LOG_INFO, "[kysdk-conf2] %s -> creat new id : %s\n", __func__, new_id);
    GDBusConnection *connection;
    GError *error = NULL;
    GDBusProxy *proxy;
    GVariant *result;
    int response = FALSE;

    // 获取 DBus 会话总线
    connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (NULL == connection)
    {
        connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        if (error != NULL)
        {
            syslog(LOG_INFO, "[kysdk-conf2] %s -> Connection Error: %s\n", __func__, error->message);
            g_error_free(error);
            return NULL;
        }
    }

    // 创建代理对象
    proxy = g_dbus_proxy_new_sync(connection,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  NULL,
                                  "com.kylin.kysdk.conf2",
                                  "/com/kylin/kysdk/conf2",
                                  "com.kylin.kysdk.conf2",
                                  NULL,
                                  &error);
    if (error != NULL)
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s -> Proxy Error: %s\n", __func__, error->message);
        g_error_free(error);
        return NULL;
    }

    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_INFO, "【kysdk-conf2】 Get user name failed! use root config\n");
        strncpy(user_name, "root", sizeof(user_name));
    }
    else
    {
        strncpy(user_name, pw->pw_name, sizeof(user_name));
        endpwent();
    }

    GVariant *variant = g_variant_new("(ssss)",
                                      old_id,
                                      new_id,
                                      version ? version : "",
                                      user_name);
    result = g_dbus_proxy_call_sync(proxy,
                                    "extends_id",
                                    variant,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
    if (error != NULL)
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s -> Method Call Error: %s\n", __func__, error->message);
        g_error_free(error);
        return NULL;
    }

    // 处理方法调用的返回值
    g_variant_get(result, "(b)", &response);

    // 释放资源
    g_variant_unref(result);
    g_object_unref(proxy);

    if (response)
    {
        kdk_conf2_schema_update_schemas_table();
        return kdk_conf2_new(new_id, version);
    }
    return NULL;
}

int kdk_conf2_is_schema(const char *id, const char *version)
{
    if (NULL == id)
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s ->: id is NULL\n", __func__);
        return FALSE;
    }

    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();

    if (NULL == schemas_table)
    {
        syslog(LOG_INFO, "[kysdk-conf2] %s ->: load user.db failed\n", __func__);
        return FALSE;
    }

    KSettingsSchema *schema = kdk_conf2_schema_table_lookup(id, version);
    if (NULL == schema)
        return FALSE;

    return TRUE;
}

char *kdk_conf2_get_schema_summary(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;

    return ksettings->priv->schema->summary ? strdup(ksettings->priv->schema->summary) : NULL;
}

char *kdk_conf2_get_schema_desription(KSettings *ksettings)
{
    if (!K_IS_SETTINGS(ksettings))
        return NULL;

    return ksettings->priv->schema->description ? strdup(ksettings->priv->schema->description) : NULL;
}

char **kdk_conf2_list_applications()
{
    char **result = NULL;
    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();
    if (NULL == schemas_table)
    {
        return NULL;
    }

    GList *curr, *keys;
    curr = keys = g_hash_table_get_keys(schemas_table);

    guint length = g_list_length(keys);
    result = calloc(length + 1, sizeof(char*));
    for (guint i = 0; i < length; i++)
    {
        AppData *app = (AppData*)curr->data;
        result[i] = strdup(app->name);
        curr = curr->next;
    }
    g_list_free(keys);
    return result;
}

char **kdk_conf2_list_versions(char *app)
{
    if (NULL == app)
        return NULL;

    if (NULL == schemas_table)
        kdk_conf2_schema_update_schemas_table();
    if (NULL == schemas_table)
    {
        return NULL;
    }

    AppData app_data;
    app_data.name = app;
    app_data.default_version = NULL;
    GHashTable* app_hash = (VersionData*)g_hash_table_lookup(schemas_table, (gconstpointer)(&app_data));
    if (NULL == app_hash)
        return NULL;

    char **result = NULL;
    GList *curr, *keys;
    curr = keys = g_hash_table_get_keys(app_hash);

    guint length = g_list_length(keys);
    result = calloc(length, sizeof(char*));
    for (guint i = 0; i < length; i++)
    {
        VersionData *version = (AppData*)curr->data;
        result[i] = strdup(version->name);
        curr = curr->next;
    }
    g_list_free(keys);
    return result;
}
