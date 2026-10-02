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
#ifndef INPUTWLCOMDEVICE_H
#define INPUTWLCOMDEVICE_H

#include <QObject>
#include "input-common.h"
#include "input-device.h"

class InputWlcomDevice : public InputDevice
{
    Q_OBJECT
public:
    explicit InputWlcomDevice(QVariant deviceId, DeviceType type, QVariant productId, QString deviceName = "", QObject *parent = nullptr);

    /*set function*/
    void setEnable(QVariant value) override;
    void setLeftMode(QVariant value) override;
    void setNaturalScroll(QVariant value) override;
    void setTapclick(QVariant value) override;
    void setTapDrag(QVariant value) override;
    void setDisableTyping(QVariant value) override;
    void setAccelSpeed(QVariant value) override;
    void setAcceleration(QVariant value) override;
    void setMiddleButtonEmulation(QVariant value) override;
    void setScrolling(QVariant value) override;
    void setDisableTpMoPresent(QVariant value) override;
    void setWheelSpeed(QVariant value) override;
    void setDoubleClickTime(QVariant value) override;
    void setRepeatInfo(QVariant value) override;

    void setTwoFingerClick(QVariant value) override;
    void setClickMethodEnabled(QVariant value) override;
    void setZoneGestureLeftEdgeEnabled(QVariant value) override;
    void setZoneGestureRightEdgeEnabled(QVariant value) override;
    void setZoneGestureTopEdgeEnabled(QVariant value) override;

    void setTwoFingerPinch(QVariant value) override;

Q_SIGNALS:

};

#endif // INPUTWLCOMDEVICE_H
