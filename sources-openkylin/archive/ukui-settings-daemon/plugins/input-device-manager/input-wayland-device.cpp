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
#include "input-wayland-device.h"
#include "input-gsettings.h"
#include <QDBusConnection>
#include "input-device-manager.h"

extern "C" {
#include "clib-syslog.h"
}

InputWaylandDevice::InputWaylandDevice(QVariant deviceId, DeviceType type, QString deviceName, QObject *parent)
    : InputDevice(deviceId, type, deviceName, parent)
{
    m_interface = new QDBusInterface(QStringLiteral(KDE_KWIN_SERVICE),
                                     QStringLiteral(KDE_KWIN_OBJECT_PATH) + m_deviceId.toString(),
                                     QStringLiteral(KDE_KWIN_INTERFACE),
                                     QDBusConnection::sessionBus(), this);
    if (!m_interface->isValid()) {
        m_interface = new QDBusInterface(QStringLiteral(UKUI_KWIN_SERVICE),
                                         QStringLiteral(UKUI_KWIN_OBJECT_PATH) + m_deviceId.toString(),
                                         QStringLiteral(UKUI_KWIN_INTERFACE),
                                         QDBusConnection::sessionBus(), this);
    }

    m_productId = initProductId();
    initDeviceProperty();
}

void InputWaylandDevice::disable()
{
    setProperty("enabled", false);
}

void InputWaylandDevice::enable()
{
    setProperty("enabled", true);
}

void InputWaylandDevice::setEnable(QVariant value)
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

void InputWaylandDevice::setLeftMode(QVariant value)
{
    setProperty("leftHanded", value);
}

void InputWaylandDevice::setNaturalScroll(QVariant value)
{
    setProperty("naturalScroll", value);
}

void InputWaylandDevice::setTapclick(QVariant value)
{
    setProperty("tapToClick", value);
}

void InputWaylandDevice::setTapDrag(QVariant value)
{
    setProperty("tapAndDrag", value);
}

void InputWaylandDevice::setDisableTyping(QVariant value)
{
    setProperty("disableWhileTyping", value);
}

void InputWaylandDevice::setAccelSpeed(QVariant value)
{
    setProperty("pointerAccelerationProfileAdaptive", value);
    setProperty("pointerAccelerationProfileFlat", !value.toBool());
}

void InputWaylandDevice::setAcceleration(QVariant value)
{
    double acceleration = value.toDouble();
    if (acceleration <= 1.0) {
        acceleration = 1.0;
    } else if (acceleration >= 8.0) {
        acceleration = 8.0;
    }
    acceleration = (acceleration - 1.0) * 2.0 / 7.0 - 1;
    setProperty("pointerAcceleration", acceleration);
}

void InputWaylandDevice::setMiddleButtonEmulation(QVariant value)
{
    setProperty("middleButtonEmulation", value);

}

void InputWaylandDevice::setScrolling(QVariant value)
{
    Q_UNUSED(value)
    bool edgeScroll = getGsettingsValue(KEY_VERT_EDGE_SCROLL).toBool();
    bool twoFingerScroll = getGsettingsValue(KEY_VERT_TWO_FINGER_SCROLL).toBool();
    USD_LOG(LOG_DEBUG,"setScrolling edgeScroll: %d twoFingerScroll: %d",edgeScroll, twoFingerScroll );
    setProperty("scrollEdge", edgeScroll);
    setProperty("scrollTwoFinger", twoFingerScroll);
}

void InputWaylandDevice::setDisableTpMoPresent(QVariant value)
{
    if (value.toBool() && InputDeviceManager::mousePresent()) {
        disable();
    } else {
        enable();
    }
}

void InputWaylandDevice::setWheelSpeed(QVariant value)
{
    if (isMouse()) {
        setProperty("scrollFactor", value);
    }
}

void InputWaylandDevice::setRepeatInfo(QVariant value)
{
    Q_UNUSED(value)
}

QVariant InputWaylandDevice::initProductId()
{
    return getProperty("product").toString() + getProperty("vendor").toString();
}

void InputWaylandDevice::setProperty(const char *prop, QVariant value)
{
    if (m_interface->isValid()) {
        USD_LOG(LOG_DEBUG,"set prop %s",prop);
        m_interface->setProperty(prop, value);
    } else {
        USD_LOG(LOG_WARNING,"wayland device interface is not valid .");
    }
}

QVariant InputWaylandDevice::getProperty(const char *prop)
{
    return m_interface->property(prop);
}
