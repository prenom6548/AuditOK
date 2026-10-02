/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 */

#include "watcher-private.h"
#include "statusnotifierwatcheradaptor.h"
#include "statusnotifieriteminterface.h"
#include "watcher.h"

WatcherPrivate::WatcherPrivate(QObject *parent) : QObject(parent)
{
    qDBusRegisterMetaType<UintList>();
    new StatusNotifierWatcherAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/StatusNotifierWatcher"), this);
    dbus.registerService(QStringLiteral("org.kde.StatusNotifierWatcher"));
    m_serviceWatcher = new QDBusServiceWatcher(this);
    m_serviceWatcher->setConnection(dbus);
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &WatcherPrivate::serviceUnregistered);
}

WatcherPrivate::~WatcherPrivate()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService(QStringLiteral("org.kde.StatusNotifierWatcher"));
}

QStringList WatcherPrivate::RegisteredStatusNotifierItems() const
{
    return m_registeredServices.keys();
}

UintList WatcherPrivate::RegisteredStatusNotifierItemPids() const
{
    QStringList pidList =  m_registeredServices.values();
    pidList.removeDuplicates();
    pidList.removeAll("0");
    QList<quint32> pids;
    for(const QString &pid : pidList) {
        if (pid.toUInt() > 0) {
            pids.append(pid.toUInt());
        }
    }
    return pids;
}

bool WatcherPrivate::IsStatusNotifierHostRegistered() const
{
//    return !m_statusNotifierHostServices.isEmpty();
    return true;
}

int WatcherPrivate::ProtocolVersion() const
{
    return 0;
}

void WatcherPrivate::RegisterStatusNotifierItem(const QString &service)
{
    QString realService;
    QString path;
    int slash = service.indexOf('/');
    if (slash == 0) {
        realService = message().service();
        path = service;
    } else if(slash > 0){
        realService = service.left(slash);
        path = service.mid(slash);
    } else {
        realService = service;
        path = QStringLiteral("/StatusNotifierItem");
    }
    QString notifierItemId = realService + path;
    if (m_registeredServices.contains(notifierItemId)) {
        return;
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(realService).value()) {
        // check if the service has registered a SystemTray object
        org::kde::StatusNotifierItem item(realService, path, QDBusConnection::sessionBus());
        if (item.isValid()) {
            qDebug() << "Registering" << notifierItemId << "to system tray";
            m_serviceWatcher->addWatchedService(realService);
            //get Pid for item
            QString pid = "0";
            if(realService.contains(QStringLiteral("org.ukui.proxy.StatusNotifierItem"))) {
                QString pidStr = realService.section("-", 1, 1);
                if(!pidStr.isEmpty()) {
                    pid = pidStr;
                }
            } else {
                QDBusReply<uint> pidReply = connection().interface()->servicePid(message().service());
                if(pidReply.isValid()) {
                    pid = QString::number(pidReply.value());
                }
            }
            m_registeredServices.insert(notifierItemId, pid);
            Q_EMIT StatusNotifierItemRegistered(notifierItemId);
            if(pid.toUInt() > 0) {
                Q_EMIT StatusNotifierItemRegistered(pid.toUInt());
            }
        }
    }
}

void WatcherPrivate::RegisterStatusNotifierHost(const QString &service)
{
    if (service.contains(QLatin1String("org.kde.StatusNotifierHost-")) && QDBusConnection::sessionBus().interface()->isServiceRegistered(service).value()
        && !m_statusNotifierHostServices.contains(service)) {
        qDebug() << "Registering" << service << "as system tray";
        m_statusNotifierHostServices.insert(service);
        m_serviceWatcher->addWatchedService(service);
        Q_EMIT StatusNotifierHostRegistered();
    }
}

void WatcherPrivate::serviceUnregistered(const QString &service)
{
    qDebug() << "Service " << service << "unregistered";
    m_serviceWatcher->removeWatchedService(service);

    const QString match = service + QLatin1Char('/');
    for(const QString &servicePath : m_registeredServices.keys()) {
        if(servicePath.startsWith(match)) {
            QString pid = m_registeredServices.take(servicePath);
            Q_EMIT StatusNotifierItemUnregistered(servicePath);
            if(!m_registeredServices.values().contains(pid) && pid.toUInt() > 0) {
                Q_EMIT StatusNotifierItemUnregistered(pid.toUInt());
            }
        }
    }

    if (m_statusNotifierHostServices.contains(service)) {
        m_statusNotifierHostServices.remove(service);
        Q_EMIT StatusNotifierHostUnregistered();
    }
}

Watcher &Watcher::self()
{
    static Watcher self;
    return self;
}

Watcher::Watcher(): d(new WatcherPrivate)
{
}

Watcher::~Watcher()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}
