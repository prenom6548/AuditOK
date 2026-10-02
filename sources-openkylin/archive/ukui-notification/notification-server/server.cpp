/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include "server.h"
#include "server-private.h"
#include "notificationsadaptor.h"
#include "notificationserveradaptor.h"
#include "notificationsettingsadaptor.h"
#include "notification-server-config.h"
#include "utils.h"
#include "notification-close-reason.h"
#include "notification-settings/settings-manager.h"
#include "notification-manager.h"
#include "security-control-config.h"
using namespace NotificationServer;
using namespace UkuiNotification;

ServerPrivate::ServerPrivate(QObject *parent)
    : QObject(parent)
    , m_notificationWatchers(new QDBusServiceWatcher(this))
    , m_notificationControlRegister(new NotificationControlRegister)
    , m_notificationDefaultDirWatcher(new NotificationDefaultDirWatcher(m_notificationControlRegister, this))
    , m_notificationSettingsQuery(new NotificaitonSettingsQuery(this))
    , m_securityControlManager(std::make_unique<SecurityControlManager>(m_notificationControlRegister))
{
    m_notificationWatchers->setConnection(QDBusConnection::sessionBus());
    m_notificationWatchers->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_notificationWatchers, &QDBusServiceWatcher::serviceUnregistered,
            m_notificationWatchers, &QDBusServiceWatcher::removeWatchedService);

    connect(this, &ServerPrivate::NotificationClosed, &NotificationManager::self() ,&NotificationManager::onNotificationClosed);
    connect(m_notificationSettingsQuery, &NotificaitonSettingsQuery::alarmSoundAllowedChanged, this, &ServerPrivate::AlarmSoundAllowedChanged);
    connect(m_notificationSettingsQuery, &NotificaitonSettingsQuery::dndChanged, this, &ServerPrivate::DNDChanged);
}

ServerPrivate::~ServerPrivate() = default;

QStringList ServerPrivate::GetCapabilities() const
{
    return QStringList();
}

uint ServerPrivate::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                           const QString &body, const QStringList &actions, const QVariantMap &hints, int timeout)
{
    if (!m_securityControlManager->isEnabledNotification()) {
        qWarning() << "Under security control! All notifications are disabled now!";
        return 0;
    }
    uint id = 0;
    if(replaces_id > 0) {
        id = replaces_id;
    } else {
        id = m_increasedNotificationId++;
    }
    QVariantMap newHints = hints;
    newHints.insert(QStringLiteral("x-ukui-createdTime"), QDateTime::currentDateTime().toString());

    uint pid = 0;
    if (calledFromDBus()) {
        QDBusReply<uint> pidReply = connection().interface()->servicePid(message().service());
        if(pidReply.isValid()) {
            pid = pidReply.value();
        }
    }

    if (newHints.value(QStringLiteral("x-ukui-display")).toString().isEmpty() && pid > 0) {
        newHints.insert(QStringLiteral("x-ukui-display"), UkuiNotification::Utils::displayFromPid(pid));
    }

    QString desktopEntry = newHints.value(QStringLiteral("desktop-entry")).toString();
    const QString& checkDesktopEntry = UkuiNotification::Utils::desktopEntryFromName(desktopEntry);
    if (!checkDesktopEntry.isEmpty()) {
        desktopEntry = UkuiNotification::Utils::desktopEntry2Name(checkDesktopEntry);
    }

    if (desktopEntry.isEmpty() && pid > 0) {
        qInfo() << "check desktop from name failed, try from pid" << pid;
        desktopEntry = UkuiNotification::Utils::desktopEntryFromPid(pid);
    }
    newHints.insert(QStringLiteral("desktop-entry"), desktopEntry);

    qDebug() << "New message received:" << app_name << id << app_icon << summary << body
             << actions << hints << timeout << "new hints" << newHints;

    if (!m_securityControlManager->isNotificationAllowed(desktopEntry)) {
        qWarning() << "Under security control! " << desktopEntry << "Notification is disabled now!";
        return 0;
    }

    //APP 第一次发送通知，注册到控制面板管控
    if (calledFromDBus()) {
        if (desktopEntry.isEmpty()) {
            qWarning() << "Failed to retrieve desktop file" << app_name << pid;
        }
        //非dbus调用发送的通知不进行注册
        m_notificationControlRegister->registerNotificationControl(desktopEntry);
    }

    if (m_notificationWatchers->watchedServices().isEmpty()) {
        NotificationPtr notification = std::make_shared<Notification>();
        notification->setAppName(app_name);
        notification->setId(id);
        notification->setAppIcon(app_icon);
        notification->setSummary(summary);
        notification->setBody(body);
        notification->setActions(actions);
        notification->setHints(newHints);
        notification->setTimeout(timeout);
        m_notificationsCache.insert(id, notification);
        return id;
    }
    for(const QString &service : m_notificationWatchers->watchedServices()) {
        QDBusMessage msg = QDBusMessage::createMethodCall(service,
                                                          QStringLiteral("/NotificationClient"),
                                                          QStringLiteral("org.ukui.NotificationClient"),
                                                          QStringLiteral("Notify"));
        msg.setArguments({app_name, id, app_icon, summary, body, actions, newHints, timeout});
        QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
    }
    return id;
}

