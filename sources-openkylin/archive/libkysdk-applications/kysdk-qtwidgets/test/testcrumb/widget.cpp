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
#include "kborderlessbutton.h"
#include "kcrumb.h"
#include <QBoxLayout>
#include <QMessageBox>
#include <QTabBar>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    KCrumb *crumb = new KCrumb(this);
    //    QTabBar*crumb = new QTabBar(this);
    crumb->setUsesScrollButtons(false);
    // 默认是true
    crumb->setFlat(false);
    crumb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    crumb->addTab("页面目录一，页面目录一");
    crumb->addTab("页面目录二，页面目录二");
    crumb->addTab("页面目录三，页面目录三");
    crumb->addTab("页面目录四，页面目录四");
    crumb->addTab("页面目录五，页面目录五");
    //    crumb->setFixedWidth(1200);
    //    crumb->setIcon(QIcon::fromTheme("dialog-info"));  //设置KBreadCrumb的图标
    crumb->setUsesScrollButtons(false);
    ;
    //    crumb->addTab("长的长的长的长的长的长的");
    //    crumb->addTab("长的长的长的长的长的长的");

    //    crumb->setIcon(QIcon::fromTheme("dialog-info"));  //设置KBreadCrumb的图标
    //    KBorderlessButton *btn1 = new KBorderlessButton();
    //    KBorderlessButton *btn2 = new KBorderlessButton();
    //    btn1->setIcon(QIcon::fromTheme("go-previous.symbolic"));
    //    btn2->setIcon(QIcon::fromTheme("go-next.symbolic"));
    //    btn1->setFixedSize(24,24);
    //    btn2->setFixedSize(24,24);
    //    hLayout->addWidget(btn1);
    //    hLayout->addWidget(btn2);
    hLayout->addWidget(crumb);

    mainLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout;
    //    KCrumb*crumb2 = new KCrumb(this);
    QTabBar *crumb2 = new QTabBar(this);
    //    //默认是true
    //    crumb->setFlat(false);  //设置KBreadCrumb是否为flat类型
    //    crumb2->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    crumb2->addTab("页面目录一，页面目录一");
    crumb2->addTab("页面目录二，页面目录二");
    crumb2->addTab("页面目录三，页面目录三");
    crumb2->addTab("短的");
    crumb2->addTab("长的长的长的长的长的长的");
    ////    crumb2->setIcon(QIcon::fromTheme("dialog-warning"));
    hLayout->addWidget(crumb2);
    mainLayout->addLayout(hLayout);
    //    crumb2->setFixedWidth(200);
    ////    crumb2->setShape(QTabBar::RoundedWest);

    mainLayout->addLayout(hLayout);
    setLayout(mainLayout);

    //    if(crumb->currentIndex() == 0)
    //    {
    //        btn1->setEnabled(false);
    //    }
    //    else if(crumb->currentIndex() == crumb->count() - 1)
    //    {
    //        btn2->setEnabled(false);
    //    }

    //    connect(btn1,&KBorderlessButton::clicked,this,[=]{
    //        crumb->leftScrollTabs();
    //    });
    //    connect(btn2,&KBorderlessButton::clicked,this,[=]{
    //        crumb->rightScrollTabs();
    //    });
    setMinimumSize(600, 500);
    crumb2->setElideMode(Qt::ElideMiddle);
    connect(crumb, &KCrumb::tabBarClicked, this, [=](int index) {
        if (index == 1)
            crumb->setMaximumWidth(1300);
    });
    //    connect(crumb2,&QTabBar::tabBarClicked,this,[=](int index){
    //        if(index == 0)
    //        {
    //            crumb2->setTabText(1,"...");
    //            crumb2->adjustSize();
    //        }
    //        if(index == 1)
    //        {
    //            crumb2->setTabText(1,"页面目录二，页面目录二");
    //            crumb2->adjustSize();
    //        }
    //    });
}

Widget::~Widget()
{
}
