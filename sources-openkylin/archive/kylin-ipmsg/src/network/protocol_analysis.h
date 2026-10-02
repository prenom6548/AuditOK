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

#ifndef __PROTOCOL_ANALYSIS_H__
#define __PROTOCOL_ANALYSIS_H__

#include <QByteArray>
#include <QMap>

class ProtocolAnalysis
{
public:
    ProtocolAnalysis();
    ~ProtocolAnalysis();

    /* 反序列化 */
    QMap<QByteArray, QByteArray> analysisHeadBody(QByteArray str);
    /* 序列化 */
    QByteArray encapsulationHeadBody(QMap<QByteArray, QByteArray> map);
};

#endif

/*
 * 后续可以加控制字段 , 用于服务端有一个数据包接收失败时 , 通知客户端停止传输
 * 单个包为4096字节 , 包头和包体可扩展长度 , 包头附带数据区域长度 , 单包固定长度4096字节 ,
 * 以键值对的形式来序列化和反序列化 包头 + 包体 协议 *uuid:xxx\r\n *type:msg/file\r\n *body_size:xxx\r\n
 *
 * file_stat:info/data/end\r\n      发送文件阶段
 * file_type:file/dir\r\n
 * file_name:xx\r\n
 * file_total_size:xxx\r\n
 *
 * \r\n
 *
 * body
 */
