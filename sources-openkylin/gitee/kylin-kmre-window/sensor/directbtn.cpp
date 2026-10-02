/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "directbtn.h"
#include <QPushButton>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QDBusReply>
#include <QString>

#include <QDebug>

DirectBtn::DirectBtn(QWidget *parent):QPushButton (parent)
{
    this->resize(80,80);
    num = new QLabel(this);
    num ->hide();
    connect(this,&DirectBtn::clicked,this,&DirectBtn::connectServer);
}

void DirectBtn::connectServer(){
    QString direction = num->text();
    int btnNum = direction.toInt();
    qDebug()<<direction <<btnNum;
    QString sensorData = "";
    switch (btnNum) {
    case 1:
        sensorData = "0.0:-8.:0.0";
        break;
    case 2:
        sensorData = "0.0:8.0:0.0";
        break;
    case 3:
        sensorData = "8.0:0.0:0.0";
        break;
    case 4:
        sensorData = "-8.:0.0:0.0";
        break;
    case 5:
        sensorData = "8.0:-8.:0.0";
        break;
    case 6:
        sensorData = "8.0:8.0:0.0";
        break;
    case 7:
        sensorData = "-8.:-8.:0.0";
        break;
    case 8:
        sensorData = "-8.:8.0:0.0";
        break;
    case 9:
        sensorData = "0.0:0.0:9.8";
        break;
    default:
        break;
    }

    if (sensorData != ""){
        QDBusInterface *m_gpsBusInterface =
                new QDBusInterface("com.kylin.Kmre.sensor", "/", "com.kylin.Kmre.sensor", QDBusConnection::sessionBus());
        if (m_gpsBusInterface->isValid()) {
            QDBusReply<QString> reply = m_gpsBusInterface->call("passAcceKey", sensorData);
            if (!reply.isValid()) {
                // syslog(LOG_ERR, "Failed to call sensor server interface");
            }
            delete m_gpsBusInterface;
            m_gpsBusInterface = nullptr;
        }
    }

}
