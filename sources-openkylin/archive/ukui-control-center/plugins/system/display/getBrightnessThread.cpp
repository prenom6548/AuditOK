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
#include "getBrightnessThread.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

GetBrightnessThread::GetBrightnessThread(QString edidHash, QString i2cBus)
{
    this->edidHash = edidHash;
    this->i2cBus   = i2cBus;
    this->exitFlag = false;
}

GetBrightnessThread::~GetBrightnessThread()
{

}

void GetBrightnessThread::run()
{
    int times = 600;
    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());
    QDBusReply<int> reply;
    while (times) {
        //防止调用terminate不会结束线程，锁屏在龙芯架构上出现过
        if (exitFlag) {
            return;
        }
        if (times %10 != 0) {
            times--;
            msleep(100);
            continue;
        }
        if (this->edidHash == "") {
            Q_EMIT getBrightnessFinished(-1);
            return;
        }
        reply = ukccIfc.call("getDisplayBrightness", this->edidHash, this->i2cBus);
        if (reply.isValid() && reply.value() >= 0 && reply.value() <= 100) {
            qDebug() << Q_FUNC_INFO << "the ddc brightness is: " << reply.value() << this->edidHash << this->i2cBus;
            Q_EMIT getBrightnessFinished(reply.value());
            return;
        }
        times--;
        msleep(100);
    }
    Q_EMIT getBrightnessFinished(-1);
    return;
}

void GetBrightnessThread::setExit(bool b)
{
    exitFlag = b;
}


