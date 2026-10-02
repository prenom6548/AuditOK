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
#include "klabel.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    QVBoxLayout *layout = new QVBoxLayout();

    KLabel *label1 = new KLabel();
    label1->setText("我是一个测试tooltip的lable");
    label1->setBackgroundColor(true, Qt::red);
    //    label1->setFixedWidth(100);

    KLabel *label2 = new KLabel(this);
    label2->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16, 16));
    label2->setDataHightColor(true);
    label2->setLabelType(KLabelType::DataType);
    label2->setText("我是一个测试lable");

    KLabel *label3 = new KLabel();
    label3->setFixedWidth(100);
    label3->setText("测\n测dawdadafauphfuawhfawhfp");
    label3->setWordWrap(true);
    label3->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16, 16));
    label3->setLabelType(KLabelType::DataType);
    label3->setBackgroundColor(Qt::white);
    label3->setDataHightColor(true);

    KLabel *label4 = new KLabel();
    label4->setLabelType(KLabelType::CircleType);
    label4->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16, 16));
    label4->setFixedSize(50, 50);
    QFont font1;
    int i = font().pointSize();
    font1.setPointSize(i + 16);
    label1->setFont(font1);

    label4->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
    label4->setBackgroundColor(true, Qt::green);

    QPushButton *btn1 = new QPushButton("默认");
    QPushButton *btn2 = new QPushButton("高亮背景");
    QPushButton *btn3 = new QPushButton("自定义背景");
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(btn1);
    hlayout->addWidget(btn2);
    hlayout->addWidget(btn3);

    connect(btn1, &QPushButton::clicked, this, [=] {
        label4->setHighlightBackground(false);
        label4->setBackgroundColor(false);
        label4->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16, 16));
    });
    connect(btn2, &QPushButton::clicked, this, [=] {
        label4->setHighlightBackground(true);
        label4->setPixmap(QIcon::fromTheme("system-computer-symbolic").pixmap(16, 16));
    });
    connect(btn3, &QPushButton::clicked, this, [=] {
        label4->setBackgroundColor(true, QColor(82, 196, 41));
        label4->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
    });

    KLabel *label5 = new KLabel(this);
    label5->setText("测试进入/离开信号/点击信号/双击信号");
    connect(label5, &KLabel::enterWidget, this, [=] {
        qDebug() << "enter label5";
    });
    connect(label5, &KLabel::leaveWidget, this, [=] {
        qDebug() << "leave label5";
    });
    connect(label5, &KLabel::clicked, this, [=](bool checked) {
        qDebug() << "clicked label5" << checked;
    });
    connect(label5,&KLabel::doubleClicked,this,[=](){
        qDebug()<<"doubleClicked label5";
    });

    QHBoxLayout *hlayout1 = new QHBoxLayout(this);

    KLabel *label6 = new KLabel(this);
    label6->setText("测试三态");
    label6->setTristate(true);

    QPushButton *btn = new QPushButton(this);
    connect(btn, &QPushButton::clicked, this, [=] {
        if (label6->isTristate())
            label6->setTristate(false);
        else
            label6->setTristate(true);
    });

    hlayout1->addWidget(label6);
    hlayout1->addWidget(btn);

    QHBoxLayout *hlayout2 = new QHBoxLayout(this);

    KLabel *label7 = new KLabel(this);
    label7->setText("测试font color role");
    label7->setFontColorRole(QPalette::PlaceholderText);
    hlayout2->addWidget(label7);

    layout->addWidget(label1);
    layout->addWidget(label2);
    layout->addWidget(label3);
    layout->addLayout(hlayout);
    layout->addWidget(label4);
    layout->addWidget(label5);
    layout->addLayout(hlayout1);
    layout->addLayout(hlayout2);
    vlayout->addLayout(layout);
}

Widget::~Widget()
{
}
