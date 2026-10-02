/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#ifndef INPUTDEVICEHELPER_H
#define INPUTDEVICEHELPER_H


#include <QVariantList>
#include <QX11Info>

extern "C" {
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XInput.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "clib-syslog.h"
}


namespace InputDeviceHelper {

    /**
     * @brief supportXinputExtension 支持Xinput扩展
     * @return bool
     */
    bool supportXinputExtension();
    /**
     * @brief propertyToAtom 属性名称转换Atom
     * @param property 属性名称
     * @return Atom
     */
    Atom propertyToAtom(const char* property);

    /**
     * @brief 检查属性是否存在
     * @param device 设备ID
     * @param propName 属性名称
     * @param prop Atom
     * @return 返回属性Atom,未找到返回None
     */
    Atom deviceHadProperty(int device, const char* propName);
    Atom deviceHadProperty(int device, Atom prop);

    /**
     * @brief getDeviceProp 获取某一属性的值
     * @param device 设备ID
     * @param prop Atom
     * @return 返回QvariantList
     */
    QVariantList getDeviceProp(int device, const char *property);
    QVariantList getDeviceProp(int device, Atom prop);

    /**
     * @brief setDeviceProp 修改属性的值
     * @param device
     * @param prop
     */
    void setDeviceProp(int device, const char *property ,QVariantList value);
    void setDeviceProp(int device, Atom prop, QVariantList value);

    /**
     * @brief 获取按钮映射
     * @param device
     * @return
     */
    int getDeviceButtonMap(int device, unsigned char** map);
    /**
     * @brief 设置按钮映射
     * @param device 设备id
     * @param buttons 映射按钮的数量
     * @param map
     */
    void setDeviceButtonMap(int device, int buttons, unsigned char* map);

    /**
     * @brief changePtrFeedbackControl
     * @param device
     * @param threshold
     * @param numerator
     * @param denominator
     */
    void changePtrFeedbackControl(int device, int threshold, int numerator, int denominator);

    /**
     * @brief disable 禁用设备
     * @param device 设备ID
     */
    void disable(int device);

    /**
     * @brief enabel 启用设备
     * @param device 设备ID
     */
    void enabel(int device);
}

#endif // INPUTDEVICEHELPER_H
