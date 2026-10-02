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

#include "tcp_server.h"
#include "global/utils/globalutils.h"

TcpServer::TcpServer()
{
    qDebug() << "Info : TcpServer , tcp server start up ...";
    startListen();

    this->setMaxPendingConnections(200);
}

TcpServer::~TcpServer() {}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    slotEstablishNewLink(socketDescriptor);
}

int TcpServer::startListen(void)
{
    /* 获取需要监听的ip和port */
    QString ip = GlobalData::getInstance()->m_tcpListenIP;
    QString port = GlobalData::getInstance()->m_tcpListenPort;

    if (ip.isEmpty() || port.isEmpty()) {
        qDebug() << "Error : TcpServer , startListen , ip or port is empty";
        return -1;
    }

    if (this->listen(QHostAddress(ip), port.toUShort()) == false) {
        qDebug() << "Error : TcpServer , startListen , listen fail";
        return -1;
    }

    qDebug() << "Info : TcpServer , startListen , listen success , ip --- > " << ip << "port ---> " << port;

    return 0;
}

void TcpServer::slotEstablishNewLink(qintptr socketDescriptor)
{
    qDebug() << "Info : TcpServer , slotEstablishNewLink , new link coming in ...";

    /* 加载功能模块 */
    TcpModule *tcpModule = new TcpModule(socketDescriptor);
    tcpModule->setObjectName("kylin-ipmsg_tcpserver_TcpModule_tcpModule");

    connect(tcpModule, &TcpModule::sigMaintainTcpLinkAdd, this, &TcpServer::slotMaintainTcpLinkAdd);
    connect(tcpModule, &TcpModule::sigMaintainTcpLinkDelete, this, &TcpServer::slotMaintainTcpLinkDelete);
    connect(tcpModule, &TcpModule::sigRecvMsgSuccess, this, &TcpServer::slotRecvMsgSuccess);
    connect(tcpModule, &TcpModule::sigSendMsgSuccess, this, &TcpServer::slotSendMsgSuccess);
    connect(tcpModule, &TcpModule::sigSendFileSuccess, this, &TcpServer::slotSendFileSuccess);
    connect(tcpModule, &TcpModule::sigRecvFileSuccess, this, &TcpServer::slotRecvFileSuccess);
    connect(tcpModule, &TcpModule::sigSendFileProgress, this, &TcpServer::slotSendFileProgress);
    connect(tcpModule, &TcpModule::sigSendFileFail, this, &TcpServer::sigTcpServerSendMsgFailed);
    connect(tcpModule, &TcpModule::sigFriendOffline, this, &TcpServer::sigTcpServerOffline);

    /* 移至线程 */
    QThread *thread = new QThread;
    thread->setObjectName("kylin-ipmsg_tcpserver_QThread_thread");
    tcpModule->moveToThread(thread);

    thread->start();

    return;
}

void TcpServer::slotMaintainTcpLinkAdd(g_tcpItem item)
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : TcpServer , slotMaintainTcpLinkAdd , uuid ---> " << item.uuid;
    }

    if (GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(item)) {
        qDebug() << "Waring , TcpServer , slotMaintainTcpLinkadd , maintain tcp link fail";
    }

    /* 维护数据库 */
    emit sigTcpServerMaintanceDb(item);
}

void TcpServer::slotMaintainTcpLinkDelete(QString uuid)
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : TcpServer , slotMaintainTcpLinkDelete , uuid ---> " << uuid;
    }

    if (GlobalData::getInstance()->m_tcpLink.tcpMaintainDelete(uuid)) {
        qDebug() << "Waring , TcpServer , slotMaintainTcpLinkDelete , maintain tcp link fail";
    }
}

void TcpServer::slotRecvMsgSuccess(ChatMsgInfo *recv)
{
    emit sigTcpServerRecvMsgSuccess(recv);
}

void TcpServer::slotSendMsgSuccess(ChatMsgInfo *send)
{
    emit sigTcpServerSendMsgSuccess(send);
}

void TcpServer::slotRecvFileSuccess(ChatMsgInfo *recv)
{
    emit sigTcpServerRecvFileSuccess(recv);
}

void TcpServer::slotSendFileSuccess(ChatMsgInfo *send)
{
    emit sigTcpServerSendFileSuccess(send);
}

void TcpServer::slotSendFileProgress(ChatMsgInfo *send)
{
    emit sigTcpServerSendFileProgress(send);
}
