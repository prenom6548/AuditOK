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
#include "knativetabbar.h"
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QWidget *w = new QWidget();
    KNativeTabbar *tabBar1 = new KNativeTabbar(KNativeTabbarStyle::SegmentLight, w);
    tabBar1->addTab(QIcon::fromTheme("kylin-music"), "Segment1");
    tabBar1->addTab("Segment2");
    tabBar1->addTab(QIcon::fromTheme("kylin-music"), "");
    tabBar1->addTab("Segment4");
    tabBar1->addTab("Segment5");
    tabBar1->addTab("Segment6");
    KNativeTabbar *tabBar2 = new KNativeTabbar(KNativeTabbarStyle::SegmentDark, w);
    tabBar2->addTab("SegmentDark1");
    tabBar2->addTab("SegmentDark2");
    tabBar2->addTab(QIcon::fromTheme("kylin-music"), "SegmentDark3");
    tabBar2->addTab("SegmentDark4");
    tabBar2->addTab("SegmentDark5");
    tabBar2->addTab(QIcon::fromTheme("kylin-music"), "");
    KNativeTabbar *tabBar3 = new KNativeTabbar(KNativeTabbarStyle::Sliding, w);
    tabBar3->addTab(QIcon::fromTheme("ukui-dialog-success"), "正常");
    tabBar3->addTab(QIcon::fromTheme("dialog-warning"), "错误");
    tabBar3->addTab(QIcon::fromTheme("dialog-info"), "注意");
    tabBar3->addTab(QIcon::fromTheme("kylin-music"), "Sliding4");
    tabBar3->addTab("Sliding5");
    tabBar3->addTab(QIcon::fromTheme("kylin-music"), "Sliding6");
    //    tabBar3->setIconSize(QSize(24,24));
    KNativeTabbar *tabBar4 = new KNativeTabbar(KNativeTabbarStyle::Sliding, w);
    tabBar4->addTab("Segment1");
    tabBar4->addTab("Segment2");
    tabBar4->addTab("Segment3");
    tabBar4->addTab("Segment4");
    tabBar4->setTabbarStyle(KNativeTabbarStyle::SegmentDark); // 设置style
    tabBar4->setBorderRadius(10); // 设置圆角半径，只对SegmentDark，SegmentLight样式生效
    tabBar4->setBackgroundColor(QColor(0, 255, 0)); // 设置背景色

    QLabel *pLabel = new QLabel(w);
    pLabel->setAlignment(Qt::AlignCenter);

    QMenu *menu = new QMenu();
    QAction *action1 = new QAction("action1", menu);
    QAction *action2 = new QAction("action2", menu);
    QAction *action3 = new QAction("action3", menu);
    menu->addAction(action1);
    menu->addAction(action2);
    menu->addAction(action3);

    connect(tabBar1, &KNativeTabbar::tabBarClicked, this, [=](int index) {
        pLabel->setText(tabBar1->tabText(index));
    });
    connect(tabBar2, &KNativeTabbar::tabBarClicked, this, [=](int index) {
        pLabel->setText(tabBar2->tabText(index));
    });
    connect(tabBar3, &KNativeTabbar::tabBarClicked, this, [=](int index) {
        pLabel->setText(tabBar3->tabText(index));
    });
    connect(tabBar4, &KNativeTabbar::tabBarClicked, this, [=](int index) {
        pLabel->setText(tabBar4->tabText(index));
    });
    connect(tabBar4, &KNativeTabbar::rightlicked, this, [=](QPoint point) {
        menu->move(point);
        menu->show();
    });

    QHBoxLayout *hlayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText("just a test label");
    hlayout->addWidget(label);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(tabBar1);
    vLayout->addWidget(tabBar2);
    vLayout->addWidget(tabBar3);
    vLayout->addWidget(tabBar4);
    vLayout->addStretch();
    vLayout->addWidget(pLabel);
    vLayout->addStretch();
    vLayout->addLayout(hlayout);
    vLayout->addStretch();
    setLayout(vLayout);
}

Widget::~Widget()
{
}
