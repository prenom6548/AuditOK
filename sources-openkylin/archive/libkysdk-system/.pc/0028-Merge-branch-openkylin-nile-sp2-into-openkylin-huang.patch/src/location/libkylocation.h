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

#ifndef LIBKYLOCATION_H
#define LIBKYLOCATION_H

/**
 * @file libkylocation.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 地址信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2023
 * @defgroup libkysdk-location
 * @{
 * @since 2.0.0.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 获取本机地理位置(自3.0.1.0起不在使用)
 * 
 * @return char* 成功返回描述地理位置的json字符串
 */
extern char *kdk_loaction_get();

/**
 * @brief 获取本机地理位置
 * 
 * @return char* 成功返回描述地理位置的json字符串
 * @since 3.0.1.0
 */
extern char *kdk_location_get();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example kysdk-system/src/location/test/kylocation-test.c
 * 
 */
