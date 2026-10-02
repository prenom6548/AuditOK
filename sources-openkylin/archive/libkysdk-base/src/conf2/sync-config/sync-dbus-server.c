/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
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
#include "sync-dbus-server.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <gio/gsettings.h>
#include "../api/libkysettings.h"

#define G_SCHEMAS "/etc/kylin-config/conf2.yaml"

/***************************************dbus服务********************************************/

static SyncConfig *skeleton = NULL;

static gboolean on_handle_register_schema(SyncConfig *skeleton, GDBusMethodInvocation *invocation,
                                          const gchar *arg_Conf2Id,
                                          const gchar *arg_SchemaId,
                                          const gchar *arg_Path,
                                          gpointer user_data)
{
    klog_info("%s: Regester call %s\n", __func__, arg_Conf2Id);
    sync_config__complete_register_schema(skeleton, invocation, TRUE);
    return TRUE;
}

static gboolean on_handle_unregister_schema(SyncConfig *skeleton, GDBusMethodInvocation *invocation,
                                            const gchar *arg_SchemaId,
                                            const gchar *arg_Path,
                                            gpointer user_data)
{
    klog_info("%s: Unregester call\n", __func__);
    sync_config__complete_unregister_schema(skeleton, invocation, TRUE);
    return TRUE;
}

/**
 * 连接上bus daemon的回调
 **/
void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    klog_info("%s: has been invoked, name is %s\n", __func__, name);
    GError *error = NULL;
    skeleton = sync_config__skeleton_new();
    // 连接方法同步处理函数
    g_signal_connect(G_OBJECT(skeleton), "handle-register-schema", G_CALLBACK(on_handle_register_schema), NULL);
    g_signal_connect(G_OBJECT(skeleton), "handle-unregister-schema", G_CALLBACK(on_handle_unregister_schema), NULL);
    // 发布服务到总线
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, "/com/kylin/kysdk/syncConfig", &error);
    if (error != NULL)
    {
        klog_err("%s: Failed to export object. Reason: %s.\n", __func__, error->message);
        g_error_free(error);
    }
}

/**
 * 成功注册busName的回调
 **/
void on_bus_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

/**
 * busName丢失的回调，一般是server挂了？？
 **/
void on_bus_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

/**
 * 测试发送信号函数，调用Hello.h自动生成的方法
 **/
static gint status_value = 1;
static gboolean emit_test_state_signal(gconstpointer p)
{
    printf("emit_test_status_signal invoked\n");
    if (skeleton != NULL)
    {
        sync_config__emit_state(skeleton, status_value);
    }
    status_value++;
}

gboolean init_gdbus_server(GBusType type)
{
    guint owner_id;
    GMainLoop *loop;
#if !GLIB_CHECK_VERSION(2, 35, 0)
    g_type_init();
#endif
    owner_id = g_bus_own_name(type, "com.kylin.kysdk.SyncConfig",
                              G_BUS_NAME_OWNER_FLAGS_NONE,
                              on_bus_acquired, on_bus_name_acquired, on_bus_name_lost, NULL, NULL);

    // 测试，每3s发一次signal
    // g_timeout_add(3000, (GSourceFunc)emit_test_state_signal, NULL);

    // 主线程进入循环
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_bus_unown_name(owner_id);

    return TRUE;
}

/***************************************同步配置********************************************/
#define OUT
#define KPATH_LEN 512
#define APP_LEN 128
#define VER_LEN 32
#define LINE_LEN 4096

struct _conf2_gsetting_handle_duple
{
    KSettings *ksetting;
    GSettings *gsetting;
};

GHashTable *id_tables = NULL;

static void _on_gsettings_key_changed(GSettings *gsetting, const char *key, void *user_data)
{
    GVariant *value = g_settings_get_value(gsetting, key);
    KSettings *ksetting = (KSettings *)user_data;
    char *conf2_value = kdk_conf2_get_value(ksetting, key);
    if (0 == strcmp(conf2_value, g_variant_print(value, TRUE)))
        return;
    int ret = kdk_conf2_set_value(ksetting, key, g_variant_print(value, TRUE));
    if (ret)
    {
        // g_signal_emit(ksetting, k_settings_signals[SIGNAL_CHANGED], g_quark_from_string(key), key);
    }
    g_variant_unref(value);
}

static void _on_conf2_key_changed(KSettings *ksetting, const char *key, void *user_data)
{
    GSettings *gsetting = (GSettings*)user_data;
    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *gsetting_value = g_settings_get_value(gsetting, key);
    if (NULL != gsetting_value)
    {
        char *value = kdk_conf2_get_value(ksetting, key);
        if (0 != strcmp(value, g_variant_print(gsetting_value, TRUE)))
        {
            char *type = kdk_conf2_get_type(ksetting, key);
            GVariant *variant;
            if (0 == strcmp("s", type))
                variant = g_variant_new_string(value);
            else
                variant = g_variant_parse(G_VARIANT_TYPE(type), value, NULL, &endptr, &error);
            if (NULL == variant)
            {
                klog_err("%s: %s\n", __func__, error->message);
            }
            else
            {
                g_settings_set_value(gsetting, key, variant);
                g_settings_sync();
                // g_variant_unref(variant);
            }
            free(value);
        }
        g_variant_unref(gsetting_value);
    }
}

