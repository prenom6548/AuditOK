/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */

#include "wifi-dbus-query.h"
#include <QProcess>
#include <QDBusReply>
#include <QDBusInterface>
#include <QApplication>
#include <QTranslator>
#include <QDebug>

WifiDbusQuery::WifiDbusQuery(QObject *parent) : QObject(parent)
{

}

void WifiDbusQuery::getNetworkDeviceData(QDBusInterface *networkInterface)
{
    if (networkInterface != nullptr) {
        QVariantList data;
        QDBusReply<QVariantList> result = networkInterface->call("getNetworkDeviceData", 0);
        QVariantList outArgs;

        if (result.isValid()) {
            outArgs = result.value();
            if (outArgs.isEmpty()) {
                qWarning() << "netWorkdbus call getNetworkDeviceData(0) return empty";
                data.append({0, 0, ""});
            } else {
                data.append(outArgs);
            }
        } else {
            QDBusError error = result.error();
            qWarning() << "netWorkdbus call getNetworkDeviceData(0) failed: " << error.name() << "-" << error.message();
            data.append({0, 0, ""});
        }

        result = networkInterface->call("getNetworkDeviceData", 1);
        if (result.isValid()) {
            outArgs = result.value();
            if (outArgs.isEmpty()) {
                qWarning() << "netWorkdbus call getNetworkDeviceData(1) return empty";
                data.append({0, 0, ""});
            } else {
                data.append(outArgs);
            }
        } else {
            QDBusError error = result.error();
            qWarning() << "netWorkdbus call getNetworkDeviceData(1) failed: " << error.name() << "-" << error.message();
            data.append({0, 0, ""});
        }

        Q_EMIT networkDeviceData(data);

    } else {
        qWarning() <<"netWorkdbus is nullptr, return false";
    }
}
