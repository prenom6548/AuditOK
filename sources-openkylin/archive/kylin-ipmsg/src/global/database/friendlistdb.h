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

#ifndef FRIENDLISTDB_H
#define FRIENDLISTDB_H

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
#include <QMessageBox>
#include <QDebug>

#include "database.h"

class FriendInfoData : public QObject
{
    Q_OBJECT
public:
    FriendInfoData(int friendId = 0, QString friendUuid = "", QString friendIp = "", int friendPort = 0,
                   QString friendMac = "", QString username = "", QString nickname = "", QString system = "",
                   QString platform = "", QString avatarUrl = "", QString recentMsgCont = "",
                   QString recentMsgTime = "", int unreadMsgNum = 0, int onlineState = 0, int priority = 0,
                   int m_display = 1);

    FriendInfoData(QMap<QString, QString> strMap, QMap<QString, int> intMap);
    ~FriendInfoData();

    int m_friendId = 0;
    QString m_friendUuid = "";
    QString m_friendIp = "";
    int m_friendPort = 0;
    QString m_friendMac = "";
    QString m_username = "";
    QString m_nickname = "";
    QString m_system = "";
    QString m_platform = "";
    QString m_avatarUrl = "";
    QString m_recentMsgCont = "";
    QString m_recentMsgTime = "";
    QString m_recentDispalyMsgTime = "";
    int m_unreadMsgNum = 0;
    int m_onlineState = 0;
    int m_priority = 0;
    int m_display = 1;

private:
};

class FriendInfoDataBase : public QObject
{
    Q_OBJECT
public:
    explicit FriendInfoDataBase(QObject *parent = nullptr);
    ~FriendInfoDataBase();

    // 单例，初始化返回指针，完成数据库文件建立
    static FriendInfoDataBase *getInstance();

    // 插入数据
    int insertData(FriendInfoData *friendInfo);

    // 查询数据
    void queryData();

    // 更新数据
    bool updateData(FriendInfoData *friendInfo);

    // 删除好友
    bool deleteFriend(int friendId);

    // 更新好友上线信息
    bool updateFriend(FriendInfoData *friendInfo);

    // 更新好友优先级
    bool updatePriority(FriendInfoData *friendInfo);

    // 更新好友备注
    bool updateNickname(FriendInfoData *friendInfo);

    // 更新好友聊天消息 时间 消息数
    bool updateMessage(FriendInfoData *friendInfo);

    // 清空所有好友聊天消息 时间 消息数
    bool clearAllMessage();

    // 更新好友显示状态
    bool updateDisplay(int friendId, int display);

    // 清空好友未读消息数
    bool clearUnreadMsgNum(int friendId);

    // 根据ID获取好友信息
    FriendInfoData *queryInfoById(int friendId);

    // 根据UUID获取好友信息
    FriendInfoData *queryInfoByUuid(QString friendUuid);

    QSqlTableModel *m_model;

    QSqlQuery m_query;

private:
    QSqlDatabase m_db;
    QString m_dbName;
    QString m_dbTableName;


    enum { UPDATE = 0, INSERT, DELETE };
    int m_operator;
};




#endif // FRIENDLISTDB_H
