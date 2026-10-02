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
#include "karrowtabbar.h"
#include "knativetabbar.h"
#include <QHBoxLayout>
#include <QLabel>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    KArrowTabBar *tab1 = new KArrowTabBar();
    KNativeTabbar *tabBar1 = tab1->tabbar();
    tabBar1->setTabbarStyle(KNativeTabbarStyle::SegmentLight);
    tabBar1->addTab(QIcon::fromTheme("kylin-music"), "Segment1");
    tabBar1->addTab("Segment2");
    tabBar1->addTab(QIcon::fromTheme("kylin-music"), "");
    tabBar1->addTab("Segment4");
    tabBar1->addTab("Segment5");
    tabBar1->addTab("Segment6");

    KArrowTabBar *tab2 = new KArrowTabBar();
    KNativeTabbar *tabBar2 = tab2->tabbar();
    tabBar2->setTabbarStyle(KNativeTabbarStyle::SegmentDark);

    tabBar2->addTab("SegmentDark1");
    tabBar2->addTab("SegmentDark2");
    tabBar2->addTab(QIcon::fromTheme("kylin-music"), "SegmentDark3");
    tabBar2->addTab("SegmentDark4");
    tabBar2->addTab("SegmentDark5");
    tabBar2->addTab(QIcon::fromTheme("kylin-music"), "");
    KArrowTabBar *tab3 = new KArrowTabBar();
    KNativeTabbar *tabBar3 = tab3->tabbar();
    tabBar3->setTabbarStyle(KNativeTabbarStyle::Sliding);
    tabBar3->addTab(QIcon::fromTheme("ukui-dialog-success"), "正常");
    tabBar3->addTab(QIcon::fromTheme("dialog-warning"), "错误");
    tabBar3->addTab(QIcon::fromTheme("dialog-info"), "注意");
    tabBar3->addTab(QIcon::fromTheme("kylin-music"), "Sliding4");
    tabBar3->addTab("Sliding5");
    tabBar3->addTab(QIcon::fromTheme("kylin-music"), "Sliding6");

    KArrowTabBar *tab4 = new KArrowTabBar();
    KNativeTabbar *tabBar4 = tab4->tabbar();
    tabBar4->addTab("Segment1");
    tabBar4->addTab("Segment2");
    tabBar4->addTab("Segment3");
    tabBar4->addTab("Segment4");
    tabBar4->setTabbarStyle(KNativeTabbarStyle::SegmentDark); // 设置style
    tabBar4->setBorderRadius(10); // 设置圆角半径，只对SegmentDark，SegmentLight样式生效
    tabBar4->setBackgroundColor(QColor(0, 255, 0)); // 设置背景色

    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText("just a test label");
    hlayout->addWidget(label);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addStretch();
    vLayout->addWidget(tab1);
    vLayout->addWidget(tab2);
    vLayout->addWidget(tab3);
    vLayout->addWidget(tab4);
    vLayout->addStretch();
    vLayout->addStretch();
    vLayout->addLayout(hlayout);
    vLayout->addStretch();
    setFixedWidth(500);
}

Widget::~Widget()
{
}
