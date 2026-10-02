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

#ifndef __CHAT_SEARCH_DELEGATE_H__
#define __CHAT_SEARCH_DELEGATE_H__

#include <QAbstractItemDelegate>
#include <QSize>
#include <QAbstractTextDocumentLayout>

#include "global/utils/global_data.h"
#include "view/chatmsg/chatmsg.h"
#include "view/chatsearch/chat_search.h"
#include <QStyleOptionViewItem>

class ChatMsg;
class ChatSearch;

class ChatSearchDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    friend class TestChatSearchDelegate;
public:
    ChatSearchDelegate(ChatMsg *);
    ChatSearchDelegate(ChatMsg *chatMsg, ChatSearch *chatSearch);
    ~ChatSearchDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
                   const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    ChatMsg *m_chatMsg;
    ChatSearch *m_chatSearch;

signals:

public slots:
};

#endif
