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
#ifndef __CUSTOM_LIST_VIEW_H__
#define __CUSTOM_LIST_VIEW_H__

#include <QTreeView>
#include <QStyledItemDelegate>

class ListItemDelegate: public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize s = QStyledItemDelegate::sizeHint(option, index);
        s.setHeight(36);
        return s;
    }
};

class CustomListView : public QTreeView
{
    Q_OBJECT
public:
    CustomListView(QWidget *parent = nullptr);
    ~CustomListView();

    void setModel(QAbstractItemModel *model) override;

};
#endif // __CUSTOM_LIST_VIEW_H__