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
#ifndef CUSTOMTABLEWIDGET_H
#define CUSTOMTABLEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QScrollArea>
#include <QHeaderView>
#include <QTreeWidgetItem>

class CustomTableWidget : public QTreeWidget
{
    Q_OBJECT
public:
    CustomTableWidget(QList<QString> header, QWidget *parent = nullptr);

    void insertItem(int i, QStringList content);
    QStringList getSelectedRow();
};

#endif // CUSTOMTABLEWIDGET_H
