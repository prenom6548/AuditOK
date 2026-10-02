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

#ifndef KDK_SYSTEM_PACKAGES_H__
#define KDK_SYSTEM_PACKAGES_H__

/**
 * @file libkypackages.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 包列表信息
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-package
 * @{
 * 
 * @since 1.1.0
 */
// typedef enum kdk_packageStatus {
//     UNINIT,
//     INSTALLED,
//     UNCONFIG,
//     NONE
// }kdk_packageStatus;
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

typedef enum _InstallMethod
{
    NONE_METHOD,
    DPKG_METHOD,
    KM_METHOD,
    KARE_METHOD,
    KMRE_METHOD,
    KWRE_METHOD
} InstallMethod;

typedef struct _kdk_package_t{
    char *name;     // 包名
    char *version;  // 版本号
    char *section;  // 包类型
    char *status;   // 状态
    unsigned long size_kb;   // 包大小
    struct _kdk_package_t **depends; // 依赖包
}kdk_package_t;

typedef struct {
    unsigned int nums;
    kdk_package_t **list;
}kdk_package_list;

typedef struct _kdk_app_t
{
    char *name;    // 程序名称
    char *version; // 版本号
    char *company; // 所属公司
    char *cmd;     // 可执行文件路径
    char *param;   // 启动参数
    char *icon;    // 图标信息
} kdk_app_t;

typedef struct _kdk_app_info
{
    unsigned int nums;      //list成员个数
    kdk_app_t **list; //开始菜单应用信息数组
} kdk_app_info;

typedef enum _KDK_PACKAGE_LIST_MODE
{
    KDK_PACKAGE_AUTO = 0,
    KDK_PACKAGE_KARE,
    KDK_PACKAGE_KAIMING,
    KDK_PACKAGE_DEBIAN
} KDK_PACKAGE_LIST_MODE;

//定义回调函数
typedef void (*FinishedCallBack)(int state, const char *standard_output, const char *standard_err);
typedef void (*UpdateCallBack)(const char *output);

typedef struct _kdk_cmd_node
{
    char cmd[512];                      // cmd
    FinishedCallBack finished;          // 回调函数-结束
    UpdateCallBack update;              // 回调函数-更新
    int stat;                           // 1为debian安装，0为kare安装
}kdk_cmd_node;

/**
 * @brief 获取系统中所有包列表
 * 
 * @return kdk_package_list* 包描述结构体列表；返回的结构体需要由kdk_package_free_packagelist()回收
 */
extern kdk_package_list* kdk_package_get_packagelist();

/**
 * @brief 回收由kdk_package_get_packagelist()返回的结构体
 * 
 * @param list 由kdk_package_get_packagelist()返回的结构体
 */
extern void kdk_package_free_packagelist(kdk_package_list *list);

/**
 * @brief 判断指定包的安装方式，返回值为枚举类型
 * 
 * @param name 包名
 * @param version 版本
 * @return InstallMethod 安装模式枚举值
 * @since 2.5.1.2
 */
InstallMethod kdk_package_get_installation_method(const char *name, const char *version);

/**
 * @brief 安装指定包回调函数
*/
typedef void (*InstallCallBack)(int progress, int state, char *loginfo);

/**
 * @brief 离线安装指定的软件包
 * 
 * @param uri 软件包路径
 * @param list_mode 指定软件包安装方式，当不确定具体安装途径时，传0，则按自动推导安装途径安装。
 * @param call_back 回调函数，用于获取软件包安装进度和中间状态
 * 
 * @return int 成功返回1，失败返回0
 * @since 2.5.1.2
 */
extern int kdk_package_install_package_offline(const char *uri, int list_mode, InstallCallBack call_back);

/**
 * @brief 在线安装指定的软件包
 * 
 * @param pkg_name 软件包名
 * @param list_mode 指定软件包安装方式，当不确定具体安装途径时，传0，则按自动推导安装途径安装。
 * @param call_back 回调函数，用于获取软件包安装进度和中间状态
 * 
 * @return int 成功返回1，失败返回0
 * @since 2.5.1.2
 */
extern int kdk_package_install_package_online(const char *pkg_name, int list_mode, InstallCallBack call_back);

/**
 * @brief 通用安装指定的软件包
 * 
 * @param pkg_name 传入软件包名则进行在线安装，传入本地软件包路径则进行离线安装
 * @param list_mode 指定软件包安装方式，当不确定具体安装途径时，传0，则按自动推导安装途径安装。
 * @param call_back 回调函数，用于获取软件包安装进度和中间状态
 * 
 * @return int 成功返回1，失败返回0
 * @since 2.5.1.2
 */
