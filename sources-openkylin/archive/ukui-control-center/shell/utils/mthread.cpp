/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mthread.h"
#include <unistd.h>
#include <QTime>

MThread::MThread(QObject *parent) : QObject(parent)
{

}

MThread::~MThread()
{

}

void MThread::run()
{
    qDebug()<<QThread::currentThreadId();
    QTime timedebuge;//声明一个时钟对象
    timedebuge.start();//开始计时
    m_cloudInterface = new QDBusInterface("org.kylinssoclient.dbus",
                                          "/org/kylinssoclient/path",
                                          "org.freedesktop.kylinssoclient.interface",
                                          QDBusConnection::sessionBus());
    if (!m_cloudInterface->isValid()) {
        qDebug() << "fail to connect to service";
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }
    QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"),
                                          QString(
                                              "org.freedesktop.kylinssoclient.interface"), "keyChanged", this,
                                          SLOT(keychanged(QString)));
    // 将以后所有DBus调用的超时设置为 milliseconds
    m_cloudInterface->setTimeout(2147483647); // -1 为默认的25s超时
     qDebug()<<"NetWorkAcount"<<"  线程耗时: "<<timedebuge.elapsed()<<"ms";
}

void MThread::keychanged(QString str)
{
    emit keychangedsignal(str);
}
