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
#include "knavigationtrail.h"
#include <QHBoxLayout>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    KNavigationTrail *trail = new KNavigationTrail();
    KCrumb *crumb1 = trail->crumb();
    crumb1->addTab("页面目录一，页面目录一");
    crumb1->addTab("页面目录二，页面目录二");
    crumb1->addTab("页面目录三，页面目录三");
    crumb1->addTab("页面目录四，页面目录四");
    crumb1->addTab("页面目录五，页面目录五");
    crumb1->setFlat(false);
    //    crumb1->setIcon(QIcon::fromTheme("dialog-info"));  //设置KBreadCrumb的图标
    layout->addWidget(trail);
    connect(crumb1, &KCrumb::tabBarClicked, this, [=](int index) {
        if (index == 1)
            trail->setPosition(ButtonPosition::LeftOrRight);
        if (index == 2)
            trail->setPosition(ButtonPosition::LeftPosition);
        if (index == 3)
            trail->setPosition(ButtonPosition::RightPosition);
    });
}

Widget::~Widget()
{
}
