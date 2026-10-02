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

#include "../../packages/libkypackages.h"
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "../cpp-stub/src/stub.h"

static void print_package(kdk_package_t *package)
{
    if (NULL == package)
        return;
    printf("包名：%s\t", package->name);
    printf("版本号：%s\t", package->version);
    printf("包类型：%s\t", package->section);
    printf("包状态：%s\t", package->status);
    printf("包大小：%lu\n", package->size_kb);
}

void callbacke(int progress, int state, char *loginfo)
{
    printf("progress = %d,  state = %d, loginfo = %s\n", progress, state, loginfo);
}

void install_call_back_install(int progress, int state, char *loginfo)
{
    printf("progress = %d,  state = %d, loginfo = %s\n", progress, state, loginfo);
}

bool callback = false;

void on_finished(int state, const char *standard_output, const char *standard_err)
{
    printf("state = %d,  standard_output = %s, standard_err = %s\n", state, standard_output, standard_err);
    callback = true;
}

void on_update(const char *output)
{
    printf("output = %s\n", output);
}

extern int access_stub(const char *__name, int __type)
{
    return 0;
}

TEST(KdkPackageTest, PackageInfoStub)
{
    Stub st;
    st.set(access, access_stub);

    kdk_package_list *list = kdk_package_get_packagelist();
    printf("系统中一共%u个软件包\n", list->nums);
    for (size_t i = 0; i < list->nums; i++)
    {
        print_package(list->list[i]);
    }
    kdk_package_free_packagelist(list);

    InstallMethod ret = kdk_package_get_installation_method("scram-gui", "0.16.2-1build2");
    switch (ret)
    {
    case DPKG_METHOD:
        printf("Install method: %s", "dpkg");
        break;
    case KM_METHOD:
        printf("Install method: %s", "km");
        break;
    case KARE_METHOD:
        printf("Install method: %s", "kare");
        break;
    case KMRE_METHOD:
        printf("Install method: %s", "kmre");
        break;
    case KWRE_METHOD:
        printf("Install method: %s", "kwre");
        break;
    default:
        printf("Install method: %s", "None");
        break;
    }

    printf("top.openkylin.Sdk是否安装：%s\n", kdk_package_is_installed("libkysdk-log", NULL) == 1 ? "是" : "否");
    char *version = kdk_package_get_version("libkysdk-log");
    printf("Evolution版本号：%s\n", version);
    free(version);

    char *description = kdk_package_get_description("aapt");
    if (description != NULL)
    {
        printf("%s\n", description);
        free(description);
    }

    int res;

    res = kdk_package_install_package_online("top.openkylin.PermissionManager", KDK_PACKAGE_KAIMING, install_call_back_install);
    printf("res = %d\n", res);

	res = kdk_package_install_package_online("d-feet", KDK_PACKAGE_DEBIAN, install_call_back_install);
    printf("d-feet res = %d\n", res);

    res = kdk_package_install_package_offline("/home/kylin/tcpdump_4.99.4-ok1_amd64.deb", KDK_PACKAGE_DEBIAN, install_call_back_install);
    printf("tcpdump res = %d\n", res);

	res = kdk_package_install_package_offline("/home/kylin/iputils-ping_3%3a20240117-ok1.0_amd64.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("iputils-ping res = %d\n", res);

	res = kdk_package_install_package_offline("/home/kylin/桌面/top.openkylin.Notebook-x86_64-3.2.1.1-ok8.3-stable.ok", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("Notebook res = %d\n", res);

	version = kdk_package_get_version("top.openkylin.Notebook");
	printf("top.openkylin.Notebook版本号：%s\n", version);
    free(version);

	version = kdk_package_get_version("d-feet");
	printf("d-feet版本号：%s\n", version);
    free(version);

    res = kdk_package_remove_package("top.openkylin.PermissionManager", callbacke);
    printf("res = %d\n", res);
	
	res = kdk_package_remove_package("d-feet", callbacke);
    printf("res = %d\n", res);

	res = kdk_package_remove_package("tcpdump", callbacke);
    printf("res = %d\n", res);
}

TEST(KdkPackageTest, PackageInfo)
{
    kdk_package_list *list = kdk_package_get_packagelist();
    printf("系统中一共%u个软件包\n", list->nums);
    for (size_t i = 0; i < list->nums; i++)
    {
        print_package(list->list[i]);
    }
    kdk_package_free_packagelist(list);

    InstallMethod ret = kdk_package_get_installation_method("kylin-video", "3.1.4.2k1-ok26.21k0.18");
    switch (ret)
    {
    case DPKG_METHOD:
        printf("Install method: %s", "dpkg");
        break;
    case KM_METHOD:
        printf("Install method: %s", "km");
        break;
    case KARE_METHOD:
        printf("Install method: %s", "kare");
        break;
    case KMRE_METHOD:
        printf("Install method: %s", "kmre");
        break;
    case KWRE_METHOD:
        printf("Install method: %s", "kwre");
        break;
    default:
        printf("Install method: %s", "None");
        break;
    }

    printf("top.openkylin.Sdk是否安装：%s\n", kdk_package_is_installed("libkysdk-log", NULL) == 1 ? "是" : "否");
    char *version = kdk_package_get_version("libkysdk-log");
    printf("Evolution版本号：%s\n", version);
    free(version);

    char **dir_list = kdk_package_get_code_path("kylin-video");
    if (NULL != dir_list)
    {
        int i = 0;
        while (dir_list[i])
        {
            printf("%s\n", dir_list[i++]);
        }
        for (int i = 0; dir_list[i]; i++)
        {
            free(dir_list[i]);
        }
        free(dir_list);
    }

    printf("%d\n", kdk_package_get_file_count("kylin-video"));

    printf("%d\n", kdk_package_verify_disk_space("/home/kylin/tcpdump_4.99.4-ok1_amd64.deb"));

    printf("%s\n", kdk_package_get_default_browser() ? kdk_package_get_default_browser() : "get browser failed");
    printf("%s\n", kdk_package_get_default_image_viewer() ? kdk_package_get_default_image_viewer() : "get image failed");
    printf("%s\n", kdk_package_get_default_audio_player() ? kdk_package_get_default_audio_player() : "get audio failed");
    printf("%s\n", kdk_package_get_default_video_player() ? kdk_package_get_default_video_player() : "get video failed");
    printf("%s\n", kdk_package_get_default_pdf_viewer() ? kdk_package_get_default_pdf_viewer() : "get pdf failed");
    printf("%s\n", kdk_package_get_default_word_viewer() ? kdk_package_get_default_word_viewer() : "get word failed");
    printf("%s\n", kdk_package_get_default_excel_viewer() ? kdk_package_get_default_excel_viewer() : "get execl failed");
    printf("%s\n", kdk_package_get_default_ppt_viewer() ? kdk_package_get_default_ppt_viewer() : "get ppt failed");

    kdk_app_info *start_menu_list = kdk_package_get_startmenu_list();
    if (NULL != start_menu_list)
    {
        for (int i = 0; i < start_menu_list->nums; i++)
        {
            kdk_app_t *tmp = start_menu_list->list[i];
            printf("%s\n", tmp->cmd);
            printf("%s\n", tmp->company);
            printf("%s\n", tmp->icon);
            printf("%s\n", tmp->name);
            printf("%s\n", tmp->param);
            printf("%s\n", tmp->version);
            printf("\n");
        }
        kdk_package_free_startmenu_list(start_menu_list);
    }

    kdk_app_info *app_list = kdk_package_get_application_list();
    if (NULL != app_list)
    {
        for (int i = 0; i < app_list->nums; i++)
        {
            kdk_app_t *tmp = app_list->list[i];
            printf("%s\n", tmp->cmd);
            printf("%s\n", tmp->company);
            printf("%s\n", tmp->icon);
            printf("%s\n", tmp->name);
            printf("%s\n", tmp->param);
            printf("%s\n", tmp->version);
            printf("\n");
        }
        kdk_package_free_app_info(app_list);
    }

    int res;
    res = kdk_package_install_package_online("top.openkylin.PermissionManager=1.0.0.0", KDK_PACKAGE_KAIMING, install_call_back_install);
    printf("WeChat res = %d\n", res);

	res = kdk_package_install_package_online("d-feet=0.3.16-ok1", KDK_PACKAGE_DEBIAN, install_call_back_install);
    printf("d-feet res = %d\n", res);

	res = kdk_package_install_package_online("traceroute=1:2.1.5-ok1", KDK_PACKAGE_AUTO, install_call_back_install);
	printf("iperf res = %d\n", res);

    res = kdk_package_install_package_online("wps-office", KDK_PACKAGE_AUTO, install_call_back_install);
	printf("iperf res = %d\n", res);

    res = kdk_package_install_package_offline("/home/kylin/tcpdump_4.99.4-ok1_amd64.deb", KDK_PACKAGE_DEBIAN, install_call_back_install);
    printf("tcpdump res = %d\n", res);

	res = kdk_package_install_package_offline("/home/kylin/桌面/V11安装包/X86/wemeet_3.19.2.400_amd64.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("wemeet res = %d\n", res);

    res = kdk_package_install_package("/home/kylin/telnet_0.17+2.5-ok1_all.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("wemeet res = %d\n", res);

    res = kdk_package_install_package("/home/kylin/桌面/V11安装包/X86/wechat-beta_4.0.0.21_amd64.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("wemeet res = %d\n", res);

    res = kdk_package_install_package_offline("/home/kylin/baobab_45.0-ok1_amd64.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("wemeet res = %d\n", res);

	res = kdk_package_install_package_offline("/home/kylin/桌面/top.openkylin.Notebook-x86_64-3.2.1.1-ok8.3-stable.ok", KDK_PACKAGE_AUTO, install_call_back_install);
    printf("Notebook res = %d\n", res);

	version = kdk_package_get_version("top.openkylin.Notebook");
	printf("top.openkylin.Notebook版本号：%s\n", version);
    free(version);

	version = kdk_package_get_version("d-feet");
	printf("d-feet版本号：%s\n", version);
    free(version);

    printf("top.openkylin.Sdk是否安装：%s\n", kdk_package_is_installed("top.openkylin.Notebook", NULL) == 1 ? "是" : "否");

    printf("top.openkylin.Sdk是否安装：%s\n", kdk_package_is_installed("wemeet", NULL) == 1 ? "是" : "否");

    char *description = kdk_package_get_description("kylin-video");
    if (description != NULL)
    {
        printf("%s\n", description);
        free(description);
    }

	ret = kdk_package_get_installation_method("top.openkylin.Notebook", "3.2.1.1-ok8.3");
    switch (ret)
    {
    case DPKG_METHOD:
        printf("Install method: %s", "dpkg");
        break;
    case KM_METHOD:
        printf("Install method: %s", "km");
        break;
    case KARE_METHOD:
        printf("Install method: %s", "kare");
        break;
    case KMRE_METHOD:
        printf("Install method: %s", "kmre");
        break;
    case KWRE_METHOD:
        printf("Install method: %s", "kwre");
        break;
    default:
        printf("Install method: %s", "None");
        break;
    }

	ret = kdk_package_get_installation_method("wemeet", "3.19.2.400");
    switch (ret)
    {
    case DPKG_METHOD:
        printf("Install method: %s", "dpkg");
        break;
    case KM_METHOD:
        printf("Install method: %s", "km");
        break;
    case KARE_METHOD:
        printf("Install method: %s", "kare");
        break;
    case KMRE_METHOD:
        printf("Install method: %s", "kmre");
        break;
    case KWRE_METHOD:
        printf("Install method: %s", "kwre");
        break;
    default:
        printf("Install method: %s", "None");
        break;
    }

	ret = kdk_package_get_installation_method("tcpdump", "4.99.4-ok1");
    switch (ret)
    {
    case DPKG_METHOD:
        printf("Install method: %s", "dpkg");
        break;
    case KM_METHOD:
        printf("Install method: %s", "km");
        break;
    case KARE_METHOD:
        printf("Install method: %s", "kare");
        break;
    case KMRE_METHOD:
        printf("Install method: %s", "kmre");
        break;
    case KWRE_METHOD:
        printf("Install method: %s", "kwre");
        break;
    default:
        printf("Install method: %s", "None");
        break;
    }

	char *name = kdk_package_get_name("/usr/share/applications/kylin-installer.desktop");
	printf("name = %s\n", name);

	name = kdk_package_get_name("/opt/kare/usr/share/applications/org.gnome.baobab.desktop");
	printf("name = %s\n", name);

	bool removable = kdk_package_is_removable("kylin-installer");
	printf("removable = %d\n", removable);

	removable = kdk_package_is_removable("tcpdump");
	printf("removable = %d\n", removable);

    res = kdk_package_remove_package("top.openkylin.PermissionManager", callbacke);
    printf("res = %d\n", res);
	
	res = kdk_package_remove_package("d-feet", callbacke);
    printf("res = %d\n", res);

	res = kdk_package_remove_package("tcpdump", callbacke);
    printf("res = %d\n", res);

    res = kdk_package_remove_package("wemeet", callbacke);
    printf("res = %d\n", res);

    bool remove = kdk_package_is_removable_by_desktop("/opt/compatibility_layer/kaiming/usr/share/applications/todesk.desktop");
    printf("是否可卸载：%d\n", remove);

    remove = kdk_package_is_removable_by_desktop("/opt/kare/usr/share/applications/org.gnome.baobab.desktop");
    printf("是否可卸载：%d\n", remove);

    kdk_cmd_node *node = kdk_package_cmd_init("telnet www.baidu.com 80");
    node->finished = on_finished;  // 设置命令完成时的回调
    node->update = on_update;

    int cmd = kdk_package_launch_cmd(node);
    printf("cmd = %d\n", cmd);

	sleep(20);
    kdk_package_cmd_close(node);
	sleep(20);
/*
    while (1)
    {
        if(callback == true)
        {
            break;
        }
    }
*/
    kdk_package_free_cmd(node);

    kdk_cmd_node *init = kdk_package_cmd_init("ifconfig");
    init->finished = on_finished;  // 设置命令完成时的回调
    init->update = on_update;

    cmd = kdk_package_launch_cmd(init);
    printf("cmd = %d\n", cmd);

	sleep(20);
    kdk_package_cmd_close(init);
	sleep(20);

    kdk_package_free_cmd(init);
}
