/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <QDBusInterface>
#include <QDebug>
#include <QDBusReply>
#include "dbusupperinterface.h"
#include "msysdbus.h"
#include "rsac.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <QTimer>

MSysDbus::MSysDbus(DbusUpperInterface *interface, QObject *parent) : QObject(parent), m_interface(interface)
{
    registerUniauth();
}

bool MSysDbus::sendPassword(const QString username, QByteArray password)
{
    if (m_interface)
        return m_interface->sendPassword(username, password);

    qWarning() << "m_interface is nullptr";
    return false;
}

QString MSysDbus::getPublicEncrypt()
{
    if (m_interface)
        return m_interface->getPublicEncrypt();

    qWarning() << "m_interface is nullptr";
    return "";
}

void MSysDbus::registerUniauth()
{
    QDBusInterface iface(
        "org.ukui.UniauthBackend", "/org/ukui/UniauthBackend", "org.ukui.UniauthBackend", QDBusConnection::systemBus());
    iface.call("registerLoginApp");
}

void MSysDbus::sendSignalLoginFinished(QString username, bool res)
{
    QDBusInterface iface(
        "org.ukui.UniauthBackend", "/org/ukui/UniauthBackend", "org.ukui.UniauthBackend", QDBusConnection::systemBus());
    iface.call("sendSignalLoginFinished", username, res);
}
