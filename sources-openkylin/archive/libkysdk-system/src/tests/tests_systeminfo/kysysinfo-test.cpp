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

#include "../../systeminfo/libkysysinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"
#include "../../packages/libkypackages.h"
#include <dbus-1.0/dbus/dbus.h>
#include <string.h>

void callbacke(char *old_user, char *new_user, void *userdata)
{
    printf("%s  %s\n", old_user, new_user);
}

extern int strcmp_stub(const char *__s1, const char *__s2)
{
    if(strstr(__s2, "Unknown") || strstr(__s2, "aarch64") || strstr(__s2, "x86_64") || strstr(__s2, "ukui-control-center"))
    {
        return 0;
    }
    else
        return 1;
}

int kdk_package_is_installed_stub(const char *name, const char *version)
{
    return 1;
}

extern int access_stub (const char *__name, int __type)
{
    if(strstr(__name, "/etc/kylin-version/kylin-system-version.conf"))
    {
        return 1;
    }
    else
        return 0;
}

DBusMessage *dbus_message_new_method_call_stub(const char *destination, const char *path, const char *iface, const char *method)
{
    return nullptr;
}

extern __uid_t geteuid_stub (void) __THROW
{
    return 1;
}

extern char *getenv_stub(const char *__name)
{
    return "0";
}

extern void *realloc_stub (void *__ptr, size_t __size)
{
    return nullptr;
}

FILE *fopen_stub(const char *__restrict__ __filename, const char *__restrict__ __modes)
{
    return nullptr;
}

TEST(KdkSysinfoTest, SysinfoInfoStub)
{
    Stub st;
    st.set(strcmp, strcmp_stub);
    const char *line = kdk_system_get_production_line();
    printf("line = %s\n", line);

    const char *minor_version = kdk_system_get_minor_version();
    printf("minor_version = %s\n", minor_version);

    const char *major_version = kdk_system_get_major_version();
    printf("major_version = %s\n", major_version);

    st.set(kdk_package_is_installed, kdk_package_is_installed_stub);
    major_version = kdk_system_get_major_version();
    printf("major_version = %s\n", major_version);

    st.set(access, access_stub);
    version_t test = kdk_system_get_version_detaile();
    printf("test.os_version = %s\n", test.os_version);
    printf("test.update_version = %s\n", test.update_version);
    free(test.os_version);
    free(test.update_version);
}

TEST(KdkSysinfoTest, SysinfoInfoStub2)
{
    Stub st;
    st.set(geteuid, geteuid_stub);
    char *res = kdk_system_get_hostCloudPlatform();
    printf("云平台类型：%s\n", res);
    free(res);

    st.set(getenv, getenv_stub);
    char **name = kdk_system_get_resolving_power();
    size_t count = 0;
    if (NULL != name)
    {
        // printf("name = %s\n", name);
        while (name[count])
        {
            printf("No. %d\t %s\n", count + 1, name[count]);
            count++;
        }
        kdk_resolving_freeall(name);
    }

    st.set(realloc, realloc_stub);
    name = kdk_system_get_resolving_power();
    count = 0;
    if (NULL != name)
    {
        // printf("name = %s\n", name);
        while (name[count])
        {
            printf("No. %d\t %s\n", count + 1, name[count]);
            count++;
        }
         kdk_resolving_freeall(name);
    }
}

