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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include "usd-common.h"
#include "gsettings_manager.h"
using namespace UKUI;
class Keyboard : public QObject,QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.Keyboard")

    DEFINE_PROPERTY(bool, RepeatState)
    DEFINE_PROPERTY(bool, LockTipState)
public:
    explicit Keyboard(QObject *parent = nullptr);

private:
    /**
     * @brief initSignals 初始化dbus信号连接
     */
    void initSignals();

    QHash<QString, OnKeyChanged> setChangedHandler();
public Q_SLOTS:
    /**
     * @brief GetKbdNum 获取键盘数量
     */
    int GetKbdCount();

    /**
     * @brief GetRepeatState
     * @return 获取按下重复输入状态
     */
    bool GetRepeatState();

    /**
     * @brief GetLockTipState
     * @return 获取caps lock是否显示的状态
     */
    bool GetLockTipState();

    /**
     * @brief SetRepeatState
     * @param state
     *设置重复状态
     */
    void SetRepeatState(bool state);

    /**
     * @brief SetLockTioState
     * @param state
     * 设置caplock状态
     */
    void SetLockTipState(bool state);
Q_SIGNALS:
    /**
     * @brief KbdNumChanged 键盘数量改变信号
     */
    void KbdCountChanged(int count);

    //按下重复
    void RepeatStateChanged(bool state);

    void LockTipStateChanged(bool state);
private:
    GsettingsManager *m_pGsettingsManager = nullptr;
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;
    const QByteArray C_KEYBOARD_SCHEMA = "org.ukui.peripherals-keyboard";

    /**
     * @brief C_KEYBOARD_REPEAT    持续按下重复输入
     */
    const QString C_KEYBOARD_REPEAT = "repeat";

    const QString C_KEYBOARD_SHOW_TIP = "show-lock-tip";

    const QString C_ERROR_STRING = "";

    const int C_ERROR_NUMBER = 0;
};

#endif // KEYBOARD_H
