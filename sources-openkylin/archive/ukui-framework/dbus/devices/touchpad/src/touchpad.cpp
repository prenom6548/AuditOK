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
 */

#include "touchpad.h"

TouchpadManager::TouchpadManager(QObject *parent):
    QObject(parent)
{
    m_pGsettingsManager = new GsettingsManager(this, C_TOUCHPAD_SCHEMA ,setChangedHandler());
    initSignals();
    setChangedHandler();
}

TouchpadManager::~TouchpadManager()
{
    m_settingsChangedHandlers.clear();
    if (m_pGsettingsManager != nullptr) {
        delete m_pGsettingsManager;
        m_pGsettingsManager = nullptr;
    }
}

void TouchpadManager::SetTouchpadEnabled(bool enabled)
{
    PEEK_AND_SET(C_TOUCHPAD_ENABLE, enabled);
}

void TouchpadManager::SetDisableOnExternalMouse(bool disabled)
{
    PEEK_AND_SET(C_DISABLE_ON_EXTERNAL_MOUSE, disabled);
}


void TouchpadManager::SetMotionAcceleration(double acceleration)
{
    PEEK_AND_SET(C_MOTION_ACCELERATION, acceleration);
}

int TouchpadManager::GetTouchpadCount()
{
    QDBusInterface deviceManagerInterface(USD_SESSION_SERVER,
                                USD_SESSION_PATH,
                                USD_SESSION_INTERFACE,
                                QDBusConnection::sessionBus());
    QDBusReply<int> reply = deviceManagerInterface.call(USD_METHOD_GETDEVCOUNTBYTYPE, "touchpad");
    return reply.isValid() ? reply.value() : -1;
}

void TouchpadManager::SetDisableWhileTyping(bool disabled)
{
    PEEK_AND_SET(C_DISABLE_WHILE_TYPING, disabled);
}

void TouchpadManager::SetTapToClick(bool enabled)
{
    PEEK_AND_SET(C_TAP_TO_CLICK, enabled);
}

void TouchpadManager::SetNaturalScroll(bool enabled)
{
    PEEK_AND_SET(C_NATURAL_SCROLL, enabled);
}

bool TouchpadManager::GetTouchpadEnabled()
{
    PEEK_SCHEMA(C_TOUCHPAD_ENABLE,C_ERROR_NUMBER);
    return getGSetting(C_TOUCHPAD_ENABLE).toBool();
}

bool TouchpadManager::GetDisableOnExternalMouse()
{
    PEEK_SCHEMA(C_DISABLE_ON_EXTERNAL_MOUSE,C_ERROR_NUMBER);
    return getGSetting(C_DISABLE_ON_EXTERNAL_MOUSE).toBool();
}

double TouchpadManager::GetMotionAcceleration()
{
    PEEK_SCHEMA(C_MOTION_ACCELERATION,C_ERROR_NUMBER);
    return getGSetting(C_MOTION_ACCELERATION).toDouble();
}

bool TouchpadManager::GetDisableWhileTyping()
{
    PEEK_SCHEMA(C_DISABLE_WHILE_TYPING,C_ERROR_NUMBER);
    return getGSetting(C_DISABLE_WHILE_TYPING).toBool();
}

bool TouchpadManager::GetTapToClick()
{
    PEEK_SCHEMA(C_TAP_TO_CLICK,C_ERROR_NUMBER);
    return getGSetting(C_TAP_TO_CLICK).toBool();
}

bool TouchpadManager::GetNaturalScroll()
{
    PEEK_SCHEMA(C_NATURAL_SCROLL,C_ERROR_NUMBER);
    return getGSetting(C_NATURAL_SCROLL).toBool();
}

int TouchpadManager::GetScrollType()
{
    if (isEdgeScroll()) {
        return ScrollType::edge_scroll;
    }

    if (isTwoFingerScroll()) {
        return ScrollType::two_finger_scroll;
    }

    return ScrollType::none;
}

void TouchpadManager::SetScrollType(int type)
{
    QString errorInfo;
    if (!m_pGsettingsManager->peekActionIsValid(C_HORIZONTAL_EDGE_SCROLLING, errorInfo)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));\
        return;
    }

    switch(type) {
    case ScrollType::none:
        setEdgeScroll(false);
        setTwoScroll(false);
        break;
    case ScrollType::edge_scroll:
        setEdgeScroll(true);
        setTwoScroll(false);
        break;
    case ScrollType::two_finger_scroll:
        setEdgeScroll(false);
        setTwoScroll(true);
        break;
    }
}

void TouchpadManager::onTouchpadGsettingsChanged(QString key)
{
    if(key == C_TOUCHPAD_ENABLE) {
        emit TouchpadEnabledChanged(getGSetting(key).toBool());
    } else if(key == C_DISABLE_ON_EXTERNAL_MOUSE) {
        emit DisableOnExternalMouseChanged(getGSetting(key).toBool());
    } else if(key == C_MOTION_ACCELERATION) {
        emit MotionAccelerationChanged(getGSetting(key).toDouble());
    } else if(key == C_DISABLE_WHILE_TYPING) {
        emit DisableWhileTypingChanged(getGSetting(key).toBool());
    } else if(key == C_TAP_TO_CLICK) {
        emit TapToClickChanged(getGSetting(key).toBool());
    } else if(key == C_NATURAL_SCROLL) {
        emit NaturalScrollChanged(getGSetting(key).toBool());
    } else if (key == C_HORIZONTAL_EDGE_SCROLLING || key == C_VERTICAL_EDGE_SCROLLING ||
               key == C_HORIZONTAL_TWO_FINGER_SCROLLING || key == C_VERTICAL_TWO_FINGER_SCROLLING) {
        emitScrollTypeChanged();
    }
}

