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
#include "kclocktimer.h"
#include <QBoxLayout>
#include <QDebug>
#include <QPushButton>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    KClockTimer *clock = new KClockTimer(this);

    QHBoxLayout *hlayout = new QHBoxLayout();
    QPushButton *btn1 = new QPushButton("开始");
    QPushButton *btn2 = new QPushButton("停止");
    QPushButton *btn3 = new QPushButton("复位");
    QPushButton *btn4 = new QPushButton("继续");
    QPushButton *btn5 = new QPushButton("计次");

    hlayout->addWidget(btn1);
    hlayout->addWidget(btn2);
    vlayout->addWidget(clock);
    vlayout->addLayout(hlayout);

    hlayout = new QHBoxLayout();
    hlayout->addWidget(btn3);
    hlayout->addWidget(btn4);
    vlayout->addLayout(hlayout);

    hlayout = new QHBoxLayout();
    hlayout->addWidget(btn5);
    vlayout->addLayout(hlayout);

    connect(btn1, &QPushButton::clicked, clock, &KClockTimer::startStopwatch);
    connect(btn2, &QPushButton::clicked, clock, &KClockTimer::stopStopwatch);
    connect(btn3, &QPushButton::clicked, clock, &KClockTimer::resetStopwatch);
    connect(btn4, &QPushButton::clicked, clock, &KClockTimer::startStopwatch);
    connect(btn5, &QPushButton::clicked, this, [=] {
        qDebug() << clock->time();
    });

    //    qint64 otherMilliseconds = stringToMilliseconds(otherTimeString); // 转换为毫秒数

    //    // 比较两个时间
    //    if (compareMilliseconds(currentMilliseconds, otherMilliseconds)) {
    //        // currentMilliseconds小于otherMilliseconds
    //    } else {
    //        // currentMilliseconds大于或等于otherMilliseconds
    //    }

    //    setFixedSize(300,200);
}

Widget::~Widget()
{
}
