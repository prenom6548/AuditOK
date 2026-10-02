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

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QObject>
#include <QThread>

#include "global/utils/global_data.h"
#include "network/tcp_server.h"
#include "network/tcp_client.h"
#include "network/udp_socket.h"
#include "view/friendlist/friendlist.h"

class Control : public QObject
{
    Q_OBJECT
    friend class TestControl;

public:
    Control();
    ~Control();

    // 单例，初始化返回指针
    static Control *getInstance();

    void init(void);
    void establishInterrupt(void);
    void monitorFont(void);
    void changeFont(void);

    // 发送离线消息
    void sayGoodbye();

private:
    TcpServer *m_server;
    TcpClient *m_client;
    UdpSocket *m_udpSocket;
    QThread *m_thread;

signals:
    void sigFontChange();

private slots:
    // 对聊天框进行信号绑定
    void slotConnentNewChatMsg(ChatMsg *chatMsg);

    void slotSend(ChatMsgInfo *send);
    void slotRecvMsgSuccess(ChatMsgInfo *recv);
    void slotSendMsgSuccess(ChatMsgInfo *send);
    void slotRecvFileSuccess(ChatMsgInfo *recv);
    void slotSendFileSuccess(ChatMsgInfo *send);
    void slotSendMsgFailed(ChatMsgInfo *send);
    void slotSendFileProgress(ChatMsgInfo *send);

    void slotCancel(QString friendUuid, int msgId);
    void slotFriendOffline(int friendId);
    void slotMaintanceDb(g_tcpItem item);
};

#endif