TEST(KdkSysinfoTest, SysinfoInfo)
{
    char *res = NULL;
    res = kdk_system_get_architecture();
    printf("架构：%s\n", res);
    free(res);

    res = kdk_system_get_systemName();
    printf("系统名称：%s\n", res);
    free(res);

    res = kdk_system_get_version(0);
    printf("系统简略版本：%s\n", res);
    free(res);

    res = kdk_system_get_version(1);
    printf("系统详细版本：%s\n", res);
    free(res);

    res = kdk_system_get_kernelVersion();
    printf("内核版本：%s\n", res);
    free(res);

    res = kdk_system_get_serialNumber();
    printf("序列号：%s\n", res);
    free(res);

    int p;
    int d;
    int act = kdk_system_get_activationStatus(&p, &d);
    printf("--------------\n");
    printf("激活状态码：%d\n", act);
    printf("激活状态：%s\n", act == 1 ? "已激活" : act == 0 ? "未激活"
                                                            : "已过期");

    res = kdk_system_get_eUser();
    printf("当前用户：%s\n", res);
    free(res);

    res = kdk_system_get_eUser_login_time();
    printf("登录时间：%s\n", res);
    free(res);

    res = kdk_system_get_projectName();
    printf("项目编号名：%s\n", res);
    free(res);

    int zyj = kdk_system_is_zyj();
    printf("专用机：%s\n", zyj == 0 ? "非专用机" : "专用机");

    res = kdk_system_get_hostVirtType();
    printf("虚拟机类型：%s\n", res);
    free(res);

    res = kdk_system_get_hostCloudPlatform();
    printf("云平台类型：%s\n", res);
    free(res);

    version_t test = kdk_system_get_version_detaile();
    printf("test.os_version = %s\n", test.os_version);
    printf("test.update_version = %s\n", test.update_version);
    free(test.os_version);
    free(test.update_version);

    char **name = kdk_system_get_resolving_power();
    size_t count = 0;
    if (NULL != name)
    {
        // printf("name = %s\n", name);
        while (name[count])
        {
            printf("No. %d\t %s\n", count + 1, name[count]);
            count++;
        }
        kdk_resolving_freeall(name);
    }

    char *ver = kdk_system_get_systemCategory();
    printf("系统硬件版本类别 = %s\n", ver);
    free(ver);

    char **ud_time = kdk_system_get_startup_time();
    count = 0;
    while (ud_time[count])
    {
        printf("开机时间 = %s\n", ud_time[count]);
        count++;
    }
    kdk_resolving_freeall(ud_time);

    ud_time = kdk_system_get_shutdown_time();
    count = 0;
    while (ud_time[count])
    {
        printf("关机时间 = %s\n", ud_time[count]);
        count++;
    }
    kdk_resolving_freeall(ud_time);

    res = kdk_get_host_vendor();
    printf("整机制造商 = %s\n", res);
    free(res);

    res = kdk_get_host_product();
    printf("整机型号 = %s\n", res);
    free(res);

    res = kdk_get_host_serial();
    printf("整机序列号 = %s\n", res);
    free(res);

    res = kdk_system_get_buildTime();
    printf("构建时间：%s\n", res);
    free(res);

    res = kdk_system_get_hostName();
    printf("主机名：%s\n", res);
    free(res);

    printf("系统位数：%d\n", kdk_system_get_word());

    struct KPci *pci = kdk_hw_get_pci_info();
    struct KPci *tmp = pci;
    while (tmp)
    {
        printf("slot path : %s\n", tmp->slot_path);
        printf("\tclass name :%s\n", tmp->class_name);
        printf("\tproduct name :%s\n", tmp->product_name);
        printf("\trev :%02x\n", tmp->rev);
        printf("\tsubsystem name :%s\n", tmp->ss_name);
        printf("\tdriver user :%s\n", tmp->driver_use);
        printf("\tmodules :");
        for (int i = 0; i < tmp->module_count; i++)
        {
            printf("\t%s", tmp->modules[i]);
        }
        printf("\n");
        tmp = tmp->next;
    }
    kdk_hw_free_pci_info(pci);

    res = kdk_system_get_appScene();
    printf("应用场景：%s\n", res);
    free(res);

    bool status = kdk_system_user_logout_status("kylin");
    printf("status = %d\n", status);

    kdk_system_register_switch_user_handle(callbacke, NULL);
    sleep(5);
    kdk_system_unregister_switch_user_handle();

    char *takeTime = kdk_system_get_startup_takeTime();
    printf("takeTime = %s\n", takeTime);
    free(takeTime);

    printf("系统中文件描述符数：%d\n", kdk_system_get_file_descriptor());
    printf("系统中进程数：%d\n", kdk_system_get_process_nums());
    printf("系统中线程数：%d\n", kdk_system_get_thread_nums());
    kdk_loadavg loadAvg = kdk_system_get_loadavg();
    printf("CPU 负载均衡 1m %0.2f, 5m %0.2f, 15m %0.2f\n", loadAvg.loadavg_1m, loadAvg.loadavg_5m, loadAvg.loadavg_15m);

    printf("grub启动项的json为：%s\n", kdk_system_get_grub_menu());

    printf("系统类型为：%d\n", kdk_system_get_basic_form());

    char *manufacturer = kdk_system_get_system_manufacturer();
    if (manufacturer)
    {
        printf("Manufacturer: %s\n", manufacturer);
        free(manufacturer);
    }

    char *locale = kdk_system_get_system_locale();
    if (locale)
    {
        printf("locale: %s\n", locale);
        free(locale);
    }

    char **envrion = kdk_system_get_env();
    if (envrion)
    {
        int i = 0;
        while (envrion[i])
        {
            printf("%s\n", envrion[i]);
            free(envrion[i]);
            i++;
        }
        free(envrion);
    }

    const char *service_status[] = {"enabled",
                                    "enabled-runtime",
                                    "linked",
                                    "linked-runtime",
                                    "masked",
                                    "masked-runtime",
                                    "static",
                                    "disabled",
                                    "indirect",
                                    "generated",
                                    "transient",
                                    "bad",
                                    "unknown"};
    const char *service_presets[] = {"unknown",
                                     "enable",
                                     "disable"};

    kdk_service_list *list = kdk_system_get_all_service_list();
    printf("Service :\n");
    for (kdk_service_list *tmp = list; tmp; tmp = tmp->next)
    {
        printf("\t%s %s %s\n", tmp->name, tmp->status == -1 ? service_status[K__UNIT_FILE_STATE_MAX - 1] : service_status[tmp->status], service_presets[tmp->vendorpreset]);
    }
    kdk_system_free_service_list(list);

    list = kdk_system_get_automatic_start_service_list();
    printf("Auto matic service :\n");
    for (kdk_service_list *tmp = list; tmp; tmp = tmp->next)
    {
        printf("\t%s %s %s\n", tmp->name, tmp->status == -1 ? service_status[K__UNIT_FILE_STATE_MAX - 1] : service_status[tmp->status], service_presets[tmp->vendorpreset]);
    }
    kdk_system_free_service_list(list);

    MachineType machine_type = kdk_system_get_machine_type();
    printf("%d\n", machine_type);

    const char *line = kdk_system_get_production_line();
    printf("line = %s\n", line);

    const char *major_version = kdk_system_get_major_version();
    printf("major_version = %s\n", major_version);

    const char *minor_version = kdk_system_get_minor_version();
    printf("minor_version = %s\n", minor_version);

    const char *alias = kdk_system_get_version_alias();
    printf("alias = %s\n", alias);

    const char *serial_name = kdk_system_get_serial_name();
    printf("serial_name = %s\n", serial_name);

    const kdk_production_inner_info info = kdk_system_get_production_inner_version();
    printf("production line = %s\n", info.production_line);
    printf("major version = %s\n", info.major_ver);
    printf("minor version = %s\n", info.minor_ver);
    printf("version alias = %s\n", info.alias);
    printf("serial name = %s\n", info.serial_name);

    const kdk_custom_info custom = kdk_system_get_custom_version();
    printf("is customized = %s\n", custom.version_type);
    printf("customized type = %s\n", custom.custom_type);

    char *projectSubName = kdk_system_get_projectSubName();
    printf("projectSubName = %s\n", projectSubName);

    unsigned int productFeatures = kdk_system_get_productFeatures();
    printf("productFeatures = %d\n", productFeatures);
}

