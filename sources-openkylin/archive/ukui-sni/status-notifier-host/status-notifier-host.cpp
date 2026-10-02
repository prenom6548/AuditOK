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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QHash>
#include <mutex>
#include "status-notifier-host.h"
#include "status-notifier-item.h"
#include "statusnotifierwatcher_interface.h"
#include "dbusproperties.h"

namespace UkuiSni {

static std::once_flag flag;
static StatusNotifierHost *s_self;
static const QString s_watcherServiceName(QStringLiteral("org.kde.StatusNotifierWatcher"));
class StatusNotifierHostPrivate : public QObject
{
    friend class StatusNotifierHost;
    Q_OBJECT
public:
    explicit StatusNotifierHostPrivate(StatusNotifierHost *q);
    ~StatusNotifierHostPrivate();

    void registerHost();
private:
    void serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner);
    void registerHost(const QString &service);
    void unregisterHost(const QString &service);
    void itemRegistered(const QString &service);
    void itemUnregistered(const QString &service);
    void removeAllItems();
    QString displayFromPid(uint pid);

    org::kde::StatusNotifierWatcher *m_statusNotifierWatcher = nullptr;
    QString m_serviceName;
    static const int s_protocolVersion = 0;
    QHash<QString, StatusNotifierItem *> m_registeredItems;
    StatusNotifierHost *q = nullptr;
    QString m_display;
    QString m_sessionType;
};

StatusNotifierHostPrivate::StatusNotifierHostPrivate(StatusNotifierHost *q) : QObject(q), q(q)
{
    if(QString(getenv("XDG_SESSION_TYPE")) == "wayland") {
        m_sessionType = "wayland";
        m_display = getenv("WAYLAND_DISPLAY");
    } else {
        m_sessionType = "x11";
        m_display = getenv("DISPLAY");
    }
    qDebug() << "Current DISPLAY: " << m_display << "XDG_SESSION_TYPE" << m_sessionType;
}

StatusNotifierHostPrivate::~StatusNotifierHostPrivate()
{
}

void StatusNotifierHostPrivate::registerHost()
{
    if (QDBusConnection::sessionBus().isConnected()) {
        m_serviceName = "org.kde.StatusNotifierHost-" + QString::number(QCoreApplication::applicationPid());
        QDBusConnection::sessionBus().registerService(m_serviceName);

        auto *watcher = new QDBusServiceWatcher(s_watcherServiceName,
                                                QDBusConnection::sessionBus(),
                                                QDBusServiceWatcher::WatchForOwnerChange,
                                                this);
        connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &StatusNotifierHostPrivate::serviceChange);

        registerHost(s_watcherServiceName);
    }
}

void StatusNotifierHostPrivate::serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    qDebug() << "Service" << name << "status change, old owner:" << oldOwner << "new:" << newOwner;

    if (newOwner.isEmpty()) {
        unregisterHost(name);
    } else if (oldOwner.isEmpty()) {
        registerHost(name);
    }
}

void StatusNotifierHostPrivate::registerHost(const QString &service)
{
    if (service == s_watcherServiceName) {
        if(m_statusNotifierWatcher) {
            delete m_statusNotifierWatcher;
            m_statusNotifierWatcher = nullptr;
        }

        m_statusNotifierWatcher = new org::kde::StatusNotifierWatcher(s_watcherServiceName,
                                                                      QStringLiteral("/StatusNotifierWatcher"),
                                                                      QDBusConnection::sessionBus(),
                                                                      this);
        if (m_statusNotifierWatcher->isValid()) {
            m_statusNotifierWatcher->call(QDBus::NoBlock, QStringLiteral("RegisterStatusNotifierHost"), m_serviceName);
            connect(m_statusNotifierWatcher, QOverload<const QString &>::of(&OrgKdeStatusNotifierWatcherInterface::StatusNotifierItemRegistered),
                    this, &StatusNotifierHostPrivate::itemRegistered);
            connect(m_statusNotifierWatcher, QOverload<const QString &>::of(&OrgKdeStatusNotifierWatcherInterface::StatusNotifierItemUnregistered),
                    this, &StatusNotifierHostPrivate::itemUnregistered);

            OrgFreedesktopDBusPropertiesInterface propertiesIface(m_statusNotifierWatcher->service(),
                                                                  m_statusNotifierWatcher->path(),
                                                                  m_statusNotifierWatcher->connection());

            QDBusPendingReply<QDBusVariant> pendingItems = propertiesIface.Get(m_statusNotifierWatcher->interface(), "RegisteredStatusNotifierItems");

            auto *watcher = new QDBusPendingCallWatcher(pendingItems, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [&, watcher]() {
                watcher->deleteLater();
                QDBusReply<QDBusVariant> reply = *watcher;
                QStringList registeredItems = reply.value().variant().toStringList();
                        for(const QString &service : registeredItems) {
                        if (!m_registeredItems.contains(service)) {
                            itemRegistered(service);
                        }
                    }
            });
        } else {
            if(m_statusNotifierWatcher) {
                delete m_statusNotifierWatcher;
                m_statusNotifierWatcher = nullptr;
            }
            qDebug() << "Status notifier Watcher not reachable";
        }
    }
}

