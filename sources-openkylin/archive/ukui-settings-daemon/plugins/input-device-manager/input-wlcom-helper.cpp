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
#include "input-wlcom-helper.h"
#include <QDBusPendingCall>
QDBusInterface* InputWlcomHelper::s_wlcomInputInterface = nullptr;
QDBusInterface* InputWlcomHelper::s_wlcomSeatInterface = nullptr;

InputWlcomHelper::InputWlcomHelper(QObject *parent)
    : QObject{parent}
{

}

void InputWlcomHelper::setLeftHand(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableLeftHand"), device, enabled);
    }
}

void InputWlcomHelper::setNatualScroll(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableNaturalScroll"), device, enabled);

    }
}

void InputWlcomHelper::setTapAndDrag(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableTapAndDrag"), device, enabled);

    }
}

void InputWlcomHelper::setTapToClick(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableTapToClick"), device, enabled);

    }
}

void InputWlcomHelper::setAccekProfile(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetAccelProfile"), device, value);

    }
}

void InputWlcomHelper::setDisableWhileTyping(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetDisableWhileTyping"), device, enabled);

    }
}

void InputWlcomHelper::setPointerSpeed(const QString &device, double value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetPointerSpeed"), device, value);

    }
}

void InputWlcomHelper::setScrollMethod(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetScrollMethod"), device, value);

    }
}

void InputWlcomHelper::setSendEventsMode(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetSendEventsMode"), device, value);

    }
}

void InputWlcomHelper::setScrollFactor(const QString &device, double value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetScrollFactor"), device, value);

    }
}

void InputWlcomHelper::setDoubleClickTime(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetDoubleClickTime"), device, value);

    }
}

void InputWlcomHelper::setRepeatInfo(const QString &device, int rate, int delay)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetRepeatInfo"), device, rate, delay);
    }
}

void InputWlcomHelper::setCursor(const QString &seat, const QString &cursorTheme, uint32_t size)
{
    if (s_wlcomSeatInterface) {
        s_wlcomSeatInterface->asyncCall(QStringLiteral("SetCursor"), seat, cursorTheme, size);
    }
}

void InputWlcomHelper::setTwoFingerClick(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetTapButtonMap"), device, value);
    }
}

void InputWlcomHelper::setClickMethodEnabled(const QString &device, uint32_t value)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("SetClickMethod"), device, value);
    }
}

void InputWlcomHelper::setZoneGestureLeftEdgeEnabled(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableZoneGestureLeft"), device, enabled);
    }
}

void InputWlcomHelper::setZoneGestureRightEdgeEnabled(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableZoneGestureRight"), device, enabled);
    }
}

void InputWlcomHelper::setZoneGestureTopEdgeEnabled(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnableZoneGestureTop"), device, enabled);
    }
}

void InputWlcomHelper::setTwoFingerPinch(const QString &device, bool enabled)
{
    if (s_wlcomInputInterface->isValid()) {
        s_wlcomInputInterface->asyncCall(QStringLiteral("EnablePinchGesture"), device, enabled);
    }
}