static gboolean _connect_signal_sync_config(const char *conf2_id, const char *gsetting_id)
{
    // klog_info("%s: Sync config [%s] and [%s]\n", __func__, conf2_id, gsetting_id);
    GSettings *gsetting = NULL;
    KSettings *ksetting = NULL;

    gsetting = g_settings_new(gsetting_id);
    if(NULL == gsetting)
    {
        klog_err("%s: Create handle fail\n", __func__, gsetting_id);
        return FALSE;
    }

    ksetting = kdk_conf2_new(conf2_id, NULL);
    if (NULL == ksetting)
    {
        g_object_unref(gsetting);
        return FALSE;
    }
    

    //conf2默认值写入到gsetting设置中
    char **keys = kdk_conf2_list_keys(ksetting);
    if (NULL != keys)
    {
        for(char **key = keys; *key; key++)
        {
            char *value = kdk_conf2_get_default_value(ksetting, *key);
            char *type = kdk_conf2_get_type(ksetting, *key);
            if ((NULL != value) && (NULL != type))
            {
                if (0 == strcmp(type, "s"))
                    g_settings_set_string(gsetting, *key, value);
                else
                {
                    GError *err = NULL;
                    GVariant *variant = g_variant_parse(G_VARIANT_TYPE(type), value, NULL, NULL, &err);
                    if (NULL != err)
                    {
                        klog_err("%s\n", err->message);
                        g_error_free(err);
                        err = NULL;
                    }
                    else
                    {
                        g_settings_set_value(gsetting, *key, variant);
                        g_variant_unref(variant);
                    }
                }
                free(value);
                free(type);
            }
        }
        g_settings_sync();
    }
    g_signal_connect(gsetting, "changed", G_CALLBACK(_on_gsettings_key_changed), ksetting);
    kdk_conf2_connect_signal(ksetting, "changed", (KCallBack)_on_conf2_key_changed, gsetting);
    return TRUE;
}

static void _read_id_tables()
{
    FILE *fp = fopen(G_SCHEMAS, "r");
    if (NULL == fp)
    {
        klog_err("open %s fial: %s\n", G_SCHEMAS, strerror(errno));
        return;
    }
    else
    {
        gboolean read = FALSE;
        char line[1024];
        while (fgets(line, 1024, fp))
        {
            g_strstrip(line);
            if (!read && g_str_has_prefix(line, "sync-applications"))
            {
                read = TRUE;
                continue;
            }
            if (read)
            {
                if (!g_str_has_prefix(line, "-"))
                    break;

                if (!id_tables)
                    id_tables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

                char **id_map = g_strsplit(line + 2, ": ", -1);
                g_hash_table_insert(id_tables, id_map[0], id_map[1]);

                char target_file[1024];
                snprintf(target_file, sizeof(target_file), "/etc/kylin-config/basec/%s.yaml", id_map[0]);
                gboolean exists = !access(target_file, F_OK);
                gboolean is_root = g_strcmp0(g_get_user_name(), "root");
                if (!exists && !is_root)
                {
                    char cmd[1024];
                    snprintf(cmd, sizeof(cmd), "/usr/bin/gschema_xml2yaml --id %s --version 1.0.0.0 --path /etc/kylin-config/basic", id_map[0]);
                    system(cmd);
                }
                g_free(id_map);
            }
        }
        fclose(fp);
    }
}

int monitoring_requires_synchronized_configuration()
{
    _read_id_tables();
    if (NULL == id_tables)
    {
        klog_err("Resolution error or no members need to be synchronized\n");
        return 1;
    }

    const char * const *gsetting_schemas = g_settings_list_schemas();

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, id_tables);

    char *key, *value;
    while (g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
    {
        char **ksetting_schemas = kdk_conf2_list_schemas(value, NULL);
        if (NULL == ksetting_schemas)
        {
            klog_err("Gsettings file %s not changed ksetting file\n", key);
            continue;
        }

        for(int i = 0; ksetting_schemas[i]; i++)
        {
            for(int j = 0; gsetting_schemas[j]; j++)
            {
                if (g_str_has_prefix(gsetting_schemas[j], key) && g_str_has_suffix(gsetting_schemas[j], ksetting_schemas[i]))
                {
                    _connect_signal_sync_config(ksetting_schemas[i], gsetting_schemas[j]);
                }
            }
        }
    }

    return 0;
}
