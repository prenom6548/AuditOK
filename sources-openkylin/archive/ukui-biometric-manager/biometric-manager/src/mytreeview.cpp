/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "mytreeview.h"
#include <QModelIndex>
#include <QEvent>
#include <QDebug>
#include <QToolTip>
MyTreeView::MyTreeView(QWidget *parent) : QTreeView(parent)
{
    this->installEventFilter(this);
    setAttribute(Qt::WA_Hover, true);
}

MyTreeView::~MyTreeView() {}

bool MyTreeView::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QPoint point = QCursor::pos();
        QPoint windowPoint = this->mapFromGlobal(point);
        QModelIndex index = indexAt(QPoint(windowPoint.x(), windowPoint.y() - 40));
        QString idxName = index.data().toString();
        if (idxName != "" && index.column() == 1) {
            QToolTip::showText(point, idxName);
        }
    } else if (event->type() == QEvent::HoverMove) {
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    }
    return QTreeView::event(event);
}

void MyTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            Q_EMIT rightClicked(index);
        }
    }
    QTreeView::mousePressEvent(event);
}
