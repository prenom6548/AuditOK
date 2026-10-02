/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "customtablewidget.h"

CustomTableWidget::CustomTableWidget(QList<QString> header, QWidget *parent) : QTreeWidget(parent)
{
    this->setColumnCount(3);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 将表格变为禁止编辑,取消焦点
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setFocusPolicy(Qt::NoFocus);
    // 设置表格不可选中
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    // 设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
    this->setHeaderLabels(header);

    //设置表头文字左对齐
    this->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->header()->setFixedHeight(40);
    this->header()->setSectionResizeMode(QHeaderView::Stretch);
    //设置交替背景色
    this->setAlternatingRowColors(true);
    this->setUniformRowHeights(true);

    this->setMouseTracking(true);
    this->setRootIsDecorated(false);
}

void CustomTableWidget::insertItem(int i, QStringList content)
{
    QWidget *widget = new QWidget;
    widget->setFixedHeight(40);
    QTreeWidgetItem *item = new QTreeWidgetItem(content);
    item->setToolTip(0, content[0]);
    item->setSizeHint(i, QSize(260, 40));
    item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
    this->insertTopLevelItem(i, item);
    this->setItemWidget(item, 2, widget);
}

QStringList CustomTableWidget::getSelectedRow()
{
    QList<QTreeWidgetItem*> items = this->selectedItems();
    QStringList contents;
    for(int i = 0; i < items.length(); i++){
       for(int j = 0; j <items.at(i)->columnCount(); j++){
           contents.append(items.at(i)->text(j));
       }
    }
    return contents;
}
