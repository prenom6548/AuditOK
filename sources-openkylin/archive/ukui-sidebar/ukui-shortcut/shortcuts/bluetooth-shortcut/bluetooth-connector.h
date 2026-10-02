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
#ifndef BLUETOOTHCONNECTOR_H
#define BLUETOOTHCONNECTOR_H

#include <QObject>
#include <QThread>
#include <QDBusInterface>

#define BLUETOOTH_SERVICE_NAME         "com.ukui.bluetooth"
#define BLUETOOTH_SERVICE_PATH         "/com/ukui/bluetooth"
#define BLUETOOTH_SERVICE_INTERFACE    "com.ukui.bluetooth"

class BluetoothConnector : public QThread
{
    Q_OBJECT
public:
    BluetoothConnector(QObject *parent = nullptr);
Q_SIGNALS:
    void ready(QDBusInterface *);
    void failed();
private:
    void run();
    QDBusInterface *m_interface = nullptr;
};

#endif // BLUETOOTHCONNECTOR_H
