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

#ifndef __SIMPLE_TCP_LINK_H__
#define __SIMPLE_TCP_LINK_H__

#include <QList>
#include <pthread.h>
#include <QString>

// 简化版的数据结构
typedef struct TcpMaintain
{
    QString uuid;
    QString selfIp;
    QString selfPort;
    QString peerIp;
    QString peerPort;
    QString peerListenIp;
    QString peerListenPort;
    void *module;  // 使用void*代替TcpModule*

} g_tcpMaintain;

typedef struct TcpItem
{
    QString uuid;
    QString selfIp;
    QString selfPort;
    QString peerIp;
    QString peerPort;
    void *module;  // 使用void*代替TcpModule*
} g_tcpItem;

typedef struct UdpItem
{
    QString uuid;
    QString peerListenIp;
    QString peerListenPort;
} g_udpItem;

class TcpLink
{
public:
    TcpLink();
    ~TcpLink();

    int init();
    void ergodic(void);
    int tcpMaintainAdd(g_tcpItem);
    int tcpMaintainDelete(QString uuid);
    int udpMaintainAdd(g_udpItem);
    int udpMaintainDelete(QString uuid);
    g_tcpMaintain select(QString uuid);

private:
    pthread_rwlock_t m_rwLock;
    QList<g_tcpMaintain *> m_tcpMaintain;
};

#endif