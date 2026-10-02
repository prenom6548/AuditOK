/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "input-process-settings.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDirIterator>
extern "C" {
#include "clib-syslog.h"
}

ProcessSettings::ProcessSettings(QObject *parent) : QObject(parent)
{

}

ProcessSettings *ProcessSettings::instance()
{
    static ProcessSettings instance;
    return &instance;
}

void ProcessSettings::setLocatePointer(bool state)
{
    if (state) {
        if (!isProcessRunning("usd-locate-pointer")) {
            QString str = "usd-locate-pointer";
            m_locatePointerRunning = QProcess::startDetached(str);
        }

    } else {
        if (isProcessRunning("usd-locate-pointer")) {
            QString str = "killall usd-locate-pointer";
            QProcess::startDetached(str);
        }
    }
}


void ProcessSettings::setDisableWTypingSynaptics(bool state)
{
    if (state) {
        if (!isProcessRunning("syndaemon")) {
            QString cmdStart = "syndaemon -i 0.3 -K -R";
            QProcess::startDetached(cmdStart);
        }
    } else {
        if (isProcessRunning("syndaemon")) {
            QString cmdKill = "killall syndaemon";
            QProcess::startDetached(cmdKill);
        }
    }
}

bool ProcessSettings::isProcessRunning(const QString& processName)
{
    QDirIterator dirIterator("/proc", QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIterator.hasNext()) {
        dirIterator.next();
        bool ok;
        dirIterator.fileName().toInt(&ok);
        if (!ok) {
            continue;
        }

        QFile comdLineFile("/proc/" + dirIterator.fileName() + "/cmdline");
        if (comdLineFile.open(QIODevice::ReadOnly)) {
            QString processComm = comdLineFile.readLine().trimmed();
            comdLineFile.close();

            if (processComm.contains(processName)) {
                return true;
            }
        }
    }
    return false;
}
