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

#ifndef GLOBALUTILS_H
#define GLOBALUTILS_H

#include <QObject>
#include <QProcess>
#include <QHostInfo>
#include <QUuid>
#include <QByteArray>

#define DEFAULT_UDP_PORT 39900
#define DEFAULT_TCP_PORT 39901

class GlobalUtils : public QObject
{
    Q_OBJECT
public:
    GlobalUtils();
    ~GlobalUtils();

    // debug日志打印标识
    static bool DEBUG_MODE;

    // 获取可用的TCP端口号
    static int getAvailTcpPort();

    // 获取系统用户名
    static QString getUsername();

    // 获取本机昵称
    static QString getNickname();

    // 获取系统平台名
    static QString getPlatformName();

    // 生成用户UUID
    static QString getUserUuid();

    // 默认文件保存目录
    static QString getDefaultPath(bool isSendPath = false);

    // 时间戳格式
    static QString getTimeFormat(bool isMsec = true);

    // 根据用户名或备注获取头像
    static QString getAvatarByName(QString name);

    // 将数据进行加密
    static QString encryptData(QVariant data);

    // 将数据进行解密
    static QString uncryptData(QVariant data);

    // 获取文件夹大小
    static quint64 getDirSize(const QString filePath);

private:
    /* data */
};




#endif // GLOBALUTILS_H