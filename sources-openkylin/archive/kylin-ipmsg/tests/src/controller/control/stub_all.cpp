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

#include <QObject>
#include <QString>
#include "controller/control.h"
#include "network/udp_socket.h"
#include "network/tcp_server.h"
#include "network/tcp_client.h"
#include "view/chatmsg/chatmsg.h"
#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "view/localinfo/localinfo.h"
#include "view/friendlist/homepagesearch/searchmsglist.h"
#include "view/friendlist/homepagesearch/search_msg_model.h"
#include "global/utils/globaldateformat.h"
#include "model/friendlistmodel.h"
#include "view/kyview.h"
#include "global/utils/inisetting.h"

// ---- test counters / toggles (extern in tests) ----
int g_test_tcpclient_tran_calls = 0;
int g_test_tcpclient_send_cancel_calls = 0;

int g_test_chatmsg_send_success_calls = 0;
int g_test_chatmsg_send_failed_calls = 0;
int g_test_chatmsg_recv_success_calls = 0;

int g_test_chatmsgmodel_add_calls = 0;
int g_test_chatmsgmodel_update_state_calls = 0;
int g_test_chatmsgmodel_update_transfer_calls = 0;

int g_test_friendlist_add_friend_calls = 0;
// If >= 0, FriendListModel::getIdByUuid returns this value (for any uuid).
int g_test_friendlist_get_id_by_uuid_override = -1;

// === UdpSocket ===
UdpSocket::UdpSocket() {}
UdpSocket::~UdpSocket() {}
void UdpSocket::udpSocketBroadcast(bool) {}
void UdpSocket::slotSocketRead() {}
void UdpSocket::slotCheckOnline() {}
void UdpSocket::slotOffline() {}
void UdpSocket::slotOnline() {}

// === TcpServer ===
TcpServer::TcpServer() : QTcpServer(nullptr) {}
TcpServer::~TcpServer() {}
void TcpServer::incomingConnection(qintptr) {}
void TcpServer::slotEstablishNewLink(qintptr) {}
void TcpServer::slotRecvMsgSuccess(ChatMsgInfo*) {}
void TcpServer::slotSendMsgSuccess(ChatMsgInfo*) {}
void TcpServer::slotRecvFileSuccess(ChatMsgInfo*) {}
void TcpServer::slotSendFileSuccess(ChatMsgInfo*) {}
void TcpServer::slotSendFileProgress(ChatMsgInfo*) {}
void TcpServer::slotMaintainTcpLinkAdd(g_tcpItem) {}
void TcpServer::slotMaintainTcpLinkDelete(QString) {}

// === TcpClient ===
TcpClient::TcpClient() {}
TcpClient::~TcpClient() {}
TcpClient* TcpClient::getInstance() {
    static TcpClient instance;
    return &instance;
}
int TcpClient::tran(ChatMsgInfo*) { ++g_test_tcpclient_tran_calls; return 0; }
void TcpClient::sendMsgCancel(QString, int) { ++g_test_tcpclient_send_cancel_calls; }
void TcpClient::slotMaintainTcpLinkDelete(QString) {}
void TcpClient::slotSendMsgSuccess(ChatMsgInfo*) {}
void TcpClient::slotRecvMsgSuccess(ChatMsgInfo*) {}
void TcpClient::slotSendFileSuccess(ChatMsgInfo*) {}
void TcpClient::slotRecvFileSuccess(ChatMsgInfo*) {}
void TcpClient::slotSendFileProgress(ChatMsgInfo*) {}

// === TcpModule ===
TcpModule::TcpModule(qintptr) : QObject(nullptr) {}
TcpModule::TcpModule(QString, QString, QString) : QObject(nullptr) {}
TcpModule::~TcpModule() {}
void TcpModule::slotSocketError(QAbstractSocket::SocketError) {}
void TcpModule::startSocket() {}
void TcpModule::slotTaskProcess() {}
void TcpModule::slotSocketConnect() {}
void TcpModule::slotReadSocket() {}
void TcpModule::slotSocketDisconnect() {}
void TcpModule::slotConnTimeout() {}

// === ChatMsg ===
ChatMsg::ChatMsg(int friendId, QString friendUuid, QWidget *parent) : QWidget(parent) {
    m_chatMsgModel = new ChatMsgModel(friendId, friendUuid);
}
ChatMsg::~ChatMsg() {}
ChatMsgModel* ChatMsg::getchatMsgModel() { return m_chatMsgModel; }
void ChatMsg::sendMsgSuccess() { ++g_test_chatmsg_send_success_calls; }
void ChatMsg::sendMsgFailed() { ++g_test_chatmsg_send_failed_calls; }
void ChatMsg::recvMsgSuccess() { ++g_test_chatmsg_recv_success_calls; }
void ChatMsg::slotUpdateTimeFormat(QString) {}
bool ChatMsg::eventFilter(QObject*, QEvent*) { return false; }
void ChatMsg::keyPressEvent(QKeyEvent*) {}
void ChatMsg::paintEvent(QPaintEvent*) {}
void ChatMsg::closeEvent(QCloseEvent*) {}
void ChatMsg::dragEnterEvent(QDragEnterEvent*) {}
void ChatMsg::dropEvent(QDropEvent*) {}

