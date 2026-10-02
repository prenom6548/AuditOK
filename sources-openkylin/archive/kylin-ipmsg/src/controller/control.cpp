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

#include <QThread>
#include <gsettingmonitor.h>

#include "view/localinfo/localinfo.h"
#include "view/kyview.h"
#include "control.h"
#include "global/utils/inisetting.h"
#include "global/utils/globaldateformat.h"

#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"

Control::Control()
{
    init();
    establishInterrupt();
}

Control::~Control() {}

Control *Control::getInstance()
{
    static Control *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new Control();
            instance->setObjectName("kylin-ipmsg_Control_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new Control();
        instance->setObjectName("kylin-ipmsg_Control_instance");
#endif
    }

    return instance;
}

void Control::init(void)
{
    /* 实例数据库 */
    // DataBase::getInstance();

    /* 实例主界面搜索 Model , 移至线程中执行 */
    SearchMsgModel::getInstance();
    this->m_thread = new QThread();
    this->m_thread->setObjectName("kylin-ipmsg_control_QThread_mthread");
    SearchMsgModel::getInstance()->moveToThread(this->m_thread);
    this->m_thread->start();

    /* 实例设置 */
    IniSettings::getInstance();

    /* 实例数据仓库 */
    GlobalData::getInstance();

    /* 监听系统字体变化 */
    monitorFont();

    /* 时间格式转化实例设置 */
    GlobalDateFormat::getInstance();

    /* 实例后端服务 */
    this->m_server = new TcpServer;
    this->m_server->setObjectName("kylin-ipmsg_control_TcpServer_mserver");
    this->m_client = TcpClient::getInstance();
    this->m_udpSocket = new UdpSocket;
    this->m_udpSocket->setObjectName("kylin-ipmsg_control_UdpSocket_mudpSocket");

    /* 实例界面 */
    KyView::getInstance();

    return;
}

void Control::establishInterrupt(void)
{
    connect(this->m_server, &TcpServer::sigTcpServerRecvMsgSuccess, this, &Control::slotRecvMsgSuccess);
    connect(this->m_server, &TcpServer::sigTcpServerSendMsgSuccess, this, &Control::slotSendMsgSuccess);
    connect(this->m_server, &TcpServer::sigTcpServerRecvFileSuccess, this, &Control::slotRecvFileSuccess);
    connect(this->m_server, &TcpServer::sigTcpServerSendFileSuccess, this, &Control::slotSendFileSuccess);
    connect(this->m_server, &TcpServer::sigTcpServerSendFileProgress, this, &Control::slotSendFileProgress);
    connect(this->m_server, &TcpServer::sigTcpServerSendMsgFailed, this, &Control::slotSendMsgFailed);
    connect(this->m_server, &TcpServer::sigTcpServerOffline, this, &Control::slotFriendOffline);
    connect(this->m_server, &TcpServer::sigTcpServerMaintanceDb, this, &Control::slotMaintanceDb);

    connect(this->m_client, &TcpClient::sigTcpClientRecvMsgSuccess, this, &Control::slotRecvMsgSuccess);
    connect(this->m_client, &TcpClient::sigTcpClientSendMsgSuccess, this, &Control::slotSendMsgSuccess);
    connect(this->m_client, &TcpClient::sigTcpClientRecvFileSuccess, this, &Control::slotRecvFileSuccess);
    connect(this->m_client, &TcpClient::sigTcpClientSendFileSuccess, this, &Control::slotSendFileSuccess);
    connect(this->m_client, &TcpClient::sigTcpClientSendMsgFailed, this, &Control::slotSendMsgFailed);
    connect(this->m_client, &TcpClient::sigTcpClientSendFileProgress, this, &Control::slotSendFileProgress);
    connect(this->m_client, &TcpClient::sigTcpClientOffline, this, &Control::slotFriendOffline);

    /* 好友上线离线消息 */
    connect(this->m_udpSocket, &UdpSocket::addOnlineFriend, FriendListModel::getInstance(),
            &FriendListModel::addFriend);
    connect(this->m_udpSocket, &UdpSocket::updateOfflineFriend, FriendListModel::getInstance(),
            &FriendListModel::updateFriend);

    /* 聊天框信号绑定 */
    connect(FriendListView::getInstance(), &FriendListView::sigNewChatMsgWid, this, &Control::slotConnentNewChatMsg);

    /* 更改本机昵称 */
    connect(LocalInfo::getInstance(), &LocalInfo::changeLocalName, this->m_udpSocket, &UdpSocket::udpSocketBroadcast);

    /* 打开聊天窗口 */
    connect(SearchMsgList::getInstance(), &SearchMsgList::sigStartChat, FriendListView::getInstance(),
            &FriendListView::slotShowMsgWidById);
    connect(SearchMsgList::getInstance(), &SearchMsgList::sigSelectMsg, SearchMsgModel::getInstance(),
            &SearchMsgModel::slotSelect);

    return;
}

void Control::monitorFont(void)
{
    GlobalData::getInstance()->getSystemFontSize();

    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        GlobalData::getInstance()->getSystemFontSize();
        changeFont();
        emit this->sigFontChange();
    });
    return;
}

void Control::changeFont(void)
{
    QFont font14 = GlobalData::getInstance()->getFontSize14px();

    LocalInfo::getInstance()->changeFontSize();

    KyView::getInstance()->m_titleBar->m_pFuncLabel->setFont(font14);

    return;
}

// 发送离线消息
void Control::sayGoodbye()
{
    this->m_udpSocket->udpSocketBroadcast(false);
}

