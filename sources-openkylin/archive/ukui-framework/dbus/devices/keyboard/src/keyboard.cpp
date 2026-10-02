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

#include "keyboard.h"

Keyboard::Keyboard(QObject *parent) : QObject(parent)
{
    qInfo() << "Initializing Keyboard";
    m_pGsettingsManager = new GsettingsManager(parent, C_KEYBOARD_SCHEMA, setChangedHandler());
    initSignals();
}

int Keyboard::GetKbdCount()
{
    QDBusInterface deviceManagerInterface(USD_SESSION_SERVER,
                                USD_SESSION_PATH,
                                USD_SESSION_INTERFACE,
                                QDBusConnection::sessionBus());
    QDBusReply<int> reply = deviceManagerInterface.call(USD_METHOD_GETDEVCOUNTBYTYPE, "kbd");
    return reply.isValid() ? reply.value() : -1;
}

bool Keyboard::GetRepeatState()
{
    PEEK_SCHEMA(C_KEYBOARD_REPEAT, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_KEYBOARD_REPEAT).toBool();
}

bool Keyboard::GetLockTipState()
{
    PEEK_SCHEMA(C_KEYBOARD_SHOW_TIP, C_ERROR_NUMBER);
    return m_pGsettingsManager->get(C_KEYBOARD_SHOW_TIP).toBool();
}


void Keyboard::SetRepeatState(bool state)
{
    PEEK_AND_SET(C_KEYBOARD_REPEAT, state);
}

void Keyboard::SetLockTipState(bool state)
{
    PEEK_AND_SET(C_KEYBOARD_SHOW_TIP, state);
}


void Keyboard::initSignals()
{
    QDBusConnection::sessionBus().connect(USD_SESSION_SERVER,
                                         USD_SESSION_PATH,
                                         USD_SESSION_INTERFACE,
                                         USD_SIGNAL_KBDCOUNTCHANGED,
                                         this,
                                          SIGNAL(KbdCountChanged(int)));
}

QHash<QString, OnKeyChanged> Keyboard::setChangedHandler()
{
    m_settingsChangedHandlers[C_KEYBOARD_REPEAT] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_KEYBOARD_REPEAT<<GetRepeatState();
#endif
        emit RepeatStateChanged(GetRepeatState());
    };
    m_settingsChangedHandlers[C_KEYBOARD_SHOW_TIP] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_KEYBOARD_SHOW_TIP<<GetLockTipState();
#endif
        emit LockTipStateChanged(GetLockTipState());
    };

    return m_settingsChangedHandlers;
}

