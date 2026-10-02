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

#ifndef KYSDK_SYSTEM_PNP_H_
#define KYSDK_SYSTEM_PNP_H_

/**
 * @file libkypnp.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief pnp设备模块
 * @version 1.0
 * @date 2025-4-2
 * 
 * @copyright Copyright (c) 2025
 * @defgroup libkysdk-pnp
 * @{
 * @since 3.0.1.0
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*KdkPnpCallback)(char *action, char *sys_path);

typedef struct _kdk_pnp_info
{
    char *sysname;              // 设备名称
    char *sysnum;               // 设备数字标识符
    char *id;                   // 设备的ID信息。
    char *status;               // 设备的当前状态。
    char *mem;                  // 设备内存资源（起始地址-结束地址）
    char *hid;                  // 设备的HID。
    char *control;              // 设备的电源状态
    struct _kdk_pnp_info *next; // 指针
} kdk_pnp_info;

/**
 * @brief 获取系统中pnp设备信息
 * 
 * @return kdk_pnp_info* 设备信息结构体列表，失败返回NULL
 * @since 3.0.1.0
 */
extern kdk_pnp_info *kdk_pnp_get_info();

/**
 * @brief 释放设备信息结构体列表
 * 
 * @param info kdk_pnp_get_info的返回
 * @since 3.0.1.0
 */
extern void kdk_pnp_free_info(kdk_pnp_info* info);

/**
 * @brief 注册监听pnp设备连接事件的回调函数
 * 
 * @param call_back 回调函数指针
 * @param user_data 用户数据，
 * @return int 成功返回id，失败返回0
 * @since 3.0.1.0
 */
extern int kdk_pnp_register_switch_connect_handle(KdkPnpCallback call_back, void *user_data);

/**
 * @brief 注销pnp设备连接事件回调函数
 * 
 * @return int 成功返回0，失败返回-1
 * @since 3.0.1.0
 */
extern int kdk_pnp_unregister_switch_connect_handle();

/**
 * @brief 注册监听pnp设备断开连接事件的回调函数
 * 
 * @param call_back 回调函数指针
 * @param user_data 用户数据
 * @return int 成功返回id，失败返回0
 * @since 3.0.1.0
 */
extern int kdk_pnp_register_switch_disconnect_handle(KdkPnpCallback call_back, void *user_data);

/**
 * @brief 注销pnp设备断开连接事件回调函数
 * 
 * @return int 成功返回0，失败返回-1
 * @since 3.0.1.0
 */
extern int kdk_pnp_unregister_switch_disconnect_handle();


#ifdef __cplusplus
}
#endif

#endif // KYSDK_SYSTEM_PNP_H_