void TouchpadManager::setTouchpadGSetting(const QString &key, const QVariant &value)
{
    m_pGsettingsManager->set(key, value);
}

QVariant TouchpadManager::getGSetting(const QString &key)
{
    return m_pGsettingsManager->get(key);
}

bool TouchpadManager::isEdgeScroll()
{
    return getGSetting(C_HORIZONTAL_EDGE_SCROLLING).toBool() &&
            getGSetting(C_VERTICAL_EDGE_SCROLLING).toBool();
}

bool TouchpadManager::isTwoFingerScroll()
{
    return getGSetting(C_HORIZONTAL_TWO_FINGER_SCROLLING).toBool() &&
            getGSetting(C_VERTICAL_TWO_FINGER_SCROLLING).toBool();
}

bool TouchpadManager::isNoneScroll()
{
   return !getGSetting(C_HORIZONTAL_TWO_FINGER_SCROLLING).toBool() &&
            !getGSetting(C_VERTICAL_TWO_FINGER_SCROLLING).toBool() &&
            !getGSetting(C_HORIZONTAL_EDGE_SCROLLING).toBool() &&
            !getGSetting(C_VERTICAL_EDGE_SCROLLING).toBool();
}

void TouchpadManager::setEdgeScroll(bool enable)
{
    setTouchpadGSetting(C_HORIZONTAL_EDGE_SCROLLING, enable);
    setTouchpadGSetting(C_VERTICAL_EDGE_SCROLLING, enable);
}

void TouchpadManager::setTwoScroll(bool enable)
{
    setTouchpadGSetting(C_HORIZONTAL_TWO_FINGER_SCROLLING, enable);
    setTouchpadGSetting(C_VERTICAL_TWO_FINGER_SCROLLING, enable);
}

void TouchpadManager::emitScrollTypeChanged()
{
    if (isNoneScroll()) {
        Q_EMIT ScrollTypeChanged(ScrollType::none);
    } else if (isEdgeScroll()) {
        Q_EMIT ScrollTypeChanged(ScrollType::edge_scroll);
    } else if (isTwoFingerScroll()) {
        Q_EMIT ScrollTypeChanged(ScrollType::two_finger_scroll);
    }
}

template <typename...Args>
QDBusMessage TouchpadManager::callInputDbus(const QString &method, Args &&...args)
{
    QDBusInterface inputManagerInterface(USD_SESSION_SERVER,
                                         USD_SESSION_PATH,
                                         USD_SESSION_INTERFACE);
    return inputManagerInterface.call(method, std::forward<Args>(args)...);
 }

void TouchpadManager::initSignals()
{
    QDBusConnection::sessionBus().connect(USD_SESSION_SERVER,
                                         USD_SESSION_PATH,
                                         USD_SESSION_INTERFACE,
                                         USD_SIGNAL_TOUCHPADCOUNTCHANGED,
                                         this,
                                         SIGNAL(TouchpadCountChanged(int)));


}

void TouchpadManager::onDeviceNumChanged()
{
    int touchpadCount = GetTouchpadCount();
    static int touchpadCountBak;
    if (touchpadCountBak == touchpadCount) {
        return;
    }
    touchpadCountBak = touchpadCount;

    Q_EMIT TouchpadCountChanged(touchpadCount);
}

QHash<QString, OnKeyChanged> TouchpadManager::setChangedHandler()
{
    m_settingsChangedHandlers[C_TOUCHPAD_ENABLE] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_TOUCHPAD_ENABLE<<GetTouchpadEnabled();
#endif
        emit TouchpadEnabledChanged(GetTouchpadEnabled());
    };
    m_settingsChangedHandlers[C_DISABLE_ON_EXTERNAL_MOUSE] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_DISABLE_ON_EXTERNAL_MOUSE<<GetDisableOnExternalMouse();
#endif
        emit DisableOnExternalMouseChanged(GetDisableOnExternalMouse());
    };
    m_settingsChangedHandlers[C_DISABLE_WHILE_TYPING] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_DISABLE_WHILE_TYPING<<GetDisableWhileTyping();
#endif
        emit DisableWhileTypingChanged(GetDisableWhileTyping());
    };
    m_settingsChangedHandlers[C_MOTION_ACCELERATION] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_MOTION_ACCELERATION<<GetMotionAcceleration();
#endif
        emit MotionAccelerationChanged(GetMotionAcceleration());
    };
    m_settingsChangedHandlers[C_TAP_TO_CLICK] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_TAP_TO_CLICK<<GetTapToClick();
#endif
        emit TapToClickChanged(GetTapToClick());
    };
    m_settingsChangedHandlers[C_NATURAL_SCROLL] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_NATURAL_SCROLL<<GetNaturalScroll();
#endif
        emit NaturalScrollChanged(GetNaturalScroll());
    };
    m_settingsChangedHandlers[C_VERTICAL_EDGE_SCROLLING] = [this]{
        emit emitScrollTypeChanged();
    };
    m_settingsChangedHandlers[C_HORIZONTAL_EDGE_SCROLLING] = [this]{
        emit emitScrollTypeChanged();
    };
    m_settingsChangedHandlers[C_VERTICAL_TWO_FINGER_SCROLLING] = [this]{
        emit emitScrollTypeChanged();
    };
    m_settingsChangedHandlers[C_HORIZONTAL_TWO_FINGER_SCROLLING] = [this]{
        emit emitScrollTypeChanged();
    };

    return m_settingsChangedHandlers;

}