QString ServerPrivate::GetServerInformation(QString &vendor, QString &version, QString &spec_version) const
{
    vendor = QStringLiteral("Kylin");
    version = QLatin1String(NOTIFICATION_SERVER_VERSION);
    spec_version = QStringLiteral("1.2");
    return QStringLiteral("UKUI");
}

void ServerPrivate::CloseNotification(uint id)
{
    if(m_notificationsCache.contains(id)) {
        m_notificationsCache.remove(id);
    }
    qDebug() << "CloseNotification(Revoked)" << id;
    Q_EMIT NotificationClosed(id, UkuiNotification::NotificationCloseReason::Revoked);
}

bool ServerPrivate::init()
{
    new NotificationsAdaptor(this);
    new NotificationServerAdaptor(this);
    new NotificationSettingsAdaptor(this);

    QDBusConnection conn = QDBusConnection::sessionBus();
    if(!conn.registerObject(notificationServicePath(), this)) {
        qWarning() << "Failed to register Notification DBus object!" << conn.lastError().message();
        return false;
    }
    auto registration = conn.interface()->registerService(notificationServiceName(),
                                                          QDBusConnectionInterface::ReplaceExistingService,
                                                          QDBusConnectionInterface::DontAllowReplacement);
    if (registration.value() != QDBusConnectionInterface::ServiceRegistered) {
        qWarning() << "Failed to register Notification service on DBus!";
        return false;
    }
    UkuiNotification::SettingsManager::self();
    return true;
}

QString ServerPrivate::notificationServiceName()
{
    return QStringLiteral("org.freedesktop.Notifications");
}

QString ServerPrivate::notificationServicePath()
{
    return QStringLiteral("/org/freedesktop/Notifications");
}

QString ServerPrivate::notificationServiceInterface()
{
    return QStringLiteral("org.freedesktop.Notifications");;
}

void ServerPrivate::RegisterClient()
{
    m_notificationWatchers->addWatchedService(message().service());
    qDebug() << "Watched services:" << m_notificationWatchers->watchedServices();
    sendCache();
}

void ServerPrivate::UnRegisterClient()
{
    m_notificationWatchers->removeWatchedService(message().service());
}

void ServerPrivate::CloseNotification(uint id, uint reason)
{
    if(m_notificationsCache.contains(id)) {
        m_notificationsCache.remove(id);
    }

    qDebug() << "CloseNotification" << id << reason;
    Q_EMIT NotificationClosed(id, reason);
}

void ServerPrivate::InvokeAction(uint id, const QString &action_key)
{
    NotificationManager::self().invokedAction(id, action_key);
    qDebug() << "InvokeAction" << id << action_key;
    Q_EMIT ActionInvoked(id, action_key);
}

void ServerPrivate::sendCache()
{
    for(const uint &id : m_notificationsCache.keys()) {
        auto notification = m_notificationsCache.take(id);
        for(const QString &service : m_notificationWatchers->watchedServices()) {
            QDBusMessage msg = QDBusMessage::createMethodCall(service,
                                                              QStringLiteral("/NotificationClient"),
                                                              QStringLiteral("org.ukui.NotificationClient"),
                                                              QStringLiteral("Notify"));
            msg.setArguments({notification->appName(), notification->id(), notification->appIcon(), notification->summary(),
                              notification->body(), notification->actions(), notification->hints(), notification->timeout()});
            QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
        }
    }
}

void ServerPrivate::UpdateUnreadMessagesNumber(const QString &desktopEntry, uint number)
{
    Q_EMIT UnreadMessagesNumberUpdated(desktopEntry, number);
}

void ServerPrivate::RegisterNotificationControl(const QString &desktopEntry)
{
    if (UkuiNotification::Utils::desktopEntryFromName(desktopEntry).isEmpty()) {
        qWarning() << "RegisterNotificationControl failed: " << desktopEntry << "is illegal.";
        return;
    }
    m_notificationControlRegister->registerNotificationControl(desktopEntry, NotificationControlRegister::RegisterMode::Interface);
}

bool ServerPrivate::IsAlarmSoundAllowed()
{
    return m_notificationSettingsQuery->isAlarmSoundAllowed();
}

bool ServerPrivate::IsSoundAllowed(const QString& desktopEntry)
{
    return m_notificationSettingsQuery->isSoundAllowed(desktopEntry);
}

void ServerPrivate::SetDND(bool enable)
{
    m_notificationSettingsQuery->setDND(enable);
}

bool ServerPrivate::IsDND()
{
    return m_notificationSettingsQuery->isDND();
}

Server::Server(QObject *parent): QObject(parent), d(new ServerPrivate(this))
{
}

Server &Server::self()
{
    static Server self;
    return self;
}

bool Server::init()
{
    return d->init();
}

uint Server::notify(NotificationPtr notification)
{
    return d->Notify(notification->appName(),
                     notification->replaceId(),
                     notification->appIcon(),
                     notification->summary(),
                     notification->body(),
                     notification->actions(),
                     notification->hints(),
                     notification->timeout());
}

void Server::closeNotification(uint id)
{
    d->CloseNotification(id);
}

Server::~Server() = default;
