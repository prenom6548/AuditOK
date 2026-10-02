/*
 * libkysdk-system's Library
 *
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

#ifndef LIBKYGLOBAL_H
#define LIBKYGLOBAL_H

/**
 * @file libkyglobal.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 国际化模块
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * @defgroup libkysdk-global
 * @{
 * @since 2.5.1.2
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 获取系统支持的语言列表
 * 
 * @return char** 系统支持的语言列表，失败返回NULL
 */
extern char** kdk_global_get_system_support_language();

/**
 * @brief 判断当前语言和地区是否匹配
 * 
 * @return bool 匹配-true，不匹配-false
 */
extern bool kdk_global_get_region_match_language();

/**
 * @brief 获取系统是否为 从右至左显示语言
 * 
 * @return bool 是-true，否-false
 */
extern bool kdk_global_get_rtl();

/**
 * @brief 获取系统默认时区与UTC时区的偏差
 * 
 * @return int 偏差的小时数；例如：东八区 = 8；西二区 = -2
 */
extern int kdk_global_get_raw_offset();

/**
 * @brief 获取系统语言
 * 
 * @param username 用户名
 * @return char* 返回系统语言：中文:"zh_CN"；英文:"en_CN"；藏文:"bo_CN"；返回的字符串需要被 free 释放。失败返回NULL。
 */
extern char* kdk_global_get_system_language(const char *username);

/**
 * @brief 释放由kdk_global_get_system_support_language返回的系统支持的语言列表
 * 
 * @param langlist 由kdk_global_get_system_support_language返回的字符串指针
 */
extern void kdk_free_langlist(char** langlist);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example kysdk-system/src/battery/test/kybattery-test.c
 * 
 */
