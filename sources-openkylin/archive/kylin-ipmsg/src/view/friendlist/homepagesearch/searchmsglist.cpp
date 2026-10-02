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

#include "searchmsglist.h"

SearchMsgList::SearchMsgList(QListView *parent) : QListView(parent)
{
    setViewUi();
}

SearchMsgList::~SearchMsgList() {}

// 单例，初始化返回指针
SearchMsgList *SearchMsgList::getInstance(QListView *parent)
{
    static SearchMsgList *instance = nullptr;

    if (instance == nullptr) {
        instance = new SearchMsgList(parent);
        instance->setAccessibleName("kylin-ipmsg_searchmsglist_SearchMsgList_instance");
    }

    return instance;
}


// 初始化组件
void SearchMsgList::setViewUi()
{
    m_chatMsgDelegate = new SearchMsgDelegate();
    m_chatMsgDelegate->setObjectName("kylin-ipmsg_searchmsglist_SearchMsgDelegate_mchatMsgDelegate");
    this->setItemDelegate(m_chatMsgDelegate);

    this->setModel(SearchMsgModel::getInstance());

    this->setSpacing(0);
    this->setDragEnabled(false);
    this->setFrameShape(QListView::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->verticalScrollBar()->setDisabled(true);

    connect(this, &SearchMsgList::doubleClicked, this, &SearchMsgList::startChat);
}

// 根据字符串过滤好友聊天记录
void SearchMsgList::filterFriendByReg(QString regStr)
{

    /* sdk功能点打点：主界面搜索 */
    GlobalSizeData::SDKPointMainSearch();

    if (regStr == "") {
        return;
    }

    emit this->sigSelectMsg(regStr);

    return;
}

// 发起聊天
void SearchMsgList::startChat()
{
    // 将代理index转换为好友index
    QModelIndex friendIndex = this->currentIndex();
    int friendId = friendIndex.data(ChatMsgModel::FriendId).toInt();

    // 清空好友未读消息数
    FriendListModel::getInstance()->clearUnreadMsgNum(friendId);

    emit sigStartChat(friendId);
}
