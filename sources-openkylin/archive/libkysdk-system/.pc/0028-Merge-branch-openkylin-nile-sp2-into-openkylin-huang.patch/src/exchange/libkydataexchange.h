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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LIBKYDATAEXCHANGE_H
#define LIBKYDATAEXCHANGE_H

/**
 * @file libkydataexchange.h
 * @author wangjingwen (wangjingwen@kylinos.cn)
 * @brief 剪贴板数据交换模块
 * @version 1.0
 * @date 2025-3-23
 * 
 * @copyright Copyright (c) 2025
 * @defgroup libkysdk-exchange
 * @{
 * @since 3.0.1.0
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 剪贴板数据类型枚举
 */
typedef enum {
    CLIPBOARD_TYPE_TEXT = 1,  /**< 文本类型数据 */
    CLIPBOARD_TYPE_IMAGE = 2  /**< 图像类型数据 */
} ClipboardDataType;

/**
 * @brief 剪贴板操作结果枚举
 */
typedef enum {
    CLIPBOARD_OPERATION_SUCCESS = 0,              /**< 操作成功 */
    CLIPBOARD_OPERATION_FAILURE = -1,             /**< 操作失败 */
    CLIPBOARD_OPERATION_UNSUPPORTED_TYPE = -2     /**< 不支持的数据类型 */
} ClipboardOperationResult;

/**
 * @brief 剪贴板图像数据结构
 */
typedef struct {
    unsigned char* data;          /**< 图像数据指针 */
    size_t width;                 /**< 图像宽度 */
    size_t height;                /**< 图像高度 */
    size_t bytes_per_pixel;       /**< 每个像素的字节数 */
} ClipboardImage;

/**
 * @brief 剪贴板变化回调函数类型
 * @param user_data 用户自定义数据指针
 */
typedef void (*ClipboardChangeCallback)(void* user_data);

/**
 * @brief 初始化剪贴板系统
 * 
 * @return bool 初始化成功返回true，失败返回false
 */
extern bool kdk_exchange_init(void);

/**
 * @brief 清理剪贴板系统资源
 */
extern void kdk_exchange_cleanup(void);

/**
 * @brief 设置剪贴板数据
 * 
 * @param type 数据类型
 * @param data 数据指针
 * @param size 数据大小
 * @return ClipboardOperationResult 操作结果
 */
extern ClipboardOperationResult kdk_exchange_set_data(ClipboardDataType type, const void* data, size_t size);

/**
 * @brief 获取剪贴板数据
 * 
 * @param type 返回的数据类型
 * @param data 返回的数据指针，需要调用者释放
 * @return ClipboardOperationResult 操作结果
 */
extern ClipboardOperationResult kdk_exchange_get_data(ClipboardDataType* type, void** data);

/**
 * @brief 清空剪贴板
 * 
 * @return ClipboardOperationResult 操作结果
 */
extern ClipboardOperationResult kdk_exchange_clear(void);

/**
 * @brief 检查剪贴板是否为空
 * 
 * @return bool 为空返回true，否则返回false
 */
extern bool kdk_exchange_is_empty(void);

/**
 * @brief 注册剪贴板变化回调函数
 * 
 * @param callback 回调函数指针
 * @param user_data 用户自定义数据指针
 * @return ClipboardOperationResult 操作结果
 */
extern ClipboardOperationResult kdk_exchange_register_callback(ClipboardChangeCallback callback, void* user_data);

/**
 * @brief 注销剪贴板变化回调函数
 * 
 * @param callback 要注销的回调函数指针
 * @return ClipboardOperationResult 操作结果
 */
extern ClipboardOperationResult kdk_exchange_unregister_callback(ClipboardChangeCallback callback);

#ifdef __cplusplus
}
#endif

#endif // LIBKYDATAEXCHANGE_H

/**
 * \example kysdk-system/src/exchange/test/kyclipboard-test.c
 * 
 */