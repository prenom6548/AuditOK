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

#ifndef SEARCHMSGLIST_H
#define SEARCHMSGLIST_H

#include <QWidget>
#include <QListView>
#include <string.h>
#include <stdio.h>

#include "view/chatmsg/chatmsg.h"
#include "search_msg_delegate.h"
#include "searchpage.h"
#include "view/localinfo/localinfo.h"
#include "view/friendlist/homepagesearch/search_msg_model.h"

class SearchMsgList : public QListView
{
    Q_OBJECT
    friend class TestSearchMsgList;
public:
    explicit SearchMsgList(QListView *parent = nullptr);
    ~SearchMsgList();

    // 单例，初始化返回指针
    static SearchMsgList *getInstance(QListView *parent = nullptr);

    // 初始化组件
    void setViewUi();


    // 根据字符串过滤好友
    void filterFriendByReg(QString regStr);

    void startChat();


private:
    SearchMsgDelegate *m_chatMsgDelegate;
    QHash<int, ChatMsg *> m_msgWidMap;

signals:
    void sigStartChat(int friendId);
    void sigSelectMsg(QString);


public slots:
};

#endif // SEARCHMSGLIST_H
