/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TOUCHCALIBRATEX_H
#define TOUCHCALIBRATEX_H

#include <QDBusInterface>
#include <QDBusPendingCall>

#include "abstract_touch_calibrate.h"

extern "C"{
#include <X11/Xlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <xorg/xserver-properties.h>
#include <gudev/gudev.h>
#include "clib-syslog.h"
#ifdef None
#undef None
#endif

#ifdef Always
#undef Always
#endif
}

class TouchCalibrateX:
        public AbstractTouchCalibrate
{
    Q_OBJECT
public:
    TouchCalibrateX();

    /**
     * @brief 获取已连接屏幕信息
     */
    void getScreenList() override;

    /**
     * @brief 获取触摸设备信息
     */
    void getTouchDeviceList() override;

    /**
     * @brief touchscreen 映射
     */
    void calibrateTouchScreen() override;

    /**
     * @brief calibrateDevice
     * @param id 触摸设备ID
     * @param output 屏幕名称
     */
    void calibrateDevice(QVariant id, const QString &output);
     /**
     * @brief 获取设备节点
     * @param id
     * @return
     */
    QString getDeviceNode(int id);

    /**
     * @brief 获取设备productId ，分为vid，pid
     * @param id
     * @return
     */
    QVariantList getDeviceProductId(int id);

    /**
     * @brief createTouchDevice
     * @param device
     * @return
     */
    void addTouchDevice(const XDeviceInfo& device, TouchDeviceList& touchList);


public:
    void calibrate();
};

#endif // TOUCHCALIBRATEX_H
