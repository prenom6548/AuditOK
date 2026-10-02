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
#include "kconf2.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <libkysettings.h>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test");
        connect(settings, &KConf2::changed, this, [=](const QString &key) {
            qDebug() << "恭喜你，gsetting切换成功了,变化的key是" << key << "style-name is" << settings->get("style-name").toString();
        });
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *btn1 = new QPushButton("get value");
    QPushButton *btn2 = new QPushButton("set enabled");
    QPushButton *btn3 = new QPushButton("get keys");
    QPushButton *btn4 = new QPushButton("schemas");
    QPushButton *btn5 = new QPushButton("getDescrition");
    QPushButton *btn6 = new QPushButton("get summary");
    QPushButton *btn7 = new QPushButton("get type");
    QPushButton *btn8 = new QPushButton("is Writable");
    QPushButton *btn9 = new QPushButton("get id ");
    QPushButton *btn10 = new QPushButton("get version ");
    QPushButton *btn11 = new QPushButton("get child ");
    QPushButton *btn12 = new QPushButton("children ");

    layout->addWidget(btn1);
    layout->addWidget(btn2);
    layout->addWidget(btn3);
    layout->addWidget(btn4);
    layout->addWidget(btn5);
    layout->addWidget(btn6);
    layout->addWidget(btn7);
    layout->addWidget(btn8);
    layout->addWidget(btn9);
    layout->addWidget(btn10);
    layout->addWidget(btn11);
    layout->addWidget(btn12);

    connect(btn1, &QPushButton::clicked, this, [=] {
        onClicked1();
    });

    connect(btn2, &QPushButton::clicked, this, [=] {
        onClicked2();
    });

    connect(btn3, &QPushButton::clicked, this, [=] {
        onClicked3();
    });

    connect(btn4, &QPushButton::clicked, this, [=] {
        onClicked4();
    });

    connect(btn5, &QPushButton::clicked, this, [=] {
        onClicked5();
    });

    connect(btn6, &QPushButton::clicked, this, [=] {
        onClicked6();
    });

    connect(btn7, &QPushButton::clicked, this, [=] {
        onClicked7();
    });

    connect(btn8, &QPushButton::clicked, this, [=] {
        onClicked8();
    });

    connect(btn9, &QPushButton::clicked, this, [=] {
        onClicked9();
    });

    connect(btn10, &QPushButton::clicked, this, [=] {
        onClicked10();
    });

    connect(btn11, &QPushButton::clicked, this, [=] {
        onClicked11();
    });

    connect(btn12, &QPushButton::clicked, this, [=] {
        onClicked12();
    });
}

Widget::~Widget()
{
}

void Widget::onClicked1()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        auto stylename = settings->get("int").toString();
        qDebug() << "stylename =" << stylename;
    }
}

void Widget::onClicked2()
{

    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.powermanager");
        if (settings->get("sleep").toBool())
            settings->set("sleep", "false");
        else
            settings->set("sleep", "true");
        qDebug() << "success" << settings->get("int").toString();
    }
}

void Widget::onClicked3()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        auto list = settings->keys();
        qDebug() << "keys list is" << list;
    }
}

void Widget::onClicked4()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        auto list = settings->schemas("test", "3.20.1.31");
        qDebug() << "schemas list is" << list;
    }
}

void Widget::onClicked5()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        QString str("int");
        QString str1 = settings->getDescrition(str);
        qDebug() << "getDescrition is" << str1;
    }
}

void Widget::onClicked6()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        QString str("int");
        QString str1 = settings->getSummary(str);
        qDebug() << "getSummary is" << str1;
    }
}

void Widget::onClicked7()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        QString str("int");
        QString str1 = settings->getType(str);
        qDebug() << "getType is" << str1;
    }
}

void Widget::onClicked8()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        QString str("int");
        settings->isWritable(str);
        qDebug() << "isWritable " << settings->isWritable(str);
    }
}

void Widget::onClicked9()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        qDebug() << "getId is" << settings->getId();
    }
}

void Widget::onClicked10()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        qDebug() << "getVersion is" << settings->getVersion();
    }
}

void Widget::onClicked11()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        QString str("int");
        KConf2 *gsetting = settings->getChild(str);
        qDebug() << "getChild is" << gsetting;
    }
}

void Widget::onClicked12()
{
    if (KConf2::isSettingsAvailable("test")) {
        KConf2 *settings = new KConf2("test.notice");
        auto list = settings->children();
        qDebug() << "children list is" << list;
    }
}