// === FriendListView ===
// Moved to stub_view.cpp

// === TrayIconWid ===
TrayIconWid* TrayIconWid::getInstance() {
    static TrayIconWid *instance = nullptr;
    if (!instance) instance = new TrayIconWid();
    return instance;
}
TrayIconWid::TrayIconWid(QWidget *parent) : QWidget(parent) {}
TrayIconWid::~TrayIconWid() {}
bool TrayIconWid::eventFilter(QObject*, QEvent*) { return false; }
void TrayIconWid::keyPressEvent(QKeyEvent*) {}
void TrayIconWid::paintEvent(QPaintEvent*) {}
void TrayIconWid::closeEvent(QCloseEvent*) {}

// === LocalInfo ===
// Moved to stub_view.cpp

// === SearchMsgList ===
// Moved to stub_view.cpp

// === SearchMsgModel ===
// Moved to stub_view.cpp

// === ChatMsgModel ===
ChatMsgModel::ChatMsgModel(int, QString) : QStandardItemModel(nullptr) {}
ChatMsgModel::~ChatMsgModel() {}
bool ChatMsgModel::addChatMsg(ChatMsgInfo*) { ++g_test_chatmsgmodel_add_calls; return true; }
bool ChatMsgModel::updateMsgState(ChatMsgInfo*) { ++g_test_chatmsgmodel_update_state_calls; return true; }
void ChatMsgModel::updateTransferSize(ChatMsgInfo*) { ++g_test_chatmsgmodel_update_transfer_calls; }

// === TcpLink ===
TcpLink::TcpLink() {}
TcpLink::~TcpLink() {}
int TcpLink::init() { return 0; }
void TcpLink::ergodic(void) {}
int TcpLink::tcpMaintainAdd(g_tcpItem) { return 0; }
int TcpLink::tcpMaintainDelete(QString) { return 0; }
int TcpLink::udpMaintainAdd(g_udpItem) { return 0; }
int TcpLink::udpMaintainDelete(QString) { return 0; }
g_tcpMaintain TcpLink::select(QString) { return g_tcpMaintain(); }

// === ProtocolAnalysis ===
ProtocolAnalysis::ProtocolAnalysis() {}
ProtocolAnalysis::~ProtocolAnalysis() {}
QMap<QByteArray, QByteArray> ProtocolAnalysis::analysisHeadBody(QByteArray) { return QMap<QByteArray, QByteArray>(); }
QByteArray ProtocolAnalysis::encapsulationHeadBody(QMap<QByteArray, QByteArray>) { return QByteArray(); }

// === AddrSet ===
AddrSet::AddrSet(QWidget *parent) : QWidget(parent) {}
AddrSet::~AddrSet() {}

// === GlobalDateFormat ===
// Moved to stub_view.cpp

// === FriendListModel ===
FriendListModel::FriendListModel() : QStandardItemModel(nullptr) {}
FriendListModel::~FriendListModel() {}
FriendListModel* FriendListModel::getInstance() {
    static FriendListModel instance;
    return &instance;
}
void FriendListModel::updateOnlineState(int, bool) {}
int FriendListModel::getIdByUuid(QString) {
    if (g_test_friendlist_get_id_by_uuid_override >= 0) {
        return g_test_friendlist_get_id_by_uuid_override;
    }
    return -1;
}
QStandardItem* FriendListModel::getFriendByUuid(QString) {
    static QStandardItem *item = new QStandardItem();
    return item;
}
void FriendListModel::addFriend(FriendInfoData*) { ++g_test_friendlist_add_friend_calls; }
void FriendListModel::updateFriend(FriendInfoData*) {}

// === IniSettings ===
IniSettings::IniSettings(const QString&) {}
IniSettings::~IniSettings() {}
IniSettings* IniSettings::getInstance() {
    static IniSettings *instance = nullptr;
    if (!instance) {
        instance = new IniSettings("");
    }
    return instance;
}
void IniSettings::initSettings() {}
bool IniSettings::isExistNickname() { return true; }
QString IniSettings::getLocalNickname() { return ""; }
void IniSettings::setLocalNickname(QString) {}
int IniSettings::getLocalTcpPort() { return 0; }
void IniSettings::setLocalTcpPort(int) {}
QString IniSettings::getLocalUuid() { return ""; }
void IniSettings::setLocalUuid(QString) {}
QString IniSettings::getFilePath() { return ""; }
void IniSettings::setFilePath(QString) {}
