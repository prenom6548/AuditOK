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

#include "udp_socket.h"
#include "global/utils/addrset.h"

UdpSocket::UdpSocket()
{
    init();
    slotOnline();
}

UdpSocket::~UdpSocket()
{
    if (this->m_timer != NULL) {
        delete this->m_timer;
    }

    if (this->m_onlineTimer != NULL) {
        m_onlineTimer->deleteLater();
    }

    if (this->m_udpSocket != NULL) {
        delete this->m_udpSocket;
    }
}

void UdpSocket::init(void)
{
    /* 实例udpsocket */
    this->m_udpSocket = new QUdpSocket;
    this->m_udpSocket->setObjectName("kylin-ipmsg_udpsocket_QUdpSocket_mudpSocket");

    /* 获取udp监听端口 */
    QString port = GlobalData::getInstance()->m_udpListenPort;

    if (this->m_udpSocket->bind(QHostAddress::AnyIPv4, port.toUShort(), QUdpSocket::ReuseAddressHint) == true) {
        qDebug() << "Info : UdpSocket , bind success";
    }

    connect(this->m_udpSocket, &QUdpSocket::readyRead, this, &UdpSocket::slotSocketRead);

    /* 启动定时器 */
    this->m_timer = new QTimer;
    this->m_timer->setObjectName("kylin-ipmsg_udpsocket_QTimer_mtimer");
    connect(this->m_timer, &QTimer::timeout, this, &UdpSocket::slotOnline);
    this->m_timer->start(3000);

    // 好友上线时间计时器
    m_maxDiffSecs = 20;
    this->m_onlineTimer = new QTimer(this);
    this->m_onlineTimer->setObjectName("kylin-ipmsg_udpsocket_QTimer_monlineTimer");
    connect(this->m_onlineTimer, &QTimer::timeout, this, &UdpSocket::slotCheckOnline);
    m_onlineTimer->start(m_maxDiffSecs * 1000);

    return;
}

void UdpSocket::slotSocketRead(void)
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : UdpSocket , recv udp package";
    }

    while (this->m_udpSocket->hasPendingDatagrams()) {
        QByteArray package;
        package.resize(1024);
        QHostAddress peerIp;
        int size = this->m_udpSocket->readDatagram(package.data(), package.size(), &peerIp);
        package.resize(size);

        /* udp包接收完毕 , 解析udp包 */
        char msgType = package.at(0);
        package.remove(0, 1);
        QString data = QString::fromUtf8(package);
        QStringList dataList = data.split(" ");

        if (GlobalUtils::DEBUG_MODE) {
            qDebug() << msgType << data;
        }

        if (msgType == UDP_BROADCAST) {
            /* 上线广播 */
            QString uuid = dataList.at(1);
            QString peerListenIp = dataList.at(2);
            QString peerListenPort = dataList.at(3);
            QString username = dataList.at(4);
            QString platform = dataList.at(5);

            // 本机UDP数据
            if (uuid == GlobalData::getInstance()->m_uuid || uuid == "") {
                return;
            }

            // 刷新好友上线时间
            qint64 currentTimeSec = QDateTime::currentDateTime().toSecsSinceEpoch();
            if (m_friendOnlineTimeMap.contains(uuid)) {
                m_friendOnlineTimeMap.find(uuid).value() = currentTimeSec;
            } else {
                m_friendOnlineTimeMap.insert(uuid, currentTimeSec);
            }

            /* 维护tcp链接表 */
            g_udpItem tmp;
            tmp.uuid = uuid;
            tmp.peerListenIp = peerListenIp;
            tmp.peerListenPort = peerListenPort;

            GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(tmp);

            FriendInfoData *friendInfo = new FriendInfoData();
            friendInfo->setObjectName("kylin-ipmsg_udpsocket_FriendInfoData_friendInfo");

            friendInfo->m_friendUuid = uuid;
            friendInfo->m_friendIp = peerListenIp;
            friendInfo->m_friendPort = peerListenPort.toInt();
            friendInfo->m_username = username;
            friendInfo->m_avatarUrl = GlobalUtils::getAvatarByName(username);
            friendInfo->m_onlineState = 1;

            emit addOnlineFriend(friendInfo);

            /* 测试输出 */
            if (GlobalUtils::DEBUG_MODE) {
                qDebug() << "Info : UdpSocket , slotUdpSocketRead , recv udp data";
                qDebug() << "uuid = " << uuid << "peerListenIp = " << peerListenIp
                         << "peerListenPort = " << peerListenPort;
            }

            GlobalData::getInstance()->m_tcpLink.ergodic();

        } else if (msgType == UDP_GOODBYE) {
            QString uuid = dataList.at(1);

            if (uuid == GlobalData::getInstance()->m_uuid) {
                return;
            }

            // 删除好友上线时间
            if (m_friendOnlineTimeMap.contains(uuid)) {
                m_friendOnlineTimeMap.remove(uuid);
            }

            /* 维护tcp链接表 */
            // GlobalData::getInstance()->m_tcpLink.udpMaintainDelete(uuid);
            // GlobalData::getInstance()->m_tcpLink.ergodic();

            FriendInfoData *friendInfo = new FriendInfoData();
            friendInfo->setObjectName("kylin-ipmsg_udpsocket_FriendInfoData_friendInfo");

            friendInfo->m_friendUuid = uuid;
            friendInfo->m_onlineState = FriendListModel::OnlineType::Offline;

            emit updateOfflineFriend(friendInfo);

            qDebug() << "Info : UdpSocket , slotUdpSocketRead , recv udp offline";
        }
    }

    return;
}

