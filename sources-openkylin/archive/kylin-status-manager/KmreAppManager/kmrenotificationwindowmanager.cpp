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

#include "kmrenotificationwindowmanager.h"
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

KmreNotificationWindowManager::KmreNotificationWindowManager(QObject *parent) : QObject(parent)
{
}

void KmreNotificationWindowManager::show(QString notificationText)
{
    setNotificationWindowContent(notificationText);
    connectNotificationWindowOperationSignal();
}

void KmreNotificationWindowManager::connectNotificationWindowOperationSignal()
{
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          "ActionInvoked", this,
                                          SLOT(notificationWindowOperationHandle(uint, const QString &)));
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          "NotificationClosed", this,
                                          SLOT(notificationWindowClosed(uint, uint)));
}

void KmreNotificationWindowManager::disconnectNotificationWindowSignal()
{
    QDBusConnection::sessionBus().disconnect("org.freedesktop.Notifications",
                                             "/org/freedesktop/Notifications",
                                             "org.freedesktop.Notifications",
                                             "ActionInvoked", this,
                                             SLOT(notificationWindowOperationHandle(uint, const QString &)));
    QDBusConnection::sessionBus().disconnect("org.freedesktop.Notifications",
                                             "/org/freedesktop/Notifications",
                                             "org.freedesktop.Notifications",
                                             "NotificationClosed", this,
                                             SLOT(notificationWindowClosed(uint, uint)));
}

void KmreNotificationWindowManager::setNotificationWindowContent(const QString &notificationContent)
{
    const QString str1 = "Switch to " + notificationContent + " mode?";
    const QString str2 = "Switching to " + notificationContent+ " will close all open mobile applications";

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    QStringList actions;
    actions.append("ok");
    actions.append(tr("ok"));//按钮动作1
    actions.append("cancel");
    actions.append(tr("cancel"));//按钮动作2
    QMap<QString, QVariant> hints;
    hints.insert("display-location", 1);//居中
    args << tr("test")
         << ((unsigned int) 0)
         << QString("dialog-warning")
         << tr(str1.toStdString().c_str())
         << tr(str2.toStdString().c_str())
         << actions
         << hints
         << (int)0; //0 常驻
    QDBusPendingCall async = iface.asyncCallWithArgumentList("Notify", args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(saveRepliedNotificationWindowId(QDBusPendingCallWatcher*)));

}

void KmreNotificationWindowManager::saveRepliedNotificationWindowId(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<uint> reply = *watcher;
    if (reply.isError()) {
        Q_EMIT canSwitchMode(false);
        return;
    } else {
        m_notificationWindwoId = reply.argumentAt<0>();
        qInfo() << __FILE__ << __LINE__ << "【调用通知接口】成功！返回值ID=>：" << m_notificationWindwoId;
    }
    watcher->deleteLater();
}

void KmreNotificationWindowManager::notificationWindowOperationHandle(uint id, const QString &value)
{
    if (id != m_notificationWindwoId) {
        return;
    }
    if (value == "ok") {
        Q_EMIT canSwitchMode(true);
    } else if (value == "cancel") {
        Q_EMIT canSwitchMode(false);
    }
    disconnectNotificationWindowSignal();
}

void KmreNotificationWindowManager::notificationWindowClosed(uint id, uint type)
{
    Q_UNUSED(type);
    if (id == m_notificationWindwoId) {
        Q_EMIT canSwitchMode(false);
    }
    disconnectNotificationWindowSignal();
}
