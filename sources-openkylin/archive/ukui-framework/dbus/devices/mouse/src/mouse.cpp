/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
 */

#include "mouse.h"


MouseManager::MouseManager(QObject *parent):
    QObject(parent)
{
    m_pGsettingsManager = new GsettingsManager(parent, C_MOUSE_SCHEMA, setChangedHandler());
    initSignals();
    setChangedHandler();
}

MouseManager::~MouseManager()
{
    m_settingsChangedHandlers.clear();
    if (m_pGsettingsManager != nullptr) {
        delete m_pGsettingsManager;
        m_pGsettingsManager = nullptr;
    }
}

int MouseManager::GetMouseCount()
{
    QDBusInterface deviceManagerInterface(USD_SESSION_SERVER,
                                USD_SESSION_PATH,
                                USD_SESSION_INTERFACE,
                                QDBusConnection::sessionBus());
    QDBusReply<int> reply = deviceManagerInterface.call(USD_METHOD_GETDEVCOUNTBYTYPE, "mouse");
    return reply.isValid() ? reply.value() : -1;
}

void MouseManager::onDeviceNumChanged()
{
    int mouseCount = GetMouseCount();
    static int mouseCountBak;
    if (mouseCountBak == mouseCount) {
        return;
    }
    mouseCountBak = mouseCount;

    Q_EMIT MouseCountChanged(mouseCount);
}

QHash<QString, OnKeyChanged> MouseManager::setChangedHandler()
{
    m_settingsChangedHandlers[C_CURSOR_THEME] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_CURSOR_THEME<<GetCursorTheme();
#endif
        emit CursorThemeChanged(GetCursorTheme());
    };
    m_settingsChangedHandlers[C_LOCATE_POINTER] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_LOCATE_POINTER<<GetLocatePointer();
#endif
        emit LocatePointerChanged(GetLocatePointer());
    };
    m_settingsChangedHandlers[C_MOTION_ACCELERATION] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_MOTION_ACCELERATION<<GetMotionAcceleration();
#endif
        emit MotionAccelerationChanged(GetMotionAcceleration());
    };
    m_settingsChangedHandlers[C_MOUSE_ACCEL] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_MOUSE_ACCEL<<GetMouseAccel();
#endif
        emit MouseAccelChanged(GetMouseAccel());
    };
    m_settingsChangedHandlers[C_WHEEL_SPEED] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_WHEEL_SPEED<<GetWheelSpeed();
#endif
        emit WheelSpeedChanged(GetWheelSpeed());
    };
    m_settingsChangedHandlers[C_CURSOR_SIZE] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_CURSOR_SIZE<<GetCursorSize();
#endif
        emit CursorSizeChanged(GetCursorSize());
    };
    m_settingsChangedHandlers[C_DOUBLE_CLICK] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_DOUBLE_CLICK<<GetDoubleClick();
#endif
        emit DoubleClickChanged(GetDoubleClick());
    };
    m_settingsChangedHandlers[C_LEFT_HANDED] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_LEFT_HANDED<<GetLeftHanded();
#endif
        emit LeftHandedChanged(GetLeftHanded());
    };
    m_settingsChangedHandlers[C_MIDDLE_BUTTON_ENABLED] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_MIDDLE_BUTTON_ENABLED<<GetMiddleButtonEnabled();
#endif
        emit MiddleButtonEnabledChanged(GetMiddleButtonEnabled());
    };
    m_settingsChangedHandlers[C_NATURAL_SCROLL] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_NATURAL_SCROLL<<GetNaturalScroll();
#endif
        emit NaturalScrollChanged(GetNaturalScroll());
    };

    return m_settingsChangedHandlers;
}

void MouseManager::initSignals()
{
    QDBusConnection::sessionBus().connect(USD_SESSION_SERVER,
                                         USD_SESSION_PATH,
                                         USD_SESSION_INTERFACE,
                                         USD_SIGNAL_MOUSECOUNTCHANGED,
                                         this,
                                         SIGNAL(MouseCountChanged(int)));
}


QString MouseManager::GetCursorTheme()
{
    PEEK_SCHEMA(C_CURSOR_THEME, C_ERROR_STRING);
    return m_pGsettingsManager->get(C_CURSOR_THEME).toString();
}

int MouseManager::GetLocatePointer() {
    PEEK_SCHEMA(C_LOCATE_POINTER, C_ERROR_NUMBER);
    PEEK_SCHEMA(C_SHAKE_POINTER, C_ERROR_NUMBER);

    if (!m_pGsettingsManager->get(C_LOCATE_POINTER).toBool() &&
            !m_pGsettingsManager->get(C_SHAKE_POINTER).toBool()) {
        return MouseManager::none;
    }

    return m_pGsettingsManager->get(C_LOCATE_POINTER).toBool() ? MouseManager::MouseShowType::CTRL :
                                                    MouseManager::MouseShowType::SHARK;
}

