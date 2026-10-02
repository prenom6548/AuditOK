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
#ifndef INPUTWAYLANDDEVICE_H
#define INPUTWAYLANDDEVICE_H

#include <QObject>
#include <QDBusInterface>
#include "input-common.h"
#include "input-device.h"

#define UKUI_KWIN_SERVICE "org.ukui.KWin"
#define UKUI_KWIN_OBJECT_PATH  "/org/ukui/KWin/InputDevice/"
#define UKUI_KWIN_INTERFACE "org.ukui.KWin.InputDevice"

#define KDE_KWIN_SERVICE "org.kde.KWin"
#define KDE_KWIN_OBJECT_PATH  "/org/kde/KWin/InputDevice/"
#define KDE_KWIN_INTERFACE "org.kde.KWin.InputDevice"

class InputWaylandDevice : public InputDevice
{
    Q_OBJECT
public:
    explicit InputWaylandDevice(QVariant deviceId, DeviceType type, QString deviceName = "", QObject *parent = nullptr);

public:
    void disable();
    void enable();
public:
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
private:
    QVariant initProductId();
    void setProperty(const char* prop, QVariant);
    QVariant getProperty(const char* prop);

Q_SIGNALS:

private:
    QDBusInterface *m_interface;
};

#endif // INPUTWAYLANDDEVICE_H