void StatusNotifierHostPrivate::unregisterHost(const QString &service)
{
    if (service == s_watcherServiceName) {
        removeAllItems();
        if(m_statusNotifierWatcher) {
            delete m_statusNotifierWatcher;
            m_statusNotifierWatcher = nullptr;
        }
    }
}

void StatusNotifierHostPrivate::itemRegistered(const QString &service)
{
    if(!m_registeredItems.contains(service)) {
        QString realService;
        int slash = service.indexOf('/');
        if (slash == 0) {
            qWarning() << "Invalid service:" << service;
            return;
        } else if(slash > 0){
            realService = service.left(slash);
        } else {
            realService = service;
        }
        uint pid = 0;
        if(realService.contains(QStringLiteral("org.ukui.proxy.StatusNotifierItem"))) {
            QString pidStr = realService.section("-", 1, 1);
            if(!pidStr.isEmpty()) {
                pid = pidStr.toUInt();
            }
        } else {
            QDBusReply<uint> pidReply = QDBusConnection::sessionBus().interface()->servicePid(realService);
            if(pidReply.isValid()) {
                pid = pidReply.value();
            }
        }
        QString display = pid > 0 ? displayFromPid(pid) : "";
        if(m_display != display && !display.isEmpty()) {
            return;
        }
        auto *item = new StatusNotifierItem(service);
        item->setPid(pid);
        m_registeredItems.insert(service, item);
        Q_EMIT q->itemAdded(service);
        qDebug() << "Registering sni" << service;
    }
}

void StatusNotifierHostPrivate::itemUnregistered(const QString &service)
{
    if (m_registeredItems.contains(service)) {
        Q_EMIT q->itemRemoved(service);
        auto item = m_registeredItems.value(service);
        item->disconnect();
        item->deleteLater();
        m_registeredItems.remove(service);
    }
}

void StatusNotifierHostPrivate::removeAllItems()
{
    for(const QString &service : m_registeredItems.keys()) {
        Q_EMIT q->itemRemoved(service);
        auto item = m_registeredItems.value(service);
        item->disconnect();
        item->deleteLater();
        m_registeredItems.remove(service);
    }
}

QString StatusNotifierHostPrivate::displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY = m_sessionType == "wayland" ? QByteArrayLiteral("WAYLAND_DISPLAY")
                                                              : QByteArrayLiteral("DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }
    return {};
}

StatusNotifierHost *StatusNotifierHost::self()
{
    std::call_once(flag, [ & ] {
        s_self = new StatusNotifierHost();
    });
    return s_self;
}

StatusNotifierHost::StatusNotifierHost(QObject *parent): QObject(parent), d(new StatusNotifierHostPrivate(this))
{
}

StatusNotifierHost::~StatusNotifierHost()
{
}

QList<StatusNotifierItem *> StatusNotifierHost::items()
{
    return d->m_registeredItems.values();
}

const QList<QString> StatusNotifierHost::services() const
{
    return d->m_registeredItems.keys();
}

StatusNotifierItem *StatusNotifierHost::itemForService(const QString service)
{
    return d->m_registeredItems.value(service);
}

void StatusNotifierHost::registerHost()
{
    d->registerHost();
}
}
#include "status-notifier-host.moc"
