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

#include "notification-portal.h"
#include <QLoggingCategory>
#include <application-info.h>
#include <QVariant>
#include "notifications_interface.h"
#include "portal-icon.h"
Q_LOGGING_CATEGORY(XdgDesktopPortalUKUINotification, "xdp-ukui-notification")
namespace {
    UkuiSearch::ApplicationInfo& applicationInfo() {
        static UkuiSearch::ApplicationInfo instance;
        return instance;
    }
}

NotificationActionInfo::NotificationActionInfo(const QString &appId, const QString &notificationId, const QString &action, const QVariant &target)
        : m_appId(appId),
          m_notificationId(notificationId),
          m_action(action),
          m_target(target)
{
}
QString NotificationActionInfo::appId() const
{
    return m_appId;
}

QString NotificationActionInfo::notificationId() const
{
    return m_notificationId;
}

QString NotificationActionInfo::action() const
{
    return m_action;
}
QVariant NotificationActionInfo::target() const
{
    return m_target;
}
NotificationPortal::NotificationPortal(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    m_service = new NotificationService(QStringLiteral("org.freedesktop.Notifications"), QStringLiteral("/org/freedesktop/Notifications"), QDBusConnection::sessionBus(), this);
    PortalIcon::registerDBusType();
    connect(m_service, &NotificationService::ActionInvoked, this, &NotificationPortal::actionInvoked);
    connect(m_service, &NotificationService::NotificationClosed, this, &NotificationPortal::notificationClosed);
}

void NotificationPortal::AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification)
{
    qCDebug(XdgDesktopPortalUKUINotification) << "AddNotification called with parameters:";
    qCDebug(XdgDesktopPortalUKUINotification) << "app_id: " << app_id;
    qCDebug(XdgDesktopPortalUKUINotification) << "id: " << id;
    qCDebug(XdgDesktopPortalUKUINotification) << "notification: " << notification;
    QString title = notification.value("title", QString()).toString();
    QString body = notification.value("body", QString()).toString();
    QString iconName;
    QVariantMap hintsMap;
    if (notification.contains(QStringLiteral("icon"))) {
        QVariant iconVariant = notification.value(QStringLiteral("icon"));
        if (iconVariant.type() == QVariant::String) {
            iconName = iconVariant.toString();
        } else {
            QDBusArgument argument = iconVariant.value<QDBusArgument>();
            PortalIcon icon = qdbus_cast<PortalIcon>(argument);
            QVariant iconData = icon.data.variant();
            if (icon.str == QStringLiteral("themed") && iconData.type() == QVariant::StringList) {
                iconName = iconData.toStringList().first();
            } else if (icon.str == QStringLiteral("bytes") && iconData.type() == QVariant::ByteArray) {
                hintsMap[QStringLiteral("image_data")] = iconData;
            }
        }
    }
    int urgency = -1;
    const QString priority = notification.value(QStringLiteral("priority")).toString();
    if (priority == QLatin1String("low") || priority == QLatin1String("normal")) {
        urgency = 0;
    } else if (priority == QLatin1String("high")) {
        urgency = 1;
    } else if (priority == QLatin1String("urgent")) {
        urgency = 2;
    }
    if (urgency > -1) {
        hintsMap[QStringLiteral("urgency")] = urgency;
    }
    hintsMap["desktop-entry"] = app_id;
    QStringList actions;
    QList<NotificationActionInfo> actionInfos;

    if (notification.contains(QStringLiteral("default-action")) && notification.contains(QStringLiteral("default-action-target"))) {
        actions << "default" << notification.value(QStringLiteral("default-action")).toString();
        actionInfos.append(NotificationActionInfo(app_id, id,
                                                  notification.value(QStringLiteral("default-action")).toString(),
                                                  notification.value(QStringLiteral("default-action-target")).toString()));
    }

    if (notification.contains(QStringLiteral("buttons"))) {
        const QDBusArgument dbusArgument = notification.value(QStringLiteral("buttons")).value<QDBusArgument>();
        const auto buttons = qdbus_cast<QList<QVariantMap>>(dbusArgument);
        for (const QVariantMap &button : qAsConst(buttons)) {
            actions << button.value(QStringLiteral("action")).toString() << button.value(QStringLiteral("label")).toString();
            actionInfos.append(NotificationActionInfo(app_id, id,
                                                      button.value(QStringLiteral("action")).toString(),
                                                      button.value(QStringLiteral("target")).toString()));
        }
    }
    QString appName = getAppNameById(app_id);
    QString uid = QStringLiteral("%1:%2").arg(app_id, id);
    uint newId = m_service->Notify(appName, m_notifications.value(uid, 0), iconName, title, body, actions, hintsMap, -1);
    m_notifications.insert(uid, newId);
    m_id2Actions.insert(newId, actionInfos);

}
void NotificationPortal::RemoveNotification(const QString &app_id, const QString &id)
{
    qCDebug(XdgDesktopPortalUKUINotification) << "RemoveNotification called with parameters:";
    qCDebug(XdgDesktopPortalUKUINotification) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalUKUINotification) << "    id: " << id;
    QString uid = QStringLiteral("%1:%2").arg(app_id, id);
    uint serverId = m_notifications.take(uid);
    qCDebug(XdgDesktopPortalUKUINotification) << "    serverId: " << serverId;

    m_service->CloseNotification(serverId);
    m_id2Actions.remove(serverId);
}

void NotificationPortal::actionInvoked(uint nId, const QString &action)
{
    for(const NotificationActionInfo &actionInfo : m_id2Actions.value(nId)) {
        if(actionInfo.action() == action) {
            QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/freedesktop/portal/desktop"),
                                                              QStringLiteral("org.freedesktop.impl.portal.Notification"),
                                                              QStringLiteral("ActionInvoked"));
            message << actionInfo.appId() << actionInfo.notificationId() << actionInfo.action() << actionInfo.target();
            QDBusConnection::sessionBus().send(message);
        }
    }
}
void NotificationPortal::notificationClosed(uint nId, uint reason)
{
    Q_UNUSED(reason);
    m_id2Actions.remove(nId);
    QHash<QString, uint>::const_iterator iter;
    for (iter = m_notifications.constBegin(); iter != m_notifications.constEnd(); ++iter) {
        if(iter.value() == nId) {
            break;
        }
    }
    m_notifications.erase(iter);
}
QString NotificationPortal::getAppNameById(const QString &appId)
{
    QString desktopFile;
    applicationInfo().desktopFilePathFromName(appId, desktopFile);
    return applicationInfo().getInfo(desktopFile, UkuiSearch::ApplicationProperty::LocalName).toString();
}
