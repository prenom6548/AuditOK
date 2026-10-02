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
#include <qmath.h>
#include "input-x-device.h"
#include "input-device-helper.h"
#include "input-gsettings.h"
#include "input-process-settings.h"
#include "input-device-manager.h"
extern "C" {
#include "clib-syslog.h"
}
//libinput event mode
#define EVENT_MODE_DISABLED 0
#define EVENT_MODE_DISABLED_ON_EXTERNAL_MOUSE 1

using namespace InputDeviceHelper;

QVariantList variantList(QVariant value)
{
    return QVariantList() << value;
}

InputXDevice::InputXDevice(QVariant deviceId, DeviceType type, QString deviceName, QObject* parent)
    : InputDevice(deviceId, type, deviceName, parent)
{
    m_productId = initProductId();
    initDeviceProperty();
}

void InputXDevice::disable()
{
    InputDeviceHelper::disable(m_deviceId.toInt());
}

void InputXDevice::enable()
{
    InputDeviceHelper::enabel(m_deviceId.toInt());
}

void InputXDevice::setProperty(const char *name, const QVariantList &value)
{
    InputDeviceHelper::setDeviceProp(m_deviceId.toInt(), name, value);
}

void InputXDevice::setProperty(Atom prop, const QVariantList &value)
{
    InputDeviceHelper::setDeviceProp(m_deviceId.toInt(), prop, value);
}

QVariantList InputXDevice::getProperty(const char *name)
{
    return InputDeviceHelper::getDeviceProp(m_deviceId.toInt(),name);
}

QVariantList InputXDevice::getProperty(Atom prop)
{
    return InputDeviceHelper::getDeviceProp(m_deviceId.toInt(),prop);
}

Atom InputXDevice::hasProperty(const char *name)
{
    return InputDeviceHelper::deviceHadProperty(m_deviceId.toInt(),name);
}

//左右键映射
void InputXDevice::setLeftModeByButtonMap(bool leftMode)
{
    unsigned char *map = nullptr;
    int buttons = InputDeviceHelper::getDeviceButtonMap(m_deviceId.toInt(),&map);

    if (!map || 0 == buttons) {
        USD_LOG(LOG_WARNING,"device : %d has no buttons.",m_deviceId.toInt());
        USD_LOG(LOG_WARNING,"buttons map is null . set button map faild .");
        return;
    }
    auto calculationMap = [] (int buttons , unsigned char* map, bool leftMode) -> void {
        const int left = 1;
        int right = qMin(buttons,3);
        if (map[left - 1] != left && map[left - 1] != right) {
            USD_LOG(LOG_WARNING,"The current mapping is weird. Swapping buttons is probably not a good idea.");
            return;
        }
        if (leftMode) {
            map[left - 1] = right;
            map[right - 1] = left;
        } else {
            map[left - 1] = left;
            map[right - 1] = right;
        }
    };
    calculationMap(buttons, map, leftMode);
    InputDeviceHelper::setDeviceButtonMap(m_deviceId.toInt(),buttons,map);
}

void InputXDevice::setAccelByLibinput(Atom prop, double acceleration)
{
    if (acceleration <= 1.0) {
        acceleration = 1.0;
    } else if (acceleration >= 8.0) {
        acceleration = 8.0;
    }
    acceleration = (acceleration - 1.0) * 2.0 / 7.0 - 1;
    InputDeviceHelper::setDeviceProp(m_deviceId.toInt(), prop, variantList(acceleration));
}

void InputXDevice::setAccelBySynaptic(Atom prop, double acceleration)
{
    if (acceleration <= 1.0) {
        acceleration = 1.0;
    } else if (acceleration >= 8.0) {
        acceleration = 8.0;
    }
    acceleration = 9.0 - acceleration;
    InputDeviceHelper::setDeviceProp(m_deviceId.toInt(), prop, variantList(acceleration));
}

