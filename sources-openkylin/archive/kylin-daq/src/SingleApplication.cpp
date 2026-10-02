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
/*** 
 * @Author: pengcheng
 * @Date: 2021-07-05 10:42:10
 * @LastEditTime: 2021-08-24 14:41:20
 * @LastEditors: pengcheng
 * @Description: 
 * @FilePath: /kylin-daq/src/SingleApplication.cpp
 */
#include "SingleApplication.h"

SingleApplication::SingleApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
//    , w(NULL)
    , m_isRunning(false)
    , m_localServer(NULL) {

    // 取应用程序名作为LocalServer的名字
    //Take the application name as the LocalServer name.
    m_serverName = QFileInfo(QCoreApplication::applicationFilePath()).fileName() + QLatin1String(getenv("DISPLAY"));
    qDebug() << m_serverName;
    initLocalConnection();
}

//检查是否有实例在运行，true为正在运行，false为没有运行
bool SingleApplication::isRunningFunction() {
    return m_isRunning;
}

//通过socket通讯实现程序单实例运行，监听到新的连接时触发该函数
void SingleApplication::newLocalConnection() {
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if(socket) {
        socket->waitForReadyRead(2*TIME_OUT);
        delete socket;
        activateWindow();
    }
}

//通过socket通讯实现程序单实例运行，初始化本地连接，如果连接不上server，则创建，否则退出
void SingleApplication::initLocalConnection() {
    m_isRunning = false;
    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    if(socket.waitForConnected(TIME_OUT)) {
        qWarning() << "SingleApplication:"<<QString("%1 already running.\n").arg(m_serverName.toUtf8().constData());
        m_isRunning = true;
        return;
    }
    newLocalServer();
}

void SingleApplication::newLocalServer() {
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalConnection()));
    if(!m_localServer->listen(m_serverName)) {
        // 此时监听失败，可能是程序崩溃时,残留进程服务导致的,移除之
        /*
         * This is a listening failure,
         * probably caused by the residual process service when the program crashes,
         * remove it.
        */
        if(m_localServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(m_serverName); // <-- 重点
            m_localServer->listen(m_serverName); // 再次监听
        }
    }
}

void SingleApplication::activateWindow() {
}

