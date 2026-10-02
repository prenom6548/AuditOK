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
#include "kcomboboxwidget.h"
#include <QDebug>
#include <QGSettings/QGSettings>
#include <QMenu>
#include <QPushButton>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KComboBoxWidget *widget = new KComboBoxWidget(this);
    //    widget->setLabelWeekSelect("111");

    QStringList list;
    list.append("111");
    list.append("222");
    list.append("333");
    list.append("444444444");
    list.append("55555555555");
    list.append("66666");
    list.append("7777");
    list.append("888");
    widget->setStringList(list);
    widget->setMainText("abnfaowifoawf");
    widget->menu()->setFixedWidth(200);
    connect(widget, &KComboBoxWidget::comBoxWidgetClicked, this, [=] {
        qDebug() << "xxx" << widget->getChosenList() << widget->width() << widget->menu()->size();
    });

    QPushButton *btn = new QPushButton(this);
    QMenu *menu = new QMenu(this);
    menu->addAction("1");
    menu->addAction("2");
    menu->addAction("3");
    menu->addAction("4");
    menu->show();

    btn->move(0, 200);
    btn->setMenu(menu);
    setFixedSize(600, 500);
}

Widget::~Widget()
{
}
