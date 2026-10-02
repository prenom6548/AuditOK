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

#include "kloopplaypage.h"
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    //    QWidget*widget1 = new QWidget();

    QLabel *label1 = new QLabel();
    QPixmap pixmap1(":/1.jpeg");
    label1->setFixedWidth(400);
    label1->setPixmap(pixmap1.scaled(400, 200));
    //    QPushButton* btn1 = new QPushButton(widget1);
    //    btn1->move(100,50);
    //    btn1->setText("label1");

    //    QWidget*widget2 = new QWidget();
    QLabel *label2 = new QLabel();
    QPixmap pixmap2(":/2.jpeg");
    label2->setPixmap(pixmap2.scaled(400, 200));
    //    QPushButton* btn2 = new QPushButton(widget2);
    //    btn2->move(100,50);
    //    btn2->setText("label2");

    //    QWidget*widget3 = new QWidget(this);
    QLabel *label3 = new QLabel();
    QPixmap pixmap3(":/3.jpg");
    label3->setPixmap(pixmap3.scaled(400, 200));
    //    QPushButton* btn3 = new QPushButton(widget3);
    //    btn3->move(100,50);
    //    btn3->setText("label2");

    //    QWidget*widget4 = new QWidget();
    QLabel *label4 = new QLabel();
    QPixmap pixmap4(":/4.jpg");
    label4->setPixmap(pixmap4.scaled(400, 200));
    //    QPushButton* btn4 = new QPushButton(widget4);
    //    btn4->move(100,50);
    //    btn4->setText("label2");

    QHBoxLayout *layout = new QHBoxLayout(this);
    KLoopPlayPage *page = new KLoopPlayPage();
    page->addWidget(label1);
    page->addWidget(label2);
    page->addWidget(label3);
    page->addWidget(label4);
    page->setCurrentWidget(0);
    layout->addWidget(page);

    setFixedWidth(600);
}

Widget::~Widget()
{
}
