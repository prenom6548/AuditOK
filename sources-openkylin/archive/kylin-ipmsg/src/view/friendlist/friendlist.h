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

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QObject>
#include <QListView>
#include <QScrollBar>
#include <QDebug>
#include <QCursor>
#include <QMenu>
#include <QPoint>
#include <KWindowSystem>
#include <QRegExp>

#include "mysortfiltermodel.h"
#include "friendItemdelegate.h"
#include "view/chatmsg/chatmsg.h"
#include "friendinfowid.h"

class FriendListView : public QListView
{
    Q_OBJECT
public:
    explicit FriendListView(QListView *parent = nullptr);
    ~FriendListView();

    // 单例，初始化返回指针
    static FriendListView *getInstance(QListView *parent = nullptr);

    // 初始化组件
    void setViewUi();

    // 设置组件样式
    void setViewStyle();

    // 初始化好友列表Model
    void initListModel();

    // 显示右键菜单
    void showContextMenu(const QPoint &);

    // 根据好友id获取好友聊天框
    ChatMsg *getMsgWidById(int friendId);

    // 获取好友列表model
    FriendListModel *getFriendListModel();

    // 根据字符串过滤好友
    void filterFriendByReg(QString regStr);

    // 清理所有聊天框的聊天记录
    void clearAllMessages();

    // 发起聊天
    void startChat();

    // 设为置顶
    void setTop();

    // 修改好友备注
    void changeNickname();

    // 查看资料
    void viewInfo();

    // 删除好友
    void delFriend();

    friend class TestFriendListView;

    MySortFilterModel *m_sortFilterModel;

protected:
private:
    FriendListModel *m_friendListModel;

    FriendItemDelegate *m_myItemDelegate;

    QMenu *m_funcMenu;
    QAction *m_startChatAction;
    QAction *m_setTopAction;
    QAction *m_nicknameAction;
    QAction *m_viewInfoAction;
    QAction *m_delFriendAction;

    QHash<int, ChatMsg *> m_msgWidMap;

signals:
    void sigNewChatMsgWid(ChatMsg *chatMsgWid);
    void sigStartChat(int friendId);
    void sigFriendState();
    void sigFriendsEmpty();

public slots:
    void slotShowMsgWidById(int friendId);
    void slotFriendState();
    void slotDeleFriend();
};

#endif // FRIENDLIST_H
