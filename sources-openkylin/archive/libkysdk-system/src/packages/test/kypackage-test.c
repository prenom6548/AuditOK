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

#include "../libkypackages.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

int main()
{
    // kdk_package_list *list = kdk_package_get_packagelist();
    // printf("系统中一共%u个软件包\n", list->nums);
    // for (size_t i = 0; i < list->nums; i ++)
    // {
    //     print_package(list->list[i]);
    // }
    // kdk_package_free_packagelist(list);

    // InstallMethod ret = kdk_package_get_installation_method("depends-dbus", "1.0.0.0-7");
    // switch (ret)
    // {
    // case DPKG_METHOD:
    //     printf("Install method: %s\n", "dpkg");
    //     break;
    // case KM_METHOD:
    //     printf("Install method: %s\n", "km");
    //     break;
    // case KARE_METHOD:
    //     printf("Install method: %s\n", "kare");
    //     break;
    // case KMRE_METHOD:
    //     printf("Install method: %s\n", "kmre");
    //     break;
    // case KWRE_METHOD:
    //     printf("Install method: %s\n", "kwre");
    //     break;
    // default:
    //     printf("Install method: %s\n", "None");
    //     break;
    // }

    // printf("top.openkylin.Sdk是否安装：%s\n", kdk_package_is_installed("top.openkylin.Sdk", NULL) == 0 ? "是" : "否");
    // char *version = kdk_package_get_version("evolution");
    // printf("Evolution版本号：%s\n", version);
    // free(version);

    // char *description = kdk_package_get_description("aapt");
    // if(description != NULL)
    // {
    //     printf("%s\n", description);
    //     free(description);
    // }

    // char **dir_list = kdk_package_get_code_path("aapt");
    // if(NULL != dir_list)
    // {
    //     int i = 0;
    //     while (dir_list[i])
    //     {
    //         printf("%s\n", dir_list[i++]);
    //     }
    //     for(int i = 0; dir_list[i]; i++)
    //     {
    //         free(dir_list[i]);
    //     }
    //     free(dir_list);
    // }
    

    // printf("%d\n", kdk_package_get_file_count("aapt"));

    // printf("%d\n", kdk_package_verify_disk_space("/home/kylin/systemd_245.4-4kylin3.20k0.17oemhwy0.4.u_arm64.deb"));

    // printf("%s\n", kdk_package_get_default_browser() ? kdk_package_get_default_browser() : "get browser failed");
    // printf("%s\n", kdk_package_get_default_image_viewer() ? kdk_package_get_default_image_viewer() : "get image failed");
    // printf("%s\n", kdk_package_get_default_audio_player() ? kdk_package_get_default_audio_player() : "get audio failed");
    // printf("%s\n", kdk_package_get_default_video_player() ? kdk_package_get_default_video_player() : "get video failed");
    // printf("%s\n", kdk_package_get_default_pdf_viewer() ? kdk_package_get_default_pdf_viewer() : "get pdf failed");
    // printf("%s\n", kdk_package_get_default_word_viewer() ? kdk_package_get_default_word_viewer() : "get word failed");
    // printf("%s\n", kdk_package_get_default_excel_viewer() ? kdk_package_get_default_excel_viewer() : "get execl failed");
    // printf("%s\n", kdk_package_get_default_ppt_viewer() ? kdk_package_get_default_ppt_viewer() : "get ppt failed");

    // kdk_app_info *start_menu_list = kdk_package_get_startmenu_list();
    // if (NULL != start_menu_list)
    // {
    //     for (int i = 0; i < start_menu_list->nums; i++)
    //     {
    //         kdk_app_t *tmp = start_menu_list->list[i];
    //         printf("%s\n", tmp->cmd);
    //         printf("%s\n", tmp->company);
    //         printf("%s\n", tmp->icon);
    //         printf("%s\n", tmp->name);
    //         printf("%s\n", tmp->param);
    //         printf("%s\n", tmp->version);
    //         printf("\n");
    //     }
    //     kdk_package_free_app_info(start_menu_list);
    // }

    // kdk_app_info *app_list = kdk_package_get_application_list();
    // if (NULL != app_list)
    // {
    //     for (int i = 0; i < app_list->nums; i++)
    //     {
    //         kdk_app_t *tmp = app_list->list[i];
    //         printf("%s\n", tmp->cmd);
    //         printf("%s\n", tmp->company);
    //         printf("%s\n", tmp->icon);
    //         printf("%s\n", tmp->name);
    //         printf("%s\n", tmp->param);
    //         printf("%s\n", tmp->version);
    //         printf("\n");
    //     }
    //     kdk_package_free_app_info(app_list);
    // }

    // int ret;
    // ret =  kdk_package_remove_package("com.watchdata.authtool", callbacke);
    // printf("ret = %d\n", ret);

    // kdk_package_unregister_remove_package_handle();

    // ret = kdk_package_install_package_online("raincat", KDK_PACKAGE_DEBIAN, install_call_back_install);
    // printf("ret = %d\n", ret);  

    // kdk_package_unregister_install_package_handle();

    // ret = kdk_package_install_package_offline("/home/kylin/桌面/tool/depends-tool_1.0.0.0-1_all.deb", KDK_PACKAGE_KARE, install_call_back_install);
    // printf("ret = %d\n", ret);

    // kdk_package_unregister_install_package_handle();

    // ret = kdk_package_install_package("/home/zm/下载/fcitx-table-wubi_1%3a4.2.9.8-3kylin5k1.18_all.deb", KDK_PACKAGE_AUTO, install_call_back_install);
    // printf("11111111ret = %d\n", ret);
    // kdk_package_unregister_install_package_handle();

    // char* pkg_name = kdk_package_get_name("/usr/share/applications/todesk.desktop");
    // printf("pkg_name = %s\n", pkg_name);

    // bool removable =  kdk_package_is_removable("todesk");
    // printf("是否可卸载：%d\n", removable);

    // bool remove = kdk_package_is_removable_by_desktop("/opt/compatibility_layer/kaiming/usr/share/applications/todesk.desktop");
    // printf("是否可卸载：%d\n", remove);



    // kdk_cmd_node *node = kdk_package_cmd_init("telnet www.baidu.com 80");
    // node->finished = on_finished;  // 设置命令完成时的回调
    // node->update = on_update;

    // int cmd = kdk_package_launch_cmd(node);
    // printf("cmd = %d\n", cmd);

	// sleep(20);
    // kdk_package_cmd_close(node);
	// printf("111\n");
	// sleep(20);
/*
    while (1)
    {
        if(callback == true)
        {
            break;
        }
    }
*/
    // kdk_package_free_cmd(node);

    // char **path = kdk_package_get_standard_path("PEONY_PLUGIN_INSTALL_DIRS");
    // size_t num = 0;
    // if(path)
    // {
    //     while (path[num])
    //     {
    //         printf("path = %s\n", path[num]);
    //         num++;
    //     }
    //     kdk_package_freeall(path);
    // }

    // char **list = kdk_package_get_standard_path_lists();
    // num = 0;
    // if(list)
    // {
    //     while (list[num])
    //     {
    //         printf("key list = %s\n", list[num]);
    //         num++;
    //     }
    //     kdk_package_freeall(list);
    // }
    
    // char **list_files = kdk_package_list_files("libkysdk-accounts");
    // num = 0;
    // if(list_files)
    // {
    //     while (list_files[num])
    //     {
    //         printf("list files = %s\n", list_files[num]);
    //         num++;
    //     }
    // }

    // char *pkg = kdk_package_search_by_file("/usr/bin/ls");
    // if(pkg)
    // {
    //     printf("pkg name = %s\n", pkg);
    //     free(pkg);
    // }

    bool installable = kdk_package_is_online_installable("cn.kylin.hplip", "3.25.2.2");
    printf("cn.kylin.aztclient 是否可在线安装： %s, installable = %d\n", installable ? "是" : "否", installable);

    // kdk_package_interrupt_signal_handler();

    // int dkms_offline = kdk_package_install_tag_package_offline("/home/zm/下载/actioncompat/actioncompat/hplip_3.25.2.2_arm64.deb", "dkms", "shadow", install_call_back_install);
    // printf("dkms_offline = %d\n", dkms_offline);

    // int dkms_online = kdk_package_install_tag_package_online("hplip", "3.25.2.2", "inputMethod", NULL, install_call_back_install);
    // printf("dkms_online = %d\n", dkms_online);

    // unsigned long size = kdk_package_get_package_size("com.watchdata.authtool");
    // printf("size = %ld\n", size);

    return 0;
}
