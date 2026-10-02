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

#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QObject>
#include <QVariant>
#include "input-common.h"

class InputDevice : public QObject
{
    Q_OBJECT
public:
    explicit InputDevice(QVariant id, DeviceType type, QString name, QObject *parent);
    virtual ~InputDevice(){}
    bool isMouse();
    bool isTouchpad();
    QVariant getDeviceId(){return m_deviceId;}
    DeviceType getDeviceType(){return m_type;}
    QString getDeviceName(){return m_deviceName;}
    QVariant getProductId(){return m_productId;}
public:
    virtual void setEnable(QVariant value) = 0;
    virtual void setLeftMode(QVariant value) = 0;
    virtual void setNaturalScroll(QVariant value) = 0;
    virtual void setTapclick(QVariant value) = 0;
    virtual void setTapDrag(QVariant value) = 0;
    virtual void setDisableTyping(QVariant value) = 0;
    virtual void setAccelSpeed(QVariant value) = 0;
    virtual void setAcceleration(QVariant value) = 0;
    virtual void setMiddleButtonEmulation(QVariant value) = 0;
    virtual void setScrolling(QVariant value) = 0;
    virtual void setDisableTpMoPresent(QVariant value) = 0;
    virtual void setWheelSpeed(QVariant value) = 0;
    virtual void setDoubleClickTime(QVariant value){ Q_UNUSED(value)}
    virtual void setRepeatInfo(QVariant value) = 0;

    virtual void initDeviceProperty();

    virtual void setTwoFingerClick(QVariant value) { Q_UNUSED(value); }
    virtual void setClickMethodEnabled(QVariant value) { Q_UNUSED(value); }
    virtual void setZoneGestureLeftEdgeEnabled(QVariant value) { Q_UNUSED(value); }
    virtual void setZoneGestureRightEdgeEnabled(QVariant value)  { Q_UNUSED(value); }
    virtual void setZoneGestureTopEdgeEnabled(QVariant value)  { Q_UNUSED(value); }

    virtual void setTwoFingerPinch(QVariant value) { Q_UNUSED(value); }

protected:
    QVariant getGsettingsValue(const QString& key);
    void setGsettingsValue(const QString& key, const QVariant& value);

protected:
    QVariant m_deviceId;
    DeviceType m_type;
    QString m_deviceName;
    QVariant m_productId;
};

#endif // INPUTDEVICE_H
