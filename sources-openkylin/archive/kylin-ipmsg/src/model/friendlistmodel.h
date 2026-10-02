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

#ifndef FRIENDLISTMODEL_H
#define FRIENDLISTMODEL_H

#include <QObject>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVariant>
#include <QHash>
#include <QDebug>
#include <QUuid>

#include "global/database/friendlistdb.h"

class FriendListModel : public QStandardItemModel
{
    Q_OBJECT
    friend class TestChatMsg;
    friend class TestFriendInfoWid;
public:
    explicit FriendListModel();
    ~FriendListModel();

    // 单例，初始化返回指针
    static FriendListModel *getInstance();

    // 通过好友uuid获取item
    QStandardItem *getItemByUuid(QString friendUuid);

    // 添加好友
    void addFriend(FriendInfoData *friendInfo);

    // 更新好友信息
    void updateFriend(FriendInfoData *friendInfo);

    // 更新好友状态
    // void updateFriendState(QString uuid， bool isOnline);

    // 更新日期短格式和时间的格式
    void updateDateTimeFormat();

    // 删除好友
    void removeFriend(QString uuid, int row);
    void removeFriend(int friendId, int row);

    // 从数据库获取好友信息
    QSqlTableModel *getModelFromDb();

    // 更新好友在线状态
    void updateOnlineState(QString uuid, bool isOnline);
    void updateOnlineState(int friendId, bool isOnline);

    // 更新好友优先级
    bool updatePriority(FriendInfoData *friendInfo);

    // 更新好友备注
    bool updateNickname(FriendInfoData *friendInfo);

    // 更新好友聊天消息 时间 消息数
    bool updateMessage(FriendInfoData *friendInfo);

    // 清空好友未读消息数
    bool clearUnreadMsgNum(int friendId);

    // 清空所有好友聊天消息 时间 消息数
    bool clearAllMessage();

    // 数据库model
    QSqlTableModel *m_tableModel;

    // 通过id获取好友item
    QStandardItem *getFriendById(int id);

    // 通过uuid获取好友item
    QStandardItem *getFriendByUuid(QString uuid);

    // 通过id获取好友uuid
    QString getUuidById(int id);

    // 通过uuid获取好友id
    int getIdByUuid(QString uuid);

    enum FriendRoles {
        Id = Qt::UserRole + 1,
        Uuid,
        Ip,
        Port,
        Mac,
        Username,
        Nickname,
        System,
        Platform,
        Avatar,
        RecentMsgContent,
        RecentMsgTime,
        UnreadMsgNum,
        OnlineState,
        Priority,
        Display,
        DisplayRecentMsgTime, /* 用于"日期短格式+时间"的更新，无其它作用 */
    };

    enum OnlineType {
        Offline = 0,
        Online,
    };

    enum PriorityType {
        PriDefault = 0,
        PriStayTop,
    };

    enum DisplayType {
        DisHide = 0,
        DisShow,
    };

    // 已存在的好友
    /*<"id" , QStandardItem *>*/
    QHash<int, QStandardItem *> m_idItemsMap;

    /*<"uuid" , QStandardItem *>*/
    QHash<QString, QStandardItem *> m_uuidItemsMap;

    // 已加载的好友ID
    QList<int> m_loadFriendIdList;

private:
signals:
    void updateFriendInfo(int friendId);
    void sigUpdateFriendState();
    void sigDeleFriend();
};

#endif // FRIENDLISTMODEL_H
