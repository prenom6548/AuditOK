/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "dbus.h"
#include "controller/core/core.h"
#include <QDBusConnection>
#include <QDBusError>

Dbus::Dbus(Core *core) : m_core(core) {}

bool Dbus::registerService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("org.kylin.weather")) {
        qWarning() << "Register DBus service failed:" << connection.lastError().message();
        return false;
    }
    if (!connection.registerObject("/org/kylin/weather", this, QDBusConnection::ExportAllSlots)) {
        qWarning() << "Register DBus object failed:" << connection.lastError().message();
        return false;
    }
    m_connectSeccess = true;
    return true;
}

QString Dbus::showCityWeather(const QString &cityName)
{
    if (!m_core) {
        return "{\"status\": 1}";
    }

    // 调用Core中的逻辑处理城市切换和显示
    return m_core->showCityWeather(cityName);
}