double MouseManager::GetMotionAcceleration() {
    PEEK_SCHEMA(C_MOTION_ACCELERATION, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_MOTION_ACCELERATION).toDouble();
}

bool MouseManager::GetMouseAccel() {
    PEEK_SCHEMA(C_MOUSE_ACCEL, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_MOUSE_ACCEL).toBool();
}

int MouseManager::GetWheelSpeed() {
    PEEK_SCHEMA(C_WHEEL_SPEED, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_WHEEL_SPEED).toInt();
}

int MouseManager::GetCursorSize() {
    PEEK_SCHEMA(C_CURSOR_SIZE, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_CURSOR_SIZE).toInt();
}

int MouseManager::GetDoubleClick() {
    PEEK_SCHEMA(C_DOUBLE_CLICK, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_DOUBLE_CLICK).toInt();
}

bool MouseManager::GetLeftHanded() {
    PEEK_SCHEMA(C_LEFT_HANDED, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_LEFT_HANDED).toBool();
}

bool MouseManager::GetMiddleButtonEnabled() {
    PEEK_SCHEMA(C_MIDDLE_BUTTON_ENABLED, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_MIDDLE_BUTTON_ENABLED).toBool();
}

bool MouseManager::GetNaturalScroll() {
    PEEK_SCHEMA(C_NATURAL_SCROLL,C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_NATURAL_SCROLL).toInt();
}

void MouseManager::SetCursorTheme(const QString &cursorTheme)
{
    const QSet<QString> canSetSize = {"dark-sense", "DMZ-White",  "blue-crystal"};
    PEEK_AND_SET(C_CURSOR_THEME, cursorTheme);
}

void MouseManager::SetLocatePointer(const int locatePointer)
{
    if (locatePointer == MouseManager::MouseShowType::none) {
        {
        PEEK_AND_SET(C_LOCATE_POINTER, false);
        }
        PEEK_AND_SET(C_SHAKE_POINTER, false);
    } else if (locatePointer == MouseManager::MouseShowType::CTRL) {
        PEEK_AND_SET(C_LOCATE_POINTER, true);
        PEEK_AND_SET(C_SHAKE_POINTER, false);
    } else if (locatePointer == MouseManager::MouseShowType::SHARK) {
        PEEK_AND_SET(C_LOCATE_POINTER, false);
        PEEK_AND_SET(C_SHAKE_POINTER, true);
    } else {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("The machine does not support the args"));
        }
    }
}

void MouseManager::SetMotionAcceleration(const double motionAcceleration)
{
    if (motionAcceleration > 10 || motionAcceleration < 1) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("The machine does not support the args"));
        }
        return;
    }
    PEEK_AND_SET(C_MOTION_ACCELERATION, motionAcceleration);
}

void MouseManager::SetMouseAccel(const bool mouseAccel)
{
    PEEK_AND_SET(C_MOUSE_ACCEL, mouseAccel);
}

void MouseManager::SetWheelSpeed(const int wheelSpeed)
{
    PEEK_AND_SET(C_WHEEL_SPEED, wheelSpeed);
}

void MouseManager::SetCursorSize(const int cursorSize)
{
    const QSet<int> canSetSize = {24, 36, 48};

    if (!canSetSize.contains(cursorSize)) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("The machine does not support the args"));
        }
        return;
    }
    PEEK_AND_SET(C_CURSOR_SIZE, cursorSize);
}

void MouseManager::SetDoubleClick(const int doubleClick)
{
    if (doubleClick > 1000 || doubleClick < 170) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("The machine does not support the args"));
        }
        return;
    }

    PEEK_AND_SET(C_DOUBLE_CLICK, doubleClick);
}

void MouseManager::SetLeftHanded(const bool leftHanded)
{
     PEEK_AND_SET(C_LEFT_HANDED, leftHanded);
}

void MouseManager::SetMiddleButtonEnabled(const bool middleButtonEnabled)
{
     PEEK_AND_SET(C_MIDDLE_BUTTON_ENABLED, middleButtonEnabled);
}

void MouseManager::SetMotionThreshold(const double motionThreshold)
{
    PEEK_AND_SET(C_MOTION_THRESHOLD, motionThreshold);
}

void MouseManager::SetNaturalScroll(const bool naturalScroll)
{
    PEEK_AND_SET(C_NATURAL_SCROLL, naturalScroll);
}