void InputXDevice::setAccelByFeedback(double acceleration)
{
    //feefback光标速度设置
    int numerator = -1;
    int denominator = -1;
    int threshold = -1;

    threshold = getGsettingsValue(KEY_MOTION_THRESHOLD).toInt();
    /* Calculate acceleration */
    if (acceleration >= 1.0) {
        /* we want to get the acceleration, with a resolution of 0.5
             */
        if ((acceleration -  floor(acceleration)) < 0.25) {
            numerator = floor(acceleration);
            denominator = 1;
        } else if ((acceleration - floor(acceleration)) < 0.5) {
            numerator = ceil(2.0 * acceleration);
            denominator = 2;
        } else if ((acceleration - floor(acceleration)) < 0.75) {
            numerator = floor(2.0 *acceleration);
            denominator = 2;
        } else {
            numerator = ceil(acceleration);
            denominator = 1;
        }
    } else if (acceleration < 1.0 && acceleration > 0) {
        /* This we do to 1/10ths */
        numerator = floor (acceleration * 10) + 1;
        denominator= 10;
    }
    InputDeviceHelper::changePtrFeedbackControl(m_deviceId.toInt(), threshold, numerator, denominator);
}

void InputXDevice::setSynapticsTapAction(Atom prop)
{
    QVariantList list = getProperty(prop);
    if (list.isEmpty()) {
        USD_LOG(LOG_WARNING,"prop value is null .");
        return;
    }
    bool state = getGsettingsValue(KEY_TOUCHPAD_TAP_TO_CLICK).toBool();
    int oneTapAction = getGsettingsValue(KEY_TOUCHPAD_ONE_FINGER_TAP).toInt();
    int twoTapAction = getGsettingsValue(KEY_TOUCHPAD_TWO_FINGER_TAP).toInt();
    int threeTapActon = getGsettingsValue(KEY_TOUCHPAD_THREE_FINGER_TAP).toInt();
    USD_LOG(LOG_DEBUG,"finger action is one : %d  two : %d  three : %d",oneTapAction,twoTapAction,threeTapActon);

    if (oneTapAction > 3 || oneTapAction < 1) {
        oneTapAction = 1;
    }
    if (twoTapAction > 3 || twoTapAction < 1) {
        twoTapAction = 3;
    }
    if (threeTapActon > 3 || threeTapActon < 1) {
        threeTapActon = 0;
    }
    //属性列表4，5，6代表单指，双指，三指对应的动作，1：鼠标左键，2：鼠标中键，3：鼠标右键 0：禁用
    list[4] = state ? oneTapAction : 0;
    list[5] = state ? twoTapAction : 0;
    list[6] = state ? threeTapActon : 0;
    setProperty(prop,list);
}

void InputXDevice::setLibinputScrolling(Atom prop)
{
    QVariantList list = getProperty(prop);
    if (list.isEmpty()) {
        USD_LOG(LOG_WARNING,"prop list value is null .");
        return;
    }
    bool edgeScroll = getGsettingsValue(KEY_VERT_EDGE_SCROLL).toBool();
    bool twoFingerScroll = getGsettingsValue(KEY_VERT_TWO_FINGER_SCROLL).toBool();
    if (twoFingerScroll && !edgeScroll) {        //双指滚动
        list[0] = 1;
        list[1] = 0;
    } else if (edgeScroll && !twoFingerScroll) {            //边缘滚动
        list[0] = 0;
        list[1] = 1;
    } else if (!edgeScroll && !twoFingerScroll) {
        list[0] = 0;
        list[1] = 0;
    }
    setProperty(prop,list);
}

