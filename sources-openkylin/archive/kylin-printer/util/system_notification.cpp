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
#include "system_notification.h"
#include <QApplication>
#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>


SystemNotification::SystemNotification(QObject *parent) : QObject(parent)
{
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}

void SystemNotification::sendMessage(const QString &text)
{
    sendMessage(text, tr("Printer Info"), 5000, QStringList());
}

void SystemNotification::sendMessage(const QString &text, const QString &title, int timeout)
{
    sendMessage(text, title, 5000, QStringList());
}

void SystemNotification::sendMessage(const QString &text, const QString &title, int timeout, const QStringList &actions)
{
    QList<QVariant> args;
    // QVariantMap hintsMap;
    QMap<QString, QVariant> hintsMap;
    hintsMap.insert("x-ukui-popup-timeout", timeout);
    hintsMap.insert("desktop-entry", "kylin-printer");

    args << tr("Printer")                 // appname
         << static_cast<unsigned int>(0) // id
         << "kylin-printer"              // icon
         << title                        // summary
         << text                         // body
         << actions                      // actions
         << hintsMap                     // hints
         << -1;                          // timeout
    auto s = m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
    qDebug() << s;
}
