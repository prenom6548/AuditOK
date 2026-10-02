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

#include "search_msg_model.h"
#include "global/utils/globalutils.h"
#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "view/friendlist/homepagesearch/searchpage.h"

SearchMsgModel::SearchMsgModel() {}

SearchMsgModel::~SearchMsgModel() {}

SearchMsgModel *SearchMsgModel::getInstance()
{
    static SearchMsgModel *instance = nullptr;

    if (instance == nullptr) {
        instance = new SearchMsgModel();
        instance->setObjectName("kylin-ipmsg_searchmsgmodel_SearchMsgModel_instance");
    }

    return instance;
}

void SearchMsgModel::getModel()
{
    QMap<int, int>::iterator begin = this->m_searchResult.begin();
    QMap<int, int>::iterator end = this->m_searchResult.end();
    while (begin != end) {
        int friendId = begin.key();
        int count = begin.value();

        QStandardItem *newItem = new QStandardItem();
        newItem->setAccessibleText("kylin-ipmsg_searchmsgmodel_QStandardItem_newItem");
        newItem->setData(friendId, ChatMsgModel::FriendId);
        newItem->setData(count, ChatMsgModel::MsgCount);

        if (FriendListModel::getInstance()->getFriendById(friendId) != nullptr) {
            if (FriendListModel::getInstance()->getFriendById(friendId)->data(FriendListModel::Display).toInt() != 0) {
                this->appendRow(newItem);
                this->m_msgIdItemsMap.insert(friendId, newItem);
            }
        }

        begin++;
    }

    return;
}

void SearchMsgModel::getSearchResult(QString need)
{

    QSqlDatabase db = DataBase::getInstance()->openDatabse("chat_msg");
    QSqlQuery query(db);

    QString sql = QString("select friend_id , msg_content from chat_msg");

    query.prepare(sql);
    query.exec();

    while (query.next()) {
        int friendId = query.value("friend_id").toInt();
        QString msg = GlobalUtils::uncryptData(query.value("msg_content").toString());

        if (msg.contains(need)) {
            if (this->m_searchResult.contains(friendId)) {
                int tmp = this->m_searchResult.value(friendId);
                tmp++;
                this->m_searchResult.remove(friendId);
                this->m_searchResult.insert(friendId, tmp);

            } else {
                this->m_searchResult.insert(friendId, 1);
            }
        }
    }

    return;
}

void SearchMsgModel::slotSelect(QString msg)
{
    /* 清空原先的model */
    this->clear();

    /* 清空 map 表 */
    this->m_msgIdItemsMap.clear();

    /* 清空原先数据 */
    this->m_searchResult.clear();

    this->getSearchResult(msg);

    this->getModel();

    SearchPage::getInstance()->MsgListSize(this->rowCount());

    return;
}
