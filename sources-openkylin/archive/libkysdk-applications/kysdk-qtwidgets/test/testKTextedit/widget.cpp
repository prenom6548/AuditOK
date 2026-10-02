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
#include "kbubblewidget.h"
#include "ktextedit.h"
#include <QBoxLayout>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *btn1 = new QPushButton();
    btn1->setText("无tag-正常背景");
    QPushButton *btn2 = new QPushButton();
    btn2->setText("无tag-高亮背景");
    QPushButton *btn3 = new QPushButton();
    btn3->setText("有tag-正常背景");
    QPushButton *btn4 = new QPushButton();
    btn4->setText("有tag-高亮背景");

    KBubbleWidget *bubbleWidget1 = new KBubbleWidget();
    bubbleWidget1->setTailPosition(TopDirection);
    QVBoxLayout *layout1 = new QVBoxLayout(bubbleWidget1);
    KTextEdit *edit1 = new KTextEdit();

    QPushButton *button1 = new QPushButton();
    button1->setText("关闭");
    layout1->addWidget(edit1);
    layout1->addWidget(button1);
    connect(btn1, &QPushButton::clicked, this, [=] {
        bubbleWidget1->show();
    });
    connect(button1, &QPushButton::clicked, this, [=] {
        bubbleWidget1->hide();
    });

    KBubbleWidget *bubbleWidget2 = new KBubbleWidget();
    bubbleWidget2->setTailPosition(RightDirection, TailLocation::LeftLocation);
    bubbleWidget2->setHighlightBackground(true);
    QVBoxLayout *layout2 = new QVBoxLayout(bubbleWidget2);
    KTextEdit *edit2 = new KTextEdit();
    edit2->setReadOnly(true);
    edit2->setText("这是一段测试性文字，用来测试ktextedit在只读状态（不可编辑状态）下的背景色。");

    QPushButton *button2 = new QPushButton();
    button2->setText("关闭");
    layout2->addWidget(edit2);
    layout2->addWidget(button2);
    connect(btn2, &QPushButton::clicked, this, [=] {
        bubbleWidget2->show();
    });
    connect(button2, &QPushButton::clicked, this, [=] {
        bubbleWidget2->hide();
    });

    KBubbleWidget *bubbleWidget3 = new KBubbleWidget();
    bubbleWidget3->setTailPosition(BottomDirection, TailLocation::RightLocation);
    QVBoxLayout *layout3 = new QVBoxLayout(bubbleWidget3);
    KTextEdit *edit3 = new KTextEdit();

    edit3->setLabelVisible(true);
    edit3->setPixmap(QIcon("system-computer-symbolic").pixmap(16, 16));
    edit3->setLabelText("翻译");

    QPushButton *button3 = new QPushButton();
    button3->setText("关闭");
    layout3->addWidget(edit3);
    layout3->addWidget(button3);
    connect(btn3, &QPushButton::clicked, this, [=] {
        bubbleWidget3->show();
    });
    connect(button3, &QPushButton::clicked, this, [=] {
        bubbleWidget3->hide();
    });

    KBubbleWidget *bubbleWidget4 = new KBubbleWidget();
    bubbleWidget4->setTailPosition(LeftDirection, TailLocation::RightLocation);
    bubbleWidget4->setHighlightBackground(true);
    QVBoxLayout *layout4 = new QVBoxLayout(bubbleWidget4);
    KTextEdit *edit4 = new KTextEdit();
    edit4->setLabelVisible(true);
    edit4->setPixmap(QIcon("system-computer-symbolic").pixmap(16, 16));
    edit4->setLabelText("翻译");
    edit4->setText("这是一段测试性文字，用来测试ktextedit在只读状态（不可编辑状态）下的背景色。");
    edit4->setReadOnly(true);

    QPushButton *button4 = new QPushButton();
    button4->setText("关闭");
    QPushButton *button5 = new QPushButton();
    button5->setText("切换是否可编辑状态");
    layout4->addWidget(edit4);
    layout4->addWidget(button4);
    layout4->addWidget(button5);
    connect(btn4, &QPushButton::clicked, this, [=] {
        bubbleWidget4->show();
    });
    connect(button4, &QPushButton::clicked, this, [=] {
        bubbleWidget4->hide();
    });

    connect(button5, &QPushButton::clicked, this, [=] {
        if (edit4->isReadOnly()) {
            edit4->setReadOnly(false);
            bubbleWidget4->setHighlightBackground(false);
        } else {
            edit4->setReadOnly(true);
            bubbleWidget4->setHighlightBackground(true);
        }
    });

    QHBoxLayout *hlayout1 = new QHBoxLayout();
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    hlayout1->addWidget(btn1);
    hlayout1->addWidget(btn2);
    hlayout2->addWidget(btn3);
    hlayout2->addWidget(btn4);
    mainLayout->addLayout(hlayout1);
    mainLayout->addLayout(hlayout2);
}

Widget::~Widget()
{
}
