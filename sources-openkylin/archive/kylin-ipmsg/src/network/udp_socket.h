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

#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#define UDP_BROADCAST 0x01 // 广播
#define UDP_UNICAST 0x02   // 单播
#define UDP_MULTICAST 0x03 // 多播
#define UDP_GOODBYE 0x04   // 下线

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QNetworkInterface>
#include <QMap>
#include <QDateTime>

#include "model/friendlistmodel.h"
#include "global/utils/globalutils.h"
#include "global/utils/global_data.h"

class UdpSocket : public QObject
{
    Q_OBJECT
    friend class TestUdpSocket_SlotSocketReadOnlineTest_Test;
    friend class TestUdpSocket_SlotSocketReadOfflineTest_Test;
    friend class TestUdpSocket_UdpSocketBroadcastIpTest_Test;
    friend class TestUdpSocket_SlotOfflineTest_Test;
    friend class TestUdpSocket_SlotCheckOnlineTest_Test;

public:
    UdpSocket();
    ~UdpSocket();

    void udpSocketBroadcast(bool flag);

private:
    QUdpSocket *m_udpSocket;
    QTimer *m_timer;

    // 好友上线时间列表 <好友uuid, 时间秒数>
    QMap<QString, qint64> m_friendOnlineTimeMap;
    QTimer *m_onlineTimer;
    int m_maxDiffSecs;

private:
    void init(void);
signals:
    // 添加或更新好友上线信息
    void addOnlineFriend(FriendInfoData *friendInfo);

    // 更新好友离线信息
    void updateOfflineFriend(FriendInfoData *friendInfo);

private slots:
    void slotSocketRead(void);
    void slotOnline(void);
    void slotOffline(void);

    // 检测好友上线时间
    void slotCheckOnline();
};

#endif