extern void *malloc_stub (size_t __size)
{
    return nullptr;
}

TEST(KdkSysinfoTest, SysinfoInfoStub3)
{
    Stub st;
    st.set(realloc, realloc_stub);
    char **ud_time = kdk_system_get_startup_time();
    size_t count = 0;
    while (ud_time[count])
    {
        printf("开机时间 = %s\n", ud_time[count]);
        count++;
    }

    ud_time = kdk_system_get_shutdown_time();
    count = 0;
    while (ud_time[count])
    {
        printf("关机时间 = %s\n", ud_time[count]);
        count++;
    }
}

dbus_bool_t   dbus_message_is_signal_stub (DBusMessage   *message, const char    *iface, const char    *signal_name)
{
    return true;
}

TEST(KdkSysinfoTest, SysinfoInfoStub1)
{
    Stub st;
    st.set(dbus_message_new_method_call, dbus_message_new_method_call_stub);
    printf("系统类型为：%d\n", kdk_system_get_basic_form());

    char *res = NULL;
    res = kdk_get_host_serial();
    printf("整机序列号 = %s\n", res);
    free(res);

    st.set(fopen, fopen_stub);
    const kdk_custom_info custom = kdk_system_get_custom_version();
    printf("is customized = %s\n", custom.version_type);
    printf("customized type = %s\n", custom.custom_type);

    bool status = kdk_system_user_logout_status("kylin");
    printf("status = %d\n", status);
} 