// 对聊天框进行信号绑定
void Control::slotConnentNewChatMsg(ChatMsg *chatMsg)
{
    connect(chatMsg, &ChatMsg::sigSendMsg, this, &Control::slotSend);
    connect(chatMsg, &ChatMsg::sigCancelMsg, this, &Control::slotCancel);
}

/* 唯一的接口 */
void Control::slotSend(ChatMsgInfo *send)
{
    QStandardItem *friendItem = FriendListModel::getInstance()->getFriendByUuid(send->m_friendUuid);
    int onlineState = friendItem->data(FriendListModel::OnlineState).toInt();

    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << send->m_msgContent << send->m_filePath << send->m_totalSize;
    }

    if (onlineState == FriendListModel::OnlineType::Online) {
        this->m_client->tran(send);
    } else {
        slotSendMsgFailed(send);
    }
}

void Control::slotCancel(QString friendUuid, int msgId)
{
    this->m_client->sendMsgCancel(friendUuid, msgId);
}

void Control::slotRecvMsgSuccess(ChatMsgInfo *recv)
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Debug : Control , slotRecvMsgSuccess , recv msg ---> " << recv->m_msgContent << recv->m_friendUuid;
    }

    recv->m_friendId = FriendListModel::getInstance()->getIdByUuid(recv->m_friendUuid);

    recv->m_isSend = ChatMsgModel::SendType::RecvMsg;
    recv->m_msgTime = QDateTime::currentDateTime().toString(GlobalUtils::getTimeFormat());

    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(recv->m_friendId);

    if (chatMsgWid->getchatMsgModel()->addChatMsg(recv)) {
        chatMsgWid->recvMsgSuccess();

        delete recv;
    }

    return;
}

// 消息发送成功 添加消息记录
void Control::slotSendMsgSuccess(ChatMsgInfo *send)
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Debug , Control , slotSendMsgSuccess , send msg ---> " << send->m_msgContent;
    }

    send->m_friendId = FriendListModel::getInstance()->getIdByUuid(send->m_friendUuid);
    send->m_isSend = ChatMsgModel::SendType::SendMsg;
    send->m_sendState = ChatMsgModel::MesssageState::SendSucceed;

    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(send->m_friendId);

    chatMsgWid->getchatMsgModel()->updateMsgState(send);
    chatMsgWid->sendMsgSuccess();

    return;
}

// 消息发送失败
void Control::slotSendMsgFailed(ChatMsgInfo *send)
{
    send->m_friendId = FriendListModel::getInstance()->getIdByUuid(send->m_friendUuid);
    send->m_sendState = ChatMsgModel::MesssageState::SendFailed;

    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(send->m_friendId);

    chatMsgWid->sendMsgFailed();
    if (chatMsgWid->getchatMsgModel()->updateMsgState(send)) {
        delete send;
    }
}

// 更新好友为离线状态
void Control::slotFriendOffline(int friendId)
{
    FriendListModel::getInstance()->updateOnlineState(friendId, false);
}

/* 文件发送进度 */
void Control::slotSendFileProgress(ChatMsgInfo *send)
{
    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(send->m_friendId);

    chatMsgWid->getchatMsgModel()->updateTransferSize(send);
    chatMsgWid->sendMsgSuccess();
}

void Control::slotSendFileSuccess(ChatMsgInfo *send)
{
    // qDebug() << send->m_friendId  << send->m_friendUuid;
    // send->m_friendId  = FriendListModel::getInstance()->getIdByUuid(send->m_friendUuid);
    send->m_isSend = ChatMsgModel::SendType::SendMsg;
    send->m_sendState = ChatMsgModel::MesssageState::SendSucceed;
    send->m_transferSize = send->m_totalSize;

    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(send->m_friendId);

    chatMsgWid->getchatMsgModel()->updateTransferSize(send);
    chatMsgWid->getchatMsgModel()->updateMsgState(send);
    chatMsgWid->sendMsgSuccess();

    return;
}

void Control::slotRecvFileSuccess(ChatMsgInfo *recv)
{
    recv->m_friendId = FriendListModel::getInstance()->getIdByUuid(recv->m_friendUuid);
    recv->m_isSend = ChatMsgModel::SendType::RecvMsg;
    recv->m_msgTime = QDateTime::currentDateTime().toString(GlobalUtils::getTimeFormat());
    recv->m_msgContent = GlobalUtils::encryptData(recv->m_filePath.split("/").back());

    if (recv->m_msgType == ChatMsgModel::MessageType::DirMsg) {
        recv->m_totalSize = GlobalUtils::getDirSize(recv->m_filePath);
        recv->m_transferSize = recv->m_totalSize;
    }

    ChatMsg *chatMsgWid = FriendListView::getInstance()->getMsgWidById(recv->m_friendId);

    if (chatMsgWid->getchatMsgModel()->addChatMsg(recv)) {
        delete recv;
    }

    return;
}

void Control::slotMaintanceDb(g_tcpItem item)
{
    if (FriendListModel::getInstance()->getIdByUuid(item.uuid) != -1) {
        return;
    } else {
        FriendInfoData *friendInfo = new FriendInfoData();
        friendInfo->setObjectName("kylin-ipmsg_control_FriendInfoData_friendInfo");

        friendInfo->m_friendUuid = item.uuid;
        friendInfo->m_friendIp = item.peerIp;
        friendInfo->m_friendPort = item.peerPort.toInt();
        friendInfo->m_username = tr("Anonymous");
        friendInfo->m_avatarUrl = GlobalUtils::getAvatarByName(friendInfo->m_username);
        friendInfo->m_onlineState = 1;

        FriendListModel::getInstance()->addFriend(friendInfo);
    }
    return;
}
