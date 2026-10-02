/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "widget.h"
#include "kaddfilebutton.h"
#include <QAbstractItemDelegate>
#include <QBoxLayout>
#include <QDebug>
#include <QTreeView>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QTreeView *view = new QTreeView(this);
    QHBoxLayout *layout = new QHBoxLayout(view);
    KAddFileButton *button1 = new KAddFileButton();
    KAddFileButton *button2 = new KAddFileButton();
    QStringList list;
    list << "JPEG Files(*.jpg)"
         << "PNG Files(*.png)";
    button2->setNameFilters(list);

    layout->addWidget(button1);
    layout->addWidget(button2);

    connect(button1, &KAddFileButton::clicked, [=] {
        qDebug() << "button1 选择路径" << button1->fileDialog()->selectedFiles();
    });
    connect(button2, &KAddFileButton::clicked, [=] {
        qDebug() << "button2 选择路径" << button2->fileDialog()->selectedFiles();
    });

    setFixedSize(600, 500);
}

Widget::~Widget()
{
}
