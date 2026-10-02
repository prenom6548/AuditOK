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
#include <QHeaderView>

#include "custom_list_view.h"

CustomListView::CustomListView(QWidget *parent)
    : QTreeView(parent)
{
    // 选择整行
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // 隔行变色
    setAlternatingRowColors(true);
    // 禁用双击编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 表头居中
    // header()->setDefaultAlignment(Qt::AlignCenter);
    // 拉伸最后一列
    header()->setStretchLastSection(true);
    // 取消第一列箭头
    setRootIsDecorated(false);
    // 禁止移动列
    header()->setSectionsMovable(false);
    // 代理中设置每行宽度
    setItemDelegate(new ListItemDelegate());
}

CustomListView::~CustomListView()
{
}

void CustomListView::setModel(QAbstractItemModel *model)
{
    int offset = 0;
    QTreeView::setModel(model);
    for(int i = 0; i < header()->count(); i++) {
        setColumnWidth(i, columnWidth(i) + offset);
    }
}