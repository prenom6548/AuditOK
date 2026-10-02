/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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
#include "input-wlcom-device.h"
#include "input-gsettings.h"
#include "input-process-settings.h"
#include "input-device-function.h"
#include "input-device-manager.h"
#include "input-wlcom-helper.h"
extern "C" {
#include "clib-syslog.h"
}

//设备禁用
#define LIBINPUT_CONFIG_SEND_EVENTS_ENABLED 0
#define LIBINPUT_CONFIG_SEND_EVENTS_DISABLED (1 << 0)
#define LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE (1 << 1)

//滚动方法设置
//禁用滚动
#define LIBINPUT_CONFIG_SCROLL_NO_SCROLL 0
//当两个手指在设备上按逻辑向下时发送滚动事件。
#define LIBINPUT_CONFIG_SCROLL_2FG (1 << 0)
//当手指沿设备底部或右边缘移动时，发送滚动事件。
#define LIBINPUT_CONFIG_SCROLL_EDGE (1 << 1)
//当按下按钮且设备沿可滚动轴移动时，发送滚动事件。
#define LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN (1 << 2)


//指针设备加速
//没有可配置指针的设备的占位符加速度剖面。
#define LIBINPUT_CONFIG_ACCEL_PROFILE_NONE  0
//一个平坦的加速剖面。指针运动加速度为a常数(特定于设备)因素，取决于电流速度。
// @see libinput_device_config_accel_set_speed
#define LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT (1 << 0)
//自适应加速配置文件。指针加速度取决于输入速度。这是大多数设备的默认配置文件。
#define LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE (1 << 1)


InputWlcomDevice::InputWlcomDevice(QVariant deviceId, DeviceType type, QVariant productId, QString deviceName, QObject *parent)
    :InputDevice(deviceId, type, deviceName, parent)
{
    m_productId = productId;
    initDeviceProperty();
}

void InputWlcomDevice::setEnable(QVariant value)
{
    uint32_t enable = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    if (isTouchpad()) {
        bool disableTouchpadOnMouse = getGsettingsValue(KEY_TOUCHPAD_DISBLE_O_E_MOUSE).toBool();
        //触摸板进行插入鼠标禁用触摸板的检测
        if (!value.toBool()) {
            enable |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
        }
        if (disableTouchpadOnMouse) {
            enable |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;
        }
    } else {
        if (!value.toBool()) {
            enable |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
        }
    }
    InputWlcomHelper::setSendEventsMode(m_deviceId.toString(), enable);
}

void InputWlcomDevice::setLeftMode(QVariant value)
{
    InputWlcomHelper::setLeftHand(m_deviceId.toString(), value.toBool());
}

void InputWlcomDevice::setNaturalScroll(QVariant value)
{
    InputWlcomHelper::setNatualScroll(m_deviceId.toString(), value.toBool());

}

void InputWlcomDevice::setTapclick(QVariant value)
{
    //目前轻触拖拽和轻触点击，一起设置。
    InputWlcomHelper::setTapToClick(m_deviceId.toString(), value.toBool());
    setTapDrag(value.toBool());
}

void InputWlcomDevice::setTapDrag(QVariant value)
{
    InputWlcomHelper::setTapAndDrag(m_deviceId.toString(), value.toBool());

}

void InputWlcomDevice::setDisableTyping(QVariant value)
{
    InputWlcomHelper::setDisableWhileTyping(m_deviceId.toString(), value.toBool());

}

void InputWlcomDevice::setAccelSpeed(QVariant value)
{
    uint32_t accleProfile = LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
    if (value.toBool()) {
        accleProfile |= LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
    } else {
        accleProfile |= LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT;
    }
    InputWlcomHelper::setAccekProfile(m_deviceId.toString(), accleProfile);

}

