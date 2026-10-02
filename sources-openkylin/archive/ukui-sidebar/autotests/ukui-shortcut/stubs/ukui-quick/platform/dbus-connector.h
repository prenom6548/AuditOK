/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef AUTOTESTS_STUB_DBUS_CONNECTOR_H
#define AUTOTESTS_STUB_DBUS_CONNECTOR_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>

namespace UkuiQuick {

class DbusConnector : public QObject
{
    Q_OBJECT
public:
    explicit DbusConnector(const QString &service,
                           const QString &path,
                           const QString &interface,
                           const QDBusConnection &connection,
                           QObject *parent = nullptr)
        : QObject(parent)
        , m_service(service)
        , m_path(path)
        , m_interface(interface)
        , m_connection(connection)
    {
    }

    void start()
    {
    }

    QString service() const
    {
        return m_service;
    }

    QString path() const
    {
        return m_path;
    }

    QString interface() const
    {
        return m_interface;
    }

    QDBusConnection connection() const
    {
        return m_connection;
    }

Q_SIGNALS:
    void ready(QDBusInterface *interface);

private:
    QString m_service;
    QString m_path;
    QString m_interface;
    QDBusConnection m_connection;
};

} // namespace UkuiQuick

#endif // AUTOTESTS_STUB_DBUS_CONNECTOR_H
