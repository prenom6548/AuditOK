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

#include <QCoreApplication>
#include <QDebug>

#include "input_device_manager.h"
#include "manager_adaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    InputDeviceManager *manager = new InputDeviceManager(nullptr);

    new ManagerAdaptor(manager);

    //QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusConnection conn = QDBusConnection::systemBus();

    if (!conn.registerService("com.kylin.inputdevice.manager")) {
        qDebug() << conn.lastError().message();
    }

    conn.registerObject("/", manager);
    return a.exec();
}
