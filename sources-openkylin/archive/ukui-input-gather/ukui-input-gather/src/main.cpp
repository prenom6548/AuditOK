/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QCoreApplication>

#include <QDebug>
#include <QTime>
#include <QThread>

#include "libinputeventgather.h"
#include "localserver.h"
#include "socketserver.h"
#include "libinputdbusserver.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<Event>("Event");

    QCoreApplication a(argc, argv);

    LibinputEventGather libinputEventGather;

    SocketServer::self()->run();
    // LocalServer::self()->runServer();

    LibinputDBusServer dbusserver;

    QObject::connect(&libinputEventGather, &LibinputEventGather::sendEvent, &a, [=](Event event){
        // qDebug() << "connect thread: " << QThread::currentThread();
        // LocalServer::self()->send(event);
        SocketServer::self()->send(event);
    }, Qt::ConnectionType::QueuedConnection);

    libinputEventGather.start();
    return a.exec();
}
