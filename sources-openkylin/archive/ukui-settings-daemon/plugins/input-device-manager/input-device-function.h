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
#ifndef INPUTDEVICEFUNCTION_H
#define INPUTDEVICEFUNCTION_H
#include "input-device.h"
#include <functional>

typedef QMap<QString, std::function<void(QVariant, InputDevice*)>> DeviceFunctionMap;

class InputDeviceFunction
{
public:
    /*static set fuction*/

#define STATIC_FUNCTION_DECLARATION(name) \
    static void name(QVariant value, InputDevice* device = nullptr); \

    STATIC_FUNCTION_DECLARATION(setLeftMode)
    STATIC_FUNCTION_DECLARATION(setAcceleration)
    STATIC_FUNCTION_DECLARATION(setAccelSpeed)
    STATIC_FUNCTION_DECLARATION(setMiddleButtonEmulation)
    STATIC_FUNCTION_DECLARATION(setWheelSpeed)
    STATIC_FUNCTION_DECLARATION(setLocatePointer)

    STATIC_FUNCTION_DECLARATION(setNaturalScroll)
    STATIC_FUNCTION_DECLARATION(setDisableTyping)
    STATIC_FUNCTION_DECLARATION(setTapclick)
    STATIC_FUNCTION_DECLARATION(setScrolling)
    STATIC_FUNCTION_DECLARATION(setEnable)
    STATIC_FUNCTION_DECLARATION(setTapDrag)
    STATIC_FUNCTION_DECLARATION(setDisableTpMoPresent)
    STATIC_FUNCTION_DECLARATION(setDoubleClickTime)
    STATIC_FUNCTION_DECLARATION(setRepeatInfo)

    STATIC_FUNCTION_DECLARATION(setTwoFingerClick)
    STATIC_FUNCTION_DECLARATION(setClickMethodEnabled)
    STATIC_FUNCTION_DECLARATION(setZoneGestureLeftEdgeEnabled)
    STATIC_FUNCTION_DECLARATION(setZoneGestureRightEdgeEnabled)
    STATIC_FUNCTION_DECLARATION(setZoneGestureTopEdgeEnabled)
    STATIC_FUNCTION_DECLARATION(setTwoFingerPinch)
};

#endif // INPUTDEVICEFUNCTION_H
