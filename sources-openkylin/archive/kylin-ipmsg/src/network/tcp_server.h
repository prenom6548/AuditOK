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

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>

#include "global/declare/declare.h"
#include "global/utils/global_data.h"
#include "tcp_module.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
    friend class TestTcpServer_IncomingConnectionTest_Test;
    friend class TestTcpServer_StartListenErrorTest_Test;
    friend class TestTcpServer_SlotFunctionsTest_Test;

public:
    TcpServer();
    ~TcpServer();

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    int startListen(void);

    /* 对外模块信号 */
signals:
    void sigTcpServerRecvMsgSuccess(ChatMsgInfo *recv);
    void sigTcpServerSendMsgSuccess(ChatMsgInfo *send);
    void sigTcpServerRecvFileSuccess(ChatMsgInfo *recv);
    void sigTcpServerSendFileSuccess(ChatMsgInfo *send);
    void sigTcpServerSendFileProgress(ChatMsgInfo *send);
    void sigTcpServerSendMsgFailed(ChatMsgInfo *send);
    void sigTcpServerOffline(int friendId);
    void sigTcpServerMaintanceDb(g_tcpItem item);

private slots:
    void slotEstablishNewLink(qintptr socketDescriptor);
    void slotRecvMsgSuccess(ChatMsgInfo *recv);
    void slotSendMsgSuccess(ChatMsgInfo *send);
    void slotRecvFileSuccess(ChatMsgInfo *recv);
    void slotSendFileSuccess(ChatMsgInfo *send);
    void slotSendFileProgress(ChatMsgInfo *send);
    void slotMaintainTcpLinkAdd(g_tcpItem item);
    void slotMaintainTcpLinkDelete(QString uuid);
};

#endif
