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

#ifndef INPUTXDEVICE_H
#define INPUTXDEVICE_H

#include <QVariantList>
#include "input-common.h"
#include "input-device.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
}
class InputXDevice;

class InputXDevice : public InputDevice
{
    Q_OBJECT
public:
    explicit InputXDevice(QVariant deviceId, DeviceType type, QString deviceName = "", QObject *parent = nullptr);
    void disable();
    void enable();
private:
    QVariant initProductId();

    void setProperty(const char *name, const QVariantList &value);
    void setProperty(Atom prop, const QVariantList &value);

    QVariantList getProperty(const char *name);
    QVariantList getProperty(Atom prop);
    Atom hasProperty(const char *name);

    void setLeftModeByButtonMap(bool leftMode);

    void setAccelByLibinput(Atom prop, double acceleration);
    void setAccelBySynaptic(Atom prop, double acceleration);
    void setAccelByFeedback(double acceleration);
    void setSynapticsTapAction(Atom prop);
    void setLibinputScrolling(Atom prop);
    void setSynapticsScrolling();
    void setSendEventsMode(Atom prop,int mode, int value);

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
    void setRepeatInfo(QVariant value) override;
};

#endif // INPUTXDEVICE_H
