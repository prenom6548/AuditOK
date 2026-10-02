/*
 * Copyright 2021 KylinSoft Co., Ltd.
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

#include <QApplication>
#include <QDebug>
#include <QDBusConnection>
#include <memory>
#include <ukui-log4qt.h>
#include "upowerdbus.h"
#include "batterydbus.h"

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-upower");
    QApplication a(argc, argv);

    std::unique_ptr<UpowerDBus> dbus = std::make_unique<UpowerDBus>();
    std::unique_ptr<BatteryDBus> batDbus = std::make_unique<BatteryDBus>();

    QDBusConnection con = QDBusConnection::sessionBus();
    if (!con.registerService("org.ukui.upower")) {
        qWarning() << "QDbus register service failed reason:" << con.lastError();
        exit(0);
    }

    if (!con.registerObject("/upower", dbus.get(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)
        || !con.registerObject(
            "/upower/BatteryInfo", batDbus.get(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qWarning() << "QDbus register object failed reason:" << con.lastError();
        exit(1);
    }
    return a.exec();
}
