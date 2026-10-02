/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Fnoily <liuyang@kylinos.cn>
 *         wangweiran <wangweiran@kylinos.cn>
 *         wumuti <wumuti@kylinos.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WX_WECHAT_TYPE_H
#define WX_WECHAT_TYPE_H

#include <stdio.h>
#include <libintl.h>
#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kywechat.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBBIOMETRIC_DOMAIN_NAME
#define LIBBIOMETRIC_DOMAIN_NAME "biometric-driver-wechat"
#endif

#ifndef LOCALEDIR
#define LOCALEDIR "/usr/local/share/locale/"
#endif

#define _(String) dgettext (LIBBIOMETRIC_DOMAIN_NAME, String)
#define N_(String) (String)

// 控制标志定义
#define CONTROL_FLAG_IDLE 0
#define CONTROL_FLAG_RUNNING 1
#define CONTROL_FLAG_STOPING 2
#define CONTROL_FLAG_STOPPED 3
#define CONTROL_FLAG_DONE 4


// 扩展提示信息的长度
#define EXTRA_INFO_LENGTH 1024


// 检测时间间隔
//#define EM1600DEV_FINGER_CHECK_INTERVAL_MS 100

// 定义操作提示
enum wechat_notify_mid {
    MID_EXTENDED_MESSAGE = NOTIFY_COMM_MAX + 1, // 定义扩展信息
    MID_NETWORK_ERROR,
    MID_GETTING,

};
enum wechat_ops_mid {
    MID_OPS_EXTENDED_MESSAGE = OPS_COMM_MAX + 1,
    MID_NETWORK_FAILED,
    MID_OPS_EXCEED_ENROOL_MAX,
    MID_TIME_ERROR,
};

// 驱动私有结构体
// 设备驱动私有结构体，指向设备驱动还需要的额外结构体
typedef struct wechat_driver_t wechat_driver;
struct wechat_driver_t {
    // 超时时间，单位毫秒
    int timeoutMS;

    // 控制标志
    int ctrlFlag;

    // 扩展提示信息
    char extra_info[EXTRA_INFO_LENGTH];

    /*
     * 厂商可根据需要在此处进行定义
    */
 
    int frameFD;

};

#ifdef __cplusplus
}
#endif



#endif 
