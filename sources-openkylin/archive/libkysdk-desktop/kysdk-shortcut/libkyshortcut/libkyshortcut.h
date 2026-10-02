/*
 * libkysdk-shortcut's Library
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef LIBKYSHORTCUT_H
#define LIBKYSHORTCUT_H

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup ShortcutModule
  * @{
  */

/**
 * @file libkyshortcut.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 提供快捷键管理功能
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025
 * @since 3.0.1.0
 */

#include "libkykeymap.h"

#define KYSDK_SUCCESS                    0
#define KYSDK_SHORTCUT_NAME_ERROR       -1  // 快捷键名错误
#define KYSDK_SHORTCUT_KEY_ERROR        -2  // 快捷键错误
#define KYSDK_SHORTCUT_ACTION_ERROR     -3  // 快捷键命令错误
#define KYSDK_SHORTCUT_NOT_EXISTS       -4  // 快捷键不存在
#define KYSDK_SHORTCUT_EXISTED          -5  // 快捷键已存在

    typedef struct _KYSDKGlobalShortcutInfo
    {
        char *compenteName;           // 所属组件
        char *name;                   // 快捷键名称
        unsigned int keys[4];         // 快捷键字符串组合
        char *action;                 // 执行程序，仅组件kysdk_bindings时有值
    } KYSDKGlobalShortcutInfo;

    typedef struct _KYSDKGlobalShortcutInfoList
    {
        KYSDKGlobalShortcutInfo *data;
        struct _KYSDKGlobalShortcutInfoList *next;
    } KYSDKGlobalShortcutInfoList;

    /**
     * @brief 创建一个新的全局快捷键
     *
     * @param name 快捷键名称(唯一)
     * @param key_sequence 快捷键,例如Ctrl + P
     * @param action 应用的desktop文件,或应用的执行命令(可执行程序传绝对路径)
     * @return int 成功返回0，失败返回错误码
     * @since 3.0.1.0
     */
    int kdk_shortcut_create_global_shortcut(const char *name, const char *key, const char *action);

    /**
     * @brief 删除全局快捷键
     *
     * @param name 快捷键名称
     * @return int 成功返回0，失败返回错误码
     * @since 3.0.1.0
     */
    int kdk_shortcut_delete_global_shortcut(const char *name);

    /**
     * @brief 检测全局快捷键是否可用
     *
     * @param key_sequence 快捷键
     * @return int 成功返回0，失败返回错误码
     * @since 3.0.1.0
     */
    int kdk_shortcut_global_shortcut_is_available(const char *key);

    /**
     * @brief 修改全局快捷键
     *
     * @param name 快捷键名称，全局唯一的名称
     * @param key_sequence 快捷键，传空字符则不修改
     * @param action 执行命令，传空字符则不修改
     * @return int 成功返回0，失败返回错误码
     * @since 3.0.1.0
     */
    int kdk_shortcut_set_global_shortcut(const char *name, const char *key, const char *action);

    /**
     * @brief 获取全局快捷键信息
     *
     * @return GlobalKeyEntryList* 快捷键信息链表
     * @since 3.0.1.0
     */
    KYSDKGlobalShortcutInfoList *kdk_shortcut_get_global_shortcuts();

    /**
     * @brief 获取快捷键信息
     *
     * @param key_sequence 快捷键
     * @return GlobalKeyEntryList* 快捷键信息链表
     * @since 3.0.1.0
     */
    KYSDKGlobalShortcutInfoList *kdk_shortcut_get_global_shortcuts_by_key(const char *key);

    /**
     * @brief 销毁GlobalShortcutInfoList列表
     *
     * @since 3.0.1.0
     */
    void kdk_shortcut_destroy_info_list(KYSDKGlobalShortcutInfoList *list);

    /**
     * @brief 键值组合转换为字符串
     *
     * @param key 键值组合, 控制键 + 功能键
     * @return char* 组合字符串
     * @since 3.0.1.0
     */
    const char *kdk_shortcut_key_combination_to_string(unsigned int key);

    /**
     * @brief 字符串转换为键值组合
     *
     * @param key 组合字符串
     * @return unsigned int 键值组合
     * @since 3.0.1.0
     */
    unsigned int kdk_shortcut_string_to_key_combination(const char *key);

#ifdef __cplusplus
}
#endif

#endif // LIBKYSHORTCUT_H
