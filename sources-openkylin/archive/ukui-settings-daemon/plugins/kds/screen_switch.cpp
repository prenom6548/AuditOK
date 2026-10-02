/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "screen_switch.h"
#include <QDBusInterface>
#include <QDBusConnection>
#include <QApplication>
#include <QJsonDocument>
#include "usd_global_define.h"
#include "clib-syslog.h"

Q_GLOBAL_STATIC(ScreenSwitch, s_screenSwitch)
ScreenSwitch::ScreenSwitch(QObject *parent) : QObject(parent)
{
    m_xrandrInterface = new QDBusInterface(DBUS_XRANDR_NAME,
                                           DBUS_XRANDR_PATH,
                                           DBUS_XRANDR_INTERFACE,
                                           QDBusConnection::sessionBus(),
                                           this);
    if (m_xrandrInterface->isValid()) {
        connect(m_xrandrInterface, SIGNAL(screensParamChanged(QString)), this, SIGNAL(sigScreenParamChanged(QString)));
    }
}

ScreenSwitch *ScreenSwitch::instance()
{
    return s_screenSwitch;
}

QStringList ScreenSwitch::getScreenList()
{
    if (m_xrandrInterface) {
        QDBusReply<QString> replay = m_xrandrInterface->call(DBUS_XRANDR_GET_SCREEN_PARAM, qAppName());
        if (replay.isValid()) {
            QStringList screenList;
            QJsonDocument parser;
            QVariantList screens = parser.fromJson(replay.value().toUtf8().data()).toVariant().toList();
            for (const auto& screenInfo : screens) {
                const QString& outputName = ((screenInfo.toMap())[("metadata")].toMap())["name"].toString();
                screenList << outputName;
            }
            return screenList;
        }
    }
    return QStringList();
}

int ScreenSwitch::getScreenMode()
{
    if (m_xrandrInterface) {
        QDBusReply<int> replay = m_xrandrInterface->call(DBUS_XRANDR_GET_MODE, qAppName());
        if (replay.isValid()) {
            return replay.value();
        }
    }
    return 0;
}

void ScreenSwitch::setScreenMode(const QString& mode)
{
    if (m_xrandrInterface) {
        USD_LOG(LOG_DEBUG, "set screen %s", mode.toLatin1().data());
        m_xrandrInterface->asyncCall(DBUS_XRANDR_SET_MODE , mode, qAppName());
    }
}

