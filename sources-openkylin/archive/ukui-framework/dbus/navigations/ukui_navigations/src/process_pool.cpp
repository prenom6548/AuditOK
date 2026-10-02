/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
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
 */
#include <QDebug>
#include "process_pool.h"

ProcessPool::ProcessPool(int maxSize, QObject *parent)
    : QObject(parent), m_maxPoolSize(maxSize) {}

//运行一半退出的怎么算呢？
bool ProcessPool::startProcess(const QString &program, const QStringList &args) {
    QMutexLocker locker(&m_mutex);
    if(m_processes.size() >= m_maxPoolSize) {
        qWarning()<<m_processes.size() <<" > "<<m_maxPoolSize;
        return false;
    }
    QProcess *process = new QProcess(this);

    connect(process, QOverload<int>::of(&QProcess::finished),
            this, &ProcessPool::handleProcessFinished);

    process->start(program, args);
    if(!process->waitForStarted(3000)) {
        delete process;
        return false;
    }
    qDebug()<<process->processId()<<"start";
    process->setProperty(c_processId, process->processId());
    m_processes.insert(process->processId(), process);
    return true;
}

void ProcessPool::handleProcessFinished(int exitCode) {
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        QMutexLocker locker(&m_mutex);
        qDebug()<<process->property(c_processId).toUInt()<<"exit" << exitCode;
        m_processes.remove(process->property(c_processId).toUInt());
        process->deleteLater();
    }
}

void ProcessPool::stopAllProcesses() {
    QMutexLocker locker(&m_mutex);
    for(auto it = m_processes.begin(); it != m_processes.end(); ++it) {
        it.value()->terminate();
    }
}


