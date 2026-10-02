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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "bluetooth-connector.h"
#include <QTime>
#include <QDebug>
#include <QApplication>

BluetoothConnector::BluetoothConnector(QObject *parent) : QThread(parent)
{
}

void BluetoothConnector::run()
{
    QTime dieTime = QTime::currentTime().addMSecs(3000);
    while(QTime::currentTime() < dieTime) {
        qDebug() << "bluetooth short begin init dbus interface";
        m_interface = new QDBusInterface(BLUETOOTH_SERVICE_NAME,
                                         BLUETOOTH_SERVICE_PATH,
                                         BLUETOOTH_SERVICE_INTERFACE,
                                         QDBusConnection::systemBus());
        if (m_interface->isValid()) {
            qWarning() << "BluetoothConnector: dbus connection success!";
            m_interface->moveToThread(QApplication::instance()->thread());
            Q_EMIT ready(m_interface);
            return;
        } else {
            delete m_interface;
        }
    }
    qWarning() << "BluetoothConnector: dbus connection failed!";
    Q_EMIT failed();
}
