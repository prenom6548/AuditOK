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

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QTimer>
#include <QMap>

#include "tcp_module.h"
#include "global/utils/global_data.h"
#include "global/declare/declare.h"

class TcpClient : public QObject
{
    Q_OBJECT
    friend class TestTcpClient;

public:
    TcpClient();
    ~TcpClient();

    // 单例，初始化返回指针
    static TcpClient *getInstance();

    int tran(ChatMsgInfo *send);

    // 主动取消发送
    void sendMsgCancel(QString friendUuid, int msgId);

private:
    ChatMsgInfo *m_send;

    int m_timeoutCount;

signals:
    void sigTcpClientSendMsgFailed(ChatMsgInfo *send);
    void sigTcpClientSendMsgSuccess(ChatMsgInfo *send);
    void sigTcpClientRecvMsgSuccess(ChatMsgInfo *recv);
    void sigTcpClientSendFileSuccess(ChatMsgInfo *send);
    void sigTcpClientRecvFileSuccess(ChatMsgInfo *recv);
    void sigTcpClientSendFileProgress(ChatMsgInfo *send);

    void sigTcpClientSendMsg(ChatMsgInfo *send);
    void sigTcpClientSendFile(ChatMsgInfo *send);

    void sigTcpClientSendMsg2(ChatMsgInfo send);

    void sigTcpClientOffline(int friendId);

private slots:
    void slotSendMsgSuccess(ChatMsgInfo *send);
    void slotRecvMsgSuccess(ChatMsgInfo *recv);
    void slotSendFileSuccess(ChatMsgInfo *send);
    void slotRecvFileSuccess(ChatMsgInfo *recv);
    void slotSendFileProgress(ChatMsgInfo *send);
    void slotMaintainTcpLinkDelete(QString uuid);
};

#endif
