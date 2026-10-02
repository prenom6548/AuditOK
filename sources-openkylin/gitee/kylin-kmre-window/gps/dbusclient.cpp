/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusclient.h"
namespace kmre_gps
{
void DbusClient::sendGpsData(QString gpsData)
{
    QDBusInterface *m_gpsBusInterface =
        new QDBusInterface("com.kylin.Kmre.gpsserver", "/", "com.kylin.Kmre.gpsserver", QDBusConnection::sessionBus());
    if (m_gpsBusInterface->isValid()) {
        QDBusReply<QString> reply = m_gpsBusInterface->call("passGpsData", gpsData);
        if (!reply.isValid()) {
            // syslog(LOG_ERR, "Failed to call sensor server interface");
        }
        delete m_gpsBusInterface;
        m_gpsBusInterface = nullptr;
    }
}

} // namespace kmre_gps
