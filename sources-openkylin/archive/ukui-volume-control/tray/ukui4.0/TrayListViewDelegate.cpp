/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "TrayListViewDelegate.h"

#include <QPainter>

void TrayListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRect rect = option.rect;

    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(rect, opt.palette.highlight());
    }

    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    int iconSize = 32;
    QRect iconRect(rect.x() + 5, rect.y() + (rect.height() - iconSize) / 2, iconSize, iconSize);
    icon.paint(painter, iconRect);

    QString text = index.data(Qt::DisplayRole).toString();
    QRect textRect(rect.x() + iconSize + 50, rect.y(), rect.width() - iconSize - 55, rect.height());
    painter->drawText(textRect, Qt::AlignVCenter, text);
}

QSize TrayListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(150, 50);
}
