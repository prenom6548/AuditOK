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
#include "getsyncres.h"
#include <sys/timex.h>
#include <QDBusInterface>
#include <QDebug>
#include <QtMath>

GetSyncRes::GetSyncRes(QObject *parent)
    : QThread(parent)
{

}

GetSyncRes::~GetSyncRes()
{

}
void GetSyncRes::run()
{
    for(int i = 0; i < 300; ++i) {
        struct timex txc = {};
        if (adjtimex(&txc) < 0 || txc.maxerror >= 16000000) {  //未能同步时间
            int picNum = i - qFloor(i/8)*8; //限制在0~7
            QString pixName = QString(":/images/loading%1.svg").arg(picNum+10);
            QPixmap pix(pixName);
            Q_EMIT inSync(pix);
            msleep(70);
            continue;
        } else {                                               //同步时间成功
            Q_EMIT syncStatus(true);
            return;
        }
    }
    Q_EMIT syncStatus(false);
    QDBusInterface *r_datetimeiface = new QDBusInterface("org.freedesktop.timedate1",
                                           "/org/freedesktop/timedate1",
                                           "org.freedesktop.timedate1",
                                           QDBusConnection::systemBus());
    while (true) {
        r_datetimeiface->call("SetNTP", true, true);
        struct timex txc = {};
        if (adjtimex(&txc) >= 0 && txc.maxerror < 16000000) { //同步时间成功
            Q_EMIT syncStatus(true);
            delete r_datetimeiface;
            r_datetimeiface = nullptr;
            return;
        }
        sleep(1);
    }
    return;
}
