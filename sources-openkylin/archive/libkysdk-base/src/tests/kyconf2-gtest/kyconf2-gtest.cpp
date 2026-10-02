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
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../cpp_stub/src/stub.h"
extern "C"
{
#include "../../conf2/api/libkysettings.h"
#include <glib.h>
}

// 键值改变信号回调函数
static void on_key_changed(KSettings *setting, char *key, void *user_data)
{
    char *value = kdk_conf2_get_value(setting, key);
    printf("%s change to %s\n", key, value);

    g_main_loop_quit((GMainLoop*)user_data);
}

// 键值改变信号回调函数
static void on_reload(KSettings *setting, void *user_data)
{
    printf("reload signal\n");
}

TEST(TestSuite1, kdk_conf2_new_fail)
{
    KSettings *setting = kdk_conf2_new("not.exists.id", NULL);
    printf("Import not exists id\n");

    setting = kdk_conf2_new(NULL, NULL);
    printf("Import NULL id\n");

    char *schema_summary = kdk_conf2_get_schema_summary(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_get_schema_summary\n");

    char *schema_description = kdk_conf2_get_schema_desription(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_get_schema_desription\n");

    char *id = kdk_conf2_get_id(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_get_id\n");

    char *version = kdk_conf2_get_version(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_get_version\n");

    char **children = kdk_conf2_list_children(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_list_children\n");

    char **keys = kdk_conf2_list_keys(setting);
    printf("kdk_conf2_new_fail -> kdk_conf2_list_keys\n");

    // 把childname替换为正确的子组名
    KSettings *child = kdk_conf2_get_child(setting, "proxy");
    printf("kdk_conf2_new_fail -> kdk_conf2_get_child\n");

    kdk_conf2_has_key(setting, "type");
    printf("kdk_conf2_new_fail -> kdk_conf2_has_key\n");

    char *range = kdk_conf2_get_range(setting, "type");
    printf("kdk_conf2_new_fail -> kdk_conf2_get_range\n");

    int success = 0;
    success = kdk_conf2_range_check(setting, "int", "65540");

    {
        // /**********通用读写**********/
        char *value = kdk_conf2_get_default_value(setting, "int");

        value = kdk_conf2_get_value(setting, "int");

        success = kdk_conf2_set_value(setting, "int", "8081");

        /**********枚举读写**********/
        int e = kdk_conf2_get_default_enum(setting, "type");

        e = kdk_conf2_get_enum(setting, "type");

        success = kdk_conf2_set_enum(setting, "type", 0x2);

        /**********整形读写**********/
        int i = kdk_conf2_get_default_int(setting, "int");

        i = kdk_conf2_get_int(setting, "int");

        success = kdk_conf2_set_int(setting, "int", i + 1);

        /**********长整形读写**********/
        long l = kdk_conf2_get_default_int64(setting, "long");

        l = kdk_conf2_get_int64(setting, "long");

        success = kdk_conf2_set_int64(setting, "long", l + 1);

        /**********无符号整形读写**********/
        unsigned int u = kdk_conf2_get_default_uint(setting, "uint");

        u = kdk_conf2_get_uint(setting, "uint");

        success = kdk_conf2_set_uint(setting, "uint", u + 1);

        /**********无符号长整形读写**********/
        unsigned long t = kdk_conf2_get_default_uint64(setting, "ulong");

        t = kdk_conf2_get_uint64(setting, "ulong");

        success = kdk_conf2_set_uint64(setting, "ulong", t + 1);

        /**********布尔型读写**********/
        int b = kdk_conf2_get_default_boolean(setting, "enable");

        b = kdk_conf2_get_boolean(setting, "enable");

        success = kdk_conf2_set_boolean(setting, "enable", !b);

        /**********浮点型读写**********/
        double d = kdk_conf2_get_default_double(setting, "transparency");

        d = kdk_conf2_get_double(setting, "transparency");

        success = kdk_conf2_set_double(setting, "transparency", d + 1);

        /**********字符串读写**********/
        char *s = kdk_conf2_get_default_string(setting, "name");

        s = kdk_conf2_get_string(setting, "name");

        success = kdk_conf2_set_string(setting, "name", "dsefe");

        /**********字符串列表读写**********/
        char **sv = kdk_conf2_get_default_strv(setting, "list");

        sv = kdk_conf2_get_strv(setting, "list");

        const char *v[] = {"CESI仿宋-GB13000", "CESI_FS_GB13000", NULL};
    }
    char *description = kdk_conf2_get_descrition(setting, "int");

    char *summary = kdk_conf2_get_summary(setting, "int");

    char *type = kdk_conf2_get_type(setting, "int");

    kdk_conf2_reset(setting, "int");

    int writable = kdk_conf2_is_writable(setting, "int");
}

TEST(TestSuite2, kdk_conf2_new_success)
{
    KSettings *setting = kdk_conf2_new("test.notice", NULL);
    printf("kdk_conf2_new success\n");

    char *schema_summary = kdk_conf2_get_schema_summary(setting);

    char *schema_description = kdk_conf2_get_schema_desription(setting);

    char *id = kdk_conf2_get_id(setting);

    char *version = kdk_conf2_get_version(setting);

    char **children = kdk_conf2_list_children(setting);

    char **keys = kdk_conf2_list_keys(setting);

    // 把childname替换为正确的子组名
    KSettings *child = kdk_conf2_get_child(setting, "proxy");

    kdk_conf2_has_key(setting, "type");

    char *range = kdk_conf2_get_range(setting, "type");

    int success = 0;
    success = kdk_conf2_range_check(setting, "int", "65540");

    {
        /**********通用读写**********/
        char *value = kdk_conf2_get_default_value(setting, "int");

        value = kdk_conf2_get_value(setting, "int");

        success = kdk_conf2_set_value(setting, "int", "8081");

        /**********枚举读写**********/
        int e = kdk_conf2_get_default_enum(setting, "type");

        e = kdk_conf2_get_enum(setting, "type");

        success = kdk_conf2_set_enum(setting, "type", 0x2);

        /**********整形读写**********/
        int i = kdk_conf2_get_default_int(setting, "int");

        i = kdk_conf2_get_int(setting, "int");

        success = kdk_conf2_set_int(setting, "int", i + 1);

        /**********长整形读写**********/
        long l = kdk_conf2_get_default_int64(setting, "long");

        l = kdk_conf2_get_int64(setting, "long");

        success = kdk_conf2_set_int64(setting, "long", l + 1);

        /**********无符号整形读写**********/
        unsigned int u = kdk_conf2_get_default_uint(setting, "uint");

        u = kdk_conf2_get_uint(setting, "uint");

        success = kdk_conf2_set_uint(setting, "uint", u + 1);

        /**********无符号长整形读写**********/
        unsigned long t = kdk_conf2_get_default_uint64(setting, "ulong");

        t = kdk_conf2_get_uint64(setting, "ulong");

        success = kdk_conf2_set_uint64(setting, "ulong", t + 1);

        /**********布尔型读写**********/
        int b = kdk_conf2_get_default_boolean(setting, "enable");

        b = kdk_conf2_get_boolean(setting, "enable");

        success = kdk_conf2_set_boolean(setting, "enable", !b);

        /**********浮点型读写**********/
        double d = kdk_conf2_get_default_double(setting, "transparency");

        d = kdk_conf2_get_double(setting, "transparency");

        success = kdk_conf2_set_double(setting, "transparency", d + 1);

        /**********字符串读写**********/
        char *s = kdk_conf2_get_default_string(setting, "name");

        s = kdk_conf2_get_string(setting, "name");

        success = kdk_conf2_set_string(setting, "name", "dsefe");

        /**********字符串列表读写**********/
        char **sv = kdk_conf2_get_default_strv(setting, "list");

        sv = kdk_conf2_get_strv(setting, "list");

        const char *v[] = {"CESI仿宋-GB13000", "CESI_FS_GB13000", NULL};
    }

    char *description = kdk_conf2_get_descrition(setting, "int");

    char *summary = kdk_conf2_get_summary(setting, "int");

    char *type = kdk_conf2_get_type(setting, "int");

    kdk_conf2_reset(setting, "int");

    int writable = kdk_conf2_is_writable(setting, "int");

    kdk_conf2_ksettings_destroy(setting);
}

TEST(TestSuite3, kdk_conf2_reload)
{
    kdk_conf2_reload();
    printf("kdk_conf2_reload\n");
}

TEST(TestSuite4, kdk_conf2_list_schemas)
{
    char **schemas = kdk_conf2_list_schemas("test", NULL);
    printf("kdk_conf2_list_schemas\n");
}

TEST(TestSuite5, kdk_conf2_save_user_configure)
{
    kdk_conf2_save_user_configure("/home/kylin/output");
    printf("kdk_conf2_save_user_configure\n");
}

TEST(TestSuite6, kdk_conf2_new_extends_id)
{
    KSettings *externds_setting = kdk_conf2_new_extends_id("test.notice", "test.notice.tss", NULL);
    printf("kdk_conf2_new_extends_id\n");
}

TEST(TestSuite7, kdk_conf2_is_schema)
{
    int is_schema = kdk_conf2_is_schema("test.notice", NULL);
    printf("kdk_conf2_is_schema\n");
}

int main(int argc, char** argv)
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    KSettings *setting = kdk_conf2_new("test.notice", NULL);

    kdk_conf2_connect_signal(setting, "changed", (KCallBack)on_key_changed, loop);

    int enable = kdk_conf2_get_boolean(setting, "enable");

    kdk_conf2_set_double(setting, "enable", !enable);

    g_main_loop_run(loop);

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}