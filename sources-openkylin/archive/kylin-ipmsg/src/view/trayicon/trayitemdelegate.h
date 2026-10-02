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

#ifndef TRAYITEMDELEGATE_H
#define TRAYITEMDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QAbstractItemDelegate>
#include <QStyleOption>
#include <QPainter>
#include <QRectF>
#include <QPoint>

class TrayItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    friend class TestTrayItemDelegate;
public:
    explicit TrayItemDelegate(QAbstractItemDelegate *parent = nullptr);
    ~TrayItemDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TRAYITEMDELEGATE_H