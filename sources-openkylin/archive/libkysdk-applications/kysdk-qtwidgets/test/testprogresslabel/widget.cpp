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
#include "kprogresslabel.h"
#include <QDebug>
#include <QPushButton>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    QHBoxLayout *layout = new QHBoxLayout();
    KProgressLabel *label = new KProgressLabel();
    QStringList list;
    list.append("11");
    list.append("22");
    list.append("33");
    list.append("44");
    list.append("55");

    QStringList list2;
    list2.append("1");
    list2.append("2");
    list2.append("3");
    list2.append("4");
    list2.append("5");
    label->setStringList(list);
    label->setCircleSize(QSize(24, 24));
    label->setCurrentIndex(1);
    //    label->setStyleSheet("background :red");

    QPushButton *btn1 = new QPushButton("");

    QPushButton *btn2 = new QPushButton("");

    layout->addWidget(btn1);
    layout->addWidget(btn2);
    vlayout->addWidget(label);
    vlayout->addLayout(layout);
    connect(btn1, &QPushButton::clicked, this, [=] {
        label->setCurrentIndex(label->currentIndex() - 1);
        qDebug() << label->size();
    });
    connect(btn2, &QPushButton::clicked, this, [=] {
        label->setCurrentIndex(label->currentIndex() + 1);
    });
    setFixedSize(600, 500);
}

Widget::~Widget()
{
}
