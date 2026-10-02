/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "worker.h"
#include "utils.h"
#include "dbus_client.h"
#include "dialog.h"
#include "preinstallappmanager.h"

#include <unistd.h>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QThread>

Worker::Worker(const QString &pkg, QObject *parent) :
    QObject(parent)
    , m_timer(new QTimer(this))
    , m_pkg(pkg)
{
    m_timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    m_timer->start(1000);
}

Worker::~Worker()
{
}

void Worker::onTimeOut()
{
    bool b = Utils::isAndroidDeskStart();
    syslog(LOG_DEBUG, "Android %s !", b ? "started" : "have not started yet");
    if (b) {
        m_timer->stop();//stop checking if the container started
        emit this->androidRunCompleted();//emit signal to close dialog window on right bottom
        PreInstallAppManager::waitForPreInstallFinished();

        if (m_pkg != "start_kmre") {
            //Utils::awakeAndroidDesk();
            Utils::startApplication(m_pkg);
        }

        QString homeDirPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QString lockDirPath = homeDirPath + "/.kmre";
        QString lockFilePath = lockDirPath + "/startapp-lock";
        std::string str = lockFilePath.toStdString();
        const char* ch = str.c_str();
        if (remove(ch) != 0) {
            //qDebug()<<"delete file startapp-lock failed!!!";
        }
    }
}

