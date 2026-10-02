/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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

/*!
    \class UkuiQuick::DbusConnector
    \brief DBus连接工具类，提供异步DBus服务连接。

    DbusConnector是一个用于连接DBus服务的工具类，继承自QThread，
    在后台线程中异步连接DBus服务。该类会自动处理连接超时和重连逻辑。

    \section1 主要特性

    \list
    \li 异步DBus连接
    \li 自动重连机制
    \li 连接超时处理
    \li 后台线程操作
    \li 信号通知机制
    \endlist

    \section1 使用示例

    在C++中使用：
    \code
    auto connector = new UkuiQuick::DbusConnector(
        "com.example.Service",
        "/com/example/Service",
        "com.example.Service.Interface"
    )
    
    connect(connector, &DbusConnector::ready, this, [](QDBusInterface* interface) {
        // 使用DBus接口
    })
    
    connector->start() // 开始异步连接
    \endcode

    \note 该类不会管理QDBusInterface指针的生命周期，需要使用者自行管理
    \note 该类线程不安全，不要在多线程环境中使用

    \sa Settings, AppLauncher
*/

#include "dbus-connector.h"

#include <QCoreApplication>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QTimer>
#include <QTimer>

namespace UkuiQuick {
class DbusConnectorPrivate
{
public:
    DbusConnectorPrivate(const QString& service
                         , const QString& path
                         , const QString& interface
                         , QDBusConnection conn)
        : m_service(service)
        , m_path(path)
        , m_interface(interface)
        , m_conn(std::move(conn))
    {
    }
    QString m_service;
    QString m_path;
    QString m_interface;
    QDBusConnection m_conn;
};

DbusConnector::DbusConnector(const QString &service, const QString& path, const QString& interface, QDBusConnection conn, QObject* parent)
    : QThread(parent), d(new DbusConnectorPrivate(service, path, interface, conn))
{
}

DbusConnector::~DbusConnector()
{
    quit();
    wait();
    if (d) {
        delete d;
        d = nullptr;
    }
}

void DbusConnector::run()
{
    auto createAndValidateInterface = [this]() -> QDBusInterface* {
        auto dbus(new QDBusInterface(d->m_service, d->m_path, d->m_interface, d->m_conn));
        if (dbus->isValid()) {
            qWarning() << "DbusConnector: dbus connection success!";
            dbus->moveToThread(QCoreApplication::instance()->thread());
            return dbus;
        }
        qWarning() << "DBus connection failed:" << dbus->lastError().message();
        delete dbus;
        return nullptr;
    };

    qDebug() << "DbusConnector begin init dbus interface" << d->m_service << d->m_path << d->m_interface;

    auto watcher = new QDBusServiceWatcher(d->m_service, d->m_conn, QDBusServiceWatcher::WatchForRegistration);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, watcher, [this, createAndValidateInterface]() {
        qDebug() << "DbusConnector: service registered, begin init dbus interface" << d->m_service << d->m_path << d->m_interface;
        if (auto dbus = createAndValidateInterface()) {
            Q_EMIT ready(dbus);
        }
        this->quit();
    });

    if (auto dbus = createAndValidateInterface()) {
        Q_EMIT ready(dbus);
        delete watcher;
        return;
    }
    exec();
    delete watcher;
}
}