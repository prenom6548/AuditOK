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

#ifndef __DECLARE_H__
#define __DECLARE_H__

#include <QString>

class TcpModule;

/* 代表tcp链接表的一条数据 */
typedef struct TcpMaintain
{
    QString uuid;
    QString selfIp;
    QString selfPort;
    QString peerIp;
    QString peerPort;
    QString peerListenIp;
    QString peerListenPort;
    TcpModule *module;

} g_tcpMaintain;

/* tcp维护tcp链接表 */
typedef struct TcpItem
{
    QString uuid;
    QString selfIp;
    QString selfPort;
    QString peerIp;
    QString peerPort;
    TcpModule *module;
} g_tcpItem;

/* udp维护tcp链接表 */
typedef struct UdpItem
{
    QString uuid;
    QString peerListenIp;
    QString peerListenPort;
} g_udpItem;

#endif
