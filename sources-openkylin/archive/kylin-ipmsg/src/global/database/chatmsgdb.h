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

#ifndef CHATMSGDB_H
#define CHATMSGDB_H

#include <QObject>
#include <QThread>
#include <QMutexLocker>
#include <QFile>
#include <QApplication>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QList>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

#include "database.h"

class ChatMsgInfo
{
public:
    explicit ChatMsgInfo(int msgId = 0, int friendId = 0, QString friendUuid = "", int isSend = 0, QString msgTime = "",
                         QString msgContent = "", int msgType = 0, QString filePath = "", int sendState = 0,
                         int readState = 0, qint64 totalSize = 0, qint64 transferSize = 0);
    ~ChatMsgInfo();

    int m_msgId = 0;
    int m_friendId = 0;
    QString m_friendUuid = "";
    int m_isSend = 0;
    QString m_msgTime = "";
    QString m_msgContent = "";
    int m_msgType = 0;
    QString m_filePath = "";
    int m_sendState = 0;
    int m_readState = 0;
    qint64 m_totalSize = 0;
    qint64 m_transferSize = 0;
    int m_chooseMsgState = 0;
    QString m_displayMsgTime = "";

    // get set 函数

private:
    /* data */
};


class ChatMsgDB : public QObject
{
    Q_OBJECT

public:
    explicit ChatMsgDB(QObject *parent = nullptr);
    ~ChatMsgDB();

    // 单例，初始化返回指针，完成数据库文件建立
    static ChatMsgDB *getInstance();

    // 插入数据
    int insertData(ChatMsgInfo *chatMsg);

    // 查询数据
    void queryData();

    // 删除数据
    bool deleteMsg(int msgId);

    // 清空消息
    bool clearMsgByFriendId(int friendId);

    // 清空所有消息
    bool clearAllMsg();

    // 根据ID获取聊天信息
    ChatMsgInfo *queryMsgById(int msgId);

    // 更新消息发送状态
    bool updateMsgState(ChatMsgInfo *chatMsgInfo);

    // 更新文件传输大小
    bool updateTransferSize(ChatMsgInfo *chatMsgInfo);

    // 根据friendUuid获取聊天信息
    QList<ChatMsgInfo *> queryMsgByUuid(QString uuid);

    // 根据friendId获取聊天信息
    QList<ChatMsgInfo *> queryMsgByFriendId(int friendId);

    QSqlTableModel *m_model;

    QSqlQuery m_query;

private:
    QSqlDatabase m_db;
    QString m_dbName;
    QString m_dbTableName;
};



#endif // CHATMSGDB_H