void InputXDevice::setSynapticsScrolling()
{
    //Synaptics Scrolling
    Atom edgeProp = hasProperty("Synaptics Edge Scrolling");
    Atom twoFingerProp = hasProperty("Synaptics Two-Finger Scrolling");
    if (edgeProp) {
        QVariantList list = getProperty(edgeProp);
        if (list.isEmpty()) {
            USD_LOG(LOG_WARNING,"prop list value is null .");
            return;
        }

        bool verScroll = getGsettingsValue(KEY_VERT_EDGE_SCROLL).toBool();
        bool horScroll = getGsettingsValue(KEY_HORIZ_EDGE_SCROLL).toBool();
        list[0] = verScroll ? 1 : 0;
        list[1] = horScroll ? 1 : 0;
        setProperty(edgeProp,list);
    }
    if (twoFingerProp) {
        QVariantList list = getProperty(twoFingerProp);
        if (list.isEmpty()) {
            USD_LOG(LOG_WARNING,"prop list value is null .");
            return;
        }

        bool verScroll = getGsettingsValue(KEY_VERT_TWO_FINGER_SCROLL).toBool();
        bool horScroll = getGsettingsValue(KEY_HORIZ_TWO_FINGER_SCROLL).toBool();
        list[0] = verScroll ? 1 : 0;
        list[1] = horScroll ? 1 : 0;
        setProperty(twoFingerProp,list);
    }
}

void InputXDevice::setSendEventsMode(Atom prop, int mode, int value)
{
    QVariantList list = getProperty(prop);
    if (list.isEmpty()) {
        USD_LOG(LOG_WARNING,"prop list value is null .");
        return;
    }
    if (value) {
        list[mode] = 1;
    } else {
        list[mode] = 0;
    }
    setProperty(prop,list);
}

/*get property*/

QVariant InputXDevice::initProductId()
{
    QVariantList list  = getProperty("Device Product ID");
    uint product = 0;
    for (QVariant var : list){
        product <<= 16;
        product += var.toInt();
    }
    return product;
}

/****************set function******************/

void InputXDevice::setEnable(QVariant value)
{
    if (isTouchpad()) {
        //触摸板进行插入鼠标禁用触摸板的检测
        if (value.toBool()) {
            setDisableTpMoPresent(getGsettingsValue(KEY_TOUCHPAD_DISBLE_O_E_MOUSE));
        } else {
            disable();
        }
    } else {
        if (value.toBool()) {
            enable();
        } else {
            disable();
        }
    }
}

void InputXDevice::setLeftMode(QVariant value)
{
    if (Atom prop = hasProperty("libinput Left Handed Enabled")) {
        setProperty(prop, variantList(value));
    } else {
        //无此属性，可通过按钮映射方式实现左手模式
        setLeftModeByButtonMap(value.toBool());
    }
}

//滚动方向
void InputXDevice::setNaturalScroll(QVariant value)
{
    if (Atom prop = hasProperty("libinput Natural Scrolling Enabled")) {
        setProperty(prop, variantList(value));
    } else if (Atom prop = hasProperty("Synaptics Scrolling Distance")) {
        //Synaptics
        QVariantList list = getProperty(prop);
        for (int i = 0; i < list.size(); ++i) {
            if (value.toBool()) {
                list[i] = -qAbs(list.at(i).toInt());
            } else {
                list[i] = qAbs(list.at(i).toInt());
            }
        }
        setProperty(prop, list);
    }
}

//触摸板轻触点击
void InputXDevice::setTapclick(QVariant value)
{
    if (isTouchpad()) {
        if (Atom prop = hasProperty("libinput Tapping Enabled")) {
            setProperty(prop, variantList(value));
        } else if (Atom prop = hasProperty("Synaptics Tap Action")) {
            setSynapticsTapAction(prop);
        }
    }
}

//触摸板轻触拖拽
void InputXDevice::setTapDrag(QVariant value)
{
    if (isTouchpad()) {
        if (Atom prop = hasProperty("libinput Tapping Drag Enabled")) {
            setProperty(prop, variantList(value));
        } else if (Atom prop = hasProperty("Synaptics Gestures")) {
            setProperty(prop, variantList(value));
        }
    }
}

