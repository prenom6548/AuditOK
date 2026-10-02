/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "upowerlinepowerdbus.h"

UpowerLinePowerDBus::UpowerLinePowerDBus(int linePowerId, const QString &linePowerDBusPath, QObject *parent)
    : QObject(parent), m_linePowerId(linePowerId)
{
    m_DBusInterface = new QDBusInterface("org.freedesktop.UPower",
                                         linePowerDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus(),
                                         this);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         linePowerDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealLinePowerPropertiesMessage(QDBusMessage)));

    QDBusReply<QVariant> reply = m_DBusInterface->call("Get", "org.freedesktop.UPower.Device", "Online");
    if (reply.isValid()) {
        m_acOnlineState = reply.value().toBool();
    } else {
        m_acOnlineState = false;
    }
}

UpowerLinePowerDBus::~UpowerLinePowerDBus() {}

void UpowerLinePowerDBus::dealLinePowerPropertiesMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    if (map.contains("Online")) {
        bool acOnlineState = map.value(QString("Online")).toBool();
        if (m_acOnlineState != acOnlineState) {
            m_acOnlineState = acOnlineState;
            emit acOnlineStateChanged(m_linePowerId, m_acOnlineState);
        }
    }
}

int UpowerLinePowerDBus::getAcOnlineState()
{
    return m_acOnlineState;
}
