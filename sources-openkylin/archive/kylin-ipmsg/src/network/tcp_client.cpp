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

#include "tcp_client.h"
#include "global/utils/inisetting.h"
#include "global/utils/globalutils.h"
#include "global/utils/addrset.h"

TcpClient::TcpClient() {}

TcpClient::~TcpClient() {}

/* 单例，初始化返回指针 */
TcpClient *TcpClient::getInstance()
{
    static TcpClient *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new TcpClient();
            instance->setObjectName("kylin-ipmsg_TcpClient_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new TcpClient();
        instance->setObjectName("kylin-ipmsg_TcpClient_instance");
#endif
    }

    return instance;
}

/* 网络模块暴露出来的唯一接口 */
int TcpClient::tran(ChatMsgInfo *send)
{
    /* 检测链接是否建立 */
    g_tcpMaintain item = GlobalData::getInstance()->m_tcpLink.select(send->m_friendUuid);

    this->m_send = send;

    /* 链接已经建立 , 直接添加任务队列 */
    if (item.module != NULL) {
        if (send->m_msgType == ChatMsgModel::MessageType::TextMsg) {
            item.module->m_msgTaskQueue.enqueue(send);
            emit item.module->sigTaskAdd();
        } else if (send->m_msgType == ChatMsgModel::MessageType::FileMsg
                   || send->m_msgType == ChatMsgModel::MessageType::DirMsg) {
            item.module->m_fileTaskQueue.enqueue(send);
            emit item.module->sigTaskAdd();
        }

        return 0;
    }

    /* 链接未建立 */
    QString ip = item.peerListenIp;
    QString port = item.peerListenPort;

    qDebug() << "Info : TcpClient , tran , connect peer ip ---> " << ip << "port ---> " << port;

    /* 加载功能模块 */
    TcpModule *module = new TcpModule(ip, port, send->m_friendUuid);
    module->setObjectName("kylin-ipmsg_tcpclient_TcpModule_module");

    /* 添加任务队列 */
    if (send->m_msgType == ChatMsgModel::MessageType::TextMsg) {
        module->m_msgTaskQueue.enqueue(send);
    } else if (send->m_msgType == ChatMsgModel::MessageType::FileMsg
               || send->m_msgType == ChatMsgModel::MessageType::DirMsg) {
        module->m_fileTaskQueue.enqueue(send);
    }

    connect(module, &TcpModule::sigSendMsgSuccess, this, &TcpClient::slotSendMsgSuccess);
    connect(module, &TcpModule::sigRecvMsgSuccess, this, &TcpClient::slotRecvMsgSuccess);
    connect(module, &TcpModule::sigSendFileSuccess, this, &TcpClient::slotSendFileSuccess);
    connect(module, &TcpModule::sigRecvFileSuccess, this, &TcpClient::slotRecvFileSuccess);
    connect(module, &TcpModule::sigSendFileProgress, this, &TcpClient::slotSendFileProgress);
    connect(module, &TcpModule::sigMaintainTcpLinkDelete, this, &TcpClient::slotMaintainTcpLinkDelete);
    connect(module, &TcpModule::sigSendFileFail, this, &TcpClient::sigTcpClientSendMsgFailed);
    connect(module, &TcpModule::sigFriendOffline, this, &TcpClient::sigTcpClientOffline);

    /* 移至线程中 */
    QThread *thread = new QThread;
    thread->setObjectName("kylin-ipmsg_tcpclient_QThread_thread");
    module->moveToThread(thread);

    connect(thread, &QThread::started, module, &TcpModule::startSocket);

    thread->start();

    /* 维护tcp链接表 */
    /* PRO : 此处数据为错误数据 , 应在链接建立后才能拿到正确数据 */
    g_tcpItem newItem;
    newItem.uuid = this->m_send->m_friendUuid;
    newItem.selfIp = AddrSet::getInstance()->getHostIpAddress();
    newItem.selfPort = IniSettings::getInstance()->getLocalTcpPort();
    newItem.peerIp = ip;
    newItem.peerPort = port;
    newItem.module = module;

    if (GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(newItem)) {
        qDebug() << "Error : TcpClient , slotLinkSuccess , maintain tcp link fail";
    }

    return 0;
}

/* 主动取消发送 */
void TcpClient::sendMsgCancel(QString friendUuid, int msgId)
{
    /* 检测链接是否建立 */
    g_tcpMaintain item = GlobalData::getInstance()->m_tcpLink.select(friendUuid);

    if (item.module != NULL) {
        item.module->m_cancelTask.append(msgId);
    }

    return;
}

void TcpClient::slotSendMsgSuccess(ChatMsgInfo *send)
{
    emit sigTcpClientSendMsgSuccess(send);

    return;
}

void TcpClient::slotRecvMsgSuccess(ChatMsgInfo *recv)
{
    emit sigTcpClientRecvMsgSuccess(recv);

    return;
}

void TcpClient::slotSendFileProgress(ChatMsgInfo *send)
{
    emit sigTcpClientSendFileProgress(send);

    return;
}

void TcpClient::slotSendFileSuccess(ChatMsgInfo *send)
{
    emit sigTcpClientSendFileSuccess(send);

    return;
}

void TcpClient::slotRecvFileSuccess(ChatMsgInfo *recv)
{
    emit sigTcpClientRecvFileSuccess(recv);

    return;
}

void TcpClient::slotMaintainTcpLinkDelete(QString uuid)
{
    if (GlobalData::getInstance()->m_tcpLink.tcpMaintainDelete(uuid)) {
        qDebug() << "Waring , TcpClient , slotMaintainTcpLinkDelete , maintain tcp link fail";
    }

    return;
}
