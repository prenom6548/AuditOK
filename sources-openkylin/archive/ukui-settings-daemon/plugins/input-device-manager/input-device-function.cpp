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
#include "input-device-function.h"
#include "input-process-settings.h"


extern const DeviceFunctionMap deviceFuncMap = {
    {KEY_LEFT_HANDED,                   InputDeviceFunction::setLeftMode},
    {KEY_MOTION_ACCELERATION,           InputDeviceFunction::setAcceleration},

    {KEY_MOUSE_ACCEL,                   InputDeviceFunction::setAccelSpeed},
    {KEY_MIDDLE_BUTTON_EMULATION,       InputDeviceFunction::setMiddleButtonEmulation},
    {KEY_MOUSE_WHEEL_SPEED,             InputDeviceFunction::setWheelSpeed}, //imwheel
    {KEY_MOUSE_LOCATE_POINTER,          InputDeviceFunction::setLocatePointer},//locatepointer
    {KEY_MOUSE_NATRUAL_SCROLLING,       InputDeviceFunction::setNaturalScroll},

    {KEY_TOUCHPAD_DISABLE_W_TYPING,     InputDeviceFunction::setDisableTyping},
    {KEY_TOUCHPAD_TAP_TO_CLICK,         InputDeviceFunction::setTapclick},
    {KEY_VERT_EDGE_SCROLL,              InputDeviceFunction::setScrolling},
    {KEY_VERT_TWO_FINGER_SCROLL,        InputDeviceFunction::setScrolling},
    {KEY_TOUCHPAD_ENABLED,              InputDeviceFunction::setEnable},
    {KEY_TOUCHPAD_DOUBLE_CLICK_DRAG,    InputDeviceFunction::setTapDrag},
    {KEY_TOUCHPAD_DISBLE_O_E_MOUSE,     InputDeviceFunction::setDisableTpMoPresent},
    {KEY_MOUSE_DOUBLE_CLICK,            InputDeviceFunction::setDoubleClickTime},
    {KEY_REPEAT,                        InputDeviceFunction::setRepeatInfo},
    {KEY_RATE,                          InputDeviceFunction::setRepeatInfo},
    {KEY_DELAY,                         InputDeviceFunction::setRepeatInfo},
    {KEY_TRACKPOINT_ENABLED,            InputDeviceFunction::setEnable},

    {KEY_TOUCHPAD_TWO_FINGER_CLICK,         InputDeviceFunction::setTwoFingerClick},
    {KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M,       InputDeviceFunction::setClickMethodEnabled},
    {KEY_ZONE_GESTURE_LEFT_EDGE_ENABLED,    InputDeviceFunction::setZoneGestureLeftEdgeEnabled},
    {KEY_ZONE_GESTURE_RIGHT_EDGE_ENABLED,   InputDeviceFunction::setZoneGestureRightEdgeEnabled},
    {KEY_ZONE_GESTURE_TOP_EDGE_ENABLED,     InputDeviceFunction::setZoneGestureTopEdgeEnabled},
    {KEY_TOUCHPAD_TWO_FINGER_PINCH,         InputDeviceFunction::setTwoFingerPinch},
};

#define STATIC_FUNCTION_DEFINITION(name) \
    void InputDeviceFunction::name(QVariant value, InputDevice* device) \
    { \
        if (device) { \
            device->name(value); \
        } \
    }

STATIC_FUNCTION_DEFINITION(setLeftMode)
STATIC_FUNCTION_DEFINITION(setAcceleration)
STATIC_FUNCTION_DEFINITION(setAccelSpeed)
STATIC_FUNCTION_DEFINITION(setMiddleButtonEmulation)
STATIC_FUNCTION_DEFINITION(setWheelSpeed)

STATIC_FUNCTION_DEFINITION(setNaturalScroll)
STATIC_FUNCTION_DEFINITION(setDisableTyping)
STATIC_FUNCTION_DEFINITION(setTapclick)
STATIC_FUNCTION_DEFINITION(setScrolling)
STATIC_FUNCTION_DEFINITION(setEnable)
STATIC_FUNCTION_DEFINITION(setTapDrag)
STATIC_FUNCTION_DEFINITION(setDisableTpMoPresent)
STATIC_FUNCTION_DEFINITION(setDoubleClickTime)
STATIC_FUNCTION_DEFINITION(setRepeatInfo)

STATIC_FUNCTION_DEFINITION(setTwoFingerClick)
STATIC_FUNCTION_DEFINITION(setClickMethodEnabled)
STATIC_FUNCTION_DEFINITION(setZoneGestureLeftEdgeEnabled)
STATIC_FUNCTION_DEFINITION(setZoneGestureRightEdgeEnabled)
STATIC_FUNCTION_DEFINITION(setZoneGestureTopEdgeEnabled)
STATIC_FUNCTION_DEFINITION(setTwoFingerPinch)


void InputDeviceFunction::setLocatePointer(QVariant value, InputDevice*)
{
    ProcessSettings::instance()->setLocatePointer(value.toBool());
}
