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
#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QNetworkConfigurationManager>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QHostInfo>
#include <QDebug>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QTimer>
#include "controller/core/errorreact.h"
#include "global/corevar.h"
class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);
    void checkNetWork();       //检查网络
    void initConnectionInfo(); //初始化网络dbus

private:
    QStringList m_oldPathInfo;
    QList<QDBusObjectPath> m_oldPaths;

    void initConnect(); //初始化信号槽
private slots:
    void checkNetAvailable(const QHostInfo &host); //检查网络可用
    void onStateChanged(uint state);     //监听断网联网
    void onTimeFinished();                         //延时发送重新定位请求
    void onScreenSaverUnlock();                    //监听屏幕解锁 
    void onPrepareForSleep(bool isSleep);
signals:
    void effectiveNetwork(bool isOnline); //网络连接正常
    void haveNet();                       //网络从无到有，开始刷新
};


#endif // NETWORK_H
