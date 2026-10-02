/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
 */
#ifndef _PROCESS_POOL_H__
#define _PROCESS_POOL_H__
#include <QObject>
#include <QProcess>
#include <QMap>
#include <QMutex>

class ProcessPool : public QObject {
    Q_OBJECT
public:
    explicit ProcessPool(int maxSize = 10, QObject *parent = nullptr);
    bool startProcess(const QString &program, const QStringList &args);
    void stopAllProcesses();

private slots:
    void handleProcessFinished(int exitCode);

private:
    const char *c_processId = "processId";
    QMutex m_mutex;
    int m_maxPoolSize;
    QMap<quint64, QProcess*> m_processes;
};

#endif //_PROCESS_POOL_H__
