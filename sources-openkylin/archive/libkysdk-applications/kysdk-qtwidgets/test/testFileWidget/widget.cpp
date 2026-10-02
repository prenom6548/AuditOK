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
#include "kfilewidget.h"
#include "kpressbutton.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QObject>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();

    KFileWidget *widget1 = new KFileWidget(this);
    widget1->setFixedSize(150, 56);
    widget1->setMainText("文档.word");
    widget1->setSubText("大小 1MB");
    widget1->setIcon(QIcon::fromTheme("wps-office2019-wpsmain"));
    widget1->setBorderRadius(0, 10, 0, 10);
    connect(widget1, &KFileWidget::clicked, this, [=] {
        widget1->hide();
    });

    KFileWidget *widget2 = new KFileWidget(this);
    widget2->setMainText("文档.word");
    widget2->setIcon(QIcon::fromTheme("wps-office2019-wpsmain"));
    widget2->setEnabled(false);
    connect(widget2, &KFileWidget::clicked, this, [=] {
        widget2->hide();
    });

    KFileWidget *widget3 = new KFileWidget(this);
    widget3->setMainText("文档.word");
    widget3->setIcon(QIcon::fromTheme("wps-office2019-wpsmain"));
    widget3->setFixedSize(150, 56);
    widget3->setBorderRadius(0);
    connect(widget3, &KFileWidget::clicked, this, [=] {
        widget3->hide();
    });

    layout->addStretch();
    layout->addWidget(widget1);
    layout->addWidget(widget2);
    layout->addWidget(widget3);
    layout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addLayout(layout);

    baseBar()->setLayout(hlayout);
}

Widget::~Widget()
{
}
