/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "RegisterNumberInfoCollection.h"

RegisterNumberInfoCollection::RegisterNumberInfoCollection(QObject *parent) : InfoCollectionBase(parent)
{

}

QString RegisterNumberInfoCollection::getSerialNumber()
{
    QString retStr;
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qWarning() << "RegisterNumberInfoCollection::getSerialNumbe: system bus not connected";
        return retStr;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        s_serverName, s_objectPath, s_interfaceName, s_methodName);

    // 设置硬超时，避免卡死（可按需调大，建议 2~5s）
    const int timeoutMs = 3000;
    QDBusMessage reply = bus.call(msg, QDBus::Block, timeoutMs);

    if (reply.type() == QDBusMessage::ReplyMessage &&
        reply.arguments().size() == 2 &&
        reply.arguments().last().toInt() == 0)
    {
        retStr = reply.arguments().first().toString();
    } else {
        qWarning() << "RegisterNumberInfoCollection: dbus call failed"
                   << reply.errorName() << reply.errorMessage();
    }
    return retStr;
}