//打字禁用触摸板
void InputXDevice::setDisableTyping(QVariant value)
{
    if (isTouchpad()) {
        if (Atom prop = hasProperty("libinput Disable While Typing Enabled")) {
            setProperty(prop, variantList(value));
        } else if (Atom prop = hasProperty("Synaptics Off")){
            Q_UNUSED(prop)
            //Syndaemon
            ProcessSettings::instance()->setDisableWTypingSynaptics(value.toBool());
        }
    }
}

//光标加速配置文件
void InputXDevice::setAccelSpeed(QVariant value)
{
    if (Atom prop = hasProperty("libinput Accel Profile Enabled")) {
        QVariantList list;
        if (value.toBool()) {
            list << 1 << 0; //0，1 平滑
        } else {
            list << 0 << 1; //1，0 自适应
        }
        setProperty(prop, list);
    } else if (Atom prop = hasProperty("Device Accel Profile")) {
        setProperty(prop, variantList(value));
    } else {
        USD_LOG(LOG_WARNING,"property is not libinput or Synaptics");
    }
}

//光标速度
void InputXDevice::setAcceleration(QVariant value)
{
    //待确定速度标准
    if (Atom prop = hasProperty("libinput Accel Speed")) {
        //光标速度值根据属性不同需要改动,慢-快[-1 ，1]
        setAccelByLibinput(prop, value.toDouble());
    } else if (Atom prop = hasProperty("Device Accel Constant Deceleration")) {
        //[1 - 8]
        setAccelBySynaptic(prop, value.toDouble());
    } else {
        //by feedback
        setAccelByFeedback(value.toDouble());
    }
}

//左右键同时按下模拟右键
void InputXDevice::setMiddleButtonEmulation(QVariant value)
{
     if (Atom prop = hasProperty("libinput Middle Emulation Enabled")) {
         setProperty(prop, variantList(value));
     } else if (Atom prop = hasProperty("Evdev Middle Button Emulation")) {
         setProperty(prop, variantList(value));
     } else {
         USD_LOG(LOG_WARNING,"property is not libinput or Synaptics");
     }
}

//滚动
void InputXDevice::setScrolling(QVariant value)
{
    Q_UNUSED(value)
    if (Atom prop = hasProperty("libinput Scroll Method Enabled")) {
        setLibinputScrolling(prop);
    } else {
        setSynapticsScrolling();
    }
}

//插入鼠标禁用触摸板
void InputXDevice::setDisableTpMoPresent(QVariant value)
{
//    if (Atom prop = hasProperty("libinput Send Events Mode Enabled")) {
//        setSendEventsMode(prop, EVENT_MODE_DISABLED_ON_EXTERNAL_MOUSE, value.toBool());
//    } else {
    if (value.toBool() && InputDeviceManager::mousePresent()) {
        disable();
    } else {
        enable();
    }
//    }
}


void InputXDevice::setWheelSpeed(QVariant value)
{
    if (isMouse()) {
        if (Atom prop = hasProperty("libinput Button Scrolling Speed")) {
            // 与支撑确认，此属性取值范围 0 < x <= 20.0 ,用户设置入口为1-10，进行x2
            setProperty(prop, variantList(2 * value.toFloat()));
        } else if (Atom prop = hasProperty("Synaptics Scrolling Distance")){
    //        Synaptics 滚动取值未定
    //        QVariantList list = getProperty(prop);
    //        for (int i = 0; i < list.size(); ++i) {
    //            int val = list[i].toInt();
    //            if (val >= 0) {
    //                list[i] = qAbs(value.toInt());
    //            } else {
    //                list[i] = -qAbs(value.toInt());
    //            }
    //        }
    //        setProperty(prop, list);
        }
    }
}

void InputXDevice::setRepeatInfo(QVariant value)
{
    Q_UNUSED(value)
}
