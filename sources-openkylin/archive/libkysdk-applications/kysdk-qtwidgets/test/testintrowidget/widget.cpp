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
#include "kintrowidget.h"
#include <QBoxLayout>
#include <QMenu>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    KIntroWidget *widget1 = new KIntroWidget(this);
    widget1->setMainLabel("我是一段测试文本，测试显示不全是否可以换行(单行测试文本)");
    widget1->setSubLabel("2024216161");
    widget1->setFixedHeight(76);
    widget1->setBorderRadius(15);
    KIntroWidget *widget2 = new KIntroWidget(this);
    widget2->setMainLabel("我是一段测试文本，测试换行符号\n('\\n'不显示只换行)是否可以正常换行以及显示不全是否可以换行");
    widget2->setSubLabel("2024216161");
    widget2->setViewType(ViewType::IconView);
    widget2->setFixedSize(218, 200);
    widget2->setTitleColor(Qt::red);
    widget2->setBorderRadius(0, 20, 20, 30);
    layout->addWidget(widget1);
    layout->addWidget(widget2);

    QMenu *menu = new QMenu();
    QAction *action1 = new QAction("action1", menu);
    QAction *action2 = new QAction("action2", menu);
    QAction *action3 = new QAction("action3", menu);
    menu->addAction(action1);
    menu->addAction(action2);
    menu->addAction(action3);

    connect(widget1, &KIntroWidget::rightClick, this, [=](QPoint point) {
        menu->move(point);
        menu->show();
    });

    QMenu *menu1 = new QMenu();
    QAction *action4 = new QAction("action4", menu);
    QAction *action5 = new QAction("action5", menu);
    QAction *action6 = new QAction("action6", menu);
    menu1->addAction(action4);
    menu1->addAction(action5);
    menu1->addAction(action6);

    connect(widget2, &KIntroWidget::rightClick, this, [=](QPoint point) {
        menu1->move(point);
        menu1->show();
    });
}

Widget::~Widget()
{
}
