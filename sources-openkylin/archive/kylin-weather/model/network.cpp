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
#include "network.h"
#include <QThread>

Network::Network(QObject *parent) : QObject(parent)
{
    initConnect();
    //初始化dbus
    initConnectionInfo();
}
//检查网络
void Network::checkNetWork()
{
    qDebug() << " ##### Function Positioning ##### 检查网络 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;

    QNetworkConfigurationManager mgr;
    if (mgr.isOnline()) {
        qDebug() << " ##### Function Positioning ##### QNetworkConfigurationManager ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        //判断网络是否有连接，不一定能上网，如果连接了，则开始检查互联网是否可以ping通
        QHostInfo::lookupHost("www.gov.cn", this, SLOT(checkNetAvailable(QHostInfo)));
    } else {
        ErrorReact::getInstance()->networkFeedBack(false, "无网络");
        emit effectiveNetwork(false);
    }
}
//检查网络可用
void Network::checkNetAvailable(const QHostInfo &host)
{
    qDebug() << " ##### Function Positioning ##### 检查网络可用 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //判断网络可用---返回错误原因
    if (host.error() != QHostInfo::NoError) {
        ErrorReact::getInstance()->networkFeedBack(false, host.errorString());
        emit effectiveNetwork(false);
    } else {
        emit effectiveNetwork(true);
    }
}
void Network::initConnectionInfo()
{
    // service名,path,interface；systemBus() or sessionBus()
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    // method：get（接口名，属性名）  ----返回值为：Variant-----返回当前的所有连接的网络
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    // outArgs放返回的数据
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    //循环存
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;
        m_oldPaths.append(objPath);
        //目的可能是检测更换网络：更换网络---刷新
        QDBusInterface interface("org.freedesktop.NetworkManager", objPath.path(), "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
        //获得网络类型--type是有线还是无线：802-3-ethernet，802-11-wireless
        QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
        m_oldPathInfo.append(reply.value().toString());
    }
    dbusArgs.endArray();
}

// 删除 onPropertiesChanged 槽函数，新增 onStateChanged 槽函数
void Network::onStateChanged(uint state)
{
    qDebug() << "----------state" << state;
    // 70: NM_STATE_CONNECTED_GLOBAL, 50: NM_STATE_CONNECTED_LOCAL, 20: NM_STATE_DISCONNECTED
    if (state == 70) {
        const bool wasOnline = CoreVar::HAS_NETWORK;
        CoreVar::HAS_NETWORK = true;
        qDebug() << "NetworkManager connected global, wasOnline:" << wasOnline;
        static const int refreshDelays[] = {1000, 3000, 8000, 15000, 31000};
        for (int i = 0; i < int(sizeof(refreshDelays) / sizeof(refreshDelays[0])); ++i) {
            const int delay = refreshDelays[i];
            QTimer::singleShot(delay, this, [this, i, delay]() {
                if (!CoreVar::HAS_NETWORK) {
                    qDebug() << "Skip reconnect refresh attempt" << (i + 1) << "network offline";
                    return;
                }
                qDebug() << "Reconnect refresh attempt" << (i + 1) << "delay(ms):" << delay;
                emit haveNet();
            });
        }
    } else if (state == 20) {
        CoreVar::HAS_NETWORK = false;
        qDebug() << "NetworkManager disconnected";
        ErrorReact::getInstance()->networkFeedBack(false, "无网络");
    }
}

void Network::initConnect()
{

    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForShutdown"), this,
                                         SLOT(onPrepareForSleep(bool)));
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));

    // dbus-信号槽
    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"), QString("StateChanged"), this,
                                         SLOT(onStateChanged(uint)));
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"), QString("unlock"), this,
                                         SLOT(onScreenSaverUnlock()));
}

void Network::onTimeFinished()
{
    //发送信号--刷新界面
    emit haveNet();
}

void Network::onScreenSaverUnlock()
{
    emit haveNet();
}

void Network::onPrepareForSleep(bool isSleep)
{
    qDebug() << "------------------into weather prepare for sleep" << isSleep;
    if(isSleep){
        if(!QDBusConnection::systemBus().isConnected()){
            qDebug() << "------------------system dbus is not connected";
            return;
        }
       bool disconnect_flag =  QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"),
                                                                    QString("/org/freedesktop/NetworkManager"),
                                                                    QString("org.freedesktop.NetworkManager"), QString("StateChanged"), this,
                                                                    SLOT(onStateChanged(uint)));
       qDebug() << "disconnect systemdbus result:" << disconnect_flag;
    }else{
        qDebug() << "sleep for 5s ";
        QThread::sleep(5);
        if(!QDBusConnection::systemBus().isConnected()){
            qDebug() << "------------------system dbus is not connected";
            return;
        }
       bool connect_flag =  QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                                                 QString("/org/freedesktop/NetworkManager"),
                                                                 QString("org.freedesktop.NetworkManager"), QString("StateChanged"), this,
                                                                 SLOT(onStateChanged(uint)));
       qDebug() << "after sleep connect systemdbus result:" << connect_flag;
    }
}