void InputWlcomDevice::setAcceleration(QVariant value)
{
    double acceleration = value.toDouble();
    if (acceleration <= 1.0) {
        acceleration = 1.0;
    } else if (acceleration >= 8.0) {
        acceleration = 8.0;
    }
    acceleration = (acceleration - 1.0) * 2.0 / 7.0 - 1;
    InputWlcomHelper::setPointerSpeed(m_deviceId.toString(), acceleration);
}

void InputWlcomDevice::setMiddleButtonEmulation(QVariant value)
{
    //控制面板暂未对外提供此功能，搁置
    Q_UNUSED(value)
}

void InputWlcomDevice::setScrolling(QVariant value)
{
    Q_UNUSED(value)
    bool edgeScroll = getGsettingsValue(KEY_VERT_EDGE_SCROLL).toBool();
    bool twoFingerScroll = getGsettingsValue(KEY_VERT_TWO_FINGER_SCROLL).toBool();
    uint32_t scrollMethod = LIBINPUT_CONFIG_SCROLL_NO_SCROLL;
    if (edgeScroll) {
        scrollMethod |= LIBINPUT_CONFIG_SCROLL_EDGE;
    }
    if (twoFingerScroll) {
        scrollMethod |= LIBINPUT_CONFIG_SCROLL_2FG;
    }
    InputWlcomHelper::setScrollMethod(m_deviceId.toString(), scrollMethod);
}

void InputWlcomDevice::setDisableTpMoPresent(QVariant value)
{
    //触摸板有此设置
    uint32_t enable = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    bool touchpadEnable = getGsettingsValue(KEY_TOUCHPAD_ENABLED).toBool();
    //检查当前触摸板状态，恢复
    if (!touchpadEnable) {
        enable |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
    }
    //插入鼠标禁用触摸板设置
    if (value.toBool()) {
        enable |= LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;
    }
    InputWlcomHelper::setSendEventsMode(m_deviceId.toString(), enable);
}

void InputWlcomDevice::setWheelSpeed(QVariant value)
{
    InputWlcomHelper::setScrollFactor(m_deviceId.toString(), value.toDouble());
}

void InputWlcomDevice::setDoubleClickTime(QVariant value)
{
    InputWlcomHelper::setDoubleClickTime(m_deviceId.toString(), value.toDouble());
}

void InputWlcomDevice::setRepeatInfo(QVariant value)
{
    Q_UNUSED(value)
    int rate = 0;
    bool repeat = getGsettingsValue(KEY_REPEAT).toBool();
    int delay = getGsettingsValue(KEY_DELAY).toUInt();
    if (repeat) {
        rate = getGsettingsValue(KEY_RATE).toUInt();
    }
    InputWlcomHelper::setRepeatInfo(m_deviceId.toString(), rate, delay);

}

void InputWlcomDevice::setTwoFingerClick(QVariant value)
{
    InputWlcomHelper::setTwoFingerClick(m_deviceId.toString(), value.toUInt());
}

void InputWlcomDevice::setClickMethodEnabled(QVariant value)
{
    InputWlcomHelper::setClickMethodEnabled(m_deviceId.toString(), value.toBool());
}

void InputWlcomDevice::setZoneGestureLeftEdgeEnabled(QVariant value)
{
    InputWlcomHelper::setZoneGestureLeftEdgeEnabled(m_deviceId.toString(), value.toBool());
}

void InputWlcomDevice::setZoneGestureRightEdgeEnabled(QVariant value)
{
    InputWlcomHelper::setZoneGestureRightEdgeEnabled(m_deviceId.toString(), value.toBool());
}

void InputWlcomDevice::setZoneGestureTopEdgeEnabled(QVariant value)
{
    InputWlcomHelper::setZoneGestureTopEdgeEnabled(m_deviceId.toString(), value.toBool());
}

void InputWlcomDevice::setTwoFingerPinch(QVariant value)
{
    InputWlcomHelper::setTwoFingerPinch(m_deviceId.toString(), value.toBool());
}

