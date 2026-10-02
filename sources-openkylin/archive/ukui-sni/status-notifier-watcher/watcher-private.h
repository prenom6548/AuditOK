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

#ifndef UKUI_SNI_WATCHER_PRIVATE_H
#define UKUI_SNI_WATCHER_PRIVATE_H

#include <QObject>
#include <QDBusContext>
#include <QStringList>
#include <QDBusServiceWatcher>
#include <QSet>
#include "typedefs.h"

class WatcherPrivate : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ RegisteredStatusNotifierItems)
    Q_PROPERTY(UintList RegisteredStatusNotifierItemPids READ RegisteredStatusNotifierItemPids)
    Q_PROPERTY(bool IsStatusNotifierHostRegistered READ IsStatusNotifierHostRegistered)
    Q_PROPERTY(int ProtocolVersion READ ProtocolVersion)
public:
    explicit WatcherPrivate(QObject *parent = nullptr);
    ~WatcherPrivate() override;

    QStringList RegisteredStatusNotifierItems() const;
    UintList RegisteredStatusNotifierItemPids() const;
    bool IsStatusNotifierHostRegistered() const;
    int ProtocolVersion() const;

public Q_SLOTS:
    void RegisterStatusNotifierItem(const QString &service);
    void RegisterStatusNotifierHost(const QString &service);

Q_SIGNALS:
    void StatusNotifierItemRegistered(const QString &service);
    void StatusNotifierHostRegistered();
    void StatusNotifierItemUnregistered(const QString &service);
    void StatusNotifierHostUnregistered();
    void StatusNotifierItemRegistered(quint32 pid);
    void StatusNotifierItemUnregistered(quint32 pid);

private:
    void serviceUnregistered(const QString &service);

    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    QMultiMap<QString, QString> m_registeredServices; // notifierItemId->pid
    QSet<QString> m_statusNotifierHostServices;
};

#endif //UKUI_SNI_WATCHER_PRIVATE_H
