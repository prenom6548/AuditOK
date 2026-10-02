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

#ifndef SEARCHMSGDELEGATE_H
#define SEARCHMSGDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QStyleOption>
#include <QModelIndex>
#include <QEvent>
#include <QAbstractItemDelegate>
#include <QDateTime>
#include <QPainter>
#include <QVariant>
#include <QRectF>
#include <QPoint>
#include <QDebug>
#include <QAbstractTextDocumentLayout>

#include "model/chatmsgmodel.h"
#include "global/utils/global_data.h"

class SearchMsgDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    friend class TestSearchMsgList;
    friend class TestSearchMsgDelegate;
public:
    SearchMsgDelegate();
    ~SearchMsgDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool findHash(int friendId, int msgCount) const;
    QHash<int, int> m_friendIDHash;
    QHash<int, int> countHash(int fid, int count) const;
};





#endif // SEARCHMSGDELEGATE_H
