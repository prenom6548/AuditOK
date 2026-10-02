/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef TITLESEARCHLISTDELEGATE_H
#define TITLESEARCHLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QStandardItemModel>

#include <QMouseEvent>
#include "global/viewvar.h"
#include "global/corevar.h"

class TitleSearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
signals:

public:
    explicit TitleSearchListDelegate(QObject *parent = 0);
    ~TitleSearchListDelegate();

    // set the drop down list style of the searchview widget.
    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const;

    // set the size of drop down item of the searchview widget.
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //此事件可对在item上的操作做出反应
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void changeCityFromSearch(QModelIndex m_index); //从搜索列表改变当前城市

};


#endif 
