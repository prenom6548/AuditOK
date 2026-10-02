/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "leavelock.h"
#include "sessiondbusregister.h"

LeaveLock::LeaveLock(QObject *parent) : QObject(parent)
{
}

LeaveLock::~LeaveLock() {
    if(m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}

bool LeaveLock::setPing(QString uuid, QString dev, bool on) {
    if (dev == "" || !on) {
        if(m_process) {
            m_process->disconnect(this);
            m_process->deleteLater();
            m_process = nullptr;
        }
        m_manualCancel = true;
        return true;
    }
    m_process = new QProcess();
    connect(m_process, &QProcess::readyReadStandardOutput, this, &LeaveLock::readOutput);
    connect(m_process, &QProcess::readChannelFinished, this, &LeaveLock::pingFinished);
    m_manualCancel = false;
    m_uid = uuid;
    m_lockdev = dev;
    KyDebug() << m_lockdev;
    QString cmd = "l2ping " + m_lockdev;
    m_process->start(cmd);
    return true;
}

void LeaveLock::readOutput() {
    Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(m_process->readAllStandardOutput());
}

void LeaveLock::pingFinished() {
    if(m_process) {
        QByteArray errorMsg = m_process->readAllStandardError();
        QByteArray output = m_process->readAllStandardOutput();
        KyDebug() << output << errorMsg;
        if (!output.isEmpty()) {
            Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(output);
        }
        if (!errorMsg.isEmpty()) {
            Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(errorMsg);
            return;
        }
        if (m_manualCancel)
            Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(QString("ping out").toLatin1());
        else
            Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(QString("loss connect").toLatin1());
    }

    m_manualCancel = false;
}