extern int kdk_package_install_package(const char *uri, int list_mode, InstallCallBack call_back);

/**
 * @brief 注销获取软件包安装进度和中间状态的回调函数
 * @since 2.5.1.2
*/
extern void kdk_package_unregister_install_package_handle();

/**
 * @brief 卸载指定包回调函数
*/
typedef void (*RemoveCallBack)(int progress, int state, char *loginfo);

/**
 * @brief 删除指定的包
 * 
 * @param name 包名
 * @param call_back 回调函数，用于获取软件包卸载进度和中间状态
 * @return int 成功返回1，失败返回0
 * @since 2.5.1.2
 */
extern int kdk_package_remove_package(const char *pkg_name, RemoveCallBack call_back);

/**
 * @brief 注销获取软件包卸载进度和中间状态的回调函数
 * @since 2.5.1.2
*/
extern void kdk_package_unregister_remove_package_handle();

/**
 * @brief 获取系统中指定包的版本号
 * 
 * @param name 软件包名
 * @return char* 版本号，由alloc生成，需要被free；若给定的包不存在，则返回NULL
 */
extern char* kdk_package_get_version(const char *name);

/**
 * @brief 检测指定包名的软件包是否正确安装
 * 
 * @param name 包名
 * @param version 版本号，大部分情况下为NULL，预留为snap做准备
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_package_is_installed(const char *name, const char *version);

/**
 * @brief 获取指定应用的描述
 * 
 * @param name 应用名
 * @return char* 应用的描述信息
 * @since 2.4.1.0
 */
extern char *kdk_package_get_description(const char *name);

/**
 * @brief 获取指定应用的安装目录路径
 * 
 * @param name 应用名
 * @return char** 应用的安装目录路径
 * @since 2.4.1.0
 */
extern char **kdk_package_get_code_path(const char *name);

/**
 * @brief 获取指定应用的安装文件总数
 * 
 * @param name 应用名
 * @return int 应用的安装文件总数
 * @since 2.4.1.0
 */
extern int kdk_package_get_file_count(const char *name);

/**
 * @brief 判断软件包是否可卸载
 * 
 * @param pkg_name 软件包名
 * @return bool true-可卸载；false-不可卸载
 * @since 2.5.1.2
 */
extern bool kdk_package_is_removable(const char *pkg_name);

/**
 * @brief 判断安装deb包是否有足够的磁盘空间
 * 
 * @param path deb包路径
 * @return int 布尔值 1 true； 0 false
 * @since 2.4.1.0
 */
extern int kdk_package_verify_disk_space(const char *path);

/**
 * @brief 获取默认浏览器名称
 * 
 * @return char* 默认浏览器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_browser(void);

/**
 * @brief 获取默认图片查看器名称
 * 
 * @return char* 默认图片查看器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_image_viewer(void);

/**
 * @brief 获取默认音频播放器名称
 * 
 * @return char* 默认音频播放器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_audio_player(void);

/**
 * @brief 获取默认视频播放器名称
 * 
 * @return char* 默认视频播放器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_video_player(void);

/**
 * @brief 获取默认PDF文档查看器名称
 * 
 * @return char* 默认PDF文档查看器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_pdf_viewer(void);

/**
 * @brief 获取默认WORD文档查看器名称
 * 
 * @return char* 默认WORD文档查看器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_word_viewer(void);

/**
 * @brief 获取默认EXCEL文档查看器名称
 * 
 * @return char* 默认EXCEL文档查看器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_excel_viewer(void);

/**
 * @brief 获取默认PPT文档查看器名称
 * 
 * @return char* 默认PPT文档查看器名称
 * @since 2.4.1.0
 */
extern char* kdk_package_get_default_ppt_viewer(void);

/**
 * @brief 获取开始菜单中的所有应用信息
 * 
 * @return kdk_app_info* 开始菜单中的所有应用信息
 * @since 2.4.1.0
 */
extern kdk_app_info *kdk_package_get_startmenu_list(void);

/**
 * @brief 释放kdk_app_info链表
 * 
 * @param list kdk_app_info链表
 * @since 2.4.1.0
 */
extern kdk_app_info *kdk_package_free_startmenu_list(kdk_app_info *list);

/**
 * @brief 获取所有应用信息
 * 
 * @return kdk_app_info* 所有应用信息
 * @since 2.5.1.2
 */
extern kdk_app_info* kdk_package_get_application_list(void);

/**
 * @brief 释放kdk_app_info链表
 * 
 * @param list kdk_app_info链表
 * @since 2.5.1.2
 */
extern void kdk_package_free_app_info(kdk_app_info *list);

