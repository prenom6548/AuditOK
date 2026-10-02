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

#ifndef INPUTWLCOMHELPER_H
#define INPUTWLCOMHELPER_H

#include <QObject>
#include <QDBusInterface>


/***wlcom service***/
#define WLCOM_SERVICE "com.kylin.Wlcom"
#define WLCOM_INPUT_PATH "/com/kylin/Wlcom/Input"
#define WLCOM_INPUT_INTERFACE "com.kylin.Wlcom.Input"

#define WLCOM_SEAT_PATH "/com/kylin/Wlcom/Seat"
#define WLCOM_SEAT_INTERFACE "com.kylin.Wlcom.Seat"
/*************/

class InputWlcomHelper : public QObject
{
    Q_OBJECT
public:
    static inline QDBusInterface* wlcomInputInterface () {
        if (!s_wlcomInputInterface) {
            s_wlcomInputInterface = new QDBusInterface(QStringLiteral(WLCOM_SERVICE),
                                                       QStringLiteral(WLCOM_INPUT_PATH),
                                                       QStringLiteral(WLCOM_INPUT_INTERFACE),
                                                       QDBusConnection::sessionBus());
        }

        return s_wlcomInputInterface;
    }

    static inline QDBusInterface* wlcomSeatInterface () {
        if (!s_wlcomSeatInterface) {
            s_wlcomSeatInterface = new QDBusInterface(QStringLiteral(WLCOM_SERVICE),
                                                       QStringLiteral(WLCOM_SEAT_PATH),
                                                       QStringLiteral(WLCOM_SEAT_INTERFACE),
                                                       QDBusConnection::sessionBus());
        }

        return s_wlcomSeatInterface;
    }
public:
    explicit InputWlcomHelper(QObject *parent = nullptr);

    static void setLeftHand(const QString& device, bool enabled);
    static void setNatualScroll(const QString& device, bool enabled);
    static void setTapAndDrag(const QString& device, bool enabled);
    static void setTapToClick(const QString& device, bool enabled);
    static void setAccekProfile(const QString& device, uint32_t value);
    static void setDisableWhileTyping(const QString& device, bool enabled);
    static void setPointerSpeed(const QString& device, double value);
    static void setScrollMethod(const QString& device, uint32_t value);
    static void setSendEventsMode(const QString& device, uint32_t value);
    static void setScrollFactor(const QString& device, double value);
    static void setDoubleClickTime(const QString& device, uint32_t value);
    static void setRepeatInfo(const QString& device, int rate, int dealy);
    static void setCursor(const QString& device, const QString& cursorTheme, uint32_t size);

    static void setTwoFingerClick(const QString& device, uint32_t value);
    static void setClickMethodEnabled(const QString& device, uint32_t value);
    static void setZoneGestureLeftEdgeEnabled(const QString& device, bool enabled);
    static void setZoneGestureRightEdgeEnabled(const QString& device, bool enabled);
    static void setZoneGestureTopEdgeEnabled(const QString& device, bool enabled);
    static void setTwoFingerPinch(const QString& device, bool enabled);

private:
    static QDBusInterface* s_wlcomInputInterface;
    static QDBusInterface* s_wlcomSeatInterface;

};

#endif // INPUTWLCOMHELPER_H
