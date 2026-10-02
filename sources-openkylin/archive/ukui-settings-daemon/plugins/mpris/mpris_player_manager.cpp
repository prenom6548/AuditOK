/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "mpris_player_manager.h"
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

class MprisPlayerManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MprisPlayerManagerPrivate(QObject* parent = nullptr);
    ~MprisPlayerManagerPrivate(){qDebug() << "MprisPlayerManagerPrivate" << "析构";}
private Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
private:
    void addPlayer(const QString& service);
    void removePlayer(const QString& service);
public:
    QHash<QString, MprisPlayerPtr> m_players;
    //service to pid
    QHash<QString, uint> m_services;
};

MprisPlayerManager::MprisPlayerManager(QObject *parent)
    : QObject(parent)
    , d(new MprisPlayerManagerPrivate(this))
{

}

MprisPlayerManager::~MprisPlayerManager()
{
    qDebug() << "MprisPlayerManager" << "析构";
}

MprisPlayerWeakPtr MprisPlayerManager::player(const QString &service) const
{
    return d->m_players.value(service);
}

MprisPlayerWeakPtr MprisPlayerManager::player(const uint &pid) const
{
    return d->m_players.value(d->m_services.key(pid));
}

MprisPlayerWeakPtr MprisPlayerManager::activePlayer()
{
    if (d->m_players.count() == 0) {
        //如果没有player，构建空对象
        return std::make_shared<MprisPlayer>("nullplayer");
    }
    return d->m_players.begin().value();
}

//private
MprisPlayerManagerPrivate::MprisPlayerManagerPrivate(QObject *parent)
    : QObject(parent)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    auto watcher = new QDBusServiceWatcher(QStringLiteral("org.mpris.MediaPlayer2*"), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MprisPlayerManagerPrivate::serviceOwnerChanged);
#else
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &MprisPlayerManagerPrivate::serviceOwnerChanged);
#endif
    //获取列表
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QStringLiteral("ListNames"));
    QDBusPendingCallWatcher *asyncWatcher = new QDBusPendingCallWatcher(async, this);
    connect(asyncWatcher, &QDBusPendingCallWatcher::finished, [=](QDBusPendingCallWatcher* watcher){
        QDBusPendingReply<QStringList> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            //error
            qWarning()  << "Error message was" << reply.error().name() << reply.error().message();

        } else {
            for (const QString& serviceName : reply.value()) {
                if (serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
                    addPlayer(serviceName);
                    qDebug() << serviceName;
                }
            }
        }
    });
}

void MprisPlayerManagerPrivate::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (!name.startsWith("org.mpris.MediaPlayer2.")) {
        //过滤不符合mpris的seivice
        return;
    }
    if (!oldOwner.isEmpty()) {
        //dbus 注销 remove player
        removePlayer(name);
    }
    if (!newOwner.isEmpty()) {
        //dbus 注册 add player
        addPlayer(name);
        qDebug() << "add service" << name;
    }
}

void MprisPlayerManagerPrivate::addPlayer(const QString &service)
{
    MprisPlayerPtr player = std::make_shared<MprisPlayer>(service);
    if (player.get()) {
        connect(player.get(), &MprisPlayer::initialFetchFinished, this, [=] () {
            disconnect(player.get(), &MprisPlayer::initialFetchFinished, this, nullptr);
            //add service pid
            m_services.insert(player->objectName(), player->pid());
            //add service player
            m_players.insert(player->objectName(), player);
        });
    }
}

void MprisPlayerManagerPrivate::removePlayer(const QString &service)
{
    m_players.take(service);
    m_services.take(service);
    qDebug() << "remove service" << service;
}

#include "mpris_player_manager.moc"
