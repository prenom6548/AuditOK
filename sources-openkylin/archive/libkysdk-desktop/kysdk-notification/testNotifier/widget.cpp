/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "widget.h"
#include <QHBoxLayout>
#include <QDBusConnection>
#include <QDebug>
#include <QProcess>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QPushButton *btn1 = new QPushButton("常规通知",this);
    QPushButton *btn2 = new QPushButton("通知内容替换",this);
    QPushButton *btn3 = new QPushButton("常驻通知",this);
    QPushButton *btn4 = new QPushButton("关闭通知",this);

    hLayout->addWidget(btn1);
    hLayout->addWidget(btn2);
    hLayout->addWidget(btn3);
    hLayout->addWidget(btn4);

    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications", "ActionInvoked", this,
                                          SLOT(onActionInvoked(quint32, QString))
    );

    connect(btn1,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("ukui-control-center");
        notifier.setAppIcon("ukui-control-center");
        notifier.setBodyText("bodyText");
        notifier.setSummary("summary");
        notifier.setDefaultAction("ukui-control-center");
        notifier.addAction("kylin-music","music");
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });

    connect(btn2,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("ukui-control-center");
        notifier.setAppIcon("ukui-control-center");
        notifier.setBodyText("replaced bodyText");
        notifier.setSummary("summary");

        notifier.addAction("kylin-music","music");
        notifier.setDefaultAction("ukui-control-center");
        notifier.setReplaceId(1);
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });

    connect(btn3,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("ukui-control-center");
        notifier.setAppIcon("ukui-control-center");
        notifier.setBodyText("bodyText");
        notifier.setSummary("summary");
        notifier.setDefaultAction("ukui-control-center");
        notifier.addAction("kylin-music","music");
        notifier.setShowTime(KNotifier::AllTheTime);
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });


    connect(btn4,&QPushButton::clicked,this,[=](){
        KNotifier::closeNotification(m_id);
    });
}

Widget::~Widget()
{
}

void Widget::onActionInvoked(quint32 id, QString actionKey)
{
    qDebug()<<id <<actionKey;
    QProcess *process = new QProcess(this);
    QStringList l;
    if(actionKey == "kylin-music")
    {
        l <<"kylin-music";
        process->start("kylin-music",l);
    }
    else
    {
        l <<"ukui-control-center";
        process->start("ukui-control-center",l);
    }
}