void UdpSocket::udpSocketBroadcast(bool flag)
{
    QByteArray package;
    QString udpListenPort = GlobalData::getInstance()->m_udpListenPort;

    // 无正确IP地址时 不发送广播
    if (GlobalData::getInstance()->m_tcpListenIP.isEmpty()) {
        QString hostIpStr = AddrSet::getInstance()->getHostIpAddress();

        if (hostIpStr == QHostAddress(QHostAddress::LocalHost).toString()) {
            return;
        } else {
            GlobalData::getInstance()->m_tcpListenIP = hostIpStr;
        }
    }

    // uuid不正确时不进行组装包，和发送广播
    if (GlobalData::getInstance()->m_uuid.isEmpty()) {
        return;
    }

    /* 组装数据包 */
    static QString appName = "kylin-ipmsg";
    QString uuid = GlobalData::getInstance()->m_uuid;
    QString tcpListenIp = GlobalData::getInstance()->m_tcpListenIP;
    QString tcpListenPort = GlobalData::getInstance()->m_tcpListenPort;

    if (flag == true) {
        QString signature = QString("%1 %2 %3 %4 %5 %6")
                                .arg(appName)
                                .arg(uuid)
                                .arg(tcpListenIp)
                                .arg(tcpListenPort)
                                .arg(GlobalUtils::getNickname())
                                .arg(GlobalUtils::getPlatformName());

        package.append(UDP_BROADCAST);
        package.append(signature);

    } else {
        qDebug() << "say goodbye";
        QString signature = QString("%1 %2").arg(appName).arg(uuid);

        package.append(UDP_GOODBYE);
        package.append(signature);
    }

    /* 广播发送 */
    QList<QNetworkInterface> interface = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interface.count(); i++) {
        QList<QNetworkAddressEntry> addrs = interface.at(i).addressEntries();

        for (int j = 0; j < addrs.count(); j++) {
            if (addrs.at(j).ip().protocol() == QAbstractSocket::IPv4Protocol
                && (addrs.at(j).broadcast().toString() != "")) {
                this->m_udpSocket->writeDatagram(package.data(), package.length(), addrs.at(j).broadcast(),
                                                 udpListenPort.toUShort());
                this->m_udpSocket->flush();
            }
        }
    }

    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "Info : UdpSocket , udpSocketBroadcast , udp pkg ---> " << package;
    }

    return;
}

void UdpSocket::slotOnline(void)
{
    udpSocketBroadcast(true);

    return;
}

void UdpSocket::slotOffline(void)
{
    udpSocketBroadcast(false);

    return;
}

// 检测好友上线时间
void UdpSocket::slotCheckOnline()
{
    QMap<QString, qint64>::iterator checkOnlineIt = m_friendOnlineTimeMap.begin();

    qint64 currentSecs = QDateTime::currentDateTime().toSecsSinceEpoch();

    while (checkOnlineIt != m_friendOnlineTimeMap.end()) {
        if (currentSecs - checkOnlineIt.value() > m_maxDiffSecs) {

            QString uuid = checkOnlineIt.key();

            // 删除好友上线时间
            if (m_friendOnlineTimeMap.contains(uuid)) {
                m_friendOnlineTimeMap.remove(uuid);
            }

            /* 维护tcp链接表 */
            GlobalData::getInstance()->m_tcpLink.udpMaintainDelete(uuid);
            GlobalData::getInstance()->m_tcpLink.ergodic();

            FriendInfoData *friendInfo = new FriendInfoData();
            friendInfo->setObjectName("kylin-ipmsg_udpsocket_FriendInfoData_friendInfo");

            friendInfo->m_friendUuid = uuid;
            friendInfo->m_onlineState = FriendListModel::OnlineType::Offline;

            emit updateOfflineFriend(friendInfo);

            return;
        }

        checkOnlineIt++;
    }
}