/**
 * @brief 获取软件包名
 * 
 * @param desktop desktop文件路径
 * @return char * 软件包名
 * @since 2.5.1.2
 */
extern char* kdk_package_get_name(char *desktop);

/**
 * @brief 判断是否可卸载
 * 
 * @param desktop desktop文件路径
 * @return bool true-可卸载；false-不可卸载
 * @since 2.5.1.2
 */
extern bool kdk_package_is_removable_by_desktop(const char *desktop);

/**
 * @brief 初始化
 * 
 * @param cmd 命令
 * @return kdk_cmd_node 结构体
 * @since 2.5.1.2
 */
extern kdk_cmd_node* kdk_package_cmd_init(const char *cmd);

/**
 * @brief 执行命令（异步执行，不阻塞，通过回调函数finished知晓是否同步完成）
 * 
 * @param node kdk_cmd_node 结构体
 * @return int 成功返回0，失败返回-1
 * @since 2.5.1.2
 */
extern int kdk_package_launch_cmd(kdk_cmd_node* node);

/**
 * @brief 释放kdk_cmd_node 结构体
 * 
 * @param node kdk_cmd_node 结构体
 * @since 2.5.1.2
 */
extern void kdk_package_free_cmd(kdk_cmd_node* node);

/**
 * @brief 关闭进程
 * 
 * @param node kdk_cmd_node 结构体
 * @since 2.5.1.2
 */
extern void kdk_package_cmd_close(kdk_cmd_node* node);

/**
 * @brief 获取安装文件路径
 * @param name 路径名称字符串，如“KYLIN_USER_GUIDE_FILE_PATH”
 * 
 * @return char ** 成功返回配置文件中的路径列表，需要释放。失败返回NULL。
 * @since 3.0.1.0
 */
extern char **kdk_package_get_standard_path(const char *name);

/**
 * @brief 获取安装文件路径名列表
 * 
 * @return char ** 成功返回配置文件中的路径名列表，需要释放。失败返回NULL。
 * @since 3.0.1.0
 */
extern char **kdk_package_get_standard_path_lists();

/**
 * @brief 获取软件包的已安装文件接口
 * @param pkg_name 待查询的软件包名，如“ukui-control-center”
 * 
 * @return char ** 成功返回软件包的资源文件，需要释放。失败返回NULL。
 * @since 3.0.1.0
 */
extern char **kdk_package_list_files(const char *pkg_name);

/**
 * @brief 文件到包的转换接口
 * @param file_name 待查询的文件名，如“/usr/share/applications/ukui-control-center.desktop”；为文件绝对路径
 * 
 * @return char * 成功返回存在文件名的软件包名，需要释放。失败返回NULL
 * @since 3.0.1.0
 */
extern char *kdk_package_search_by_file(const char *file_name);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern void kdk_package_freeall(char **ptr);

/**
 * @brief 判断包是否可在线安装
 * @param name 软件包名
 * @param version 版本号，可能为空值NULL，空值相当于不指定版本
 * 
 * @return bool true-可安装；false-不可安装
 * @since 3.0.1.0
 */
extern bool kdk_package_is_online_installable(const char *name, const char *version);

/**
 * @brief 在线安装带有标签的指定安装包
 * @param pkg_name 软件包名
 * @param version 版本号，可能为空值NULL，空值相当于不指定版本
 * @param type 包类型标签
 * @param compat 兼容版本标签
 * @param call_back 回调函数，用于获取软件包安装进度和中间状态
 * 
 * @return int 成功返回1，失败返回0
 * @since 3.0.1.0
 */
extern int kdk_package_install_tag_package_online(const char *pkg_name, const char *version, const char *type, const char *compat, InstallCallBack call_back);

/**
 * @brief 离线安装带有标签的指定安装包
 * @param uri 软件包路径
 * @param type 包类型标签
 * @param compat 兼容版本标签
 * @param call_back 回调函数，用于获取软件包安装进度和中间状态
 * 
 * @return int 成功返回1，失败返回0
 * @since 3.0.1.0
 */
extern int kdk_package_install_tag_package_offline(const char *uri, const char *type, const char *compat, InstallCallBack call_back);

/**
 * @brief 获取软件包大小
 * @param pkg_name 软件包名
 * 
 * @return unsigned long 软件包大小，单位为kb
 * @since 3.0.1.0
 */
extern unsigned long kdk_package_get_package_size(const char *pkg_name);

/**
 * @brief 安装中中断状态恢复
 * @since 3.0.1.0
 */
extern void kdk_package_interrupt_signal_handler();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example kysdk-system/src/packages/test/kypackage-test.c
 * 
 */

/**
  * @}
  */
