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

//#include "libwechat/wechat_internal.h"
#include "libwechat/wechat_realization.h"
#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>
#include "id_tables.h"

#define WECHAT_NAME			"wechat_driver"
#define WECHAT_FULL_NAME	    "_wechat_certification_driver"


int ops_configure(bio_dev * dev, GKeyFile * conf)
{
    bio_print_debug("start configure");
    int ret = 0;

    // 设置驱动简称和全称
    dev->device_name    = (char*)WECHAT_NAME;
    dev->full_name      = (char*)WECHAT_FULL_NAME;

    // 设置驱动所支持的USB设备的ID表
    // 支持的ID表在id_tables.h中
    dev->usb_info.id_table = wechat_driver_id_table;
    // 保留字
    dev->usb_info.driver_data = 0;

    dev->ops_configure = ops_configure;

    ret = wechat_ops_configure(dev, conf);

    return ret;
}
